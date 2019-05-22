// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Vas Crabb
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


struct ui_software_info;

namespace ui {

class machine_static_info;

class menu_select_launch : public menu
{
public:

	virtual ~menu_select_launch() override;

protected:
	static constexpr std::size_t MAX_ICONS_RENDER = 128;
	static constexpr std::size_t MAX_VISIBLE_SEARCH = 200;

	// tab navigation
	enum class focused_menu
	{
		MAIN,
		LEFT,
		RIGHTTOP,
		RIGHTBOTTOM
	};

	struct texture_and_bitmap
	{
		template <typename T>
		texture_and_bitmap(T &&tex) : texture(std::forward<T>(tex)) { }
		texture_and_bitmap(texture_and_bitmap &&that) = default;
		texture_and_bitmap &operator=(texture_and_bitmap &&that) = default;

		texture_ptr     texture;
		bitmap_argb32   bitmap;
	};

	template <typename Key, typename Compare = std::less<Key> >
	using texture_lru = util::lru_cache_map<Key, texture_and_bitmap, Compare>;

	class system_flags
	{
	public:
		system_flags(machine_static_info const &info);
		system_flags(system_flags const &) = default;
		system_flags(system_flags &&) = default;
		system_flags &operator=(system_flags const &) = default;
		system_flags &operator=(system_flags &&) = default;

		::machine_flags::type machine_flags() const { return m_machine_flags; }
		device_t::feature_type unemulated_features() const { return m_unemulated_features; }
		device_t::feature_type imperfect_features() const { return m_imperfect_features; }
		bool has_keyboard() const { return m_has_keyboard; }
		bool has_analog() const { return m_has_analog; }
		rgb_t status_color() const { return m_status_color; }

	private:
		::machine_flags::type   m_machine_flags;
		device_t::feature_type  m_unemulated_features;
		device_t::feature_type  m_imperfect_features;
		bool                    m_has_keyboard;
		bool                    m_has_analog;
		rgb_t                   m_status_color;
	};

	class reselect_last
	{
	public:
		static std::string const &driver() { return s_driver; }
		static std::string const &software() { return s_software; }
		static std::string const &swlist() { return s_swlist; }

		static void reselect(bool value) { s_reselect = value; }
		static bool get() { return s_reselect; }
		static void reset();

		static void set_driver(std::string const &name);
		static void set_driver(game_driver const &driver) { set_driver(driver.name); }
		static void set_software(game_driver const &driver, ui_software_info const &swinfo);

	private:
		static std::string  s_driver, s_software, s_swlist;
		static bool         s_reselect;
	};

	menu_select_launch(mame_ui_manager &mui, render_container &container, bool is_swlist);

	focused_menu get_focus() const { return m_focus; }
	void set_focus(focused_menu focus) { m_focus = focus; }
	void next_image_view();
	void previous_image_view();

	bool dismiss_error();
	void set_error(reset_options ropt, std::string &&message);

	system_flags const &get_system_flags(game_driver const &driver);

	void launch_system(game_driver const &driver) { launch_system(ui(), driver, nullptr, nullptr, nullptr); }
	void launch_system(game_driver const &driver, ui_software_info const &swinfo) { launch_system(ui(), driver, &swinfo, nullptr, nullptr); }
	void launch_system(game_driver const &driver, ui_software_info const &swinfo, std::string const &part) { launch_system(ui(), driver, &swinfo, &part, nullptr); }

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	// handlers
	void inkey_navigation();
	virtual void inkey_export() = 0;
	void inkey_dats();

	// draw arrow
	void draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title);
	void draw_info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width);

	bool draw_error_text();

	template <typename Filter>
	float draw_left_panel(
			typename Filter::type current,
			std::map<typename Filter::type, typename Filter::ptr> const &filters,
			float x1, float y1, float x2, float y2);

	// icon helpers
	void check_for_icons(char const *listname);
	std::string make_icon_paths(char const *listname) const;
	bool scale_icon(bitmap_argb32 &&src, texture_and_bitmap &dst) const;

	// forcing refresh
	void set_switch_image() { m_switch_image = true; }

	template <typename T> bool select_bios(T const &driver, bool inlist);
	bool select_part(software_info const &info, ui_software_info const &ui_info);

	void *get_selection_ptr() const
	{
		void *const selected_ref(get_selection_ref());
		return (uintptr_t(selected_ref) > skip_main_items) ? selected_ref : m_prev_selected;
	}

	int         visible_items;
	void        *m_prev_selected;
	int         m_total_lines;
	int         m_topline_datsview;
	int         m_filter_highlight;
	std::string m_search;

