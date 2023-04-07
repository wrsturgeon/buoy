#!/bin/sh

# based on https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html

set -eux

mkdir -p ${HOME}/esp
cd ${HOME}/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32 # ESP32 only, not S2, S3, etc.
