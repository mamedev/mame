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


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MAX_FONT_HEIGHT      (1.0f / 15.0f)

/* width of lines drawn in the UI */
#define UI_LINE_WIDTH           (1.0f / 500.0f)

/* handy colors */
#define UI_GREEN_COLOR          rgb_t(0xef,0x0a,0x66,0x0a)
#define UI_YELLOW_COLOR         rgb_t(0xef,0xcc,0x7a,0x28)
#define UI_RED_COLOR            rgb_t(0xef,0xb2,0x00,0x00)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mame_ui_manager;

// ======================> ui_colors

class ui_colors
{
public:
	rgb_t border_color() const { return m_border_color; }
	rgb_t background_color() const { return m_background_color; }
	rgb_t gfxviewer_bg_color() const { return m_gfxviewer_bg_color; }
	rgb_t unavailable_color() const { return m_unavailable_color; }
	rgb_t text_color() const { return m_text_color; }
	rgb_t text_bg_color() const { return m_text_bg_color; }
	rgb_t subitem_color() const { return m_subitem_color; }
	rgb_t clone_color() const { return m_clone_color; }
	rgb_t selected_color() const { return m_selected_color; }
	rgb_t selected_bg_color() const { return m_selected_bg_color; }
	rgb_t mouseover_color() const { return m_mouseover_color; }
	rgb_t mouseover_bg_color() const { return m_mouseover_bg_color; }
	rgb_t mousedown_color() const { return m_mousedown_color; }
	rgb_t mousedown_bg_color() const { return m_mousedown_bg_color; }
	rgb_t dipsw_color() const { return m_dipsw_color; }
	rgb_t slider_color() const { return m_slider_color; }

	void refresh(const ui_options &options);

private:
	rgb_t m_border_color;
	rgb_t m_background_color;
	rgb_t m_gfxviewer_bg_color;
	rgb_t m_unavailable_color;
	rgb_t m_text_color;
	rgb_t m_text_bg_color;
	rgb_t m_subitem_color;
	rgb_t m_clone_color;
	rgb_t m_selected_color;
	rgb_t m_selected_bg_color;
	rgb_t m_mouseover_color;
	rgb_t m_mouseover_bg_color;
	rgb_t m_mousedown_color;
	rgb_t m_mousedown_bg_color;
	rgb_t m_dipsw_color;
	rgb_t m_slider_color;
};

// ======================> mame_ui_manager

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
	bool update_and_render(render_container &container);

	// getting display font and metrics
	render_font *get_font();
	float get_line_height(float scale = 1.0F);
	float target_font_height() const { return m_target_font_height; }
	float get_char_width(char32_t ch);
	float get_string_width(std::string_view s);
	float get_string_width(std::string_view s, float text_size);
	float box_lr_border() const { return target_font_height() * 0.25f; }
	float box_tb_border() const { return target_font_height() * 0.25f; }

	// drawing boxes and text
	void draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor);
	void draw_outlined_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t fgcolor, rgb_t bgcolor);
	void draw_textured_box(render_container &container, float x0, float y0, float x1, float y1, rgb_t backcolor, rgb_t linecolor, render_texture *texture = nullptr, uint32_t flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	void draw_text(render_container &container, std::string_view buf, float x, float y);
	void draw_text_full(render_container &container, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth = nullptr, float *totalheight = nullptr);
	void draw_text_full(render_container &container, std::string_view origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight, float text_size);
	void draw_text_box(render_container &container, std::string_view text, ui::text_layout::text_justify justify, float xpos, float ypos, rgb_t backcolor);
	void draw_text_box(render_container &container, ui::text_layout &layout, float xpos, float ypos, rgb_t backcolor);
	void draw_message_window(render_container &container, std::string_view text);

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
	void show_menu();
	virtual bool is_menu_active() override;
	bool can_paste();
	bool found_machine_warnings() const { return m_has_warnings; }
	void image_handler_ingame();
	void request_quit();
	void set_pointer_activity_timeout(int target, std::chrono::steady_clock::duration timeout) noexcept;
	void set_hide_inactive_pointers(int target, bool hide) noexcept;
	void restore_initial_pointer_options(int target) noexcept;
	std::chrono::steady_clock::duration pointer_activity_timeout(int target) const noexcept;
	bool hide_inactive_pointers(int target) const noexcept;

	// drawing informational overlays
	void draw_fps_counter(render_container &container);
	void draw_profiler(render_container &container);

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
	ui::text_layout create_layout(render_container &container, float width = 1.0, ui::text_layout::text_justify justify = ui::text_layout::text_justify::LEFT, ui::text_layout::word_wrapping wrap = ui::text_layout::word_wrapping::WORD);
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

	using handler_callback_func = delegate<uint32_t (render_container &)>;
	using device_feature_set = std::set<std::pair<std::string, std::string> >;
	using session_data_map = std::unordered_map<std::type_index, std::any>;
	using active_pointer_vector = std::vector<active_pointer>;
	using pointer_options_vector = std::vector<pointer_options>;
	using display_pointer_vector = std::vector<display_pointer>;

	// instance variables
	std::unique_ptr<render_font> m_font;
	handler_callback_func   m_handler_callback;
	ui_callback_type        m_handler_callback_type;
	bool                    m_ui_active;
	bool                    m_single_step;
	bool                    m_showfps;
	osd_ticks_t             m_showfps_end;
	bool                    m_show_profiler;
	osd_ticks_t             m_popup_text_end;
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
	bool                    m_has_warnings;
	bool                    m_unthrottle_mute;
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
	uint32_t handler_ingame(render_container &container);

	// private methods
	void set_handler(ui_callback_type callback_type, handler_callback_func &&callback);
	void frame_update();
	void exit();
	void increase_frameskip();
	void decrease_frameskip();
	void config_load_warnings(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save_warnings(config_type cfg_type, util::xml::data_node *parentnode);
	void config_load_pointers(config_type cfg_type, config_level cfg_level, util::xml::data_node const *parentnode);
	void config_save_pointers(config_type cfg_type, util::xml::data_node *parentnode);
	template <typename... Params> void slider_alloc(Params &&...args) { m_sliders.push_back(std::make_unique<slider_state>(std::forward<Params>(args)...)); }

	// slider controls
	int32_t slider_volume(std::string *str, int32_t newval);
	int32_t slider_devvol(device_sound_interface *snd, std::string *str, int32_t newval);
	int32_t slider_devvol_chan(device_sound_interface *snd, int channel, std::string *str, int32_t newval);
	int32_t slider_adjuster(ioport_field &field, std::string *str, int32_t newval);
	int32_t slider_overclock(device_t &device, std::string *str, int32_t newval);
	int32_t slider_refresh(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_brightness(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_contrast(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_gamma(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_xscale(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_yscale(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_xoffset(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_yoffset(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_overxscale(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overyscale(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overxoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_overyoffset(laserdisc_device &laserdisc, std::string *str, int32_t newval);
	int32_t slider_flicker(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_beam_width_min(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_beam_width_max(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_beam_dot_size(screen_device &screen, std::string *str, int32_t newval);
	int32_t slider_beam_intensity_weight(screen_device &screen, std::string *str, int32_t newval);
	std::string slider_get_screen_desc(screen_device &screen);
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
