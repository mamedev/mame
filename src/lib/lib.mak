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
	$(LIBOBJ)/cothread \



#-------------------------------------------------
# utility library objects
#-------------------------------------------------

UTILOBJS = \
	$(LIBOBJ)/util/astring.o \
	$(LIBOBJ)/util/avcomp.o \
	$(LIBOBJ)/util/aviio.o \
	$(LIBOBJ)/util/bitmap.o \
	$(LIBOBJ)/util/cdrom.o \
	$(LIBOBJ)/util/chd.o \
	$(LIBOBJ)/util/chdcd.o \
	$(LIBOBJ)/util/corefile.o \
	$(LIBOBJ)/util/corestr.o \
	$(LIBOBJ)/util/coreutil.o \
	$(LIBOBJ)/util/harddisk.o \
	$(LIBOBJ)/util/huffman.o \
	$(LIBOBJ)/util/jedparse.o \
	$(LIBOBJ)/util/md5.o \
	$(LIBOBJ)/util/opresolv.o \
	$(LIBOBJ)/util/options.o \
	$(LIBOBJ)/util/palette.o \
	$(LIBOBJ)/util/png.o \
	$(LIBOBJ)/util/pool.o \
	$(LIBOBJ)/util/sha1.o \
	$(LIBOBJ)/util/tagmap.o \
	$(LIBOBJ)/util/unicode.o \
	$(LIBOBJ)/util/unzip.o \
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

$(LIBOBJ)/expat/%.o: $(LIBSRC)/explat/%.c | $(OSPREBUILD)
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
# cothread library objects
#-------------------------------------------------

COTHREADOBJS = \
	$(LIBOBJ)/cothread/libco.o

$(OBJ)/libco.a: $(COTHREADOBJS)

$(LIBOBJ)/cothread/%.o: $(LIBSRC)/cothread/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CCOMFLAGS) -c -fomit-frame-pointer $< -o $@
