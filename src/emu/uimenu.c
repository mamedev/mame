/*********************************************************************

    uimenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "ui.h"
#include "rendutil.h"
#include "cheat.h"
#include "uimenu.h"
#include "audit.h"
#include "deprecat.h"

#ifdef MESS
#include "uimess.h"
#include "inputx.h"
#endif /* MESS */

#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MENU_STACK_DEPTH		8
#define MENU_STRING_POOL_SIZE	(64*1024)

#define MENU_TEXTCOLOR			ARGB_WHITE
#define MENU_SELECTCOLOR		MAKE_ARGB(0xff,0xff,0xff,0x00)
#define MENU_UNAVAILABLECOLOR	MAKE_ARGB(0xff,0x40,0x40,0x40)

#define VISIBLE_GAMES_IN_LIST	15

#define MAX_PHYSICAL_DIPS		10

/* DIP switch rendering parameters */
#define DIP_SWITCH_HEIGHT		0.05f
#define DIP_SWITCH_SPACING		0.01
#define SINGLE_TOGGLE_SWITCH_FIELD_WIDTH 0.02f
#define SINGLE_TOGGLE_SWITCH_WIDTH 0.015f
/* make the switch 80% of the width space and 1/2 of the switch height */
#define PERCENTAGE_OF_HALF_FIELD_USED 0.80f
#define SINGLE_TOGGLE_SWITCH_HEIGHT ((DIP_SWITCH_HEIGHT / 2) * PERCENTAGE_OF_HALF_FIELD_USED)


enum
{
	INPUT_TYPE_DIGITAL = 0,
	INPUT_TYPE_ANALOG = 1,
	INPUT_TYPE_ANALOG_DEC = 2,
	INPUT_TYPE_ANALOG_INC = 3
};

enum
{
	ANALOG_ITEM_KEYSPEED = 0,
	ANALOG_ITEM_CENTERSPEED,
	ANALOG_ITEM_REVERSE,
	ANALOG_ITEM_SENSITIVITY,
	ANALOG_ITEM_COUNT
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _menu_state menu_state;
struct _menu_state
{
	ui_menu_handler handler;			/* handler callback */
	UINT32			state;				/* state value */
};


typedef struct _input_item_data input_item_data;
struct _input_item_data
{
	input_seq *		seq;				/* pointer to the sequence we are operating on */
	const input_seq *defseq;			/* pointer to the sequence we are operating on */
	const char *	name;
	const char *	seqname;
	UINT16 			sortorder;			/* sorting information */
	UINT8 			type;				/* type of port */
	UINT8 			invert;				/* type of port */
};


typedef struct _dip_descriptor dip_descriptor;
struct _dip_descriptor
{
	const char * 	dip_name;
	UINT16			total_dip_mask;
	UINT16			total_dip_settings;
	UINT16			dip_invert_mask;
	UINT16 			selected_dip_feature_mask;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* menu states */
static int menu_stack_index;
static menu_state menu_stack[MENU_STACK_DEPTH];

static UINT32 menu_string_pool_offset;
static char menu_string_pool[MENU_STRING_POOL_SIZE];

static input_seq starting_seq;

static char select_game_buffer[40];
static const game_driver *select_game_list[VISIBLE_GAMES_IN_LIST];
static const game_driver **select_game_driver_list;

static dip_descriptor dip_switch_model[MAX_PHYSICAL_DIPS];

static mame_bitmap *hilight_bitmap;
static render_texture *hilight_texture;

static render_texture *arrow_texture;

static const char *const input_format[] =
{
	"%s",
	"%s Analog",
	"%s Dec",
	"%s Inc"
};

static const rgb_t text_fgcolor = MAKE_ARGB(0xff,0xff,0xff,0xff);
static const rgb_t text_bgcolor = MAKE_ARGB(0xe0,0x80,0x80,0x80);
static const rgb_t sel_fgcolor = MAKE_ARGB(0xff,0xff,0xff,0x00);
static const rgb_t sel_bgcolor = MAKE_ARGB(0xe0,0x80,0x80,0x00);



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_menu_exit(running_machine *machine);

/* menu handlers */
static UINT32 menu_main(UINT32 state);
static UINT32 menu_input_groups(UINT32 state);
static UINT32 menu_input(UINT32 state);
static UINT32 menu_switches(UINT32 state);
static UINT32 menu_analog(UINT32 state);
#ifndef MESS
static UINT32 menu_bookkeeping(UINT32 state);
#endif
static UINT32 menu_game_info(UINT32 state);
static UINT32 menu_cheat(UINT32 state);
static UINT32 menu_memory_card(UINT32 state);
static UINT32 menu_video(UINT32 state);
static UINT32 menu_quit_game(UINT32 state);
static UINT32 menu_select_game(UINT32 state);
#ifdef MESS
static UINT32 menu_file_manager(UINT32 state);
#if HAS_WAVE
static UINT32 menu_tape_control(UINT32 state);
#endif /* HAS_WAVE */
#endif /* MESS */

/* menu helpers */
static void menu_render_triangle(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param);

static int input_menu_get_items(input_item_data *itemlist, int group);
static int input_menu_get_game_items(input_item_data *itemlist);
static void input_menu_toggle_none_default(input_seq *selected_seq, input_seq *original_seq, const input_seq *selected_defseq);
static int input_menu_compare_items(const void *i1, const void *i2);
static void switches_menu_add_item(ui_menu_item *item, const input_port_entry *in, int switch_entry, void *ref);
static void switches_menu_select_previous(input_port_entry *in, int switch_entry);
static void switches_menu_select_next(input_port_entry *in, int switch_entry);
//static int switches_menu_compare_items(const void *i1, const void *i2);
static void analog_menu_add_item(ui_menu_item *item, const input_port_entry *in, const char *append_string, int which_item);

/* DIP switch helpers */
static void dip_switch_build_model(input_port_entry *entry, int item_is_selected);
static void dip_switch_draw_one(float dip_menu_x1, float dip_menu_y1, float dip_menu_x2, float dip_menu_y2, int model_index);
static void dip_switch_render(const menu_extra *extra, float x1, float y1, float x2, float y2);

static int CLIB_DECL select_game_driver_compare(const void *elem1, const void *elem2);
static void select_game_build_driver_list(void);
static void select_game_render(const menu_extra *extra, float x1, float y1, float x2, float y2);
static int select_game_handle_key(input_code keycode, char value);


/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    menu_string_pool_add - add a formatted string
    to the string pool
-------------------------------------------------*/

INLINE const char *CLIB_DECL ATTR_PRINTF(1,2) menu_string_pool_add(const char *format, ...)
{
	char *result = &menu_string_pool[menu_string_pool_offset];
	va_list arg;

	/* print to the string pool */
	va_start(arg, format);
	menu_string_pool_offset += vsprintf(result, format, arg) + 1;
	va_end(arg);

	return result;
}


/*-------------------------------------------------
    get_num_dips - return the number of physical
    DIP switches that are to be drawn
-------------------------------------------------*/

INLINE int get_num_dips(void)
{
	int num = 0;

	while (dip_switch_model[num].dip_name != NULL && num < MAX_PHYSICAL_DIPS)
		num++;

	return num;
}



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    ui_menu_init - initialize the menu system
-------------------------------------------------*/

void ui_menu_init(running_machine *machine)
{
	int x;

	/* initialize the menu stack */
	ui_menu_stack_reset();
	select_game_buffer[0] = 0;

	/* create a texture for hilighting items */
	hilight_bitmap = bitmap_alloc(256, 1, BITMAP_FORMAT_ARGB32);
	for (x = 0; x < 256; x++)
	{
		int alpha = 0xff;
		if (x < 25) alpha = 0xff * x / 25;
		if (x > 256 - 25) alpha = 0xff * (255 - x) / 25;
		*BITMAP_ADDR32(hilight_bitmap, 0, x) = MAKE_ARGB(alpha,0xff,0xff,0xff);
	}
	hilight_texture = render_texture_alloc(NULL, NULL);
	render_texture_set_bitmap(hilight_texture, hilight_bitmap, NULL, 0, TEXFORMAT_ARGB32);

	/* create a texture for arrow icons */
	arrow_texture = render_texture_alloc(menu_render_triangle, NULL);

	/* add an exit callback to free memory */
	add_exit_callback(machine, ui_menu_exit);
}


/*-------------------------------------------------
    ui_menu_exit - clean up after ourselves
-------------------------------------------------*/

static void ui_menu_exit(running_machine *machine)
{
	/* free textures */
	render_texture_free(hilight_texture);
	bitmap_free(hilight_bitmap);
	render_texture_free(arrow_texture);

	/* free the driver list */
	if (select_game_driver_list != NULL)
		free((void *)select_game_driver_list);
	select_game_driver_list = NULL;
}


/*-------------------------------------------------
    ui_menu_draw - draw a menu
-------------------------------------------------*/

int ui_menu_draw(const ui_menu_item *items, int numitems, int selected, const menu_extra *extra)
{
	float line_height = ui_get_line_height();
	float lr_arrow_width = 0.4f * line_height * render_get_ui_aspect();
	float ud_arrow_width = line_height * render_get_ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;
	float x1, y1, x2, y2;

	float effective_width, effective_left;
	float visible_width, visible_main_menu_height;
	float visible_extra_menu_height = 0;
	float visible_top, visible_left;
	int selected_subitem_too_big = 0;
	int visible_lines;
	int top_line;
	int itemnum, linenum;

	/* compute the width and height of the full menu */
	visible_width = 0;
	visible_main_menu_height = 0;
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		const ui_menu_item *item = &items[itemnum];
		float total_width;

		/* compute width of left hand side */
		total_width = gutter_width + ui_get_string_width(item->text) + gutter_width;

		/* add in width of right hand side */
		if (item->subtext)
			total_width += 2.0f * gutter_width + ui_get_string_width(item->subtext);

		/* track the maximum */
		if (total_width > visible_width)
			visible_width = total_width;

		/* track the height as well */
		visible_main_menu_height += line_height;
	}

	/* if agumenting the menu, find out how much extra space is needed */
	if (extra != NULL)
		visible_extra_menu_height = extra->top + extra->bottom;

	/* add a little bit of slop for rounding */
	visible_width += 0.01f;
	visible_main_menu_height += 0.01f;

