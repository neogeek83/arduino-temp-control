#include <SPI.h>
#include <Ethernet.h> //NEEDS DIGITAL PINS 10,11,12,13
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 65);
IPAddress server(10, 0, 0, 24);

EthernetClient ethClient;
PubSubClient client(ethClient);

////////////////////////
//  DEBUG OPTION :    //
////////////////////////
// use DEBUG_ON for debugging (requires serial connection on startup)
#define DEBUG_ON


#ifdef DEBUG_ON
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)   Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x) 
#endif

//////////////////
// RELAY SETTINGS
//////////////////
int relay_pins[]    = {  A1, A2 };
//starting state
int relay_state[]   = { LOW, LOW }; //Default, LOW=OFF, HIGH=ON

//Relay output Mappings(name to index in array relay_pins)
#define HEAT_RELAY 0
#define FAN_RELAY 1


bool localControl = false;

void callback(char* topic, byte* payload, unsigned int length) {
DEBUG_PRINT("Message arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  char recvMsg[length+1];
  strncpy(recvMsg, (char*)payload, length);
  recvMsg[length]='\0';

  String topicMonitored="NodeMCU1/heat_cmd";
  String isOn="ON";
  if(topicMonitored.equals(topic)){
     if (isOn.equals(recvMsg)){
       DEBUG_PRINT("HEAT ON requested");
       if(!localControl) setRelay(HEAT_RELAY,true);
     } else {
       DEBUG_PRINT("HEAT OFF requested");
       if(!localControl) setRelay(HEAT_RELAY,false);
     }
  }
  topicMonitored="NodeMCU1/fan_cmd";
  if(topicMonitored.equals(topic)){
     if (isOn.equals(recvMsg)){
       DEBUG_PRINT("FAN ON requested");
       if(!localControl) setRelay(FAN_RELAY,true);
     } else {
       DEBUG_PRINT("FAN OFF requested");
       if(!localControl) setRelay(FAN_RELAY,false);
     }
  }

  //DEBUG_PRINT(recvMsg);
  //client.publish("outTopic",recvMsg,length);  
  //DEBUG_PRINTLN("");
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())  {
      DEBUG_PRINT("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      DEBUG_PRINTLN("connected");

      // ... and resubscribe
      client.subscribe("NodeMCU1/heat_cmd");
      client.subscribe("NodeMCU1/fan_cmd");
    } else {
      DEBUG_PRINT("failed, rc=");
      DEBUG_PRINT(client.state());
      DEBUG_PRINTLN(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


#include "idDHTLib.h"

int DHTpin = 2; //Digital pin for comunications
idDHTLib DHTLib(DHTpin, idDHTLib::DHT22);

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

#define arr_len( x )  ( sizeof( x ) / sizeof( *x ) )

void setup() {
  for (int i = 0; i < arr_len(relay_pins); i++){
     pinMode(relay_pins[i], OUTPUT);
     digitalWrite(relay_pins[i], relay_state[i]);
  }

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


  int result = DHTLib.acquireAndWait();
  switch (result) {
  case IDDHTLIB_OK:
    readSensors();
    controlRelays();
    break;
  case IDDHTLIB_ERROR_CHECKSUM:
    //DEBUG_PRINTLN("Error:Checksum error");
    break;
  case IDDHTLIB_ERROR_TIMEOUT: 
    //DEBUG_PRINTLN("Error:Interrupt time out error");
    break;
  case IDDHTLIB_ERROR_ACQUIRING: 
    DEBUG_PRINTLN("Error:Acquiring"); 
    break;
  case IDDHTLIB_ERROR_DELTA: 
    DEBUG_PRINTLN("Error:Delta time to small"); 
    break;
  case IDDHTLIB_ERROR_NOTSTARTED: 
    DEBUG_PRINTLN("Error:Not started"); 
    break;
  default: 
    DEBUG_PRINTLN("Error:Unknown"); 
    break;
  }

  delay(500);// DHT11 sampling rate is 1HZ, DHT22 is 2HZ.
}

void publishTempHum(){
  //int tempC = ((curTempF-32)*5)/8;
  int humidity = curHumidity;
  char temp[3], hum[3];
  dtostrf(curTempF,3,3,temp);
  dtostrf(humidity,3,3,hum);
  client.publish("NodeMCU1/temp",temp);
  client.publish("NodeMCU1/hum",hum);
  
  DEBUG_PRINT("NodeMCU1/temp:");
  DEBUG_PRINTLN(temp);
  DEBUG_PRINT("NodeMCU1/hum:");
  DEBUG_PRINTLN(hum);
}

void readSensors(){
  curTempF = (int)DHTLib.getFahrenheit();
  curHumidity = (int)DHTLib.getHumidity();
}

void controlRelays(){
  if(localControl){
  switch (opMode){
    case HEAT:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setTemp  <  (curTempF - thresholdTempMargin))){
          setRelay(HEAT_RELAY, false);

        //ON
        } else if ( (setTemp > curTempF) || overrideOn){
          setRelay(HEAT_RELAY, true);
        }
        
      } else {
        setRelay(HEAT_RELAY, setTemp > curTempF || overrideOn);
      }
      break;
    case AC:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setTemp  >  (curTempF + thresholdTempMargin))){
          setRelay(0, false);

        //ON
        } else if ( (setTemp < curTempF) || overrideOn){
          setRelay(0, true);
        }
        
      } else {
        setRelay(0, setTemp < curTempF || overrideOn);
      }
      break;
    case HUMIDIFIER:
      if (thresholdEnabled){
        
        //OFF
        if ( !overrideOn && (setHumidity  <  (curHumidity - thresholdTempMargin))){
          setRelay(0, false);

        //ON
        } else if ( (setHumidity > curHumidity) || overrideOn){
          setRelay(0, true);
        }
        
      } else {
          setRelay(0, setHumidity > curHumidity || overrideOn);
        }
        break;
      case DEHUMIDIFIER:
        if (thresholdEnabled){
        
          //OFF
          if ( !overrideOn && (setHumidity  >  (curHumidity + thresholdTempMargin))){
            setRelay(0, false);

          //ON
          } else if ( (setHumidity < curHumidity) || overrideOn){
            setRelay(0, true);
          }
          
        } else {
          setRelay(0, setHumidity < curHumidity || overrideOn);
        }
        break;
    }
  }
}

void setRelay(int i, boolean on){
  if (on){
     relay_state[i] = HIGH;
     digitalWrite(relay_pins[i], relay_state[i]);
     if (i == HEAT_RELAY) {
       client.publish("NodeMCU1/heat","ON");
     } else if (i == FAN_RELAY) {
       client.publish("NodeMCU1/fan","ON");
     }
  } else {
     relay_state[i] = LOW;
     digitalWrite(relay_pins[i], relay_state[i]);
     if (i == HEAT_RELAY) {
       client.publish("NodeMCU1/heat","OFF");
     } else if (i == FAN_RELAY) {
       client.publish("NodeMCU1/fan","OFF");
     }
  }
   //DEBUG_PRINT("RELAY PIN TRIGGERED: ");
   //DEBUG_PRINT(relay_pins[i]);
   //DEBUG_PRINT(" turned ");
   //DEBUG_PRINT(relay_state[i]);
   DEBUG_PRINT("RELAY_");DEBUG_PRINT(i);DEBUG_PRINT(": ");printRelayState(FAN_RELAY);
}

void printRelayState(int i){
   if (relay_state[i]){
     DEBUG_PRINTLN("ON ");
   } else {
     DEBUG_PRINTLN("OFF");
   }
}

