#!/bin/bash

if [ "$(uname)" == "Darwin" ]; then
	echo "--- Intall Dependencies for Mac OS ---"
	brew update
	brew list openssl || brew install openssl
	brew list xctool || brew install xctool
	brew list pkg-config || brew install pkg-config
	brew list cmake || brew install cmake
	brew list gmp || brew install gmp
	brew list boost || brew install boost
else
	echo "--- Intall Dependencies for Linux Ubuntu ---"
	CC=`lsb_release -rs | cut -c 1-2`
	VER=`expr $CC + 0`
	if [[ $VER -gt 15 ]]; then
		apt-get install -y software-properties-common
		apt-get update
		apt-get install -y cmake git build-essential libssl-dev libgmp-dev python 
		apt-get install -y libboost-dev
		apt-get install -y libboost-{chrono,log,program-options,date-time,thread,system,filesystem,regex,test}-dev
	else
		apt-get install -y software-properties-common
		add-apt-repository -y ppa:george-edison55/cmake-3.x
		add-apt-repository -y ppa:kojoley/boost
		apt-get update
		apt-get install -y cmake git build-essential libssl-dev libgmp-dev python 
		apt-get install -y libboost-all-dev
#		sudo apt-get install -y libboost1.58-dev
#		sudo apt-get install -y libboost-{chrono,log,program-options,date-time,thread,system,filesystem,regex,test}1.58-dev
	fi
fi
