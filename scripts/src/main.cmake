# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   main.cmake
##
##   Rules for building main binary
##
##########################################################################

macro(mainProject _target _subtarget)
	if (SOURCES STREQUAL "")
		if(${_target} STREQUAL ${_subtarget})
			set(projectname ${_target})
		elseif(${_subtarget} STREQUAL "mess")
			set(projectname ${_subtarget})
		else()
			set(projectname ${_target}${_subtarget})
		endif()
	else()
		set(projectname ${_subtarget})
	endif()

if(NOT STANDALONE)
	set(MAINFILE  ${MAME_DIR}/src/${_target}/${_subtarget}.cpp)
	if(NOT EXISTS ${MAINFILE})
		set(MAINFILE  ${MAME_DIR}/src/${_target}/${_target}.cpp)
	endif()
	set(MAINPROJECT_SRCS
		${MAINFILE}
		${GEN_DIR}/version.cpp
		${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
	)

	add_executable(${projectname} ${MAINPROJECT_SRCS})
endif()

if(STANDALONE)
	standalone(${projectname})
endif()

add_project_to_group(emulator ${projectname})

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	if(MAP)
		target_link_options(${projectname} PRIVATE "-Wl,-Map,${projectname}.map")
	endif()
endif()

if (SYMBOLS AND MINGW)
	add_custom_command(
		TARGET ${projectname}
		POST_BUILD
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Dumping symbols to ${projectname}.sym."
		COMMAND ${CMAKE_OBJDUMP} --section=.text --line-numbers --syms --demangle ${projectname}.exe > ${projectname}.sym
	)
endif()
minimal_symbols(${projectname})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${projectname}")
	if(PROFILE)
		set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${projectname}p")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${projectname}d")
	if(PROFILE)
		set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${projectname}dp")
	endif()
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	target_sources(${projectname} PRIVATE ${MAME_DIR}/3rdparty/SDL2/src/main/android/SDL_android_main.c)
	set_target_properties(${projectname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${MAME_DIR}/android-project/app/src/main/libs/${ANDROID_ABI}/" )
	set_target_properties(${projectname} PROPERTIES OUTPUT_NAME "main")
	set_target_properties(${projectname} PROPERTIES SUFFIX ".so")
	target_link_options(${projectname} PRIVATE "-shared")
	target_link_options(${projectname} PRIVATE "-Wl,-soname,libmain.so")
	target_link_libraries(${projectname} PRIVATE
		EGL
		GLESv1_CM
		GLESv2
		SDL2
	)
endif()

get_target_property(binary_name ${PROJECT_NAME} OUTPUT_NAME)
strip_executable(${binary_name})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	set_target_properties(${projectname} PROPERTIES SUFFIX ".html")
	target_compile_options(${projectname} PRIVATE -r)
	target_link_options(${projectname} PRIVATE "SHELL:-s USE_SDL=2")
	target_link_options(${projectname} PRIVATE "SHELL:-s USE_SDL_TTF=2")
	target_link_options(${projectname} PRIVATE "SHELL:--memory-init-file 0")
	target_link_options(${projectname} PRIVATE "SHELL:-s EXCEPTION_CATCHING_ALLOWED=\"['__ZN15running_machine17start_all_devicesEv','__ZN12cli_frontend7executeEiPPc','__ZN8chd_file11open_commonEb','__ZN8chd_file13read_metadataEjjRNSt3__212basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE','__ZN8chd_file13read_metadataEjjRNSt3__26vectorIhNS0_9allocatorIhEEEE','__ZNK19netlist_mame_device19base_validity_checkER16validity_checker']\"")
	target_link_options(${projectname} PRIVATE "SHELL:-s EXPORTED_FUNCTIONS=\"['_main', '_malloc', '__ZN15running_machine30emscripten_get_running_machineEv', '__ZN15running_machine17emscripten_get_uiEv', '__ZN15running_machine20emscripten_get_soundEv', '__ZN15mame_ui_manager12set_show_fpsEb', '__ZNK15mame_ui_manager8show_fpsEv', '__ZN13sound_manager4muteEbh', '_SDL_PauseAudio', '_SDL_SendKeyboardKey', '__ZN15running_machine15emscripten_saveEPKc', '__ZN15running_machine15emscripten_loadEPKc', '__ZN15running_machine21emscripten_hard_resetEv', '__ZN15running_machine21emscripten_soft_resetEv', '__ZN15running_machine15emscripten_exitEv']\"")
	target_link_options(${projectname} PRIVATE "SHELL:-s EXPORTED_RUNTIME_METHODS=\"['cwrap']\"")
	target_link_options(${projectname} PRIVATE "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0")
	target_link_options(${projectname} PRIVATE "SHELL:-s USE_WEBGL2=1")
	target_link_options(${projectname} PRIVATE "SHELL:-s LEGACY_GL_EMULATION=1")
	target_link_options(${projectname} PRIVATE "SHELL:-s GL_UNSAFE_OPTS=0")
	target_link_options(${projectname} PRIVATE "SHELL:--pre-js ${MAME_DIR}/src/osd/modules/sound/js_sound.js")
	target_link_options(${projectname} PRIVATE "SHELL:--post-js ${MAME_DIR}/scripts/resources/emscripten/emscripten_post.js")
	target_link_options(${projectname} PRIVATE "SHELL:--embed-file ${MAME_DIR}/bgfx/chains@bgfx/chains")
	target_link_options(${projectname} PRIVATE "SHELL:--embed-file ${MAME_DIR}/bgfx/effects@bgfx/effects")
	target_link_options(${projectname} PRIVATE "SHELL:--embed-file ${MAME_DIR}/bgfx/shaders/essl@bgfx/shaders/essl")
	target_link_options(${projectname} PRIVATE "SHELL:--embed-file ${MAME_DIR}/artwork/bgfx@artwork/bgfx")
	target_link_options(${projectname} PRIVATE "SHELL:--embed-file ${MAME_DIR}/artwork/slot-mask.png@artwork/slot-mask.png")
	if(SYMBOLS)
		target_link_options(${projectname} PRIVATE "SHELL:-s DEMANGLE_SUPPORT=1")
	endif()
	if (WEBASSEMBLY)
		target_link_options(${projectname} PRIVATE "SHELL:-s WASM=1")
		target_link_options(${projectname} PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=1")
	else()
		target_link_options(${projectname} PRIVATE "SHELL:-s WASM=0")
		target_link_options(${projectname} PRIVATE "SHELL:-s ALLOW_MEMORY_GROWTH=0")
		target_link_options(${projectname} PRIVATE "SHELL:-s TOTAL_MEMORY=268435456")
	endif()
endif()

if(NOT STANDALONE)
	if(${CMAKE_VERSION} VERSION_LESS "3.18.0")
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build/eval_linkProjects.cmake "linkProjects_${TARGET}_${SUBTARGET}(${TARGET} ${SUBTARGET} ${projectname})")
		include(${CMAKE_CURRENT_BINARY_DIR}/build/eval_linkProjects.cmake)
	else()
		cmake_language(CALL linkProjects_${TARGET}_${SUBTARGET} ${TARGET} ${SUBTARGET} ${projectname})
	endif()
endif()

target_link_libraries(${projectname} PRIVATE osd_${OSD})

if(NOT STANDALONE)
	target_link_libraries(${projectname} PRIVATE frontend)
endif()
target_link_libraries(${projectname} PRIVATE optional emu)
target_link_libraries(${projectname} PRIVATE formats)

list(LENGTH DASM_SRCS DASM_SRCS_LEN)
if(${DASM_SRCS_LEN} GREATER 0)
	target_link_libraries(${projectname} PRIVATE dasm)
endif()

target_link_libraries(${projectname} PRIVATE
	utils
	wdlfft
	${EXT_LIB_JPEG}
)

set(OVERRIDE_RESOURCES FALSE)

maintargetosdoptions(${projectname})

target_include_directories(${projectname} PRIVATE
		${MAME_DIR}/src/osd
		${MAME_DIR}/src/emu
		${MAME_DIR}/src/devices
		${MAME_DIR}/src/${_target}
		${MAME_DIR}/src/lib
		${MAME_DIR}/src/lib/util
		${MAME_DIR}/3rdparty
		${GEN_DIR}/${_target}/layout
		${GEN_DIR}/resource
)

if(NOT STANDALONE)
	string(TOUPPER ${_subtarget} _build_name)
	if (${_subtarget} STREQUAL "mess")
		# MESS
		set(resource_author "MESS Team")
		set(resource_comments "Multi Emulation Super System")
		set(resource_company_name "MESS Team")
	else()
		# MAME
		set(resource_author "Nicola Salmoria and the MAME Team")
		set(resource_comments "Multi-purpose emulation framework")
		set(resource_company_name "MAME Team")
	endif()
	set(resource_file_description "${_build_name}")
	set(resource_internal_name "${_build_name}")
	set(resource_original_filename "${_build_name}")
	set(resource_product_name "${_build_name}")
	set(resource_bundle_identifier "org.mamedev.${_subtarget}")

	if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		configure_file(${MAME_DIR}/scripts/resources/android/AndroidManifest.xml.in ${MAME_DIR}/android-project/app/src/main/AndroidManifest.xml @ONLY)
	endif()
	if((${CMAKE_SYSTEM_NAME} STREQUAL "Darwin") AND (NOT OVERRIDE_RESOURCES))
		configure_file(${MAME_DIR}/scripts/resources/macos/Info.plist.in ${GEN_DIR}/resource/${_subtarget}-Info.plist @ONLY)
		target_link_options(${projectname} PRIVATE LINKER:-sectcreate,__TEXT,__info_plist,${GEN_DIR}/resource/${_subtarget}-Info.plist)
	endif()
	if((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND (NOT OVERRIDE_RESOURCES))
		set(rctarget ${_target})
		set(rcfile ${MAME_DIR}/scripts/resources/windows/${_subtarget}/${rctarget}.rc)
		if(NOT EXISTS ${rcfile})
			set(rctarget "mame")
			set(rcfile ${MAME_DIR}/scripts/resources/windows/mame/mame.rc)
		endif()
		configure_file(${MAME_DIR}/scripts/resources/windows/vers.rc.in ${GEN_DIR}/resource/${rctarget}vers.rc @ONLY)
		target_sources(${projectname} PRIVATE ${rcfile})
	endif()
	if(SOURCES STREQUAL "")
		if(EXISTS ${MAME_DIR}/src/${_target}/${_subtarget}.flt)
			add_custom_command(
				COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${_target}/${_subtarget}/
				COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py driverlist ${MAME_DIR}/src/${_target}/${_target}.lst -f ${MAME_DIR}/src/${_target}/${_subtarget}.flt > ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
				DEPENDS ${MAME_DIR}/scripts/build/makedep.py ${MAME_DIR}/src/${_target}/${_target}.lst ${MAME_DIR}/src/${_target}/${_subtarget}.flt
				OUTPUT ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
				COMMENT "Building driver list..."
			)
		elseif(EXISTS ${MAME_DIR}/src/${_target}/${_subtarget}.lst)
			add_custom_command(
				COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${_target}/${_subtarget}/
				COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py driverlist ${MAME_DIR}/src/${_target}/${_subtarget}.lst > ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
				DEPENDS ${MAME_DIR}/scripts/build/makedep.py ${MAME_DIR}/src/${_target}/${_subtarget}.lst
				OUTPUT ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
				COMMENT "Building driver list..."
			)
		else()
			add_custom_command(
				COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${_target}/${_target}/
				COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py driverlist ${MAME_DIR}/src/${_target}/${_target}.lst > ${GEN_DIR}/${_target}/${_target}/drivlist.cpp
				DEPENDS ${MAME_DIR}/scripts/build/makedep.py ${MAME_DIR}/src/${_target}/${_target}.lst
				OUTPUT ${GEN_DIR}/${_target}/${_target}/drivlist.cpp
				COMMENT "Building driver list..."
			)
		endif()
	else()
		add_custom_command(
			COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${_target}/${_subtarget}/
			COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py driverlist ${MAME_DIR}/src/${_target}/${_target}.lst -f ${GEN_DIR}/${_target}/sources_${_subtarget}.flt > ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
			DEPENDS ${MAME_DIR}/scripts/build/makedep.py ${MAME_DIR}/src/${_target}/${_target}.lst ${GEN_DIR}/${_target}/sources_${_subtarget}.flt
			OUTPUT ${GEN_DIR}/${_target}/${_subtarget}/drivlist.cpp
			COMMENT "Building driver list..."
		)
	endif()

endif() # (NOT STANDALONE)

	if(MSVC)
		target_link_options(${projectname} PRIVATE "/MANIFEST:NO")

		if(NOT DEBUG_DIR STREQUAL "")
			set_target_properties(${projectname} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${DEBUG_DIR})
		else()
			set_target_properties(${projectname} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${MAME_DIR})
		endif()
		if(NOT DEBUG_ARGS STREQUAL "")
			set_target_properties(${projectname} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS ${DEBUG_ARGS})
		else()
			set_target_properties(${projectname} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "-window")
		endif()
	endif()
endmacro()
