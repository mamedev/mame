/***************************************************************************

    ui.h

    Functions used to handle MAME's crude user interface.

    Copyright (c) 1996-2007, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __USRINTRF_H__
#define __USRINTRF_H__

#include "mamecore.h"
#include "render.h"
#include "unicode.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* preferred font height; use ui_get_line_height() to get actual height */
#define UI_TARGET_FONT_HEIGHT	(1.0f / 25.0f)
#define UI_MAX_FONT_HEIGHT		(1.0f / 15.0f)

/* width of lines drawn in the UI */
#define UI_LINE_WIDTH			(1.0f / 500.0f)

/* border between outlines and inner text on left/right and top/bottom sides */
#define UI_BOX_LR_BORDER		(UI_TARGET_FONT_HEIGHT * 0.25f)
#define UI_BOX_TB_BORDER		(UI_TARGET_FONT_HEIGHT * 0.25f)

/* handy colors */
#define ARGB_WHITE				MAKE_ARGB(0xff,0xff,0xff,0xff)
#define ARGB_BLACK				MAKE_ARGB(0xff,0x00,0x00,0x00)
#define UI_FILLCOLOR			MAKE_ARGB(0xe0,0x10,0x10,0x30)
#define UI_YELLOWCOLOR			MAKE_ARGB(0xe0,0x60,0x60,0x10)
#define UI_REDCOLOR				MAKE_ARGB(0xf0,0x60,0x10,0x10)

/* cancel return value for a UI handler */
#define UI_HANDLER_CANCEL		((UINT32)~0)

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



/***************************************************************************
    MACROS
***************************************************************************/

#define ui_draw_message_window(text) ui_draw_text_box(text, JUSTIFY_LEFT, 0.5f, 0.5f, UI_FILLCOLOR)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* main init/exit routines */
int ui_init(running_machine *machine);

/* display the startup screens */
int ui_display_startup_screens(int first_time, int show_disclaimer);

/* set the current text to display at startup */
void ui_set_startup_text(const char *text, int force);

/* once-per-frame update and render */
void ui_update_and_render(void);

/* returns the current UI font */
render_font *ui_get_font(void);

/* returns the line height of the font used by the UI system */
float ui_get_line_height(void);

/* returns the width of a character or string in the UI font */
float ui_get_char_width(unicode_char ch);
float ui_get_string_width(const char *s);

/* draw an outlined box filled with a given color */
void ui_draw_outlined_box(float x0, float y0, float x1, float y1, rgb_t backcolor);

/* simple text draw at the given coordinates */
void ui_draw_text(const char *buf, float x, float y);

/* full-on text draw with all the options */
void ui_draw_text_full(const char *origs, float x, float y, float wrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight);

/* draw a multi-line message with a box around it */
void ui_draw_text_box(const char *text, int justify, float xpos, float ypos, rgb_t backcolor);

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

/* return true if a menu or the slider is displayed */
int ui_is_menu_active(void);
int ui_is_slider_active(void);

/* print the game info string into a buffer */
int sprintf_game_info(char *buffer);

#endif	/* __USRINTRF_H__ */