	/* if we are too wide or too tall, clamp it down */
	if (visible_width + 2.0f * UI_BOX_LR_BORDER > 1.0f)
		visible_width = 1.0f - 2.0f * UI_BOX_LR_BORDER;

	/* if the menu and extra menu won't fit, take away part of the regular menu, it will scroll */
	if (visible_main_menu_height + visible_extra_menu_height + 2.0f * UI_BOX_TB_BORDER > 1.0f)
		visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;

	visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)visible_lines * line_height;

	/* compute top/left of inner menu area by centering */
	visible_left = (1.0f - visible_width) * 0.5f;
	visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	/* if the menu is at the bottom of the extra, adjust */
	if (extra != NULL)
		visible_top += extra->top;

	/* first add us a box */
	x1 = visible_left - UI_BOX_LR_BORDER;
	y1 = visible_top - UI_BOX_TB_BORDER;
	x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	ui_draw_outlined_box(x1, y1, x2, y2, UI_FILLCOLOR);

	/* determine the first visible line based on the current selection */
	top_line = selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= numitems)
		top_line = numitems - visible_lines;

	/* determine effective positions taking into account the hilighting arrows */
	effective_width = visible_width - 2.0f * gutter_width;
	effective_left = visible_left + gutter_width;

	/* loop over visible lines */
	for (linenum = 0; linenum < visible_lines; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		int itemnum = top_line + linenum;
		const ui_menu_item *item = &items[itemnum];
		const char *itemtext = item->text;
		rgb_t fgcolor = text_fgcolor;
		rgb_t bgcolor = text_bgcolor;

		/* if we're selected, draw with a different background */
		if (itemnum == selected)
		{
			fgcolor = sel_fgcolor;
			bgcolor = sel_bgcolor;
			render_ui_add_quad(	x1 + 0.5f * UI_LINE_WIDTH,
								line_y,
								x2 - 0.5f * UI_LINE_WIDTH,
								line_y + line_height,
								bgcolor,
								hilight_texture,
								PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}

		/* if we're on the top line, display the up arrow */
		if (linenum == 0 && top_line != 0)
		{
			render_ui_add_quad(	0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
								line_y + 0.25f * line_height,
								0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
								line_y + 0.75f * line_height,
								fgcolor,
								arrow_texture,
								PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT0));
		}

		/* if we're on the bottom line, display the down arrow */
		else if (linenum == visible_lines - 1 && itemnum != numitems - 1)
		{
			render_ui_add_quad(	0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
								line_y + 0.25f * line_height,
								0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
								line_y + 0.75f * line_height,
								fgcolor,
								arrow_texture,
								PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT0 ^ ORIENTATION_FLIP_Y));
		}

		/* if we're just a divider, draw a line */
		else if (strcmp(itemtext, "-") == 0)
			render_ui_add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* if we don't have a subitem, just draw the string centered */
		else if (item->subtext == NULL)
			ui_draw_text_full(itemtext, effective_left, line_y, effective_width,
						JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);

		/* otherwise, draw the item on the left and the subitem text on the right */
		else
		{
			int subitem_invert = item->flags & MENU_FLAG_INVERT;
			const char *subitem_text = item->subtext;
			float item_width, subitem_width;

			/* draw the left-side text */
			ui_draw_text_full(itemtext, effective_left, line_y, effective_width,
						JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, &item_width, NULL);

			/* give 2 spaces worth of padding */
			item_width += 2.0f * gutter_width;

			/* if the subitem doesn't fit here, display dots */
			if (ui_get_string_width(subitem_text) > effective_width - item_width)
			{
				subitem_text = "...";
				if (itemnum == selected)
					selected_subitem_too_big = 1;
			}

			/* draw the subitem right-justified */
			ui_draw_text_full(subitem_text, effective_left + item_width, line_y, effective_width - item_width,
						JUSTIFY_RIGHT, WRAP_TRUNCATE, subitem_invert ? DRAW_OPAQUE : DRAW_NORMAL, fgcolor, bgcolor, &subitem_width, NULL);

			/* apply arrows */
			if (itemnum == selected && (item->flags & MENU_FLAG_LEFT_ARROW))
			{
				render_ui_add_quad(	effective_left + effective_width - subitem_width - gutter_width,
									line_y + 0.1f * line_height,
									effective_left + effective_width - subitem_width - gutter_width + lr_arrow_width,
									line_y + 0.9f * line_height,
									fgcolor,
									arrow_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90 ^ ORIENTATION_FLIP_X));
			}
			if (itemnum == selected && (item->flags & MENU_FLAG_RIGHT_ARROW))
			{
				render_ui_add_quad(	effective_left + effective_width + gutter_width - lr_arrow_width,
									line_y + 0.1f * line_height,
									effective_left + effective_width + gutter_width,
									line_y + 0.9f * line_height,
									fgcolor,
									arrow_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90));
			}
		}
	}

	/* if the selected subitem is too big, display it in a separate offset box */
	if (selected_subitem_too_big)
	{
		const ui_menu_item *item = &items[selected];
		int subitem_invert = item->flags & MENU_FLAG_INVERT;
		int linenum = selected - top_line;
		float line_y = visible_top + (float)linenum * line_height;
		float target_width, target_height;
		float target_x, target_y;

		/* compute the multi-line target width/height */
		ui_draw_text_full(item->subtext, 0, 0, visible_width * 0.75f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);

		/* determine the target location */
		target_x = visible_left + visible_width - target_width - UI_BOX_LR_BORDER;
		target_y = line_y + line_height + UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > visible_main_menu_height)
			target_y = line_y - target_height - UI_BOX_TB_BORDER;

		/* add a box around that */
		ui_draw_outlined_box(target_x - UI_BOX_LR_BORDER,
						 target_y - UI_BOX_TB_BORDER,
						 target_x + target_width + UI_BOX_LR_BORDER,
						 target_y + target_height + UI_BOX_TB_BORDER, subitem_invert ? sel_bgcolor : UI_FILLCOLOR);
		ui_draw_text_full(item->subtext, target_x, target_y, target_width,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, sel_fgcolor, sel_bgcolor, NULL, NULL);
	}

	/* if there is somthing special to add, do it by calling the passed routine */
	if (extra != NULL)
		(*extra->render)(extra, x1, y1, x2, y2);

	/* return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow */
	return visible_lines - (top_line != 0) - (top_line + visible_lines != numitems);
}


/*-------------------------------------------------
    ui_menu_draw_text_box - draw a multiline
    word-wrapped text box with a menu item at the
    bottom
-------------------------------------------------*/

