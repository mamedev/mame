# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

if(MSVC)
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

function(add_project_to_group folder target)
	if(MSVC)
		set(SOURCE_GROUP_DELIMITER "/")
		set(last_dir "")
		set(files "")

		get_target_property(sources ${target} SOURCES)

		foreach(file ${sources})
			file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
			get_filename_component(dir "${relative_file}" PATH)
			if(NOT "${dir}" STREQUAL "${last_dir}")
				if(files)
					source_group("${last_dir}" FILES ${files})
				endif()
				set(files "")
			endif()
			set(files ${files} ${file})
			set(last_dir "${dir}")
		endforeach()

		if(files)
			source_group("${last_dir}" FILES ${files})
		endif()
		set_target_properties(${target} PROPERTIES FOLDER ${folder})
	endif()
endfunction()

function(add_project_to_group_and_include folder target include_file)
	include(${include_file})
	add_project_to_group(${folder} ${target})
endfunction()

function(generate_has_header var)
	string(TOLOWER ${var} name)
	set(fname ${GEN_DIR}/has_${name}.h)

	set(GENERATED_HEADER "")
	string(APPEND GENERATED_HEADER "// Generated file, edition is futile\n")
	string(APPEND GENERATED_HEADER "\n")
	string(APPEND GENERATED_HEADER "#ifndef GENERATED_HAS_" ${var} "_H\n")
	string(APPEND GENERATED_HEADER "#define GENERATED_HAS_" ${var} "_H\n")
	string(APPEND GENERATED_HEADER "\n")
	foreach(item IN ITEMS ${${var}})
		string(APPEND GENERATED_HEADER "#define HAS_" ${var} "_" ${item} "\n")
	endforeach()
	string(APPEND GENERATED_HEADER "\n")
	string(APPEND GENERATED_HEADER "#endif\n")

	if(EXISTS ${fname})
		file(READ ${fname} GENERATED_HEADER_CURRENT)
	else()
		set(GENERATED_HEADER_CURRENT "")
	endif()

	if (NOT "${GENERATED_HEADER}" STREQUAL "${GENERATED_HEADER_CURRENT}")
		file(WRITE ${fname} "${GENERATED_HEADER}")
	endif()
endfunction()

function(layoutbuildtask _folder _name)
	add_custom_command(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${_folder}
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/complay.py ${MAME_DIR}/src/${_folder}/${_name}.lay ${GEN_DIR}/${_folder}/${_name}.lh layout_${_name}
		DEPENDS ${MAME_DIR}/scripts/build/complay.py ${MAME_DIR}/src/${_folder}/${_name}.lay
		OUTPUT ${GEN_DIR}/${_folder}/${_name}.lh
		COMMENT "Compressing src/${_folder}/${_name}.lay..."
	)
endfunction()

function(translationbuildtask _language)
	add_custom_command(
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/msgfmt.py --output-file ${MAME_DIR}/language/${_language}/strings.mo ${MAME_DIR}/language/${_language}/strings.po
		DEPENDS ${MAME_DIR}/scripts/build/msgfmt.py ${MAME_DIR}/language/${_language}/strings.po
		OUTPUT ${MAME_DIR}/language/${_language}/strings.mo
		COMMENT "Converting translation language/${_language}/strings.po..."
	)
endfunction()

macro(addprojectflags _project)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		target_compile_options(${_project} PRIVATE -Wsuggest-override)
		target_compile_options(${_project} PRIVATE -flifetime-dse=1)
	endif()
endmacro()

