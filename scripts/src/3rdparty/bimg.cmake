# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## BIMG library objects
##################################################

add_library(bimg STATIC EXCLUDE_FROM_ALL)

target_link_libraries(bimg PUBLIC bx)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(bimg PRIVATE -Wno-unused-const-variable)
endif()

target_include_directories(bimg
	PUBLIC
		${MAME_DIR}/3rdparty/bimg/include
	PRIVATE
		${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec
		${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/include
)

target_sources(bimg PRIVATE
	${MAME_DIR}/3rdparty/bimg/src/image.cpp
	${MAME_DIR}/3rdparty/bimg/src/image_gnf.cpp

	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/astc_file.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/codec.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/endpoint_codec.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/footprint.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/integer_sequence_codec.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/intermediate_astc_block.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/logical_astc_block.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/partition.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/physical_astc_block.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/quantization.cc
	${MAME_DIR}/3rdparty/bimg/3rdparty/astc-codec/src/decoder/weight_infill.cc
)
