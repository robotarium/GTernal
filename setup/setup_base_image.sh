#!/bin/bash

source ./env_variables.sh

# Exit immediately if a command exits with a non-zero status
set -e

# Define a function to handle errors
error_handler() {
    echo "Error encountered during step: $current_step. Exiting script with status $?."
}

# Trap errors and call the error_handler function
trap error_handler ERR

DEL="-------"
FIN="DONE"

print_start() {
	echo "$DEL ""$1"" $DEL"
}

print_end() {
	echo "$DEL ""$FIN ""$1"" $DEL"	
}

current_step="DISABLING WIFI POWER MANAGEMENT"
print_start "current_step"
echo '#!/bin/sh' >> turn_off_wifi_power
echo '/sbin/iw dev wlan0 set power_save off' >> turn_off_wifi_power
chmod +x turn_off_wifi_power
sudo mv turn_off_wifi_power /etc/network/if-up.d/
print_end "$current_step"

current_step="DISABLING UNUSED SERVICES"
print_start "$current_step"
sudo apt-get purge -y --remove plymouth
sudo systemctl disable triggerhappy.service
sudo systemctl disable hciuart.service
sudo systemctl disable keyboard-setup.service
sudo systemctl disable dphys-swapfile.service
print_end "current_step"

current_step="DISABLING BlUETOOTH"
print_start "current_step"
sudo echo "dtoverlay=disable-bt" >> /boot/config.txt
print_end "$current_step"

current_step="ENABLING UART SERIAL"
print_start "$current_step"
sudo raspi-config nonint do_serial 2
print_end "$current_step"

# Date and time needs to be updated to properly install Docker
current_step="SYNCHRONIZING DATE AND TIME"
print_start "$current_step"
sudo date -s "$(wget -qSO- --max-redirect=0 google.com 2>&1 | grep Date: | cut -d' ' -f5-8)Z"
print_end "$current_step"

current_step="INSTALLING DOCKER"
print_start "$current_step"
for pkg in docker.io docker-doc docker-compose podman-docker containerd runc; do sudo apt-get remove -y $pkg /dev/null || true; done
sudo apt-get update > /dev/null
sudo apt-get install -y ca-certificates curl > /dev/null
sudo install -m 0755 -d /etc/apt/keyrings
sudo curl -fsSL https://download.docker.com/linux/debian/gpg -o /etc/apt/keyrings/docker.asc > /dev/null
sudo chmod a+r /etc/apt/keyrings/docker.asc
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/debian \
  $(. /etc/os-release && echo "$VERSION_CODENAME") stable" | \
  sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update > /dev/null
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin > /dev/null
sudo usermod -aG docker pi
print_end "$current_step"

if [ -n "$LOCAL_DOCKER_REGISTRY" ]; then
	current_step="ADDING LOCAL DOCKER REGISTRY"
	print_start "$current_step"
	# sudo echo '{"insecure-registries":["192.168.1.8:9000"]}' >> /etc/docker/daemon.json
	sudo bash -c "echo '{\"insecure-registries\":[\"$LOCAL_DOCKER_REGISTRY\"]}' > /etc/docker/daemon.json"
	sudo service docker restart
	print_end "$current_step"
fi

current_step="INSTALLING PYTHON AND DEPS"
print_start "$current_step"
sudo apt-get install -y python3-pip git > /dev/null
sudo pip3 install pyserial > /dev/null
sudo pip3 install netifaces > /dev/null
print_end "$current_step"

current_step="CLONING GIT REPOS"
git config --global credential.helper store
print_start "$current_step"
cd /home/pi
mkdir git
sudo chown -R pi:pi git
cd git
sudo -u pi git clone -b $FIRMWARE_REPO_BRANCH $FIRMWARE_REPO_URL
sudo -u pi git clone https://github.com/robotarium/mac_discovery
print_end "$current_step"

current_step="STARTING WATCHTOWER CONTAINER"
print_start "$current_step"
cd /home/pi/git/GTernal/docker
sudo ./docker_watch.sh
print_end "$current_step"

# current_step="STARTING MAC DISCOVERY CONTAINER"
# print_start "$current_step"
# cd /home/pi/git/mac_discovery/docker
# sudo ./docker_run.sh
# print_end "$current_step"

STR="REMOVING SETUP SCRIPTS"
print_start "$STR"
rm /home/pi/setup_base_image.sh
rm /home/pi/env_variables.sh
print_end "$STR"

print_end "SETTING UP BASE IMAGE"