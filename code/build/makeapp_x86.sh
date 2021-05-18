#!/bin/bash

# Build application program
./makeclean.sh

source x86/setval_x86.sh

#编译执行文件及cgi
make -f makefile $PARA $PLAT
# make -f makecgi.mk data_processing

