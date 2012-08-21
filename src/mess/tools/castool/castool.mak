###########################################################################
#
#   castool.mak
#
#   MESS castool makefile
#
###########################################################################


# castool executable name
CASTOOL = castool$(EXE)

# add path to castool headers
INCPATH += -I$(SRC)/$(TARGET)/tools/castool

# castool directories
CASTOOLOBJ = $(MESS_TOOLS)/castool


#-------------------------------------------------
# castool objects
#-------------------------------------------------

OBJDIRS += \
	$(CASTOOLOBJ)

CASTOOL_OBJS = \
	$(CASTOOLOBJ)/main.o \



#-------------------------------------------------
# rules to build the castool executable
#-------------------------------------------------

$(CASTOOL): $(CASTOOL_OBJS) $(FORMATS_LIB) $(LIBUTIL) $(EXPAT) $(ZLIB) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
