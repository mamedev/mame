// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_UI_H
#define MAME_FRONTEND_UI_UI_H

#pragma once

#include "language.h"
#include "ui/uimain.h"
#include "ui/menuitem.h"
#include "ui/moptions.h"
#include "ui/slider.h"
#include "ui/text.h"

#include "render.h"

#include "interface/uievents.h"

#include <array>
#include <any>
#include <cassert>
#include <chrono>
#include <ctime>
#include <functional>
#include <set>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>


namespace ui {

class menu_item;
class machine_info;

} // namespace ui

class laserdisc_device;
class vector_device;

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MAX_FONT_HEIGHT      (1.0F / 15.0F)

/* width of lines drawn in the UI */
#define UI_LINE_WIDTH           (1.0F / 500.0F)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mame_ui_manager;


class ui_colors
{
public:
	// a themed foreground/background pair that must be selected atomically
	struct color_pair
	{
		rgb_t foreground;
		rgb_t background;
	};

	struct palette_color
	{
		char const *id;
		char const *name;
		rgb_t value;
	};
	// Complete semantic role values for one predefined UI palette.
	struct semantic_colors
	{
		color_pair normal;
		color_pair selected;
		color_pair mouseover;
		color_pair mousedown;
		color_pair focus;
		rgb_t subitem;
		rgb_t clone;
		rgb_t border;
		rgb_t background;
		rgb_t dipsw;
		rgb_t unavailable;
		rgb_t slider;
		rgb_t gfxviewer_background;
		rgb_t config_deemphasized;
		rgb_t colored_text;
		rgb_t accent;
		rgb_t selection_toolbar;
		rgb_t status_good;
		rgb_t status_warning;
		rgb_t status_error;
		rgb_t focus_outline;
		rgb_t focus_gradient_top;
		rgb_t focus_gradient_bottom;
		rgb_t overlay;
	};

	enum class palette_family : uint8_t
	{
		CLASSIC,
		COMPACT,
		SOLARIZED
	};
	struct palette_definition
	{
		char const *id;
		char const *name;
		palette_family family;
		std::array<palette_color, 16> colors;
	};

	static std::array<palette_definition, 5> const &predefined_palettes();
	std::array<palette_color, 16> const &palette_colors() const { return m_palette_colors; }

	// pair accessors
	color_pair const &normal() const { return m_normal; }
	color_pair const &selected() const { return m_selected; }
	color_pair const &mouseover() const { return m_mouseover; }
	color_pair const &mousedown() const { return m_mousedown; }
	color_pair const &focus() const { return m_focus; }
	// component accessors
	rgb_t border_color() const { return m_border_color; }
	rgb_t background_color() const { return m_background_color; }
	rgb_t gfxviewer_bg_color() const { return m_gfxviewer_bg_color; }
	rgb_t unavailable_color() const { return m_unavailable_color; }
	rgb_t text_color() const { return m_normal.foreground; }
	rgb_t text_bg_color() const { return m_normal.background; }
	rgb_t subitem_color() const { return m_subitem_color; }
	rgb_t clone_color() const { return m_clone_color; }
	rgb_t selected_color() const { return m_selected.foreground; }
	rgb_t selected_bg_color() const { return m_selected.background; }
	rgb_t mouseover_color() const { return m_mouseover.foreground; }
	rgb_t mouseover_bg_color() const { return m_mouseover.background; }
	rgb_t mousedown_color() const { return m_mousedown.foreground; }
	rgb_t mousedown_bg_color() const { return m_mousedown.background; }
	rgb_t dipsw_color() const { return m_dipsw_color; }
	rgb_t slider_color() const { return m_slider_color; }
	// foreground for accent and status backgrounds
	rgb_t colored_text_color() const { return m_colored_text_color; }
	rgb_t accent_color() const { return m_accent_color; }
	rgb_t selection_toolbar_color() const { return m_selection_toolbar_color; }
	rgb_t status_good_color() const { return m_status_good_color; }
	rgb_t status_warning_color() const { return m_status_warning_color; }
	rgb_t status_error_color() const { return m_status_error_color; }
	rgb_t config_deemphasized_color() const { return m_config_deemphasized_color; }
	rgb_t focus_color() const { return m_focus.foreground; }
	rgb_t focus_bg_color() const { return m_focus.background; }
	rgb_t focus_outline_color() const { return m_focus_outline_color; }
	rgb_t focus_gradient_top() const { return m_focus_gradient_top; }
	rgb_t focus_gradient_bottom() const { return m_focus_gradient_bottom; }
	rgb_t overlay_color() const { return m_overlay_color; }

