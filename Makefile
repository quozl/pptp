VERSION = 1.2.0-rc1

#################################################################
# CHANGE THIS LINE to point to the location of your pppd binary.
PPPD = /usr/sbin/pppd
#################################################################

CC	= gcc
RM	= rm -f
DEBUG	= -g
INCLUDE =
CFLAGS  = -Wall -O0 $(DEBUG) $(INCLUDE)
LIBS	=
LDFLAGS	= -lutil

PPTP_BIN = pptp

PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o \
            pptp_ctrl.o dirutil.o vector.o \
            inststr.o util.o version.o \
	    pptp_quirks.o orckit_quirks.o pqueue.o

PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h util.h \
	    pptp_quirks.h orckit_quirks.h config.h pqueue.h

all: config.h $(PPTP_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)

config.h: 
	echo "/* text added by Makefile target config.h */" > config.h
	echo "#define PPTP_LINUX_VERSION \"$(VERSION)\"" >> config.h
	echo "#define PPPD_BINARY \"$(PPPD)\"" >> config.h

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o

clean:
	$(RM) *.o config.h

clobber: clean
	$(RM) $(PPTP_BIN) vector_test

distclean: clobber

test: vector_test

dist: clobber
	$(RM) pptp-linux-$(VERSION).tar.gz
	$(RM) -r pptp-linux-$(VERSION)
	mkdir pptp-linux-$(VERSION)
	cp --recursive ChangeLog Makefile *.c *.h pptp.8 Documentation \
Reference AUTHORS COPYING INSTALL NEWS README DEVELOPERS TAGS TODO USING \
	pptp-linux-$(VERSION)/
	tar czf pptp-linux-$(VERSION).tar.gz pptp-linux-$(VERSION)
	$(RM) -r pptp-linux-$(VERSION)
	md5sum pptp-linux-$(VERSION).tar.gz
