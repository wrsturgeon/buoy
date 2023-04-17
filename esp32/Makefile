EXPORT_IDF=. ${HOME}/esp/esp-idf/export.sh

flash: build/$(pwd | rev | cut -d '/' -f 1 | rev).bin
	${EXPORT_IDF} && \
	idf.py flash && \
	idf.py monitor

build/%: $(shell find main -type f) | deps-installed
	git submodule update --init --recursive
	${EXPORT_IDF} && idf.py build
	mkdir -p .vscode
	echo '{' > .vscode/c_cpp_properties.json
	echo '  "configurations": [' >> .vscode/c_cpp_properties.json
	echo '    {' >> .vscode/c_cpp_properties.json
	echo '      "name": "ESP32",' >> .vscode/c_cpp_properties.json
	echo '      "includePath": [' >> .vscode/c_cpp_properties.json
	echo '        "$${workspaceFolder}/main/include",' >> .vscode/c_cpp_properties.json
	find -X ${IDF_PATH}/components -type d -name include ! -path '*esp32c2*' ! -path '*esp32c3*' ! -path '*esp32c6*' ! -path '*esp32h2*' ! -path '*esp32h4*' ! -path '*esp32s2*' ! -path '*esp32s3*' | xargs -I {} echo '        "{}",' >> .vscode/c_cpp_properties.json
	find -X ${IDF_PATH}/components -type d -name esp32 -path '*/include/esp32' | xargs -I {} echo '        "{}",' >> .vscode/c_cpp_properties.json
	echo '        "$${workspaceFolder}/build/config"' >> .vscode/c_cpp_properties.json
	echo '      ],' >> .vscode/c_cpp_properties.json
	echo '      "defines": [' >> .vscode/c_cpp_properties.json
	# cat sdkconfig | grep '=' | grep -v '#' | xargs -I {} echo '    "{}",' >> .vscode/c_cpp_properties.json
	echo '      ]' >> .vscode/c_cpp_properties.json
	echo '    }' >> .vscode/c_cpp_properties.json
	echo '  ],' >> .vscode/c_cpp_properties.json
	echo '  "version": 4' >> .vscode/c_cpp_properties.json
	echo '}' >> .vscode/c_cpp_properties.json

deps-installed:
	mkdir -p ${HOME}/esp
	if [ ! -d ${HOME}/esp/esp-idf ]; then cd ${HOME}/esp && git clone --recursive https://github.com/espressif/esp-idf.git; fi
	cd ${HOME}/esp/esp-idf &&	git pull && ./install.sh esp32 # ESP32 only, not S2, S3, etc.
	${EXPORT_IDF} && idf.py set-target esp32
	echo 'This file means you have all dependencies installed! Delete to reinstall.' > $@