	// load a predefined palette by stable ID
	bool load_palette(char const *palette);

	// load the effective palette from the UI options
	void refresh(const ui_options &options);

private:
	std::array<palette_color, 16> m_palette_colors{};
	color_pair m_normal = { rgb_t::transparent(), rgb_t::transparent() };
	color_pair m_selected = { rgb_t::transparent(), rgb_t::transparent() };
	color_pair m_mouseover = { rgb_t::transparent(), rgb_t::transparent() };
	color_pair m_mousedown = { rgb_t::transparent(), rgb_t::transparent() };
	color_pair m_focus = { rgb_t::transparent(), rgb_t::transparent() };
	rgb_t m_border_color = rgb_t::transparent();
	rgb_t m_background_color = rgb_t::transparent();
	rgb_t m_gfxviewer_bg_color = rgb_t::transparent();
	rgb_t m_unavailable_color = rgb_t::transparent();
	rgb_t m_subitem_color = rgb_t::transparent();
	rgb_t m_clone_color = rgb_t::transparent();
	rgb_t m_dipsw_color = rgb_t::transparent();
	rgb_t m_slider_color = rgb_t::transparent();
	rgb_t m_config_deemphasized_color = rgb_t::transparent();
	rgb_t m_colored_text_color = rgb_t::transparent();
	rgb_t m_accent_color = rgb_t::transparent();
	rgb_t m_selection_toolbar_color = rgb_t::transparent();
	rgb_t m_status_good_color = rgb_t::transparent();
	rgb_t m_status_warning_color = rgb_t::transparent();
	rgb_t m_status_error_color = rgb_t::transparent();
	rgb_t m_focus_outline_color = rgb_t::transparent();
	rgb_t m_focus_gradient_top = rgb_t::transparent();
	rgb_t m_focus_gradient_bottom = rgb_t::transparent();
	rgb_t m_overlay_color = rgb_t::transparent();
};


class mame_ui_manager : public ui_manager
{
public:
	enum : uint32_t
	{
	   HANDLER_UPDATE = 1U << 0, // force video update
	   HANDLER_CANCEL = 1U << 1  // return to in-game event handler
	};

	enum draw_mode
	{
		NONE,
		NORMAL,
		OPAQUE_
	};

	struct display_pointer
	{
		std::reference_wrapper<render_target> target;
		osd::ui_event_handler::pointer type;
		float x, y;

		bool operator!=(display_pointer const &that) const noexcept
		{
			return (&target.get() != &that.target.get()) || (type != that.type) || (x != that.x) || (y != that.y);
		}
	};

	// construction/destruction
	mame_ui_manager(running_machine &machine);
	~mame_ui_manager();

	void init();

	// getters
	running_machine &machine() const { return m_machine; }
	bool single_step() const { return m_single_step; }
	ui_options &options() { return m_ui_options; }
	ui_colors &colors() { return m_ui_colors; }
	ui::machine_info &machine_info() const { assert(m_machine_info); return *m_machine_info; }

	// setters
	void set_single_step(bool single_step) { m_single_step = single_step; }

	// methods
	void initialize(running_machine &machine);
	std::vector<ui::menu_item> slider_init(running_machine &machine);

	void display_startup_screens(bool first_time);
	virtual void set_startup_text(const char *text, bool force) override;
	bool update_and_render(render_target &target);

	// getting display font and metrics
	render_font *get_font();
	float get_line_height(render_target &target, float scale = 1.0F);
	float target_font_height() const { return m_target_font_height; }
	float get_char_width(render_target &target, char32_t ch);
	float get_string_width(render_target &target, std::string_view s);
	float get_string_width(render_target &target, std::string_view s, float text_size);
	float box_lr_border() const { return target_font_height() * 0.25F; }
	float box_tb_border() const { return target_font_height() * 0.25F; }

