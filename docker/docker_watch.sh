#!/bin/bash

#--label=com.centurylinklabs.watchtower.enable=false \

docker run -d \
	--restart always \
	--name watchtower \
	-v /var/run/docker.sock:/var/run/docker.sock \
	v2tec/watchtower:armhf-latest -i 60 --debug --cleanup
