/*********************************************************************

    ui.c

    Functions used to handle MAME's user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "video/vector.h"
#include "machine/laserdsc.h"
#include "profiler.h"
#include "render.h"
#include "cheat.h"
#include "rendfont.h"
#include "ui.h"
#include "uiinput.h"
#include "uimenu.h"
#include "uigfx.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	LOADSAVE_NONE,
	LOADSAVE_LOAD,
	LOADSAVE_SAVE
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

/* list of natural keyboard keys that are not associated with UI_EVENT_CHARs */
static const input_item_id non_char_keys[] =
{
	ITEM_ID_ESC,
	ITEM_ID_F1,
	ITEM_ID_F2,
	ITEM_ID_F3,
	ITEM_ID_F4,
	ITEM_ID_F5,
	ITEM_ID_F6,
	ITEM_ID_F7,
	ITEM_ID_F8,
	ITEM_ID_F9,
	ITEM_ID_F10,
	ITEM_ID_F11,
	ITEM_ID_F12,
	ITEM_ID_NUMLOCK,
	ITEM_ID_0_PAD,
	ITEM_ID_1_PAD,
	ITEM_ID_2_PAD,
	ITEM_ID_3_PAD,
	ITEM_ID_4_PAD,
	ITEM_ID_5_PAD,
	ITEM_ID_6_PAD,
	ITEM_ID_7_PAD,
	ITEM_ID_8_PAD,
	ITEM_ID_9_PAD,
	ITEM_ID_DEL_PAD,
	ITEM_ID_PLUS_PAD,
	ITEM_ID_MINUS_PAD,
	ITEM_ID_INSERT,
	ITEM_ID_DEL,
	ITEM_ID_HOME,
	ITEM_ID_END,
	ITEM_ID_PGUP,
	ITEM_ID_PGDN,
	ITEM_ID_UP,
	ITEM_ID_DOWN,
	ITEM_ID_LEFT,
	ITEM_ID_RIGHT,
	ITEM_ID_PAUSE,
	ITEM_ID_CANCEL
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* font for rendering */
static render_font *ui_font;

/* current UI handler */
static UINT32 (*ui_handler_callback)(running_machine *, render_container *, UINT32);
static UINT32 ui_handler_param;

/* flag to track single stepping */
static int single_step;

/* FPS counter display */
static int showfps;
static osd_ticks_t showfps_end;

/* profiler display */
static int show_profiler;

/* popup text display */
static osd_ticks_t popup_text_end;

/* messagebox buffer */
static astring messagebox_text;
static rgb_t messagebox_backcolor;

/* slider info */
static slider_state *slider_list;
static slider_state *slider_current;

/* natural keyboard info */
static int ui_use_natural_keyboard;
static UINT8 non_char_keys_down[(ARRAY_LENGTH(non_char_keys) + 7) / 8];


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_exit(running_machine &machine);

/* text generators */
static astring &disclaimer_string(running_machine *machine, astring &buffer);
static astring &warnings_string(running_machine *machine, astring &buffer);

/* UI handlers */
static UINT32 handler_messagebox(running_machine *machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_ok(running_machine *machine, render_container *container, UINT32 state);
static UINT32 handler_messagebox_anykey(running_machine *machine, render_container *container, UINT32 state);
static UINT32 handler_ingame(running_machine *machine, render_container *container, UINT32 state);
static UINT32 handler_load_save(running_machine *machine, render_container *container, UINT32 state);

/* slider controls */
static slider_state *slider_alloc(running_machine *machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg);
static slider_state *slider_init(running_machine *machine);
static INT32 slider_volume(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_mixervol(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_adjuster(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overclock(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_refresh(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_brightness(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_contrast(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_gamma(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xscale(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yscale(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_xoffset(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_yoffset(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxscale(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyscale(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overxoffset(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_overyoffset(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_flicker(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_beam(running_machine *machine, void *arg, astring *string, INT32 newval);
static char *slider_get_screen_desc(screen_device &screen);
static char *slider_get_laserdisc_desc(device_t *screen);
#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine *machine, void *arg, astring *string, INT32 newval);
static INT32 slider_crossoffset(running_machine *machine, void *arg, astring *string, INT32 newval);
#endif


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ui_set_handler - set a callback/parameter
    pair for the current UI handler
-------------------------------------------------*/

INLINE UINT32 ui_set_handler(UINT32 (*callback)(running_machine *, render_container *, UINT32), UINT32 param)
{
	ui_handler_callback = callback;
	ui_handler_param = param;
	return param;
}


/*-------------------------------------------------
    is_breakable_char - is a given unicode
    character a possible line break?
-------------------------------------------------*/

INLINE int is_breakable_char(unicode_char ch)
{
	/* regular spaces and hyphens are breakable */
	if (ch == ' ' || ch == '-')
		return TRUE;

	/* In the following character sets, any character is breakable:
        Hiragana (3040-309F)
        Katakana (30A0-30FF)
        Bopomofo (3100-312F)
        Hangul Compatibility Jamo (3130-318F)
        Kanbun (3190-319F)
        Bopomofo Extended (31A0-31BF)
        CJK Strokes (31C0-31EF)
        Katakana Phonetic Extensions (31F0-31FF)
        Enclosed CJK Letters and Months (3200-32FF)
        CJK Compatibility (3300-33FF)
        CJK Unified Ideographs Extension A (3400-4DBF)
        Yijing Hexagram Symbols (4DC0-4DFF)
        CJK Unified Ideographs (4E00-9FFF) */
	if (ch >= 0x3040 && ch <= 0x9fff)
		return TRUE;

	/* Hangul Syllables (AC00-D7AF) are breakable */
	if (ch >= 0xac00 && ch <= 0xd7af)
		return TRUE;

	/* CJK Compatibility Ideographs (F900-FAFF) are breakable */
	if (ch >= 0xf900 && ch <= 0xfaff)
		return TRUE;

	return FALSE;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ui_init - set up the user interface
-------------------------------------------------*/

int ui_init(running_machine *machine)
{
	/* make sure we clean up after ourselves */
	machine->add_notifier(MACHINE_NOTIFY_EXIT, ui_exit);

	/* allocate the font and messagebox string */
	ui_font = render_font_alloc("ui.bdf");

	/* initialize the other UI bits */
	ui_menu_init(machine);
	ui_gfx_init(machine);

	/* reset globals */
	single_step = FALSE;
	ui_set_handler(handler_messagebox, 0);
	/* retrieve options */
	ui_use_natural_keyboard = options_get_bool(machine->options(), OPTION_NATURAL_KEYBOARD);

	return 0;
}


/*-------------------------------------------------
    ui_exit - clean up ourselves on exit
-------------------------------------------------*/

static void ui_exit(running_machine &machine)
{
	/* free the font */
	if (ui_font != NULL)
		render_font_free(ui_font);
	ui_font = NULL;
}


/*-------------------------------------------------
    ui_display_startup_screens - display the
    various startup screens
-------------------------------------------------*/

int ui_display_startup_screens(running_machine *machine, int first_time, int show_disclaimer)
{
	const int maxstate = 3;
	int str = options_get_int(machine->options(), OPTION_SECONDS_TO_RUN);
	int show_gameinfo = !options_get_bool(machine->options(), OPTION_SKIP_GAMEINFO);
	int show_warnings = TRUE;
	int state;

	/* disable everything if we are using -str for 300 or fewer seconds, or if we're the empty driver,
       or if we are debugging */
	if (!first_time || (str > 0 && str < 60*5) || machine->gamedrv == &GAME_NAME(empty) || (machine->debug_flags & DEBUG_FLAG_ENABLED) != 0)
		show_gameinfo = show_warnings = show_disclaimer = FALSE;

	/* initialize the on-screen display system */
	slider_list = slider_current = slider_init(machine);

	/* loop over states */
	ui_set_handler(handler_ingame, 0);
	for (state = 0; state < maxstate && !machine->scheduled_event_pending() && !ui_menu_is_force_game_select(); state++)
	{
		/* default to standard colors */
		messagebox_backcolor = UI_BACKGROUND_COLOR;

		/* pick the next state */
		switch (state)
		{
			case 0:
				if (show_disclaimer && disclaimer_string(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_ok, 0);
				break;

			case 1:
				if (show_warnings && warnings_string(machine, messagebox_text).len() > 0)
				{
					ui_set_handler(handler_messagebox_ok, 0);
					if (machine->gamedrv->flags & (GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS | GAME_REQUIRES_ARTWORK | GAME_IMPERFECT_GRAPHICS | GAME_IMPERFECT_SOUND | GAME_NO_SOUND))
						messagebox_backcolor = UI_YELLOW_COLOR;
					if (machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
						messagebox_backcolor = UI_RED_COLOR;
				}
				break;

			case 2:
				if (show_gameinfo && game_info_astring(machine, messagebox_text).len() > 0)
					ui_set_handler(handler_messagebox_anykey, 0);
				break;
		}

		/* clear the input memory */
		input_code_poll_switches(machine, TRUE);
		while (input_code_poll_switches(machine, FALSE) != INPUT_CODE_INVALID) ;

		/* loop while we have a handler */
		while (ui_handler_callback != handler_ingame && !machine->scheduled_event_pending() && !ui_menu_is_force_game_select())
			video_frame_update(machine, FALSE);

		/* clear the handler and force an update */
		ui_set_handler(handler_ingame, 0);
		video_frame_update(machine, FALSE);
	}

	/* if we're the empty driver, force the menus on */
	if (ui_menu_is_force_game_select())
		ui_set_handler(ui_menu_ui_handler, 0);

	return 0;
}


/*-------------------------------------------------
    ui_set_startup_text - set the text to display
    at startup
-------------------------------------------------*/

void ui_set_startup_text(running_machine *machine, const char *text, int force)
{
	static osd_ticks_t lastupdatetime = 0;
	osd_ticks_t curtime = osd_ticks();

	/* copy in the new text */
	messagebox_text.cpy(text);
	messagebox_backcolor = UI_BACKGROUND_COLOR;

	/* don't update more than 4 times/second */
	if (force || (curtime - lastupdatetime) > osd_ticks_per_second() / 4)
	{
		lastupdatetime = curtime;
		video_frame_update(machine, FALSE);
	}
}


/*-------------------------------------------------
    ui_update_and_render - update the UI and
    render it; called by video.c
-------------------------------------------------*/

void ui_update_and_render(running_machine *machine, render_container *container)
{
	/* always start clean */
	render_container_empty(container);

	/* if we're paused, dim the whole screen */
	if (machine->phase() >= MACHINE_PHASE_RESET && (single_step || machine->paused()))
	{
		int alpha = (1.0f - options_get_float(machine->options(), OPTION_PAUSE_BRIGHTNESS)) * 255.0f;
		if (ui_menu_is_force_game_select())
			alpha = 255;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			render_container_add_rect(container, 0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(alpha,0x00,0x00,0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}

	/* render any cheat stuff at the bottom */
	cheat_render_text(machine, container);

	/* call the current UI handler */
	assert(ui_handler_callback != NULL);
	ui_handler_param = (*ui_handler_callback)(machine, container, ui_handler_param);

	/* display any popup messages */
	if (osd_ticks() < popup_text_end)
		ui_draw_text_box(container, messagebox_text, JUSTIFY_CENTER, 0.5f, 0.9f, messagebox_backcolor);
	else
		popup_text_end = 0;

	/* cancel takes us back to the ingame handler */
	if (ui_handler_param == UI_HANDLER_CANCEL)
		ui_set_handler(handler_ingame, 0);
}


/*-------------------------------------------------
    ui_get_font - return the UI font
-------------------------------------------------*/

render_font *ui_get_font(void)
{
	return ui_font;
}


/*-------------------------------------------------
    ui_get_line_height - return the current height
    of a line
-------------------------------------------------*/

float ui_get_line_height(void)
{
	INT32 raw_font_pixel_height = render_font_get_pixel_height(ui_font);
	INT32 target_pixel_width, target_pixel_height;
	float one_to_one_line_height;
	float target_aspect;
	float scale_factor;

	/* get info about the UI target */
	render_target_get_bounds(render_get_ui_target(), &target_pixel_width, &target_pixel_height, &target_aspect);

	/* compute the font pixel height at the nominal size */
	one_to_one_line_height = (float)raw_font_pixel_height / (float)target_pixel_height;

	/* determine the scale factor */
	scale_factor = UI_TARGET_FONT_HEIGHT / one_to_one_line_height;

	/* if our font is small-ish, do integral scaling */
	if (raw_font_pixel_height < 24)
	{
		/* do we want to scale smaller? only do so if we exceed the threshhold */
		if (scale_factor <= 1.0f)
		{
			if (one_to_one_line_height < UI_MAX_FONT_HEIGHT || raw_font_pixel_height < 12)
				scale_factor = 1.0f;
		}

		/* otherwise, just ensure an integral scale factor */
		else
			scale_factor = floor(scale_factor);
	}

	/* otherwise, just make sure we hit an even number of pixels */
	else
	{
		INT32 height = scale_factor * one_to_one_line_height * (float)target_pixel_height;
		scale_factor = (float)height / (one_to_one_line_height * (float)target_pixel_height);
	}

	return scale_factor * one_to_one_line_height;
}


/*-------------------------------------------------
    ui_get_char_width - return the width of a
    single character
-------------------------------------------------*/

float ui_get_char_width(unicode_char ch)
{
	return render_font_get_char_width(ui_font, ui_get_line_height(), render_get_ui_aspect(), ch);
}


/*-------------------------------------------------
    ui_get_string_width - return the width of a
    character string
-------------------------------------------------*/

float ui_get_string_width(const char *s)
{
	return render_font_get_utf8string_width(ui_font, ui_get_line_height(), render_get_ui_aspect(), s);
}


/*-------------------------------------------------
    ui_draw_outlined_box - add primitives to draw
    an outlined box with the given background
    color
-------------------------------------------------*/

void ui_draw_outlined_box(render_container *container, float x0, float y0, float x1, float y1, rgb_t backcolor)
{
	render_container_add_rect(container, x0, y0, x1, y1, backcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_container_add_line(container, x0, y0, x1, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_container_add_line(container, x1, y0, x1, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_container_add_line(container, x1, y1, x0, y1, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	render_container_add_line(container, x0, y1, x0, y0, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}


/*-------------------------------------------------
    ui_draw_text - simple text renderer
-------------------------------------------------*/

void ui_draw_text(render_container *container, const char *buf, float x, float y)
{
	ui_draw_text_full(container, buf, x, y, 1.0f - x, JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/*-------------------------------------------------
    ui_draw_text_full - full featured text
    renderer with word wrapping, justification,
    and full size computation
-------------------------------------------------*/

void ui_draw_text_full(render_container *container, const char *origs, float x, float y, float origwrapwidth, int justify, int wrap, int draw, rgb_t fgcolor, rgb_t bgcolor, float *totalwidth, float *totalheight)
{
	float lineheight = ui_get_line_height();
	const char *ends = origs + strlen(origs);
	float wrapwidth = origwrapwidth;
	const char *s = origs;
	const char *linestart;
	float cury = y;
	float maxwidth = 0;

	/* if we don't want wrapping, guarantee a huge wrapwidth */
	if (wrap == WRAP_NEVER)
		wrapwidth = 1000000.0f;
	if (wrapwidth <= 0)
		return;

	/* loop over lines */
	while (*s != 0)
	{
		const char *lastbreak = NULL;
		int line_justify = justify;
		unicode_char schar;
		int scharcount;
		float lastbreak_width = 0;
		float curwidth = 0;
		float curx = x;

		/* get the current character */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		/* if the line starts with a tab character, center it regardless */
		if (schar == '\t')
		{
			s += scharcount;
			line_justify = JUSTIFY_CENTER;
		}

		/* remember the starting position of the line */
		linestart = s;

		/* loop while we have characters and are less than the wrapwidth */
		while (*s != 0 && curwidth <= wrapwidth)
		{
			float chwidth;

			/* get the current chcaracter */
			scharcount = uchar_from_utf8(&schar, s, ends - s);
			if (scharcount == -1)
				break;

			/* if we hit a newline, stop immediately */
			if (schar == '\n')
				break;

			/* get the width of this character */
			chwidth = ui_get_char_width(schar);

			/* if we hit a space, remember the location and width *without* the space */
			if (schar == ' ')
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}

			/* add the width of this character and advance */
			curwidth += chwidth;
			s += scharcount;

			/* if we hit any non-space breakable character, remember the location and width
               *with* the breakable character */
			if (schar != ' ' && is_breakable_char(schar) && curwidth <= wrapwidth)
			{
				lastbreak = s;
				lastbreak_width = curwidth;
			}
		}

		/* if we accumulated too much for the current width, we need to back off */
		if (curwidth > wrapwidth)
		{
			/* if we're word wrapping, back up to the last break if we can */
			if (wrap == WRAP_WORD)
			{
				/* if we hit a break, back up to there with the appropriate width */
				if (lastbreak != NULL)
				{
					s = lastbreak;
					curwidth = lastbreak_width;
				}

				/* if we didn't hit a break, back up one character */
				else if (s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					curwidth -= ui_get_char_width(schar);
				}
			}

			/* if we're truncating, make sure we have enough space for the ... */
			else if (wrap == WRAP_TRUNCATE)
			{
				/* add in the width of the ... */
				curwidth += 3.0f * ui_get_char_width('.');

				/* while we are above the wrap width, back up one character */
				while (curwidth > wrapwidth && s > linestart)
				{
					/* get the previous character */
					s = (const char *)utf8_previous_char(s);
					scharcount = uchar_from_utf8(&schar, s, ends - s);
					if (scharcount == -1)
						break;

					curwidth -= ui_get_char_width(schar);
				}
			}
		}

		/* align according to the justfication */
		if (line_justify == JUSTIFY_CENTER)
			curx += (origwrapwidth - curwidth) * 0.5f;
		else if (line_justify == JUSTIFY_RIGHT)
			curx += origwrapwidth - curwidth;

		/* track the maximum width of any given line */
		if (curwidth > maxwidth)
			maxwidth = curwidth;

		/* if opaque, add a black box */
		if (draw == DRAW_OPAQUE)
			render_container_add_rect(container, curx, cury, curx + curwidth, cury + lineheight, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* loop from the line start and add the characters */
		while (linestart < s)
		{
			/* get the current character */
			unicode_char linechar;
			int linecharcount = uchar_from_utf8(&linechar, linestart, ends - linestart);
			if (linecharcount == -1)
				break;

			if (draw != DRAW_NONE)
			{
				render_container_add_char(container, curx, cury, lineheight, render_get_ui_aspect(), fgcolor, ui_font, linechar);
				curx += ui_get_char_width(linechar);
			}
			linestart += linecharcount;
		}

		/* append ellipses if needed */
		if (wrap == WRAP_TRUNCATE && *s != 0 && draw != DRAW_NONE)
		{
			render_container_add_char(container, curx, cury, lineheight, render_get_ui_aspect(), fgcolor, ui_font, '.');
			curx += ui_get_char_width('.');
			render_container_add_char(container, curx, cury, lineheight, render_get_ui_aspect(), fgcolor, ui_font, '.');
			curx += ui_get_char_width('.');
			render_container_add_char(container, curx, cury, lineheight, render_get_ui_aspect(), fgcolor, ui_font, '.');
			curx += ui_get_char_width('.');
		}

		/* if we're not word-wrapping, we're done */
		if (wrap != WRAP_WORD)
			break;

		/* advance by a row */
		cury += lineheight;

		/* skip past any spaces at the beginning of the next line */
		scharcount = uchar_from_utf8(&schar, s, ends - s);
		if (scharcount == -1)
			break;

		if (schar == '\n')
			s += scharcount;
		else
			while (*s && isspace(schar))
			{
				s += scharcount;
				scharcount = uchar_from_utf8(&schar, s, ends - s);
				if (scharcount == -1)
					break;
			}
	}

	/* report the width and height of the resulting space */
	if (totalwidth)
		*totalwidth = maxwidth;
	if (totalheight)
		*totalheight = cury - y;
}


/*-------------------------------------------------
    ui_draw_text_box - draw a multiline text
    message with a box around it
-------------------------------------------------*/

void ui_draw_text_box(render_container *container, const char *text, int justify, float xpos, float ypos, rgb_t backcolor)
{
	float target_width, target_height;
	float target_x, target_y;

	/* compute the multi-line target width/height */
	ui_draw_text_full(container, text, 0, 0, 1.0f - 2.0f * UI_BOX_LR_BORDER,
				justify, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);
	if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
		target_height = floor((1.0f - 2.0f * UI_BOX_TB_BORDER) / ui_get_line_height()) * ui_get_line_height();

	/* determine the target location */
	target_x = xpos - 0.5f * target_width;
	target_y = ypos - 0.5f * target_height;

	/* make sure we stay on-screen */
	if (target_x < UI_BOX_LR_BORDER)
		target_x = UI_BOX_LR_BORDER;
	if (target_x + target_width + UI_BOX_LR_BORDER > 1.0f)
		target_x = 1.0f - UI_BOX_LR_BORDER - target_width;
	if (target_y < UI_BOX_TB_BORDER)
		target_y = UI_BOX_TB_BORDER;
	if (target_y + target_height + UI_BOX_TB_BORDER > 1.0f)
		target_y = 1.0f - UI_BOX_TB_BORDER - target_height;

	/* add a box around that */
	ui_draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
					 target_y - UI_BOX_TB_BORDER,
					 target_x + target_width + UI_BOX_LR_BORDER,
					 target_y + target_height + UI_BOX_TB_BORDER, backcolor);
	ui_draw_text_full(container, text, target_x, target_y, target_width,
				justify, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
}


/*-------------------------------------------------
    ui_popup_time - popup a message for a specific
    amount of time
-------------------------------------------------*/

void CLIB_DECL ui_popup_time(int seconds, const char *text, ...)
{
	va_list arg;

	/* extract the text */
	va_start(arg,text);
	messagebox_text.vprintf(text, arg);
	messagebox_backcolor = UI_BACKGROUND_COLOR;
	va_end(arg);

	/* set a timer */
	popup_text_end = osd_ticks() + osd_ticks_per_second() * seconds;
}


/*-------------------------------------------------
    ui_show_fps_temp - show the FPS counter for
    a specific period of time
-------------------------------------------------*/

void ui_show_fps_temp(double seconds)
{
	if (!showfps)
		showfps_end = osd_ticks() + seconds * osd_ticks_per_second();
}


/*-------------------------------------------------
    ui_set_show_fps - show/hide the FPS counter
-------------------------------------------------*/

void ui_set_show_fps(int show)
{
	showfps = show;
	if (!show)
	{
		showfps = 0;
		showfps_end = 0;
	}
}


/*-------------------------------------------------
    ui_get_show_fps - return the current FPS
    counter visibility state
-------------------------------------------------*/

int ui_get_show_fps(void)
{
	return showfps || (showfps_end != 0);
}


/*-------------------------------------------------
    ui_set_show_profiler - show/hide the profiler
-------------------------------------------------*/

void ui_set_show_profiler(int show)
{
	if (show)
	{
		show_profiler = TRUE;
		profiler_start();
	}
	else
	{
		show_profiler = FALSE;
		profiler_stop();
	}
}


/*-------------------------------------------------
    ui_get_show_profiler - return the current
    profiler visibility state
-------------------------------------------------*/

int ui_get_show_profiler(void)
{
	return show_profiler;
}


/*-------------------------------------------------
    ui_show_menu - show the menus
-------------------------------------------------*/

void ui_show_menu(void)
{
	ui_set_handler(ui_menu_ui_handler, 0);
}


/*-------------------------------------------------
    ui_is_menu_active - return TRUE if the menu
    UI handler is active
-------------------------------------------------*/

int ui_is_menu_active(void)
{
	return (ui_handler_callback == ui_menu_ui_handler);
}



/***************************************************************************
    TEXT GENERATORS
***************************************************************************/

/*-------------------------------------------------
    disclaimer_string - print the disclaimer
    text to the given buffer
-------------------------------------------------*/

static astring &disclaimer_string(running_machine *machine, astring &string)
{
	string.cpy("Usage of emulators in conjunction with ROMs you don't own is forbidden by copyright law.\n\n");
	string.catprintf("IF YOU ARE NOT LEGALLY ENTITLED TO PLAY \"%s\" ON THIS EMULATOR, PRESS ESC.\n\n", machine->gamedrv->description);
	string.cat("Otherwise, type OK or move the joystick left then right to continue");
	return string;
}


/*-------------------------------------------------
    warnings_string - print the warning flags
    text to the given buffer
-------------------------------------------------*/

static astring &warnings_string(running_machine *machine, astring &string)
{
#define WARNING_FLAGS (	GAME_NOT_WORKING | \
						GAME_UNEMULATED_PROTECTION | \
						GAME_WRONG_COLORS | \
						GAME_IMPERFECT_COLORS | \
						GAME_REQUIRES_ARTWORK | \
						GAME_NO_SOUND |  \
						GAME_IMPERFECT_SOUND |  \
						GAME_IMPERFECT_GRAPHICS | \
						GAME_NO_COCKTAIL)
	int i;

	string.reset();

	/* if no warnings, nothing to return */
	if (rom_load_warnings(machine) == 0 && !(machine->gamedrv->flags & WARNING_FLAGS))
		return string;

	/* add a warning if any ROMs were loaded with warnings */
	if (rom_load_warnings(machine) > 0)
	{
		string.cat("One or more ROMs/CHDs for this game are incorrect. The " GAMENOUN " may not run correctly.\n");
		if (machine->gamedrv->flags & WARNING_FLAGS)
			string.cat("\n");
	}

	/* if we have at least one warning flag, print the general header */
	if (machine->gamedrv->flags & WARNING_FLAGS)
	{
		string.cat("There are known problems with this " GAMENOUN "\n\n");

		/* add one line per warning flag */
		if (input_machine_has_keyboard(machine))
			string.cat("The keyboard emulation may not be 100% accurate.\n");
		if (machine->gamedrv->flags & GAME_IMPERFECT_COLORS)
			string.cat("The colors aren't 100% accurate.\n");
		if (machine->gamedrv->flags & GAME_WRONG_COLORS)
			string.cat("The colors are completely wrong.\n");
		if (machine->gamedrv->flags & GAME_IMPERFECT_GRAPHICS)
			string.cat("The video emulation isn't 100% accurate.\n");
		if (machine->gamedrv->flags & GAME_IMPERFECT_SOUND)
			string.cat("The sound emulation isn't 100% accurate.\n");
		if (machine->gamedrv->flags & GAME_NO_SOUND)
			string.cat("The game lacks sound.\n");
		if (machine->gamedrv->flags & GAME_NO_COCKTAIL)
			string.cat("Screen flipping in cocktail mode is not supported.\n");

		/* check if external artwork is present before displaying this warning? */
		if (machine->gamedrv->flags & GAME_REQUIRES_ARTWORK)
			string.cat("The game requires external artwork files\n");

		/* if there's a NOT WORKING or UNEMULATED PROTECTION warning, make it stronger */
		if (machine->gamedrv->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION))
		{
			const game_driver *maindrv;
			const game_driver *clone_of;
			int foundworking;

			/* add the strings for these warnings */
			if (machine->gamedrv->flags & GAME_UNEMULATED_PROTECTION)
				string.cat("The game has protection which isn't fully emulated.\n");
			if (machine->gamedrv->flags & GAME_NOT_WORKING)
				string.cat("THIS " CAPGAMENOUN " DOESN'T WORK. The emulation for this game is not yet complete. "
									 "There is nothing you can do to fix this problem except wait for the developers to improve the emulation.\n");

			/* find the parent of this driver */
			clone_of = driver_get_clone(machine->gamedrv);
			if (clone_of != NULL && !(clone_of->flags & GAME_IS_BIOS_ROOT))
				maindrv = clone_of;
			else
				maindrv = machine->gamedrv;

			/* scan the driver list for any working clones and add them */
			foundworking = FALSE;
			for (i = 0; drivers[i] != NULL; i++)
				if (drivers[i] == maindrv || driver_get_clone(drivers[i]) == maindrv)
					if ((drivers[i]->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) == 0)
					{
						/* this one works, add a header and display the name of the clone */
						if (!foundworking)
							string.cat("\n\nThere are working clones of this game: ");
						else
							string.cat(", ");
						string.cat(drivers[i]->name);
						foundworking = TRUE;
					}

			if (foundworking)
				string.cat("\n");
		}
	}

	/* add the 'press OK' string */
	string.cat("\n\nType OK or move the joystick left then right to continue");
	return string;
}


/*-------------------------------------------------
    game_info_astring - populate an allocated
    string with the game info text
-------------------------------------------------*/

astring &game_info_astring(running_machine *machine, astring &string)
{
	int scrcount = screen_count(*machine->config);
	int found_sound = FALSE;

	/* print description, manufacturer, and CPU: */
	string.printf("%s\n%s %s\n\nCPU:\n", machine->gamedrv->description, machine->gamedrv->year, machine->gamedrv->manufacturer);

	/* loop over all CPUs */
	device_execute_interface *exec;
	for (bool gotone = machine->m_devicelist.first(exec); gotone; gotone = exec->next(exec))
	{
		/* get cpu specific clock that takes internal multiplier/dividers into account */
		int clock = exec->device().clock();

		/* count how many identical CPUs we have */
		int count = 1;
		device_execute_interface *scan;
		for (bool gotone = exec->next(scan); gotone; gotone = scan->next(scan))
		{
			if (exec->device().type() != scan->device().type() || exec->device().clock() != scan->device().clock())
				break;
			count++;
			exec = scan;
		}

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(exec->device().name());

		/* display clock in kHz or MHz */
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
	}

	/* loop over all sound chips */
	device_sound_interface *sound = NULL;
	for (bool gotone = machine->m_devicelist.first(sound); gotone; gotone = sound->next(sound))
	{
		/* append the Sound: string */
		if (!found_sound)
			string.cat("\nSound:\n");
		found_sound = TRUE;

		/* count how many identical sound chips we have */
		int count = 1;
		device_sound_interface *scan;
		for (bool gotanother = sound->next(scan); gotanother; gotanother = scan->next(scan))
		{
			if (sound->device().type() != scan->device().type() || sound->device().clock() != scan->device().clock())
				break;
			count++;
			sound = scan;
		}

		/* if more than one, prepend a #x in front of the CPU name */
		if (count > 1)
			string.catprintf("%d" UTF8_MULTIPLY, count);
		string.cat(sound->device().name());

		/* display clock in kHz or MHz */
		int clock = sound->device().clock();
		if (clock >= 1000000)
			string.catprintf(" %d.%06d" UTF8_NBSP "MHz\n", clock / 1000000, clock % 1000000);
		else if (clock != 0)
			string.catprintf(" %d.%03d" UTF8_NBSP "kHz\n", clock / 1000, clock % 1000);
		else
			string.cat("\n");
	}

	/* display screen information */
	string.cat("\nVideo:\n");
	if (scrcount == 0)
		string.cat("None\n");
	else
	{
		for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		{
			if (scrcount > 1)
			{
				string.cat(slider_get_screen_desc(*screen));
				string.cat(": ");
			}

			if (screen->screen_type() == SCREEN_TYPE_VECTOR)
				string.cat("Vector\n");
			else
			{
				const rectangle &visarea = screen->visible_area();

				string.catprintf("%d " UTF8_MULTIPLY " %d (%s) %f" UTF8_NBSP "Hz\n",
						visarea.max_x - visarea.min_x + 1,
						visarea.max_y - visarea.min_y + 1,
						(machine->gamedrv->flags & ORIENTATION_SWAP_XY) ? "V" : "H",
						ATTOSECONDS_TO_HZ(screen->frame_period().attoseconds));
			}
		}
	}

	return string;
}



/***************************************************************************
    UI HANDLERS
***************************************************************************/

/*-------------------------------------------------
    handler_messagebox - displays the current
    messagebox_text string but handles no input
-------------------------------------------------*/

static UINT32 handler_messagebox(running_machine *machine, render_container *container, UINT32 state)
{
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);
	return 0;
}


/*-------------------------------------------------
    handler_messagebox_ok - displays the current
    messagebox_text string and waits for an OK
-------------------------------------------------*/

static UINT32 handler_messagebox_ok(running_machine *machine, render_container *container, UINT32 state)
{
	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* an 'O' or left joystick kicks us to the next state */
	if (state == 0 && (input_code_pressed_once(machine, KEYCODE_O) || ui_input_pressed(machine, IPT_UI_LEFT)))
		state++;

	/* a 'K' or right joystick exits the state */
	else if (state == 1 && (input_code_pressed_once(machine, KEYCODE_K) || ui_input_pressed(machine, IPT_UI_RIGHT)))
		state = UI_HANDLER_CANCEL;

	/* if the user cancels, exit out completely */
	else if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine->schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	return state;
}


/*-------------------------------------------------
    handler_messagebox_anykey - displays the
    current messagebox_text string and waits for
    any keypress
-------------------------------------------------*/

static UINT32 handler_messagebox_anykey(running_machine *machine, render_container *container, UINT32 state)
{
	/* draw a standard message window */
	ui_draw_text_box(container, messagebox_text, JUSTIFY_LEFT, 0.5f, 0.5f, messagebox_backcolor);

	/* if the user cancels, exit out completely */
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		machine->schedule_exit();
		state = UI_HANDLER_CANCEL;
	}

	/* if any key is pressed, just exit */
	else if (input_code_poll_switches(machine, FALSE) != INPUT_CODE_INVALID)
		state = UI_HANDLER_CANCEL;

	return state;
}


/*-------------------------------------------------
    ui_use_newui - determines if "newui" is in use
-------------------------------------------------*/

int ui_use_newui( void )
{
	#ifdef MESS
	#if (defined(WIN32) || defined(_MSC_VER)) && !defined(SDLMAME_WIN32)
		if (options_get_bool(mame_options(), "newui"))
			return TRUE;
	#endif
	#endif
	return FALSE;
}

/*-------------------------------------------------
    process_natural_keyboard - processes any
    natural keyboard input
-------------------------------------------------*/

static void process_natural_keyboard(running_machine *machine)
{
	ui_event event;
	int i, pressed;
	input_item_id itemid;
	input_code code;
	UINT8 *key_down_ptr;
	UINT8 key_down_mask;

	/* loop while we have interesting events */
	while (ui_input_pop_event(machine, &event))
	{
		/* if this was a UI_EVENT_CHAR event, post it */
		if (event.event_type == UI_EVENT_CHAR)
			inputx_postc(machine, event.ch);
	}

	/* process natural keyboard keys that don't get UI_EVENT_CHARs */
	for (i = 0; i < ARRAY_LENGTH(non_char_keys); i++)
	{
		/* identify this keycode */
		itemid = non_char_keys[i];
		code = input_code_from_input_item_id(machine, itemid);

		/* ...and determine if it is pressed */
		pressed = input_code_pressed(machine, code);

		/* figure out whey we are in the key_down map */
		key_down_ptr = &non_char_keys_down[i / 8];
		key_down_mask = 1 << (i % 8);

		if (pressed && !(*key_down_ptr & key_down_mask))
		{
			/* this key is now down */
			*key_down_ptr |= key_down_mask;

			/* post the key */
			inputx_postc(machine, UCHAR_MAMEKEY_BEGIN + code);
		}
		else if (!pressed && (*key_down_ptr & key_down_mask))
		{
			/* this key is now up */
			*key_down_ptr &= ~key_down_mask;
		}
	}
}

/*-------------------------------------------------
    ui_paste - does a paste from the keyboard
-------------------------------------------------*/

void ui_paste(running_machine *machine)
{
	/* retrieve the clipboard text */
	char *text = osd_get_clipboard_text();

	/* was a result returned? */
	if (text != NULL)
	{
		/* post the text */
		inputx_post_utf8(machine, text);

		/* free the string */
		osd_free(text);
	}
}

/*-------------------------------------------------
    ui_image_handler_ingame - execute display
    callback function for each image device
-------------------------------------------------*/

void ui_image_handler_ingame(running_machine *machine)
{
	device_image_interface *image = NULL;

	/* run display routine for devices */
	if (machine->phase() == MACHINE_PHASE_RUNNING)
	{
		for (bool gotone = machine->m_devicelist.first(image); gotone; gotone = image->next(image))
		{
			image->call_display();
		}
	}

}

/*-------------------------------------------------
    handler_ingame - in-game handler takes care
    of the standard keypresses
-------------------------------------------------*/

static UINT32 handler_ingame(running_machine *machine, render_container *container, UINT32 state)
{
	bool is_paused = machine->paused();

	/* first draw the FPS counter */
	if (showfps || osd_ticks() < showfps_end)
	{
		ui_draw_text_full(container, video_get_speed_text(machine), 0.0f, 0.0f, 1.0f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}
	else
		showfps_end = 0;

	/* draw the profiler if visible */
	if (show_profiler)
	{
		astring profilertext;
		profiler_get_text(machine, profilertext);
		ui_draw_text_full(container, profilertext, 0.0f, 0.0f, 1.0f, JUSTIFY_LEFT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
	}

	/* if we're single-stepping, pause now */
	if (single_step)
	{
		machine->pause();
		single_step = FALSE;
	}

	/* determine if we should disable the rest of the UI */
	int ui_disabled = (input_machine_has_keyboard(machine) && !machine->ui_active);

	/* is ScrLk UI toggling applicable here? */
	if (input_machine_has_keyboard(machine))
	{
		/* are we toggling the UI with ScrLk? */
		if (ui_input_pressed(machine, IPT_UI_TOGGLE_UI))
		{
			/* toggle the UI */
			machine->ui_active = !machine->ui_active;

			/* display a popup indicating the new status */
			if (machine->ui_active)
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: PARTIAL Emulation",
					"UI:   Enabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
			else
			{
				ui_popup_time(2, "%s\n%s\n%s\n%s\n%s\n%s\n",
					"Keyboard Emulation Status",
					"-------------------------",
					"Mode: FULL Emulation",
					"UI:   Disabled",
					"-------------------------",
					"**Use ScrLock to toggle**");
			}
		}
	}

	/* is the natural keyboard enabled? */
	if (ui_get_use_natural_keyboard(machine) && (machine->phase() == MACHINE_PHASE_RUNNING))
		process_natural_keyboard(machine);

	if (!ui_disabled)
	{
		/* paste command */
		if (ui_input_pressed(machine, IPT_UI_PASTE))
			ui_paste(machine);
	}

	ui_image_handler_ingame(machine);

	if (ui_disabled) return ui_disabled;

	/* if the user pressed ESC, stop the emulation (except in MESS with newui, where ESC toggles the menubar) */
	if (ui_input_pressed(machine, IPT_UI_CANCEL) && !ui_use_newui())
		machine->schedule_exit();

	/* turn on menus if requested */
	if (ui_input_pressed(machine, IPT_UI_CONFIGURE) && !ui_use_newui())
		return ui_set_handler(ui_menu_ui_handler, 0);

	/* if the on-screen display isn't up and the user has toggled it, turn it on */
	if ((machine->debug_flags & DEBUG_FLAG_ENABLED) == 0 && ui_input_pressed(machine, IPT_UI_ON_SCREEN_DISPLAY))
		return ui_set_handler(ui_slider_ui_handler, 1);

	/* handle a reset request */
	if (ui_input_pressed(machine, IPT_UI_RESET_MACHINE))
		machine->schedule_hard_reset();
	if (ui_input_pressed(machine, IPT_UI_SOFT_RESET))
		machine->schedule_soft_reset();

	/* handle a request to display graphics/palette */
	if (ui_input_pressed(machine, IPT_UI_SHOW_GFX))
	{
		if (!is_paused)
			machine->pause();
		return ui_set_handler(ui_gfx_ui_handler, is_paused);
	}

	/* handle a save state request */
	if (ui_input_pressed(machine, IPT_UI_SAVE_STATE))
	{
		machine->pause();
		return ui_set_handler(handler_load_save, LOADSAVE_SAVE);
	}

	/* handle a load state request */
	if (ui_input_pressed(machine, IPT_UI_LOAD_STATE))
	{
		machine->pause();
		return ui_set_handler(handler_load_save, LOADSAVE_LOAD);
	}

	/* handle a save snapshot request */
	if (ui_input_pressed(machine, IPT_UI_SNAPSHOT))
		video_save_active_screen_snapshots(machine);

	/* toggle pause */
	if (ui_input_pressed(machine, IPT_UI_PAUSE))
	{
		/* with a shift key, it is single step */
		if (is_paused && (input_code_pressed(machine, KEYCODE_LSHIFT) || input_code_pressed(machine, KEYCODE_RSHIFT)))
		{
			single_step = TRUE;
			machine->resume();
		}
		else if (machine->paused())
			machine->resume();
		else
			machine->pause();
	}

	/* handle a toggle cheats request */
	if (ui_input_pressed(machine, IPT_UI_TOGGLE_CHEAT))
		cheat_set_global_enable(machine, !cheat_get_global_enable(machine));

	/* toggle movie recording */
	if (ui_input_pressed(machine, IPT_UI_RECORD_MOVIE))
	{
		if (!video_mng_is_movie_active(machine))
		{
			video_mng_begin_recording(machine, NULL);
			popmessage("REC START");
		}
		else
		{
			video_mng_end_recording(machine);
			popmessage("REC STOP");
		}
	}

	/* toggle profiler display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_PROFILER))
		ui_set_show_profiler(!ui_get_show_profiler());

	/* toggle FPS display */
	if (ui_input_pressed(machine, IPT_UI_SHOW_FPS))
		ui_set_show_fps(!ui_get_show_fps());

	/* increment frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_INC))
	{
		/* get the current value and increment it */
		int newframeskip = video_get_frameskip() + 1;
		if (newframeskip > MAX_FRAMESKIP)
			newframeskip = -1;
		video_set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* decrement frameskip? */
	if (ui_input_pressed(machine, IPT_UI_FRAMESKIP_DEC))
	{
		/* get the current value and decrement it */
		int newframeskip = video_get_frameskip() - 1;
		if (newframeskip < -1)
			newframeskip = MAX_FRAMESKIP;
		video_set_frameskip(newframeskip);

		/* display the FPS counter for 2 seconds */
		ui_show_fps_temp(2.0);
	}

	/* toggle throttle? */
	if (ui_input_pressed(machine, IPT_UI_THROTTLE))
		video_set_throttle(!video_get_throttle());

	/* check for fast forward */
	if (input_type_pressed(machine, IPT_UI_FAST_FORWARD, 0))
	{
		video_set_fastforward(TRUE);
		ui_show_fps_temp(0.5);
	}
	else
		video_set_fastforward(FALSE);

	return 0;
}


/*-------------------------------------------------
    handler_load_save - leads the user through
    specifying a game to save or load
-------------------------------------------------*/

static UINT32 handler_load_save(running_machine *machine, render_container *container, UINT32 state)
{
	char filename[20];
	input_code code;
	char file = 0;

	/* if we're not in the middle of anything, skip */
	if (state == LOADSAVE_NONE)
		return 0;

	/* okay, we're waiting for a key to select a slot; display a message */
	if (state == LOADSAVE_SAVE)
		ui_draw_message_window(container, "Select position to save to");
	else
		ui_draw_message_window(container, "Select position to load from");

	/* check for cancel key */
	if (ui_input_pressed(machine, IPT_UI_CANCEL))
	{
		/* display a popup indicating things were cancelled */
		if (state == LOADSAVE_SAVE)
			popmessage("Save cancelled");
		else
			popmessage("Load cancelled");

		/* reset the state */
		machine->resume();
		return UI_HANDLER_CANCEL;
	}

	/* check for A-Z or 0-9 */
	for (code = KEYCODE_A; code <= (input_code)KEYCODE_Z; code++)
		if (input_code_pressed_once(machine, code))
			file = code - KEYCODE_A + 'a';
	if (file == 0)
		for (code = KEYCODE_0; code <= (input_code)KEYCODE_9; code++)
			if (input_code_pressed_once(machine, code))
				file = code - KEYCODE_0 + '0';
	if (file == 0)
		for (code = KEYCODE_0_PAD; code <= (input_code)KEYCODE_9_PAD; code++)
			if (input_code_pressed_once(machine, code))
				file = code - KEYCODE_0_PAD + '0';
	if (file == 0)
		return state;

	/* display a popup indicating that the save will proceed */
	sprintf(filename, "%c", file);
	if (state == LOADSAVE_SAVE)
	{
		popmessage("Save to position %c", file);
		machine->schedule_save(filename);
	}
	else
	{
		popmessage("Load from position %c", file);
		machine->schedule_load(filename);
	}

	/* remove the pause and reset the state */
	machine->resume();
	return UI_HANDLER_CANCEL;
}



/***************************************************************************
    SLIDER CONTROLS
***************************************************************************/

/*-------------------------------------------------
    ui_get_slider_list - get the list of sliders
-------------------------------------------------*/

const slider_state *ui_get_slider_list(void)
{
	return slider_list;
}


/*-------------------------------------------------
    slider_alloc - allocate a new slider entry
-------------------------------------------------*/

static slider_state *slider_alloc(running_machine *machine, const char *title, INT32 minval, INT32 defval, INT32 maxval, INT32 incval, slider_update update, void *arg)
{
	int size = sizeof(slider_state) + strlen(title);
	slider_state *state = (slider_state *)auto_alloc_array_clear(machine, UINT8, size);

	state->minval = minval;
	state->defval = defval;
	state->maxval = maxval;
	state->incval = incval;
	state->update = update;
	state->arg = arg;
	strcpy(state->description, title);

	return state;
}


/*-------------------------------------------------
    slider_init - initialize the list of slider
    controls
-------------------------------------------------*/

static slider_state *slider_init(running_machine *machine)
{
	const input_field_config *field;
	const input_port_config *port;
	device_t *device;
	slider_state *listhead = NULL;
	slider_state **tailptr = &listhead;
	astring string;
	int numitems, item;

	/* add overall volume */
	*tailptr = slider_alloc(machine, "Master Volume", -32, 0, 0, 1, slider_volume, NULL);
	tailptr = &(*tailptr)->next;

	/* add per-channel volume */
	numitems = sound_get_user_gain_count(machine);
	for (item = 0; item < numitems; item++)
	{
		INT32 maxval = 2000;
		INT32 defval = sound_get_default_gain(machine, item) * 1000.0f + 0.5f;

		if (defval > 1000)
			maxval = 2 * defval;

		string.printf("%s Volume", sound_get_user_gain_name(machine, item));
		*tailptr = slider_alloc(machine, string, 0, defval, maxval, 20, slider_mixervol, (void *)(FPTR)item);
		tailptr = &(*tailptr)->next;
	}

	/* add analog adjusters */
	for (port = machine->m_portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (field->type == IPT_ADJUSTER)
			{
				void *param = (void *)field;
				*tailptr = slider_alloc(machine, field->name, 0, field->defvalue, 100, 1, slider_adjuster, param);
				tailptr = &(*tailptr)->next;
			}

	/* add CPU overclocking (cheat only) */
	if (options_get_bool(machine->options(), OPTION_CHEAT))
	{
		device_execute_interface *exec;
		for (bool gotone = machine->m_devicelist.first(exec); exec != NULL; gotone = exec->next(exec))
		{
			void *param = (void *)&exec->device();
			string.printf("Overclock CPU %s", exec->device().tag());
			*tailptr = slider_alloc(machine, string, 10, 1000, 2000, 1, slider_overclock, param);
			tailptr = &(*tailptr)->next;
		}
	}

	/* add screen parameters */
	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
	{
		int defxscale = floor(screen->config().xscale() * 1000.0f + 0.5f);
		int defyscale = floor(screen->config().yscale() * 1000.0f + 0.5f);
		int defxoffset = floor(screen->config().xoffset() * 1000.0f + 0.5f);
		int defyoffset = floor(screen->config().yoffset() * 1000.0f + 0.5f);
		void *param = (void *)screen;

		/* add refresh rate tweaker */
		if (options_get_bool(machine->options(), OPTION_CHEAT))
		{
			string.printf("%s Refresh Rate", slider_get_screen_desc(*screen));
			*tailptr = slider_alloc(machine, string, -10000, 0, 10000, 1000, slider_refresh, param);
			tailptr = &(*tailptr)->next;
		}

		/* add standard brightness/contrast/gamma controls per-screen */
		string.printf("%s Brightness", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 10, slider_brightness, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Contrast", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 2000, 50, slider_contrast, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Gamma", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 100, 1000, 3000, 50, slider_gamma, param);
		tailptr = &(*tailptr)->next;

		/* add scale and offset controls per-screen */
		string.printf("%s Horiz Stretch", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defxscale, 1500, 2, slider_xscale, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Horiz Position", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_xoffset, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Vert Stretch", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, 500, defyscale, 1500, 2, slider_yscale, param);
		tailptr = &(*tailptr)->next;
		string.printf("%s Vert Position", slider_get_screen_desc(*screen));
		*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_yoffset, param);
		tailptr = &(*tailptr)->next;
	}

	for (device = machine->m_devicelist.first(LASERDISC); device != NULL; device = device->typenext())
	{
		const laserdisc_config *config = (const laserdisc_config *)downcast<const legacy_device_config_base &>(device->baseconfig()).inline_config();
		if (config->overupdate != NULL)
		{
			int defxscale = floor(config->overscalex * 1000.0f + 0.5f);
			int defyscale = floor(config->overscaley * 1000.0f + 0.5f);
			int defxoffset = floor(config->overposx * 1000.0f + 0.5f);
			int defyoffset = floor(config->overposy * 1000.0f + 0.5f);
			void *param = (void *)device;

			/* add scale and offset controls per-overlay */
			string.printf("%s Horiz Stretch", slider_get_laserdisc_desc(device));
			*tailptr = slider_alloc(machine, string, 500, (defxscale == 0) ? 1000 : defxscale, 1500, 2, slider_overxscale, param);
			tailptr = &(*tailptr)->next;
			string.printf("%s Horiz Position", slider_get_laserdisc_desc(device));
			*tailptr = slider_alloc(machine, string, -500, defxoffset, 500, 2, slider_overxoffset, param);
			tailptr = &(*tailptr)->next;
			string.printf("%s Vert Stretch", slider_get_laserdisc_desc(device));
			*tailptr = slider_alloc(machine, string, 500, (defyscale == 0) ? 1000 : defyscale, 1500, 2, slider_overyscale, param);
			tailptr = &(*tailptr)->next;
			string.printf("%s Vert Position", slider_get_laserdisc_desc(device));
			*tailptr = slider_alloc(machine, string, -500, defyoffset, 500, 2, slider_overyoffset, param);
			tailptr = &(*tailptr)->next;
		}
	}

	for (screen_device *screen = screen_first(*machine); screen != NULL; screen = screen_next(screen))
		if (screen->screen_type() == SCREEN_TYPE_VECTOR)
		{
			/* add flicker control */
			*tailptr = slider_alloc(machine, "Vector Flicker", 0, 0, 1000, 10, slider_flicker, NULL);
			tailptr = &(*tailptr)->next;
			*tailptr = slider_alloc(machine, "Beam Width", 10, 100, 1000, 10, slider_beam, NULL);
			tailptr = &(*tailptr)->next;
			break;
		}

#ifdef MAME_DEBUG
	/* add crosshair adjusters */
	for (port = machine->m_portlist.first(); port != NULL; port = port->next())
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (field->crossaxis != CROSSHAIR_AXIS_NONE && field->player == 0)
			{
				void *param = (void *)field;
				string.printf("Crosshair Scale %s", (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 1000, 3000, 100, slider_crossscale, param);
				tailptr = &(*tailptr)->next;
				string.printf("Crosshair Offset %s", (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y");
				*tailptr = slider_alloc(machine, string, -3000, 0, 3000, 100, slider_crossoffset, param);
				tailptr = &(*tailptr)->next;
			}
#endif

	return listhead;
}


/*-------------------------------------------------
    slider_volume - global volume slider callback
-------------------------------------------------*/

static INT32 slider_volume(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		sound_set_attenuation(machine, newval);
	if (string != NULL)
		string->printf("%3ddB", sound_get_attenuation(machine));
	return sound_get_attenuation(machine);
}


/*-------------------------------------------------
    slider_mixervol - single channel volume
    slider callback
-------------------------------------------------*/

static INT32 slider_mixervol(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	int which = (FPTR)arg;
	if (newval != SLIDER_NOCHANGE)
		sound_set_user_gain(machine, which, (float)newval * 0.001f);
	if (string != NULL)
		string->printf("%4.2f", sound_get_user_gain(machine, which));
	return floor(sound_get_user_gain(machine, which) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_adjuster - analog adjuster slider
    callback
-------------------------------------------------*/

static INT32 slider_adjuster(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	const input_field_config *field = (const input_field_config *)arg;
	input_field_user_settings settings;

	input_field_get_user_settings(field, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.value = newval;
		input_field_set_user_settings(field, &settings);
	}
	if (string != NULL)
		string->printf("%d%%", settings.value);
	return settings.value;
}


/*-------------------------------------------------
    slider_overclock - CPU overclocker slider
    callback
-------------------------------------------------*/

static INT32 slider_overclock(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	device_t *cpu = (device_t *)arg;
	if (newval != SLIDER_NOCHANGE)
		cpu_set_clockscale(cpu, (float)newval * 0.001f);
	if (string != NULL)
		string->printf("%3.0f%%", floor(cpu_get_clockscale(cpu) * 100.0f + 0.5f));
	return floor(cpu_get_clockscale(cpu) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_refresh - refresh rate slider callback
-------------------------------------------------*/

static INT32 slider_refresh(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	double defrefresh = ATTOSECONDS_TO_HZ(screen->config().refresh());
	double refresh;

	if (newval != SLIDER_NOCHANGE)
	{
		int width = screen->width();
		int height = screen->height();
		const rectangle &visarea = screen->visible_area();
		screen->configure(width, height, visarea, HZ_TO_ATTOSECONDS(defrefresh + (double)newval * 0.001));
	}
	if (string != NULL)
		string->printf("%.3ffps", ATTOSECONDS_TO_HZ(machine->primary_screen->frame_period().attoseconds));
	refresh = ATTOSECONDS_TO_HZ(machine->primary_screen->frame_period().attoseconds);
	return floor((refresh - defrefresh) * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_brightness - screen brightness slider
    callback
-------------------------------------------------*/

static INT32 slider_brightness(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.brightness = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.brightness);
	return floor(settings.brightness * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_contrast - screen contrast slider
    callback
-------------------------------------------------*/

static INT32 slider_contrast(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.contrast = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.contrast);
	return floor(settings.contrast * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_gamma - screen gamma slider callback
-------------------------------------------------*/

static INT32 slider_gamma(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.gamma = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.gamma);
	return floor(settings.gamma * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_xscale(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.xscale = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.xscale);
	return floor(settings.xscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_yscale(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.yscale = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.yscale);
	return floor(settings.yscale * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_xoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_xoffset(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.xoffset = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.xoffset);
	return floor(settings.xoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_yoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_yoffset(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	screen_device *screen = reinterpret_cast<screen_device *>(arg);
	render_container *container = render_container_get_screen(screen);
	render_container_user_settings settings;

	render_container_get_user_settings(container, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.yoffset = (float)newval * 0.001f;
		render_container_set_user_settings(container, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.yoffset);
	return floor(settings.yoffset * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxscale - screen horizontal scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overxscale(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	device_t *laserdisc = (device_t *)arg;
	laserdisc_config settings;

	laserdisc_get_config(laserdisc, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.overscalex = (float)newval * 0.001f;
		laserdisc_set_config(laserdisc, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.overscalex);
	return floor(settings.overscalex * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyscale - screen vertical scale slider
    callback
-------------------------------------------------*/

static INT32 slider_overyscale(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	device_t *laserdisc = (device_t *)arg;
	laserdisc_config settings;

	laserdisc_get_config(laserdisc, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.overscaley = (float)newval * 0.001f;
		laserdisc_set_config(laserdisc, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.overscaley);
	return floor(settings.overscaley * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overxoffset - screen horizontal position
    slider callback
-------------------------------------------------*/

static INT32 slider_overxoffset(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	device_t *laserdisc = (device_t *)arg;
	laserdisc_config settings;

	laserdisc_get_config(laserdisc, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.overposx = (float)newval * 0.001f;
		laserdisc_set_config(laserdisc, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.overposx);
	return floor(settings.overposx * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_overyoffset - screen vertical position
    slider callback
-------------------------------------------------*/

static INT32 slider_overyoffset(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	device_t *laserdisc = (device_t *)arg;
	laserdisc_config settings;

	laserdisc_get_config(laserdisc, &settings);
	if (newval != SLIDER_NOCHANGE)
	{
		settings.overposy = (float)newval * 0.001f;
		laserdisc_set_config(laserdisc, &settings);
	}
	if (string != NULL)
		string->printf("%.3f", settings.overposy);
	return floor(settings.overposy * 1000.0f + 0.5f);
}


/*-------------------------------------------------
    slider_flicker - vector flicker slider
    callback
-------------------------------------------------*/

static INT32 slider_flicker(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_flicker((float)newval * 0.1f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_flicker());
	return floor(vector_get_flicker() * 10.0f + 0.5f);
}


/*-------------------------------------------------
    slider_beam - vector beam width slider
    callback
-------------------------------------------------*/

static INT32 slider_beam(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	if (newval != SLIDER_NOCHANGE)
		vector_set_beam((float)newval * 0.01f);
	if (string != NULL)
		string->printf("%1.2f", vector_get_beam());
	return floor(vector_get_beam() * 100.0f + 0.5f);
}


/*-------------------------------------------------
    slider_get_screen_desc - returns the
    description for a given screen
-------------------------------------------------*/

static char *slider_get_screen_desc(screen_device &screen)
{
	int scrcount = screen_count(*screen.machine->config);
	static char descbuf[256];

	if (scrcount > 1)
		sprintf(descbuf, "Screen '%s'", screen.tag());
	else
		strcpy(descbuf, "Screen");

	return descbuf;
}


/*-------------------------------------------------
    slider_get_laserdisc_desc - returns the
    description for a given laseridsc
-------------------------------------------------*/

static char *slider_get_laserdisc_desc(device_t *laserdisc)
{
	int ldcount = laserdisc->machine->m_devicelist.count(LASERDISC);
	static char descbuf[256];

	if (ldcount > 1)
		sprintf(descbuf, "Laserdisc '%s'", laserdisc->tag());
	else
		strcpy(descbuf, "Laserdisc");

	return descbuf;
}


/*-------------------------------------------------
    slider_crossscale - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossscale(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	input_field_config *field = (input_field_config *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->crossscale = (float)newval * 0.001f;
	if (string != NULL)
		string->printf("%s %s %1.3f", "Crosshair Scale", (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	return floor(field->crossscale * 1000.0f + 0.5f);
}
#endif


/*-------------------------------------------------
    slider_crossoffset - crosshair scale slider
    callback
-------------------------------------------------*/

#ifdef MAME_DEBUG
static INT32 slider_crossoffset(running_machine *machine, void *arg, astring *string, INT32 newval)
{
	input_field_config *field = (input_field_config *)arg;

	if (newval != SLIDER_NOCHANGE)
		field->crossoffset = (float)newval * 0.001f;
	if (string != NULL)
		string->printf("%s %s %1.3f", "Crosshair Offset", (field->crossaxis == CROSSHAIR_AXIS_X) ? "X" : "Y", (float)newval * 0.001f);
	return field->crossoffset;
}
#endif


/*-------------------------------------------------
    ui_get_use_natural_keyboard - returns
    whether the natural keyboard is active
-------------------------------------------------*/

int ui_get_use_natural_keyboard(running_machine *machine)
{
	return ui_use_natural_keyboard;
}



/*-------------------------------------------------
    ui_set_use_natural_keyboard - specifies
    whether the natural keyboard is active
-------------------------------------------------*/

void ui_set_use_natural_keyboard(running_machine *machine, int use_natural_keyboard)
{
	ui_use_natural_keyboard = use_natural_keyboard;
	options_set_bool(machine->options(), OPTION_NATURAL_KEYBOARD, use_natural_keyboard, OPTION_PRIORITY_CMDLINE);
}

