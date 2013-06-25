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
	$(LIBOBJ)/lib7z \
	$(LIBOBJ)/portmidi \
	$(LIBOBJ)/lua \
	$(LIBOBJ)/librocket \
	$(LIBOBJ)/librocket/Controls \
	$(LIBOBJ)/librocket/Core \
	$(LIBOBJ)/librocket/Debugger \


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
	$(LIBOBJ)/util/plaparse.o \
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
	$(LIBOBJ)/formats/cassimg.o     \
	$(LIBOBJ)/formats/flopimg.o     \
	$(LIBOBJ)/formats/imageutl.o    \
	$(LIBOBJ)/formats/ioprocs.o     \
	$(LIBOBJ)/formats/basicdsk.o    \
	$(LIBOBJ)/formats/a26_cas.o     \
	$(LIBOBJ)/formats/a5105_dsk.o   \
	$(LIBOBJ)/formats/ace_tap.o     \
	$(LIBOBJ)/formats/adam_cas.o    \
	$(LIBOBJ)/formats/adam_dsk.o    \
	$(LIBOBJ)/formats/ami_dsk.o     \
	$(LIBOBJ)/formats/ap2_dsk.o     \
	$(LIBOBJ)/formats/apf_apt.o     \
	$(LIBOBJ)/formats/apridisk.o    \
	$(LIBOBJ)/formats/apollo_dsk.o  \
	$(LIBOBJ)/formats/ap_dsk35.o    \
	$(LIBOBJ)/formats/applix_dsk.o  \
	$(LIBOBJ)/formats/asst128_dsk.o \
	$(LIBOBJ)/formats/atari_dsk.o   \
	$(LIBOBJ)/formats/atarist_dsk.o \
	$(LIBOBJ)/formats/atom_tap.o    \
	$(LIBOBJ)/formats/bml3_dsk.o    \
	$(LIBOBJ)/formats/bw2_dsk.o     \
	$(LIBOBJ)/formats/bw12_dsk.o    \
	$(LIBOBJ)/formats/cbm_tap.o     \
	$(LIBOBJ)/formats/cgen_cas.o    \
	$(LIBOBJ)/formats/coco_cas.o    \
	$(LIBOBJ)/formats/coco_dsk.o    \
	$(LIBOBJ)/formats/comx35_dsk.o  \
	$(LIBOBJ)/formats/coupedsk.o    \
	$(LIBOBJ)/formats/cpis_dsk.o    \
	$(LIBOBJ)/formats/cqm_dsk.o     \
	$(LIBOBJ)/formats/csw_cas.o     \
	$(LIBOBJ)/formats/d64_dsk.o     \
	$(LIBOBJ)/formats/d67_dsk.o     \
	$(LIBOBJ)/formats/d80_dsk.o     \
	$(LIBOBJ)/formats/d81_dsk.o     \
	$(LIBOBJ)/formats/d88_dsk.o     \
	$(LIBOBJ)/formats/dfi_dsk.o     \
	$(LIBOBJ)/formats/dim_dsk.o     \
	$(LIBOBJ)/formats/dsk_dsk.o     \
	$(LIBOBJ)/formats/ep64_dsk.o    \
	$(LIBOBJ)/formats/esq8_dsk.o    \
	$(LIBOBJ)/formats/esq16_dsk.o   \
	$(LIBOBJ)/formats/fdi_dsk.o     \
	$(LIBOBJ)/formats/fm7_cas.o     \
	$(LIBOBJ)/formats/fmsx_cas.o    \
	$(LIBOBJ)/formats/g64_dsk.o     \
	$(LIBOBJ)/formats/gtp_cas.o     \
	$(LIBOBJ)/formats/hect_dsk.o    \
	$(LIBOBJ)/formats/hect_tap.o    \
	$(LIBOBJ)/formats/iq151_dsk.o   \
	$(LIBOBJ)/formats/imd_dsk.o     \
	$(LIBOBJ)/formats/ipf_dsk.o     \
	$(LIBOBJ)/formats/kc_cas.o      \
	$(LIBOBJ)/formats/kc85_dsk.o    \
	$(LIBOBJ)/formats/kim1_cas.o    \
	$(LIBOBJ)/formats/lviv_lvt.o    \
	$(LIBOBJ)/formats/m20_dsk.o     \
	$(LIBOBJ)/formats/m5_dsk.o      \
	$(LIBOBJ)/formats/mm_dsk.o      \
	$(LIBOBJ)/formats/msx_dsk.o     \
	$(LIBOBJ)/formats/mfi_dsk.o     \
	$(LIBOBJ)/formats/mz_cas.o      \
	$(LIBOBJ)/formats/nanos_dsk.o   \
	$(LIBOBJ)/formats/nes_dsk.o     \
	$(LIBOBJ)/formats/orao_cas.o    \
	$(LIBOBJ)/formats/oric_dsk.o    \
	$(LIBOBJ)/formats/oric_tap.o    \
	$(LIBOBJ)/formats/p6001_cas.o   \
	$(LIBOBJ)/formats/pasti_dsk.o   \
	$(LIBOBJ)/formats/pc_dsk.o      \
	$(LIBOBJ)/formats/pc98fdi_dsk.o \
	$(LIBOBJ)/formats/pmd_cas.o     \
	$(LIBOBJ)/formats/primoptp.o    \
	$(LIBOBJ)/formats/pyldin_dsk.o  \
	$(LIBOBJ)/formats/rk_cas.o      \
	$(LIBOBJ)/formats/sc3000_bit.o  \
	$(LIBOBJ)/formats/sf7000_dsk.o  \
	$(LIBOBJ)/formats/smx_dsk.o     \
	$(LIBOBJ)/formats/sorc_dsk.o    \
	$(LIBOBJ)/formats/sord_cas.o    \
	$(LIBOBJ)/formats/st_dsk.o      \
	$(LIBOBJ)/formats/svi_cas.o     \
	$(LIBOBJ)/formats/svi_dsk.o     \
	$(LIBOBJ)/formats/td0_dsk.o     \
	$(LIBOBJ)/formats/thom_cas.o    \
	$(LIBOBJ)/formats/thom_dsk.o    \
	$(LIBOBJ)/formats/ti99_dsk.o    \
	$(LIBOBJ)/formats/tiki100_dsk.o \
	$(LIBOBJ)/formats/trd_dsk.o     \
	$(LIBOBJ)/formats/trs_cas.o     \
	$(LIBOBJ)/formats/trs_dsk.o     \
	$(LIBOBJ)/formats/tvc_cas.o     \
	$(LIBOBJ)/formats/tvc_dsk.o     \
	$(LIBOBJ)/formats/tzx_cas.o     \
	$(LIBOBJ)/formats/uef_cas.o     \
	$(LIBOBJ)/formats/upd765_dsk.o  \
	$(LIBOBJ)/formats/vg5k_cas.o    \
	$(LIBOBJ)/formats/vt_cas.o      \
	$(LIBOBJ)/formats/vt_dsk.o      \
	$(LIBOBJ)/formats/vtech1_dsk.o  \
	$(LIBOBJ)/formats/wavfile.o     \
	$(LIBOBJ)/formats/wd177x_dsk.o  \
	$(LIBOBJ)/formats/x07_cas.o     \
	$(LIBOBJ)/formats/x1_tap.o      \
	$(LIBOBJ)/formats/xdf_dsk.o     \
	$(LIBOBJ)/formats/z80ne_dsk.o   \
	$(LIBOBJ)/formats/zx81_p.o      \
	$(LIBOBJ)/formats/hxcmfm_dsk.o  \

