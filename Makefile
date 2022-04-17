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

clean: 
	+make -f makefile.common clean
