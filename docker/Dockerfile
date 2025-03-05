# Lines 3 and 5 are needed for building the image on a computer (x86) instead on a Raspberry Pi
# Comment the two lines below if you are directly building the image on a Raspberry Pi
# FROM multiarch/qemu-user-static:x86_64-arm AS qemu
FROM arm64v8/ubuntu:jammy
# COPY --from=qemu /usr/bin/qemu-arm-static /usr/bin

# # Source the environment variables
# COPY ../setup/env_variables.sh /tmp/env_variables.sh
# RUN . /tmp/env_variables.sh && \
#     echo "export FIRMWARE_REPO_URL=$FIRMWARE_REPO_URL" >> /etc/environment && \
#     echo "export FIRMWARE_REPO_BRANCH=$FIRMWARE_REPO_BRANCH" >> /etc/environment

# Load environment variables
ARG FIRMWARE_REPO_USERNAME
ARG FIRMWARE_REPO_NAME
ARG FIRMWARE_REPO_BRANCH
ENV FIRMWARE_REPO_USERNAME=${FIRMWARE_REPO_USERNAME}
ENV FIRMWARE_REPO_NAME=${FIRMWARE_REPO_NAME}
ENV FIRMWARE_REPO_URL=https://github.com/${FIRMWARE_REPO_USERNAME}/${FIRMWARE_REPO_NAME}
ENV FIRMWARE_REPO_BRANCH=${FIRMWARE_REPO_BRANCH}

RUN apt-get -y update && apt-get -y upgrade

# Get python + deps.  Most of these are actually for netifaces install
RUN apt-get -y install git python3 python3-dev python3-pip gcc musl-dev net-tools python3-numpy python3-scipy libopenblas-dev libsuitesparse-dev build-essential

# Get the dependencies for cvxopt
RUN apt-get -y install python3-numpy python3-scipy libopenblas-dev libsuitesparse-dev build-essential

# Get ssh and sshpass for shutting down the Raspberry Pi within a Docker container
RUN apt-get -y install openssh-client sshpass

# Install cvxopt for local barriers
RUN CVXOPT_SUITESPARSE_INC_DIR=/usr/include/suitesparse pip3 install cvxopt --user

# Install Vizier
ADD https://api.github.com/repos/robotarium/vizier/git/refs/heads/master version.json
RUN git clone https://github.com/robotarium/vizier
RUN cd vizier && pip3 install .

# Install firmware
ADD https://api.github.com/repos/${FIRMWARE_REPO_USERNAME}/${FIRMWARE_REPO_NAME}/git/refs/heads/${FIRMWARE_REPO_BRANCH} version.json
RUN git clone -b ${FIRMWARE_REPO_BRANCH} ${FIRMWARE_REPO_URL}
RUN cd GTernal && pip3 install .