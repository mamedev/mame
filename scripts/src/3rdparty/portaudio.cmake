# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## PortAudio library objects
##################################################

add_library(portaudio STATIC EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(portaudio PRIVATE /wd4245) # warning C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
	target_compile_options(portaudio PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	target_compile_options(portaudio PRIVATE /wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	target_compile_options(portaudio PRIVATE /wd4389) # warning C4389: 'operator' : signed/unsigned mismatch
	target_compile_options(portaudio PRIVATE /wd4189) # warning C4189: 'xxx' : local variable is initialized but not referenced
	target_compile_options(portaudio PRIVATE /wd4127) # warning C4127: conditional expression is constant
	target_compile_options(portaudio PRIVATE /wd4456) # warning C4456: declaration of 'xxx' hides previous local declaration
	target_compile_options(portaudio PRIVATE /wd4312) # warning C4312: 'type cast': conversion from 'UINT' to 'HWAVEIN' of greater size
	target_compile_options(portaudio PRIVATE /wd4204) # warning C4204: nonstandard extension used : non-constant aggregate initializer
	target_compile_options(portaudio PRIVATE /wd4701) # warning C4701: potentially uninitialized local variable 'xxx' used
	target_compile_options(portaudio PRIVATE /wd4057) # warning C4057: 'function': 'xxx' differs in indirection to slightly different base types from 'xxx'
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(portaudio PRIVATE -Wno-bad-function-cast)
	target_compile_options(portaudio PRIVATE -Wno-missing-braces)
	target_compile_options(portaudio PRIVATE -Wno-strict-prototypes)
	target_compile_options(portaudio PRIVATE -Wno-undef)
	target_compile_options(portaudio PRIVATE -Wno-unknown-pragmas)
	target_compile_options(portaudio PRIVATE -Wno-unused-function)
	target_compile_options(portaudio PRIVATE -Wno-unused-value)
	target_compile_options(portaudio PRIVATE -Wno-unused-variable)

	target_compile_options(portaudio PRIVATE -Wno-unused-but-set-variable)
	target_compile_options(portaudio PRIVATE -Wno-maybe-uninitialized)
	target_compile_options(portaudio PRIVATE -Wno-sometimes-uninitialized)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_options(portaudio PRIVATE -Wno-unknown-warning-option)
		target_compile_options(portaudio PRIVATE -Wno-absolute-value)
		if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10)
			target_compile_options(portaudio PRIVATE -Wno-misleading-indentation)
		endif()
	else()
		target_compile_options(portaudio PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
		target_compile_options(portaudio PRIVATE -w)
	endif()
endif()


target_include_directories(portaudio PRIVATE
	${MAME_DIR}/3rdparty/portaudio/include
	${MAME_DIR}/3rdparty/portaudio/src/common
)

target_sources(portaudio PRIVATE
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_allocation.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_converters.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_cpuload.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_dither.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_debugprint.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_front.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_process.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_stream.c
	${MAME_DIR}/3rdparty/portaudio/src/common/pa_trace.c
	${MAME_DIR}/3rdparty/portaudio/src/hostapi/skeleton/pa_hostapi_skeleton.c
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	target_compile_definitions(portaudio PRIVATE
		PA_USE_DS=1
		PA_USE_WASAPI=1
		PA_USE_WDMKS=1
		PA_USE_WMME=1
	)
	target_include_directories(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/win
	)
	if (NOT MSVC)
		target_include_directories(portaudio PRIVATE
			${MAME_DIR}/3rdparty/portaudio/src/hostapi/wasapi/mingw-include
		)
	endif()

	target_sources(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/win/pa_win_util.c
		${MAME_DIR}/3rdparty/portaudio/src/os/win/pa_win_waveformat.c
		${MAME_DIR}/3rdparty/portaudio/src/os/win/pa_win_hostapis.c
		${MAME_DIR}/3rdparty/portaudio/src/os/win/pa_win_coinitialize.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/dsound/pa_win_ds.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/dsound/pa_win_ds_dynlink.c
		${MAME_DIR}/3rdparty/portaudio/src/os/win/pa_win_hostapis.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/wasapi/pa_win_wasapi.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/wdmks/pa_win_wdmks.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/wmme/pa_win_wmme.c
		${MAME_DIR}/3rdparty/portaudio/src/common/pa_ringbuffer.c
	)

	target_link_libraries(portaudio PUBLIC setupapi) # required for WDMKS
	if (MINGW)
		target_link_libraries(portaudio PUBLIC ksuser)
	endif()
endif()


if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	target_compile_definitions(portaudio PRIVATE
		PA_USE_ALSA=1
		PA_USE_OSS=1
		HAVE_LINUX_SOUNDCARD_H
	)
	target_include_directories(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/unix
	)

	target_sources(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/unix/pa_unix_hostapis.c
		${MAME_DIR}/3rdparty/portaudio/src/os/unix/pa_unix_util.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/alsa/pa_linux_alsa.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/oss/pa_unix_oss.c
	)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_compile_definitions(portaudio PRIVATE
		PA_USE_COREAUDIO=1
	)
	target_include_directories(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/unix
	)

	target_sources(portaudio PRIVATE
		${MAME_DIR}/3rdparty/portaudio/src/os/unix/pa_unix_hostapis.c
		${MAME_DIR}/3rdparty/portaudio/src/os/unix/pa_unix_util.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core_utilities.c
		${MAME_DIR}/3rdparty/portaudio/src/hostapi/coreaudio/pa_mac_core_blocking.c
		${MAME_DIR}/3rdparty/portaudio/src/common/pa_ringbuffer.c
	)
endif()
