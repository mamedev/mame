###########################################################################
#
#   osdmini.mak
#
#   Minimal OSD makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(OBJ)/$(MAMEOS)/minidir.o \
	$(OBJ)/$(MAMEOS)/minifile.o \
	$(OBJ)/$(MAMEOS)/minimisc.o \
	$(OBJ)/$(MAMEOS)/minisync.o \
	$(OBJ)/$(MAMEOS)/minitime.o \
	$(OBJ)/$(MAMEOS)/miniwork.o \
