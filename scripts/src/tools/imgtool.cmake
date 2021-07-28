# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
## imgtool
##########################################################################

add_executable(imgtool)

target_include_directories(imgtool PRIVATE
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib
	${MAME_DIR}/src/lib/util
	${MAME_DIR}/src/tools/imgtool
	${EXT_INCLUDEDIR_ZLIB}
)

target_link_libraries(imgtool PRIVATE ocore_${OSD} formats)

target_sources(imgtool PRIVATE
	${MAME_DIR}/src/tools/imgtool/main.cpp
	${MAME_DIR}/src/tools/imgtool/main.h
	${MAME_DIR}/src/tools/imgtool/stream.cpp
	${MAME_DIR}/src/tools/imgtool/stream.h
	${MAME_DIR}/src/tools/imgtool/library.cpp
	${MAME_DIR}/src/tools/imgtool/library.h
	${MAME_DIR}/src/tools/imgtool/modules.cpp
	${MAME_DIR}/src/tools/imgtool/modules.h
	${MAME_DIR}/src/tools/imgtool/iflopimg.cpp
	${MAME_DIR}/src/tools/imgtool/iflopimg.h
	${MAME_DIR}/src/tools/imgtool/filter.cpp
	${MAME_DIR}/src/tools/imgtool/filter.h
	${MAME_DIR}/src/tools/imgtool/filteoln.cpp
	${MAME_DIR}/src/tools/imgtool/filtbas.cpp
	${MAME_DIR}/src/tools/imgtool/imgtool.cpp
	${MAME_DIR}/src/tools/imgtool/imgtool.h
	${MAME_DIR}/src/tools/imgtool/imgterrs.cpp
	${MAME_DIR}/src/tools/imgtool/imgterrs.h
	${MAME_DIR}/src/tools/imgtool/imghd.cpp
	${MAME_DIR}/src/tools/imgtool/imghd.h
	${MAME_DIR}/src/tools/imgtool/charconv.cpp
	${MAME_DIR}/src/tools/imgtool/charconv.h
	${MAME_DIR}/src/tools/imgtool/formats/vt_dsk_legacy.cpp
	${MAME_DIR}/src/tools/imgtool/formats/vt_dsk_legacy.h
	${MAME_DIR}/src/tools/imgtool/formats/coco_dsk.cpp
	${MAME_DIR}/src/tools/imgtool/formats/coco_dsk.h
	${MAME_DIR}/src/tools/imgtool/formats/pc_dsk_legacy.cpp
	${MAME_DIR}/src/tools/imgtool/formats/pc_dsk_legacy.h
	${MAME_DIR}/src/tools/imgtool/modules/amiga.cpp
	${MAME_DIR}/src/tools/imgtool/modules/macbin.cpp
	${MAME_DIR}/src/tools/imgtool/modules/rsdos.cpp
	${MAME_DIR}/src/tools/imgtool/modules/dgndos.cpp
	${MAME_DIR}/src/tools/imgtool/modules/os9.cpp
	${MAME_DIR}/src/tools/imgtool/modules/mac.cpp
	${MAME_DIR}/src/tools/imgtool/modules/ti99.cpp
	${MAME_DIR}/src/tools/imgtool/modules/ti990hd.cpp
	${MAME_DIR}/src/tools/imgtool/modules/concept.cpp
	${MAME_DIR}/src/tools/imgtool/modules/fat.cpp
	${MAME_DIR}/src/tools/imgtool/modules/fat.h
	${MAME_DIR}/src/tools/imgtool/modules/pc_flop.cpp
	${MAME_DIR}/src/tools/imgtool/modules/pc_hard.cpp
	${MAME_DIR}/src/tools/imgtool/modules/prodos.cpp
	${MAME_DIR}/src/tools/imgtool/modules/vzdos.cpp
	${MAME_DIR}/src/tools/imgtool/modules/thomson.cpp
	${MAME_DIR}/src/tools/imgtool/modules/macutil.cpp
	${MAME_DIR}/src/tools/imgtool/modules/macutil.h
	${MAME_DIR}/src/tools/imgtool/modules/cybiko.cpp
	${MAME_DIR}/src/tools/imgtool/modules/cybikoxt.cpp
	${MAME_DIR}/src/tools/imgtool/modules/psion.cpp
	${MAME_DIR}/src/tools/imgtool/modules/bml3.cpp
	${MAME_DIR}/src/tools/imgtool/modules/hp48.cpp
	${MAME_DIR}/src/tools/imgtool/modules/hp9845_tape.cpp
	${MAME_DIR}/src/tools/imgtool/modules/hp85_tape.cpp
	${MAME_DIR}/src/tools/imgtool/modules/rt11.cpp
)

strip_executable(imgtool)
minimal_symbols(imgtool)