function(dump_mame_options)
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	message(STATUS "Building ${projectname} version ${${projectname}_VERSION}")
	message(STATUS "===========================================================")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(STATUS "OS                  : ${CMAKE_SYSTEM_NAME} (64 bit)")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(STATUS "OS                  : ${CMAKE_SYSTEM_NAME} (32 bit)")
	endif()
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		message(STATUS "Android platform    : ${ANDROID_PLATFORM}")
		message(STATUS "Android ABI         : ${ANDROID_ABI}")
	endif()
	message(STATUS "Platform            : ${CMAKE_SYSTEM_PROCESSOR}")
	message(STATUS "Compiler type       : ${CMAKE_CXX_COMPILER_ID}")
	message(STATUS "C Compiler          : ${CMAKE_C_COMPILER}")
	message(STATUS "          Version   : ${CMAKE_C_COMPILER_VERSION}")
	message(STATUS "C++ Compiler        : ${CMAKE_CXX_COMPILER}")
	message(STATUS "          Version   : ${CMAKE_CXX_COMPILER_VERSION}")
	message(STATUS "Build configuration : ${CMAKE_BUILD_TYPE}")
	message(STATUS "Generator           : ${CMAKE_GENERATOR}")
	message(STATUS "===========================================================")
	message(STATUS "Target              : ${TARGET}")
	message(STATUS "Subtarget           : ${SUBTARGET}")
	message(STATUS "OSD                 : ${OSD}")
	message(STATUS "===========================================================")
	if(NOT SOURCES STREQUAL "")
		string(REPLACE "," ";" SOURCES_LIST ${SOURCES})
		set(_is_first true)
		foreach(_source ${SOURCES_LIST})
			if(${_is_first})
				message(STATUS "Sources to build    : ${_source}")
			else()
				message(STATUS "                      ${_source}")
			endif()
			set(_is_first false)
		endforeach()
	message(STATUS "===========================================================")
	endif()

	if(SDL2_FOUND)
		message(STATUS "SDL2 version        : ${SDL2_VERSION_STRING}")
		message(STATUS "SDL2 include        : ${SDL2_INCLUDE_DIRS}")
		message(STATUS "SDL2 lib            : ${SDL2_LIBRARY}")
		message(STATUS "===========================================================")
	endif()
	if(SDL2_TTF_FOUND)
		message(STATUS "SDL2_ttf version    : ${SDL2_TTF_VERSION_STRING}")
		message(STATUS "SDL2_ttf include    : ${SDL2_TTF_INCLUDE_DIRS}")
		message(STATUS "SDL2_ttf lib        : ${SDL2_TTF_LIBRARY}")
		message(STATUS "===========================================================")
	endif()

	if(OPENGL_FOUND)
		message(STATUS "OpenGL include      : ${OPENGL_INCLUDE_DIR}")
		message(STATUS "OpenGL lib          : ${OPENGL_LIBRARIES}")
		message(STATUS "===========================================================")
	endif()
	if(ALSA_FOUND)
		message(STATUS "ALSA include        : ${ALSA_INCLUDE_DIRS}")
		message(STATUS "ALSA lib            : ${ALSA_LIBRARIES}")
		message(STATUS "===========================================================")
	endif()
	if(Qt5_FOUND)
		message(STATUS "Qt5 version         : ${Qt5_VERSION}")
		message(STATUS "Qt5 dir             : ${Qt5_DIR}")
		message(STATUS "===========================================================")
	endif()
	if(X11_FOUND)
		message(STATUS "X11 include         : ${X11_INCLUDE_DIR}")
		message(STATUS "===========================================================")
	endif()
	if(Fontconfig_FOUND)
		message(STATUS "Fontconfig version  : ${Fontconfig_VERSION}")
		message(STATUS "Fontconfig include  : ${Fontconfig_INCLUDE_DIRS}")
		message(STATUS "Fontconfig lib      : ${Fontconfig_LIBRARIES}")
		message(STATUS "===========================================================")
	endif()


	foreach (_variableName ${_variableNames})
		get_property(result CACHE ${_variableName} PROPERTY TYPE)
		string(FIND "${_variableName}" "CMAKE_" out)
		if(NOT("${out}" EQUAL 0))
		  if((${result} MATCHES "BOOL") OR (${result} MATCHES "STRING"))
			message(STATUS "${_variableName}=${${_variableName}}")
		  endif()
		endif()
	endforeach()
	message(STATUS "===========================================================")
endfunction()

