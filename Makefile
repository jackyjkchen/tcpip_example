all: common

common:
	+make -f makefile.common

posix:
	+make -f makefile.posix

linux:
	+make -f makefile.linux

bsd:
	+make -f makefile.bsd

mingw:
	+make -f makefile.mingw

cygwin:
	+make -f makefile.cygwin

libc5:
	+make -f makefile.libc5

clean: 
	+make -f makefile.common clean
