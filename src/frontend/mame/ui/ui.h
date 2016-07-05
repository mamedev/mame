// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

***************************************************************************/

#pragma once

#ifndef __USRINTRF_H__
#define __USRINTRF_H__

#include <vector>

#include "render.h"
#include "moptions.h"
#include "language.h"
#include "ui/uimain.h"
#include "ui/menuitem.h"
#include "ui/slider.h"
#include "ui/text.h"

namespace ui {
class menu_item;

} // namespace ui

/***************************************************************************
    CONSTANTS
***************************************************************************/

// preferred font height; use ui_get_line_height() to get actual height
#define UI_TARGET_FONT_ROWS     get_font_rows()

#define UI_TARGET_FONT_HEIGHT   (1.0f / (float)UI_TARGET_FONT_ROWS)
#define UI_MAX_FONT_HEIGHT      (1.0f / 15.0f)

// width of lines drawn in the UI
#define UI_LINE_WIDTH           (1.0f / 500.0f)

// border between outlines and inner text on left/right and top/bottom sides
#define UI_BOX_LR_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)
#define UI_BOX_TB_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)

// handy colors
#define UI_GREEN_COLOR          rgb_t(0xef,0x10,0x60,0x10)
#define UI_YELLOW_COLOR         rgb_t(0xef,0x60,0x60,0x10)
#define UI_RED_COLOR            rgb_t(0xf0,0x60,0x10,0x10)
#define UI_BORDER_COLOR         decode_ui_color(0)
#define UI_BACKGROUND_COLOR     decode_ui_color(1)
#define UI_GFXVIEWER_BG_COLOR   decode_ui_color(2)
#define UI_UNAVAILABLE_COLOR    decode_ui_color(3)
#define UI_TEXT_COLOR           decode_ui_color(4)
#define UI_TEXT_BG_COLOR        decode_ui_color(5)
#define UI_SUBITEM_COLOR        decode_ui_color(6)
#define UI_CLONE_COLOR          decode_ui_color(7)
#define UI_SELECTED_COLOR       decode_ui_color(8)
#define UI_SELECTED_BG_COLOR    decode_ui_color(9)
#define UI_MOUSEOVER_COLOR      decode_ui_color(10)
#define UI_MOUSEOVER_BG_COLOR   decode_ui_color(11)
#define UI_MOUSEDOWN_COLOR      decode_ui_color(12)
#define UI_MOUSEDOWN_BG_COLOR   decode_ui_color(13)
#define UI_DIPSW_COLOR          decode_ui_color(14)
#define UI_SLIDER_COLOR         decode_ui_color(15)

// cancel return value for a UI handler
#define UI_HANDLER_CANCEL       ((UINT32)~0)

#define SLIDER_DEVICE_SPACING   0x0ff
#define SLIDER_SCREEN_SPACING   0x0f
#define SLIDER_INPUT_SPACING    0x0f

