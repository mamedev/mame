// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/selmenu.h

    MAME system/software selection menu.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_SELMENU_H
#define MAME_FRONTEND_UI_SELMENU_H

#pragma once

#include "ui/menu.h"

#include <map>
#include <memory>
#include <mutex>
#include <vector>


namespace ui {
class menu_select_launch : public menu
{
public:

	virtual ~menu_select_launch() override;

protected:

	// tab navigation
	enum class focused_menu
	{
		main,
		left,
		righttop,
		rightbottom
	};

	menu_select_launch(mame_ui_manager &mui, render_container *container, bool is_swlist);

	focused_menu get_focus() const { return m_focus; }
	void set_focus(focused_menu focus) { m_focus = focus; }

	// draw right box
	float draw_right_box_title(float x1, float y1, float x2, float y2);

	// images render
	std::string arts_render_common(float origx1, float origy1, float origx2, float origy2);
	void arts_render_images(bitmap_argb32 *bitmap, float origx1, float origy1, float origx2, float origy2);

	// draw arrow
	void draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title);
	void draw_info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width);

	// draw toolbar
	void draw_toolbar(float x1, float y1, float x2, float y2, bool software = false);

	// draw star
	void draw_star(float x0, float y0);

	// draw snapshot if valid
	void draw_snapx(float origx1, float origy1, float origx2, float origy2);
	bool snapx_valid() const { return m_cache->snapx_bitmap().valid(); }

	int     visible_items;
	int     m_total_lines;
	int     m_topline_datsview;   // right box top line
	bool    m_ui_error;

private:
	using bitmap_ptr_vector = std::vector<bitmap_ptr>;
	using texture_ptr_vector = std::vector<texture_ptr>;

	class cache
	{
	public:
		cache(running_machine &machine);
		~cache();

		bitmap_argb32 &snapx_bitmap() { return *m_snapx_bitmap; }
		render_texture *snapx_texture() { return m_snapx_texture.get(); }
		bitmap_argb32 &no_avail_bitmap() { return *m_no_avail_bitmap; }
		render_texture *star_texture() { return m_star_texture.get(); }

		bitmap_ptr_vector const &toolbar_bitmap() { return m_toolbar_bitmap; }
		bitmap_ptr_vector const &sw_toolbar_bitmap() { return m_sw_toolbar_bitmap; }
		texture_ptr_vector const &toolbar_texture() { return m_toolbar_texture; }
		texture_ptr_vector const &sw_toolbar_texture() { return m_sw_toolbar_texture; }

	private:
		bitmap_ptr          m_snapx_bitmap;
		texture_ptr         m_snapx_texture;
		bitmap_ptr          m_no_avail_bitmap;
		bitmap_ptr          m_star_bitmap;
		texture_ptr         m_star_texture;

		bitmap_ptr_vector   m_toolbar_bitmap;
		bitmap_ptr_vector   m_sw_toolbar_bitmap;
		texture_ptr_vector  m_toolbar_texture;
		texture_ptr_vector  m_sw_toolbar_texture;
	};
	using cache_ptr = std::shared_ptr<cache>;
	using cache_ptr_map = std::map<running_machine *, cache_ptr>;

	static constexpr std::size_t MAX_ICONS_RENDER = 40;

	void reset_pressed() { m_pressed = false; m_repeat = 0; }
	bool mouse_pressed() const { return (osd_ticks() >= m_repeat); }
	void set_pressed();

	float draw_icon(int linenum, void *selectedref, float x1, float y1);

	void get_title_search(std::string &title, std::string &search);

	// handle keys
	virtual void handle_keys(UINT32 flags, int &iptkey) override;

	// handle mouse
	virtual void handle_events(UINT32 flags, event &ev) override;

	// draw game list
	virtual void draw(UINT32 flags) override;

	// cleanup function
	static void exit(running_machine &machine);

	cache_ptr		m_cache;
	bool            m_is_swlist;
	focused_menu    m_focus;
	bool            m_pressed;          // mouse button held down
	osd_ticks_t     m_repeat;

	render_texture      *m_icons_texture[MAX_ICONS_RENDER];
	bitmap_ptr          m_icons_bitmap[MAX_ICONS_RENDER];
	game_driver const   *m_old_icons[MAX_ICONS_RENDER];

	static std::mutex       s_cache_guard;
	static cache_ptr_map    s_caches;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELMENU_H