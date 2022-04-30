#!/bin/bash

cd ..
mkdir -p include

echo "---------- installing dependencies ----------"
if [ "$EUID" -eq 0 ]
  then 
    ./TinyGarble2.0/install_scripts/install_dependencies.sh
  else
    sudo ./TinyGarble2.0/install_scripts/install_dependencies.sh
fi

echo "---------- installing emp-tool ----------"
./TinyGarble2.0/install_scripts/install_emp-tool.sh

echo "---------- installing emp-ot ----------"
./TinyGarble2.0/install_scripts/install_emp-ot.sh

echo "---------- building TinyGarble2.0 ----------"
cd TinyGarble2.0
cmake . -DCMAKE_INSTALL_PREFIX=../include
if [ "$EUID" -eq 0 ]
  then 
    make #having the user as root ususally implies we're inside a docker, which sometimes have issues with "-j"
  else
    make
fi
