FROM ubuntu:16.04
# Install dependencies
RUN apt-get update && apt-get install -y apt-utils git \
# Build dependencies \
    cmake libboost-dev libboost-program-options-dev libboost-math-dev libboost-log-dev libboost-system-dev libboost-timer-dev build-essential \
# Python dependencies \
    python3 python3-pip \
# Cleanup to reduce docker image size
   && rm -rf /var/lib/apt/lists/*

# Install S3M
RUN mkdir -p /S3M
ADD . /S3M
WORKDIR /S3M
# Compile C++ code
RUN mkdir build
WORKDIR build
RUN cmake ../code/cpp
RUN make
# Add s3m binary to path
ENV PATH="/S3M/build:${PATH}"
# Preprocessing scripts are path sensitive
WORKDIR /S3M/code/python

# Setup python environment
RUN pip3 install -r requirements.txt
# Add analysis scripts to path
ENV PYTHONPATH="/S3M/code/python:/S3M/code/python/utils"
ENV PATH="/S3M/code/python:${PATH}"
