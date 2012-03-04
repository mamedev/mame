###########################################################################
#
#   lib.mak
#
#   MAME dependent library makefile
#
#   Copyright Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
###########################################################################


LIBSRC = $(SRC)/lib
LIBOBJ = $(OBJ)/lib

OBJDIRS += \
	$(LIBOBJ)/util \
	$(LIBOBJ)/expat \
	$(LIBOBJ)/formats \
	$(LIBOBJ)/zlib \
	$(LIBOBJ)/softfloat \
	$(LIBOBJ)/libjpeg \
	$(LIBOBJ)/libflac \
	$(LIBOBJ)/libflacpp \
	$(LIBOBJ)/lib7z \



#-------------------------------------------------
# utility library objects
#-------------------------------------------------

UTILOBJS = \
	$(LIBOBJ)/util/astring.o \
	$(LIBOBJ)/util/avhuff.o \
	$(LIBOBJ)/util/aviio.o \
	$(LIBOBJ)/util/bitmap.o \
	$(LIBOBJ)/util/cdrom.o \
	$(LIBOBJ)/util/chd.o \
	$(LIBOBJ)/util/chdcd.o \
	$(LIBOBJ)/util/chdcodec.o \
	$(LIBOBJ)/util/corefile.o \
	$(LIBOBJ)/util/corestr.o \
	$(LIBOBJ)/util/coreutil.o \
	$(LIBOBJ)/util/flac.o \
	$(LIBOBJ)/util/harddisk.o \
	$(LIBOBJ)/util/hashing.o \
	$(LIBOBJ)/util/huffman.o \
	$(LIBOBJ)/util/jedparse.o \
	$(LIBOBJ)/util/md5.o \
	$(LIBOBJ)/util/opresolv.o \
	$(LIBOBJ)/util/options.o \
	$(LIBOBJ)/util/palette.o \
	$(LIBOBJ)/util/png.o \
	$(LIBOBJ)/util/pool.o \
	$(LIBOBJ)/util/sha1.o \
	$(LIBOBJ)/util/unicode.o \
	$(LIBOBJ)/util/unzip.o \
	$(LIBOBJ)/util/un7z.o \
	$(LIBOBJ)/util/vbiparse.o \
	$(LIBOBJ)/util/xmlfile.o \
	$(LIBOBJ)/util/zippath.o \

$(OBJ)/libutil.a: $(UTILOBJS)



#-------------------------------------------------
# expat library objects
#-------------------------------------------------

EXPATOBJS = \
	$(LIBOBJ)/expat/xmlparse.o \
	$(LIBOBJ)/expat/xmlrole.o \
	$(LIBOBJ)/expat/xmltok.o

$(OBJ)/libexpat.a: $(EXPATOBJS)

$(LIBOBJ)/expat/%.o: $(LIBSRC)/expat/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) -c $< -o $@



#-------------------------------------------------
# formats library objects
#-------------------------------------------------

