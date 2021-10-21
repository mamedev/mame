# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

cmake_minimum_required(VERSION 3.16)
# 3.16 introduce precompile headers support

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/scripts/src/modules)

set(TARGET "mame" CACHE STRING "Building target")
set(SUBTARGET "mame" CACHE STRING "Building subtarget")

if(${TARGET} STREQUAL ${SUBTARGET})
	set(projectname ${TARGET})
elseif(${SUBTARGET} STREQUAL "mess")
	set(projectname ${SUBTARGET})
else()
	set(projectname ${TARGET}${SUBTARGET})
endif()

project(${projectname} VERSION 0.236 LANGUAGES CXX C)
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	enable_language(OBJCXX OBJC)
endif()

include(scripts/src/helpers.cmake)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
	set(OSD_DEFAULT "windows")
else()
	set(OSD_DEFAULT "sdl")
endif()
set(OSD ${OSD_DEFAULT} CACHE STRING "Choose OSD layer implementation")

# silence "no symbols" warnings on apple clang
if(APPLE)
	SET(CMAKE_C_ARCHIVE_CREATE     "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
	SET(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
	SET(CMAKE_C_ARCHIVE_FINISH     "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
	SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build/lib)
#if (SEPARATE_BIN)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#else()
#    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
#endif()

set(STANDALONE false)
set(MAME_DIR "${CMAKE_SOURCE_DIR}")

option(PRECOMPILE "Precompiled headers generation." ON)

set(CPUS "")
set(SOUNDS "")
set(MACHINES "")
set(VIDEOS "")
set(BUSES "")
set(FORMATS "")

option(TOOLS "Enable building tools." OFF)
option(TESTS "Enable building tests." OFF)
option(BENCHMARKS "Enable building benchmarks." OFF)

set(DISTRO "generic" CACHE STRING "Choose distribution.")
set_property(CACHE DISTRO PROPERTY STRINGS "generic" "debian-stable")

set(SOURCES "" CACHE STRING "List of sources to compile.")

option(PROFILE "Enable profiling." OFF)
option(PROFILER "Include the internal profiler." OFF)

set(SYMBOLS_DEFAULT OFF)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(SYMBOLS_DEFAULT ON)
endif()

option(SYMBOLS "Enable symbols." ${SYMBOLS_DEFAULT})
## set the symbols level
if(NOT SOURCES STREQUAL "")
	set(SYMLEVEL_DEFAULT "2")
else()
	set(SYMLEVEL_DEFAULT "1")
endif()

set(SYMLEVEL ${SYMLEVEL_DEFAULT} CACHE STRING "Symbols level.")
set_property(CACHE SYMLEVEL PROPERTY STRINGS "0" "1" "2")

set(OPTIMIZE "3" CACHE STRING "Optimization level.")
set_property(CACHE OPTIMIZE PROPERTY STRINGS "0" "1" "2" "3")

option(MAP "Generate a link map." OFF)

option(NOASM "Disable implementations based on assembler code." OFF)
option(FORCE_DRC_C_BACKEND "Force DRC C backend." OFF)
option(NOWERROR "Do not stop on errors." OFF)
option(DEPRECATED "Generate deprecation warnings during compilation." ON)

option(SSE2 "SSE2 optimized code and SSE2 code generation." OFF)
option(SSE3 "SSE3 optimized code and SSE3 code generation." OFF)
option(OPENMP "OpenMP optimized code." OFF)
option(SEPARATE_BIN "Use separate bin folders." OFF)
option(SHADOW_CHECK "Shadow checks." OFF)
option(STRIP_SYMBOLS "Symbols stripping." OFF)
option(SHLIB "Generate shared libs." OFF)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	option(LLVM_TOOLS "Use LLVM linking tools." OFF)
endif()

if(MSVC)
	set(DEBUG_DIR "" CACHE STRING "Default directory for debugger.")
	set(DEBUG_ARGS "" CACHE STRING "Arguments for running debug build.")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	option(WEBASSEMBLY "Produce WebAssembly output when building with Emscripten." ON)
endif()

set(SANITIZE "" CACHE STRING "Specifies the santizer(s) to use.")
set_property(CACHE SANITIZE PROPERTY STRINGS "address" "memory" "undefined")

## enable symbols as it is useless without them
if(NOT ${SANITIZE} STREQUAL "")
	set_option(SYMBOLS ON)
endif()

## profiler defaults to on for DEBUG builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_option(PROFILER ON)
endif()

## allow gprof profiling as well, which overrides the internal PROFILER
## also enable symbols as it is useless without them
if(PROFILE)
	set_option(PROFILER OFF)
	set_option(SYMBOLS ON)
endif()

include(scripts/src/extlib.cmake)

if(SHLIB)
	set(LIBTYPE SHARED)
else()
	set(LIBTYPE STATIC)
endif()

find_package(PythonInterp 3.2 REQUIRED)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
	set_option(NOASM ON)
endif()

if(NOT ((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64") OR (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "AMD64")))
	set_option(FORCE_DRC_C_BACKEND ON)
endif()

if(NOASM AND (NOT FORCE_DRC_C_BACKEND))
	set_option(FORCE_DRC_C_BACKEND ON)
endif()

set(GEN_DIR "${CMAKE_SOURCE_DIR}/build/generated")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	set_option(ANDROID_PLATFORM 24)
	set_option(NO_USE_MIDI ON)
	set_option(NO_OPENGL ON)
	set_option(USE_QTDEBUG OFF)
	set_option(NO_X11 ON)
	set_option(USE_TAPTUN OFF)
	set_option(USE_PCAP OFF)
	set_option(NOASM ON)
	set_option(WITH_SYSTEM_SDL2 OFF)
endif()

if ("${TARGET}" STREQUAL "")
	message(FATAL_ERROR "Target for build must be specified")
endif()
if ("${SUBTARGET}" STREQUAL "")
	message(FATAL_ERROR "Subtarget for build must be specified")
endif()

set(CMAKE_CONFIGURATION_TYPES Debug Release)
if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Release")
	set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED on)
set(CMAKE_C_EXTENSIONS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(LLVM_TOOLS)
		find_program(CHECK_LLVM_AR "llvm-ar")
		find_program(CHECK_LLVM_RANLIB "llvm-ranlib")
		find_program(CHECK_LLVM_LLD "lld")
		if(CHECK_LLVM_AR AND CHECK_LLVM_RANLIB AND CHECK_LLVM_LLD)
			message(STATUS "[LLVM TOOLS] Overriding AR to ${CHECK_LLVM_AR}")
			set(CMAKE_AR ${CHECK_LLVM_AR} CACHE FILEPATH INTERNAL FORCE)
			message(STATUS "[LLVM TOOLS] Overriding RUNLIB to ${CHECK_LLVM_RANLIB}")
			set(CMAKE_RANLIB ${CHECK_LLVM_RANLIB} CACHE FILEPATH INTERNAL FORCE)
			message(STATUS "[LLVM TOOLS] Adding linking flags '-fuse-ld=lld'")
			set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld" CACHE STRING INTERNAL FORCE)
		else()
			message(FATAL_ERROR "Some of LLVM tools are missing, check for llvm-ar, llvm-ranlib and lld")
		endif()
	endif()
endif()

if (MSVC)
	if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug") # NoMultiProcessorCompilation
		add_compile_options(/MP)         # enable multi-core compilation
	endif()
	add_compile_options(/bigobj)
	add_compile_options(/Zc:__cplusplus)
	add_compile_options(/W4) # ExtraWarnings
	if(NOT NOWERROR)
		add_compile_options(/WX) # FatalWarnings
	endif()
	add_link_options($<$<CONFIG:Release>:/INCREMENTAL:NO>) # NoIncrementalLink
endif()

if (SOURCES STREQUAL "")
	if(NOT EXISTS ${MAME_DIR}/scripts/target/${TARGET}/${SUBTARGET}.cmake)
		message(FATAL_ERROR "File definition for TARGET=${TARGET} SUBTARGET=${SUBTARGET} does not exist")
	endif()
	include(scripts/target/${TARGET}/${SUBTARGET}.cmake)
else()
	string(REPLACE "," ";" SOURCES_LIST ${SOURCES})
	foreach(srcfile ${SOURCES_LIST})
		if(NOT EXISTS "${MAME_DIR}/${srcfile}")
			message(FATAL_ERROR "file '${srcfile}' does not exist")
		endif()
	endforeach()
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build
	)
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py sourcesproject -r ${MAME_DIR} -t ${SUBTARGET} ${SOURCES_LIST}
		OUTPUT_FILE ${CMAKE_BINARY_DIR}/build/sources_${SUBTARGET}.cmake
		RESULT_VARIABLE sourceproject_result
	)
	include(${CMAKE_BINARY_DIR}/build/sources_${SUBTARGET}.cmake)
	execute_process(
		COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_DIR}/${TARGET}
	)
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} ${MAME_DIR}/scripts/build/makedep.py sourcesfilter ${SOURCES_LIST}
		OUTPUT_FILE ${GEN_DIR}/${TARGET}/sources_${SUBTARGET}.flt
		RESULT_VARIABLE sourcesfilter_result
	)
