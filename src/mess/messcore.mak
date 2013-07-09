###########################################################################
#
#   messcore.mak
#
#   MESS emulation core makefile
#
###########################################################################


#-------------------------------------------------
# MESS core defines
#-------------------------------------------------

# add some additional include libraries for the mame files
INCPATH += \
	-I$(SRC)/mame \
	-I$(OBJ)/mame/layout \
	-I$(SRC)/mess/osd \
	-I$(SRC)/mess/osd/$(OSD)


# Root object directories
MAMESRC = $(SRC)/mame
MAMEOBJ = $(OBJ)/mame
MESSSRC = $(SRC)/mess
MESSOBJ = $(OBJ)/mess
EMUSRC = $(SRC)/emu
EMUOBJ = $(OBJ)/emu

# MAME directories
EMU_MACHINE = $(EMUOBJ)/machine
EMU_VIDEO = $(EMUOBJ)/video
MAME_AUDIO = $(MAMEOBJ)/audio
MAME_MACHINE = $(MAMEOBJ)/machine
MAME_DRIVERS = $(MAMEOBJ)/drivers
MAME_VIDEO = $(MAMEOBJ)/video
MAME_LAYOUT = $(MAMEOBJ)/layout

# MESS directories
MESS_AUDIO = $(MESSOBJ)/audio
MESS_DRIVERS = $(MESSOBJ)/drivers
MESS_LAYOUT = $(MESSOBJ)/layout
MESS_MACHINE = $(MESSOBJ)/machine
MESS_VIDEO = $(MESSOBJ)/video

OBJDIRS += \
	$(EMU_MACHINE) \
	$(EMU_VIDEO) \
	$(MAME_AUDIO) \
	$(MAME_DRIVERS) \
	$(MAME_LAYOUT) \
	$(MAME_MACHINE) \
	$(MAME_VIDEO) \
	$(MESS_AUDIO) \
	$(MESS_DRIVERS) \
	$(MESS_LAYOUT) \
	$(MESS_MACHINE) \
	$(MESS_MACHINE)/c64 \
	$(MESS_VIDEO) \

# System-specific directories

OBJDIRS += $(MESS_MACHINE)/ti99 \

#-------------------------------------------------
# MESS core objects
#-------------------------------------------------

LIBOCORE_NOMAIN = $(OBJ)/libocore_nomain.a
