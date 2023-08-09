#!/usr/bin/env bash

PIO=$HOME/.platformio/penv/bin/pio

$PIO run -t upload | tee ./.logs/upload.txt && $PIO run -t uploadfs | tee ./.logs/uploadfs.txt
sleep 1
$PIO device monitor | tee ./.logs/monitor.json
