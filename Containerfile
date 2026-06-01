FROM quay.io/almalinuxorg/almalinux:10.1

RUN dnf install -y epel-release \
    && dnf install -y gcc-c++ make cmake git python3-devel python3-pyyaml clang-tools-extra cppcheck \
    && dnf clean all

WORKDIR /src
COPY . .

RUN make check
