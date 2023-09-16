#!/bin/bash

# Use the detect_serial module to get the correct serial port
docker run -d \
	   --restart unless-stopped \
	   --name firmware \
	   --net host \
	   --device $(sudo -u pi python3 ../gritsbot/detect_serial.py):/dev/ttyAMA0 \
	   skim743/firmware