$(OBJ)/libformats.a: $(FORMATSOBJS)



#-------------------------------------------------
# zlib library objects
#-------------------------------------------------

ifdef DEBUG
ZLIBOPTS=-Dverbose=-1
endif

ZLIBOBJS = \
	$(LIBOBJ)/zlib/adler32.o \
	$(LIBOBJ)/zlib/compress.o \
	$(LIBOBJ)/zlib/crc32.o \
	$(LIBOBJ)/zlib/deflate.o \
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
	$(CC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) $(ZLIBOPTS) -c $< -o $@



#-------------------------------------------------
# SoftFloat library objects
#-------------------------------------------------

PROCESSOR_H = $(LIBSRC)/softfloat/processors/mamesf.h
SOFTFLOAT_MACROS = $(LIBSRC)/softfloat/softfloat/bits64/softfloat-macros

SOFTFLOATOBJS = \
	$(LIBOBJ)/softfloat/softfloat.o \
	$(LIBOBJ)/softfloat/fsincos.o \
	$(LIBOBJ)/softfloat/fyl2x.o

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
ARCHFLAGS = -DWORDS_BIGENDIAN=1
else
ARCHFLAGS = -DWORDS_BIGENDIAN=0
endif
else
ARCHFLAGS = -DWORDS_BIGENDIAN=0
endif

