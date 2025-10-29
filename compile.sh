#!/bin/bash

NOMBRE="tp_so_g15"

# Select memory manager: -buddy (default) or -bitmap
MM_FLAG=""
case "$1" in
	"-buddy")
		MM_FLAG="BUDDY"
		;;
	"-bitmap")
		MM_FLAG="BITMAP"
		;;
	*)
		MM_FLAG="BUDDY"
		;;
esac

sudo docker start $NOMBRE
sudo docker exec -it $NOMBRE bash -c "make clean -C /root/Toolchain"
sudo docker exec -it $NOMBRE bash -c "make clean -C /root/"
sudo docker exec -it $NOMBRE bash -c "make MM=$MM_FLAG -C /root/Toolchain"
sudo docker exec -it $NOMBRE bash -c "make MM=$MM_FLAG -C /root/"
#docker stop $NOMBRE