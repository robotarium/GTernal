# Lines 2 and 4 are needed for building the image on a computer instead on a Raspberry Pi
FROM multiarch/qemu-user-static:x86_64-arm as qemu
FROM arm64v8/alpine
COPY --from=qemu /usr/bin/qemu-arm-static /usr/bin

#TODO find a better place to put .git-credentials
# COPY .git-credentials /root/.git-credentials

RUN apk update && apk upgrade

# Get python + deps.  Most of these are actually for netifaces install
RUN apk add --no-cache git python3 python3-dev py3-pip gcc musl-dev linux-headers

RUN git config --global credential.helper store

# Install Vizier
ADD https://api.github.com/repos/robotarium/vizier/git/refs/heads/master version.json
RUN git clone https://github.com/robotarium/vizier
RUN cd vizier && python3 -m pip install .

# Install firmware
RUN git clone https://github.com/skim743/gritsbot_3
RUN cd gritsbot_3 && python3 -m pip install .

ARG ROBO_HOST
ARG ROBO_PORT

ENV ROBO_HOST ${ROBO_HOST}
ENV ROBO_PORT ${ROBO_PORT}

# Has to be in this format!  Docker doesn't like to interpret the env variables properly in JSON form
ENTRYPOINT python3 gritsbot_3/gritsbot/firmware.py -host ${ROBO_HOST} -port ${ROBO_PORT} gritsbot_3/config/mac_list.json
CMD [ ]
