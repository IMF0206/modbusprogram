#!/bin/bash

# Build application program
./makeclean.sh

source setval_imx.sh

#编译执行文件及cgi
make -f makeapp.mk app $PARA $PLAT
# make -f makecgi.mk data_processing

