# Initial Setup

apt-get update
apt-get upgrade -y
apt-get install -y vim git

# Install Mosquitto
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
apt-key add mosquitto-repo.gpg.key
rm mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
wget http://repo.mosquitto.org/debian/mosquitto-jessie.list
apt-get update
apt-get install -y mosquitto mosquitto-clients

# Install HomeAssistant
apt-get install -y python3 python3-venv python3-pip
useradd -rm homeassistant
cd /srv
mkdir homeassistant
chown homeassistant:homeassistant homeassistant
su -c ./pip.sh homeassistant
