# arduino-temp-control
Uses an Arduino along with a LCD, remote and Temp/Humidity Sensor to control a themostat


<img src="https://github.com/neogeek83/arduino-temp-control/blob/master/TempReadOut2/20161226_224157.png?raw=true" />

# Sketches
 - IR_Receiver_Module -- useful to map the button codes to what they actually are.
 - TempReadOut -- intial attempt
 - TempReadOut2 -- Has the better error handling and controls a relay connected to `relay_pin` (digital 4 by default). Fully functional heater termostat(`ON` if temp falls below threshold), as well as humidifier controller(`ON` if humidity falls below a set threshold).  
   - `0` button disables override ON
   - `1` button enables override ON
   - `6` button disables the threshold feature for local control
   - `9` button enables the threshold feature for local control
   - `FUNC/STOP` toggles local/remote control of the relay (L or R is displayed on the top right of the LCD)
   - `>>|` button will increase the humidity on threshold
   - `|<<` button will decrease the humidity on threshold
   - `VOL+` button will increase tempeture on threshold
   - `VOL-` button will decrease tempeture on threshold 
 - `ethernet_mqtt` -- Does not have LCD or any sensor support, but joins a `10.x.x.x` network via an eithernet shield and communicates with an MQTT server subscribing to a channel and echoing any received messages out to another channel.
 - `wifi_esp8266_dht_mqtt` -- Reads tempatures successfully after joining a wifi network and publishes them to topics. Ran into reliability issues with the ESP8266 chips so didn't finish. If you use this, make sure you update the ssid and password fields for your network settings.
 - TempReadOut3 -- merges `TempReadOut2` and `ethernet_mqtt` so supports reporting temp back to a PI or other MQTT device. See readme in it's folder for specifics.
 - TempReadOut4 -- takes `TempReadOut3` and upgrades temp sensor to library that supports DHT11 and DHT22 (currently set to use DHT22), also fixed a but in Local/Remote control
 
# Circuits:
 - For the relay, you'll need to have a NPN transistor to prevent pulling too much current from the arduino. R1=2kohm and R2=10kohm are the values used from Figure 7-2 in http://www.w9xt.com/page_microdesign_pt7_transistor_switching.html using this transitor https://www.onsemi.com/pub/Collateral/PN2222-D.PDF
