###########################################################################
#
#   makefile
#
#   Additional makefile for building UME
#
###########################################################################

GEN_FOLDERS += $(GENDIR)/mame/layout/ $(GENDIR)/mess/layout/
LAYOUTS += $(wildcard $(SRC)/mame/layout/*.lay) $(wildcard $(SRC)/mess/layout/*.lay)
