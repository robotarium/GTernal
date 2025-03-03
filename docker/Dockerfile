# Lines 3 and 5 are needed for building the image on a computer (x86) instead on a Raspberry Pi
# Comment the two lines below if you are directly building the image on a Raspberry Pi
FROM multiarch/qemu-user-static:x86_64-arm as qemu
FROM arm64v8/alpine
COPY --from=qemu /usr/bin/qemu-arm-static /usr/bin

# Source the environment variables
RUN source ../setup/env_vars.sh

RUN apk update && apk upgrade

# Get python + deps.  Most of these are actually for netifaces install
RUN apk add --no-cache git python3 python3-dev py3-pip gcc musl-dev linux-headers

# Get ssh and sshpass for shutting down the Raspberry Pi within a Docker container
RUN apk add --update --no-cache openssh sshpass

# Install cvxopt for local barriers
RUN python3 -m pip install numpy scipy cvxopt --user --break-system-packages

# Install Vizier
ADD https://api.github.com/repos/robotarium/vizier/git/refs/heads/master vizier_version.json
RUN git clone https://github.com/robotarium/vizier
RUN cd vizier && pip3 install .  --break-system-packages

# Install firmware
ADD ${FIRMWARE_REPO_URL}/commits/main gternal_version.json
RUN git clone ${FIRMWARE_REPO_URL}
RUN cd GTernal && pip3 install . --break-system-packages