FLACOPTS=-DFLAC__NO_ASM -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -DHAVE_CONFIG_H=0 -DFLAC__HAS_OGG=0 -Wno-unused-function $(ARCHFLAGS)

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
	$(CC) $(CDEFS) $(CONLYFLAGS) $(CCOMFLAGS) $(FLACOPTS) -I$(LIBSRC)/libflac/include -c $< -o $@



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
	$(CC) $(CDEFS) $(7ZOPTS) $(CCOMFLAGS) $(CONLYFLAGS) -I$(LIBSRC)/lib7z/ -c $< -o $@

#-------------------------------------------------
# portmidi library objects
#-------------------------------------------------

PMOPTS =

# common objects
LIBPMOBJS = \
	$(LIBOBJ)/portmidi/portmidi.o \
	$(LIBOBJ)/portmidi/porttime.o \
	$(LIBOBJ)/portmidi/pmutil.o

ifeq ($(TARGETOS),linux)
PMOPTS = -DPMALSA=1

LIBPMOBJS += \
	$(LIBOBJ)/portmidi/pmlinux.o \
	$(LIBOBJ)/portmidi/pmlinuxalsa.o \
	$(LIBOBJ)/portmidi/finddefaultlinux.o \
	$(LIBOBJ)/portmidi/ptlinux.o

endif

ifeq ($(TARGETOS),macosx)
LIBPMOBJS += \
	$(LIBOBJ)/portmidi/pmmac.o \
	$(LIBOBJ)/portmidi/pmmacosxcm.o \
	$(LIBOBJ)/portmidi/finddefault.o \
	$(LIBOBJ)/portmidi/readbinaryplist.o \
	$(LIBOBJ)/portmidi/ptmacosx_mach.o \
	$(LIBOBJ)/portmidi/osxsupport.o
endif

ifeq ($(TARGETOS),win32)
LIBPMOBJS += \
	$(LIBOBJ)/portmidi/pmwin.o \
	$(LIBOBJ)/portmidi/pmwinmm.o \
	$(LIBOBJ)/portmidi/ptwinmm.o
endif

$(OBJ)/libportmidi.a: $(LIBPMOBJS)

$(LIBOBJ)/portmidi/%.o: $(LIBSRC)/portmidi/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(PMOPTS) $(CCOMFLAGS) $(CONLYFLAGS) -I$(LIBSRC)/portmidi/ -c $< -o $@

#-------------------------------------------------
# LUA library objects
#-------------------------------------------------

LUAOBJS = \
	$(LIBOBJ)/lua/lapi.o \
	$(LIBOBJ)/lua/lcode.o \
	$(LIBOBJ)/lua/lctype.o \
	$(LIBOBJ)/lua/ldebug.o \
	$(LIBOBJ)/lua/ldo.o \
	$(LIBOBJ)/lua/ldump.o \
	$(LIBOBJ)/lua/lfunc.o \
	$(LIBOBJ)/lua/lgc.o \
	$(LIBOBJ)/lua/llex.o \
	$(LIBOBJ)/lua/lmem.o \
	$(LIBOBJ)/lua/lobject.o \
	$(LIBOBJ)/lua/lopcodes.o \
	$(LIBOBJ)/lua/lparser.o \
	$(LIBOBJ)/lua/lstate.o \
	$(LIBOBJ)/lua/lstring.o \
	$(LIBOBJ)/lua/ltable.o \
	$(LIBOBJ)/lua/ltm.o \
	$(LIBOBJ)/lua/lundump.o \
	$(LIBOBJ)/lua/lvm.o \
	$(LIBOBJ)/lua/lzio.o \
	$(LIBOBJ)/lua/lauxlib.o \
	$(LIBOBJ)/lua/lbaselib.o \
	$(LIBOBJ)/lua/lbitlib.o \
	$(LIBOBJ)/lua/lcorolib.o \
	$(LIBOBJ)/lua/ldblib.o \
	$(LIBOBJ)/lua/liolib.o \
	$(LIBOBJ)/lua/lmathlib.o \
	$(LIBOBJ)/lua/loslib.o \
	$(LIBOBJ)/lua/lstrlib.o \
	$(LIBOBJ)/lua/ltablib.o \
	$(LIBOBJ)/lua/loadlib.o \
	$(LIBOBJ)/lua/linit.o \

