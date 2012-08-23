###########################################################################
#
#   ume.mak
#
#   Universal target makefile
#
###########################################################################
CFLAGS += \
	-I$(SRC)/ume \
	-I$(SRC)/mame \
	-I$(OBJ)/mame/layout \
	-I$(SRC)/mess \
	-I$(OBJ)/mess/layout \
	-I$(SRC)/mess/osd \
	-I$(SRC)/mess/osd/$(OSD)

OBJDIRS += $(OBJ)/ume

$(DRIVLISTSRC): $(SRC)/mame/mame.lst $(SRC)/mess/mess.lst

include $(SRC)/mame/mame.mak
include $(SRC)/mess/mess.mak

depend_ume: maketree $(MAKEDEP_TARGET)
	@echo Rebuilding depend.mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... src/mame > depend_mame.mak
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... src/mess > depend_mess.mak
	@echo -include depend_mame.mak > depend_ume.mak
	@echo -include depend_mess.mak >> depend_ume.mak
