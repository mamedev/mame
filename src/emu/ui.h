/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __USRINTRF_H__
#define __USRINTRF_H__

#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* preferred font height; use ui_get_line_height() to get actual height */
#define UI_TARGET_FONT_ROWS     (25)
#define UI_TARGET_FONT_HEIGHT   (1.0f / (float)UI_TARGET_FONT_ROWS)
#define UI_MAX_FONT_HEIGHT      (1.0f / 15.0f)

/* width of lines drawn in the UI */
#define UI_LINE_WIDTH           (1.0f / 500.0f)

/* border between outlines and inner text on left/right and top/bottom sides */
#define UI_BOX_LR_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)
#define UI_BOX_TB_BORDER        (UI_TARGET_FONT_HEIGHT * 0.25f)

/* handy colors */
#define ARGB_WHITE              MAKE_ARGB(0xff,0xff,0xff,0xff)
#define ARGB_BLACK              MAKE_ARGB(0xff,0x00,0x00,0x00)
#define UI_BORDER_COLOR         MAKE_ARGB(0xff,0xff,0xff,0xff)
#define UI_BACKGROUND_COLOR     MAKE_ARGB(0xef,0x10,0x10,0x30)
#define UI_GFXVIEWER_BG_COLOR   MAKE_ARGB(0xef,0x10,0x10,0x30)
#define UI_GREEN_COLOR          MAKE_ARGB(0xef,0x10,0x60,0x10)
#define UI_YELLOW_COLOR         MAKE_ARGB(0xef,0x60,0x60,0x10)
#define UI_RED_COLOR            MAKE_ARGB(0xf0,0x60,0x10,0x10)
#define UI_UNAVAILABLE_COLOR    MAKE_ARGB(0xff,0x40,0x40,0x40)
#define UI_TEXT_COLOR           MAKE_ARGB(0xff,0xff,0xff,0xff)
#define UI_TEXT_BG_COLOR        MAKE_ARGB(0xef,0x00,0x00,0x00)
#define UI_SUBITEM_COLOR        MAKE_ARGB(0xff,0xff,0xff,0xff)
#define UI_CLONE_COLOR          MAKE_ARGB(0xff,0x80,0x80,0x80)
#define UI_SELECTED_COLOR       MAKE_ARGB(0xff,0xff,0xff,0x00)
#define UI_SELECTED_BG_COLOR    MAKE_ARGB(0xef,0x80,0x80,0x00)
#define UI_MOUSEOVER_COLOR      MAKE_ARGB(0xff,0xff,0xff,0x80)
#define UI_MOUSEOVER_BG_COLOR   MAKE_ARGB(0x70,0x40,0x40,0x00)
#define UI_MOUSEDOWN_COLOR      MAKE_ARGB(0xff,0xff,0xff,0x80)
#define UI_MOUSEDOWN_BG_COLOR   MAKE_ARGB(0xb0,0x60,0x60,0x00)
#define UI_DIPSW_COLOR          MAKE_ARGB(0xff,0xff,0xff,0x00)
#define UI_SLIDER_COLOR         MAKE_ARGB(0xff,0xff,0xff,0xff)

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

typedef INT32 (*slider_update)(running_machine &machine, void *arg, astring *string, INT32 newval);

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



/***************************************************************************
    MACROS
***************************************************************************/

#define ui_draw_message_window(c, text) ui_draw_text_box(c, text, JUSTIFY_LEFT, 0.5f, 0.5f, UI_BACKGROUND_COLOR)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* main init/exit routines */
int ui_init(running_machine &machine);

/* display the startup screens */
int ui_display_startup_screens(running_machine &machine, int first_time, int show_disclaimer);

/* set the current text to display at startup */
void ui_set_startup_text(running_machine &machine, const char *text, int force);

/* once-per-frame update and render */
void ui_update_and_render(running_machine &machine, render_container *container);

/* returns the current UI font */
render_font *ui_get_font(running_machine &machine);

/* returns the line height of the font used by the UI system */
float ui_get_line_height(running_machine &machine);

/* returns the width of a character or string in the UI font */
float ui_get_char_width(running_machine &machine, unicode_char ch);
float ui_get_string_width(running_machine &machine, const char *s);

/* draw an outlined box filled with a given color */
void ui_draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor);

/* simple text draw at the given coordinates */
void ui_draw_text(render_container *container, const char *buf, float x, float y);

/* full-on text draw with all the options */
void ui_draw_text_full(render_container *container, const char *origs, float x, float y, float wrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight);

/* draw a multi-line message with a box around it */
void ui_draw_text_box(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor);

/* display a temporary message at the bottom of the screen */
void CLIB_DECL ui_popup_time(int seconds, const char *text, ...) ATTR_PRINTF(2,3);

/* get/set whether or not the FPS is displayed */
void ui_show_fps_temp(double seconds);
void ui_set_show_fps(int show);
int ui_get_show_fps(void);

/* get/set whether or not the profiler is displayed */
void ui_set_show_profiler(int show);
int ui_get_show_profiler(void);

/* force the menus to display */
void ui_show_menu(void);

/* force the mouse visibility status */
void ui_show_mouse(bool status);

/* return true if a menu is displayed */
int ui_is_menu_active(void);

/* print the game info string into a buffer */
astring &game_info_astring(running_machine &machine, astring &string);

/* get the list of sliders */
const slider_state *ui_get_slider_list(void);

/* paste */
void ui_paste(running_machine &machine);

/* returns whether the natural keyboard is active */
int ui_get_use_natural_keyboard(running_machine &machine);

/* specifies whether the natural keyboard is active */
void ui_set_use_natural_keyboard(running_machine &machine, int use_natural_keyboard);

#endif  /* __USRINTRF_H__ */
