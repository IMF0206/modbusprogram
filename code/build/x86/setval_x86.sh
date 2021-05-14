#!/bin/bash

export PRODUCT_NAME=NVR-H0T0P03AN
export PROJ_DIR=$(pwd)/../../../
export COMM_DIR=$PROJ_DIR/common/
export OPENSRC_DIR=$PROJ_DIR/3rdparty/
export DRIVER_DIR=$PROJ_DIR/driver/
export PRODUCT_DIR=$PROJ_DIR/board/$PRODUCT_NAME
export PRODUCT_SRC_DIR=$PRODUCT_DIR/source/
export RELEASE_DIR=$PRODUCT_DIR/release/
export MAKEFILE_DIR=$PROJ_DIR/makefile/
export COMM_INCLUDE_DIR=$PROJ_DIR/include/
export PRODUCT_APP_NAME=nvr_main
export TOOLCHAIN_DIR=/opt/myir-imx-fb/4.1.15-2.0.1/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/
export TARGET_PLATFORM=IMX6ULL
#if run in linux,#the above line .change to the follow line
#export TARGET_PLATFORM=x86 
export TARGET_OS=linux-gnueabi

export TOOL_CHAIN_PREFIX=arm-poky-$TARGET_OS-
#if run in linux,#the above line

export PATH=$TOOLCHAIN_DIR:$PATH
