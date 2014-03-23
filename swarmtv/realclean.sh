#!/usr/bin/env bash

find . -name CMakeCache.txt -print0 -type d | xargs -0 -Ifile sh -c "if [ -e file ]; then echo removing: file; rm -rf file ; fi"
find . -name CMakeFiles -print0 -type d | xargs -0 -Ifile sh -c "if [ -e file ]; then echo removing: file; rm -rf file ; fi"
rm Makefile
