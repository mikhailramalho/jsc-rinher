# syntax=docker/dockerfile:1
FROM ubuntu:latest

RUN apt update && \
    apt install -y g++ git make curl tar zip cmake build-essential libjsoncpp-dev pkg-config

COPY ast.cpp .
COPY main.cpp .
COPY out.h .
COPY generate.h .
COPY ast.h .
COPY utils.h .
COPY CMakeLists.txt .
COPY run.sh .
RUN chmod +755 ./run.sh

RUN cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .

CMD [ "./run.sh", "/var/rinha/source.rinha.json"]
