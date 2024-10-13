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
#include "ui/utils.h"

#include "audit.h"

#include "lrucache.h"

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


struct ui_system_info;
struct ui_software_info;


namespace ui {

enum
{
	RP_FIRST = 0,
	RP_IMAGES = RP_FIRST,
	RP_INFOS,
	RP_LAST = RP_INFOS
};

class machine_static_info;

class menu_select_launch : public menu
{
public:

	virtual ~menu_select_launch() override;

protected:
	static inline constexpr std::size_t MAX_ICONS_RENDER = 128;
	static inline constexpr std::size_t MAX_VISIBLE_SEARCH = 200;

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

	bool dismiss_error();
	void set_error(reset_options ropt, std::string &&message);

	system_flags const &get_system_flags(game_driver const &driver);

	void launch_system(game_driver const &driver) { launch_system(ui(), driver, nullptr, nullptr, nullptr); }
	void launch_system(game_driver const &driver, ui_software_info const &swinfo) { launch_system(ui(), driver, &swinfo, nullptr, nullptr); }
	void launch_system(game_driver const &driver, ui_software_info const &swinfo, std::string const &part) { launch_system(ui(), driver, &swinfo, &part, nullptr); }

	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual void menu_activated() override;
	virtual void menu_deactivated() override;

	// handlers
	virtual void inkey_export() = 0;
	void inkey_dats();

	// draw arrow
	void draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title);
	void draw_info_arrow(u32 flags, int line);

	template <typename Filter>
	void draw_left_panel(u32 flags, typename Filter::type current, std::map<typename Filter::type, typename Filter::ptr> const &filters);

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
		return (uintptr_t(selected_ref) > m_skip_main_items) ? selected_ref : m_prev_selected;
	}

	bool show_left_panel() const { return !(m_panels_status & HIDE_LEFT_PANEL); }
	bool show_right_panel() const { return !(m_panels_status & HIDE_RIGHT_PANEL); }
	u8 right_panel() const { return m_right_panel; }
	u8 right_image() const { return m_image_view; }

	char const *right_panel_config_string() const;
	char const *right_image_config_string() const;
	void set_right_panel(u8 index);
	void set_right_image(u8 index);
	void set_right_panel(std::string_view value);
	void set_right_image(std::string_view value);

	static std::string make_system_audit_fail_text(media_auditor const &auditor, media_auditor::summary summary);
	static std::string make_software_audit_fail_text(media_auditor const &auditor, media_auditor::summary summary);
	static constexpr bool audit_passed(media_auditor::summary summary)
	{
		return (media_auditor::CORRECT == summary) || (media_auditor::BEST_AVAILABLE == summary) || (media_auditor::NONE_NEEDED == summary);
	}

	int         m_available_items;
	int         m_skip_main_items;
	void        *m_prev_selected;
	int         m_total_lines;
	int         m_topline_datsview;
	int         m_filter_highlight;
	std::string m_search;

