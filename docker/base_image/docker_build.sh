#!/bin/bash

# Docker v4.38.0 requires the following command to be run as a workaround for a bug
# related to the multi-platform build feature.
sudo docker run --rm --privileged multiarch/qemu-user-static --reset -p yes -c yes

# Docker build command
sudo docker buildx build --no-cache \
                         --tag gternal:base \
                         --platform linux/arm64 \
                         .