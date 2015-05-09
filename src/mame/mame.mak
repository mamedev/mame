###########################################################################
#
#   makefile
#
#   Additional makefile for building MAME
#
###########################################################################

GEN_FOLDERS += $(GENDIR)/mess/layout/
LAYOUTS += $(wildcard $(SRC)/mess/layout/*.lay)
