cd ..
mkdir -p include

echo "---------- installing dependencies ----------"
sudo ./TinyGarble2.0/install_scripts/install_dependencies.sh

echo echo "---------- installing emp-tool ----------"
./TinyGarble2.0/install_scripts/install_emp-tool.sh

echo echo "---------- installing emp-ot ----------"
./TinyGarble2.0/install_scripts/install_emp-ot.sh

echo echo "---------- building TinyGarble2.0 ----------"
cd TinyGarble2.0
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 
