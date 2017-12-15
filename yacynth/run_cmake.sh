#/bin/bash

export SRCDIR=`pwd`
export BINDIR=${SRCDIR}_bin

mkdir ${BINDIR}

if [ -d "$BINDIR" ]; then
	cd ${BINDIR}
	ccmake -D CMAKE_C_COMPILER=/usr/bin/gcc-6 -D CMAKE_CXX_COMPILER=/usr/bin/g++-6   ${SRCDIR}
	echo "***  cd ${BINDIR} and run make **** "
else 
	echo "***  can't make ${BINDIR} check access rights ***"
fi