FORMATSOBJS = \
	$(LIBOBJ)/formats/cassimg.o 	\
	$(LIBOBJ)/formats/flopimg.o		\
	$(LIBOBJ)/formats/imageutl.o	\
	$(LIBOBJ)/formats/ioprocs.o		\
	$(LIBOBJ)/formats/basicdsk.o	\
	$(LIBOBJ)/formats/a26_cas.o		\
	$(LIBOBJ)/formats/ace_tap.o		\
	$(LIBOBJ)/formats/ami_dsk.o		\
	$(LIBOBJ)/formats/ap2_dsk.o		\
	$(LIBOBJ)/formats/apf_apt.o		\
	$(LIBOBJ)/formats/apridisk.o	\
	$(LIBOBJ)/formats/ap_dsk35.o	\
	$(LIBOBJ)/formats/atari_dsk.o	\
	$(LIBOBJ)/formats/atarist_dsk.o	\
	$(LIBOBJ)/formats/atom_tap.o	\
	$(LIBOBJ)/formats/cbm_tap.o		\
	$(LIBOBJ)/formats/cgen_cas.o	\
	$(LIBOBJ)/formats/coco_cas.o	\
	$(LIBOBJ)/formats/coco_dsk.o	\
	$(LIBOBJ)/formats/comx35_dsk.o	\
	$(LIBOBJ)/formats/coupedsk.o	\
	$(LIBOBJ)/formats/cpis_dsk.o	\
	$(LIBOBJ)/formats/cqm_dsk.o		\
	$(LIBOBJ)/formats/csw_cas.o		\
	$(LIBOBJ)/formats/d64_dsk.o		\
	$(LIBOBJ)/formats/d81_dsk.o		\
	$(LIBOBJ)/formats/d88_dsk.o		\
	$(LIBOBJ)/formats/dfi_dsk.o		\
	$(LIBOBJ)/formats/dim_dsk.o		\
	$(LIBOBJ)/formats/dsk_dsk.o		\
	$(LIBOBJ)/formats/fdi_dsk.o		\
	$(LIBOBJ)/formats/fm7_cas.o		\
	$(LIBOBJ)/formats/fmsx_cas.o	\
	$(LIBOBJ)/formats/g64_dsk.o		\
	$(LIBOBJ)/formats/gtp_cas.o		\
	$(LIBOBJ)/formats/hect_dsk.o	\
	$(LIBOBJ)/formats/hect_tap.o	\
	$(LIBOBJ)/formats/imd_dsk.o		\
	$(LIBOBJ)/formats/ipf_dsk.o		\
	$(LIBOBJ)/formats/kc_cas.o		\
	$(LIBOBJ)/formats/kim1_cas.o	\
	$(LIBOBJ)/formats/lviv_lvt.o	\
	$(LIBOBJ)/formats/msx_dsk.o		\
	$(LIBOBJ)/formats/mfi_dsk.o		\
	$(LIBOBJ)/formats/mz_cas.o		\
	$(LIBOBJ)/formats/nes_dsk.o		\
	$(LIBOBJ)/formats/orao_cas.o	\
	$(LIBOBJ)/formats/oric_dsk.o	\
	$(LIBOBJ)/formats/oric_tap.o	\
	$(LIBOBJ)/formats/p6001_cas.o	\
	$(LIBOBJ)/formats/pasti_dsk.o	\
	$(LIBOBJ)/formats/pc_dsk.o		\
	$(LIBOBJ)/formats/pmd_pmd.o		\
	$(LIBOBJ)/formats/primoptp.o	\
	$(LIBOBJ)/formats/rk_cas.o		\
	$(LIBOBJ)/formats/smx_dsk.o		\
	$(LIBOBJ)/formats/sorc_dsk.o	\
	$(LIBOBJ)/formats/sord_cas.o	\
	$(LIBOBJ)/formats/st_dsk.o		\
	$(LIBOBJ)/formats/svi_cas.o		\
	$(LIBOBJ)/formats/svi_dsk.o		\
	$(LIBOBJ)/formats/td0_dsk.o		\
	$(LIBOBJ)/formats/thom_cas.o	\
	$(LIBOBJ)/formats/thom_dsk.o	\
	$(LIBOBJ)/formats/ti99_dsk.o	\
	$(LIBOBJ)/formats/trd_dsk.o		\
	$(LIBOBJ)/formats/trs_cas.o		\
	$(LIBOBJ)/formats/trs_dsk.o		\
	$(LIBOBJ)/formats/tzx_cas.o		\
	$(LIBOBJ)/formats/uef_cas.o		\
	$(LIBOBJ)/formats/vg5k_cas.o	\
	$(LIBOBJ)/formats/vt_cas.o		\
	$(LIBOBJ)/formats/vt_dsk.o		\
	$(LIBOBJ)/formats/vtech1_dsk.o	\
	$(LIBOBJ)/formats/wavfile.o		\
	$(LIBOBJ)/formats/x1_tap.o		\
	$(LIBOBJ)/formats/z80ne_dsk.o	\
	$(LIBOBJ)/formats/zx81_p.o		\
	$(LIBOBJ)/formats/hxcmfm_dsk.o	\

$(OBJ)/libformats.a: $(FORMATSOBJS)



#-------------------------------------------------
# zlib library objects
#-------------------------------------------------

