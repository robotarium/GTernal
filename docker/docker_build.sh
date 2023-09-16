#!/bin/bash

sudo docker build --no-cache --build-arg ROBO_HOST=$1 --build-arg ROBO_PORT=$2 --tag gritsbot3:firmware .
