// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

***************************************************************************/

#pragma once

#ifndef __USRINTRF_H__
#define __USRINTRF_H__

#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* preferred font height; use ui_get_line_height() to get actual height */
#define UI_TARGET_FONT_ROWS     get_font_rows()
#define UI_TARGET_FONT_HEIGHT   (1.0f / (float)UI_TARGET_FONT_ROWS)
#define UI_MAX_FONT_HEIGHT      (1.0f / 15.0f)

/* width of lines drawn in the UI */
#define UI_LINE_WIDTH           (1.0f / 500.0f)

/* border between outlines and inner text on left/right and top/bottom sides */
#define UI_BOX_LR_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)
#define UI_BOX_TB_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)

/* handy colors */
#define ARGB_WHITE              rgb_t(0xff,0xff,0xff,0xff)
#define ARGB_BLACK              rgb_t(0xff,0x00,0x00,0x00)
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

/* cancel return value for a UI handler */
#define UI_HANDLER_CANCEL       ((UINT32)~0)

/* justification options for ui_draw_text_full */
enum
{
    JUSTIFY_LEFT = 0,
    JUSTIFY_CENTER,
    JUSTIFY_RIGHT
};

/* word wrapping options for ui_draw_text_full */
enum
{
    WRAP_NEVER,
    WRAP_TRUNCATE,
    WRAP_WORD
};

/* drawing options for ui_draw_text_full */
enum
{
    DRAW_NONE,
    DRAW_NORMAL,
    DRAW_OPAQUE
};

#define SLIDER_NOCHANGE     0x12345678



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT32 (*ui_callback)(running_machine &, render_container *, UINT32);

typedef INT32(*slider_update)(running_machine &machine, void *arg, std::string *str, INT32 newval);

struct slider_state
{
    slider_state *  next;               /* pointer to next slider */
    slider_update   update;             /* callback */
    void *          arg;                /* argument */
    INT32           minval;             /* minimum value */
    INT32           defval;             /* default value */
    INT32           maxval;             /* maximum value */
    INT32           incval;             /* increment value */
    char            description[1];     /* textual description */
};


// ======================> ui_manager

class ui_manager
{
public:
    // construction/destruction
    ui_manager(running_machine &machine);

    // getters
    running_machine &machine() const { return m_machine; }
    bool single_step() const { return m_single_step; }

    // setters
    void set_single_step(bool single_step) { m_single_step = single_step; }

    // methods
    void initialize(running_machine &machine);
    UINT32 set_handler(ui_callback callback, UINT32 param);
    void display_startup_screens(bool first_time, bool show_disclaimer);
    void set_startup_text(const char *text, bool force);
    void update_and_render(render_container *container);
    render_font *get_font();
    float get_line_height();
    float get_char_width(unicode_char ch);
    float get_string_width(const char *s);
    void draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor);
    void draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, rgb_t bgcolor);
    void draw_text(render_container *container, const char *buf, float x, float y);
    void draw_text_full(render_container *container, const char *origs, float x, float y, float origwrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth = NULL, float *totalheight = NULL, float text_size = 1.0f);
    void draw_text_box(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor);
    void draw_message_window(render_container *container, const char *text);

    void CLIB_DECL popup_time(int seconds, const char *text, ...) ATTR_PRINTF(3,4);
    void show_fps_temp(double seconds);
    void set_show_fps(bool show);
    bool show_fps() const;
    bool show_fps_counter();
    void set_show_profiler(bool show);
    bool show_profiler() const;
    void show_menu();
    void show_mouse(bool status);
    bool is_menu_active();
    bool can_paste();
    void paste();
    bool use_natural_keyboard() const;
    void set_use_natural_keyboard(bool use_natural_keyboard);
    void image_handler_ingame();
    void increase_frameskip();
    void decrease_frameskip();
    void request_quit();

    // print the game info string into a buffer
    std::string &game_info_astring(std::string &str);

    // slider controls
    const slider_state *get_slider_list(void);

    // other
    void process_natural_keyboard();

    // MEWUI word wrap
    void wrap_text(render_container *container, const char *origs, float x, float y, float origwrapwidth, int &totallines, std::vector<int> &xstart, std::vector<int> &xend, float text_size = 1.0f);

    // draw an outlined box with given line color and filled with a texture
    void draw_textured_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor, rgb_t linecolor, render_texture *texture = NULL, UINT32 flags = PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

    // return text string width with given text size
    float get_string_width_ex(const char *s, float text_size);

private:
    // instance variables
    running_machine &       m_machine;
    render_font *           m_font;
    ui_callback             m_handler_callback;
    UINT32                  m_handler_param;
    bool                    m_single_step;
    bool                    m_showfps;
    osd_ticks_t             m_showfps_end;
    bool                    m_show_profiler;
    osd_ticks_t             m_popup_text_end;
    bool                    m_use_natural_keyboard;
    UINT8 *                 m_non_char_keys_down;
    render_texture *        m_mouse_arrow_texture;
    bool                    m_mouse_show;

    // text generators
    std::string &disclaimer_string(std::string &buffer);
    std::string &warnings_string(std::string &buffer);

    // UI handlers
    static UINT32 handler_messagebox(running_machine &machine, render_container *container, UINT32 state);
    static UINT32 handler_messagebox_ok(running_machine &machine, render_container *container, UINT32 state);
    static UINT32 handler_messagebox_anykey(running_machine &machine, render_container *container, UINT32 state);
    static UINT32 handler_ingame(running_machine &machine, render_container *container, UINT32 state);
    static UINT32 handler_load_save(running_machine &machine, render_container *container, UINT32 state);
    static UINT32 handler_confirm_quit(running_machine &machine, render_container *container, UINT32 state);

    // private methods
    void exit();
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
rgb_t decode_ui_color(int id, running_machine *machine = NULL);
int get_font_rows(running_machine *machine = NULL);

#endif  /* __USRINTRF_H__ */