ZLIBOBJS = \
	$(LIBOBJ)/zlib/adler32.o \
	$(LIBOBJ)/zlib/compress.o \
	$(LIBOBJ)/zlib/crc32.o \
	$(LIBOBJ)/zlib/deflate.o \
	$(LIBOBJ)/zlib/gzio.o \
	$(LIBOBJ)/zlib/inffast.o \
	$(LIBOBJ)/zlib/inflate.o \
	$(LIBOBJ)/zlib/infback.o \
	$(LIBOBJ)/zlib/inftrees.o \
	$(LIBOBJ)/zlib/trees.o \
	$(LIBOBJ)/zlib/uncompr.o \
	$(LIBOBJ)/zlib/zutil.o



$(OBJ)/libz.a: $(ZLIBOBJS)

$(LIBOBJ)/zlib/%.o: $(LIBSRC)/zlib/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) -c $< -o $@



#-------------------------------------------------
# SoftFloat library objects
#-------------------------------------------------

PROCESSOR_H = $(LIBSRC)/softfloat/processors/mamesf.h
SOFTFLOAT_MACROS = $(LIBSRC)/softfloat/softfloat/bits64/softfloat-macros

SOFTFLOATOBJS = \
	$(LIBOBJ)/softfloat/softfloat.o \
    $(LIBOBJ)/softfloat/fsincos.o

$(OBJ)/libsoftfloat.a: $(SOFTFLOATOBJS)

$(LIBOBJ)/softfloat/softfloat.o: $(LIBSRC)/softfloat/softfloat.c $(LIBSRC)/softfloat/softfloat.h $(LIBSRC)/softfloat/softfloat-macros $(LIBSRC)/softfloat/softfloat-specialize
$(LIBOBJ)/softfloat/fsincos.o: $(LIBSRC)/softfloat/fsincos.c $(LIBSRC)/softfloat/fpu_constant.h $(LIBSRC)/softfloat/softfloat.h $(LIBSRC)/softfloat/softfloat-macros $(LIBSRC)/softfloat/softfloat-specialize



#-------------------------------------------------
# libJPEG library objects
#-------------------------------------------------

LIBJPEGOBJS= \
	$(LIBOBJ)/libjpeg/jaricom.o \
	$(LIBOBJ)/libjpeg/jcapimin.o \
	$(LIBOBJ)/libjpeg/jcapistd.o \
	$(LIBOBJ)/libjpeg/jcarith.o \
	$(LIBOBJ)/libjpeg/jccoefct.o \
	$(LIBOBJ)/libjpeg/jccolor.o \
	$(LIBOBJ)/libjpeg/jcdctmgr.o \
	$(LIBOBJ)/libjpeg/jchuff.o \
	$(LIBOBJ)/libjpeg/jcinit.o \
	$(LIBOBJ)/libjpeg/jcmainct.o \
	$(LIBOBJ)/libjpeg/jcmarker.o \
	$(LIBOBJ)/libjpeg/jcmaster.o \
	$(LIBOBJ)/libjpeg/jcomapi.o \
	$(LIBOBJ)/libjpeg/jcparam.o \
	$(LIBOBJ)/libjpeg/jcprepct.o \
	$(LIBOBJ)/libjpeg/jcsample.o \
	$(LIBOBJ)/libjpeg/jctrans.o \
	$(LIBOBJ)/libjpeg/jdapimin.o \
	$(LIBOBJ)/libjpeg/jdapistd.o \
	$(LIBOBJ)/libjpeg/jdarith.o \
	$(LIBOBJ)/libjpeg/jdatadst.o \
	$(LIBOBJ)/libjpeg/jdatasrc.o \
	$(LIBOBJ)/libjpeg/jdcoefct.o \
	$(LIBOBJ)/libjpeg/jdcolor.o \
	$(LIBOBJ)/libjpeg/jddctmgr.o \
	$(LIBOBJ)/libjpeg/jdhuff.o \
	$(LIBOBJ)/libjpeg/jdinput.o \
	$(LIBOBJ)/libjpeg/jdmainct.o \
	$(LIBOBJ)/libjpeg/jdmarker.o \
	$(LIBOBJ)/libjpeg/jdmaster.o \
	$(LIBOBJ)/libjpeg/jdmerge.o \
	$(LIBOBJ)/libjpeg/jdpostct.o \
	$(LIBOBJ)/libjpeg/jdsample.o \
	$(LIBOBJ)/libjpeg/jdtrans.o \
	$(LIBOBJ)/libjpeg/jerror.o \
	$(LIBOBJ)/libjpeg/jfdctflt.o \
	$(LIBOBJ)/libjpeg/jfdctfst.o \
	$(LIBOBJ)/libjpeg/jfdctint.o \
	$(LIBOBJ)/libjpeg/jidctflt.o \
	$(LIBOBJ)/libjpeg/jidctfst.o \
	$(LIBOBJ)/libjpeg/jidctint.o \
	$(LIBOBJ)/libjpeg/jquant1.o \
	$(LIBOBJ)/libjpeg/jquant2.o \
	$(LIBOBJ)/libjpeg/jutils.o \
	$(LIBOBJ)/libjpeg/jmemmgr.o \
	$(LIBOBJ)/libjpeg/jmemansi.o \

