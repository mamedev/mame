# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## ymfm library objects
##################################################

add_library(ymfm STATIC EXCLUDE_FROM_ALL)

target_compile_definitions(ymfm PRIVATE YMFM_MAME)

target_sources(ymfm PRIVATE
	${MAME_DIR}/3rdparty/ymfm/src/ymfm.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_adpcm.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_adpcm.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_fm.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_fm.ipp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_misc.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_misc.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opl.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opl.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opm.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opm.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opn.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opn.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opq.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opq.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opz.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_opz.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_pcm.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_pcm.h
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_ssg.cpp
	${MAME_DIR}/3rdparty/ymfm/src/ymfm_ssg.h
)
