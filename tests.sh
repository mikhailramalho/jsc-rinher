#!/bin/bash

set -x

docker buildx build --platform linux/amd64 -f ./Dockerfile -t jsc-rinher .

for json_file in tests/*.json; do
  docker run -v $PWD/$json_file:/var/rinha/source.rinha.json jsc-rinher
done

for json_file in tests-2/*.json; do
  docker run -v $PWD/$json_file:/var/rinha/source.rinha.json jsc-rinher
done
