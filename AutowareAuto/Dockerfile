# syntax=docker/dockerfile:1

# see: https://github.com/osrf/docker_images/tree/master/ros/foxy/ubuntu/focal

# image from which all other stages derive
ARG BASEIMAGE=ubuntu:focal-20241011

##################
# openadkit-ros-foxy
##################
FROM ${BASEIMAGE} AS openadkit-ros-foxy
ARG UBUNTU_SNAPSHOT=20250206T040000Z

USER root
SHELL ["/bin/bash", "-c"]

# configure debian and terminal for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=localhost:0.0
RUN echo 'debconf debconf/frontend select Noninteractive' | debconf-set-selections
RUN dpkg-divert --local --rename --add /sbin/initctl
RUN ln -sf /bin/true /sbin/initctl
RUN echo -e '#!/bin/sh\nexit 0' > /usr/sbin/policy-rc.d

# configure python
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# use bash as default shell
RUN echo "dash dash/sh boolean false" | debconf-set-selections
RUN dpkg-reconfigure dash

# use a snapshot to ensure package updates don't break this image.
ADD https://github.com/xronos-inc/apt-focal-snapshot/raw/refs/tags/v1.0.0/apt-snapshot.sh /usr/local/bin/apt-snapshot
RUN chmod 0755 /usr/local/bin/apt-snapshot
RUN apt-snapshot

# # configure locales
RUN apt-get install -y -q --no-install-recommends locales
RUN locale-gen en_US.UTF-8
RUN dpkg-reconfigure -f noninteractive locales
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

# install utilities
RUN apt-get install -y -q --no-install-recommends \
        ca-certificates \
        curl \
        git \
        gnupg2 \
        wget 
RUN git config --global advice.detachedHead false

# install common build tools
RUN apt-get install -y -q --no-install-recommends \
        build-essential \
        dirmngr \
        jq

# cmake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null \
        | gpg --dearmor - \
        | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' \
        | tee /etc/apt/sources.list.d/kitware.list >/dev/null
RUN apt-get update -q 
RUN apt-get install -y -q --no-install-recommends \
        cmake=3.31.5-0kitware1ubuntu20.04.1 \
        cmake-data=3.31.5-0kitware1ubuntu20.04.1

# use ccache in this and derived images to accelerate builds
RUN mkdir -p /root/.ccache/
RUN mkdir -p /ccache
COPY ccache.conf /root/.ccache/ccache.conf
RUN apt-get install -y -q --no-install-recommends \
        ccache

# python
RUN apt-get install -y -q --no-install-recommends \
        python3-dev=3.8.2-0ubuntu2 

# boost -- install recommended packages as many are used by ROS
RUN apt-get install -y -q \
        libboost1.71-dev=1.71.0-6ubuntu6 \
        libboost1.71-tools-dev=1.71.0-6ubuntu6

# add ros distro to apt
# tag: foxy/2023-06-20
RUN curl -sSL https://github.com/ros/rosdistro/raw/refs/tags/foxy/2023-06-20/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg
RUN echo "deb [signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu focal main" \
    | tee /etc/apt/sources.list.d/ros2.list
RUN apt-get update -q

# ROS depends on an upgraded version of libssl1.1, but 1.1.1f-1ubuntu2.24 breaks
# it in the current snapshot, so version-lock to the prior version
RUN apt-get install --upgrade -y -q libssl1.1=1.1.1f-1ubuntu2.23

# install colcon tools
RUN apt-get install -y -q --no-install-recommends \
        python3-colcon-common-extensions \
        python3-colcon-mixin \
        python3-psutil \
        python3-vcstool
        
# setup colcon mixin and metadata
RUN colcon mixin add default https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml
RUN colcon mixin update default
RUN colcon metadata add default https://raw.githubusercontent.com/colcon/colcon-metadata-repository/master/index.yaml
RUN colcon metadata update default

