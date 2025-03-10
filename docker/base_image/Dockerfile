FROM arm64v8/ubuntu:jammy

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