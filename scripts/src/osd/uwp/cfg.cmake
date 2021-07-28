# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

macro(osd_cfg _project)
	add_project_to_group(libs ${_project})
	target_compile_definitions(${_project} PRIVATE
		OSD_UWP=1
		USE_QTDEBUG=0
		SDLMAME_NOASM=1
		USE_OPENGL=0
		NO_USE_MIDI=1
		WINVER=0x0603
		_WIN32_WINNT=0x0603
		NTDDI_VERSION=0x06030000
		MODERN_WIN_API
		WIN32_LEAN_AND_MEAN
		NOMINMAX
	)
##flags {
#   "Unicode",
#}
endmacro()
