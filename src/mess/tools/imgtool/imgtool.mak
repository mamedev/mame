###########################################################################
#
#   imgtool.mak
#
#   MESS imgtool makefile
#
###########################################################################


# imgtool executable name
IMGTOOL = imgtool$(EXE)

# add path to imgtool headers
INCPATH += -I$(MESSSRC)/tools/imgtool

# imgtool directories
IMGTOOLOBJ = $(MESS_TOOLS)/imgtool
IMGTOOL_MODULES = $(IMGTOOLOBJ)/modules



#-------------------------------------------------
# imgtool objects
#-------------------------------------------------

OBJDIRS += \
	$(IMGTOOLOBJ) \
	$(IMGTOOL_MODULES)

LIBIMGTOOL = $(OBJ)/libimgtool.a

# imgtool lib objects
IMGTOOL_LIB_OBJS =                  \
	$(IMGTOOLOBJ)/stream.o              \
	$(IMGTOOLOBJ)/library.o             \
	$(IMGTOOLOBJ)/modules.o             \
	$(IMGTOOLOBJ)/iflopimg.o            \
	$(IMGTOOLOBJ)/filter.o              \
	$(IMGTOOLOBJ)/filteoln.o            \
	$(IMGTOOLOBJ)/filtbas.o             \
	$(IMGTOOLOBJ)/imgtool.o             \
	$(IMGTOOLOBJ)/imgterrs.o            \
	$(IMGTOOLOBJ)/imghd.o               \
	$(IMGTOOLOBJ)/charconv.o            \
	$(IMGTOOL_MODULES)/amiga.o          \
	$(IMGTOOL_MODULES)/macbin.o         \
	$(IMGTOOL_MODULES)/rsdos.o          \
	$(IMGTOOL_MODULES)/os9.o            \
	$(IMGTOOL_MODULES)/mac.o            \
	$(IMGTOOL_MODULES)/ti99.o           \
	$(IMGTOOL_MODULES)/ti990hd.o            \
	$(IMGTOOL_MODULES)/concept.o            \
	$(IMGTOOL_MODULES)/fat.o            \
	$(IMGTOOL_MODULES)/pc_flop.o            \
	$(IMGTOOL_MODULES)/pc_hard.o            \
	$(IMGTOOL_MODULES)/prodos.o         \
	$(IMGTOOL_MODULES)/vzdos.o          \
	$(IMGTOOL_MODULES)/thomson.o            \
	$(IMGTOOL_MODULES)/macutil.o            \
	$(IMGTOOL_MODULES)/cybiko.o         \
	$(IMGTOOL_MODULES)/cybikoxt.o       \
	$(IMGTOOL_MODULES)/psion.o      \

$(LIBIMGTOOL): $(IMGTOOL_LIB_OBJS)

IMGTOOL_OBJS = \
	$(IMGTOOLOBJ)/main.o \



#-------------------------------------------------
# rules to build the imgtool executable
#-------------------------------------------------

$(IMGTOOL): $(IMGTOOL_OBJS) $(LIBIMGTOOL) $(FORMATS_LIB) $(LIBEMU) $(LIBUTIL) $(EXPAT) $(ZLIB) $(FLAC_LIB) $(7Z_LIB) $(LIBOCORE)
	@echo Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@
