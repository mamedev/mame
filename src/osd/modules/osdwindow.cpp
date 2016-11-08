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

float osd_window::pixel_aspect() const
{
	return monitor()->pixel_aspect();
}

std::unique_ptr<osd_renderer> osd_renderer::make_for_type(int mode, std::shared_ptr<osd_window> window, int extra_flags)
{
	switch(mode)
	{
		case VIDEO_MODE_NONE:
			return std::make_unique<renderer_none>(window);
		case VIDEO_MODE_BGFX:
			return std::make_unique<renderer_bgfx>(window);
		default:
			return nullptr;
	}
}
