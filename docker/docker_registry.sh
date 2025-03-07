#!/bin/bash

#--label=com.centurylinklabs.watchtower.enable=false \

docker run -d \
		   --restart unless-stopped \
	       -p 9000:5000 \
		   --name robotarium_registry \
		   registry