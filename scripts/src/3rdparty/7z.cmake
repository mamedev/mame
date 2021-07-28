# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## lib7z library objects
##################################################

add_library(7z STATIC EXCLUDE_FROM_ALL)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(7z PRIVATE -Wno-strict-prototypes)
	target_compile_options(7z PRIVATE -Wno-undef)
endif()

if((CMAKE_CXX_COMPILER_ID MATCHES "Clang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10))
	target_compile_options(7z PRIVATE -Wno-misleading-indentation)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(7z PRIVATE /wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	target_compile_options(7z PRIVATE /wd4456) # warning C4456: declaration of 'xxx' hides previous local declaration
	target_compile_options(7z PRIVATE /wd4457) # warning C4457: declaration of 'xxx' hides function parameter
endif()

target_compile_definitions(7z PRIVATE
	_7ZIP_PPMD_SUPPPORT
	_7ZIP_ST
)

target_sources(7z PRIVATE
	${MAME_DIR}/3rdparty/lzma/C/7zAlloc.c
	${MAME_DIR}/3rdparty/lzma/C/7zArcIn.c
	${MAME_DIR}/3rdparty/lzma/C/7zBuf.c
	${MAME_DIR}/3rdparty/lzma/C/7zBuf2.c
	${MAME_DIR}/3rdparty/lzma/C/7zCrc.c
	${MAME_DIR}/3rdparty/lzma/C/7zCrcOpt.c
	${MAME_DIR}/3rdparty/lzma/C/7zDec.c
	${MAME_DIR}/3rdparty/lzma/C/7zFile.c
	${MAME_DIR}/3rdparty/lzma/C/7zStream.c
	${MAME_DIR}/3rdparty/lzma/C/Aes.c
	${MAME_DIR}/3rdparty/lzma/C/AesOpt.c
	${MAME_DIR}/3rdparty/lzma/C/Alloc.c
	${MAME_DIR}/3rdparty/lzma/C/Bcj2.c
	# ${MAME_DIR}/3rdparty/lzma/C/Bcj2Enc.c
	${MAME_DIR}/3rdparty/lzma/C/Bra.c
	${MAME_DIR}/3rdparty/lzma/C/Bra86.c
	${MAME_DIR}/3rdparty/lzma/C/BraIA64.c
	${MAME_DIR}/3rdparty/lzma/C/CpuArch.c
	${MAME_DIR}/3rdparty/lzma/C/Delta.c
	# ${MAME_DIR}/3rdparty/lzma/C/DllSecur.c
	${MAME_DIR}/3rdparty/lzma/C/LzFind.c
	# ${MAME_DIR}/3rdparty/lzma/C/LzFindMt.c
	${MAME_DIR}/3rdparty/lzma/C/Lzma2Dec.c
	${MAME_DIR}/3rdparty/lzma/C/Lzma2Enc.c
	${MAME_DIR}/3rdparty/lzma/C/Lzma86Dec.c
	${MAME_DIR}/3rdparty/lzma/C/Lzma86Enc.c
	${MAME_DIR}/3rdparty/lzma/C/LzmaDec.c
	${MAME_DIR}/3rdparty/lzma/C/LzmaEnc.c
	# ${MAME_DIR}/3rdparty/lzma/C/LzmaLib.c
	# ${MAME_DIR}/3rdparty/lzma/C/MtCoder.c
	${MAME_DIR}/3rdparty/lzma/C/Ppmd7.c
	${MAME_DIR}/3rdparty/lzma/C/Ppmd7Dec.c
	${MAME_DIR}/3rdparty/lzma/C/Ppmd7Enc.c
	${MAME_DIR}/3rdparty/lzma/C/Sha256.c
	${MAME_DIR}/3rdparty/lzma/C/Sort.c
	# ${MAME_DIR}/3rdparty/lzma/C/Threads.c
	# ${MAME_DIR}/3rdparty/lzma/C/Xz.c
	# ${MAME_DIR}/3rdparty/lzma/C/XzCrc64.c
	# ${MAME_DIR}/3rdparty/lzma/C/XzCrc64Opt.c
	# ${MAME_DIR}/3rdparty/lzma/C/XzDec.c
	# ${MAME_DIR}/3rdparty/lzma/C/XzEnc.c
	# ${MAME_DIR}/3rdparty/lzma/C/XzIn.c
)
