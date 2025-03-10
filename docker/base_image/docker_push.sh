#!/bin/bash

source ../config/env_variables.sh
sudo docker tag gternal:base $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_BASE
sudo docker push $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_BASE