static void ui_menu_draw_text_box(const char *text)
{
	const char *priortext = "Return to Prior Menu";
	float line_height = ui_get_line_height();
	float lr_arrow_width = 0.4f * line_height * render_get_ui_aspect();
	float gutter_width = lr_arrow_width;
	float target_width, target_height, prior_width;
	float target_x, target_y;

	/* compute the multi-line target width/height */
	ui_draw_text_full(text, 0, 0, 1.0f - 2.0f * UI_BOX_LR_BORDER - 2.0f * gutter_width,
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);
	target_height += 2.0f * line_height;
	if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
		target_height = floor((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;

	/* maximum against "return to prior menu" text */
	prior_width = ui_get_string_width(priortext) + 2.0f * gutter_width;
	target_width = MAX(target_width, prior_width);

	/* determine the target location */
	target_x = 0.5f - 0.5f * target_width;
	target_y = 0.5f - 0.5f * target_height;

	/* make sure we stay on-screen */
	if (target_x < UI_BOX_LR_BORDER + gutter_width)
		target_x = UI_BOX_LR_BORDER + gutter_width;
	if (target_x + target_width + gutter_width + UI_BOX_LR_BORDER > 1.0f)
		target_x = 1.0f - UI_BOX_LR_BORDER - gutter_width - target_width;
	if (target_y < UI_BOX_TB_BORDER)
		target_y = UI_BOX_TB_BORDER;
	if (target_y + target_height + UI_BOX_TB_BORDER > 1.0f)
		target_y = 1.0f - UI_BOX_TB_BORDER - target_height;

	/* add a box around that */
	ui_draw_outlined_box(target_x - UI_BOX_LR_BORDER - gutter_width,
					 target_y - UI_BOX_TB_BORDER,
					 target_x + target_width + gutter_width + UI_BOX_LR_BORDER,
					 target_y + target_height + UI_BOX_TB_BORDER, UI_FILLCOLOR);
	ui_draw_text_full(text, target_x, target_y, target_width,
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);

	/* draw the "return to prior menu" text with a hilight behind it */
	render_ui_add_quad(	target_x + 0.5f * UI_LINE_WIDTH,
						target_y + target_height - line_height,
						target_x + target_width - 0.5f * UI_LINE_WIDTH,
						target_y + target_height,
						sel_bgcolor,
						hilight_texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
	ui_draw_text_full(priortext, target_x, target_y + target_height - line_height, target_width,
				JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, sel_fgcolor, sel_bgcolor, NULL, NULL);
}


/*-------------------------------------------------
    ui_menu_generic_keys - generically handle
    keys for a menu
-------------------------------------------------*/

int ui_menu_generic_keys(UINT32 *selected, int num_items, int visible_items)
{
	/* hitting cancel or selecting the last item returns to the previous menu */
	if (input_ui_pressed(IPT_UI_CANCEL) || (*selected == num_items - 1 && input_ui_pressed(IPT_UI_SELECT)))
	{
		*selected = ui_menu_stack_pop();
		return 1;
	}

	/* up backs up by one item */
	if (input_ui_pressed_repeat(IPT_UI_UP, 6))
		*selected = (*selected + num_items - 1) % num_items;

	/* down advances by one item */
	if (input_ui_pressed_repeat(IPT_UI_DOWN, 6))
		*selected = (*selected + 1) % num_items;

	/* page up backs up by visible_items */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_UP, 6))
	{
		if (*selected >= visible_items - 1)
			*selected -= visible_items - 1;
		else
			*selected = 0;
	}

	/* page down advances by visible_items */
	if (input_ui_pressed_repeat(IPT_UI_PAGE_DOWN, 6))
	{
		*selected += visible_items - 1;
		if (*selected >= num_items)
			*selected = num_items - 1;
	}

	/* home goes to the start */
	if (input_ui_pressed(IPT_UI_HOME))
		*selected = 0;

	/* end goes to the last */
	if (input_ui_pressed(IPT_UI_END))
		*selected = num_items - 1;

	/* pause enables/disables pause */
	if (input_ui_pressed(IPT_UI_PAUSE))
		mame_pause(Machine, !mame_is_paused(Machine));

	return 0;
}


/*-------------------------------------------------
    ui_menu_stack_reset - reset the menu stack
-------------------------------------------------*/

void ui_menu_stack_reset(void)
{
	menu_state *state = &menu_stack[menu_stack_index = 0];
	state->handler = NULL;
	state->state = 0;
}


/*-------------------------------------------------
    ui_menu_stack_push - push a new menu onto the
    stack
-------------------------------------------------*/

UINT32 ui_menu_stack_push(ui_menu_handler new_handler, UINT32 new_state)
{
	menu_state *state = &menu_stack[++menu_stack_index];
	assert(menu_stack_index < MENU_STACK_DEPTH);
	state->handler = new_handler;
	state->state = new_state;
	return new_state;
}


/*-------------------------------------------------
    ui_menu_stack_pop - pop a menu from the stack
-------------------------------------------------*/

UINT32 ui_menu_stack_pop(void)
{
	menu_state *state = &menu_stack[--menu_stack_index];
	assert(menu_stack_index >= 0);
	return state->state;
}


/*-------------------------------------------------
    ui_menu_ui_handler - displays the current menu
    and calls the menu handler
-------------------------------------------------*/

UINT32 ui_menu_ui_handler(running_machine *machine, UINT32 state)
{
	UINT32 newstate;

	/* if we have no menus stacked up, start with the main menu */
	if (menu_stack[menu_stack_index].handler == NULL)
		ui_menu_stack_push(menu_main, 0);

	/* update the menu state */
	newstate = (*menu_stack[menu_stack_index].handler)(menu_stack[menu_stack_index].state);
	menu_stack[menu_stack_index].state = newstate;

	/* if the menus are to be hidden, return a cancel here */
	if ((input_ui_pressed(IPT_UI_CONFIGURE) && !ui_menu_is_force_game_select()) || menu_stack[menu_stack_index].handler == NULL)
		return UI_HANDLER_CANCEL;

	return 0;
}


/*-------------------------------------------------
    ui_menu_force_game_select - force the game
    select menu to be visible and inescapable
-------------------------------------------------*/

void ui_menu_force_game_select(void)
{
	/* reset the menu stack */
	ui_menu_stack_reset();

	/* add the quit entry followed by the game select entry */
	ui_menu_stack_push(menu_quit_game, 0);
	ui_menu_stack_push(menu_select_game, 1 << 16);

	/* initialize the game name buffer */
	select_game_buffer[0] = 0;
	strcpy(select_game_buffer, options_get_string(mame_options(), OPTION_GAMENAME));

	/* force the menus on */
	ui_show_menu();

	/* make sure MAME is paused */
	mame_pause(Machine, TRUE);
}


/*-------------------------------------------------
    ui_menu_is_force_game_select - return true
    if we are currently in "force game select"
    mode
-------------------------------------------------*/

int ui_menu_is_force_game_select(void)
{
	return (menu_stack_index >= 2 && menu_stack[1].handler == menu_quit_game);
}



/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    menu_main - main UI menu
-------------------------------------------------*/

static UINT32 menu_main(UINT32 state)
{
#define ADD_MENU(name, _handler, _param) \
do { \
	item_list[menu_items].text = name; \
	handler_list[menu_items].handler = _handler; \
	handler_list[menu_items].state = _param; \
	menu_items++; \
} while (0)

	menu_state handler_list[20];
	ui_menu_item item_list[20];
	int has_categories = FALSE;
	int has_configs = FALSE;
	int has_analog = FALSE;
	int has_dips = FALSE;
	input_port_entry *in;
	int menu_items = 0;
	int visible_items;

	/* scan the input port array to see what options we need to enable */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (input_port_active(in))
		{
			if (in->type == IPT_DIPSWITCH_NAME)
				has_dips = TRUE;
			if (port_type_is_analog(in->type))
				has_analog = TRUE;
			if (in->type == IPT_CONFIG_NAME)
				has_configs = TRUE;
			if (in->category > 0)
				has_categories = TRUE;
		}

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* add input menu items */
	ADD_MENU("Input (general)", menu_input_groups, 0);
	ADD_MENU("Input (this " CAPSTARTGAMENOUN ")", menu_input, 1000 << 16);

	/* add optional input-related menus */
	if (has_dips)
		ADD_MENU("Dip Switches", menu_switches, (IPT_DIPSWITCH_NAME << 16) | (IPT_DIPSWITCH_SETTING << 24));
	if (has_configs)
		ADD_MENU("Driver Configuration", menu_switches, (IPT_CONFIG_NAME << 16) | (IPT_CONFIG_SETTING << 24));
#ifdef MESS
	if (has_categories)
		ADD_MENU("Categories", menu_switches, (IPT_CATEGORY_NAME << 16) | (IPT_CATEGORY_SETTING << 24));
#endif
	if (has_analog)
		ADD_MENU("Analog Controls", menu_analog, 0);

#ifndef MESS
  	/* add bookkeeping menu */
	ADD_MENU("Bookkeeping Info", menu_bookkeeping, 0);
#endif

	/* add game info menu */
	ADD_MENU(CAPSTARTGAMENOUN " Information", menu_game_info, 0);

#ifdef MESS
  	/* add image info menu */
	ADD_MENU("Image Information", ui_menu_image_info, 0);

  	/* add image info menu */
	ADD_MENU("File Manager", menu_file_manager, 1);

#if HAS_WAVE
  	/* add tape control menu */
	if (device_find(Machine->devices, IO_CASSETTE))
		ADD_MENU("Tape Control", menu_tape_control, 1);
#endif /* HAS_WAVE */
#endif /* MESS */

	/* add video options menu */
	ADD_MENU("Video Options", menu_video, 1000 << 16);

	/* add cheat menu */
	if (options_get_bool(mame_options(), OPTION_CHEAT))
		ADD_MENU("Cheat", menu_cheat, 1);

	/* add memory card menu */
	if (Machine->config->memcard_handler != NULL)
		ADD_MENU("Memory Card", menu_memory_card, 0);

	/* add reset and exit menus */
	ADD_MENU("Select New " CAPSTARTGAMENOUN, menu_select_game, 1 << 16);
	ADD_MENU("Return to " CAPSTARTGAMENOUN, NULL, 0);

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, state, NULL);

	/* handle the keys */
	if (ui_menu_generic_keys(&state, menu_items, visible_items))
		return state;
	if (input_ui_pressed(IPT_UI_SELECT))
		return ui_menu_stack_push(handler_list[state].handler, handler_list[state].state);

	return state;

#undef ADD_MENU
}


/*-------------------------------------------------
    menu_input_groups - menu displaying input
    groups
-------------------------------------------------*/

static UINT32 menu_input_groups(UINT32 state)
{
	ui_menu_item item_list[IPG_TOTAL_GROUPS + 2];
	int menu_items = 0;
	int visible_items;
	int player;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));

	/* build up the menu */
	item_list[menu_items++].text = "User Interface";
	for (player = 0; player < MAX_PLAYERS; player++)
		item_list[menu_items++].text = menu_string_pool_add("Player %d Controls", player + 1);
	item_list[menu_items++].text = "Other Controls";

	/* add an item for the return */
	item_list[menu_items++].text = "Return to Prior Menu";

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, state, NULL);

	/* handle the keys */
	if (ui_menu_generic_keys(&state, menu_items, visible_items))
		return state;
	if (input_ui_pressed(IPT_UI_SELECT))
		return ui_menu_stack_push(menu_input, state << 16);

	return state;
}


/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/

static UINT32 menu_input(UINT32 state)
{
	ui_menu_item item_list[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	input_item_data item_data[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	input_item_data *selected_item_data;
	UINT32 selected = state & 0x3fff;
	int record_next = (state >> 14) & 1;
	int polling = (state >> 15) & 1;
	int group = state >> 16;
	int visible_items;
	int menu_items;
	int item;

	/* get the list of items */
	menu_string_pool_offset = 0;
	menu_items = input_menu_get_items(item_data, group);

	/* build the menu */
	memset(item_list, 0, sizeof(item_list));
	for (item = 0; item < menu_items; item++)
	{
		input_item_data *id = &item_data[item];

		/* set the item text from the precomputed data */
		item_list[item].text = id->name;
		item_list[item].subtext = id->seqname;
		if (id->invert)
			item_list[item].flags |= MENU_FLAG_INVERT;

		/* keep the sequence pointer as a ref */
		item_list[item].ref = id;
	}

	/* sort the list canonically */
	qsort(item_list, menu_items, sizeof(item_list[0]), input_menu_compare_items);

	/* add an item to return */
	item_list[menu_items++].text = "Return to Prior Menu";

	/* if we're polling, just put an empty entry and arrows for the subitem */
	if (polling)
	{
		item_list[selected].subtext = "   ";
		item_list[selected].flags = MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
	}

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, selected, NULL);

	/* if we're polling, read the sequence */
	selected_item_data = item_list[selected].ref;
	if (polling)
	{
		input_seq newseq;

		/* if UI_CANCEL is pressed, abort */
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			record_next = polling = FALSE;
			input_menu_toggle_none_default(selected_item_data->seq, &starting_seq, selected_item_data->defseq);
		}

		/* poll again; if finished, update the sequence */
		if (input_seq_poll(&newseq))
		{
			record_next = TRUE;
			polling = FALSE;
			*selected_item_data->seq = newseq;
		}
	}

	/* otherwise, handle the keys */
	else
	{
		int prevsel = selected;

		/* handle generic menu keys */
		if (ui_menu_generic_keys(&selected, menu_items, visible_items))
			return selected;

		/* if an item was selected, start polling on it */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			input_seq_poll_start((selected_item_data->type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH, record_next ? selected_item_data->seq : NULL);
			starting_seq = *selected_item_data->seq;
			polling = TRUE;
		}

		/* if the clear key was pressed, reset the selected item */
		if (input_ui_pressed(IPT_UI_CLEAR))
		{
			input_menu_toggle_none_default(selected_item_data->seq, selected_item_data->seq, selected_item_data->defseq);
			record_next = FALSE;
		}

		/* if the selection changed, update and reset the "record first" flag */
		if (selected != prevsel)
			record_next = FALSE;
	}

	return selected | (record_next << 14) | (polling << 15) | (group << 16);
}