endif()

if ((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND CMAKE_SIZEOF_VOID_P EQUAL 8)
	add_compile_definitions(X64_WINDOWS_ABI)
endif()

# define PTR64 if we are a 64-bit target
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	add_compile_definitions(PTR64=1)
endif()

# define MAME_DEBUG if we are a debugging build
add_compile_definitions($<$<CONFIG:DEBUG>:MAME_DEBUG>)
add_compile_definitions($<$<CONFIG:DEBUG>:MAME_PROFILER>)
#add_compile_definitions($<$<CONFIG:DEBUG>:BGFX_CONFIG_DEBUG=1>)

if(PROFILER)
	add_compile_definitions(MAME_PROFILER) # define MAME_PROFILER if we are a profiling build
endif()

add_compile_definitions($<$<CONFIG:RELEASE>:NDEBUG>)

if(WIN32)
	add_compile_definitions(CRLF=3)
else()
	add_compile_definitions(CRLF=2)
endif()

include (TestBigEndian)
TEST_BIG_ENDIAN(BIG_ENDIAN)

if (BIG_ENDIAN)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		add_compile_definitions(OSX_PPC)
		add_compile_options(-Wno-unused-label)
		add_compile_options(-flax-vector-conversions)
		if(SYMBOLS)
			add_compile_options(-mlong-branch)
		endif()
		if(CMAKE_SIZEOF_VOID_P EQUAL 8)
			add_compile_options(-arch ppc64)
			add_link_options(-arch ppc64)
		else()
			add_compile_options(-arch ppc)
			add_link_options(-arch ppc)
		endif()
	endif()
else()
	add_compile_definitions(LSB_FIRST)
	if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
		if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm64")
			add_compile_options(-arch arm64)
			add_link_options(-arch arm64)
		else()
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				add_compile_options(-arch x86_64)
				add_link_options(-arch x86_64)
			else()
				add_compile_options(-arch i386)
				add_link_options(-arch i386)
			endif()
		endif()
	endif()
endif()

if (NOASM)
	add_compile_definitions(MAME_NOASM)
endif()

if(NOT FORCE_DRC_C_BACKEND AND NOT BIGENDIAN)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		add_compile_definitions(NATIVE_DRC=drcbe_x64)
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		add_compile_definitions(NATIVE_DRC=drcbe_x86)
	endif()
endif()

# GCC
if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
if(NOT MSVC) # Visual Studio Clang CL does not support this
	# this speeds it up a bit by piping between the preprocessor/compiler/assembler
	add_compile_options(-pipe)
endif()

# add -g if we need symbols, and ensure we have frame pointers
if(SYMBOLS)
	add_compile_options(-g${SYMLEVEL})
	add_compile_options(-fno-omit-frame-pointer)
	add_compile_options(-fno-optimize-sibling-calls)
endif()

# we need to disable some additional implicit optimizations for profiling
if(PROFILE)
	add_compile_options(-mno-omit-leaf-frame-pointer)
endif()

# only show shadow warnings when enabled
if(SHADOW_CHECK)
	add_compile_options(-Wshadow)
endif()

# only show deprecation warnings when enabled
if(NOT DEPRECATED)
	add_compile_options(-Wno-deprecated-declarations)
endif()

# add profiling information for the compiler
if(PROFILE)
	add_compile_options(-pg)
	add_link_options(-pg)
endif()

# add the error warning flag

if(NOT NOWERROR)
	add_compile_options(-Werror)
endif()

# if we are optimizing, include optimization options
if(NOT MSVC) # Visual Studio Clang CL does not support this
	add_compile_options(-O${OPTIMIZE})
endif()
add_compile_options(-fno-strict-aliasing)

if(SHLIB)
	add_compile_options(-fPIC)
endif()

if(SSE2)
	add_compile_options(-msse -msse2)
endif()

if(SSE3)
	add_compile_options(-msse -msse2 -msse3)
endif()

# add a basic set of warnings
if(NOT MSVC) # Visual Studio Clang CL value is already set
	add_compile_options(-Wall)
endif()

if(OPENMP)
	add_compile_options(-fopenmp)
	add_link_options(-fopenmp)
	add_compile_definitions(USE_OPENMP=1)
else()
	add_compile_options(-Wno-unknown-pragmas)
endif()

add_compile_options(-Wcast-align)
add_compile_options(-Wformat-security)
add_compile_options(-Wundef)
add_compile_options(-Wwrite-strings)
add_compile_options(-Wno-conversion)
add_compile_options(-Wno-sign-compare)
add_compile_options(-Wno-error=deprecated-declarations)

# warnings only applicable to C compiles
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpointer-arith>)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>)

