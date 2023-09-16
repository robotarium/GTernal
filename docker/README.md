# Firmware docker image for robots

# 1 - Building the image

Run
```
./docker_build <ip-address-of-MQTT-host-computer> <port-of-MQTT-host-computer>
```
(ex. ./docker_build 192.168.1.5 1884)

# 2 - Tag the image and push it to the Docker Hub
Log into the Docker Hub with
```
docker login
```

After logging in, run
```
./docker_push.sh
```
With the current "docker_run.sh" script, each robot pulls the image from Docker Hub instead of building the firmware docker image by itself. This saves a lot of time when building multiple robots. Also, with "v2tec/watchtower" image running on the robots, each robot downloads and runs the newest firmware image uploaded to the Docker Hub automatically. Therefore, it is unnecessary to manually update the firmware of each robot when a new firmware image is built and pushed to the Docker Hub.
