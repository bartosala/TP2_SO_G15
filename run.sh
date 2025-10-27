#!/bin/bash

sudo chmod a+x ./Image/*

DEBUG=""
if [ "$1" == "-d" ]
then
    DEBUG="-s -S -d int"
fi

sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 $DEBUG