/*-------------------------------------------------
    menu_switches - display a menu for DIP
    switches
-------------------------------------------------*/

static UINT32 menu_switches(UINT32 state)
{
	ui_menu_item item_list[MAX_INPUT_PORTS * MAX_BITS_PER_PORT / 2];
	int switch_entry = (state >> 24) & 0xff;
	int switch_name = (state >> 16) & 0xff;
	UINT32 selected = state & 0xffff;
	input_port_entry *selected_in = NULL;
	input_port_entry *in;
	int menu_items = 0;
	int changed = FALSE;
	int visible_items;
	menu_extra extra;

	/* reset the menu */
	memset(item_list, 0, sizeof(item_list));
	/* reset the dip switch model */
	memset(dip_switch_model, 0, sizeof(dip_switch_model));

	/* loop over input ports and set up the current values */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (in->type == switch_name && input_port_active(in) && input_port_condition(in))
		{
			switches_menu_add_item(&item_list[menu_items], in, switch_entry, in);
			if (in->type == IPT_DIPSWITCH_NAME)
				dip_switch_build_model(in, menu_items == selected);
			menu_items++;
		}

	/* sort the list */
//  qsort(item_list, menu_items, sizeof(item_list[0]), switches_menu_compare_items);
	selected_in = item_list[selected].ref;

	/* add an item to return */
	item_list[menu_items++].text = "Return to Prior Menu";

	/* configure the extra menu */
	extra.top = 0;
	extra.bottom = get_num_dips() * (DIP_SWITCH_HEIGHT + DIP_SWITCH_SPACING) + DIP_SWITCH_SPACING;
	extra.render = dip_switch_render;

	/* go through the port map and create masks that are easy to use when drawing DIP graphics. */
	/* draw the menu, augment the regular menue drawing with an additional box for DIPs */
	visible_items = ui_menu_draw(item_list, menu_items, selected, (dip_switch_model[0].dip_name) ? &extra : NULL);

	/* handle generic menu keys */
	if (ui_menu_generic_keys(&selected, menu_items, visible_items))
		return selected;

	/* handle left/right arrows */
	if (input_ui_pressed(IPT_UI_LEFT) && (item_list[selected].flags & MENU_FLAG_LEFT_ARROW))
	{
		switches_menu_select_previous(selected_in, switch_entry);
		changed = TRUE;
	}
	if (input_ui_pressed(IPT_UI_RIGHT) && (item_list[selected].flags & MENU_FLAG_RIGHT_ARROW))
	{
		switches_menu_select_next(selected_in, switch_entry);
		changed = TRUE;
	}

	/* update the selection to match the existing entry in case things got shuffled */
	/* due to conditional DIPs changing things */
	if (changed)
	{
		int newsel = 0;
		input_port_update_defaults();
		for (in = Machine->input_ports; in->type != IPT_END; in++)
			if (in->type == switch_name && input_port_active(in) && input_port_condition(in))
			{
				if (selected_in == in)
				{
					selected = newsel;
					break;
				}
				newsel++;
			}
	}

	return selected | (switch_name << 16) | (switch_entry << 24);
}


/*-------------------------------------------------
    menu_analog - display a menu for analog
    control settings
-------------------------------------------------*/

static UINT32 menu_analog(UINT32 state)
{
	ui_menu_item item_list[MAX_INPUT_PORTS * 4 * ANALOG_ITEM_COUNT];
	input_port_entry *selected_in = NULL;
	input_port_entry *in;
	int menu_items = 0;
	int selected_item = 0;
	int visible_items;
	int delta = 0;
	int use_autocenter;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* loop over input ports and add the items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
		if (port_type_is_analog(in->type))
		{
			use_autocenter = 0;
			switch (in->type)
			{
				/* Autocenter Speed is only used for these devices */
				case IPT_POSITIONAL:
				case IPT_POSITIONAL_V:
					if (in->analog.wraps) break;

				case IPT_PEDAL:
				case IPT_PEDAL2:
				case IPT_PEDAL3:
				case IPT_PADDLE:
				case IPT_PADDLE_V:
				case IPT_AD_STICK_X:
				case IPT_AD_STICK_Y:
				case IPT_AD_STICK_Z:
					use_autocenter = 1;
					break;
			}

			/* track the selected item */
			if (state >= menu_items && state < menu_items + 3 + use_autocenter)
			{
				selected_in = in;
				selected_item = state - menu_items;
				// shift menu for missing Autocenter
				if (selected_item && !use_autocenter) selected_item++;
			}

			/* add the needed items for each analog input */
			analog_menu_add_item(&item_list[menu_items++], in, "Digital Speed", ANALOG_ITEM_KEYSPEED);
			if (use_autocenter)
				analog_menu_add_item(&item_list[menu_items++], in, "Autocenter Speed", ANALOG_ITEM_CENTERSPEED);
			analog_menu_add_item(&item_list[menu_items++], in, "Reverse", ANALOG_ITEM_REVERSE);
			analog_menu_add_item(&item_list[menu_items++], in, "Sensitivity", ANALOG_ITEM_SENSITIVITY);
		}

	/* add an item to return */
	item_list[menu_items++].text = "Return to Prior Menu";

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, state, NULL);

	/* handle generic menu keys */
	if (ui_menu_generic_keys(&state, menu_items, visible_items))
		return state;

	/* handle left/right arrows */
	if (input_ui_pressed_repeat(IPT_UI_LEFT,6) && (item_list[state].flags & MENU_FLAG_LEFT_ARROW))
		delta = -1;
	if (input_ui_pressed_repeat(IPT_UI_RIGHT,6) && (item_list[state].flags & MENU_FLAG_RIGHT_ARROW))
		delta = 1;
	if (input_code_pressed(KEYCODE_LSHIFT))
		delta *= 10;

	/* adjust the appropriate value */
	if (delta != 0 && selected_in)
		switch (selected_item)
		{
			case ANALOG_ITEM_KEYSPEED:		selected_in->analog.delta += delta;			break;
			case ANALOG_ITEM_CENTERSPEED:	selected_in->analog.centerdelta += delta;	break;
			case ANALOG_ITEM_REVERSE:		selected_in->analog.reverse += delta;		break;
			case ANALOG_ITEM_SENSITIVITY:	selected_in->analog.sensitivity += delta;	break;
		}

	return state;
}


/*-------------------------------------------------
    menu_bookkeeping - display a "menu" for
    bookkeeping information
-------------------------------------------------*/

#ifndef MESS
static UINT32 menu_bookkeeping(UINT32 state)
{
	char buf[2048];
	char *bufptr = buf;
	UINT32 selected = 0;
	int ctrnum;
	attotime total_time;

	/* show total time first */
	total_time = timer_get_time();
	if (total_time.seconds >= 60 * 60)
		bufptr += sprintf(bufptr, "Uptime: %d:%02d:%02d\n\n", total_time.seconds / (60*60), (total_time.seconds / 60) % 60, total_time.seconds % 60);
	else
		bufptr += sprintf(bufptr, "Uptime: %d:%02d\n\n", (total_time.seconds / 60) % 60, total_time.seconds % 60);

	/* show tickets at the top */
	if (dispensed_tickets)
		bufptr += sprintf(bufptr, "Tickets dispensed: %d\n\n", dispensed_tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		/* display the coin counter number */
		bufptr += sprintf(bufptr, "Coin %c: ", ctrnum + 'A');

		/* display how many coins */
		if (!coin_count[ctrnum])
			bufptr += sprintf(bufptr, "NA");
		else
			bufptr += sprintf(bufptr, "%d", coin_count[ctrnum]);

		/* display whether or not we are locked out */
		if (coinlockedout[ctrnum])
			bufptr += sprintf(bufptr, " (locked)");
		*bufptr++ = '\n';
	}
	*bufptr = 0;

	/* draw the text */
	ui_menu_draw_text_box(buf);

	/* handle the keys */
	ui_menu_generic_keys(&selected, 1, 0);
	return selected;
}
#endif


/*-------------------------------------------------
    menu_game_info - display a "menu" for
    game information
-------------------------------------------------*/

static UINT32 menu_game_info(UINT32 state)
{
	char buf[2048];
	char *bufptr = buf;
	UINT32 selected = 0;

	/* add the game info */
	bufptr += sprintf_game_info(bufptr);

	/* draw the text */
	ui_menu_draw_text_box(buf);

	/* handle the keys */
	ui_menu_generic_keys(&selected, 1, 0);
	return selected;
}


/*-------------------------------------------------
    menu_cheat - display a menu for cheat options
-------------------------------------------------*/

static UINT32 menu_cheat(UINT32 state)
{
	int result = cheat_menu(Machine, state);
	if (result == 0)
		return ui_menu_stack_pop();
	return result;
}


/*-------------------------------------------------
    menu_memory_card - display a menu for memory
    card options
-------------------------------------------------*/

