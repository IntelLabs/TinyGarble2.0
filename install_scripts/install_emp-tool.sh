#!/bin/bash

git clone https://github.com/siamumar/emp-tool-1.git
cd emp-tool
cmake . -DCMAKE_INSTALL_PREFIX=../include
make install -j 
cd ..
