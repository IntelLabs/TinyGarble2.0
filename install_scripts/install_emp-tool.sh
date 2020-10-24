#!/bin/bash

git clone https://github.com/IntelLabs/emp-tool.git
cd emp-tool
cmake . -DCMAKE_INSTALL_PREFIX=../include
make install -j 
cd ..
