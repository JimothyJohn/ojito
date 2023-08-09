FROM python:3.10-slim-buster

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
    python-dev \
    python3-dev \
    python3-pip \
    python3-venv

COPY requirements.txt /
RUN pip install --no-cache-dir -U pip && \
    pip install --no-cache-dir -r /requirements.txt

COPY ./src /src

RUN useradd -U ojitoß 
RUN chown ojito /src
USER ojito 

# CMD ["python","/input"]
