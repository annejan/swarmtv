#!/usr/bin/env bash

cat libswarmtv/libswarmtv.pc.template | sed "s|@PREFIX@|${1}|" > libswarmtv.pc
