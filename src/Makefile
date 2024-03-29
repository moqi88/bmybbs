#################################################################
# Makefile for FIREBIRD BBS v3.0K                               #
#################################################################
# 1. This package is derived from Firebird BBS 1.5, which was 
#    developed by the Department of Computer Science & Information
#    Engineering, Chung Chen University of Taiwan, ROC.
#    This package is developed by the Board of System Administrators,
#    Malaysian Students' Association In Taiwan.
#
# 2. Our BBS location is at MSIA.pine.ncu.edu.tw (140.115.210.77)
#
# 3. The developers of this package can be reached by
#    SYSOP.bbs@MSIA.pine.ncu.edu.tw
#
# 4. This Makefile is for "LINUX"

#############################################################
####   Setup the following site-specific information     ####
#############################################################

BASEPATH = ..
include $(BASEPATH)/Makefile.Base

#############################################################
###        You needn't modify the following stuff      ######
#############################################################
PROGNAME = bbs
CFILE    = bbs.c record.c delete.c main.c stuff.c mail.c register.c \
           xyz.c talk.c read.c comm_lists.c namecomplete.c chat.c \
           maintain.c edit.c more.c help.c bcache.c boards.c \
           smth_screen.c io.c term.c userinfo.c vote.c announce.c sendmsg.c\
	   bm.c list.c goodbye.c bbsgopher.c fileshm.c postheader.c \
	   convcode.c five.c tmachine.c addressbook.c\
	   backnumber.c common.c sec_hand.c regular.c 1984.c\
	   moneycenter.c editboard.c power_select.c tmpl.c logger.c identify.c

COBJS    :=$(CFILE:.c=.o)

#COBJSTEST:=$(CFILE:.c=.t)

OTHERS   = 

GARBAGE  = a.out core installchatd bbs.chatd bbsd thread

# The -DINVISIBLE makes bbs users invisible to 'finger' and 'who'.
# Simply remove it if you don't want this feature.

#SITE_DEF = -DBBSHOME='"$(BBSHOME)"' -DBBSUID=$(BBSUID) -DBBSGID=$(BBSGRP)

#DEFINES	=  -DBBSMAIN $(CSIE_DEF) $(OS_DEF)

.SUFFIXES: .o .c .ln .t

.c.o:   ;   $(CC) $(CFLAGS) -include proto.h -c $*.c

#.c.t:   ;   $(CC) -D_GNU_SOURCE $(CFLAGSTEST) $(CPPFLAGS) $(DEFINES) -DBACK_DELETE_RANGE -include proto.h -c $*.c -o $*.t

#--------------------- Dependency starts here -------------------
all: $(PROGNAME) bbs.chatd bbsd telnet thread 

ifeq (.depend,$(wildcard .depend))
include .depend
endif

#Makefile: Makefile.in $(BASEPATH)/configure
#	@echo Makefile is older than Makefile.in or $(BASEPATH)/configure, run $(BASEPATH)/configure first.
#	@false

depend dep .depend:
	gcc $(CFLAGS) -MM *.c >.depend
	#gcc $(CFLAGSTEST) -M *.c |sed -es/"o:"/"t:"/ >>.depend

proto proto.h:
	echo '#include "bbs.h"' > proto.h
	cproto $(CFILE) -I $(BASEPATH)/include -I $(BASEPATH)/ythtlib -I $(BASEPATH)/libythtbbs >> proto.h

$(COBJS) $(COBJSTEST): .depend proto.h

$(PROGNAME): $(COBJS) $(OTHERS)
	$(CC) $(COBJS) $(LIBS) $(BBSLIB) $(LDFLAGS) -o $(PROGNAME)

#bbstest: $(COBJSTEST) $(OTHERS)
#	$(CC) -o bbstest $(CFLAGSTEST) $(COBJSTEST) $(LIBSTEST)  $(BBSLIB)

bbsd: bbsd.c
	$(CC) $(CFLAGS) -o bbsd bbsd.c $(LDFLAGS) $(BBSLIB)

bbsnet: bbsnet.c
	$(CC) bbsnet.c -o bbsnet

telnet: telnet0.c
	$(CC) -g telnet0.c -o telnet

bbs.chatd: station.c
	$(CC) $(CFLAGS) -o bbs.chatd station.c

thread: thread.c record.c
	$(CC) $(CFLAGS) -include $(BASEPATH)/ythtlib/ythtlib.h -include $(BASEPATH)/libythtbbs/ythtbbs.h -DEXT_UTL -o thread record.c thread.c -L$(BBSHOME)/bin -lytht -lythtbbs

install: all
	#( cd $(BASEPATH); ./Install.sh)
	$(MAKE) installbbs
	$(MAKE) installbbsd
	$(MAKE) installthread
	$(MAKE) installchatd
	$(MAKE) installbbsnet
	$(MAKE) installtelnet

installbbs: $(PROGNAME) $(BBSHOME)/bin/bbs
$(BBSHOME)/bin/bbs: bbs
	cp bbs $@.r
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) bbs $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installbbstest: bbstest $(BBSHOME)/bin/bbstest
$(BBSHOME)/bin/bbstest: bbstest
	cp bbstest $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installthread: thread $(BBSHOME)/bin/thread
$(BBSHOME)/bin/thread: thread
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) thread $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installchatd: bbs.chatd $(BBSHOME)/bin/chatd
$(BBSHOME)/bin/chatd: bbs.chatd
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) bbs.chatd $@.new
	-rm -f $@.old
	-mv $@ $@.old
	mv $@.new $@

installbbsd: bbsd
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) bbsd $(BBSHOME)/bin/bbsd

installbbsnet: bbsnet
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) bbsnet $(BBSHOME)/bin/bbsnet.new
	-rm -f $(BBSHOME)/bin/bbsnet.old
	-mv $(BBSHOME)/bin/bbsnet $(BBSHOME)/bin/bbsnet.old
	mv $(BBSHOME)/bin/bbsnet.new $(BBSHOME)/bin/bbsnet

installtelnet: telnet
	$(INSTALL) -s -m 550 -g $(BBSGRP) -o $(BBSUID) telnet $(BBSHOME)/bin/telnet.new
	-rm -f $(BBSHOME)/bin/telnet.old
	-mv $(BBSHOME)/bin/telnet $(BBSHOME)/bin/telnet.old
	mv $(BBSHOME)/bin/telnet.new $(BBSHOME)/bin/telnet

clean: 
	rm -fr *.o *.BAK *.log
#	rm -fr ../lib/*.o ../lib/*.BAK ../lib/*.log
#	rm -rf core 'find .. type f -name 'core' -print'
#	rm -f `find .. -name '*.[oas]' ! -regex '.*lxdialog/.*' -print`
	-rm -fr $(GARBAGE) $(COBJS) $(COBJSTEST) $(PROGNAME) $(LNFILES) bbsd bbs.chatd thread pty/ptyexec
	-rm -fr core .depend

dist_clean: clean
	-rm -f CONFIG/config.log
	-rm -f CONFIG/config.status
	-rm -f CONFIG/config.cache
	-rm -f Install.sh

testregular:testregular.c regular.c
	gcc -o $@ -g $^ -I../include

tags: /tmp
	ctags $(CFILE)