static UINT32 menu_memory_card(UINT32 state)
{
	ui_menu_item item_list[5];
	int menu_items = 0;
	int cardnum = state >> 16;
	UINT32 selected = state & 0xffff;
	int insertindex = -1, ejectindex = -1, createindex = -1;
	int visible_items;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* add the card select menu */
	item_list[menu_items].text = "Card Number:";
	item_list[menu_items].subtext = menu_string_pool_add("%d", cardnum);
	if (cardnum > 0)
		item_list[menu_items].flags |= MENU_FLAG_LEFT_ARROW;
	if (cardnum < 1000)
		item_list[menu_items].flags |= MENU_FLAG_RIGHT_ARROW;
	menu_items++;

	/* add the remaining items */
	item_list[insertindex = menu_items++].text = "Load Selected Card";
	if (memcard_present() != -1)
		item_list[ejectindex = menu_items++].text = "Eject Current Card";
	item_list[createindex = menu_items++].text = "Create New Card";

	/* add an item for the return */
	item_list[menu_items++].text = "Return to Prior Menu";

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, selected, NULL);

	/* handle the keys */
	if (ui_menu_generic_keys(&selected, menu_items, visible_items))
		return selected;
	if (selected == 0 && input_ui_pressed(IPT_UI_LEFT) && cardnum > 0)
		cardnum--;
	if (selected == 0 && input_ui_pressed(IPT_UI_RIGHT) && cardnum < 1000)
		cardnum++;

	/* handle actions */
	if (input_ui_pressed(IPT_UI_SELECT))
	{
		/* handle load */
		if (selected == insertindex)
		{
			if (memcard_insert(cardnum) == 0)
			{
				popmessage("Memory card loaded");
				ui_menu_stack_reset();
				return 0;
			}
			else
				popmessage("Error loading memory card");
		}

		/* handle eject */
		else if (selected == ejectindex)
		{
			memcard_eject(Machine);
			popmessage("Memory card ejected");
		}

		/* handle create */
		else if (selected == createindex)
		{
			if (memcard_create(cardnum, FALSE) == 0)
				popmessage("Memory card created");
			else
				popmessage("Error creating memory card\n(Card may already exist)");
		}
	}

	return selected | (cardnum << 16);
}


/*-------------------------------------------------
    menu_video - display a menu for video options
-------------------------------------------------*/

static UINT32 menu_video(UINT32 state)
{
	ui_menu_item item_list[100];
	render_target *targetlist[16];
	int curtarget = state >> 16;
	UINT32 selected = state & 0xffff;
	int menu_items = 0;
	int visible_items;
	int targets;

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* find the targets */
	for (targets = 0; targets < ARRAY_LENGTH(targetlist); targets++)
	{
		targetlist[targets] = render_target_get_indexed(targets);
		if (targetlist[targets] == NULL)
			break;
	}

	/* if we have a current target of 1000, we may need to select from multiple targets */
	if (curtarget == 1000)
	{
		/* count up the targets, creating menu items for them */
		for (; menu_items < targets; menu_items++)
			item_list[menu_items].text = menu_string_pool_add("Screen #%d", menu_items);

		/* if we only ended up with one, auto-select it */
		if (menu_items == 1)
			return menu_video(0 << 16 | render_target_get_view(render_target_get_indexed(0)));

		/* add an item for moving the UI */
		item_list[menu_items++].text = "Move User Interface";

		/* add an item to return */
		item_list[menu_items++].text = "Return to Prior Menu";

		/* draw the menu */
		visible_items = ui_menu_draw(item_list, menu_items, selected, NULL);

		/* handle the keys */
		if (ui_menu_generic_keys(&selected, menu_items, visible_items))
			return selected;

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			if (selected == menu_items - 2)
			{
				render_target *uitarget = render_get_ui_target();
				int targnum;

				for (targnum = 0; targnum < targets; targnum++)
					if (targetlist[targnum] == uitarget)
						break;
				targnum = (targnum + 1) % targets;
				render_set_ui_target(targetlist[targnum]);
			}
			else
				return ui_menu_stack_push(menu_video, (selected << 16) | render_target_get_view(render_target_get_indexed(selected)));
		}
	}

	/* otherwise, draw the list of layouts */
	else
	{
		render_target *target = targetlist[curtarget];
		int layermask;
		assert(target != NULL);

		/* add all the views */
		for ( ; menu_items < ARRAY_LENGTH(item_list); menu_items++)
		{
			const char *name = render_target_get_view_name(target, menu_items);
			if (name == NULL)
				break;

			/* create a string for the item */
			item_list[menu_items].text = name;
		}

		/* add an item to rotate */
		item_list[menu_items++].text = "Rotate View";

		/* add an item to enable/disable backdrops */
		layermask = render_target_get_layer_config(target);
		if (layermask & LAYER_CONFIG_ENABLE_BACKDROP)
			item_list[menu_items++].text = "Hide Backdrops";
		else
			item_list[menu_items++].text = "Show Backdrops";

		/* add an item to enable/disable overlays */
		if (layermask & LAYER_CONFIG_ENABLE_OVERLAY)
			item_list[menu_items++].text = "Hide Overlays";
		else
			item_list[menu_items++].text = "Show Overlays";

		/* add an item to enable/disable bezels */
		if (layermask & LAYER_CONFIG_ENABLE_BEZEL)
			item_list[menu_items++].text = "Hide Bezels";
		else
			item_list[menu_items++].text = "Show Bezels";

		/* add an item to enable/disable cropping */
		if (layermask & LAYER_CONFIG_ZOOM_TO_SCREEN)
			item_list[menu_items++].text = "Show Full Artwork";
		else
			item_list[menu_items++].text = "Crop to Screen";

		/* add an item to return */
		item_list[menu_items++].text = "Return to Prior Menu";

		/* draw the menu */
		visible_items = ui_menu_draw(item_list, menu_items, selected, NULL);

		/* handle the keys */
		if (ui_menu_generic_keys(&selected, menu_items, visible_items))
			return selected;

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			/* rotate */
			if (selected == menu_items - 6)
			{
				render_target_set_orientation(target, orientation_add(ROT90, render_target_get_orientation(target)));
				if (target == render_get_ui_target())
					render_container_set_orientation(render_container_get_ui(), orientation_add(ROT270, render_container_get_orientation(render_container_get_ui())));
			}

			/* show/hide backdrops */
			else if (selected == menu_items - 5)
			{
				layermask ^= LAYER_CONFIG_ENABLE_BACKDROP;
				render_target_set_layer_config(target, layermask);
			}

			/* show/hide overlays */
			else if (selected == menu_items - 4)
			{
				layermask ^= LAYER_CONFIG_ENABLE_OVERLAY;
				render_target_set_layer_config(target, layermask);
			}

			/* show/hide bezels */
			else if (selected == menu_items - 3)
			{
				layermask ^= LAYER_CONFIG_ENABLE_BEZEL;
				render_target_set_layer_config(target, layermask);
			}

			/* crop/uncrop artwork */
			else if (selected == menu_items - 2)
			{
				layermask ^= LAYER_CONFIG_ZOOM_TO_SCREEN;
				render_target_set_layer_config(target, layermask);
			}

			/* else just set the selected view */
			else
				render_target_set_view(target, selected);
		}
	}

	return selected | (curtarget << 16);
}


/*-------------------------------------------------
    menu_quit_game - handle the "menu" for
    quitting the game
-------------------------------------------------*/

static UINT32 menu_quit_game(UINT32 state)
{
	/* request a reset */
	mame_schedule_exit(Machine);

	/* reset the menu stack */
	ui_menu_stack_reset();
	return 0;
}


/*-------------------------------------------------
    menu_select_game - handle the game select
    menu
-------------------------------------------------*/

static UINT32 menu_select_game(UINT32 state)
{
	ui_menu_item item_list[VISIBLE_GAMES_IN_LIST + 2];
	int error = (state >> 17) & 1;
	int recompute = (state >> 16) & 1;
	UINT32 selected = state & 0xffff;
	int visible_items;
	int menu_items = 0;
	int curitem, curkey;
	menu_extra extra;
	int matchcount;

	/* update our driver list if necessary */
	if (select_game_driver_list == NULL)
		select_game_build_driver_list();
	for (curitem = matchcount = 0; select_game_driver_list[curitem] != NULL && matchcount < VISIBLE_GAMES_IN_LIST; curitem++)
		if (!(select_game_driver_list[curitem]->flags & GAME_NO_STANDALONE))
			matchcount++;

	/* if nothing there, just display an error message and exit */
	if (matchcount == 0)
	{
		ui_draw_text_box("No games found. Please check the rompath specified in the mame.ini file.\n\n"
						 "If this is your first time using MAME, please see the config.txt file in "
						 "the docs directory for information on configuring MAME.\n\n"
						 "Press any key to exit",
		                 JUSTIFY_CENTER, 0.5f, 0.5f, UI_REDCOLOR);
		if (input_code_poll_switches(FALSE))
			return ui_menu_stack_pop();
		return 0;
	}

	/* reset the menu and string pool */
	memset(item_list, 0, sizeof(item_list));
	menu_string_pool_offset = 0;

	/* reset the text and recompute if necessary */
	if (recompute)
		driver_list_get_approx_matches(select_game_driver_list, select_game_buffer, matchcount, select_game_list);
	recompute = 0;

	/* iterate over entries */
	for (curitem = 0; curitem < matchcount; curitem++)
	{
		const game_driver *driver = select_game_list[curitem];
		if (driver != NULL)
		{
			const game_driver *cloneof = driver_get_clone(driver);
			item_list[menu_items].text = menu_string_pool_add("%s", driver->name);
			item_list[menu_items].subtext = menu_string_pool_add("%s", driver->description);
			item_list[menu_items++].flags = (cloneof == NULL || (cloneof->flags & GAME_IS_BIOS_ROOT)) ? 0 : MENU_FLAG_INVERT;
		}
	}

	/* add an item to return, but only if we're not going to pop to the quit handler */
	if (!ui_menu_is_force_game_select())
		item_list[menu_items++].text = "Return to Prior Menu";

	/* otherwise, add a general input configuration and exit menus */
	else
	{
		item_list[menu_items++].text = "Configure General Inputs";
		item_list[menu_items++].text = "Exit";
	}

	/* configure the extra menu */
	extra.top = ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	extra.bottom = 4.0f * ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER;
	extra.render = select_game_render;
	extra.param = (selected < matchcount) ? (void *)select_game_list[selected] : NULL;

	/* draw the menu */
	visible_items = ui_menu_draw(item_list, menu_items, selected, &extra);

	/* if we have an error, overlay with the error message and look for a keypress */
	if (error)
	{
		ui_draw_text_box("The selected game is missing one or more required ROM or CHD images. "
		                 "Please select a different game.\n\nPress any key to continue.",
		                 JUSTIFY_CENTER, 0.5f, 0.5f, UI_REDCOLOR);
		if (input_code_poll_switches(FALSE))
			error = FALSE;
	}

	/* non-error case process normally */
	else
	{
		/* handle typeahead */
		recompute |= select_game_handle_key(KEYCODE_BACKSPACE, 8);
		recompute |= select_game_handle_key(KEYCODE_SPACE, ' ');
		for (curkey = KEYCODE_A; curkey <= KEYCODE_Z; curkey++)
			recompute |= select_game_handle_key(curkey, curkey - KEYCODE_A + 'a');
		for (curkey = KEYCODE_0; curkey <= KEYCODE_9; curkey++)
			recompute |= select_game_handle_key(curkey, curkey - KEYCODE_0 + '0');
		for (curkey = KEYCODE_0_PAD; curkey <= KEYCODE_9_PAD; curkey++)
			recompute |= select_game_handle_key(curkey, curkey - KEYCODE_0_PAD + '0');

		/* escape pressed with non-empty text clears the text */
		if (input_ui_pressed(IPT_UI_CANCEL))
		{
			if (select_game_buffer[0] == 0)
				return ui_menu_stack_pop();
			else
			{
				select_game_buffer[0] = 0;
				recompute = TRUE;
			}
		}

		/* if we're recomputing, reselect the first item */
		if (recompute)
			selected = 0;

		/* ignore pause keys by swallowing them */
		input_ui_pressed(IPT_UI_PAUSE);

		/* handle the generic keys */
		if (!recompute && ui_menu_generic_keys(&selected, menu_items, visible_items))
			return selected;

		/* handle actions */
		if (input_ui_pressed(IPT_UI_SELECT))
		{
			/* control config */
			if (ui_menu_is_force_game_select() && selected == menu_items - 2)
				return ui_menu_stack_push(menu_input_groups, 0);

			/* valid selected game */
			if (selected < ARRAY_LENGTH(select_game_list) && select_game_list[selected] != NULL)
			{
				audit_record *audit;
				int audit_records;
				int audit_result;

				/* audit the game first to see if we're going to work */
				audit_records = audit_images(mame_options(), select_game_list[selected], AUDIT_VALIDATE_FAST, &audit);
				audit_result = audit_summary(select_game_list[selected], audit_records, audit, FALSE);
				if (audit_records > 0)
					free(audit);

				/* if everything looks good, schedule the new driver */
				if (audit_result == CORRECT || audit_result == BEST_AVAILABLE)
				{
					mame_schedule_new_driver(Machine, select_game_list[selected]);
					ui_menu_stack_reset();
					return 0;
				}

				/* otherwise, display an error */
				else
				{
					error = TRUE;
					input_code_poll_switches(TRUE);
				}
			}
		}
	}

	return selected | (recompute << 16) | (error << 17);
}


