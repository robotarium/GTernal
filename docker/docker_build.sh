#!/bin/bash

# Docker v4.38.0 requires the following command to be run as a workaround for a bug
# related to the multi-platform build feature.
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes

# Use the detect_serial module to get the correct serial port
source ../setup/env_variables.sh

# Docker build command
# Use the detect_serial module to get the correct serial port
(cd ../; sudo docker buildx build --no-cache \
                                  --tag gternal:firmware \
                                  --build-arg FIRMWARE_REPO_USERNAME=$FIRMWARE_REPO_USERNAME \
                                  --build-arg FIRMWARE_REPO_NAME=$FIRMWARE_REPO_NAME \
                                  --build-arg FIRMWARE_REPO_BRANCH=$FIRMWARE_REPO_BRANCH \
                                  --platform linux/arm64 \
                                  -f docker/Dockerfile .)