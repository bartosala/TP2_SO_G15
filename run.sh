#!/bin/bash

sudo chmod a+x ./Image/*

DEBUG=""
if [ "$1" == "-d" ]
then
    DEBUG="-s -S"
fi

## MACOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev coreaudio,id=speaker -machine pcspk-audiodev=speaker $DEBUG
## WSL
elif grep -qEi "(Microsoft|WSL)" /proc/version &> /dev/null
then
    sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=speaker,server=/mnt/wslg/PulseServer -machine pcspk-audiodev=speaker $DEBUG
else
    sudo qemu-system-x86_64 -hda Image/x64BareBonesImage.qcow2 -m 512 -audiodev pa,id=speaker -machine pcspk-audiodev=speaker $DEBUG
fi