enum
{
	SLIDER_ID_VOLUME                = 0,
	SLIDER_ID_MIXERVOL,
	SLIDER_ID_MIXERVOL_LAST         = SLIDER_ID_MIXERVOL + SLIDER_DEVICE_SPACING,
	SLIDER_ID_ADJUSTER,
	SLIDER_ID_ADJUSTER_LAST         = SLIDER_ID_ADJUSTER + SLIDER_DEVICE_SPACING,
	SLIDER_ID_OVERCLOCK,
	SLIDER_ID_OVERCLOCK_LAST        = SLIDER_ID_OVERCLOCK + SLIDER_DEVICE_SPACING,
	SLIDER_ID_REFRESH,
	SLIDER_ID_REFRESH_LAST          = SLIDER_ID_REFRESH + SLIDER_SCREEN_SPACING,
	SLIDER_ID_BRIGHTNESS,
	SLIDER_ID_BRIGHTNESS_LAST       = SLIDER_ID_BRIGHTNESS + SLIDER_SCREEN_SPACING,
	SLIDER_ID_CONTRAST,
	SLIDER_ID_CONTRAST_LAST         = SLIDER_ID_CONTRAST + SLIDER_SCREEN_SPACING,
	SLIDER_ID_GAMMA,
	SLIDER_ID_GAMMA_LAST            = SLIDER_ID_GAMMA + SLIDER_SCREEN_SPACING,
	SLIDER_ID_XSCALE,
	SLIDER_ID_XSCALE_LAST           = SLIDER_ID_XSCALE + SLIDER_SCREEN_SPACING,
	SLIDER_ID_YSCALE,
	SLIDER_ID_YSCALE_LAST           = SLIDER_ID_YSCALE + SLIDER_SCREEN_SPACING,
	SLIDER_ID_XOFFSET,
	SLIDER_ID_XOFFSET_LAST          = SLIDER_ID_XOFFSET + SLIDER_SCREEN_SPACING,
	SLIDER_ID_YOFFSET,
	SLIDER_ID_YOFFSET_LAST          = SLIDER_ID_YOFFSET + SLIDER_SCREEN_SPACING,
	SLIDER_ID_OVERLAY_XSCALE,
	SLIDER_ID_OVERLAY_XSCALE_LAST   = SLIDER_ID_OVERLAY_XSCALE + SLIDER_SCREEN_SPACING,
	SLIDER_ID_OVERLAY_YSCALE,
	SLIDER_ID_OVERLAY_YSCALE_LAST   = SLIDER_ID_OVERLAY_YSCALE + SLIDER_SCREEN_SPACING,
	SLIDER_ID_OVERLAY_XOFFSET,
	SLIDER_ID_OVERLAY_XOFFSET_LAST  = SLIDER_ID_OVERLAY_XOFFSET + SLIDER_SCREEN_SPACING,
	SLIDER_ID_OVERLAY_YOFFSET,
	SLIDER_ID_OVERLAY_YOFFSET_LAST  = SLIDER_ID_OVERLAY_YOFFSET + SLIDER_SCREEN_SPACING,
	SLIDER_ID_FLICKER,
	SLIDER_ID_FLICKER_LAST          = SLIDER_ID_FLICKER + SLIDER_SCREEN_SPACING,
	SLIDER_ID_BEAM_WIDTH_MIN,
	SLIDER_ID_BEAM_WIDTH_MIN_LAST   = SLIDER_ID_BEAM_WIDTH_MIN + SLIDER_SCREEN_SPACING,
	SLIDER_ID_BEAM_WIDTH_MAX,
	SLIDER_ID_BEAM_WIDTH_MAX_LAST   = SLIDER_ID_BEAM_WIDTH_MAX + SLIDER_SCREEN_SPACING,
	SLIDER_ID_BEAM_INTENSITY,
	SLIDER_ID_BEAM_INTENSITY_LAST   = SLIDER_ID_BEAM_INTENSITY + SLIDER_SCREEN_SPACING,
	SLIDER_ID_CROSSHAIR_SCALE,
	SLIDER_ID_CROSSHAIR_SCALE_LAST  = SLIDER_ID_CROSSHAIR_SCALE + SLIDER_INPUT_SPACING,
	SLIDER_ID_CROSSHAIR_OFFSET,
	SLIDER_ID_CROSSHAIR_OFFSET_LAST = SLIDER_ID_CROSSHAIR_OFFSET + SLIDER_INPUT_SPACING,

	SLIDER_ID_CORE_LAST         = SLIDER_ID_CROSSHAIR_OFFSET,
	SLIDER_ID_CORE_COUNT
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class mame_ui_manager;
typedef UINT32 (*ui_callback)(mame_ui_manager &, render_container *, UINT32);

enum ui_callback_type
{
	UI_CALLBACK_TYPE_GENERAL,
	UI_CALLBACK_TYPE_MODAL,
	UI_CALLBACK_TYPE_MENU
};

// ======================> mame_ui_manager

class mame_ui_manager : public ui_manager, public slider_changed_notifier
{
public:
	enum draw_mode
	{
		NONE,
		NORMAL,
		OPAQUE_
	};

	// construction/destruction
	mame_ui_manager(running_machine &machine);

	void init();

	// getters
	running_machine &machine() const { return m_machine; }
	bool single_step() const { return m_single_step; }
	ui_options &options() { return m_ui_options; }

	// setters
	void set_single_step(bool single_step) { m_single_step = single_step; }

	// methods
	void initialize(running_machine &machine);
	std::vector<ui::menu_item> slider_init(running_machine &machine);

	void set_handler(ui_callback_type callback_type, const std::function<UINT32 (render_container *)> callback);

	template<typename T, typename... Params>
	void set_handler(ui_callback_type callback_type, T &obj, UINT32(T::*callback)(render_container *, Params...), Params ...args)
	{
		auto lambda = [=, &obj](render_container *container)
		{
			return ((obj).*(callback))(container, args...);
		};
		set_handler(callback_type, lambda);
	}

	template<typename... Params>
	void set_handler(ui_callback_type callback_type, UINT32(mame_ui_manager::*callback)(render_container *, Params...), Params ...args)
	{
		set_handler(callback_type, *this, callback, args...);
	}

	template<typename... Params>
	void set_handler(ui_callback_type callback_type, UINT32(*callback)(render_container *, Params...), Params ...args)
	{
		auto lambda = [&, callback](render_container *container)
		{
			return callback(container, args...);
		};
		set_handler(callback_type, lambda);
	}

