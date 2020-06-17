
git clone https://github.com/rosariocammarota/emp-tool.git
cd emp-tool
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 
make install -j 
cd ..
