#!/bin/bash

source ../config/env_variables.sh
sudo docker tag gternal:firmware $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_NAME
sudo docker push $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_NAME