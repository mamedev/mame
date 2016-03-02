// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont, Couriersud
//============================================================
//
//  osdwindow.cpp - SDL window handling
//
//============================================================

// standard windows headers
#ifdef OSD_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#endif

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

osd_renderer* osd_renderer::make_for_type(int mode, osd_window* window, int extra_flags)
{
	switch(mode)
	{
#ifdef OSD_WINDOWS
		case VIDEO_MODE_NONE:
			return new renderer_none(window);
#endif
		case VIDEO_MODE_BGFX:
			return new renderer_bgfx(window);
#if (USE_OPENGL)
		case VIDEO_MODE_OPENGL:
			return new renderer_ogl(window);
#endif
#ifdef OSD_WINDOWS
		case VIDEO_MODE_GDI:
			return new renderer_gdi(window);
		case VIDEO_MODE_D3D:
		{
			osd_renderer *renderer = new renderer_d3d9(window);
			return renderer;
		}
#else
		case VIDEO_MODE_SDL2ACCEL:
			return new renderer_sdl2(window, extra_flags);
		case VIDEO_MODE_SOFT:
			return new renderer_sdl1(window, extra_flags);
#endif
		default:
			return nullptr;
	}
}
