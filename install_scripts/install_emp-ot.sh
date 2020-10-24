#!/bin/bash

git clone https://github.com/siamumar/emp-ot-1.git
cd emp-ot
cmake . -DCMAKE_INSTALL_PREFIX=../include
make install -j 
cd ..
