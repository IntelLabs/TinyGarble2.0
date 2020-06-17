#!/bin/bash

include_dir=../include
if [ $1 ]; then
	include_dir=$1
fi

cmake  . -DCMAKE_INSTALL_PREFIX=$include_dir 

if [ $? -eq 0 ]; then
  echo "Config is done. Now call '$ make' to compile TinyGarble."
fi
