FROM debian:11

RUN apt update && apt -y install build-essential clang entr lcov
