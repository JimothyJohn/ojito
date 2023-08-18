#!/usr/bin/env bash
PIO=$HOME/.platformio/penv/bin/pio
BUILD=false
FILE_SYSTEM=false

set -o errexit
set -o nounset
set -o pipefail
if [[ "${TRACE-0}" == "1" ]]; then
    set -o xtrace
fi

help_function() {
    echo "Usage: $0 [OPTION]"
    echo
    echo "This script checks and installs PlatformIO if not present."
    echo
    echo "-b      Rebuild Ojito library."
    echo
    echo "-f      Update file system."
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
            b) BUILD=true;;
            f) FILE_SYSTEM=true;; 
            *) help_function; exit 1;;
        esac
    done

    if $BUILD; then
        $PIO run -t clean
        sudo rm -r ./.pio
    fi

    if $FILE_SYSTEM; then
        $PIO run -t uploadfs | tee ./.logs/uploadfs.txt
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
    c
    sleep 2
    $PIO device monitor | tee ./.logs/monitor.json
}

main "$@"
