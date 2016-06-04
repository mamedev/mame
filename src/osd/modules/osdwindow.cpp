// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.cpp - SDL window handling
//
//============================================================

#include "osdwindow.h"

#include "render/drawnone.h"
#include "render/drawbgfx.h"
#if (USE_OPENGL)
#include "render/drawogl.h"
#endif
#ifdef OSD_WINDOWS
#include "render/drawgdi.h"
#include "render/drawd3d.h"
#else
#include "render/draw13.h"
#include "render/drawsdl.h"
#endif

osd_window::~osd_window()
{
}

std::unique_ptr<osd_renderer> osd_renderer::make_for_type(int mode, std::shared_ptr<osd_window> window, int extra_flags)
{
	switch(mode)
	{
#ifdef OSD_WINDOWS
		case VIDEO_MODE_NONE:
			return std::make_unique<renderer_none>(window);
#endif
		case VIDEO_MODE_BGFX:
			return std::make_unique<renderer_bgfx>(window);
#if (USE_OPENGL)
		case VIDEO_MODE_OPENGL:
			return std::make_unique<renderer_ogl>(window);
#endif
#ifdef OSD_WINDOWS
		case VIDEO_MODE_GDI:
			return std::make_unique<renderer_gdi>(window);
		case VIDEO_MODE_D3D:
			return std::make_unique<renderer_d3d9>(window);
#else
		case VIDEO_MODE_SDL2ACCEL:
			return std::make_unique<renderer_sdl2>(window, extra_flags);
		case VIDEO_MODE_SOFT:
			return std::make_unique<renderer_sdl1>(window, extra_flags);
#endif
		default:
			return nullptr;
	}
}
