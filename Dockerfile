FROM ubuntu:18.04
WORKDIR /home
RUN apt update
RUN apt install -y sudo
RUN apt install -y git
RUN mkdir TinyGarble2.0_plus_dep
WORKDIR /home/TinyGarble2.0_plus_dep
RUN git clone https://github.com/IntelLabs/TinyGarble2.0.git
WORKDIR /home/TinyGarble2.0_plus_dep/TinyGarble2.0
RUN ./setup.sh