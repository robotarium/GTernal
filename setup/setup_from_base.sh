#!/bin/bash

source /home/pi/git/GTernal/config/env_variables.sh

# Exit immediately if a command exits with a non-zero status
set -e

# Define a function to handle errors
error_handler() {
    echo "Error encountered during step: $current_step. Exiting script with status $?."
}

# Trap errors and call the error_handler function
trap error_handler ERR

DEL="-------"
FIN="DONE"

print_start() {
	echo "$DEL ""$1"" $DEL"
}

print_end() {
	echo "$DEL ""$FIN ""$1"" $DEL"	
}

# current_step="UPDATING GTERNAL REPO"
# print_start "$current_step"
# cd /home/pi/git/GTernal
# git pull --rebase origin $FIRMWARE_REPO_BRANCH
# print_end "$current_step"

current_step="PULLING GTERNAL BASE IMAGE"
print_start "$current_step"
cd /home/pi/git/GTernal/docker/base_image
sudo ./docker_pull.sh
print_end "$current_step"

current_step="STARTING FIRMWARE CONTAINER"
print_start "$current_step"
cd /home/pi/git/GTernal/docker
sudo ./docker_run.sh
print_end "$current_step"

current_step="SETTING UP NETWORK HOST NAME"
print_start "$current_step"
cd /home/pi/git/GTernal/setup
robot_id=$(python3 check_hostname.py ../config/mac_list.json)
sudo raspi-config nonint do_hostname $robot_id
print_end "$current_step"

print_end "SETTING UP FROM BASE IMAGE"