private:
	using bitmap_vector = std::vector<bitmap_argb32>;
	using texture_ptr_vector = std::vector<texture_ptr>;

	using s_parts = std::unordered_map<std::string, std::string>;
	using s_bios = std::vector<std::pair<std::string, int>>;

	class software_parts;
	class bios_selection;

	class cache
	{
	public:
		cache(running_machine &machine);
		~cache();

		bitmap_argb32 &snapx_bitmap() { return *m_snapx_bitmap; }
		render_texture *snapx_texture() { return m_snapx_texture.get(); }
		bool snapx_driver_is(game_driver const *value) const { return m_snapx_driver == value; }
		bool snapx_software_is(ui_software_info const *software) const { return m_snapx_software == software; }
		void set_snapx_driver(game_driver const *value) { m_snapx_driver = value; }
		void set_snapx_software(ui_software_info const *software) { m_snapx_software = software; }

		bitmap_argb32 &no_avail_bitmap() { return m_no_avail_bitmap; }
		render_texture *star_texture() { return m_star_texture.get(); }

		bitmap_vector const &toolbar_bitmap() { return m_toolbar_bitmap; }
		bitmap_vector const &sw_toolbar_bitmap() { return m_sw_toolbar_bitmap; }
		texture_ptr_vector const &toolbar_texture() { return m_toolbar_texture; }
		texture_ptr_vector const &sw_toolbar_texture() { return m_sw_toolbar_texture; }

	private:
		bitmap_ptr              m_snapx_bitmap;
		texture_ptr             m_snapx_texture;
		game_driver const       *m_snapx_driver;
		ui_software_info const  *m_snapx_software;

		bitmap_argb32           m_no_avail_bitmap;
		bitmap_argb32           m_star_bitmap;
		texture_ptr             m_star_texture;

		bitmap_vector           m_toolbar_bitmap;
		bitmap_vector           m_sw_toolbar_bitmap;
		texture_ptr_vector      m_toolbar_texture;
		texture_ptr_vector      m_sw_toolbar_texture;
	};
	using cache_ptr = std::shared_ptr<cache>;
	using cache_ptr_map = std::map<running_machine *, cache_ptr>;

	using flags_cache = util::lru_cache_map<game_driver const *, system_flags>;

	void reset_pressed() { m_pressed = false; m_repeat = 0; }
	bool mouse_pressed() const { return (osd_ticks() >= m_repeat); }
	void set_pressed();

	bool snapx_valid() const { return m_cache->snapx_bitmap().valid(); }

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) = 0;
	float draw_collapsed_left_panel(float x1, float y1, float x2, float y2);

	// draw infos
	void infos_render(float x1, float y1, float x2, float y2);
	virtual void general_info(const game_driver *driver, std::string &buffer) = 0;

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, game_driver const *&driver) const = 0;
	virtual bool accept_search() const { return true; }
	void select_prev()
	{
		if (!m_prev_selected)
		{
			set_selected_index(0);
		}
		else
		{
			for (int x = 0; x < item_count(); ++x)
			{
				if (item(x).ref == m_prev_selected)
				{
					set_selected_index(x);
					break;
				}
			}
		}
	}

	void draw_toolbar(float x1, float y1, float x2, float y2);
	void draw_star(float x0, float y0);
	void draw_icon(int linenum, void *selectedref, float x1, float y1);
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) = 0;

	void get_title_search(std::string &title, std::string &search);

	// handle keys
	virtual void handle_keys(uint32_t flags, int &iptkey) override;

	// handle mouse
	virtual void handle_events(uint32_t flags, event &ev) override;

	// live search active?
	virtual bool menu_has_search_active() override { return !m_search.empty(); }

	// draw game list
	virtual void draw(uint32_t flags) override;

	// draw right panel
	void draw_right_panel(float origx1, float origy1, float origx2, float origy2);
	float draw_right_box_title(float x1, float y1, float x2, float y2);

	// images render
	void arts_render(float origx1, float origy1, float origx2, float origy2);
	std::string arts_render_common(float origx1, float origy1, float origx2, float origy2);
	void arts_render_images(bitmap_argb32 &&bitmap, float origx1, float origy1, float origx2, float origy2);
	void draw_snapx(float origx1, float origy1, float origx2, float origy2);

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const = 0;
	virtual std::string make_driver_description(game_driver const &driver) const = 0;
	virtual std::string make_software_description(ui_software_info const &software) const = 0;

	// filter navigation
	virtual void filter_selected() = 0;

	static void launch_system(mame_ui_manager &mui, game_driver const &driver, ui_software_info const *swinfo, std::string const *part, int const *bios);
	static bool select_part(mame_ui_manager &mui, render_container &container, software_info const &info, ui_software_info const &ui_info);
	static bool has_multiple_bios(ui_software_info const &swinfo, s_bios &biosname);
	static bool has_multiple_bios(game_driver const &driver, s_bios &biosname);

	// cleanup function
	static void exit(running_machine &machine);

	bool        m_ui_error;
	std::string m_error_text;

	game_driver const           *m_info_driver;
	ui_software_info const      *m_info_software;
	int                         m_info_view;
	std::vector<std::string>    m_items_list;
	std::string                 m_info_buffer;

	cache_ptr               m_cache;
	bool                    m_is_swlist;
	focused_menu            m_focus;
	bool                    m_pressed;              // mouse button held down
	osd_ticks_t             m_repeat;

	int                     m_right_visible_lines;  // right box lines

	bool                    m_has_icons;
	bool                    m_switch_image;
	bool                    m_default_image;
	uint8_t                 m_image_view;
	flags_cache             m_flags;

	static std::mutex       s_cache_guard;
	static cache_ptr_map    s_caches;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELMENU_H
