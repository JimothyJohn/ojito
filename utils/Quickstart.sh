#!/usr/bin/env bash

PIO=$HOME/.platformio/penv/bin/pio
ONLY_CLEAN=false

set -o errexit
set -o nounset
set -o pipefail
if [[ "${TRACE-0}" == "1" ]]; then
    set -o xtrace
fi

help_function() {
    echo "Usage: $0 [OPTION]"
    echo
    echo "This script checks and installs PlatformIO and Docker if not present."
    echo
    echo "-h      Show this help message and exit."
}

install_platformio() {
    if ! command -v platformio &> /dev/null; then
        echo "PlatformIO not detected. Installing..."
        python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"
    else
        echo "PlatformIO is already installed."
    fi
}

main() {
    

    while getopts "hb" option; do
        case "${option}" in
            h) help_function; exit 0;;
            b) ONLY_CLEAN=true;;
            *) help_function; exit 1;;
        esac
    done

    if $ONLY_CLEAN; then
        $PIO run -t clean
        sudo rm -r ./.pio
        exit 0
    fi

    if [[ "$#" -gt 0 && "$1" == "-h" ]]; then
        help_function
        exit 0
    fi

    install_platformio

    touch include/secrets.h
    mkdir -p .logs/

    $PIO run -t clean
    sudo rm -r ./.pio
    $PIO run -t upload | tee ./.logs/upload.txt
    # $PIO run -t uploadfs | tee ./.logs/uploadfs.txt && \
    sleep 2
    $PIO device monitor | tee ./.logs/monitor.json
}

main "$@"
