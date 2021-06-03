#!/bin/bash

# Build application program
./makeclean.sh

source arm-linux-gnueabihf/setval_arm.sh

#编译执行文件及cgi
make -f makefile $PARA $PLAT
# make -f makecgi.mk data_processing

