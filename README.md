# Setup Process for GTernal

# 1 - Repository Setup
During the setup process, each robot runs an automatic setup script which clones the firmware from a GitHub repository. By default, the setup script clones the official GTernal repository. This needs to be changed to clone your repository with updated configuration files. If you are not familiar with Git or GitHub, please refer to here: https://docs.github.com/en/get-started/quickstart/fork-a-repo

1. Fork this GTernal repository.
2. Clone the forked repository to your computer.
3. Update 'GTernal/config/env_variables.sh'
4. Push the change to your repository.


# 2 - Program Teensy
1. Install Teensyduino (https://www.pjrc.com/teensy/td_download.html)
2. Copy and paste all library folders in `GTernal/firmware/Teensy/libraries` to `Arduino/libraries`
   > [!NOTE]
   > The absolute path for the `Arduino/libraries` directory is OS dependent, and this can be found here: https://support.arduino.cc/hc/en-us/articles/4412950938514-Open-the-Sketchbook
    
   > [!NOTE]
   > The firmware for Teensy requires ArduinoJson 5.13 which is included in `GTernal/firmware/Teensy/libraries.`
   > ArduinoJson 6 is currently not compatible with the firmware.
3. Connect a Teensy to the computer using a micro-USB cable.
   > [!NOTE]
   > The micro-USB cable needs to be directly connected to the Teensy and not the micro-USB port on the robot PCB.
4. Run Teensyduino and open `defaultOperation.ino` located in `GTernal/firmware/defaultOperation` directory.
5. Select the Teensy board to be flashed under `Tools > Port`
6. Click the upload icon (right arrow icon).

# 3 - Make the Base Image for Raspberry Pi
## 1 - Install Raspberry Pi OS on an SD Card
1. Install and run the Raspberry Pi Imager (https://www.raspberrypi.com/software/). The instuction is for Raspberry Pi Imager v.1.8.5
2. For 'Raspberry Pi Device,' select the version of the Pi used.
3. For 'Operating System,' select `Raspberry Pi OS (other)` and select `Raspberry Pi OS (Legacy, 64-bit) Lite.` For Raspberry Pi Zero 1s, select `Raspberry Pi OS (Legacy, 32-bit) Lite`
4. For 'Storage,' choose the micro-SD card to be used.
5. Click 'NEXT' and 'EDIT SETTINGS.'
6. Under 'GENERAL' tab, check 'Set username and password' and type `pi` for the 'Username' and `raspberry` for 'Password.'
7. Next, check 'Configure wireless LAN' and enter 'SSID' and 'Password' for the router.
8. Change 'Wireless LAN country' to 'US.' 'Set locale settings' is optional.
9. Under 'SERVICES' tab, check 'Enable SSH' and 'Use password authentication'
10. Click 'SAVE' and click 'YES.'
    > [!NOTE]
    > Whenever the Raspberry Pi Imager is restarted, make sure to re-type the passwords for the Pi and the Wifi. The Raspberry Pi Imager seems to be ruining the passwords saved in the advanced setting when the program is restarted even if the 'Image customization options' is set to 'to always use.'

## 2 - Setup the Base Image
1. Insert the SD card from the previous step into a Raspberry Pi and power it on. It should automatically connect to the wifi specified in Step 2.1.7. The Pi needs some time to boot for the first time.
2. Identify the MAC address of the Raspberry Pi. On a Linux comptuer, this can be done by running
    ```
    sudo arp-scan -I <network-device-name> -l -t 100 -r 5
    ```
    Alternatively, you can navigate to the router settings page (typically '192.168.1.1' or '10.0.0.1') using a web browser. The new Pi will appear as 'RASPBERRYPI.'
3. Add the MAC address to 'path-to-parent-directory/GTernal/config/mac_list.json' file as
    ```
    "<MAC-address-of-Pi>":"base_image"
    ```
    e.g., `{
            "d8:3a:dd:bc:0c:3c":"base_image"
           }`
4. On your computer, run
    ```
    cd <path-to-parent-directory>/GTernal/interfacing
    ./setup.sh setup_base_image
    ```
    When prompted for password, enter the password of your computer. When prompted 'Enter secrets for robots,' enter the password for the Raspberry Pis, 'raspberry'
    This will automatically setup the SD card as the base image. Wait for 'DONE SETTING UP BASE IMAGE' message on the terminal.
5. Shutdown the Pi by running
    ```
    cd <path-to-parent-directory>/GTernal/interfacing
    ./shutdown_robots.sh 1 base_image
    ```
6. Remove the SD card from the Pi and insert it to your computer.
7. On your computer, identify the name of the SD card by running
    ```
    sudo fdisk -l
    ```
    The device name will be something like '/dev/sdb'. Look for the device with the storage size that matches the capacity of the SD card.
   > [!NOTE]
   > You must enter the device name without any numbers. For example, if you see '/dev/sdb1' and '/dev/sdb2', the device name is `/dev/sdb`.
8. Create the image of the SD card on your computer by running
    ```
    sudo dd if=<name-of-the-sd-card> of=<desired-directory>/GTernal_base_image.img
    ```
    e.g., `sudo dd if=/dev/sdb of=~/GTernal_base_image.img`<br>
    This process takes a few minutes and does not provide any status messages.
9. Shrink the size of the base image by running
    ```
    wget https://raw.githubusercontent.com/Drewsif/PiShrink/master/pishrink.sh

    chmod +x ./pishrink.sh

    sudo ./pishrink.sh <path-to-the-base-image>/GTernal_base_image.img
    ```
    e.g., `sudo ./pishrink.sh ~/GTernal_base_image.img`

## 3 - Clone the Base Image
1. Insert a new SD card into your computer to load the base image.
2. Identify the name of the SD card by running
    ```
    sudo fdisk -l
    ```
3. Run
    ```
    sudo mount | grep <device-name>
    ```
    e.g. `sudo mount | grep /dev/sdb`
    to check if any of the partitions of the SD card is mounted.<br>
    If the output is not blank, run
    ```
    sudo umount <device-name1> <device-name2> ... <device-nameN>
    ```
    e.g. `sudo umount /dev/sdb1 /dev/sdb2`<br>
    to unmount all listed partitions of the device.
4. Load the SD card with the base image by running
    ```
    sudo dd if=<path-to-the-base-image>/GTernal_base_image.img of=<name-of-the-sd-card>
    ```
    e.g., `sudo dd if=~/GTernal_base_image.img of=/dev/sdb`<br>
    This process takes a few minutes and does not provide any status messages.

5. Load as many SD cards as needed with the base image. More detailed instructions on cloning an SD card image can be found here: https://beebom.com/how-clone-raspberry-pi-sd-card-windows-linux-macos/


# 4 - Load the Raspberry Pi with the Default firmware
This section assumes that:
1. You have micro-SD cards loaded as detailed in the previous section.
2. You have fully assembled robots.
   > [!NOTE]
   > Check if the Pi and Teensy are connected through UART. If the Pi is not connected with a programmed Teensy as instructed in Step 1, the firmware will not be started by the setup script properly in the following steps.
3. Only the new robots to be setup are connected to the WiFi.
   > [!WARNING]
   > The current automatic setup script looks for all robots with the MAC addresses specified in 'GTernal/config/mac_list.json' file and starts the setup process for the robots. Therefore, it will start the setup process even for the robots already with the firmware installed if they are connected to the WiFi. Since this may cause problems for the existing robots, it is advised to only turn on the new robots to be set up.


## 1 - Automatic Installation
<!-- > [!IMPORTANT]
> It is recommended to only turn on the new robots to be set up. The current automatic setup script looks for all robots with the MAC addresses specified in 'GTernal/config/mac_list.json' file and starts the setup process for the robots. Therefore, it will start the setup process even for the robots already with the firmware installed if they are connected to the WiFi. Since this may cause problems for the existing robots, it is recommended to only turn on the new robots that need to be set up. -->
1. Insert the loaded micro-SD cards to the robots' Raspberry Pis and power the robots up using the switch on each PCB. They should automatically connect to the wifi specified in Step 3.7.

2. On your computer, check if all the Raspberry Pis are connected to the WiFi router by running
    ```
    sudo arp-scan -I <network-device-name> -l -t 100 -r 5
    ```
    e.g., `sudo arp-scan -I enp4s0 -l -t 100 -r 5`

    Check if the number of Raspberry Pis found by the above command matches the total number of Raspberry Pis being set up. Wait a few more minutes and check again if not all Pis show up in the list.

   > [!NOTE]
   > The \<network-device-name\> can be found by running 'ifconfig' command into the terminal. There may be multiple network devices on your computer. The right device is the one connected to the same WiFi network as the Raspberry Pis. Typically, the local IP address assigned by a network router is '192.168.x.x' or '10.0.x.x.'

3. Assign an unallocated robot ID# for the MAC address of a Raspberry Pi found in the previous step. Then, add/replace the robot ID# and the MAC address of the Raspberry Pi in 'GTernal/config/mac_list.json'

   > [!NOTE]
   > When assigning an index to a new robot, assign an Aruco tag ID or the ID engraved on a Vicon hat plate. Make sure not to use any numbers that are already assigned to other robots. The lasercutting design files for the Vicon hats can be found in `GTernal/GTernal_design_files`

<!-- 4. Replace lines 12 and 13 in 'GTernal/docker/docker_run.sh' with the IP address and port of the MQTT host.

   > [!NOTE]
   > The default GTernal firmware requires an MQTT broker. For more information about the MQTT broker, please refer to https://github.com/robotarium/mqtt_broker. -->

4. Start the setup process by running
    ```
    cd path-to-parent-directory/GTernal/interfacing
    ./setup.sh setup_from_base <#-of-robots-being-set-up>
    ```
    e.g., `./setup.sh setup_from_base 10`

    When prompted 'Enter secrets for robots,' enter the password for the Raspberry Pis, 'raspberry'

    > [!NOTE]
    > The config/mac_list.json file of the robots will be updated to the config/mac_list.json file on your computer via SCP.

<!-- 5. Update the 'mac_list.json' file on the robots by running
```
cd path-to-parent-directory/GTernal/interfacing
./update_mac.sh <#-of-robots-being-set-up>
```
e.g. ./update_mac.sh 10

When prompted 'Enter secrets for robots,' enter the password for the Raspberry Pis, 'raspberry'

The Raspberry Pis will reboot after updating 'mac_list.json' file. -->


<!-- ## 2 - Network Host Name Change (Optional)
This process changes the name of the Raspberry Pi on the network. This helps to identify each robot easily on the router page ('192.168.1.1' or '10.0.0.1'). See Step 2.2, if you forgot how to access the router page.

After the setup process is completed,
Run
```
sudo raspi-config nonint do_hostname <hostname>
```
on the Raspberry Pi, and change the Host Name to 'robot#' where # is the new robot index assigned to the Pi in Step 3.

Reboot the Pi to apply the new network host name by selecting 'yes' when the raspi-config asks for a restart, or by using
```
sudo reboot
``` -->

<!-- ## 2 - Manual Installation

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
``` -->

# 5 - Test Robots
1. Clone the mqtt_broker repo
    ```
    git clone https://github.com/robotarium/mqtt_broker.git
    ```
2. Run the MQTT broker Docker container
    ```
    cd <path-to-parent-directory>/mqtt_broker/docker/
    ./docker_run.sh <ip-address-of-your-computer>
    ```
3. Clone the vizier repository with
    ```
    git clone https://github.com/robotarium/vizier.git
    ```

4. Install vizier with
    ```
    cd path-to-parent-directory/vizier
    python3 -m pip install .
    ```

5. Check the status of a robot with
    ```
    python3 -m vizier.vizier --host <ip-address-of-MQTT-host> --get <robot#>/status
    ```
    e.g., `python3 -m vizier.vizier --host 192.168.1.8 1884 --get 21/status`

6. Send a motor command to a robot with
    ```
    python3 -m vizier.vizier --host <ip-address-of-MQTT-host> --publish matlab_api/<robot#> '{"v":1.0,"w":0.0}'
    ```
    e.g., `python3 -m vizier.vizier --host 192.168.1.8 1884 --publish matlab_api/21 '{"v":1.0,"w":0.0}'`

    "v" and "w" are the desired linear and angular velocity of the robot, respectively. With this command, the robot will stop after 1 second for safety purposes.