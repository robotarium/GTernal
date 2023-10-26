#!/bin/bash

# Use the detect_serial module to get the correct serial port
docker run -d \
	   --restart unless-stopped \
	   --name firmware \
	   --net host \
	   --device $(sudo -u pi python3 ../firmware/RPi/detect_serial.py):/dev/ttyAMA0 \
	   -v /home/pi/git/GTernal/config/mac_list.json:/GTernal/config/mac_list.json \
	   skim743/firmware \
	   python3 GTernal/firmware/RPi/pi_firmware.py \
	   -host '192.168.1.8' \
	   -port '1884' \
	   GTernal/config/mac_list.json
