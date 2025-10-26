#!/bin/bash
NOMBRE="tp_so_g15"
sudo docker start $NOMBRE
sudo docker exec -it $NOMBRE bash -c "make clean -C /root/Toolchain"
sudo docker exec -it $NOMBRE bash -c "make clean -C /root/"
sudo docker exec -it $NOMBRE bash -c "make -C /root/Toolchain"
sudo docker exec -it $NOMBRE bash -c "make -C /root/"
#docker stop $NOMBRE