/*-------------------------------------------------
    menu_file_manager - MESS-specific menu
-------------------------------------------------*/

#ifdef MESS
static UINT32 menu_file_manager(UINT32 state)
{
	int result = filemanager(state);
	if (result == 0)
	return ui_menu_stack_pop();
	return result;
}
#endif


/*-------------------------------------------------
    menu_tape_control - MESS-specific menu
-------------------------------------------------*/

#ifdef MESS
#if HAS_WAVE
static UINT32 menu_tape_control(UINT32 state)
{
	int result = tapecontrol(state);
	if (result == 0)
	return ui_menu_stack_pop();
	return result;
}
#endif /* HAS_WAVE */
#endif /* MESS */



/***************************************************************************
    MENU HELPERS
***************************************************************************/

/*-------------------------------------------------
    menu_render_triangle - render a triangle that
    is used for up/down arrows and left/right
    indicators
-------------------------------------------------*/

static void menu_render_triangle(mame_bitmap *dest, const mame_bitmap *source, const rectangle *sbounds, void *param)
{
	int halfwidth = dest->width / 2;
	int height = dest->height;
	int x, y;

	/* start with all-transparent */
	bitmap_fill(dest, NULL, MAKE_ARGB(0x00,0x00,0x00,0x00));

	/* render from the tip to the bottom */
	for (y = 0; y < height; y++)
	{
		int linewidth = (y * (halfwidth - 1) + (height / 2)) * 255 * 2 / height;
		UINT32 *target = BITMAP_ADDR32(dest, y, halfwidth);

		/* don't antialias if height < 12 */
		if (dest->height < 12)
		{
			int pixels = (linewidth + 254) / 255;
			if (pixels % 2 == 0) pixels++;
			linewidth = pixels * 255;
		}

		/* loop while we still have data to generate */
		for (x = 0; linewidth > 0; x++)
		{
			int dalpha;

			/* first colum we only consume one pixel */
			if (x == 0)
			{
				dalpha = MIN(0xff, linewidth);
				target[x] = MAKE_ARGB(dalpha,0xff,0xff,0xff);
			}

			/* remaining columns consume two pixels, one on each side */
			else
			{
				dalpha = MIN(0x1fe, linewidth);
				target[x] = target[-x] = MAKE_ARGB(dalpha/2,0xff,0xff,0xff);
			}

			/* account for the weight we consumed */
			linewidth -= dalpha;
		}
	}
}


/*-------------------------------------------------
    input_menu_get_items - build a list of items
    for a given group of inputs
-------------------------------------------------*/

static int input_menu_get_items(input_item_data *itemlist, int group)
{
	input_item_data *item = itemlist;
	const input_port_default_entry *indef;
	input_port_default_entry *in;
	astring *seqstring;

	/* an out of range group is special; it just means the game-specific inputs */
	if (group > IPG_TOTAL_GROUPS)
		return input_menu_get_game_items(itemlist);

	/* iterate over the input ports and add menu items */
	seqstring = astring_alloc();
	for (in = get_input_port_list(), indef = get_input_port_list_defaults(); in->type != IPT_END; in++, indef++)

		/* add if we match the group and we have a valid name */
		if (in->group == group && in->name && in->name[0] != 0)
		{
			/* build an entry for the standard sequence */
			item->seq = &in->defaultseq;
			item->defseq = &indef->defaultseq;
			item->sortorder = item - itemlist;
			item->type = port_type_is_analog(in->type) ? INPUT_TYPE_ANALOG : INPUT_TYPE_DIGITAL;
			item->name = menu_string_pool_add(input_format[item->type], in->name);
			item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, item->seq)));
			item->invert = input_seq_cmp(item->seq, item->defseq);
			item++;

			/* if we're analog, add more entries */
			if (item[-1].type == INPUT_TYPE_ANALOG)
			{
				/* build an entry for the decrement sequence */
				item->seq = &in->defaultdecseq;
				item->defseq = &indef->defaultdecseq;
				item->sortorder = item - itemlist;
				item->type = INPUT_TYPE_ANALOG_DEC;
				item->name = menu_string_pool_add(input_format[item->type], in->name);
				item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, item->seq)));
				item->invert = input_seq_cmp(item->seq, item->defseq);
				item++;

				/* build an entry for the increment sequence */
				item->seq = &in->defaultincseq;
				item->defseq = &indef->defaultincseq;
				item->sortorder = item - itemlist;
				item->type = INPUT_TYPE_ANALOG_INC;
				item->name = menu_string_pool_add(input_format[item->type], in->name);
				item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, item->seq)));
				item->invert = input_seq_cmp(item->seq, item->defseq);
				item++;
			}
		}

	/* return the number of items */
	astring_free(seqstring);
	return item - itemlist;
}


/*-------------------------------------------------
    input_menu_get_game_items - build a list of
    items for the game-specific inputs
-------------------------------------------------*/

static int input_menu_get_game_items(input_item_data *itemlist)
{
	static const input_seq default_seq = SEQ_DEF_1(SEQCODE_DEFAULT);
	astring *seqstring = astring_alloc();
	input_item_data *item = itemlist;
	input_port_entry *in;

	/* iterate over the input ports and add menu items */
	for (in = Machine->input_ports; in->type != IPT_END; in++)
	{
		const char *name = input_port_name(in);

		/* add if we match the group and we have a valid name */
		if ((name != NULL) && (input_port_condition(in)) &&
#ifdef MESS
			(in->category == 0 || input_category_active(in->category)) &&
#endif /* MESS */
			((in->type == IPT_OTHER && in->name != IP_NAME_DEFAULT) || port_type_to_group(in->type, in->player) != IPG_INVALID))
		{
			UINT16 sortorder;
			const input_seq *curseq, *defseq;

			/* determine the sorting order */
			if (in->type >= IPT_START1 && in->type <= __ipt_analog_end)
				sortorder = (in->type << 2) | (in->player << 12);
			else
				sortorder = in->type | 0xf000;

			/* fetch data for the standard sequence */
			curseq = input_port_seq(in, SEQ_TYPE_STANDARD);
			defseq = input_port_default_seq(in->type, in->player, SEQ_TYPE_STANDARD);

			/* build an entry for the standard sequence */
			item->seq = &in->seq;
			item->defseq = &default_seq;
			item->sortorder = sortorder;
			item->type = port_type_is_analog(in->type) ? INPUT_TYPE_ANALOG : INPUT_TYPE_DIGITAL;
			item->name = menu_string_pool_add(input_format[item->type], name);
			item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, curseq)));
			item->invert = input_seq_cmp(curseq, defseq);
			item++;

			/* if we're analog, add more entries */
			if (item[-1].type == INPUT_TYPE_ANALOG)
			{
				/* fetch data for the decrement sequence */
				curseq = input_port_seq(in, SEQ_TYPE_DECREMENT);
				defseq = input_port_default_seq(in->type, in->player, SEQ_TYPE_DECREMENT);

				/* build an entry for the decrement sequence */
				item->seq = &in->analog.decseq;
				item->defseq = &default_seq;
				item->sortorder = sortorder;
				item->type = INPUT_TYPE_ANALOG_DEC;
				item->name = menu_string_pool_add(input_format[item->type], name);
				item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, curseq)));
				item->invert = input_seq_cmp(curseq, defseq);
				item++;

				/* fetch data for the increment sequence */
				curseq = input_port_seq(in, SEQ_TYPE_INCREMENT);
				defseq = input_port_default_seq(in->type, in->player, SEQ_TYPE_INCREMENT);

				/* build an entry for the increment sequence */
				item->seq = &in->analog.incseq;
				item->defseq = &default_seq;
				item->sortorder = sortorder;
				item->type = INPUT_TYPE_ANALOG_INC;
				item->name = menu_string_pool_add(input_format[item->type], name);
				item->seqname = menu_string_pool_add("%s", astring_c(input_seq_name(seqstring, curseq)));
				item->invert = input_seq_cmp(curseq, defseq);
				item++;
			}
		}
	}

	/* return the number of items */
	astring_free(seqstring);
	return item - itemlist;
}


