all: common

common:
	+make -f Makefile.common

linux:
	+make -f Makefile.linux

bsd:
	+make -f Makefile.bsd

clean: 
	+make -f Makefile.common clean
