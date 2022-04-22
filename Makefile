all: common

common:
	+make -f makefile.common

unix:
	+make -f makefile.unix

linux:
	+make -f makefile.linux

bsd:
	+make -f makefile.bsd

mingw:
	+make -f makefile.mingw

cygwin:
	+make -f makefile.cygwin

oldlibc:
	+make -f makefile.oldlibc

clean: 
	+make -f makefile.common clean
