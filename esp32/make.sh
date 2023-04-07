#!/bin/sh

# surprisingly easy
# note that you can initialize a project with `idf.py create-project [name]` from the parent directory

set -ex
. ${HOME}/esp/esp-idf/export.sh # incompatible with -u at the moment
set -u

idf.py build

idf.py flash
