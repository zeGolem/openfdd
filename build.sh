#!/bin/sh

set -ex;

# TODO: Better build system?

SOURCES="src/*.cpp src/usb/*.cpp src/drivers/*.cpp src/drivers/*/*.cpp"

g++ $SOURCES                     \
	-lusb-1.0                    \
	-Isrc/                       \
	-fsanitize=address,undefined \
	-Wall -Wextra                \
	-std=c++20 -pedantic         \
	-g                           \
	-o openfdd
