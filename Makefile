
CC = gcc
INSTROOT=/usr/local
BINDIR=$(INSTROOT)/bin
DOCDIR=$(INSTROOT)/doc/gdsreader-0.3.2
MANDIR=$(INSTROOT)/man/man1

#CFLAGS = -I. 
CFLAGS = -I. -Wall -g #-pg
LDFLAGS = #-pg

OBJS = GDSreader.o GDSmain.o GDSaux.o GDSboundary.o GDSpath.o \
       GDSsref.o GDSaref.o GDStext.o GDSnode.o GDSbox.o GDStoHPGL.o \
       GDSstruct.o GDStransf.o GDStoPS.o GDStoPOV.o GDSparser.o

all: gdsreader maptolayer

gdsreader: ${OBJS}
	${CC} ${LDFLAGS} -o gdsreader ${OBJS} -lm

install: gdsreader maptolayer
	mkdir -p $(DOCDIR) && cp -f COPYING README README.povray \
	 layers.config test.gds layers.config.7HP.5ML ChangeLog \
	 layers.config.5AM.4ML $(DOCDIR)
	mkdir -p $(BINDIR) && cp -f maptolayer gdsreader $(BINDIR)
	mkdir -p $(MANDIR) && cp -f maptolayer.1 gdsreader.1 $(MANDIR)

maptolayer: maptolayer.c
	$(CC) -o maptolayer maptolayer.c

clean:
	rm -f *.o *.bak core out result.xg result.txt result.ps gdsreader \
	 maptolayer result.pov



