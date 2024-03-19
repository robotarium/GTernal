#!/bin/bash

docker tag gternal:firmware 0.0.0.0:9000/firmware:test
docker push 0.0.0.0:9000/firmware:test