if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wbad-function-cast>)
endif()

# warnings only applicable to OBJ-C compiles
add_compile_options($<$<COMPILE_LANGUAGE:OBJCXX>:-Wpointer-arith>)

# warnings only applicable to C++ compiles
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)

if(NOT ${SANITIZE} STREQUAL "")
	add_compile_options(-fsanitize=${SANITIZE})
	add_link_options(-fsanitize=${SANITIZE})

	if(${SANITIZE} STREQUAL "address")
		add_compile_options(-fsanitize-address-use-after-scope)
		add_link_options(-fsanitize-address-use-after-scope)
	endif()
	if(${SANITIZE} STREQUAL "undefined")
		# 'function' produces errors without delegates by design
		# 'alignment' produces a lot of errors which we are not interested in
		add_compile_options(-fno-sanitize=function)
		add_compile_options(-fno-sanitize=alignment)
		add_link_options(-fno-sanitize=function)
		add_link_options(-fno-sanitize=alignment)
	endif()
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
		message(FATAL_ERROR "Clang version 6.0 or later needed. Found ${CMAKE_CXX_COMPILER_VERSION}.")
	endif()
	if(NOT MSVC) # Visual Studio Clang CL does not support this
		add_compile_options(-fdiagnostics-show-note-include-stack)
	endif()
	add_compile_options(-Wno-cast-align)
	add_compile_options(-Wno-constant-logical-operand)
	add_compile_options(-Wno-extern-c-compat)
	add_compile_options(-Wno-ignored-qualifiers)
	add_compile_options(-Wno-pragma-pack) #    clang 6.0 complains when the packing change lifetime is not contained within a header file.
	add_compile_options(-Wno-tautological-compare)
	add_compile_options(-Wno-unknown-attributes)
	add_compile_options(-Wno-unknown-warning-option)
	add_compile_options(-Wno-unused-value)
	add_compile_options(-Wno-unused-const-variable)

	if((CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7) OR ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10)))
		# clang 6.0 complains that [[maybe_unused]] is ignored for static data members
		add_compile_options(-Wno-error=ignored-attributes)
		add_compile_options(-Wno-error=unused-const-variable)
	endif()

	if((CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10) OR ((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 12)))
		# clang 10.0 complains that expressions like 10 ^ 7 look like exponention
		add_compile_options(-Wno-xor-used-as-pow)
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
		add_compile_options(-Xclang)
		add_compile_options(-flto-visibility-public-std)     # workround for __imp___ link errors
		add_compile_options(-Wno-nonportable-include-path) # workround for clang 9.0.0 case sensitivity bug when including GL/glext.h
	endif()

	if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		add_compile_options(-Wno-undef)
	endif()

	if(MSVC) # Visual Studio Clang CL
		add_compile_options(-Wno-tautological-constant-out-of-range-compare)
		add_compile_options(-Wno-missing-field-initializers)
		add_compile_options(-Wno-ignored-pragma-optimize)
		add_compile_options(-Wno-unused-function)
		add_compile_options(-Wno-unused-label)
		add_compile_options(-Wno-unused-local-typedef)
		add_compile_options(-Wno-unused-parameter)
		add_compile_options(-Wno-unneeded-internal-declaration)
		add_compile_options(-Wno-unused-private-field)
		add_compile_options(-Wno-missing-braces)
		add_compile_options(-Wno-unused-variable)
		add_compile_options(-Wno-tautological-pointer-compare)
		add_compile_options(-Wno-enum-conversion)
		add_compile_options(-Wno-pragma-pack)
		add_compile_options(-Wno-new-returns-null)
		add_compile_options(-Wno-sign-compare)
		add_compile_options(-Wno-switch)
		add_compile_options(-Wno-tautological-undefined-compare)
		add_compile_options(-Wno-deprecated-declarations)
		add_compile_options(-Wno-macro-redefined)
		add_compile_options(-Wno-narrowing)
		add_compile_options(-Wno-undef)
	endif()

endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
		message(FATAL_ERROR "GCC version 7.0 or later needed. Found ${CMAKE_CXX_COMPILER_VERSION}.")
	endif()

	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wimplicit-fallthrough>)
	add_compile_options(-Wno-unused-result) # needed for fgets,fread on linux
	# array bounds checking seems to be buggy in 4.8.1 (try it on video/stvvdp1.c and video/model1.c without -Wno-array-bounds)
	add_compile_options(-Wno-array-bounds)
	add_compile_options(-Wno-error=attributes) # GCC fails to recognize some uses of [[maybe_unused]]

	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
		add_compile_options(-Wno-format-overflow) # try machine/bfm_sc45_helper.cpp in GCC 8.0.1, among others
		add_compile_options(-Wno-stringop-truncation) # ImGui again
		add_compile_options(-Wno-stringop-overflow) # formats/victor9k_dsk.cpp bugs the compiler

		add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>) # many instances in ImGui and BGFX
	endif()
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10)
		add_compile_options(-Wno-return-local-addr) # sqlite3.c in GCC 10
	endif()
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11)
		add_compile_options(-Wno-nonnull)                                # luaengine.cpp lambdas do not need "this" captured but GCC 11.1 erroneously insists
		add_compile_options(-Wno-stringop-overread)            # machine/bbc.cpp in GCC 11.1
		add_compile_options(-Wno-misleading-indentation) # sqlite3.c in GCC 11.1
		add_compile_options(-Wno-maybe-uninitialized)        # expat in GCC 11.1
	endif()
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
	add_compile_options(-Wno-cast-align)
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "riscv64")
	add_compile_options(-Wno-cast-align)
