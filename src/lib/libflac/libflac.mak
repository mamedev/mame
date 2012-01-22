


DEFINES = -DFLAC__NO_ASM -DFLAC__ALIGN_MALLOC_DATA

INCLUDES = -I./include -I$(LIBSRC)/libflac/include

LIBFLACOBJS = \
	$(LIBOBJ)/libflac/bitmath.o \
	$(LIBOBJ)/libflac/bitreader.o \
	$(LIBOBJ)/libflac/bitwriter.o \
	$(LIBOBJ)/libflac/cpu.o \
	$(LIBOBJ)/libflac/crc.o \
	$(LIBOBJ)/libflac/fixed.o \
	$(LIBOBJ)/libflac/float.o \
	$(LIBOBJ)/libflac/format.o \
	$(LIBOBJ)/libflac/lpc.o \
	$(LIBOBJ)/libflac/md5.o \
	$(LIBOBJ)/libflac/memory.o \
	$(LIBOBJ)/libflac/stream_decoder.o \
	$(LIBOBJ)/libflac/stream_encoder.o \
	$(LIBOBJ)/libflac/stream_encoder_framing.o \
	$(LIBOBJ)/libflac/window.o

VERSION=\"1.2.1\"
CONFIG_CFLAGS=-DHAVE_INTTYPES_H -DHAVE_ICONV  -DFLAC__NO_ASM -D__MINGW32__ -DHAVE_LANGINFO_CODESET -DHAVE_SOCKLEN_T -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBFLACCFLAGS = -O3 -fomit-frame-pointer -funroll-loops -finline-functions -DNDEBUG $(CONFIG_CFLAGS) $(RELEASE_CFLAGS) -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -DFLaC__INLINE=__inline__ -DVERSION=$(VERSION) $(DEFINES) $(INCLUDES)

$(LIBOBJ)/libflac/%.o: $(LIBSRC)/libflac/libflac/%.c 
	@echo Compiling $<...
	$(CC) $(CDEFS) $(LIBFLACCFLAGS) $(CONLYFLAGS) -c $< -o $@


$(OBJ)/libflac.a: $(LIBFLACOBJS)
