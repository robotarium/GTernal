# Firmware docker image for robots

# 1 - Building the Firmware Image

1. Inspect and update the environment variables in `config/env_variables.sh`
2. Run
    ```
    ./docker_build
    ```

# 2 - Tag the image and push it to a Docker registry
With the current "docker_run.sh" script, each robot pulls the firmware image from either Docker Hub of a local Docker registry instead of building the firmware docker image directly on each robot. This saves a lot of time when building multiple robots. Also, with "v2tec/watchtower" image running on the robots, each robot downloads and runs the newest firmware image uploaded to the Docker Hub automatically. Therefore, it is unnecessary to manually update the firmware of each robot when a new firmware image is built and pushed to Docker Hub/local Docker registry.

## 2.1 - Docker Hub (Online)
1. Log into the Docker Hub with
    ```
    docker login
    ```
2. Run
    ```
    ./docker_push.sh
    ```

## 2.2 - Local Docker registry (Local)
1. Start a local Docker registry
    ```
    ./docker_registry.sh
    ```
2. Run
    ```
    ./docker_push_local.sh
    ```