/*-------------------------------------------------
    input_menu_toggle_none_default - toggle
    between "NONE" and the default item
-------------------------------------------------*/

static void input_menu_toggle_none_default(input_seq *selected_seq, input_seq *original_seq, const input_seq *selected_defseq)
{
	/* if we used to be "none", toggle to the default value */
	if (input_seq_get_1(original_seq) == SEQCODE_END)
		*selected_seq = *selected_defseq;

	/* otherwise, toggle to "none" */
	else
		input_seq_set_1(selected_seq, SEQCODE_END);
}


/*-------------------------------------------------
    input_menu_compare_items - compare two
    items for quicksort
-------------------------------------------------*/

static int input_menu_compare_items(const void *i1, const void *i2)
{
	const ui_menu_item *item1 = i1;
	const ui_menu_item *item2 = i2;
	const input_item_data *data1 = item1->ref;
	const input_item_data *data2 = item2->ref;
	if (data1->sortorder < data2->sortorder)
		return -1;
	if (data1->sortorder > data2->sortorder)
		return 1;
	if (data1->type < data2->type)
		return -1;
	if (data1->type > data2->type)
		return 1;
	return 0;
}


/*-------------------------------------------------
    switches_menu_add_item - add an item to the
    switches menu list
-------------------------------------------------*/

static void switches_menu_add_item(ui_menu_item *item, const input_port_entry *in, int switch_entry, void *ref)
{
	const input_port_entry *tin;

	/* set the text to the name and the subitem text to invalid */
	item->text = input_port_name(in);
	item->subtext = NULL;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if this is a match, set the subtext */
			if (in->default_value == tin->default_value)
				item->subtext = input_port_name(tin);

			/* else if we haven't seen a match yet, show a left arrow */
			else if (!item->subtext)
				item->flags |= MENU_FLAG_LEFT_ARROW;

			/* else if we have seen a match, show a right arrow */
			else
				item->flags |= MENU_FLAG_RIGHT_ARROW;
		}

	/* if no matches, we're invalid */
	if (!item->subtext)
		item->subtext = "INVALID";

	/* stash our reference */
	item->ref = ref;
}


/*-------------------------------------------------
    switches_menu_select_previous - select the
    previous item in the switches list
-------------------------------------------------*/

static void switches_menu_select_previous(input_port_entry *in, int switch_entry)
{
	int last_value = in->default_value;
	const input_port_entry *tin;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if this is a match, we're done */
			if (in->default_value == tin->default_value)
			{
				in->default_value = last_value;
				return;
			}

			/* otherwise, keep track of last one found */
			else
				last_value = tin->default_value;
		}
}


/*-------------------------------------------------
    switches_menu_select_next - select the
    next item in the switches list
-------------------------------------------------*/

static void switches_menu_select_next(input_port_entry *in, int switch_entry)
{
	const input_port_entry *tin;
	int foundit = FALSE;

	/* scan for the current selection in the list */
	for (tin = in + 1; tin->type == switch_entry; tin++)
		if (input_port_condition(tin))
		{
			/* if we found the current selection, we pick the next one */
			if (foundit)
			{
				in->default_value = tin->default_value;
				return;
			}

			/* if this is a match, note it */
			if (in->default_value == tin->default_value)
				foundit = TRUE;
		}
}


/*-------------------------------------------------
    switches_menu_compare_items - compare two
    switches items for quicksort purposes
-------------------------------------------------*/

/*
static int switches_menu_compare_items(const void *i1, const void *i2)
{
    const ui_menu_item *item1 = i1;
    const ui_menu_item *item2 = i2;
    const input_port_entry *data1 = item1->ref;
    const input_port_entry *data2 = item2->ref;
    return strcmp(input_port_name(data1), input_port_name(data2));
}
*/


/*-------------------------------------------------
    analog_menu_add_item - add an item to the
    analog controls menu
-------------------------------------------------*/

static void analog_menu_add_item(ui_menu_item *item, const input_port_entry *in, const char *append_string, int which_item)
{
	int value, minval, maxval;

	/* set the item text using the formatting string provided */
	item->text = menu_string_pool_add("%s %s", input_port_name(in), append_string);

	/* set the subitem text */
	switch (which_item)
	{
		default:
		case ANALOG_ITEM_KEYSPEED:
			value = in->analog.delta;
			minval = 0;
			maxval = 255;
			item->subtext = menu_string_pool_add("%d", value);
			break;

		case ANALOG_ITEM_CENTERSPEED:
			value = in->analog.centerdelta;
			minval = 0;
			maxval = 255;
			item->subtext = menu_string_pool_add("%d", value);
			break;

		case ANALOG_ITEM_REVERSE:
			value = in->analog.reverse;
			minval = 0;
			maxval = 1;
			item->subtext = value ? "On" : "Off";
			break;

		case ANALOG_ITEM_SENSITIVITY:
			value = in->analog.sensitivity;
			minval = 1;
			maxval = 255;
			item->subtext = menu_string_pool_add("%d%%", value);
			break;
	}

	/* put on arrows */
	if (value > minval)
		item->flags |= MENU_FLAG_LEFT_ARROW;
	if (value < maxval)
		item->flags |= MENU_FLAG_RIGHT_ARROW;
}


/*-------------------------------------------------
    dip_switch_build_model - build up the model
    for DIP switches
-------------------------------------------------*/

static void dip_switch_build_model(input_port_entry *entry, int item_is_selected)
{
	int dip_declaration_index = 0;
	int value_mask_temp = entry->mask;
	int value_mask_bit = 0;
	int toggle_switch_mask;
	int model_index = 0;
	int toggle_num;

	if (entry->diploc[dip_declaration_index].swname == NULL)
		return;

	/* get the entry in the model to work with */
	do
	{
		/* use this entry if it's not used */
		if (dip_switch_model[model_index].dip_name == NULL)
		{
			dip_switch_model[model_index].dip_name = entry->diploc[dip_declaration_index].swname;
			break;
		}

		/* reuse this entry if the switch name matches */
		if (!strcmp(entry->diploc[dip_declaration_index].swname, dip_switch_model[model_index].dip_name))
			break;

		// todo: add a check here to see if we go over the max dips and throw an error.
	} while (++model_index < MAX_PHYSICAL_DIPS);

	/* Create a mask depicting the number of toggles on the physical switch */
	while (entry->diploc[dip_declaration_index].swname)
	{
		/* get the Nth DIP toggle number */
		toggle_num = entry->diploc[dip_declaration_index].swnum;

		/* get the Nth mask bit -
         * should probably put a check here to avoid bad driver definitions
         * which could put us into an infinite loop. */
		while (!(value_mask_temp & 1))
		{
			value_mask_temp >>= 1;
			++value_mask_bit;
		}
		/* clear out the lsb to keep it moving next iteration. */
		value_mask_temp &= ~1;

		toggle_switch_mask = 1 << (toggle_num - 1);

		/* indicate the toggle exists in the switch */
		dip_switch_model[model_index].total_dip_mask |= toggle_switch_mask;

		/* if it's inverted, mark it as such */
		if (entry->diploc[dip_declaration_index].invert)
			dip_switch_model[model_index].dip_invert_mask |= toggle_switch_mask;

		/* if isolated bit is on, set the toggle on */
		if ((1 << value_mask_bit) & entry->default_value)
			dip_switch_model[model_index].total_dip_settings |= toggle_switch_mask;

		/* indicate if the toggle is selected */
		if (item_is_selected)
			dip_switch_model[model_index].selected_dip_feature_mask |= toggle_switch_mask;

		++dip_declaration_index;
	}
}


/*-------------------------------------------------
    dip_switch_draw_one - draw a single DIP
    switch
-------------------------------------------------*/