	// drawing boxes and text
	void draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor);
	void draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t fgcolor, rgb_t bgcolor);
	void draw_textured_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor, rgb_t linecolor, render_texture *texture = nullptr, uint32_t flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	void draw_text(render_target &target, std::string_view buf, float x, float y);
	void draw_text_full(render_target &target, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth = nullptr, float *totalheight = nullptr);
	void draw_text_full(render_target &target, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight, float text_size);
	void draw_text_full(render_container &container, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight, float text_size);
	void draw_text_full(render_container &container, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight, float text_size, float aspect);
	void draw_text_box(render_target &target, std::string_view text, ui::text_layout::text_justify justify, float xpos, float ypos, rgb_t backcolor);
	void draw_text_box(render_target &target, ui::text_layout &layout, float xpos, float ypos, rgb_t backcolor);
	void draw_message_window(render_target &target, std::string_view text);

	// load/save options to file
	void load_ui_options();
	void save_ui_options();
	void save_main_option();

	template <typename Format, typename... Params> void popup_time(int seconds, Format &&fmt, Params &&... args);
	void set_ui_active(bool active) { m_ui_active = active; }
	bool ui_active() const { return m_ui_active; }
	void show_fps_temp(double seconds);
	void set_show_fps(bool show);
	bool show_fps() const;
	bool show_fps_counter();
	void set_show_profiler(bool show);
	bool show_profiler() const;
	bool show_menu();
	bool show_menu(render_target &target);
	virtual bool is_menu_active() override;
	bool can_paste();
	void image_handler_ingame();
	void request_quit();
	void set_pointer_activity_timeout(int target, std::chrono::steady_clock::duration timeout) noexcept;
	void set_hide_inactive_pointers(int target, bool hide) noexcept;
	void restore_initial_pointer_options(int target) noexcept;
	std::chrono::steady_clock::duration pointer_activity_timeout(int target) const noexcept;
	bool hide_inactive_pointers(int target) const noexcept;

	// drawing informational overlays
	void draw_fps_counter(render_target &target);
	void draw_profiler(render_target &target);

	// pointer display for UI handlers
	template <typename T>
	void set_pointers(T first, T last)
	{
		auto dest = m_display_pointers.begin();
		while ((m_display_pointers.end() != dest) && (first != last))
		{
			if (*first != *dest)
			{
				*dest = *first;
				m_pointers_changed = true;
			}
			++dest;
			++first;
		}
		if (m_display_pointers.end() != dest)
		{
			m_display_pointers.erase(dest, m_display_pointers.end());
			m_pointers_changed = true;
		}
		else
		{
			while (first != last)
			{
				m_display_pointers.emplace_back(*first);
				m_pointers_changed = true;
				++first;
			}
		}
	}

	// slider controls
	std::vector<ui::menu_item> &get_slider_list();

	// metrics
	void update_target_font_height();

	// other
	void process_ui_events();
	ui::text_layout create_layout(render_target &target, float width = 1.0, ui::text_layout::text_justify justify = ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping wrap = ui::text_layout::word_wrapping::WORD);
	void set_image_display_enabled(bool image_display_enabled) { m_image_display_enabled = image_display_enabled; }
	bool image_display_enabled() const { return m_image_display_enabled; }
	virtual void popup_time_string(int seconds, std::string message) override;

	virtual void menu_reset() override;
	virtual bool set_ui_event_handler(std::function<bool ()> &&handler) override;

	template <typename Owner, typename Data, typename... Param>
	Data &get_session_data(Param &&... args)
	{
		auto const ins(m_session_data.try_emplace(typeid(Owner)));
		assert(!ins.first->second.has_value() == ins.second);
		if (ins.second)
			return ins.first->second.emplace<Data>(std::forward<Param>(args)...);
		Data *const result(std::any_cast<Data>(&ins.first->second));
		assert(result);
		return *result;
	}

	// helper for getting a general input setting - used for instruction text
	std::string get_general_input_setting(ioport_type type, int player = 0, input_seq_type seqtype = SEQ_TYPE_STANDARD);

