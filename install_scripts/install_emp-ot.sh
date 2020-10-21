#!/bin/bash

git clone https://github.com/IntelLabs/emp-ot.git
cd emp-ot
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 
make install -j 
cd ..
