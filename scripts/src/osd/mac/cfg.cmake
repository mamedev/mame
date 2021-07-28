# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

macro(osd_cfg _project)
	add_project_to_group(libs ${_project})

	if(USE_TAPTUN OR USE_PCAP)
		target_compile_definitions(${_project} PRIVATE USE_NETWORK)
		if (USE_TAPTUN)
			target_compile_definitions(${_project} PRIVATE OSD_NET_USE_TAPTUN)
		endif()
		if (USE_PCAP)
			target_compile_definitions(${_project} PRIVATE OSD_NET_USE_PCAP)
		endif()
	endif()

	target_compile_definitions(${_project} PRIVATE
		OSD_MAC
		SDLMAME_UNIX
		SDLMAME_MACOSX
		SDLMAME_DARWIN
	)

	target_include_directories(${_project} PRIVATE ${MAME_DIR}/3rdparty/bx/include/compat/osx)
endmacro()
