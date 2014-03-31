###########################################################################
#
#   floptool.mak
#
#   MESS floptool makefile
#
###########################################################################


# floptool executable name
FLOPTOOL = floptool$(EXE)

# add path to floptool headers
INCPATH += -I$(SRC)/$(TARGET)/tools/floptool

# floptool directories
FLOPTOOLOBJ = $(MESS_TOOLS)/floptool


#-------------------------------------------------
# floptool objects
#-------------------------------------------------

OBJDIRS += \
	$(FLOPTOOLOBJ)

FLOPTOOL_OBJS = \
	$(FLOPTOOLOBJ)/main.o \



#-------------------------------------------------
# rules to build the floptool executable
#-------------------------------------------------

# TODO: Visual Studio wants $(FLAC_LIB) and $(7Z_LIB) during linking...
$(FLOPTOOL): $(FLOPTOOL_OBJS) $(FORMATS_LIB) $(LIBEMU) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE) $(FLAC_LIB) $(7Z_LIB)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
