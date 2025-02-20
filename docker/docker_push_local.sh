#!/bin/bash

sudo docker tag gternal:firmware 0.0.0.0:9000/firmware
sudo docker push 0.0.0.0:9000/firmware