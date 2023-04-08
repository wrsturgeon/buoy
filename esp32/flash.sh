#!/bin/sh

# surprisingly easy
# note that you can initialize a project with `idf.py create-project [name]` from the parent directory

set -e
. ${HOME}/esp/esp-idf/export.sh # incompatible with -u at the moment
set -ux

if [ ! -d build ]
then
  idf.py set-target esp32
fi

curl --output-dir main -o Kconfig.projbuild https://raw.githubusercontent.com/espressif/esp-idf/master/examples/peripherals/gpio/generic_gpio/main/Kconfig.projbuild
# idf.py menuconfig

idf.py build

# Update VSCode information
mkdir -p .vscode
echo '{' > .vscode/c_cpp_properties.json
echo ' "configurations": [' >> .vscode/c_cpp_properties.json
echo '  {' >> .vscode/c_cpp_properties.json
echo '   "name": "ESP32",' >> .vscode/c_cpp_properties.json
echo '   "includePath": [' >> .vscode/c_cpp_properties.json
find -X ${IDF_PATH}/components -type d -name include | xargs -I {} echo '    "{}",' >> .vscode/c_cpp_properties.json
echo '   ],' >> .vscode/c_cpp_properties.json
echo '   "defines": [' >> .vscode/c_cpp_properties.json
cat sdkconfig | grep '=' | grep -v '#' | xargs -I {} echo '    "{}",' >> .vscode/c_cpp_properties.json
echo '   ]' >> .vscode/c_cpp_properties.json
echo '  }' >> .vscode/c_cpp_properties.json
echo ' ],' >> .vscode/c_cpp_properties.json
echo ' "version": 4' >> .vscode/c_cpp_properties.json
echo '}' >> .vscode/c_cpp_properties.json

idf.py flash

idf.py monitor
