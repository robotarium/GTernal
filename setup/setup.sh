#!/bin/bash

sudo ./setup_base_image.sh
sudo ./setup_from_base.sh

# DEL="-------"
# FIN="DONE"

# print_start() {
# 	echo "$DEL ""$1"" $DEL"
# }

# print_end() {
# 	echo "$DEL ""$FIN ""$1"" $DEL"	
# }

# STR="DISABLING WIFI POWER MANAGEMENT"
# print_start "$STR"
# echo '#!/bin/sh' >> turn_off_wifi_power
# echo '/sbin/iw dev wlan0 set power_save off' >> turn_off_wifi_power
# chmod +x turn_off_wifi_power
# sudo mv turn_off_wifi_power /etc/network/if-up.d/
# print_end "$STR"

# STR="DISABLING UNUSED SERVICES"
# print_start "$STR"
# sudo apt-get purge -y --remove plymouth
# sudo systemctl disable triggerhappy.service
# sudo systemctl disable hciuart.service
# sudo systemctl disable keyboard-setup.service
# sudo systemctl disable dphys-swapfile.service
# print_end "$STR"

# STR="DISABLING BlUETOOTH"
# print_start "$STR"
# sudo echo "dtoverlay=disable-bt" >> /boot/config.txt
# print_end "$STR"

# STR="ENABLING UART SERIAL"
# print_start "$STR"
# sudo raspi-config nonint do_serial 2
# print_end "$STR"

# # Date and time needs to be updated to properly install Docker
# STR="SYNCHRONIZING DATE AND TIME"
# print_start "$STR"
# sudo date -s "$(wget -qSO- --max-redirect=0 google.com 2>&1 | grep Date: | cut -d' ' -f5-8)Z"
# print_end "$STR"

# STR="INSTALLING DOCKER"
# print_start "$STR"
# sudo apt-get remove docker docker-engine docker.io
# curl -fsSL get.docker.com -o get-docker.sh && export VERSION=23.0 && sh get-docker.sh
# sudo usermod -aG docker pi
# print_end "$STR"

# STR="INSTALLING PYTHON AND DEPS"
# print_start "$STR"
# sudo apt-get install -y python3-pip git
# sudo python3 -m pip install pyserial
# sudo python3 -m pip install netifaces
# print_end "$STR"

# STR="CLONING GIT REPOS"
# git config --global credential.helper store
# print_start "$STR"
# cd /home/pi
# mkdir git
# sudo chown -R pi:pi git
# cd git
# git clone https://github.com/robotarium/GTernal
# git clone https://github.com/robotarium/mac_discovery
# print_end "$STR"

# STR="STARTING CONTAINERS"
# print_start "$STR"
# cd /home/pi/git/GTernal/docker
# sudo ./docker_run.sh
# sudo ./docker_watch.sh
# cd /home/pi/git/mac_discovery/docker
# sudo ./docker_run.sh
# print_end "$STR"

# STR="SETTING UP NETWORK HOST NAME"
# print_start "$STR"
# cd /home/pi/git/GTernal/setup
# robot_id=$(python3 check_hostname.py ../config/mac_list.json)
# sudo raspi-config nonint do_hostname $robot_id
# print_end "$STR"

# STR="REMOVING SETUP SCRIPT"
# print_start "$STR"
# rm /home/pi/setup
# rm /home/pi/get-docker.sh
# print_end "$STR"

# STR="REBOOTING TO APPLY SETTINGS"
# print_start "$STR"
# sudo reboot