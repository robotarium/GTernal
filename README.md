# Setup Process for GTernal

# 1 - Program Teensy
1. Install Teensyduino (https://www.pjrc.com/teensy/td_download.html)
2. Copy and paste all library folders in 'GTernal/firmware/Teensy/libraries' to 'Arduino/libraries'
    > [!NOTE]
    > The absolute path for the 'Arduino/libraries' directory is OS dependent, and this can be found here: https://support.arduino.cc/hc/en-us/articles/4412950938514-Open-the-Sketchbook
    
    > [!NOTE]
    > The firmware for Teensy requires ArduinoJson 5.13 which is included in 'GTernal/firmware/Teensy/libraries'
    > ArduinoJson 6 is currently not compatible with the firmware.
3. Connect a Teensy to the computer using a micro-USB cable.
4. Run Teensyduino and open 'defaultOperation.ino' located in 'GTernal/firmware/defaultOperation' directory.
5. Click the upload icon (right arrow icon).

# 2 - Make the Base Image for Raspberry Pi

This section details how to make the base image.  Relatively few changes are made to keep the image small.  Once the changes in this section have been made, copy the new image to an SD card and use that as the base image.

## 1 - Load the RPi image onto an SD card
1. Install and run the Raspberry Pi Imager (https://www.raspberrypi.com/software/).
2. For 'Operating System,' select 'Raspberry Pi OS (other)' and select 'Raspberry Pi OS Lite (64-bit).' For Raspberry Pi Zero 1s, select 'Raspberry Pi OS Lite (32-bit)'
    > [!IMPORTANT]  
    > The newest version of Raspberry Pi OS released in 10/10/2023 is not fully compatible with the current setup script. Please use 'Raspberry Pi OS (Legacy) Lite' released in 5/3/2023 for now.
3. For 'Storage,' choose the SD card to be used.
4. Before clicking the 'WRITE' button, click on the gear icon below the 'WRTIE' button to open the 'Advanced options.'
5. Check 'Enable SSH' and 'Use password authentication.'
6. Then, check 'Set username and password' and type 'pi' for the 'Username' and 'raspberry' for 'Password.'
7. Next, check 'Configure wireless LAN' and enter 'SSID' and 'Password' for the router.
8. Change 'Wireless LAN country' to 'US.' Click 'SAVE' and click 'WRITE' button to start loading the image to the SD card.

## 2 - Setup the RPi
1. Eject the card from the computer and insert it onto a Raspberry Pi and power it up.
It should automatically connect to the wifi. The Pi needs some time to boot for the first time. The boot up process can be visually inspected by plugging the Raspberry Pi to a monitor through a mini HDMI cable. When the Pi completes the booting process it will prompt a login. If the Pi shows a blue screen prompting to enter a new username, something is wrong with uploading the image to Pi, and the Raspbian OS needs to be reinstalled. Before loading another image to the SD card, make sure to re-type the passwords for the Pi and the Wifi. The Raspberry Pi Imager seems to be ruining the passwords saved in the advanced setting when the program is restarted.
2. Navigate to the router settings page by navigating to '192.168.1.1' or '10.0.0.1' using a web browser. The new Pi will appear as 'RASPBERRYPI.' 
3. Click on it to look up its IP address and MAC address. 
4. After looking up the IP address of the new Pi, ssh to it by
```
ssh pi@<IP-address-of-Pi>
```
When promted to enter password, type 'raspberry'

5. Open /boot/config.txt file by

```
sudo nano /boot/config.txt
```

6. Add to /boot/config.txt the text

```
# Disable bluetooth
dtoverlay=pi3-disable-bt
```

## 3 - Register the RPi as a Robot
Assign an unallocated robot index for the MAC address of the Raspberry Pi. Then, on the main computer,
1. Add/Replace the MAC address of the Raspberry Pi in 'GTernal/config/mac_list.json'
2. Add/Replace the robot ID in 'vicon_tracker_python/config/node_desc_tracker.json'
3. Add/Replace the robot ID in '~/git/robotarium_matlab_backend/config/node_desc_api.json'
4. Build the firmware Docker image by running
```
cd /path-to-the-parent-directory/GTernal/docker/
./docker_build.sh 192.168.1.8 1884
```
- When making making multiple robots, register the MAC address of all new robots in the files listed above before building the firmware image. Otherwise, the firmware needs to be built as many as the number of new robots.
- When assigning an index to a new robot, assign the number engraved on the Vicon hat plate. Make sure not to use any numbers that are already assigned to other robots. For more information about generating Vicon hats, see https://github.com/skim743/gritsbotx_vicon_hats

# 3 - Automated Setup

This section assumes that you have built a base image as previously detailed.

## 1 - Automatic Installation

1. Connect the Pi and Teensy with through UART. If the Pi is not connected with a programmed Teensy as instructed in Step 1, the firmware will not be started by the setup script successfully in the next step.

2. On the Pi, run the setup script with

```
sudo ./setup
```

This can be done either by directly on the Pi by connecting a mini HDMI cable and a keyboard to the Pi or SSHing to the Pi as instructed in Step 2.4.

## 2 - Network Host Name Change
This process changes the name of the Raspberry Pi on the network. This helps to identify each robot easily on the router page (192.168.1.1). See Step 2.2, if you forgot how to access the router page.

After the setup script is completed,
Run
```
sudo raspi-config
```
on the Raspberry Pi, and change the Host Name to 'robot#' where # is the new robot index assigned to the Pi in Step 3.

Reboot the Pi to apply the new network host name by selecting 'yes' when the raspi-config asks for a restart, or by using
```
sudo reboot
```

In case of using Raspberry Pi 3 and 4, serial settings needs to be modified to enable UART communication. For this,
Run
```
sudo raspi-config
```
on the Pi, and go to 'Interface Options' - 'Interface Options' - 'Serial Port'
Select 'No' for 'Would you like a login shell to be accessible over serial?'
and 'Yes' for 'Would you like the serial port hardware to be enabled?'

# 4 - Manual Installation

This section details the manual installation process for the firmware. This process is unnecessary if the automatic setup in the previous section is performed.

### 1 Remove Unused Services

Remove plymouth with 

```
sudo apt-get purge --remove plymouth
```

Disable unused services with 

```
sudo systemctl disable triggerhappy.service
sudo systemctl disable hciuart.service
sudo systemctl disable keyboard-setup.service
sudo systemctl disable dphys-swapfile.service
```

### 2 - Install Docker

This section follows from the official (Docker)[https://docs.docker.com/install/linux/docker-ce/ubuntu/].  First, remove old versions of Docker.

```
sudo apt-get remove docker docker-engine docker.io
```

Next, install Docker using the convenience script.

```
curl -fsSL get.docker.com -o get-docker.sh && export VERSION=23.0 && sh get-docker.sh
```

Now tie Docker to the pi user so that we don't need sudo to use Docker.

```
sudo usermod -aG docker pi
```

### 3 - Clone Git Repos and install Deps

Install pip for python3

```
sudo apt-get install python3-pip
```

To clone the firmware, run
```
sudo apt-get install git
git clone https://github.com/robotarium/gritsbot_2
```

Also, install the MAC discovery repository
```
git clone https://github.com/robotarium/mac_discovery
```

as well as the python serial library used to communicate to the robot.

```
python3 -m pip install pyserial
```

### 4 - WiFi Power Management

Turn off power management by adding the line
```
/sbin/iw dev wlan0 set power_save off
```

in the file /etc/rc.local.  This line disables WiFi power management on boot.

### 5 - Start necessary containers

From wherever the git repository is cloned, run 
```
cd <path_to_gritsbot_2_repo>/docker
./docker_run.sh
./docker_watch.sh
```
which will permanently start a Docker container running the firmware and the watchtower container.  Watchtower watches containers and automatically updates them from Dockerhub.  This watchtower instance
checks and updates **all running containers**, so this instance will also update the MAC container as well.

Start MAC discovery as well with
```
cd <path_to_mac_discovery_repo>/docker
./docker_run.sh
```