endif()

endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
		add_compile_options(-Wno-unused-but-set-variable)
endif()

if(${DISTRO} STREQUAL "debian-stable")
	add_compile_definitions(NO_AFFINITY_NP)
endif()

if (MINGW)
	if (OSD STREQUAL "sdl")
		add_link_options(-Wl,--start-group)
		set(CMAKE_CXX_STANDARD_LIBRARIES "-Wl,--end-group ${CMAKE_CXX_STANDARD_LIBRARIES}")
	else()
		add_link_options(-static)
	endif()
endif()

if(MSVC) # For both Visual Studio MSVC and Clang CL
	add_compile_definitions(
		XML_STATIC
		WIN32
		_WIN32
		_CRT_SECURE_NO_WARNINGS
		_CRT_NONSTDC_NO_DEPRECATE
		_CRT_SECURE_NO_DEPRECATE
		_CRT_STDIO_LEGACY_WIDE_SPECIFIERS
	)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/WX)     # Treats all compiler warnings as errors.
	add_compile_options(/w45038) # warning C5038: data member 'member1' will be initialized after data member 'member2'
	add_compile_options(/wd4003) # warning C4003: not enough actual parameters for macro 'xxx'
	add_compile_options(/wd4005) # warning C4005: The macro identifier is defined twice. The compiler uses the second macro definition
	add_compile_options(/wd4018) # warning C4018: 'x' : signed/unsigned mismatch
	add_compile_options(/wd4060) # warning C4060: switch statement contains no 'case' or 'default' labels
	add_compile_options(/wd4065) # warning C4065: switch statement contains 'default' but no 'case' labels
	add_compile_options(/wd4100) # warning C4100: 'xxx' : unreferenced formal parameter
	add_compile_options(/wd4127) # warning C4127: conditional expression is constant
	add_compile_options(/wd4146) # warning C4146: unary minus operator applied to unsigned type, result still unsigned
	add_compile_options(/wd4201) # warning C4201: nonstandard extension used : nameless struct/union
	add_compile_options(/wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	add_compile_options(/wd4245) # warning C4245: 'conversion' : conversion from 'type1' to 'type2', signed/unsigned mismatch
	add_compile_options(/wd4250) # warning C4250: 'xxx' : inherits 'xxx' via dominance
	add_compile_options(/wd4267) # warning C4267: 'var' : conversion from 'size_t' to 'type', possible loss of data
	add_compile_options(/wd4310) # warning C4310: cast truncates constant value
	add_compile_options(/wd4319) # warning C4319: 'operator' : zero extending 'type' to 'type' of greater size
	add_compile_options(/wd4324) # warning C4324: 'xxx' : structure was padded due to __declspec(align())
	add_compile_options(/wd4334) # warning C4334: '<<': result of 32-bit shift implicitly converted to 64 bits (was 64-bit shift intended?)
	add_compile_options(/wd4389) # warning C4389: 'operator' : signed/unsigned mismatch
	add_compile_options(/wd4456) # warning C4456: declaration of 'xxx' hides previous local declaration
	add_compile_options(/wd4457) # warning C4457: declaration of 'xxx' hides function parameter
	add_compile_options(/wd4458) # warning C4458: declaration of 'xxx' hides class member
	add_compile_options(/wd4459) # warning C4459: declaration of 'xxx' hides global declaration
	add_compile_options(/wd4611) # warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	add_compile_options(/wd4702) # warning C4702: unreachable code
	add_compile_options(/wd4706) # warning C4706: assignment within conditional expression
	add_compile_options(/wd4804) # warning C4804: '>>': unsafe use of type 'bool' in operation
	add_compile_options(/wd4805) # warning C4805: 'x' : unsafe mix of type 'xxx' and type 'xxx' in operation
	add_compile_options(/wd4996) # warning C4996: 'function': was declared deprecated
endif()

set_git_version()
configure_file(${MAME_DIR}/scripts/resources/version.cpp.in ${GEN_DIR}/version.cpp @ONLY)

if(NOT EXISTS ${MAME_DIR}/scripts/src/osd/${OSD}/osd.cmake)
	message(FATAL_ERROR "Unsupported value '${OSD}' for OSD")
endif()

include(scripts/src/osd/modules.cmake)
include(scripts/src/osd/${OSD}/cfg.cmake)
include(scripts/src/osd/${OSD}/osd.cmake)

# libs
add_project_to_group_and_include(libs utils scripts/src/lib/utils.cmake)
if (("NETLIST" IN_LIST MACHINES) OR TOOLS)
	add_project_to_group_and_include(libs netlist scripts/src/lib/netlist.cmake)
endif()
add_project_to_group_and_include(libs formats scripts/src/lib/formats.cmake)

# 3rdparty
include(scripts/src/3rdparty.cmake)

# core
add_project_to_group_and_include(core emu scripts/src/emu.cmake)
if(NOT STANDALONE)
	add_project_to_group_and_include(core frontend scripts/src/mame/frontend.cmake)
endif()

# devices
include(scripts/src/devices.cmake)

# drivers
if(NOT STANDALONE)

	if(${CMAKE_VERSION} VERSION_LESS "3.18.0")
			file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build/eval_createProjects.cmake "createProjects_${TARGET}_${SUBTARGET}(${TARGET} ${SUBTARGET})")
			include(${CMAKE_CURRENT_BINARY_DIR}/build/eval_createProjects.cmake)
	else()
		cmake_language(CALL createProjects_${TARGET}_${SUBTARGET} ${TARGET} ${SUBTARGET})
	endif()
