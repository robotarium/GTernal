#!/bin/bash

source ../config/env_variables.sh
docker tag gternal:firmware $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_NAME
docker push $DOCKER_HUB_REPO_USERNAME/$DOCKER_HUB_REPO_NAME
