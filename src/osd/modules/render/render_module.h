// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_OSD_MODULES_RENDER_RENDER_MODULE_H
#define MAME_OSD_MODULES_RENDER_RENDER_MODULE_H

#pragma once

#include "osdepend.h"
#include "modules/osdmodule.h"

#include <memory>


#define OSD_RENDERER_PROVIDER "video"


class osd_renderer;
class osd_window;


class render_module
{
public:
	virtual ~render_module() = default;

	virtual std::unique_ptr<osd_renderer> create(osd_window &window) = 0;

	bool is_interactive() const { return flags() & FLAG_INTERACTIVE; }
	bool sdl_needs_opengl() const { return flags() & FLAG_SDL_NEEDS_OPENGL; }

protected:
	static inline constexpr unsigned FLAG_INTERACTIVE = 1;
	static inline constexpr unsigned FLAG_SDL_NEEDS_OPENGL = 2;

	virtual unsigned flags() const = 0;
};

#endif // MAME_OSD_MODULES_RENDER_RENDER_MODULE_H
