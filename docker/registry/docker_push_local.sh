#!/bin/bash

source ../config/env_variables.sh
sudo docker tag gternal:firmware 0.0.0.0:9000/$LOCAL_FIRMWARE_NAME
sudo docker push 0.0.0.0:9000/$LOCAL_FIRMWARE_NAME