# install ros foxy base
RUN apt-get install -y -q --no-install-recommends \
        ros-foxy-ros-core=0.9.2-1* \
        ros-foxy-ros-base=0.9.2-1*

# package cleanup, but ensure snapshot persists
RUN apt-get autoremove -y -q
RUN apt-snapshot && apt-get clean -y -q

##################
# openadkit-ros-dependencies
##################
FROM openadkit-ros-foxy AS openadkit-ros-dependencies
USER root
SHELL ["/bin/bash", "-c"]

# language and encoding
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# configure debian and terminal for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=localhost:0.0

# configure python
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# use a snapshot to ensure package updates don't break this image.
# this must be run for any image that does not have apt lists cached,
# such as a base image, or a stage derived from a stage that cleaned apt.
RUN apt-snapshot

# udprelay
RUN apt-get install -y -q --no-install-recommends \
        libpcap-dev
RUN git clone https://github.com/rigtorp/udpreplay -q --branch v1.1.0 \
    && mkdir -p udpreplay/build \
    && cd udpreplay/build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release \
    && make \
    && make install \
    && cd - \
    && rm -rf udpreplay/

# networking
RUN apt-get install -y -q --no-install-recommends \
        iproute2 \
        netstat-nat

# cyclone-dds
RUN apt-get install -y -q --no-install-recommends \
        ros-foxy-cyclonedds \
        ros-foxy-rmw-cyclonedds-cpp

# scientific computing
RUN apt-get install -y -q --no-install-recommends \
        python3-numpy=1:1.17.4-5* \
        python3-scipy=1.3.3-3* 

# pcl
RUN apt-get install -y -q --no-install-recommends \
        libfreenect-dev \
        libopenni-dev \
        libopenni2-dev \
        libpcl-dev \
        libpng-dev \
        libusb-dev \
        libusb-1.0-0-dev \
        ros-foxy-pcl-ros \
        ros-foxy-pcl-conversions \
        ros-foxy-perception-pcl

# openadkit ros dependencies
RUN apt-get install -y -q --no-install-recommends \
        libeigen3-dev \
        libcgal-dev \
        libgeographic-dev \
        libomp-dev \
        libyaml-cpp-dev \
        ros-foxy-acado-vendor \
        ros-foxy-automotive-platform-msgs \
        ros-foxy-autoware-auto-msgs \
        ros-foxy-casadi-vendor \
        ros-foxy-geographic-msgs \
        ros-foxy-io-context \
        ros-foxy-joy \
        ros-foxy-joy-linux \
        ros-foxy-lanelet2 \
        ros-foxy-lanelet2-core \
        ros-foxy-lanelet2-io \
        ros-foxy-lanelet2-projection \
        ros-foxy-lanelet2-routing \
        ros-foxy-lanelet2-traffic-rules \
        ros-foxy-lgsvl-msgs \
        ros-foxy-osqp-vendor \
        ros-foxy-point-cloud-msg-wrapper \
        ros-foxy-robot-localization \
        ros-foxy-ros2-socketcan \
        ros-foxy-rosauth \
        ros-foxy-rosbridge-suite \
        ros-foxy-tvm-vendor


# NVIDIA GPU support -- this expects that the driver itself is mapped into the container
# using something like nvidia-container-toolkit or gpu-operator
ADD https://gitlab.com/nvidia/container-images/vulkan/-/raw/ubuntu18.04/nvidia_icd.json \
        /etc/vulkan/icd.d/nvidia_icd.json
RUN chmod 0644 /etc/vulkan/icd.d/nvidia_icd.json
ADD https://gitlab.com/nvidia/container-images/opengl/-/raw/ubuntu20.04/glvnd/runtime/10_nvidia.json \
        /usr/share/glvnd/egl_vendor.d/10_nvidia.json
RUN chmod 0644 /usr/share/glvnd/egl_vendor.d/10_nvidia.json
RUN apt-get install -y -q --no-install-recommends \
        libglvnd0 \
        libgl1 \
        libglx0 \
        libegl1 \
        libgles2 \
        libxcb1-dev
        
