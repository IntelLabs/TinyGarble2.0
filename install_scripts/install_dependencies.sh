#!/bin/bash

userCmd() {
	case "$(id -u)" in
	  0) sudo -u "$SUDO_USER" "$@" ;;
	  *) "$@"
	esac
}

if [ "$(uname)" == "Darwin" ]; then
	echo "--- Intall Dependencies for Mac OS ---"

	userCmd sh -c '
		packages="openssl xctool pkg-config cmake gmp boost"

		brew update

		for pkg in $packages
		do
			brew list "$pkg" | brew install "$pkg"
		done
	'
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
