#!/bin/bash

(cd ../; sudo docker build --tag gternal:firmware \
                           -f docker/Dockerfile .)