# vulkan dependencies
RUN apt-get install -y -q --no-install-recommends \
        libx11-6 \
        libxau6 \
        libxcb1 \
        libxdmcp6 \
        libxext6 \
        libvulkan1 \
        libgtk2.0-0 \
        mesa-utils \
        vulkan-utils

# rviz2 dependencies
RUN apt-get install -y -q --no-install-recommends \
        python3-matplotlib \
        ros-foxy-rviz2 \
        ros-foxy-rviz-common \
        ros-foxy-rviz-default-plugins \
        ros-foxy-qt-dotgraph

# package cleanup
RUN apt-get autoremove -y -q
RUN apt-snapshot && apt-get clean -y -q


##################
# openadkit-autoware stage
##################
FROM openadkit-ros-dependencies AS openadkit-autoware
USER root
SHELL ["/bin/bash", "-c"]

# language and encoding
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# configure debian and terminal for non-interactive installation
ENV DEBIAN_FRONTEND=noninteractive
ENV DISPLAY=localhost:0.0

# configure python
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# use a snapshot to ensure package updates don't break this image.
# this must be run for any image that does not have apt lists cached,
# such as a base image, or a stage derived from a stage that cleaned apt.
RUN apt-snapshot

## install build tools that are not needed in images that use the build artifacts they generate

# cpplint
RUN apt-get install -y -q --no-install-recommends \
        python3-pip
RUN pip3 install -U cpplint==2.0.0

# ament build tools
RUN apt-get install -y -q --no-install-recommends \
        bison \
        clang \
        doxygen \
        ros-foxy-ament-clang-format \
        ros-foxy-ament-cpplint \
        ros-foxy-ament-cmake-clang-format \
        ros-foxy-ament-cmake-cpplint \
        ros-foxy-ament-cmake-uncrustify \
        ros-foxy-ros-testing \
        ros-foxy-osrf-testing-tools-cpp \
        ros-foxy-apex-test-tools 

COPY . /opt/autoware

# Add Lexus RX 450 meshes
RUN mkdir -p /opt/autoware/src/urdf/lexus_rx_450h_description/meshes
ADD https://soafee-avp-demo.s3.us-east-1.amazonaws.com/LexusRX.tar.gz .
RUN tar -xvf LexusRX.tar.gz -C /opt/autoware/src/urdf/lexus_rx_450h_description/meshes/

WORKDIR /opt/autoware

RUN --mount=type=cache,id=openadkit-cache,target=/ccache \
        ./build.sh

RUN apt-snapshot && apt-get clean -y -q


##################
# application stage
##################
FROM ${BASEIMAGE} AS openadkit
USER root
SHELL ["/bin/bash", "-c"]

# language and encoding
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# configure python
ENV PYTHONDONTWRITEBYTECODE=1
ENV PYTHONUNBUFFERED=1

# ros dependencies
COPY --from=openadkit-ros-dependencies / /

# autoware
COPY --from=openadkit-autoware /opt/autoware /opt/autoware

# lingua-franca tracing
COPY --from=xronosinc/lf-trace-plugin-api:v1.2.1-c /usr/local/lib/libtelegraf_lf_trace_plugin.a /usr/local/lib/
COPY --from=xronosinc/lf-trace-plugin-api:v1.2.1-c /usr/local/lib/libmock_lf_trace_plugin.a /usr/local/lib/
COPY --from=xronosinc/lf-trace-plugin-api:v1.2.1-c /lf-trace-plugin-api-LICENSE /lf-trace-plugin-api-LICENSE

# we leave apt list cache in the final image to avoid a bug on arm64
# where apt-get errors with "Snapshot not available". The size of
# /var/lib/apt/lists/* is negligable.
RUN apt-snapshot && apt-get clean -y -q

COPY entrypoint /
ENTRYPOINT ["/entrypoint"]