	void display_startup_screens(bool first_time);
	virtual void set_startup_text(const char *text, bool force) override;
	void update_and_render(render_container *container);
	render_font *get_font();
	float get_line_height();
	float get_char_width(unicode_char ch);
	float get_string_width(const char *s, float text_size = 1.0f);
	void draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor);
	void draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, rgb_t bgcolor);
	void draw_text(render_container *container, const char *buf, float x, float y);
	void draw_text_full(render_container *container, const char *origs, float x, float y, float origwrapwidth, ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, draw_mode draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth = nullptr, float *totalheight = nullptr, float text_size = 1.0f);
	void draw_text_box(render_container *container, const char *text, ui::text_layout::text_justify justify, float xpos, float ypos, rgb_t backcolor);
	void draw_text_box(render_container *container, ui::text_layout &layout, float xpos, float ypos, rgb_t backcolor);
	void draw_message_window(render_container *container, const char *text);

	// load/save options to file
	void load_ui_options();
	void save_ui_options();
	void save_main_option();

	template <typename Format, typename... Params> void popup_time(int seconds, Format &&fmt, Params &&... args);
	void show_fps_temp(double seconds);
	void set_show_fps(bool show);
	bool show_fps() const;
	bool show_fps_counter();
	void set_show_profiler(bool show);
	bool show_profiler() const;
	void show_menu();
	void show_mouse(bool status);
	virtual bool is_menu_active() override;
	bool can_paste();
	void paste();
	void set_use_natural_keyboard(bool use_natural_keyboard);
	void image_handler_ingame();
	void increase_frameskip();
	void decrease_frameskip();
	void request_quit();
	void draw_fps_counter(render_container *container);
	void draw_timecode_counter(render_container *container);
	void draw_timecode_total(render_container *container);
	void draw_profiler(render_container *container);
	void start_save_state();
	void start_load_state();

	// print the game info string into a buffer
	std::string &game_info_astring(std::string &str);

	// slider controls
	std::vector<ui::menu_item>&  get_slider_list(void);

	// other
	void process_natural_keyboard();
	ui::text_layout create_layout(render_container *container, float width = 1.0, ui::text_layout::text_justify justify = ui::text_layout::LEFT, ui::text_layout::word_wrapping wrap = ui::text_layout::WORD);

	// word wrap
	int wrap_text(render_container *container, const char *origs, float x, float y, float origwrapwidth, std::vector<int> &xstart, std::vector<int> &xend, float text_size = 1.0f);

	// draw an outlined box with given line color and filled with a texture
	void draw_textured_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor, rgb_t linecolor, render_texture *texture = nullptr, UINT32 flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	virtual void popup_time_string(int seconds, std::string message) override;

	virtual void menu_reset() override;

private:
	// instance variables
	render_font *           m_font;
	std::function<UINT32 (render_container *)> m_handler_callback;
	ui_callback_type        m_handler_callback_type;
	UINT32                  m_handler_param;
	bool                    m_single_step;
	bool                    m_showfps;
	osd_ticks_t             m_showfps_end;
	bool                    m_show_profiler;
	osd_ticks_t             m_popup_text_end;
	std::unique_ptr<UINT8[]> m_non_char_keys_down;
	render_texture *        m_mouse_arrow_texture;
	bool                    m_mouse_show;
	bool                    m_load_save_hold;
	ui_options              m_ui_options;

	// static variables
	static std::string      messagebox_text;
	static std::string      messagebox_poptext;
	static rgb_t            messagebox_backcolor;

	static std::vector<ui::menu_item> slider_list;
	static slider_state     *slider_current;

	// text generators
	std::string &warnings_string(std::string &buffer);

	// UI handlers
	UINT32 handler_messagebox(render_container *container);
	UINT32 handler_messagebox_anykey(render_container *container);
	UINT32 handler_ingame(render_container *container);
	UINT32 handler_load_save(render_container *container, UINT32 state);
	UINT32 handler_confirm_quit(render_container *container);

	// private methods
	void exit();
	slider_state* slider_alloc(running_machine &machine, int id, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, void *arg);

	// slider controls
	virtual INT32 slider_changed(running_machine &machine, void *arg, int id, std::string *str, INT32 newval) override;

	INT32 slider_volume(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_mixervol(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_adjuster(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_overclock(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_refresh(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_brightness(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_contrast(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_gamma(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_xscale(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_yscale(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_xoffset(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_yoffset(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_overxscale(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_overyscale(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_overxoffset(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_overyoffset(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_flicker(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_beam_width_min(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_beam_width_max(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_beam_intensity_weight(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	std::string slider_get_screen_desc(screen_device &screen);
	#ifdef MAME_DEBUG
	INT32 slider_crossscale(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	INT32 slider_crossoffset(running_machine &machine, void *arg, int id, std::string *str, INT32 newval);
	#endif
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
rgb_t decode_ui_color(int id, running_machine *machine = nullptr);
int get_font_rows(running_machine *machine = nullptr);

template <typename Format, typename... Params>
inline void mame_ui_manager::popup_time(int seconds, Format &&fmt, Params &&... args)
{
	// extract the text
	popup_time_string(seconds, string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
}

#endif  /* __USRINTRF_H__ */
