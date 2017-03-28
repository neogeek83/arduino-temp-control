#include <dht.h>

#define dht_dpin A0 // no ; here. This is the PIN that the DHT11 is on.
#define relay_pin 2
int relayState = LOW;
dht DHT;
/*   https://github.com/knolleary/pubsubclient/  */


const int numReadings = 10;

int tempReadings[numReadings];      // the readings from the analog input
int tempReadIndex = 0;              // the index of the current reading
int tempTotal = 0;                  // the running total
int tempAverage = 0;                // the average
int tempReads = 1;                // number of readings since start. No greater than numReadings.
int humReadings[numReadings];      // the readings from the analog input
int humReadIndex = 0;              // the index of the current reading
int humTotal = 0;                  // the running total
int humAverage = 0;                // the average
int humReads = 1;                // number of readings since start. No greater than numReadings.

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(10, 0, 0, 65);
IPAddress server(10, 0, 0, 24);

EthernetClient ethClient;
PubSubClient client(ethClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char temp[length+1];
  strncpy(temp, (char*)payload, length);
  temp[length]='\0';

  String topicMonitored="NodeMCUin";
  String isOn="ON";
  if(topicMonitored.equals(topic)){
     if (isOn.equals(temp)){
      relayState = HIGH;
      Serial.print("Confirmed on");
      client.publish("NodeMCUout/hp_status","ON");
     }
     else
     {
      relayState = LOW;
      Serial.print("Confirmed off");
      client.publish("NodeMCUout/hp_status","OFF");
      }
      digitalWrite(relay_pin, relayState);
  }

  Serial.print(temp);
  client.publish("outTopic",temp,length);  
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected())  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");

      // ... and resubscribe
      client.subscribe("inTopic");
      client.subscribe("NodeMCUin");
      client.subscribe("inTopic3");
      client.subscribe("inTopic4");
      client.subscribe("inTopic5");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{

  pinMode(relay_pin,OUTPUT);
  digitalWrite(relay_pin, relayState);
  
  Serial.begin(57600);
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    tempReadings[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    humReadings[thisReading] = 0;
  }
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  publishTempHum();
  delay(2000);
}

void publishTempHum(){
  DHT.read11(dht_dpin);

  // subtract the last reading:
  tempTotal = tempTotal - tempReadings[tempReadIndex];
  // read from the sensor:
  tempReadings[tempReadIndex] = DHT.temperature;
  // add the reading to the total:
  tempTotal = tempTotal + tempReadings[tempReadIndex];
  // advance to the next position in the array:
  tempReadIndex = tempReadIndex + 1;

  // if we're at the end of the array...
  if (tempReadIndex >= numReadings) {
    // ...wrap around to the beginning:
    tempReadIndex = 0;
  }

  // calculate the average (but only the values that have been read):
  tempAverage = tempTotal / tempReads;
  if (tempReads<numReadings){
    tempReads++;
  }     

  // subtract the last reading:
  humTotal = humTotal - humReadings[humReadIndex];
  // read from the sensor:
  humReadings[humReadIndex] = DHT.humidity;
  // add the reading to the total:
  humTotal = humTotal + humReadings[humReadIndex];
  // advance to the next position in the array:
  humReadIndex = humReadIndex + 1;

  // if we're at the end of the array...
  if (humReadIndex >= numReadings) {
    // ...wrap around to the beginning:
    humReadIndex = 0;
  }

  // calculate the average (but only the values that have been read):
  humAverage = humTotal / humReads;
  if (humReads<numReadings){
    humReads++;
  }     

  
  Serial.print(tempAverage); 
  
  char temp[3], hum[3];
  dtostrf(tempAverage,3,3,temp);
  dtostrf(humAverage,3,3,hum);
  client.publish("NodeMCUout/temp",temp);
  client.publish("NodeMCUout/hum",hum);
}


