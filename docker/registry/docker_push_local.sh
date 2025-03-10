#!/bin/bash

source ../config/env_variables.sh
sudo docker tag gternal:firmware $LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_NAME
sudo docker push $LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_NAME