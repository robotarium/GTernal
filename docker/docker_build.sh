#!/bin/bash

(cd ../; sudo docker build --no-cache \
                           --build-arg ROBO_HOST=$1 \ 
                           --build-arg ROBO_PORT=$2 \
                           --tag gternal:firmware \
                           -f docker/Dockerfile .)
