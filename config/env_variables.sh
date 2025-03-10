# GitHub Repository for the Firmware
export FIRMWARE_REPO_USERNAME=robotarium
export FIRMWARE_REPO_NAME=GTernal
export FIRMWARE_REPO_URL=https://github.com/${FIRMWARE_REPO_USERNAME}/${FIRMWARE_REPO_NAME}
export FIRMWARE_REPO_BRANCH=main

# Docker Hub Repository for the Firmware
# This will not be used if LOCAL_DOCKER_REGISTRY is set
export DOCKER_HUB_REPO_USERNAME=skim743
export DOCKER_HUB_REPO_NAME=firmware
export DOCKER_HUB_REPO_BASE=base

# Local Docker Registry
export LOCAL_DOCKER_REGISTRY=192.168.1.136:9000
export LOCAL_FIRMWARE_NAME=firmware
export LOCAL_FRIMWARE_BASE=base

# MQTT HOST
export MQTT_HOST=192.168.1.136
export MQTT_PORT=1884