#!/bin/bash

# Docker v4.38.0 requires the following command to be run as a workaround for a bug
# related to the multi-platform build feature.
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes

source ../config/env_variables.sh

# Determine the Docker image to use
if [ -n "$LOCAL_DOCKER_REGISTRY" ]; then
    DOCKER_REGISTRY="0.0.0.0:9000"
    BASE_IMAGE="$LOCAL_FIRMWARE_BASE"
else
    DOCKER_REGISTRY="FIRMWARE_REPO_USERNAME"
    BASE_IMAGE="$DOCKER_HUB_REPO_BASE"
fi

# Docker build command
# Use the detect_serial module to get the correct serial port
sudo docker buildx build --no-cache \
                         --tag gternal:firmware \
                         --build-arg FIRMWARE_REPO_USERNAME=$FIRMWARE_REPO_USERNAME \
                         --build-arg FIRMWARE_REPO_NAME=$FIRMWARE_REPO_NAME \
                         --build-arg FIRMWARE_REPO_BRANCH=$FIRMWARE_REPO_BRANCH \
                         --build-arg DOCKER_REGISTRY=$DOCKER_REGISTRY \
                         --build-arg BASE_IMAGE=$BASE_IMAGE \
                         --platform linux/arm64 \
                         .