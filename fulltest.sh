#!/bin/sh

# Runs tests with different compilers against different targets

set -e

BUILD=$(mktemp -d)
echo "Building in temporary directory $BUILD"

trap cleanup 0

cleanup()
{
    if [ -e "$BUILD" ]
    then
	echo "Deleting $BUILD"
	rm -rf "$BUILD"
    fi
}

echo "Debug build with gcc"
make TARGET=debug   BUILDROOT="$BUILD/gcc"   CC=gcc

echo "Debug build with clang"
make TARGET=debug   BUILDROOT="$BUILD/clang" CC=clang

echo "Release build with gcc"
make TARGET=release BUILDROOT="$BUILD/gcc"   CC=gcc

echo "Release build with clang"
make TARGET=release BUILDROOT="$BUILD/clang" CC=clang
