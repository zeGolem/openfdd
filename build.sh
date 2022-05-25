#!/bin/sh

set -ex;

# TODO: Better build system?

SOURCES="src/*.cpp src/usb/*.cpp src/drivers/*.cpp src/drivers/*/*.cpp"

build_dev() {
	g++ $SOURCES                     \
		-lusb-1.0                    \
		-Isrc/                       \
		-fsanitize=address,undefined \
		-Wall -Wextra                \
		-std=c++20 -pedantic         \
		-g                           \
		-o openfdd
}

build_release() {
	g++ $SOURCES                     \
		-lusb-1.0                    \
		-Isrc/                       \
		-Wall -Wextra                \
		-std=c++20 -pedantic         \
		-O2                          \
		-o openfdd
}

if [ "$1" = "dev" ]; then
	build_dev;
else
	build_release;
fi;
