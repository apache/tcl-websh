#
# Makefile for websh3 demo application shop
# nca-073-9
#
# Copyright (C) 2000 by Netcetera AG.
# All rights reserved.
#
# The copyright to the computer program(s) herein is the property of
# Netcetera AG, Switzerland.  The program(s) may be used and/or copied
# only with the written permission of Netcetera AG or in accordance
# with the terms and conditions stipulated in the agreement/contract
# under which the program(s) have been supplied.
#
# @(#) $Id$
#

WEBSH = -s /usr/local/bin/websh3.0b3

INSTALLDIR = ../install

WEBLIB = CVS:nca-109-5/util/version.tcl:1.5 CVS:nca-109-5/util/config.tcl:1.7

shop: shop.ws3
	ncamerge $(WEBSH) -x $@ $(WEBLIB) $<


install: shop
	-mkdir -p $(INSTALLDIR)
	-mkdir -p $(INSTALLDIR)/cgi-bin
	-mkdir -p $(INSTALLDIR)/bin
	-mkdir -p $(INSTALLDIR)/logs
	-mkdir -p $(INSTALLDIR)/etc
	-mkdir -p $(INSTALLDIR)/data
	-mkdir -p $(INSTALLDIR)/data/shopbag
	-mkdir -p $(INSTALLDIR)/data/catalogue
	-mkdir -p $(INSTALLDIR)/data/catalogue/pimages
	-mkdir -p $(INSTALLDIR)/docs
	\cp shop $(INSTALLDIR)/bin
	\cp shop.conf $(INSTALLDIR)/etc
	\cp wrapper $(INSTALLDIR)/bin
	\rm -f $(INSTALLDIR)/cgi-bin/shop
	ln -s ../bin/wrapper $(INSTALLDIR)/cgi-bin/shop
#fixme -- not *.gif ask sh
	\cp ../docs/*.gif $(INSTALLDIR)/docs
	chgrp www $(INSTALLDIR)/logs \
                  $(INSTALLDIR)/data \
		  $(INSTALLDIR)/data/shopbag \
                  $(INSTALLDIR)/data/catalogue \
                  $(INSTALLDIR)/data/catalogue/pimages \
		  $(INSTALLDIR)/data/shopbag
	chmod ug+w $(INSTALLDIR)/logs \
                  $(INSTALLDIR)/data \
		  $(INSTALLDIR)/data/shopbag \
                  $(INSTALLDIR)/data/catalogue \
                  $(INSTALLDIR)/data/catalogue/pimages \
		  $(INSTALLDIR)/data/shopbag

clean: 
	\rm -f shop

mrproper: clean
	\rm -rf $(INSTALLDIR)
