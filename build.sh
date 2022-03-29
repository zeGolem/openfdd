#!/bin/sh

set -ex;

# TODO: Better build system?

g++ src/*.cpp src/**/*.cpp       \
	-lusb-1.0                    \
	-Isrc/                       \
	-fsanitize=address,undefined \
	-Wall -Wextra                \
	-std=c++20 -pedantic         \
	-g                           \
	-o openfdd