$(OBJ)/liblua.a: $(LUAOBJS)

LUA_FLAGS =
ifeq ($(TARGETOS),linux)
LUA_FLAGS += -DLUA_USE_POSIX
endif

ifeq ($(TARGETOS),macosx)
LUA_FLAGS += -DLUA_USE_POSIX
endif

$(LIBOBJ)/lua/%.o: $(LIBSRC)/lua/%.c | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CCOMFLAGS) $(CONLYFLAGS) -DLUA_COMPAT_ALL $(LUA_FLAGS) -c $< -o $@

#-------------------------------------------------
# LUA library objects
#-------------------------------------------------

ROCKETOBJS = \
	$(LIBOBJ)/librocket/Controls/Clipboard.o  \
	$(LIBOBJ)/librocket/Controls/Controls.o  \
	$(LIBOBJ)/librocket/Controls/DataFormatter.o  \
	$(LIBOBJ)/librocket/Controls/DataQuery.o  \
	$(LIBOBJ)/librocket/Controls/DataSource.o  \
	$(LIBOBJ)/librocket/Controls/DataSourceListener.o  \
	$(LIBOBJ)/librocket/Controls/ElementDataGrid.o  \
	$(LIBOBJ)/librocket/Controls/ElementDataGridCell.o  \
	$(LIBOBJ)/librocket/Controls/ElementDataGridExpandButton.o  \
	$(LIBOBJ)/librocket/Controls/ElementDataGridRow.o  \
	$(LIBOBJ)/librocket/Controls/ElementForm.o  \
	$(LIBOBJ)/librocket/Controls/ElementFormControl.o  \
	$(LIBOBJ)/librocket/Controls/ElementFormControlDataSelect.o  \
	$(LIBOBJ)/librocket/Controls/ElementFormControlInput.o  \
	$(LIBOBJ)/librocket/Controls/ElementFormControlSelect.o  \
	$(LIBOBJ)/librocket/Controls/ElementFormControlTextArea.o  \
	$(LIBOBJ)/librocket/Controls/ElementTabSet.o  \
	$(LIBOBJ)/librocket/Controls/ElementTextSelection.o  \
	$(LIBOBJ)/librocket/Controls/InputType.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeButton.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeCheckbox.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeRadio.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeRange.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeSubmit.o  \
	$(LIBOBJ)/librocket/Controls/InputTypeText.o  \
	$(LIBOBJ)/librocket/Controls/SelectOption.o  \
	$(LIBOBJ)/librocket/Controls/WidgetDropDown.o  \
	$(LIBOBJ)/librocket/Controls/WidgetSlider.o  \
	$(LIBOBJ)/librocket/Controls/WidgetSliderInput.o  \
	$(LIBOBJ)/librocket/Controls/WidgetTextInput.o  \
	$(LIBOBJ)/librocket/Controls/WidgetTextInputMultiLine.o  \
	$(LIBOBJ)/librocket/Controls/WidgetTextInputSingleLine.o  \
	$(LIBOBJ)/librocket/Controls/WidgetTextInputSingleLinePassword.o  \
	$(LIBOBJ)/librocket/Controls/XMLNodeHandlerDataGrid.o  \
	$(LIBOBJ)/librocket/Controls/XMLNodeHandlerTabSet.o  \
	$(LIBOBJ)/librocket/Controls/XMLNodeHandlerTextArea.o  \
	$(LIBOBJ)/librocket/Core/BaseXMLParser.o  \
	$(LIBOBJ)/librocket/Core/Box.o  \
	$(LIBOBJ)/librocket/Core/Clock.o  \
	$(LIBOBJ)/librocket/Core/Context.o  \
	$(LIBOBJ)/librocket/Core/ContextInstancer.o  \
	$(LIBOBJ)/librocket/Core/ContextInstancerDefault.o  \
	$(LIBOBJ)/librocket/Core/ConvolutionFilter.o  \
	$(LIBOBJ)/librocket/Core/Core.o  \
	$(LIBOBJ)/librocket/Core/Decorator.o  \
	$(LIBOBJ)/librocket/Core/DecoratorInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorNone.o  \
	$(LIBOBJ)/librocket/Core/DecoratorNoneInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiled.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledBox.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledBoxInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledHorizontal.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledHorizontalInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledImage.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledImageInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledInstancer.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledVertical.o  \
	$(LIBOBJ)/librocket/Core/DecoratorTiledVerticalInstancer.o  \
	$(LIBOBJ)/librocket/Core/Dictionary.o  \
	$(LIBOBJ)/librocket/Core/DocumentHeader.o  \
	$(LIBOBJ)/librocket/Core/Element.o  \
	$(LIBOBJ)/librocket/Core/ElementBackground.o  \
	$(LIBOBJ)/librocket/Core/ElementBorder.o  \
	$(LIBOBJ)/librocket/Core/ElementDecoration.o  \
	$(LIBOBJ)/librocket/Core/ElementDefinition.o  \
	$(LIBOBJ)/librocket/Core/ElementDocument.o  \
	$(LIBOBJ)/librocket/Core/ElementHandle.o  \
	$(LIBOBJ)/librocket/Core/ElementImage.o  \
	$(LIBOBJ)/librocket/Core/ElementInstancer.o  \
	$(LIBOBJ)/librocket/Core/ElementReference.o  \
	$(LIBOBJ)/librocket/Core/ElementScroll.o  \
	$(LIBOBJ)/librocket/Core/ElementStyle.o  \
	$(LIBOBJ)/librocket/Core/ElementText.o  \
	$(LIBOBJ)/librocket/Core/ElementTextDefault.o  \
	$(LIBOBJ)/librocket/Core/ElementUtilities.o  \
	$(LIBOBJ)/librocket/Core/Event.o  \
	$(LIBOBJ)/librocket/Core/EventDispatcher.o  \
	$(LIBOBJ)/librocket/Core/EventInstancer.o  \
	$(LIBOBJ)/librocket/Core/EventInstancerDefault.o  \
	$(LIBOBJ)/librocket/Core/EventListenerInstancer.o  \
	$(LIBOBJ)/librocket/Core/Factory.o  \
	$(LIBOBJ)/librocket/Core/FileInterface.o  \
	$(LIBOBJ)/librocket/Core/FileInterfaceDefault.o  \
	$(LIBOBJ)/librocket/Core/FontDatabase.o  \
	$(LIBOBJ)/librocket/Core/FontEffect.o  \
	$(LIBOBJ)/librocket/Core/FontEffectInstancer.o  \
	$(LIBOBJ)/librocket/Core/FontEffectNone.o  \
	$(LIBOBJ)/librocket/Core/FontEffectNoneInstancer.o  \
	$(LIBOBJ)/librocket/Core/FontEffectOutline.o  \
	$(LIBOBJ)/librocket/Core/FontEffectOutlineInstancer.o  \
	$(LIBOBJ)/librocket/Core/FontEffectShadow.o  \
	$(LIBOBJ)/librocket/Core/FontEffectShadowInstancer.o  \
	$(LIBOBJ)/librocket/Core/FontFace.o  \
	$(LIBOBJ)/librocket/Core/FontFaceHandle.o  \
	$(LIBOBJ)/librocket/Core/FontFaceLayer.o  \
	$(LIBOBJ)/librocket/Core/FontFamily.o  \
	$(LIBOBJ)/librocket/Core/Geometry.o  \
	$(LIBOBJ)/librocket/Core/GeometryDatabase.o  \
	$(LIBOBJ)/librocket/Core/GeometryUtilities.o  \
	$(LIBOBJ)/librocket/Core/LayoutBlockBox.o  \
	$(LIBOBJ)/librocket/Core/LayoutBlockBoxSpace.o  \
	$(LIBOBJ)/librocket/Core/LayoutEngine.o  \
	$(LIBOBJ)/librocket/Core/LayoutInlineBox.o  \
	$(LIBOBJ)/librocket/Core/LayoutInlineBoxText.o  \
	$(LIBOBJ)/librocket/Core/LayoutLineBox.o  \
	$(LIBOBJ)/librocket/Core/Log.o  \
	$(LIBOBJ)/librocket/Core/Math.o  \
	$(LIBOBJ)/librocket/Core/Plugin.o  \
	$(LIBOBJ)/librocket/Core/PluginRegistry.o  \
	$(LIBOBJ)/librocket/Core/precompiled.o  \
	$(LIBOBJ)/librocket/Core/Property.o  \
	$(LIBOBJ)/librocket/Core/PropertyDefinition.o  \
	$(LIBOBJ)/librocket/Core/PropertyDictionary.o  \
	$(LIBOBJ)/librocket/Core/PropertyParserColour.o  \
	$(LIBOBJ)/librocket/Core/PropertyParserKeyword.o  \
	$(LIBOBJ)/librocket/Core/PropertyParserNumber.o  \
	$(LIBOBJ)/librocket/Core/PropertyParserString.o  \
	$(LIBOBJ)/librocket/Core/PropertySpecification.o  \
	$(LIBOBJ)/librocket/Core/ReferenceCountable.o  \
	$(LIBOBJ)/librocket/Core/RenderInterface.o  \
	$(LIBOBJ)/librocket/Core/Stream.o  \
	$(LIBOBJ)/librocket/Core/StreamFile.o  \
	$(LIBOBJ)/librocket/Core/StreamMemory.o  \
	$(LIBOBJ)/librocket/Core/String.o  \
	$(LIBOBJ)/librocket/Core/StringCache.o  \
	$(LIBOBJ)/librocket/Core/StringUtilities.o  \
	$(LIBOBJ)/librocket/Core/StyleSheet.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetFactory.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNode.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelector.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorEmpty.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorFirstChild.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorFirstOfType.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorLastChild.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorLastOfType.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorNthChild.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorNthLastChild.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorNthLastOfType.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorNthOfType.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorOnlyChild.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetNodeSelectorOnlyOfType.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetParser.o  \
	$(LIBOBJ)/librocket/Core/StyleSheetSpecification.o  \
	$(LIBOBJ)/librocket/Core/SystemInterface.o  \
	$(LIBOBJ)/librocket/Core/Template.o  \
	$(LIBOBJ)/librocket/Core/TemplateCache.o  \
	$(LIBOBJ)/librocket/Core/Texture.o  \
	$(LIBOBJ)/librocket/Core/TextureDatabase.o  \
	$(LIBOBJ)/librocket/Core/TextureLayout.o  \
	$(LIBOBJ)/librocket/Core/TextureLayoutRectangle.o  \
	$(LIBOBJ)/librocket/Core/TextureLayoutRow.o  \
	$(LIBOBJ)/librocket/Core/TextureLayoutTexture.o  \
	$(LIBOBJ)/librocket/Core/TextureResource.o  \
	$(LIBOBJ)/librocket/Core/UnicodeRange.o  \
	$(LIBOBJ)/librocket/Core/URL.o  \
	$(LIBOBJ)/librocket/Core/Variant.o  \
	$(LIBOBJ)/librocket/Core/Vector2.o  \
	$(LIBOBJ)/librocket/Core/WidgetSlider.o  \
	$(LIBOBJ)/librocket/Core/WidgetSliderScroll.o  \
	$(LIBOBJ)/librocket/Core/WString.o  \
	$(LIBOBJ)/librocket/Core/XMLNodeHandler.o  \
	$(LIBOBJ)/librocket/Core/XMLNodeHandlerBody.o  \
	$(LIBOBJ)/librocket/Core/XMLNodeHandlerDefault.o  \
	$(LIBOBJ)/librocket/Core/XMLNodeHandlerHead.o  \
	$(LIBOBJ)/librocket/Core/XMLNodeHandlerTemplate.o  \
	$(LIBOBJ)/librocket/Core/XMLParser.o  \
	$(LIBOBJ)/librocket/Core/XMLParseTools.o  \
	$(LIBOBJ)/librocket/Debugger/Debugger.o  \
	$(LIBOBJ)/librocket/Debugger/ElementContextHook.o  \
	$(LIBOBJ)/librocket/Debugger/ElementInfo.o  \
	$(LIBOBJ)/librocket/Debugger/ElementLog.o  \
	$(LIBOBJ)/librocket/Debugger/Geometry.o  \
	$(LIBOBJ)/librocket/Debugger/Plugin.o  \
	$(LIBOBJ)/librocket/Debugger/SystemInterface.o  \


$(OBJ)/librocket.a: $(ROCKETOBJS)

$(LIBOBJ)/librocket/%.o: $(LIBSRC)/librocket/%.cpp | $(OSPREBUILD)
	@echo Compiling $<...
	$(CC) $(CDEFS) $(CFLAGS) -c -I$(LIBSRC)/librocket/Include -DSTATIC_LIB -UINLINE $< -o $@
