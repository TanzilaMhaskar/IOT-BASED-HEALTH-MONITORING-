 

#include "ThingSpeak.h"
#include "WiFi.h"
 
#include <OneWire.h>
#include <DallasTemperature.h> //15 to 125 black=gnd red=vcc yellow=output

#include <Wire.h>
#include <DFRobot_MAX30102.h>
DFRobot_MAX30102 particleSensor;

#include "DHT.h"
#define DHTPIN 19    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11  
DHT dht(DHTPIN, DHTTYPE);

const int sensor =34;
const int oneWireBus = 4; 
OneWire oneWire(oneWireBus); //setup to communicate any onewire device
DallasTemperature sensors(&oneWire); //pass onewire reference to Dallas Temperature sensor

//-------Enter your WiFi Details------//
char ssid[] = "AIM";  //SSID
char pass[] = "husna8412";  //Password
//-----------------------------------//
//-------------ThingSpeak Details-----------------//
unsigned long myChannelField =1678118  ;
const int ChannelField1= 1;
const int ChannelField2= 2;
const int ChannelField3= 3;
const int ChannelField4= 4;
const int ChannelField5= 5;
const int ChannelField6= 6;
const char * myWriteAPIKey = "EK0907D0BKO9O2S7"; //Your Write API Key
//-----------------------------------------------//
const int FAN = 15;
const int Humidifier =2;
const int Buzzer =13;
WiFiClient client;

//oximetrer
int32_t SPO2; //SPO2
int8_t SPO2Valid; //Flag to display if SPO2 calculation is valid
int32_t heartRate; //Heart-rate
int8_t heartRateValid; //Flag to display if heart-rate calculation is valid 
//--------------//

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
sensors.begin();
pinMode(FAN,OUTPUT);
pinMode(Humidifier,OUTPUT);
pinMode(sensor,INPUT);
pinMode(Buzzer,OUTPUT);
WiFi.mode(WIFI_STA);
ThingSpeak.begin(client);
dht.begin();
// pulse oximeter
if (!particleSensor.begin()) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102  was not found. Please check wiring/power. ");
    delay(1000);
  }
/*byte ledBrightness = 60; //Options: 0=Off to 255=50mA
byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
int pulseWidth = 411; //Options: 69, 118, 215, 411
int adcRange = 4096; //Options: 2048, 4096, 8192, 16384*/

//particleSensor.sensorConfiguration(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
particleSensor.sensorConfiguration(/*ledBrightness=*/50, /*sampleAverage=*/SAMPLEAVG_4, \
                        /*ledMode=*/MODE_MULTILED, /*sampleRate=*/SAMPLERATE_100, \
                        /*pulseWidth=*/PULSEWIDTH_411, /*adcRange=*/ADCRANGE_16384);
}


void loop() { 
  // put your main code here, to run repeatedly:
if(WiFi.status()!= WL_CONNECTED)
  {
    Serial.print("Attempting to Connect to SSID: ");
    Serial.println(ssid);  
  
  while(WiFi.status()!= WL_CONNECTED)
  {
    WiFi.begin(ssid,pass);
    Serial.print(".");
    delay(5000);
  }
  Serial.println("\nConnected.");
  }

//--------------BODY TEMPERATURE---------------//
sensors.requestTemperatures();
float temperatureF= sensors.getTempFByIndex(0);
Serial.print("Patient Temperature=");
Serial.print(temperatureF); 
Serial.println("F");
delay(500);
ThingSpeak.writeField(myChannelField,ChannelField1,temperatureF, myWriteAPIKey);
//-----------------------------------------------//
//---------------PULSE OXIMETER-----------------//
Serial.println(F("Wait about four seconds"));
  particleSensor.heartrateAndOxygenSaturation(/**SPO2=*/&SPO2, /**SPO2Valid=*/&SPO2Valid, /**heartRate=*/&heartRate, /**heartRateValid=*/&heartRateValid);
  //Print result 
  Serial.print(F("heartRate="));
  Serial.print(heartRate, DEC);
  Serial.println("BPM");
  
  Serial.print(F("SPO2="));
  Serial.println(SPO2, DEC);
  if(heartRate < 50 || heartRate == -999 || SPO2<85 || SPO2==-999) 
  {
    for(int i=0;i< 10;i++)
    {
    digitalWrite(Buzzer,HIGH);
    delay(500);
    digitalWrite(Buzzer,LOW);
    delay(500);
    }
  }
  /*else
  {
  digitalWrite(Buzzer,LOW);  
  }*/
  ThingSpeak.writeField(myChannelField,ChannelField2,SPO2, myWriteAPIKey);
  ThingSpeak.writeField(myChannelField,ChannelField3,heartRate, myWriteAPIKey);
//-----------------------------------------------------------//
//-----------------------code mq135--------------//
int sv =analogRead(sensor);
Serial.print("Air Quality=");
Serial.print(sv);
Serial.println("ppm");
delay(500);
ThingSpeak.writeField(myChannelField,ChannelField4,sv,myWriteAPIKey);
//----------------------------------------------//

//  ROOM TEMPERATURE Wait a few seconds between measurements-.
  delay(2000);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) 
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
Serial.print(F("Humidity: "));
Serial.println(h);
Serial.print(F("Temperature: "));
Serial.print(t);
Serial.println(F("°C ")); 
if (t > 30.00)
{
  digitalWrite(FAN,HIGH);
  
}
if(h >60)
{
  digitalWrite(Humidifier,HIGH);
}
ThingSpeak.writeField(myChannelField,ChannelField5,t, myWriteAPIKey);
ThingSpeak.writeField(myChannelField,ChannelField6,h, myWriteAPIKey);
//----------------------------------------//
}
