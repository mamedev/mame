


DEFINES = -DFLAC__NO_ASM -DFLAC__ALIGN_MALLOC_DATA

INCLUDES = -I./include -I$(LIBSRC)/libflac/include

LIBFLACPPOBJS = \
	$(LIBOBJ)/libflacpp/metadata.o \
	$(LIBOBJ)/libflacpp/stream_decoder.o \
	$(LIBOBJ)/libflacpp/stream_encoder.o


VERSION=\"1.2.1\"
CONFIG_CFLAGS=-DHAVE_INTTYPES_H -DHAVE_ICONV  -DFLAC__NO_ASM -D__MINGW32__ -DHAVE_LANGINFO_CODESET -DHAVE_SOCKLEN_T -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBFLACPPCFLAGS = -O3 -fomit-frame-pointer -funroll-loops -finline-functions -DNDEBUG $(CONFIG_CFLAGS) $(RELEASE_CFLAGS) -DFLaC__INLINE=__inline__ -DVERSION=$(VERSION) $(DEFINES) $(INCLUDES)

$(LIBOBJ)/libflacpp/%.o: $(LIBSRC)/libflac/libflac++/%.cpp 
	@echo Compiling $<...
	$(CC) $(CDEFS) $(LIBFLACPPCFLAGS) $(CPPONLYFLAGS) -c $< -o $@


$(OBJ)/libflac++.a: $(LIBFLACPPOBJS)
