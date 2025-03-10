#!/bin/bash

source ../../config/env_variables.sh
if [ -n "$LOCAL_DOCKER_REGISTRY" ]; then
	sudo docker pull $LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_BASE
else
    sudo docker pull $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_BASE
fi