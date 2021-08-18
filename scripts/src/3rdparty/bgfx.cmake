# license:BSD-3-Clause
# copyright-holders:MAMEdev Team

##################################################
## BGFX library objects
##################################################
add_library(bgfx STATIC EXCLUDE_FROM_ALL)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	target_compile_options(bgfx PRIVATE /wd4324) # warning C4324: 'xxx' : structure was padded due to __declspec(align())
	target_compile_options(bgfx PRIVATE /wd4244) # warning C4244: 'argument' : conversion from 'xxx' to 'xxx', possible loss of data
	target_compile_options(bgfx PRIVATE /wd4611) # warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	target_compile_options(bgfx PRIVATE /wd4310) # warning C4310: cast truncates constant value
	target_compile_options(bgfx PRIVATE /wd4701) # warning C4701: potentially uninitialized local variable 'xxx' used
endif()

if ((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
	target_compile_options(bgfx PRIVATE -Wno-unneeded-internal-declaration)
endif()

target_link_libraries(bgfx PRIVATE bx bimg)

target_include_directories(bgfx
	PUBLIC
		${MAME_DIR}/3rdparty/bgfx/include
	PRIVATE
		${MAME_DIR}/3rdparty/bgfx/3rdparty
		${MAME_DIR}/3rdparty/bgfx/3rdparty/khronos
		${MAME_DIR}/3rdparty/bgfx/3rdparty/dxsdk/include
)

target_compile_definitions(bgfx PRIVATE
	BGFX_CONFIG_MAX_FRAME_BUFFERS=128
)

if((${CMAKE_SYSTEM_NAME} STREQUAL "Linux") OR (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD") OR (${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD"))
	if (NO_X11)
		target_compile_definitions(bgfx PRIVATE
			BGFX_CONFIG_RENDERER_OPENGLES=1
			BGFX_CONFIG_RENDERER_OPENGL=0
		)
	endif()
endif()

if((CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang") AND (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8))
	target_compile_definitions(bgfx PRIVATE TARGET_OS_OSX=1)
endif()

target_sources(bgfx PRIVATE
	${MAME_DIR}/3rdparty/bgfx/src/bgfx.cpp
	${MAME_DIR}/3rdparty/bgfx/src/debug_renderdoc.cpp
	${MAME_DIR}/3rdparty/bgfx/src/dxgi.cpp
	${MAME_DIR}/3rdparty/bgfx/src/glcontext_egl.cpp
	${MAME_DIR}/3rdparty/bgfx/src/glcontext_glx.cpp
	${MAME_DIR}/3rdparty/bgfx/src/glcontext_html5.cpp
	${MAME_DIR}/3rdparty/bgfx/src/glcontext_wgl.cpp
	${MAME_DIR}/3rdparty/bgfx/src/nvapi.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_d3d11.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_d3d12.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_d3d9.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_gl.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_gnm.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_noop.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_nvn.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_vk.cpp
	${MAME_DIR}/3rdparty/bgfx/src/renderer_webgpu.cpp
	${MAME_DIR}/3rdparty/bgfx/src/shader.cpp
	${MAME_DIR}/3rdparty/bgfx/src/shader_dx9bc.cpp
	${MAME_DIR}/3rdparty/bgfx/src/shader_dxbc.cpp
	${MAME_DIR}/3rdparty/bgfx/src/shader_spirv.cpp
	${MAME_DIR}/3rdparty/bgfx/src/topology.cpp
	${MAME_DIR}/3rdparty/bgfx/src/vertexlayout.cpp
	${MAME_DIR}/3rdparty/bgfx/examples/common/imgui/imgui.cpp
	${MAME_DIR}/3rdparty/bgfx/examples/common/nanovg/nanovg.cpp
	${MAME_DIR}/3rdparty/bgfx/examples/common/nanovg/nanovg_bgfx.cpp
	${MAME_DIR}/3rdparty/bgfx/3rdparty/dear-imgui/imgui.cpp
	${MAME_DIR}/3rdparty/bgfx/3rdparty/dear-imgui/imgui_draw.cpp
	${MAME_DIR}/3rdparty/bgfx/3rdparty/dear-imgui/imgui_tables.cpp
	${MAME_DIR}/3rdparty/bgfx/3rdparty/dear-imgui/imgui_widgets.cpp
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
	target_sources(bgfx PRIVATE
		${MAME_DIR}/3rdparty/bgfx/src/glcontext_eagl.mm
		${MAME_DIR}/3rdparty/bgfx/src/glcontext_nsgl.mm
		${MAME_DIR}/3rdparty/bgfx/src/renderer_mtl.mm
	)

	target_compile_definitions(bgfx PRIVATE BGFX_CONFIG_MULTITHREADED=0)

	if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
		target_compile_options(bgfx PRIVATE -x objective-c++)
		target_compile_options(bgfx PRIVATE -Wno-unused-variable)
	endif()
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
	# /usr/local/include is not considered a system include directory on FreeBSD. GL.h resides there.
	target_compile_options(bgfx PRIVATE -isystem /usr/local/include)
endif()
