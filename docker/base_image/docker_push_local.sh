#!/bin/bash

source ../../config/env_variables.sh
sudo docker tag gternal:base $LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_BASE
sudo docker push $LOCAL_DOCKER_REGISTRY/$LOCAL_FIRMWARE_BASE