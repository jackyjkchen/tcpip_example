all: common

common:
	make -f makefile.common

posix:
	make -f makefile.posix

linux:
	make -f makefile.linux

bsd:
	make -f makefile.bsd

mingw:
	make -f makefile.mingw

cygwin:
	make -f makefile.cygwin

solaris:
	make -f makefile.solaris

oldbsd:
	make -f makefile.oldbsd

libc5:
	make -f makefile.libc5

libc4:
	make -f makefile.libc4

clean: 
	make -f makefile.common clean
