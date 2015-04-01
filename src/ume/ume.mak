###########################################################################
#
#   makefile
#
#   Additional makefile for building UME
#
#   Copyright (c) Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################

GEN_FOLDERS += $(GENDIR)/mame/layout/ $(GENDIR)/mess/layout/
LAYOUTS += $(wildcard $(SRC)/mame/layout/*.lay) $(wildcard $(SRC)/mess/layout/*.lay)