$(OBJ)/libjpeg.a: $(LIBJPEGOBJS)

$(LIBOBJ)/libjpeg/%.o: $(LIBSRC)/libjpeg/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) -I$(LIBSRC)/libjpeg -c $< -o $@



#-------------------------------------------------
# libflac library objects
#-------------------------------------------------

ifeq ($(TARGETOS),macosx)
ifdef BIGENDIAN
ifeq ($(PTR64),1)
ARCHFLAGS = -arch ppc64 -DWORDS_BIGENDIAN=1
else
ARCHFLAGS = -arch ppc -DWORDS_BIGENDIAN=1
endif
else	# BIGENDIAN
ifeq ($(PTR64),1)
ARCHFLAGS = -arch x86_64 -DWORDS_BIGENDIAN=0
else
ARCHFLAGS = -m32 -arch i386 -DWORDS_BIGENDIAN=0
endif
endif	# BIGENDIAN
else    # ifeq ($(TARGETOS),macosx) 
ARCHFLAGS = -DWORDS_BIGENDIAN=0
endif   # ifeq ($(TARGETOS),macosx)

FLACOPTS=-DFLAC__NO_ASM -DHAVE_INTTYPES_H -DHAVE_ICONV -DHAVE_LANGINFO_CODESET -DHAVE_SOCKLEN_T -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $(ARCHFLAGS)

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

$(OBJ)/libflac.a: $(LIBFLACOBJS)

$(LIBOBJ)/libflac/%.o: $(LIBSRC)/libflac/libflac/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(FLACOPTS) $(CONLYFLAGS) -I$(LIBSRC)/libflac/include -c $< -o $@



#-------------------------------------------------
# lib7z library objects
#-------------------------------------------------

7ZOPTS=-D_7ZIP_PPMD_SUPPPORT -D_7ZIP_ST

LIB7ZOBJS = \
	$(LIBOBJ)/lib7z/7zBuf.o \
	$(LIBOBJ)/lib7z/7zBuf2.o \
	$(LIBOBJ)/lib7z/7zCrc.o \
	$(LIBOBJ)/lib7z/7zCrcOpt.o \
	$(LIBOBJ)/lib7z/7zDec.o \
	$(LIBOBJ)/lib7z/7zIn.o \
	$(LIBOBJ)/lib7z/CpuArch.o \
	$(LIBOBJ)/lib7z/LzmaDec.o \
	$(LIBOBJ)/lib7z/Lzma2Dec.o \
	$(LIBOBJ)/lib7z/LzmaEnc.o \
	$(LIBOBJ)/lib7z/Lzma2Enc.o \
	$(LIBOBJ)/lib7z/LzFind.o \
	$(LIBOBJ)/lib7z/Bra.o \
	$(LIBOBJ)/lib7z/Bra86.o \
	$(LIBOBJ)/lib7z/Bcj2.o \
	$(LIBOBJ)/lib7z/Ppmd7.o \
	$(LIBOBJ)/lib7z/Ppmd7Dec.o \
	$(LIBOBJ)/lib7z/7zStream.o \

$(OBJ)/lib7z.a: $(LIB7ZOBJS)

$(LIBOBJ)/lib7z/%.o: $(LIBSRC)/lib7z/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(7ZOPTS) $(CONLYFLAGS) -I$(LIBSRC)/lib7z/ -c $< -o $@
