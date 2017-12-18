#/bin/bash

export SRCDIR=`pwd`
export BINDIR=${SRCDIR}_build
export GENERATOR=" "

if [ "${1,,}" == "ninja" ]
then 
	GENERATOR="-G Ninja "
	echo " ** generate Ninja files"
else
	echo " ** generate make files"
fi

mkdir ${BINDIR}

if [ -d "$BINDIR" ]; then
	cd ${BINDIR}
	cmake -D CMAKE_C_COMPILER=/usr/bin/gcc-6 -D CMAKE_CXX_COMPILER=/usr/bin/g++-6 ${GENERATOR}  ${SRCDIR}
	echo "***  cd ${BINDIR} and run make **** "
else 
	echo "***  can't make ${BINDIR} check access rights ***"
fi
