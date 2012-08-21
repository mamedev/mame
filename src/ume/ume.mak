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
