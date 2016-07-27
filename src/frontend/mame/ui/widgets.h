// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/widgets.h

    Internal MAME widgets for the user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_WIDGETS_H
#define MAME_FRONTEND_UI_WIDGETS_H

#pragma once

#include "bitmap.h"
#include "render.h"

#include <memory>
#include <functional>


namespace ui {
/***************************************************************************
TYPE DEFINITIONS
***************************************************************************/

class widgets_manager
{
public:
	widgets_manager(running_machine &machine);

	render_texture *hilight_texture() { return m_hilight_texture.get(); }
	render_texture *hilight_main_texture() { return m_hilight_main_texture.get(); }
	render_texture *arrow_texture() { return m_arrow_texture.get(); }

	using bitmap_ptr = std::unique_ptr<bitmap_argb32>;
	using texture_ptr = std::unique_ptr<render_texture, std::function<void(render_texture *)> >;

private:
	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

	bitmap_ptr  m_hilight_bitmap;
	texture_ptr m_hilight_texture;
	bitmap_ptr  m_hilight_main_bitmap;
	texture_ptr m_hilight_main_texture;
	texture_ptr m_arrow_texture;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_WIDGETS_H
