Start the broker (running by default):

mosquitto
hass

Start the command line subscriber:

	mosquitto_sub -v -t 'NodeMCUout/temp'
	mosquitto_sub -v -t 'NodeMCUout/hum'
	mosquitto_sub -v -t 'NodeMCUin'

Publish test message with the command line publisher:

	mosquitto_pub -t 'NodeMCUout/hum' -m '31'
	mosquitto_pub -t 'NodeMCUout/temp' -m '35'