/*
  MQTT DHT22, when "temperature, c" is sent it returns the temperature in celcius
  when "humidity" is sent it returns the humidity as measured by the DHT22
  the signal pin is connected to a pull-ip resistor and GPIO 2
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN            2         // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
#define DHTTYPE           DHT11     // DHT 11 
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// Update these with values suitable for your network.

const char* ssid = "<<PUT_WIFI_NETWORK_NAME_HERE>>";
const char* password = "<<PUT_PASSWORD_HERE>>";
const char* mqtt_server = "10.0.0.35";
const char* clientID = "NodeMCUDevKit";
const char* outTopicTemp = "NodeMCUout/temp";
const char* outTopicHum = "NodeMCUout/hum";
const char* outTopicStatus = "NodeMCUout/hp_status";
const char* inTopic = "NodeMCUin";

// Initialize DHT sensor 
DHT_Unified dht(DHTPIN, DHTTYPE);


float humidity, temp_c;  // Values read from sensor
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 5000;              // interval at which to read sensor
 
WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {
  // Conver the incoming byte array to a string
  payload[length] = '\0'; // Null terminator used to terminate the char array
  String message = (char*)payload;

  Serial.print("Message arrived on topic: [");
  Serial.print(topic);
  Serial.print("], ");
  Serial.println(message);

  if(message == "temperature, c"){
    gettemperature();
    Serial.print("Sending temperature:");
    Serial.println(temp_c);
    dtostrf(temp_c , 2, 2, msg);
    client.publish(outTopicTemp, msg);
  } else if (message == "humidity"){
    gettemperature();
    Serial.print("Sending humidity:");
    Serial.println(humidity);
    dtostrf(humidity , 2, 2, msg);
    client.publish(outTopicHum, msg);
  } else if (message == "ON"){
    client.publish(outTopicStatus, "ON");
    Serial.println("Commanded ON");
  } else if (message == "OFF"){
    client.publish(outTopicStatus, "OFF");
    Serial.println("Commanded OFF");
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(outTopic, clientID);
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.begin();           // initialize temperature sensor
  dump_sensor_info();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - previousMillis >= interval) {
  
    gettemperature();
    Serial.print("Current temperature:");
    Serial.println(temp_c);
    dtostrf(temp_c , 2, 2, msg);
    client.publish(outTopicTemp, msg);
    
    Serial.print("Current humidity:");
    Serial.println(humidity);
    dtostrf(humidity , 2, 2, msg);
    client.publish(outTopicHum, msg);
  }
    
}
void dump_sensor_info(){
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Temperature");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" *C");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" *C");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" *C");  
  Serial.println("------------------------------------");
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.println("Humidity");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println("%");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println("%");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println("%");  
  Serial.println("------------------------------------");
}
void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   

    sensors_event_t event;  
    dht.temperature().getEvent(&event);
    temp_c = event.temperature;
    if (isnan(temp_c)) {
      Serial.println("Error reading temperature!");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temp_c);
      Serial.println(" *C");
    }
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    humidity = event.relative_humidity;
    if (isnan(humidity)) {
      Serial.println("Error reading humidity!");
    } else {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println("%");
    }
  }
  
}
