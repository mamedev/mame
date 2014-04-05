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

UMEOBJ = $(OBJ)/ume
OBJDIRS += $(UMEOBJS)

$(DRIVLISTSRC): $(SRC)/mame/mame.lst $(SRC)/mess/mess.lst

include $(SRC)/mame/mame.mak
include $(SRC)/mess/mess.mak

depend: maketree $(MAKEDEP_TARGET)
	@echo Rebuilding depend_$(TARGET).mak...
	$(MAKEDEP) -I. $(INCPATH) -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... $(SRC)/emu > depend_emu.mak
	$(MAKEDEP) -I. $(INCPATH) -I$(SRC)/mame -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... $(SRC)/mame > depend_mame.mak
	$(MAKEDEP) -I. $(INCPATH) -I$(SRC)/mess -X$(SRC)/emu -X$(SRC)/osd/... -X$(OBJ)/... $(SRC)/mess > depend_mess.mak

	@echo -include depend_emu.mak > depend_ume.mak
	@echo -include depend_mame.mak >> depend_ume.mak
	@echo -include depend_mess.mak >> depend_ume.mak
