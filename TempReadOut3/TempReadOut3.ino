/*
 The LiquidCrystal library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface. -- Wire up using the below
   Better wiring diiagram(just use data pins defined below): http://www.instructables.com/id/How-to-use-an-LCD-displays-Arduino-Tutorial/step3/The-Circuit/

  The circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 8
 * LCD D4 pin to digital pin 9
 * LCD D5 pin to digital pin 10
 * LCD D6 pin to digital pin 11
 * LCD D7 pin to digital pin 12
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K potentiometer:
 * Ends to +5V and ground
 * LCD VO pin (pin 3) to potentiometer center lead
 * 
 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 65);
IPAddress server(10, 0, 0, 24);

EthernetClient ethClient;
PubSubClient client(ethClient);

#define DEBUG_OFF


#ifdef DEBUG_ON
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)   Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x) 
#endif

bool localControl = false;

void callback(char* topic, byte* payload, unsigned int length) {
DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  char recvMsg[length+1];
  strncpy(recvMsg, (char*)payload, length);
  recvMsg[length]='\0';

  String topicMonitored="NodeMCUin";
  String isOn="ON";
  if(topicMonitored.equals(topic)){
     if (isOn.equals(recvMsg)){
       DEBUG_PRINT("Confirmed on");
       client.publish("NodeMCUout/hp_status","ON");
       if(!localControl) setRelay(true);
     } else {
       DEBUG_PRINT("Confirmed off");
       client.publish("NodeMCUout/hp_status","OFF");
       if(!localControl) setRelay(false);
     }
  }

  DEBUG_PRINT(recvMsg);
  client.publish("outTopic",recvMsg,length);  
  DEBUG_PRINTLN("");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())  {
      DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      DEBUG_PRINTLN("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");

      // ... and resubscribe
      client.subscribe("inTopic");
      client.subscribe("NodeMCUin");
      client.subscribe("inTopic3");
      client.subscribe("inTopic4");
      client.subscribe("inTopic5");
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 5, 6, A0);

#include "idDHT11.h"

int idDHT11pin = 2; //Digital pin for comunications
int idDHT11intNumber = 0; //interrupt number (must be the one that use the previus defined pin (see table above)

//declaration
void dht11_wrapper(); // must be declared before the lib initialization

// Lib instantiate
idDHT11 DHT11(idDHT11pin,idDHT11intNumber,dht11_wrapper);

// This wrapper is in charge of calling 
// mus be defined like this for the lib work
void dht11_wrapper() {
  DHT11.isrCallback();
}

#include "IRremote.h"
int receiver = 3; // Signal Pin of IR receiver to Arduino Digital Pin 3
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'

  enum mode{AC=0,HEAT=1,HUMIDIFIER=2,DEHUMIDIFIER=3};

/**
 * CONFIG:
 */


int setTemp = 76;
int setHumidity = 50;

int curTempF = 0;
int curHumidity = 0;

int thresholdTempMargin = 3;
int thresholdHumidityMargin = 3;
bool thresholdEnabled = true;
mode opMode = HEAT;
bool overrideOn = false;


int relay_pin = 4;
int relay_state = LOW; //LOW=OFF, HIGH=ON

void setup() {
  pinMode(relay_pin, OUTPUT);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  irrecv.enableIRIn(); // Start the receiver

  #ifdef DEBUG_ON
    Serial.begin(57600);
  #endif
  
  client.setServer(server, 1883);
  client.setCallback(callback);
  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  publishTempHum();

  
  lcd.setCursor(0, 0); // setCursor(col:0-16,row:0-1)

  int result = DHT11.acquireAndWait();
  switch (result) {
  case IDDHTLIB_OK: 
    //lcd.println("OK"); 
    if (irrecv.decode(&results)){ // have we received an IR signal?
      String IRVal = translateIR();
      
      if ( IRVal == ">>|" ) setHumidity++;
      if ( IRVal == "|<<" ) setHumidity--;
      
      if ( IRVal == "VOL+" ) setTemp++;
      if ( IRVal == "VOL-" ) setTemp--;

      if ( IRVal == "0" ) overrideOn=false;
      if ( IRVal == "1" ) overrideOn=true;
      
      if ( IRVal == "6" ) thresholdEnabled=false;
      if ( IRVal == "9" ) thresholdEnabled=true;
      
      if ( IRVal == "FUNC/STOP" ) localControl=!localControl;
      
      irrecv.resume(); // receive the next value
    }
    readSensors();
    writeToLCD();
    break;
  case IDDHTLIB_ERROR_CHECKSUM: 
    //lcd.println("Error:Checksum error"); 
    //DEBUG_PRINTLN("Error:Checksum error");
    break;
  case IDDHTLIB_ERROR_ISR_TIMEOUT: 
    lcd.println("Error:ISR time out error"); 
    DEBUG_PRINTLN("Error:ISR time out error");
    break;
  case IDDHTLIB_ERROR_RESPONSE_TIMEOUT: 
    //lcd.println("Error:Response time out error"); 
    //DEBUG_PRINTLN("Error:Response time out error");
    break;
  case IDDHTLIB_ERROR_DATA_TIMEOUT: 
    lcd.println("Error:Data time out error"); 
    DEBUG_PRINTLN("Error:Data time out error"); 
    break;
  case IDDHTLIB_ERROR_ACQUIRING: 
    lcd.println("Error:Acquiring"); 
    DEBUG_PRINTLN("Error:Acquiring"); 
    break;
  case IDDHTLIB_ERROR_DELTA: 
    lcd.println("Error:Delta time to small");
    DEBUG_PRINTLN("Error:Delta time to small"); 
    break;
  case IDDHTLIB_ERROR_NOTSTARTED: 
    lcd.println("Error:Not started"); 
    DEBUG_PRINTLN("Error:Not started"); 
    break;
  default: 
    lcd.println("Error:Unknown"); 
    DEBUG_PRINTLN("Error:Unknown"); 
    break;
  }

  delay(1000);// DHT11 sampling rate is 1HZ.
}

