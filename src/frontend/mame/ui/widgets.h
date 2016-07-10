// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

	ui/widgets.h

	Internal MAME widgets for the user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_WIDGETS_H
#define MAME_FRONTEND_UI_WIDGETS_H

#pragma once

#include "ui/ui.h"

namespace ui {
/***************************************************************************
TYPE DEFINITIONS
***************************************************************************/

class widgets_manager
{
public:
	widgets_manager(running_machine &machine, ui_options const &options);

	render_texture *hilight_texture() { return m_hilight_texture.get(); }
	render_texture *hilight_main_texture() { return m_hilight_main_texture.get(); }
	render_texture *arrow_texture() { return m_arrow_texture.get(); }
	bitmap_argb32 *bgrnd_bitmap() { return m_bgrnd_bitmap.get(); }
	render_texture * bgrnd_texture() { return m_bgrnd_texture.get(); }

	using bitmap_ptr = std::unique_ptr<bitmap_argb32>;
	using texture_ptr = std::unique_ptr<render_texture, std::function<void(render_texture *)> >;

private:
	bitmap_ptr              m_hilight_bitmap;
	texture_ptr             m_hilight_texture;
	bitmap_ptr              m_hilight_main_bitmap;
	texture_ptr             m_hilight_main_texture;
	texture_ptr             m_arrow_texture;
	bitmap_ptr              m_bgrnd_bitmap;
	texture_ptr             m_bgrnd_texture;

	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_WIDGETS_H
