# $Id: Makefile,v 1.27 2003/06/11 08:17:30 quozl Exp $
VERSION=1.3.1
RELEASE=

#################################################################
# CHANGE THIS LINE to point to the location of your pppd binary.
PPPD = /usr/sbin/pppd
#################################################################

BINDIR=$(DESTDIR)/usr/sbin
MANDIR=$(DESTDIR)/usr/share/man/man8

CC	= gcc
RM	= rm -f
OPTIMIZE= -O0
DEBUG	= -g
INCLUDE =
CFLAGS  = -Wall $(OPTIMIZE) $(DEBUG) $(INCLUDE)
LIBS	= -lutil
LDFLAGS	=

PPTP_BIN = pptp

PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o \
            pptp_ctrl.o dirutil.o vector.o \
            inststr.o util.o version.o \
	    pptp_quirks.o orckit_quirks.o pqueue.o pptp_callmgr.o

PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h util.h \
	    pptp_quirks.h orckit_quirks.h config.h pqueue.h

all: config.h $(PPTP_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)

config.h: 
	echo "/* text added by Makefile target config.h */" > config.h
	echo "#define PPTP_LINUX_VERSION \"$(VERSION)$(RELEASE)\"" >> config.h
	echo "#define PPPD_BINARY \"$(PPPD)\"" >> config.h

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o

clean:
	$(RM) *.o config.h

clobber: clean
	$(RM) $(PPTP_BIN) vector_test

distclean: clobber

test: vector_test

install:
	mkdir -p $(BINDIR)
	install -o root -m 555 pptp $(BINDIR)
	mkdir -p $(MANDIR)
	install -m 644 pptp.8 $(MANDIR)

dist: clobber
	$(RM) pptp-linux-$(VERSION)$(RELEASE).tar.gz
	$(RM) -r pptp-linux-$(VERSION)
	mkdir pptp-linux-$(VERSION)
	cp --recursive ChangeLog Makefile *.c *.h pptp.8 Documentation \
Reference AUTHORS COPYING INSTALL NEWS README DEVELOPERS TODO USING \
	pptp-linux-$(VERSION)/
	rm -rf pptp-linux-$(VERSION)/CVS pptp-linux-$(VERSION)/*/CVS
	tar czf pptp-linux-$(VERSION)$(RELEASE).tar.gz pptp-linux-$(VERSION)
	$(RM) -r pptp-linux-$(VERSION)
	md5sum pptp-linux-$(VERSION)$(RELEASE).tar.gz
