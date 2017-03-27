# arduino-temp-control
Uses an Arduino along with a LCD, remote and Temp/Humidity Sensor to control a themostat


<img src="https://github.com/neogeek83/arduino-temp-control/blob/master/TempReadOut2/20161226_224157.png?raw=true" />

#Sketches
 - IR_Receiver_Module -- useful to map the button codes to what they actually are.
 - TempReadOut -- intial attempt
 - TempReadOut2 -- Has the better error handling and is ready for addition of relays to control a themostat.
 - ethernet_mqtt -- Does not have LCD or any sensor support, but joins a 10.x.x.x network via an eithernet shield and communicates with an MQTT server subscribing to a channel and echoing any received messages out to another channel.
 - wifi_esp8266_dht_mqtt -- Reads tempatures successfully after joining a wifi network and publishes them to topics. Ran into reliability issues with the ESP8266 chips so didn't finish. If you use this, make sure you update the ssid and password fields for your network settings.