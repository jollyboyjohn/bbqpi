MAKE = make
PREFIX = /usr/local
INSTALL = install -m 755
MFLAGS = 'INSTALL=$(INSTALL)' 'PREFIX=$(PREFIX)'

all:
	@(cd src; $(MAKE) $(MFLAGS) all)

install: 
	@(cd src; $(MAKE) $(MFLAGS) install)

clean: 
	@(cd src; $(MAKE) $(MFLAGS) clean)
