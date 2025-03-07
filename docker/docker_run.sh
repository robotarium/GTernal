#!/bin/bash

source ../config/env_variables.sh

# Determine the Docker image to use
if [ -n "$LOCAL_DOCKER_REGISTRY" ]; then
    DOCKER_IMAGE="$LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_NAME"
else
    DOCKER_IMAGE="$DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_NAME"
fi

# Use the detect_serial module to get the correct serial port
docker run -d \
	   --restart unless-stopped \
	   --name firmware \
	   --net host \
	   --device $(sudo -u pi python3 ../firmware/RPi/detect_serial.py):/dev/ttyAMA0 \
	   -v /home/pi/git/GTernal/config/mac_list.json:/GTernal/config/mac_list.json \
	   $DOCKER_IMAGE \
	   python3 GTernal/firmware/RPi/pi_firmware.py \
	   -host $MQTT_HOST \
	   -port $MQTT_PORT \
	   GTernal/config/mac_list.json
