#!/bin/bash

git clone https://github.com/IntelLabs/emp-ot.git
cd emp-ot
cmake . -DCMAKE_INSTALL_PREFIX=../include
make install -j 
cd ..