private:
	enum class pointer_action
	{
		NONE,
		MAIN_TRACK_LINE,
		MAIN_TRACK_RBUTTON,
		MAIN_DRAG,
		LEFT_TRACK_LINE,
		LEFT_DRAG,
		RIGHT_TRACK_TAB,
		RIGHT_TRACK_ARROW,
		RIGHT_TRACK_LINE,
		RIGHT_SWITCH,
		RIGHT_DRAG,
		TOOLBAR_TRACK,
		DIVIDER_TRACK
	};

	using bitmap_vector = std::vector<bitmap_argb32>;
	using texture_ptr_vector = std::vector<texture_ptr>;

	using s_parts = std::unordered_map<std::string, std::string>;
	using s_bios = std::vector<std::pair<std::string, int> >;

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

		bitmap_vector const &toolbar_bitmaps() { return m_toolbar_bitmaps; }
		texture_ptr_vector const &toolbar_textures() { return m_toolbar_textures; }

		void cache_toolbar(running_machine &machine, float width, float height);

	private:
		bitmap_ptr              m_snapx_bitmap;
		texture_ptr             m_snapx_texture;
		game_driver const       *m_snapx_driver;
		ui_software_info const  *m_snapx_software;

		bitmap_argb32           m_no_avail_bitmap;

		bitmap_vector           m_toolbar_bitmaps;
		texture_ptr_vector      m_toolbar_textures;
	};

	// this is to satisfy the std::any requirement that objects be copyable
	class cache_wrapper : public cache
	{
	public:
		cache_wrapper(running_machine &machine) : cache(machine), m_machine(machine) { }
		cache_wrapper(cache_wrapper const &that) : cache(that.m_machine), m_machine(that.m_machine) { }
	private:
		running_machine         &m_machine;
	};

	using flags_cache = util::lru_cache_map<game_driver const *, system_flags>;

	// various helpers for common calculations
	bool main_at_top() const noexcept { return !top_line; }
	bool main_at_bottom() const noexcept { return (top_line + m_primary_lines) >= m_available_items; }
	bool is_main_up_arrow(int index) const noexcept { return !index && !main_at_top(); }
	bool is_main_down_arrow(int index) const noexcept { return ((m_primary_lines - 1) == index) && !main_at_bottom(); }
	bool left_at_top() const noexcept { return !m_left_visible_top; }
	bool left_at_bottom() const noexcept { return (m_left_visible_top + m_left_visible_lines) >= m_left_item_count; }
	bool is_left_up_arrow(int index) const noexcept { return !index && !left_at_top(); }
	bool is_left_down_arrow(int index) const noexcept { return ((m_left_visible_lines - 1) == index) && !left_at_bottom(); }
	bool info_at_top() const noexcept { return !m_topline_datsview; }
	bool info_at_bottom() const noexcept { return (m_topline_datsview + m_right_visible_lines) >= m_total_lines; }

	// getting precalculated geometry
	float left_panel_left() const noexcept { return lr_border(); }
	float left_panel_right() const noexcept { return lr_border() + m_left_panel_width; }
	float right_panel_left() const noexcept { return 1.0F - lr_border() - m_right_panel_width; }
	float right_panel_right() const noexcept { return 1.0F - lr_border(); }
	float right_tab_width() const noexcept { return m_right_panel_width / float(RP_LAST - RP_FIRST + 1); }
	float right_arrows_top() const noexcept { return m_right_heading_top + (0.1F * line_height()); }
	float right_arrows_bottom() const noexcept { return m_right_heading_top + (0.9F * line_height()); }
	float left_divider_left() const noexcept { return lr_border() + m_left_panel_width; }
	float left_divider_right() const noexcept { return lr_border() + m_left_panel_width + m_divider_width; }
	float right_divider_left() const noexcept { return 1.0F - lr_border() - m_right_panel_width - m_divider_width; }
	float right_divider_right() const noexcept { return 1.0F - lr_border() - m_right_panel_width; }

	bool snapx_valid() const { return m_cache.snapx_bitmap().valid(); }

	void draw_divider(u32 flags, float x1, bool right);

	// draw left panel
	virtual void draw_left_panel(u32 flags) = 0;

	// draw infos
	void infos_render(u32 flags);
	void general_info(ui_system_info const *system, game_driver const &driver, std::string &buffer);

	// get selected software and/or driver
	virtual void get_selection(ui_software_info const *&software, ui_system_info const *&system) const = 0;

	// show configuration menu
	virtual void show_config_menu(int index) = 0;

	virtual bool accept_search() const { return true; }
	void select_prev()
	{
		if (!m_prev_selected)
		{
			if (m_available_items)
				set_selected_index(0);
		}
		else
		{
			for (int x = 0; x < item_count(); ++x)
			{
				if (item(x).ref() == m_prev_selected)
				{
					set_selected_index(x);
					break;
				}
			}
		}
	}
	void set_focus(focused_menu focus) { m_focus = focus; }
	void rotate_focus(int dir);
	std::pair<bool, bool> next_right_panel_view();
	std::pair<bool, bool> previous_right_panel_view();
	std::pair<bool, bool> next_image_view();
	std::pair<bool, bool> previous_image_view();
	std::pair<bool, bool> next_info_view();
	std::pair<bool, bool> previous_info_view();

	void draw_toolbar(u32 flags, float x1, float y1, float x2, float y2);
	void draw_star(float x0, float y0);
	void draw_icon(int linenum, void *selectedref, float x1, float y1);
	virtual render_texture *get_icon_texture(int linenum, void *selectedref) = 0;

	std::string get_arts_searchpath();

	// event handling
	virtual bool handle_events(u32 flags, event &ev) override;
	virtual bool handle_keys(u32 flags, int &iptkey) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;

	// pointer handling helpers
	std::tuple<int, bool, bool> handle_primary_down(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> handle_right_down(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> handle_middle_down(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_main_track_line(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_main_track_rbutton(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_main_drag(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_left_track_line(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_left_drag(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_right_track_tab(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_right_track_arrow(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_right_track_line(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_right_switch(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_right_drag(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_toolbar_track(bool changed, ui_event const &uievt);
	std::tuple<int, bool, bool> update_divider_track(bool changed, ui_event const &uievt);

	bool main_force_visible_selection();

	// draw game list
	virtual void draw(u32 flags) override;

	// draw right panel
	void draw_right_panel(u32 flags);
	void draw_right_box_tabs(u32 flags);
	void draw_right_box_heading(u32 flags, bool larrow, bool rarrow, std::string_view text);

	// images render
	void arts_render(u32 flags);
	void arts_render_images(bitmap_argb32 &&bitmap);
	void draw_snapx();

	// text for main top/bottom panels
	virtual void make_topbox_text(std::string &line0, std::string &line1, std::string &line2) const = 0;
	virtual std::string make_software_description(ui_software_info const &software, ui_system_info const *system) const = 0;

	// filter navigation
	virtual void filter_selected(int index) = 0;

	static void make_audit_fail_text(std::ostream &str, media_auditor const &auditor, media_auditor::summary summary);
	static void launch_system(mame_ui_manager &mui, game_driver const &driver, ui_software_info const *swinfo, std::string const *part, int const *bios);
	static bool select_part(mame_ui_manager &mui, render_container &container, software_info const &info, ui_software_info const &ui_info);
	static bool has_multiple_bios(ui_software_info const &swinfo, s_bios &biosname);
	static bool has_multiple_bios(game_driver const &driver, s_bios &biosname);

	bool check_scroll_repeat(float top, std::pair<float, float> hbounds, float height)
	{
		float const linetop(top + (float(m_clicked_line) * height));
		float const linebottom(top + (float(m_clicked_line + 1) * height));
		if (pointer_in_rect(hbounds.first, linetop, hbounds.second, linebottom))
		{
			if (std::chrono::steady_clock::now() >= m_scroll_repeat)
			{
				m_scroll_repeat += std::chrono::milliseconds(100);
				return true;
			}
		}
		return false;
	}

	bool        m_ui_error;
	std::string m_error_text;

	game_driver const           *m_info_driver;
	ui_software_info const      *m_info_software;
	int                         m_info_view;
	std::vector<std::string>    m_items_list;
	std::string                 m_info_buffer;
	std::optional<text_layout>  m_info_layout;

	int                         m_icon_width;
	int                         m_icon_height;
	float                       m_divider_width;
	float                       m_divider_arrow_width;
	float                       m_divider_arrow_height;
	float                       m_info_line_height;

	cache                       &m_cache;
	bool                        m_is_swlist;
	focused_menu                m_focus;

	pointer_action              m_pointer_action;
	std::chrono::steady_clock::time_point m_scroll_repeat;
	std::pair<float, float>     m_base_pointer;
	std::pair<float, float>     m_last_pointer;
	int                         m_clicked_line;
	focused_menu                m_wheel_target;
	int                         m_wheel_movement;

	std::pair<float, float>     m_primary_vbounds;
	float                       m_primary_items_top;
	std::pair<float, float>     m_primary_items_hbounds;
	int                         m_primary_lines;

	float                       m_left_panel_width;
	std::pair<float, float>     m_left_items_hbounds;
	float                       m_left_items_top;
	int                         m_left_item_count;
	int                         m_left_visible_lines;
	int                         m_left_visible_top;

	float                       m_right_panel_width;
	float                       m_right_tabs_bottom;
	float                       m_right_heading_top;
	std::pair<float, float>     m_right_content_vbounds;
	std::pair<float, float>     m_right_content_hbounds;
	int                         m_right_visible_lines;  // right box lines

	std::pair<float, float>     m_toolbar_button_vbounds;
	float                       m_toolbar_button_width;
	float                       m_toolbar_button_spacing;
	float                       m_toolbar_backtrack_left;
	float                       m_toolbar_main_left;

	u8                          m_panels_status;
	u8                          m_right_panel;
	bool                        m_has_icons;
	bool                        m_switch_image;
	u8                          m_image_view;
	flags_cache                 m_flags;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_SELMENU_H
