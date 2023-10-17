#!/bin/bash

(cd ../; sudo docker build --no-cache \
                           --tag gternal:firmware \
                           -f docker/Dockerfile .)
