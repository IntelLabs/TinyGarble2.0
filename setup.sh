cd ..
mkdir -p include

echo "---------- installing dependencies ----------"
sudo ./tinygarble2/install_scripts/install_dependencies.sh

echo echo "---------- installing emp-tool ----------"
./tinygarble2/install_scripts/install_emp-tool.sh

echo echo "---------- installing emp-ot ----------"
./tinygarble2/install_scripts/install_emp-ot.sh

echo echo "---------- building tinygarble2 ----------"
cd tinygarble2
cmake . -DCMAKE_INSTALL_PREFIX=../include
make -j 