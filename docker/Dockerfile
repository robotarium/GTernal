# Lines 2 and 4 are needed for building the image on a computer instead on a Raspberry Pi
FROM multiarch/qemu-user-static:x86_64-arm as qemu
FROM arm64v8/alpine
COPY --from=qemu /usr/bin/qemu-arm-static /usr/bin

RUN apk update && apk upgrade

# Get python + deps.  Most of these are actually for netifaces install
RUN apk add --no-cache git python3 python3-dev py3-pip gcc musl-dev linux-headers

# Install Vizier
ADD https://api.github.com/repos/robotarium/vizier/git/refs/heads/master version.json
RUN git clone https://github.com/robotarium/vizier
RUN cd vizier && python3 -m pip install .

# Install firmware
COPY . /GTernal
RUN cd GTernal && python3 -m pip install .