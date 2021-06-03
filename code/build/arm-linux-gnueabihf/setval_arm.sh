#!/bin/bash

export PROJ_DIR=$(pwd)/../../../
#export COMM_DIR=$PROJ_DIR/common/
export PRODUCT_SRC_DIR=$PROJ_DIR/code/src
export RELEASE_DIR=$PRODUCT_DIR/code/release/
export MAKEFILE_DIR=$PROJ_DIR/code/build/
export INCLUDE_DIR=$PROJ_DIR/code/include/
export PRODUCT_APP_NAME=framework
#export TOOLCHAIN_DIR=/opt/myir-imx-fb/4.1.15-2.0.1/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/
export TARGET_PLATFORM=arm-linux-gnueabihf
#if run in linux,#the above line .change to the follow line
#export TARGET_PLATFORM=x86 
#export TARGET_OS=linux-gnueabi

#export TOOL_CHAIN_PREFIX=arm-poky-$TARGET_OS-
#if run in linux,#the above line

export PATH=$TOOLCHAIN_DIR:$PATH
