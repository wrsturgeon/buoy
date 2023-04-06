#!/bin/sh

set -eux

clang-tidy src/* include/* -- -Iinclude -isystem${HOME}/.platformio/packages/toolchain-atmelavr/avr/include -DPLATFORMIO=60106 -DARDUINO_AVR_UNO -DF_CPU=16000000L -DARDUINO_ARCH_AVR -DARDUINO=10808 -D__AVR_ATmega328P__