macro(set_option option value)
  set(${option} ${value} CACHE BOOL INTERNAL FORCE)
endmacro()

macro(subdir_list _result _curdir)
  file(GLOB children RELATIVE ${_curdir} ${_curdir}/*)
  set(dirlist "")
  foreach(child ${children})
	if(IS_DIRECTORY ${_curdir}/${child})
	  list(APPEND dirlist ${child})
	endif()
	endforeach()
  set(${_result} ${dirlist})
endmacro()

macro(subdir_list_full _result _curdir)
  file(GLOB children RELATIVE ${_curdir} ${_curdir}/*)
  set(dirlist "")
  foreach(child ${children})
	if(IS_DIRECTORY ${_curdir}/${child})
	  list(APPEND dirlist ${_curdir}/${child})
	endif()
	endforeach()
  set(${_result} ${dirlist})
endmacro()

macro(precompiledheaders _project)
	if(PRECOMPILE)
		if(${CMAKE_GENERATOR} MATCHES "Makefiles")
			# Precompile works fine with CMake but for Makefiles it also add comments in flags.make
			# this creates a problem since after adding new file to library using precompile header
			# all needs to be recompiled even if there is not need. For MAME this is no go.
			# Patch is here until it is resolved upstream
			set(PCH_FILENAME  "cmake_pch.hxx")
			set(PCH_EXTENSION "pch")
			if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
				set(PCH_EXTENSION "gch")
			endif()
			if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
				if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
					set(PCH_FILENAME  "cmake_pch_arm64.hxx")
				endif()
			endif()
			add_dependencies(${_project} precompile)
			target_compile_options(${_project} PRIVATE -include ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/precompile.dir/${PCH_FILENAME})
			get_target_property(MY_PROJECT_SOURCES ${_project} SOURCES)
			set_source_files_properties(${MY_PROJECT_SOURCES} PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/precompile.dir/${PCH_FILENAME};${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/precompile.dir/${PCH_FILENAME}.${PCH_EXTENSION}")
		else()
			target_precompile_headers(${_project} REUSE_FROM precompile)
		endif()
	endif()
endmacro()

macro(precompiledheaders_novs _project)
	if(NOT MSVC)
		precompiledheaders(${_project})
	endif()
endmacro()

option(IGNORE_GIT "Do not use GIT for versioning." OFF)

macro(set_git_version)
	find_package(Git QUIET)

	if (NOT IGNORE_GIT AND GIT_FOUND AND (EXISTS ${MAME_DIR}/.git))
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --dirty
					OUTPUT_VARIABLE NEW_GIT_VERSION
					ERROR_QUIET)

		string(STRIP "${NEW_GIT_VERSION}" NEW_GIT_VERSION)
	endif()

	if ("${NEW_GIT_VERSION}" STREQUAL "")
		set(NEW_GIT_VERSION "unknown")
	endif()
endmacro()

function(strip_executable _projectname)
	if((EXISTS ${CMAKE_STRIP}) AND STRIP_SYMBOLS)
		set(_binary_name "${_projectname}")
		if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
			set(_binary_name "${_projectname}.exe")
		endif()
		add_custom_command(
			TARGET ${_projectname}
			POST_BUILD
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Stripping symbols of ${_binary_name}"
			COMMAND ${CMAKE_STRIP} -s ${_binary_name}
		)
	endif()
endfunction()

function(minimal_symbols _projectname)
	# always include minimum symbols for executables
	if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
		target_compile_options(${_projectname} PRIVATE -g)
	endif()
endfunction()

function(shaders_vs_build _file)
	add_custom_command(
		COMMAND ${CMAKE_BINARY_DIR}/shaderc --platform linux -p 120 --type vertex --depends -o ${BUILD_INTERMEDIATE_DIR}/${_file}.bin -f ${_file}.vs --disasm
		DEPENDS ${_file}
		OUTPUT ${BUILD_INTERMEDIATE_DIR}/${_file}.bin
		COMMENT "Converting translation language/${_language}/strings.po..."
	)
endfunction()
