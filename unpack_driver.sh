#!/bin/bash

if [ ! -d ./drivers/release/build ] 
then
  ARCH=`uname -m`
  if [[  $ARCH == x86_64 ]] 
  then
    tar -xvzf ./drivers/libftd2xx-x86_64-1.3.6.tgz  -C ./drivers 
  fi
  if [[  $ARCH == i386 ]] || [[ $ARCH == i686 ]] 
  then
    tar -xvzf ./drivers/libftd2xx-i386-1.3.6.tgz  -C ./drivers 
  fi
  if [[  $ARCH == *"armv7"* ]] 
  then
    tar -xvzf ./drivers/libftd2xx-arm-v7-hf-1.3.6.tgz  -C ./drivers 
  fi
  if [[  $ARCH == *"armv6"* ]] 
  then
    tar -xvzf ./drivers/libftd2xx-arm-v6-hf-1.3.6.tgz  -C ./drivers 
  fi
  if [[  $ARCH == *"armv5"* ]] 
  then
    tar -xvzf ./drivers/libftd2xx-arm-v5-sf-1.3.6.tgz  -C ./drivers 
  fi
  if [ -d ./drivers/release ]
  then
    ln ./drivers/release/build/libftd2xx.so.1.3.6  ./drivers/release/build/libftd2xx.so
  else
    echo "Could not specify needed drivers. Extract correct drivers manually in drivers directory"
  fi  
fi