static void dip_switch_draw_one(float dip_menu_x1, float dip_menu_y1, float dip_menu_x2, float dip_menu_y2, int model_index)
{
	int toggle_index;
	int num_toggles = 0;
	float segment_start_x;
	float dip_field_y;
	float y1_on, y2_on, y1_off, y2_off;
	float switch_toggle_gap;
	float name_width;

	/* determine the number of toggles in the DIP */
	for (toggle_index = 0; toggle_index < 16; toggle_index++)
		if (dip_switch_model[model_index].total_dip_mask & (1 << toggle_index))
			num_toggles = toggle_index + 1;

	/* calculate the starting x coordinate so that the entire switch is centered */
	dip_field_y = dip_menu_y1 + DIP_SWITCH_SPACING + (model_index * (DIP_SWITCH_SPACING + DIP_SWITCH_HEIGHT));

	switch_toggle_gap = ((DIP_SWITCH_HEIGHT/2) - SINGLE_TOGGLE_SWITCH_HEIGHT)/2;

 	segment_start_x = dip_menu_x1 + (dip_menu_x2 - dip_menu_x1 - num_toggles * SINGLE_TOGGLE_SWITCH_FIELD_WIDTH) / 2;

	y1_off = dip_field_y + UI_LINE_WIDTH + switch_toggle_gap;
	y2_off = y1_off + SINGLE_TOGGLE_SWITCH_HEIGHT;

	y1_on = dip_field_y + DIP_SWITCH_HEIGHT/2 + switch_toggle_gap;
	y2_on = y1_on + SINGLE_TOGGLE_SWITCH_HEIGHT;

	for (toggle_index = 0; toggle_index < num_toggles; toggle_index++)
	{
		int bit_mask, dip_on;
		float dip_field_x1, x1, x2;

		bit_mask = 1 << toggle_index;

		/* draw the field for a single toggle on a DIP switch */
		dip_field_x1 = segment_start_x + (SINGLE_TOGGLE_SWITCH_FIELD_WIDTH * toggle_index);

		ui_draw_outlined_box(dip_field_x1,
							 dip_field_y,
							 dip_field_x1 + SINGLE_TOGGLE_SWITCH_FIELD_WIDTH,
							 dip_field_y + DIP_SWITCH_HEIGHT,
							 UI_FILLCOLOR);

		x1 = dip_field_x1 + (SINGLE_TOGGLE_SWITCH_FIELD_WIDTH - SINGLE_TOGGLE_SWITCH_WIDTH) / 2;
		x2 = x1 + SINGLE_TOGGLE_SWITCH_WIDTH;

		/* see if the switch is actually used */
		if (dip_switch_model[model_index].total_dip_mask & bit_mask)
		{
			/* yes, draw the switch position for a single toggle switch in a switch field */
			int feature_field_selected = ((bit_mask & dip_switch_model[model_index].selected_dip_feature_mask) != 0);
			dip_on = ((bit_mask & (dip_switch_model[model_index].total_dip_settings ^ dip_switch_model[model_index].dip_invert_mask)) != 0);

			render_ui_add_rect(x1, dip_on ? y1_on : y1_off,
							   x2, dip_on ? y2_on : y2_off,
							   feature_field_selected ? MENU_SELECTCOLOR : ARGB_WHITE,
							   PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			/* no, draw it grayed out */
			render_ui_add_rect(x1, y1_off,
							   x2, y2_on,
							   MENU_UNAVAILABLECOLOR,
							   PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
	}

	/* add the dip switch name */
	name_width = ui_get_string_width(dip_switch_model[model_index].dip_name) + ui_get_string_width(" ") / 2;

	ui_draw_text_full(	dip_switch_model[model_index].dip_name,
						segment_start_x - name_width,
						dip_field_y + (DIP_SWITCH_HEIGHT - UI_TARGET_FONT_HEIGHT)/2,
						name_width,
						JUSTIFY_LEFT,
						WRAP_NEVER,
						DRAW_NORMAL,
						ARGB_WHITE,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA),
						NULL ,
						NULL);
}


/*-------------------------------------------------
    dip_switch_render - perform our special
    rendering
-------------------------------------------------*/

static void dip_switch_render(const menu_extra *extra, float x1, float y1, float x2, float y2)
{
	int num_dips;
	int dip_model_index;
	float dip_menu_y1, dip_menu_y2;

	/* how many dips does this game have? */
	num_dips = get_num_dips();

	dip_menu_y1 = y2 + UI_BOX_TB_BORDER;
	dip_menu_y2 = dip_menu_y1 + extra->bottom;

	/* draw extra menu area */
	ui_draw_outlined_box(x1, dip_menu_y1, x2, dip_menu_y2, UI_FILLCOLOR);

	/* draw all the dip switches */
	for (dip_model_index = 0; dip_model_index < num_dips; dip_model_index++)
		dip_switch_draw_one(x1, dip_menu_y1, x2, dip_menu_y2, dip_model_index);
}


/*-------------------------------------------------
    select_game_driver_compare - compare the
    names of two drivers
-------------------------------------------------*/

static int CLIB_DECL select_game_driver_compare(const void *elem1, const void *elem2)
{
	const game_driver **driver1_ptr = (const game_driver **)elem1;
	const game_driver **driver2_ptr = (const game_driver **)elem2;
	const char *driver1 = (*driver1_ptr)->name;
	const char *driver2 = (*driver2_ptr)->name;

	while (*driver1 == *driver2 && *driver1 != 0)
		driver1++, driver2++;
	return *driver1 - *driver2;
}


/*-------------------------------------------------
    select_game_build_driver_list - build a list
    of available drivers
-------------------------------------------------*/

static void select_game_build_driver_list(void)
{
	int driver_count = driver_list_get_count(drivers);
	int drivnum, listnum;
	mame_path *path;
	UINT8 *found;

	/* first allocate a copy of the full driver list */
	if (select_game_driver_list == NULL)
		select_game_driver_list = malloc_or_die(driver_count * sizeof(*select_game_driver_list));
	memcpy((void *)select_game_driver_list, drivers, driver_count * sizeof(*select_game_driver_list));

	/* sort it */
	qsort((void *)select_game_driver_list, driver_count, sizeof(*select_game_driver_list), select_game_driver_compare);

	/* allocate a temporary array to track which ones we found */
	found = malloc_or_die((driver_count + 1) * sizeof(*found));
	memset(found, 0, (driver_count + 1) * sizeof(*found));

	/* open a path to the ROMs and find them in the array */
	path = mame_openpath(mame_options(), OPTION_ROMPATH);
	if (path != NULL)
	{
		const osd_directory_entry *dir;

		/* iterate while we get new objects */
		while ((dir = mame_readpath(path)) != NULL)
		{
			game_driver tempdriver;
			game_driver *tempdriver_ptr;
			const game_driver **found_driver;
			char drivername[50];
			char *dst = drivername;
			const char *src;

			/* build a name for it */
			for (src = dir->name; *src != 0 && *src != '.' && dst < &drivername[ARRAY_LENGTH(drivername) - 1]; src++)
				*dst++ = tolower(*src);
			*dst = 0;

			/* find it in the array */
			tempdriver.name = drivername;
			tempdriver_ptr = &tempdriver;
			found_driver = bsearch(&tempdriver_ptr, select_game_driver_list, driver_count, sizeof(*select_game_driver_list), select_game_driver_compare);

			/* if found, mark the corresponding entry in the array */
			if (found_driver != NULL)
				found[found_driver - select_game_driver_list] = TRUE;
		}
		mame_closepath(path);
	}

	/* now build the final list */
	for (drivnum = listnum = 0; drivnum < driver_count; drivnum++)
		if (found[drivnum])
			select_game_driver_list[listnum++] = select_game_driver_list[drivnum];

	/* NULL-terminate */
	select_game_driver_list[listnum] = NULL;
	free(found);
}


/*-------------------------------------------------
    select_game_render - perform our special
    rendering
-------------------------------------------------*/

static void select_game_render(const menu_extra *extra, float origx1, float origy1, float origx2, float origy2)
{
	const game_driver *driver;
	float width, maxwidth;
	float x1, y1, x2, y2;
	char tempbuf[4][256];
	rgb_t color;
	int line;

	/* display the current typeahead */
	if (select_game_buffer[0] != 0)
		sprintf(&tempbuf[0][0], "Type name or select: %s_", select_game_buffer);
	else
		sprintf(&tempbuf[0][0], "Type name or select: (random)");

	/* get the size of the text */
	ui_draw_text_full(&tempbuf[0][0], 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
					  DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
	width += 2 * UI_BOX_LR_BORDER;
	maxwidth = MAX(width, origx2 - origx1);

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy1 - extra->top;
	y2 = origy1 - UI_BOX_TB_BORDER;

	/* draw a box */
	ui_draw_outlined_box(x1, y1, x2, y2, UI_FILLCOLOR);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw the text within it */
	ui_draw_text_full(&tempbuf[0][0], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
					  DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);

	/* determine the text to render below */
	driver = extra->param;
	if (driver != NULL)
	{
		const char *gfxstat, *soundstat;

		/* first line is game name */
		sprintf(&tempbuf[0][0], "%-.100s", driver->description);

		/* next line is year, manufacturer */
		sprintf(&tempbuf[1][0], "%s, %-.100s", driver->year, driver->manufacturer);

		/* next line is overall driver status */
		if (driver->flags & GAME_NOT_WORKING)
			strcpy(&tempbuf[2][0], "Overall: NOT WORKING");
		else if (driver->flags & GAME_UNEMULATED_PROTECTION)
			strcpy(&tempbuf[2][0], "Overall: Unemulated Protection");
		else
			strcpy(&tempbuf[2][0], "Overall: Working");

		/* next line is graphics, sound status */
		if (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS))
			gfxstat = "Imperfect";
		else
			gfxstat = "OK";

		if (driver->flags & GAME_NO_SOUND)
			soundstat = "Unimplemented";
		else if (driver->flags & GAME_IMPERFECT_SOUND)
			soundstat = "Imperfect";
		else
			soundstat = "OK";

		sprintf(&tempbuf[3][0], "Gfx: %s, Sound: %s", gfxstat, soundstat);
	}
	else
	{
		int line = 0;
		int col = 0;
		const char *s = COPYRIGHT;

		/* first line is version string */
		sprintf(&tempbuf[line++][0], "%s %s", APPLONGNAME, build_version);

		/* output message */
		while(line < ARRAY_LENGTH(tempbuf))
		{
			if ((*s == '\0') || (*s == '\n'))
			{
				tempbuf[line++][col] = '\0';
				col = 0;
			}
			else
			{
				tempbuf[line][col++] = *s;
			}

			if (*s != '\0')
				s++;
		}
	}

	/* get the size of the text */
	maxwidth = origx2 - origx1;
	for (line = 0; line < 4; line++)
	{
		ui_draw_text_full(&tempbuf[line][0], 0.0f, 0.0f, 1.0f, JUSTIFY_CENTER, WRAP_TRUNCATE,
						  DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &width, NULL);
		width += 2 * UI_BOX_LR_BORDER;
		maxwidth = MAX(maxwidth, width);
	}

	/* compute our bounds */
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy2 + UI_BOX_TB_BORDER;
	y2 = origy2 + extra->bottom;

	/* draw a box */
	color = UI_FILLCOLOR;
	if (driver != NULL && (driver->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0)
		color = UI_REDCOLOR;
	ui_draw_outlined_box(x1, y1, x2, y2, color);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw all lines */
	for (line = 0; line < 4; line++)
	{
		ui_draw_text_full(&tempbuf[line][0], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
						  DRAW_NORMAL, ARGB_WHITE, ARGB_BLACK, NULL, NULL);
		y1 += ui_get_line_height();
	}
}


/*-------------------------------------------------
    select_game_handle_key - handle a keypress
-------------------------------------------------*/

static int select_game_handle_key(input_code keycode, char value)
{
	/* is this key pressed? */
	if (input_code_pressed_once(keycode))
	{
		int buflen = strlen(select_game_buffer);

		/* if it's a backspace and we can handle it, do so */
		if (value == 8)
		{
			if (buflen > 0)
			{
				select_game_buffer[buflen - 1] = 0;
				return TRUE;
			}
		}

		/* if it's any other key and we're not maxed out, update */
		else if (buflen < ARRAY_LENGTH(select_game_buffer) - 1)
		{
			select_game_buffer[buflen] = value;
			select_game_buffer[buflen + 1] = 0;
			return TRUE;
		}
	}
	return FALSE;
}
