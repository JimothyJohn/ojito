FROM debian:bookworm-20230725-slim

ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get -y install --no-install-recommends \
    curl \
    build-essential \ 
    unzip \
    cmake \
    cmake-data \
    pkg-config \
    python3 \
    python3-dev \
    python3-pip \
    python3-venv && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"

COPY requirements.txt /
RUN pip install --no-cache-dir -U pip && \
    pip install --no-cache-dir -r /requirements.txt

COPY . /src
