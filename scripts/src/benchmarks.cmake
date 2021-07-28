# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##########################################################################
##
##   benchmarks.cmake
##
##   Rules for building benchmarks
##
##########################################################################

##################################################
## Google Benchmark library objects
##################################################

add_library(benchmark STATIC)

target_compile_definitions(benchmark PRIVATE HAVE_STD_REGEX)
target_include_directories(benchmark PRIVATE ${MAME_DIR}/3rdparty/benchmark/include)

target_sources(benchmark PRIVATE
	${MAME_DIR}/3rdparty/benchmark/src/benchmark.cc
	${MAME_DIR}/3rdparty/benchmark/src/colorprint.cc
	${MAME_DIR}/3rdparty/benchmark/src/commandlineflags.cc
	${MAME_DIR}/3rdparty/benchmark/src/complexity.cc
	${MAME_DIR}/3rdparty/benchmark/src/console_reporter.cc
	${MAME_DIR}/3rdparty/benchmark/src/csv_reporter.cc
	${MAME_DIR}/3rdparty/benchmark/src/json_reporter.cc
	${MAME_DIR}/3rdparty/benchmark/src/reporter.cc
	${MAME_DIR}/3rdparty/benchmark/src/sleep.cc
	${MAME_DIR}/3rdparty/benchmark/src/string_util.cc
	${MAME_DIR}/3rdparty/benchmark/src/sysinfo.cc
	${MAME_DIR}/3rdparty/benchmark/src/timers.cc
	${MAME_DIR}/3rdparty/benchmark/src/re_std.cc
)

add_project_to_group(benchmarks benchmark)

##################################################
## MAME benchmarks
##################################################

add_executable(benchmarks)

target_include_directories(benchmarks PRIVATE
	${MAME_DIR}/3rdparty/benchmark/include
	${MAME_DIR}/src/osd
	${MAME_DIR}/src/lib/util
)

target_link_libraries(benchmarks PRIVATE benchmark)
if (WIN32)
	target_link_libraries(benchmarks PRIVATE shlwapi)
else()
	if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
		target_link_libraries(benchmarks PRIVATE pthread)
	endif()
endif()

target_sources(benchmarks PRIVATE
	${MAME_DIR}/benchmarks/main.cpp
	${MAME_DIR}/benchmarks/eminline_native.cpp
	${MAME_DIR}/benchmarks/eminline_noasm.cpp
)

add_project_to_group(benchmarks benchmarks)

strip_executable(benchmarks)
minimal_symbols(benchmarks)
