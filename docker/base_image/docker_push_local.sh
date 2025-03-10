#!/bin/bash

source ../../config/env_variables.sh
sudo docker tag gternal:base 0.0.0.0:9000/$LOCAL_FIRMWARE_BASE
sudo docker push 0.0.0.0:9000/$LOCAL_FIRMWARE_BASE