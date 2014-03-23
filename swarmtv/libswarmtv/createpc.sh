#!/usr/bin/env bash

#cat libswarmtv/libswarmtv.pc.template | sed "s|@PREFIX@|${1}|" > libswarmtv.pc
cat ${2}/libswarmtv.pc.template | sed "s|@PREFIX@|${1}|" > ${3}/libswarmtv.pc