void publishTempHum(){
  //int tempC = ((curTempF-32)*5)/8;
  int humidity = curHumidity;
  char temp[3], hum[3];
  dtostrf(curTempF,3,3,temp);
  dtostrf(humidity,3,3,hum);
  client.publish("NodeMCUout/temp",temp);
  client.publish("NodeMCUout/hum",hum);
  
  //DEBUG_PRINT("NodeMCUout/temp:");
  //DEBUG_PRINTLN(temp);
  //DEBUG_PRINT("NodeMCUout/hummp:");
  //DEBUG_PRINTLN(hum);
}

void readSensors(){
  curTempF = (int)DHT11.getFahrenheit();
  curHumidity = (int)DHT11.getHumidity();
}

void writeToLCD(){
  int tempF = curTempF;
  int humidity = curHumidity;
  lcd.print("Temp:");
  DEBUG_PRINT("Temp:");
  lcd.print(tempF); lcd.print("F|");
  DEBUG_PRINT(tempF); DEBUG_PRINT("F|");
  

  if (opMode == HEAT || opMode == AC) {
    lcd.print(setTemp);
    lcd.print("F");
    DEBUG_PRINT(setTemp);
    DEBUG_PRINTLN("F");
  } else {
    lcd.print(setHumidity);
    lcd.print("%");
    DEBUG_PRINT(setHumidity);
    DEBUG_PRINTLN("%");
  }
  lcd.setCursor(15, 0);
  if(localControl){
    lcd.print("L");
  } else {
    lcd.print("R");
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Humidity:");
  lcd.print(humidity); lcd.print("%|");
  
  DEBUG_PRINT("Humidity:");
  DEBUG_PRINT(humidity); DEBUG_PRINT("%|");


  printRelayState();
}

void controlRelay(){
  if(localControl){
  switch (opMode){
    case HEAT:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setTemp  <  (curTempF - thresholdTempMargin))){
          setRelay(false);

        //ON
        } else if ( (setTemp > curTempF) || overrideOn){
          setRelay(true);
        }
        
      } else {
        setRelay(setTemp > curTempF || overrideOn);
      }
      break;
    case AC:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setTemp  >  (curTempF + thresholdTempMargin))){
          setRelay(false);

        //ON
        } else if ( (setTemp < curTempF) || overrideOn){
          setRelay(true);
        }
        
      } else {
        setRelay(setTemp < curTempF || overrideOn);
      }
      break;
    case HUMIDIFIER:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setHumidity  <  (curHumidity - thresholdTempMargin))){
          setRelay(false);

        //ON
        } else if ( (setHumidity > curHumidity) || overrideOn){
          setRelay(true);
        }
        
      } else {
          setRelay(setHumidity > curHumidity || overrideOn);
        }
        break;
      case DEHUMIDIFIER:
        if (thresholdEnabled){
        
          //OFF
          if ( !overrideOn && (setHumidity  >  (curHumidity + thresholdTempMargin))){
            setRelay(false);

          //ON
          } else if ( (setHumidity < curHumidity) || overrideOn){
            setRelay(true);
          }
          
        } else {
          setRelay(setHumidity < curHumidity || overrideOn);
        }
        break;
    }
  }
}

void setRelay(boolean on){
  if (on){
     relay_state = HIGH;
     digitalWrite(relay_pin, relay_state);
  } else {
     relay_state = LOW;
     digitalWrite(relay_pin, relay_state);
  }
}

void printRelayState(){
   if (relay_state){
     lcd.print("ON ");
     DEBUG_PRINTLN("ON ");
   } else {
     lcd.print("OFF");
     DEBUG_PRINTLN("OFF");
   }
}

String translateIR(){ // takes action based on IR code received

// describing Remote IR codes 
  switch(results.value) {
    //row 1
    case 0xFFA25D: return "PWR";
    case 0xFF629D: return "VOL+";
    case 0xFFE21D: return "FUNC/STOP";
  
    //row 2
    case 0xFF22DD: return "|<<";
    case 0xFF02FD: return "PLAY/PAUSE";
    case 0xFFC23D: return ">>|";
  
    //row 3
    case 0xFFE01F: return "DOWN";
    case 0xFFA857: return "VOL-";
    case 0xFF906F: return "UP";
  
    //row 4
    case 0xFF6897: return "0";
    case 0xFF9867: return "EQ";
    case 0xFFB04F: return "ST/REPT";
  
    //row 5
    case 0xFF30CF: return "1";
    case 0xFF18E7: return "2";
    case 0xFF7A85: return "3";
  
    //row 6
    case 0xFF10EF: return "4";
    case 0xFF38C7: return "5";
    case 0xFF5AA5: return "6";
  
    //row 7
    case 0xFF42BD: return "7";
    case 0xFF4AB5: return "8";
    case 0xFF52AD: return "9";
  
    //holding a button down results in a repeat code
    case 0xFFFFFFFF: return "REPEAT";

    default: 
      String retValue = "0x";
      retValue = retValue + String(results.value, HEX);
      return retValue;
  }// End Case
} //END translateIR