endif()

# emulator
include(scripts/src/main.cmake)
mainProject(${TARGET} ${SUBTARGET})

if (TOOLS)
	include(scripts/src/tools.cmake)
endif()

if (TESTS)
	include(scripts/src/tests.cmake)
endif()

if (BENCHMARKS)
	include(scripts/src/benchmarks.cmake)
endif()

generate_has_header(CPUS)
generate_has_header(SOUNDS)
generate_has_header(MACHINES)
generate_has_header(VIDEOS)
generate_has_header(BUSES)
generate_has_header(FORMATS)

dump_mame_options()

#################################################
# Source cleanup
#################################################

if (TOOLS)
	add_custom_target(cleansrc)
	file(GLOB_RECURSE CLEARSRC_FILES
		${MAME_DIR}/src/*.c ${MAME_DIR}/src/*.cpp
		${MAME_DIR}/src/*.h ${MAME_DIR}/src/*.hpp ${MAME_DIR}/src/*.hxx
		${MAME_DIR}/src/*.ipp
		${MAME_DIR}/src/*.lay
		${MAME_DIR}/src/*.mm
		${MAME_DIR}/src/*.lst
		${MAME_DIR}/hash/*.hsi ${MAME_DIR}/hash/*.xml
		${MAME_DIR}/bgfx/*.json
		${MAME_DIR}/plugins/*.lua ${MAME_DIR}/plugins/*.json
		${MAME_DIR}/scripts/*.cmake
	)
	list(APPEND CLEARSRC_FILES ${MAME_DIR}/CMakeLists.txt)
	foreach(cleansrc_file ${CLEARSRC_FILES})
		add_custom_command(
			TARGET cleansrc
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			COMMAND srcclean ${cleansrc_file}
		)
	endforeach()
	add_dependencies(cleansrc srcclean)
endif()

#################################################
# BGFX shaders
#
# to build all just use : make shaders
#
# to build specific chain use for example : make shaders CHAIN=eagle
# data for chain is taken from src/osd/modules/render/bgfx/shaders/chains/
# subfolder named in CHAIN
# NOTE: shaders can be only built on Windows for now
# due to restrictions of way how hlsl shaders are compiled
#################################################

add_project_to_group_and_include(shaderc shaderc scripts/src/tools/shaderc.cmake)

set(SHADER_TARGETS "")
if (WIN32)
	list(APPEND SHADER_TARGETS "dx9")
	list(APPEND SHADER_TARGETS "dx11")
else()
	message(WARNING "Shaders can be only fully built on Windows for now")
endif()
list(APPEND SHADER_TARGETS "essl") # android
list(APPEND SHADER_TARGETS "glsl")
list(APPEND SHADER_TARGETS "metal")
list(APPEND SHADER_TARGETS "spirv")

list(APPEND SHADER_DIRS ${MAME_DIR}/src/osd/modules/render/bgfx/shaders)

set(CHAIN "" CACHE STRING "Name of chain to build with shader compiler.")

if (CHAIN STREQUAL "")
	file(GLOB_RECURSE SHADER_VS_FILES RELATIVE ${SHADER_DIRS} ${SHADER_DIRS}/vs_*.sc)
	file(GLOB_RECURSE SHADER_FS_FILES RELATIVE ${SHADER_DIRS} ${SHADER_DIRS}/fs_*.sc)
else()
	file(GLOB_RECURSE SHADER_VS_FILES RELATIVE ${SHADER_DIRS} ${SHADER_DIRS}/chains/${CHAIN}/vs_*.sc)
	file(GLOB_RECURSE SHADER_FS_FILES RELATIVE ${SHADER_DIRS} ${SHADER_DIRS}/chains/${CHAIN}/fs_*.sc)
endif()

set(shares_build_list "")
foreach(shader_target ${SHADER_TARGETS})
	if(${shader_target} STREQUAL "dx9")
		set(VS_FLAGS --platform windows -p vs_3_0 -O 3)
		set(FS_FLAGS --platform windows -p ps_3_0 -O 3)
	elseif(${shader_target} STREQUAL "dx11")
		set(VS_FLAGS --platform windows -p vs_5_0 -O 3)
		set(FS_FLAGS --platform windows -p ps_5_0 -O 3)
	elseif(${shader_target} STREQUAL "essl")
		set(VS_FLAGS --platform android)
		set(FS_FLAGS --platform android)
	elseif(${shader_target} STREQUAL "glsl")
		set(VS_FLAGS --platform linux -p 120)
		set(FS_FLAGS --platform linux -p 120)
	elseif(${shader_target} STREQUAL "metal")
		set(VS_FLAGS --platform osx -p metal)
		set(FS_FLAGS --platform osx -p metal)
	elseif(${shader_target} STREQUAL "spirv")
		set(VS_FLAGS --platform linux -p spirv)
		set(FS_FLAGS --platform linux -p spirv)
	endif()

	set(INC_FLAGS -i "${MAME_DIR}/3rdparty/bgfx/src/\;${MAME_DIR}/3rdparty/bgfx/examples/common")

	foreach(_location ${SHADER_VS_FILES})
		get_filename_component(_dir  ${_location} DIRECTORY)
		get_filename_component(_fn  ${_location} NAME_WE)
		add_custom_command(
			VERBATIM
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}
			COMMAND ${CMAKE_BINARY_DIR}/shaderc ${VS_FLAGS} ${INC_FLAGS} --type vertex --depends -o ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin -f ${SHADER_DIRS}/${_location} --disasm
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin ${MAME_DIR}/bgfx/shaders/${shader_target}/${_dir}/${_fn}.bin
			DEPENDS ${SHADER_DIRS}/${_location}
			OUTPUT  ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin.depend
			COMMENT "[${shader_target}] Compiling vertex shader ${_fn}..."
		)
		list(APPEND shares_build_list ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin.depend)
	endforeach()
	foreach(_location ${SHADER_FS_FILES})
		get_filename_component(_dir  ${_location} DIRECTORY)
		get_filename_component(_fn  ${_location} NAME_WE)
		add_custom_command(
			VERBATIM
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}
			COMMAND ${CMAKE_BINARY_DIR}/shaderc ${FS_FLAGS} ${INC_FLAGS} --type fragment --depends -o ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin -f ${SHADER_DIRS}/${_location} --disasm
			COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin ${MAME_DIR}/bgfx/shaders/${shader_target}/${_dir}/${_fn}.bin
			DEPENDS ${SHADER_DIRS}/${_location}
			OUTPUT  ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin.depend
			COMMENT "[${shader_target}] Compiling fragment shader ${_fn}..."
		)
		list(APPEND shares_build_list ${CMAKE_BINARY_DIR}/build/shader/${shader_target}/${_dir}/${_fn}.bin.depend)
	endforeach()
endforeach()

add_custom_target(shaders DEPENDS ${shares_build_list})
add_dependencies(shaders shaderc)

#################################################
# Translation
#################################################

add_custom_command(
	WORKING_DIRECTORY ${MAME_DIR}
	COMMAND find src -iname "*.cpp" -print0 | xargs -0 xgettext --from-code=UTF-8 -k_ -k__ -o ${GEN_DIR}/mame.pot
	COMMAND find plugins -iname "*.lua" -print0 | xargs -0 xgettext --from-code=UTF-8 -k_ -k__ -j -o ${GEN_DIR}/mame.pot
	OUTPUT ${GEN_DIR}/mamepot.flag
	COMMENT "Extracting translations..."
)

add_custom_command(
	WORKING_DIRECTORY ${MAME_DIR}
	COMMAND find language -name "*.po" -print0 | xargs -0 -n 1 -I %% msgmerge -U -N %% ${GEN_DIR}/mame.pot
	COMMAND find language -name "*.po" -print0 | xargs -0 -n 1 -I %% msgattrib --clear-fuzzy --empty %% -o %%
	DEPENDS ${GEN_DIR}/mamepot.flag
	OUTPUT ${GEN_DIR}/translation.flag
	COMMENT "Updating translations..."
)

add_custom_target(translation DEPENDS ${GEN_DIR}/mamepot.flag ${GEN_DIR}/translation.flag)

#################################################
# Converting translation
#################################################
subdir_list(LANGUAGE_DIRS ${MAME_DIR}/language)
set(LANGUAGE_FILES "")
foreach(subdir ${LANGUAGE_DIRS})
	list(APPEND LANGUAGE_FILES ${MAME_DIR}/language/${subdir}/strings.mo)
	translationbuildtask(${subdir})
endforeach()
add_custom_target(languages ALL DEPENDS ${LANGUAGE_FILES})
set_target_properties(languages PROPERTIES FOLDER generate)


#################################################
# Compressing layouts
#################################################
file(GLOB LAYOUT_FILES ${MAME_DIR}/src/${TARGET}/layout/*.lay)
set(GEN_LAYOUT_FILES "")
foreach(lay ${LAYOUT_FILES})
	get_filename_component(lay_name ${lay} NAME_WLE)
	layoutbuildtask("${TARGET}/layout" ${lay_name})
	list(APPEND GEN_LAYOUT_FILES ${GEN_DIR}/${TARGET}/layout/${lay_name}.lh)
endforeach()
add_custom_target(layouts DEPENDS ${GEN_LAYOUT_FILES})
set_target_properties(layouts PROPERTIES FOLDER generate)