private:
	enum class ui_callback_type : int;

	struct active_pointer;
	class pointer_options;

	using handler_callback_func = delegate<uint32_t ()>;
	using device_feature_set = std::set<std::pair<std::string, std::string> >;
	using session_data_map = std::unordered_map<std::type_index, std::any>;
	using active_pointer_vector = std::vector<active_pointer>;
	using pointer_options_vector = std::vector<pointer_options>;
	using display_pointer_vector = std::vector<display_pointer>;

	// instance variables
	std::unique_ptr<render_font> m_font;
	handler_callback_func   m_handler_callback;
	ui_callback_type        m_handler_callback_type;
	render_target *         m_ui_target;
	bool                    m_ui_active;
	bool                    m_paused_for_menu;
	bool                    m_single_step;
	bool                    m_showfps;
	osd_ticks_t             m_showfps_end;
	bool                    m_show_profiler;
	osd_ticks_t             m_popup_text_end;
	osd_ticks_t             m_last_frame_update;
	std::unique_ptr<uint8_t []> m_non_char_keys_down;

	pointer_options_vector  m_pointer_options;
	active_pointer_vector   m_active_pointers;
	display_pointer_vector  m_display_pointers;
	bitmap_argb32           m_mouse_bitmap;
	render_texture *        m_mouse_arrow_texture;
	bool                    m_pointers_changed;

	ui_options              m_ui_options;
	ui_colors               m_ui_colors;
	float                   m_target_font_height;
	bool                    m_unthrottle_mute;
	bool                    m_ui_follow_focus;
	bool                    m_image_display_enabled;

	std::unique_ptr<ui::machine_info> m_machine_info;
	device_feature_set      m_unemulated_features;
	device_feature_set      m_imperfect_features;
	std::time_t             m_last_launch_time;
	std::time_t             m_last_warning_time;

	session_data_map        m_session_data;

	// static variables
	static std::string      messagebox_text;
	static std::string      messagebox_poptext;

	static std::vector<ui::menu_item> slider_list;

	// UI handlers
	uint32_t handler_ingame();

	// private methods
	void set_handler(ui_callback_type callback_type, handler_callback_func &&callback);
	void activate_menu();
	void frame_update();
	void exit();
	void increase_frameskip();
	void decrease_frameskip();
	void config_load_warnings(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save_warnings(config_type cfg_type, util::xml::data_node *parentnode);
	void config_load_pointers(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save_pointers(config_type cfg_type, util::xml::data_node *parentnode);
	template <typename... Params> void slider_alloc(Params &&...args) { m_sliders.push_back(std::make_unique<slider_state>(std::forward<Params>(args)...)); }
	render_target &current_ui_target() const;

	// slider controls
	int32_t slider_volume(std::string *str, int32_t newval);
	int32_t slider_devvol(device_sound_interface *snd, std::string *str, int32_t newval);
	int32_t slider_devvol_chan(device_sound_interface *snd, int channel, std::string *str, int32_t newval);
	int32_t slider_adjuster(ioport_field &field, std::string *str, int32_t newval);
	int32_t slider_speed(std::string *str, int32_t newval);
	int32_t slider_overclock(device_t &device, std::string *str, int32_t newval);
	int32_t slider_refresh(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_brightness(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_contrast(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_gamma(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_xscale(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_yscale(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_xoffset(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_yoffset(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_overxscale(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overyscale(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overxoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overyoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_flicker(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_beam_width_min(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_beam_width_max(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_beam_dot_size(device_video_output_interface &screen, std::string *str, int32_t newval);
	int32_t slider_beam_intensity_weight(device_video_output_interface &screen, std::string *str, int32_t newval);
	std::string slider_get_screen_desc(device_video_output_interface &screen);
#ifdef MAME_DEBUG
	int32_t slider_crossscale(ioport_field &field, std::string *str, int32_t newval);
	int32_t slider_crossoffset(ioport_field &field, std::string *str, int32_t newval);
#endif

	std::vector<std::unique_ptr<slider_state>> m_sliders;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

template <typename Format, typename... Params>
inline void mame_ui_manager::popup_time(int seconds, Format &&fmt, Params &&... args)
{
	// extract the text
	popup_time_string(seconds, string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

#endif  /* MAME_FRONTEND_UI_UI_H */
