.PHONY: flash update-idf

EXPORT_IDF=. ${HOME}/esp/esp-idf/export.sh

flash: build/$(pwd | rev | cut -d '/' -f 1 | rev).bin
	${EXPORT_IDF} && \
	idf.py flash && \
	idf.py monitor

build/%: includes.txt tidy
	${EXPORT_IDF} && idf.py build
	mkdir -p .vscode
	echo '{' > .vscode/c_cpp_properties.json
	echo '  "configurations": [' >> .vscode/c_cpp_properties.json
	echo '    {' >> .vscode/c_cpp_properties.json
	echo '      "name": "ESP32",' >> .vscode/c_cpp_properties.json
	echo '      "includePath": [' >> .vscode/c_cpp_properties.json
	echo '        "$${workspaceFolder}/main/include",' >> .vscode/c_cpp_properties.json
	cat includes.txt | xargs -I {} echo '        "{}",' >> .vscode/c_cpp_properties.json
	echo '        "$${workspaceFolder}/build/config"' >> .vscode/c_cpp_properties.json
	echo '      ],' >> .vscode/c_cpp_properties.json
	echo '      "defines": [' >> .vscode/c_cpp_properties.json
	# cat sdkconfig | grep '=' | grep -v '#' | xargs -I {} echo '    "{}",' >> .vscode/c_cpp_properties.json # already taken care of with <sdkconfig.h>
	echo '      ]' >> .vscode/c_cpp_properties.json
	echo '    }' >> .vscode/c_cpp_properties.json
	echo '  ],' >> .vscode/c_cpp_properties.json
	echo '  "version": 4' >> .vscode/c_cpp_properties.json
	echo '}' >> .vscode/c_cpp_properties.json

includes.txt: ${HOME}/esp/esp-idf/components | update-idf
	find -X ${HOME}/esp/esp-idf/components -type d -name include ! -path '*esp32c2*' ! -path '*esp32c3*' ! -path '*esp32c6*' ! -path '*esp32h2*' ! -path '*esp32h4*' ! -path '*esp32s2*' ! -path '*esp32s3*' -print > includes.txt
	find -X ${HOME}/esp/esp-idf/components -type d -name esp32 -path '*/include/esp32' -print >> includes.txt

update-idf: | deps-installed
	cd ${HOME}/esp/esp-idf && git submodule update --init --recursive

deps-installed:
	rm -f $@
	mkdir -p ${HOME}/esp
	if [ ! -d ${HOME}/esp/esp-idf ]; then cd ${HOME}/esp && git clone --recursive https://github.com/espressif/esp-idf.git; fi
	cd ${HOME}/esp/esp-idf &&	git pull && ./install.sh esp32 # ESP32 only, not S2, S3, etc.
	${EXPORT_IDF} && idf.py set-target esp32
	echo 'This file means you have all dependencies installed! Delete to reinstall.' > $@

tidy: $(shell find main -type f)
	rm -f $@
	clang-tidy --config-file=.clang-tidy --header-filter='.*/main/include/.*' $(shell find main -name '*.c' -o -name '*.h') -- -D__CLANG_TIDY__ -I main/include -I build/config $(shell cat includes.txt | sed 's/^/-I /g' | tr '\n' ' ') \
		|| :
	echo 'This file means clang-tidy had no problems with the source code.' > $@
