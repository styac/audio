#/bin/bash

export SRCDIR=`pwd`
export BINDIR=${SRCDIR}_bin

mkdir ${BINDIR}

if [ -d "$BINDIR" ]; then
	cd ${BINDIR}
	ccmake ${SRCDIR}
	echo "***  cd ${BINDIR} and run make **** "
else 
	echo "***  can't make ${BINDIR} check access rights ***"
fi
