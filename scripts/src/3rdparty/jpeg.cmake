# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## libJPEG library objects
##################################################

add_library(jpeg STATIC EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(jpeg PRIVATE /wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	target_compile_options(jpeg PRIVATE /wd4127) # warning C4127: conditional expression is constant
	target_compile_options(jpeg PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
endif()

target_sources(jpeg PRIVATE
	${MAME_DIR}/3rdparty/libjpeg/jaricom.c
	${MAME_DIR}/3rdparty/libjpeg/jcapimin.c
	${MAME_DIR}/3rdparty/libjpeg/jcapistd.c
	${MAME_DIR}/3rdparty/libjpeg/jcarith.c
	${MAME_DIR}/3rdparty/libjpeg/jccoefct.c
	${MAME_DIR}/3rdparty/libjpeg/jccolor.c
	${MAME_DIR}/3rdparty/libjpeg/jcdctmgr.c
	${MAME_DIR}/3rdparty/libjpeg/jchuff.c
	${MAME_DIR}/3rdparty/libjpeg/jcinit.c
	${MAME_DIR}/3rdparty/libjpeg/jcmainct.c
	${MAME_DIR}/3rdparty/libjpeg/jcmarker.c
	${MAME_DIR}/3rdparty/libjpeg/jcmaster.c
	${MAME_DIR}/3rdparty/libjpeg/jcomapi.c
	${MAME_DIR}/3rdparty/libjpeg/jcparam.c
	${MAME_DIR}/3rdparty/libjpeg/jcprepct.c
	${MAME_DIR}/3rdparty/libjpeg/jcsample.c
	${MAME_DIR}/3rdparty/libjpeg/jctrans.c
	${MAME_DIR}/3rdparty/libjpeg/jdapimin.c
	${MAME_DIR}/3rdparty/libjpeg/jdapistd.c
	${MAME_DIR}/3rdparty/libjpeg/jdarith.c
	${MAME_DIR}/3rdparty/libjpeg/jdatadst.c
	${MAME_DIR}/3rdparty/libjpeg/jdatasrc.c
	${MAME_DIR}/3rdparty/libjpeg/jdcoefct.c
	${MAME_DIR}/3rdparty/libjpeg/jdcolor.c
	${MAME_DIR}/3rdparty/libjpeg/jddctmgr.c
	${MAME_DIR}/3rdparty/libjpeg/jdhuff.c
	${MAME_DIR}/3rdparty/libjpeg/jdinput.c
	${MAME_DIR}/3rdparty/libjpeg/jdmainct.c
	${MAME_DIR}/3rdparty/libjpeg/jdmarker.c
	${MAME_DIR}/3rdparty/libjpeg/jdmaster.c
	${MAME_DIR}/3rdparty/libjpeg/jdmerge.c
	${MAME_DIR}/3rdparty/libjpeg/jdpostct.c
	${MAME_DIR}/3rdparty/libjpeg/jdsample.c
	${MAME_DIR}/3rdparty/libjpeg/jdtrans.c
	${MAME_DIR}/3rdparty/libjpeg/jerror.c
	${MAME_DIR}/3rdparty/libjpeg/jfdctflt.c
	${MAME_DIR}/3rdparty/libjpeg/jfdctfst.c
	${MAME_DIR}/3rdparty/libjpeg/jfdctint.c
	${MAME_DIR}/3rdparty/libjpeg/jidctflt.c
	${MAME_DIR}/3rdparty/libjpeg/jidctfst.c
	${MAME_DIR}/3rdparty/libjpeg/jidctint.c
	${MAME_DIR}/3rdparty/libjpeg/jquant1.c
	${MAME_DIR}/3rdparty/libjpeg/jquant2.c
	${MAME_DIR}/3rdparty/libjpeg/jutils.c
	${MAME_DIR}/3rdparty/libjpeg/jmemmgr.c
	${MAME_DIR}/3rdparty/libjpeg/jmemansi.c
)
