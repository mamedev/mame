/*********************************************************************

    uimenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "ui.h"
#include "rendutil.h"
#include "cheat.h"
#include "uiinput.h"
#include "uimenu.h"
#include "audit.h"
#include "eminline.h"

#ifdef MESS
#include "uimess.h"
#include "inputx.h"
#endif /* MESS */

#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE		65536
#define UI_MENU_ALLOC_ITEMS		256

#define VISIBLE_GAMES_IN_LIST	15

#define MAX_PHYSICAL_DIPS		10
#define MAX_INPUT_PORTS			32
#define MAX_BITS_PER_PORT		32

/* DIP switch rendering parameters */
#define DIP_SWITCH_HEIGHT		0.05f
#define DIP_SWITCH_SPACING		0.01
#define SINGLE_TOGGLE_SWITCH_FIELD_WIDTH 0.025f
#define SINGLE_TOGGLE_SWITCH_WIDTH 0.020f
/* make the switch 80% of the width space and 1/2 of the switch height */
#define PERCENTAGE_OF_HALF_FIELD_USED 0.80f
#define SINGLE_TOGGLE_SWITCH_HEIGHT ((DIP_SWITCH_HEIGHT / 2) * PERCENTAGE_OF_HALF_FIELD_USED)


enum
{
	INPUT_TYPE_DIGITAL = 0,
	INPUT_TYPE_ANALOG = 1,
	INPUT_TYPE_ANALOG_DEC = INPUT_TYPE_ANALOG + SEQ_TYPE_DECREMENT,
	INPUT_TYPE_ANALOG_INC = INPUT_TYPE_ANALOG + SEQ_TYPE_INCREMENT,
	INPUT_TYPE_TOTAL = INPUT_TYPE_ANALOG + SEQ_TYPE_TOTAL
};

enum
{
	ANALOG_ITEM_KEYSPEED = 0,
	ANALOG_ITEM_CENTERSPEED,
	ANALOG_ITEM_REVERSE,
	ANALOG_ITEM_SENSITIVITY,
	ANALOG_ITEM_COUNT
};

enum
{
	MEMCARD_ITEM_SELECT = 1,
	MEMCARD_ITEM_LOAD,
	MEMCARD_ITEM_EJECT,
	MEMCARD_ITEM_CREATE
};

enum
{
	VIDEO_ITEM_ROTATE = 0x80000000,
	VIDEO_ITEM_VIEW
};

enum
{
	CROSSHAIR_ITEM_VIS = 0,
	CROSSHAIR_ITEM_PIC,
	CROSSHAIR_ITEM_AUTO_TIME
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _ui_menu_pool ui_menu_pool;
struct _ui_menu_pool
{
	ui_menu_pool *		next;			/* chain to next one */
	UINT8 *				top;			/* top of the pool */
	UINT8 *				end;			/* end of the pool */
};


typedef struct _ui_menu_item ui_menu_item;
struct _ui_menu_item
{
   const char *			text;
   const char *			subtext;
   UINT32 				flags;
   void *				ref;
};


struct _ui_menu
{
	running_machine *	machine;			/* machine we are attached to */
	ui_menu_handler_func handler;			/* handler callback */
	void *				parameter;			/* parameter */
	ui_menu_event		event;				/* the UI event that occurred */
	ui_menu *			parent;				/* pointer to parent menu */
	void *				state;				/* menu-specific state */
	ui_menu_destroy_state_func destroy_state; /* destroy state callback */
	int					resetpos;			/* reset position */
	void *				resetref;			/* reset reference */
	int					selected;			/* which item is selected */
	int					hover;				/* which item is being hovered over */
	int					visitems;			/* number of visible items */
	int					numitems;			/* number of items in the menu */
	int					allocitems;			/* allocated size of array */
	ui_menu_item *		item;				/* pointer to array of items */
	ui_menu_custom_func custom;				/* callback for custom rendering */
	float				customtop;			/* amount of extra height to add at the top */
	float				custombottom;		/* amount of extra height to add at the bottom */
	ui_menu_pool *		pool;				/* list of memory pools */
};


/* internal input menu item data */
typedef struct _input_item_data input_item_data;
struct _input_item_data
{
	input_item_data *	next;				/* pointer to next item in the list */
	const void *		ref;				/* reference to type description for global inputs or field for game inputs */
	input_seq_type		seqtype;			/* sequence type */
	input_seq			seq;				/* copy of the live sequence */
	const input_seq *	defseq;				/* pointer to the default sequence */
	const char *		name;				/* pointer to the base name of the item */
	UINT16 				sortorder;			/* sorting information */
	UINT8 				type;				/* type of port */
};


/* internal analog menu item data */
typedef struct _analog_item_data analog_item_data;
struct _analog_item_data
{
	const input_field_config *field;
	int					type;
	int					min, max;
	int					cur;
	int 				defvalue;
};


/* DIP switch descriptor */
typedef struct _dip_descriptor dip_descriptor;
struct _dip_descriptor
{
	dip_descriptor *	next;
	const char *	 	name;
	UINT32				mask;
	UINT32				state;
};


/* extended settings menu state */
typedef struct _settings_menu_state settings_menu_state;
struct _settings_menu_state
{
	dip_descriptor *	diplist;
};


/* extended input menu state */
typedef struct _input_menu_state input_menu_state;
struct _input_menu_state
{
	UINT16 				last_sortorder;
	const void *		pollingref;
	input_item_data *	pollingitem;
	UINT8				record_next;
	input_seq 			starting_seq;
};


/* extended select game menu state */
typedef struct _select_game_state select_game_state;
struct _select_game_state
{
	UINT8				error;
	UINT8				rerandomize;
	char				search[40];
	const game_driver *	matchlist[VISIBLE_GAMES_IN_LIST];
	const game_driver *	driverlist[1];
};


/* internal crosshair menu item data */
typedef struct _crosshair_item_data crosshair_item_data;
struct _crosshair_item_data
{
	UINT8				type;
	UINT8				player;
	UINT8				min, max;
	UINT8				cur;
	UINT8				defvalue;
	char				last_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
	char				next_name[CROSSHAIR_PIC_NAME_LENGTH + 1];
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* menu states */
static ui_menu *menu_stack;
static ui_menu *menu_free;

static bitmap_t *hilight_bitmap;
static render_texture *hilight_texture;

static render_texture *arrow_texture;

static const char priortext[] = "Return to Prior Menu";
static const char backtext[] = "Return to " CAPSTARTGAMENOUN;
static const char exittext[] = "Exit";



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void ui_menu_exit(running_machine *machine);

/* internal menu processing */
static void ui_menu_draw(running_machine *machine, ui_menu *menu, int customonly);
static void ui_menu_draw_text_box(ui_menu *menu);
static void ui_menu_handle_events(ui_menu *menu);
static void ui_menu_handle_keys(ui_menu *menu, UINT32 flags);
static void ui_menu_validate_selection(ui_menu *menu, int scandir);
static void ui_menu_clear_free_list(running_machine *machine);

/* menu handlers */
static void menu_main(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_main_populate(running_machine *machine, ui_menu *menu, void *state);
static void menu_input_groups(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_input_groups_populate(running_machine *machine, ui_menu *menu, void *state);
static void menu_input_general(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_input_general_populate(running_machine *machine, ui_menu *menu, input_menu_state *menustate, int group);
static void menu_input_specific(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_input_specific_populate(running_machine *machine, ui_menu *menu, input_menu_state *menustate);
static void menu_input_common(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static int CLIB_DECL menu_input_compare_items(const void *i1, const void *i2);
static void menu_input_populate_and_sort(running_machine *machine, ui_menu *menu, input_item_data *itemlist, input_menu_state *menustate);
static void menu_settings_dip_switches(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_settings_driver_config(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_settings_categories(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_settings_common(running_machine *machine, ui_menu *menu, void *state, UINT32 type);
static void menu_settings_populate(running_machine *machine, ui_menu *menu, settings_menu_state *menustate, UINT32 type);
static void menu_analog(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_analog_populate(running_machine *machine, ui_menu *menu);
#ifndef MESS
static void menu_bookkeeping(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_bookkeeping_populate(running_machine *machine, ui_menu *menu, attotime *curtime);
#endif
static void menu_game_info(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_cheat(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_cheat_populate(running_machine *machine, ui_menu *menu);
static void menu_memory_card(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_memory_card_populate(running_machine *machine, ui_menu *menu, int cardnum);
static void menu_sliders(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_sliders_populate(running_machine *machine, ui_menu *menu, int menuless_mode);
static void menu_sliders_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2);
static void menu_video_targets(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_video_targets_populate(running_machine *machine, ui_menu *menu);
static void menu_video_options(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_video_options_populate(running_machine *machine, ui_menu *menu, render_target *target);
static void menu_crosshair(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_crosshair_populate(running_machine *machine, ui_menu *menu);
static void menu_quit_game(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_select_game(running_machine *machine, ui_menu *menu, void *parameter, void *state);
static void menu_select_game_populate(running_machine *machine, ui_menu *menu, select_game_state *menustate);
static int CLIB_DECL menu_select_game_driver_compare(const void *elem1, const void *elem2);
static void menu_select_game_build_driver_list(ui_menu *menu, select_game_state *menustate);
static void menu_select_game_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

/* menu helpers */
static void menu_render_triangle(bitmap_t *dest, const bitmap_t *source, const rectangle *sbounds, void *param);
static void menu_settings_custom_render_one(float x1, float y1, float x2, float y2, const dip_descriptor *dip, UINT32 selectedmask);
static void menu_settings_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x, float y, float x2, float y2);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_field_default_seq - return a pointer
    to the default sequence for the given field
-------------------------------------------------*/

INLINE const input_seq *get_field_default_seq(const input_field_config *field, input_seq_type seqtype)
{
	if (input_seq_get_1(&field->seq[seqtype]) == SEQCODE_DEFAULT)
		return input_type_seq(field->port->machine, field->type, field->player, seqtype);
	else
		return &field->seq[seqtype];
}


/*-------------------------------------------------
    toggle_none_default - toggle between "NONE"
    and the default item
-------------------------------------------------*/

INLINE void toggle_none_default(input_seq *selected_seq, input_seq *original_seq, const input_seq *selected_defseq)
{
	/* if we used to be "none", toggle to the default value */
	if (input_seq_get_1(original_seq) == SEQCODE_END)
		*selected_seq = *selected_defseq;

	/* otherwise, toggle to "none" */
	else
		input_seq_set_1(selected_seq, SEQCODE_END);
}


/*-------------------------------------------------
    item_is_selectable - return TRUE if the given
    item is selectable
-------------------------------------------------*/

INLINE int item_is_selectable(const ui_menu_item *item)
{
	return ((item->flags & (MENU_FLAG_MULTILINE | MENU_FLAG_DISABLE)) == 0 && strcmp(item->text, MENU_SEPARATOR_ITEM) != 0);
}


/*-------------------------------------------------
    exclusive_input_pressed - return TRUE if the
    given key is pressed and we haven't already
    reported a key
-------------------------------------------------*/

INLINE int exclusive_input_pressed(ui_menu *menu, int key, int repeat)
{
	if (menu->event.iptkey == IPT_INVALID && ui_input_pressed_repeat(menu->machine, key, repeat))
	{
		menu->event.iptkey = key;
		return TRUE;
	}
	return FALSE;
}



/***************************************************************************
    CORE SYSTEM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    ui_menu_init - initialize the menu system
-------------------------------------------------*/

void ui_menu_init(running_machine *machine)
{
	int x;

	/* initialize the menu stack */
	ui_menu_stack_reset(machine);

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
	render_texture_set_bitmap(hilight_texture, hilight_bitmap, NULL, TEXFORMAT_ARGB32, NULL);

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
	/* free menus */
	ui_menu_stack_reset(machine);
	ui_menu_clear_free_list(machine);

	/* free textures */
	render_texture_free(hilight_texture);
	bitmap_free(hilight_bitmap);
	render_texture_free(arrow_texture);
}



/***************************************************************************
    CORE MENU MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    ui_menu_alloc - allocate a new menu
-------------------------------------------------*/

ui_menu *ui_menu_alloc(running_machine *machine, ui_menu_handler_func handler, void *parameter)
{
	ui_menu *menu;

	/* allocate and clear memory for the menu and the state */
	menu = alloc_clear_or_die(ui_menu);

	/* initialize the state */
	menu->machine = machine;
	menu->handler = handler;
	menu->parameter = parameter;

	/* reset the menu (adds a default entry) */
	ui_menu_reset(menu, UI_MENU_RESET_SELECT_FIRST);
	return menu;
}


/*-------------------------------------------------
    ui_menu_free - free a menu
-------------------------------------------------*/

void ui_menu_free(ui_menu *menu)
{
	/* free the pools */
	while (menu->pool != NULL)
	{
		ui_menu_pool *pool = menu->pool;
		menu->pool = pool->next;
		free(pool);
	}

	/* free the item array */
	if (menu->item != NULL)
		free(menu->item);

	/* free the state */
	if (menu->state != NULL)
	{
		if (menu->destroy_state != NULL)
			(*menu->destroy_state)(menu, menu->state);
		free(menu->state);
	}

	/* free the menu */
	free(menu);
}


/*-------------------------------------------------
    ui_menu_reset - free all items in the menu,
    and all memory allocated from the memory pool
-------------------------------------------------*/

void ui_menu_reset(ui_menu *menu, ui_menu_reset_options options)
{
	ui_menu_pool *pool;

	/* based on the reset option, set the reset info */
	menu->resetpos = 0;
	menu->resetref = NULL;
	if (options == UI_MENU_RESET_REMEMBER_POSITION)
		menu->resetpos = menu->selected;
	else if (options == UI_MENU_RESET_REMEMBER_REF)
		menu->resetref = menu->item[menu->selected].ref;

	/* reset all the pools and the numitems back to 0 */
	for (pool = menu->pool; pool != NULL; pool = pool->next)
		pool->top = (UINT8 *)(pool + 1);
	menu->numitems = 0;
	menu->visitems = 0;
	menu->selected = 0;

	/* add an item to return */
	if (menu->parent == NULL)
		ui_menu_item_append(menu, backtext, NULL, 0, NULL);
	else if (menu->parent->handler == menu_quit_game)
		ui_menu_item_append(menu, exittext, NULL, 0, NULL);
	else
		ui_menu_item_append(menu, priortext, NULL, 0, NULL);
}


/*-------------------------------------------------
    ui_menu_populated - returns TRUE if the menu
    has any non-default items in it
-------------------------------------------------*/

int ui_menu_populated(ui_menu *menu)
{
	return (menu->numitems > 1);
}


/*-------------------------------------------------
    ui_menu_item_append - append a new item to the
    end of the menu
-------------------------------------------------*/

void ui_menu_item_append(ui_menu *menu, const char *text, const char *subtext, UINT32 flags, void *ref)
{
	ui_menu_item *item;
	int index;

	/* only allow multiline as the first item */
	if ((flags & MENU_FLAG_MULTILINE) != 0)
		assert(menu->numitems == 1);

	/* only allow a single multi-line item */
	else if (menu->numitems >= 2)
		assert((menu->item[0].flags & MENU_FLAG_MULTILINE) == 0);

	/* realloc the item array if necessary */
	if (menu->numitems >= menu->allocitems)
	{
		menu->allocitems += UI_MENU_ALLOC_ITEMS;
		menu->item = (ui_menu_item *)realloc(menu->item, menu->allocitems * sizeof(*item));
	}
	index = menu->numitems++;

	/* copy the previous last item to the next one */
	if (index != 0)
	{
		index--;
		menu->item[index + 1] = menu->item[index];
	}

	/* allocate a new item and populte it */
	item = &menu->item[index];
	item->text = (text != NULL) ? ui_menu_pool_strdup(menu, text) : NULL;
	item->subtext = (subtext != NULL) ? ui_menu_pool_strdup(menu, subtext) : NULL;
	item->flags = flags;
	item->ref = ref;

	/* update the selection if we need to */
	if (menu->resetpos == index || (menu->resetref != NULL && menu->resetref == ref))
		menu->selected = index;
	if (menu->resetpos == menu->numitems - 1)
		menu->selected = menu->numitems - 1;
}


/*-------------------------------------------------
    ui_menu_process - process a menu, drawing it
    and returning any interesting events
-------------------------------------------------*/

const ui_menu_event *ui_menu_process(running_machine *machine, ui_menu *menu, UINT32 flags)
{
	/* reset the event */
	menu->event.iptkey = IPT_INVALID;

	/* first make sure our selection is valid */
	ui_menu_validate_selection(menu, 1);

	/* draw the menu */
	if (menu->numitems > 1 && (menu->item[0].flags & MENU_FLAG_MULTILINE) != 0)
		ui_menu_draw_text_box(menu);
	else
		ui_menu_draw(machine, menu, (flags & UI_MENU_PROCESS_CUSTOM_ONLY) != 0);

	/* process input */
	if (!(flags & UI_MENU_PROCESS_NOKEYS))
	{
		/* read events */
		ui_menu_handle_events(menu);

		/* handle the keys if we don't already have an event */
		if (menu->event.iptkey == IPT_INVALID)
			ui_menu_handle_keys(menu, flags);
	}

	/* update the selected item in the event */
	if (menu->event.iptkey != IPT_INVALID && menu->selected >= 0 && menu->selected < menu->numitems)
	{
		menu->event.itemref = menu->item[menu->selected].ref;
		return &menu->event;
	}
	return NULL;
}


/*-------------------------------------------------
    ui_menu_set_custom_render - configure the menu
    for custom rendering
-------------------------------------------------*/

void ui_menu_set_custom_render(ui_menu *menu, ui_menu_custom_func custom, float top, float bottom)
{
	menu->custom = custom;
	menu->customtop = top;
	menu->custombottom = bottom;
}


/*-------------------------------------------------
    ui_menu_alloc_state - allocate permanent
    memory to represent the menu's state
-------------------------------------------------*/

void *ui_menu_alloc_state(ui_menu *menu, size_t size, ui_menu_destroy_state_func destroy_state)
{
	if (menu->state != NULL)
	{
		if (menu->destroy_state != NULL)
			(*menu->destroy_state)(menu, menu->state);
		free(menu->state);
	}
	menu->state = alloc_array_clear_or_die(UINT8, size);
	menu->destroy_state = destroy_state;

	return menu->state;
}


/*-------------------------------------------------
    ui_menu_pool_alloc - allocate temporary memory
    from the menu's memory pool
-------------------------------------------------*/

void *ui_menu_pool_alloc(ui_menu *menu, size_t size)
{
	ui_menu_pool *pool;

	assert(size < UI_MENU_POOL_SIZE);

	/* find a pool with enough room */
	for (pool = menu->pool; pool != NULL; pool = pool->next)
		if (pool->end - pool->top >= size)
		{
			void *result = pool->top;
			pool->top += size;
			return result;
		}

	/* allocate a new pool */
	pool = (ui_menu_pool *)alloc_array_clear_or_die(UINT8, sizeof(*pool) + UI_MENU_POOL_SIZE);

	/* wire it up */
	pool->next = menu->pool;
	menu->pool = pool;
	pool->top = (UINT8 *)(pool + 1);
	pool->end = pool->top + UI_MENU_POOL_SIZE;
	return ui_menu_pool_alloc(menu, size);
}


/*-------------------------------------------------
    ui_menu_pool_strdup - make a temporary string
    copy in the menu's memory pool
-------------------------------------------------*/

const char *ui_menu_pool_strdup(ui_menu *menu, const char *string)
{
	return strcpy((char *)ui_menu_pool_alloc(menu, strlen(string) + 1), string);
}


/*-------------------------------------------------
    ui_menu_get_selection - retrieves the index
    of the currently selected menu item
-------------------------------------------------*/

void *ui_menu_get_selection(ui_menu *menu)
{
	return (menu->selected >= 0 && menu->selected < menu->numitems) ? menu->item[menu->selected].ref : NULL;
}


/*-------------------------------------------------
    ui_menu_set_selection - changes the index
    of the currently selected menu item
-------------------------------------------------*/

void ui_menu_set_selection(ui_menu *menu, void *selected_itemref)
{
	int itemnum;

	menu->selected = -1;
	for (itemnum = 0; itemnum < menu->numitems; itemnum++)
		if (menu->item[itemnum].ref == selected_itemref)
		{
			menu->selected = itemnum;
			break;
		}
}



/***************************************************************************
    INTERNAL MENU PROCESSING
***************************************************************************/

/*-------------------------------------------------
    ui_menu_draw - draw a menu
-------------------------------------------------*/

static void ui_menu_draw(running_machine *machine, ui_menu *menu, int customonly)
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
	int selected_subitem_too_big = FALSE;
	int visible_lines;
	int top_line;
	int itemnum, linenum;
	int mouse_hit, mouse_button;
	render_target *mouse_target;
	INT32 mouse_target_x, mouse_target_y;
	float mouse_x = -1, mouse_y = -1;

	/* compute the width and height of the full menu */
	visible_width = 0;
	visible_main_menu_height = 0;
	for (itemnum = 0; itemnum < menu->numitems; itemnum++)
	{
		const ui_menu_item *item = &menu->item[itemnum];
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

	/* account for extra space at the top and bottom */
	visible_extra_menu_height = menu->customtop + menu->custombottom;

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
	visible_top += menu->customtop;

	/* first add us a box */
	x1 = visible_left - UI_BOX_LR_BORDER;
	y1 = visible_top - UI_BOX_TB_BORDER;
	x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	if (!customonly)
		ui_draw_outlined_box(x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* determine the first visible line based on the current selection */
	top_line = menu->selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= menu->numitems)
		top_line = menu->numitems - visible_lines;

	/* determine effective positions taking into account the hilighting arrows */
	effective_width = visible_width - 2.0f * gutter_width;
	effective_left = visible_left + gutter_width;

	/* locate mouse */
	mouse_hit = FALSE;
	mouse_button = FALSE;
	if (!customonly)
	{
		mouse_target = ui_input_find_mouse(machine, &mouse_target_x, &mouse_target_y, &mouse_button);
		if (mouse_target != NULL)
			if (render_target_map_point_container(mouse_target, mouse_target_x, mouse_target_y, render_container_get_ui(), &mouse_x, &mouse_y))
				mouse_hit = TRUE;
	}

	/* loop over visible lines */
	menu->hover = menu->numitems + 1;
	if (!customonly)
		for (linenum = 0; linenum < visible_lines; linenum++)
		{
			float line_y = visible_top + (float)linenum * line_height;
			int itemnum = top_line + linenum;
			const ui_menu_item *item = &menu->item[itemnum];
			const char *itemtext = item->text;
			rgb_t fgcolor = UI_TEXT_COLOR;
			rgb_t bgcolor = UI_TEXT_BG_COLOR;
			rgb_t fgcolor2 = UI_SUBITEM_COLOR;
			rgb_t fgcolor3 = UI_CLONE_COLOR;
			float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
			float line_y0 = line_y;
			float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
			float line_y1 = line_y + line_height;

			/* set the hover if this is our item */
			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && item_is_selectable(item))
				menu->hover = itemnum;

			/* if we're selected, draw with a different background */
			if (itemnum == menu->selected)
			{
				fgcolor = UI_SELECTED_COLOR;
				bgcolor = UI_SELECTED_BG_COLOR;
				fgcolor2 = UI_SELECTED_COLOR;
				fgcolor3 = UI_SELECTED_COLOR;
			}

			/* else if the mouse is over this item, draw with a different background */
			else if (itemnum == menu->hover)
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor2 = UI_MOUSEOVER_COLOR;
				fgcolor3 = UI_MOUSEOVER_COLOR;
			}

			/* if we have some background hilighting to do, add a quad behind everything else */
			if (bgcolor != UI_TEXT_BG_COLOR)
				render_ui_add_quad(line_x0, line_y0, line_x1, line_y1, bgcolor, hilight_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

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
				if (menu->hover == itemnum)
					menu->hover = -2;
			}

			/* if we're on the bottom line, display the down arrow */
			else if (linenum == visible_lines - 1 && itemnum != menu->numitems - 1)
			{
				render_ui_add_quad(	0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
									line_y + 0.25f * line_height,
									0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
									line_y + 0.75f * line_height,
									fgcolor,
									arrow_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT0 ^ ORIENTATION_FLIP_Y));
				if (menu->hover == itemnum)
					menu->hover = -1;
			}

			/* if we're just a divider, draw a line */
			else if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
				render_ui_add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

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
					if (itemnum == menu->selected)
						selected_subitem_too_big = TRUE;
				}

				/* draw the subitem right-justified */
				ui_draw_text_full(subitem_text, effective_left + item_width, line_y, effective_width - item_width,
							JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor, &subitem_width, NULL);

				/* apply arrows */
				if (itemnum == menu->selected && (item->flags & MENU_FLAG_LEFT_ARROW))
				{
					render_ui_add_quad(	effective_left + effective_width - subitem_width - gutter_width,
										line_y + 0.1f * line_height,
										effective_left + effective_width - subitem_width - gutter_width + lr_arrow_width,
										line_y + 0.9f * line_height,
										fgcolor,
										arrow_texture,
										PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90 ^ ORIENTATION_FLIP_X));
				}
				if (itemnum == menu->selected && (item->flags & MENU_FLAG_RIGHT_ARROW))
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
		const ui_menu_item *item = &menu->item[menu->selected];
		int subitem_invert = item->flags & MENU_FLAG_INVERT;
		int linenum = menu->selected - top_line;
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
						 target_y + target_height + UI_BOX_TB_BORDER, subitem_invert ? UI_SELECTED_BG_COLOR : UI_BACKGROUND_COLOR);
		ui_draw_text_full(item->subtext, target_x, target_y, target_width,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);
	}

	/* if there is something special to add, do it by calling the passed routine */
	if (menu->custom != NULL)
	{
		void *selectedref = (menu->selected >= 0 && menu->selected < menu->numitems) ? menu->item[menu->selected].ref : NULL;
		(*menu->custom)(menu->machine, menu, menu->state, selectedref, menu->customtop, menu->custombottom, x1, y1, x2, y2);
	}

	/* return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow */
	menu->visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != menu->numitems);
}


/*-------------------------------------------------
    ui_menu_draw_text_box - draw a multiline
    word-wrapped text box with a menu item at the
    bottom
-------------------------------------------------*/

static void ui_menu_draw_text_box(ui_menu *menu)
{
	const char *text = menu->item[0].text;
	const char *backtext = menu->item[1].text;
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
	prior_width = ui_get_string_width(backtext) + 2.0f * gutter_width;
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
					 target_y + target_height + UI_BOX_TB_BORDER, (menu->item[0].flags & MENU_FLAG_REDTEXT) ?  UI_RED_COLOR : UI_BACKGROUND_COLOR);
	ui_draw_text_full(text, target_x, target_y, target_width,
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	/* draw the "return to prior menu" text with a hilight behind it */
	render_ui_add_quad(	target_x + 0.5f * UI_LINE_WIDTH,
						target_y + target_height - line_height,
						target_x + target_width - 0.5f * UI_LINE_WIDTH,
						target_y + target_height,
						UI_SELECTED_BG_COLOR,
						hilight_texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
	ui_draw_text_full(backtext, target_x, target_y + target_height - line_height, target_width,
				JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);

	/* artificially set the hover to the last item so a double-click exits */
	menu->hover = menu->numitems - 1;
}


/*-------------------------------------------------
    ui_menu_handle_events - generically handle
    input events for a menu
-------------------------------------------------*/

static void ui_menu_handle_events(ui_menu *menu)
{
	int stop = FALSE;
	ui_event event;

	/* loop while we have interesting events */
	while (ui_input_pop_event(menu->machine, &event) && !stop)
	{
		switch (event.event_type)
		{
			/* if we are hovering over a valid item, select it with a single click */
			case UI_EVENT_MOUSE_DOWN:
				if (menu->hover >= 0 && menu->hover < menu->numitems)
					menu->selected = menu->hover;
				else if (menu->hover == -2)
				{
					menu->selected -= menu->visitems - 1;
					ui_menu_validate_selection(menu, 1);
				}
				else if (menu->hover == -1)
				{
					menu->selected += menu->visitems - 1;
					ui_menu_validate_selection(menu, 1);
				}
				break;

			/* if we are hovering over a valid item, fake a UI_SELECT with a double-click */
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if (menu->hover >= 0 && menu->hover < menu->numitems)
				{
					menu->selected = menu->hover;
					if (event.event_type == UI_EVENT_MOUSE_DOUBLE_CLICK)
					{
						menu->event.iptkey = IPT_UI_SELECT;
						if (menu->selected == menu->numitems - 1)
						{
							menu->event.iptkey = IPT_UI_CANCEL;
							ui_menu_stack_pop(menu->machine);
						}
					}
					stop = TRUE;
				}
				break;

			/* translate CHAR events into specials */
			case UI_EVENT_CHAR:
				menu->event.iptkey = IPT_SPECIAL;
				menu->event.unichar = event.ch;
				stop = TRUE;
				break;

			/* ignore everything else */
			default:
				break;
		}
	}
}


/*-------------------------------------------------
    ui_menu_handle_keys - generically handle
    keys for a menu
-------------------------------------------------*/

static void ui_menu_handle_keys(ui_menu *menu, UINT32 flags)
{
	int ignorepause = ui_menu_is_force_game_select();
	int ignoreright = FALSE;
	int ignoreleft = FALSE;
	int code;

	/* bail if no items */
	if (menu->numitems == 0)
		return;

	/* if we hit select, return TRUE or pop the stack, depending on the item */
	if (exclusive_input_pressed(menu, IPT_UI_SELECT, 0))
	{
		if (menu->selected == menu->numitems - 1)
		{
			menu->event.iptkey = IPT_UI_CANCEL;
			ui_menu_stack_pop(menu->machine);
		}
		return;
	}

	/* hitting cancel also pops the stack */
	if (exclusive_input_pressed(menu, IPT_UI_CANCEL, 0))
	{
		ui_menu_stack_pop(menu->machine);
		return;
	}

	/* validate the current selection */
	ui_menu_validate_selection(menu, 1);

	/* swallow left/right keys if they are not appropriate */
	ignoreleft = ((menu->item[menu->selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	ignoreright = ((menu->item[menu->selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);

	/* accept left/right keys as-is with repeat */
	if (!ignoreleft && exclusive_input_pressed(menu, IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(menu, IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	/* up backs up by one item */
	if (exclusive_input_pressed(menu, IPT_UI_UP, 6))
	{
		menu->selected = (menu->selected + menu->numitems - 1) % menu->numitems;
		ui_menu_validate_selection(menu, -1);
	}

	/* down advances by one item */
	if (exclusive_input_pressed(menu, IPT_UI_DOWN, 6))
	{
		menu->selected = (menu->selected + 1) % menu->numitems;
		ui_menu_validate_selection(menu, 1);
	}

	/* page up backs up by visitems */
	if (exclusive_input_pressed(menu, IPT_UI_PAGE_UP, 6))
	{
		menu->selected -= menu->visitems - 1;
		ui_menu_validate_selection(menu, 1);
	}

	/* page down advances by visitems */
	if (exclusive_input_pressed(menu, IPT_UI_PAGE_DOWN, 6))
	{
		menu->selected += menu->visitems - 1;
		ui_menu_validate_selection(menu, -1);
	}

	/* home goes to the start */
	if (exclusive_input_pressed(menu, IPT_UI_HOME, 0))
	{
		menu->selected = 0;
		ui_menu_validate_selection(menu, 1);
	}

	/* end goes to the last */
	if (exclusive_input_pressed(menu, IPT_UI_END, 0))
	{
		menu->selected = menu->numitems - 1;
		ui_menu_validate_selection(menu, -1);
	}

	/* pause enables/disables pause */
	if (!ignorepause && exclusive_input_pressed(menu, IPT_UI_PAUSE, 0))
		mame_pause(menu->machine, !mame_is_paused(menu->machine));

	/* handle a toggle cheats request */
	if (ui_input_pressed_repeat(menu->machine, IPT_UI_TOGGLE_CHEAT, 0))
		cheat_set_global_enable(menu->machine, !cheat_get_global_enable(menu->machine));

	/* see if any other UI keys are pressed */
	if (menu->event.iptkey == IPT_INVALID)
		for (code = __ipt_ui_start; code <= __ipt_ui_end; code++)
		{
			if (code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft) || (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;
			if (exclusive_input_pressed(menu, code, 0))
				break;
		}
}


/*-------------------------------------------------
    ui_menu_validate_selection - validate the
    current selection and ensure it is on a
    correct item
-------------------------------------------------*/

static void ui_menu_validate_selection(ui_menu *menu, int scandir)
{
	/* clamp to be in range */
	if (menu->selected < 0)
		menu->selected = 0;
	else if (menu->selected >= menu->numitems)
		menu->selected = menu->numitems - 1;

	/* skip past unselectable items */
	while (!item_is_selectable(&menu->item[menu->selected]))
		menu->selected = (menu->selected + menu->numitems + scandir) % menu->numitems;
}



/*-------------------------------------------------
    ui_menu_clear_free_list - clear out anything
    accumulated in the free list
-------------------------------------------------*/

static void ui_menu_clear_free_list(running_machine *machine)
{
	while (menu_free != NULL)
	{
		ui_menu *menu = menu_free;
		menu_free = menu->parent;
		ui_menu_free(menu);
	}
}



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    ui_menu_stack_reset - reset the menu stack
-------------------------------------------------*/

void ui_menu_stack_reset(running_machine *machine)
{
	while (menu_stack != NULL)
		ui_menu_stack_pop(machine);
}


/*-------------------------------------------------
    ui_menu_stack_push - push a new menu onto the
    stack
-------------------------------------------------*/

void ui_menu_stack_push(ui_menu *menu)
{
	menu->parent = menu_stack;
	menu_stack = menu;
	ui_menu_reset(menu, UI_MENU_RESET_SELECT_FIRST);
	ui_input_reset(menu->machine);
}


/*-------------------------------------------------
    ui_menu_stack_pop - pop a menu from the stack
-------------------------------------------------*/

void ui_menu_stack_pop(running_machine *machine)
{
	if (menu_stack != NULL)
	{
		ui_menu *menu = menu_stack;
		menu_stack = menu->parent;
		menu->parent = menu_free;
		menu_free = menu;
		ui_input_reset(machine);
	}
}



/***************************************************************************
    UI SYSTEM INTERACTION
***************************************************************************/

/*-------------------------------------------------
    ui_menu_ui_handler - displays the current menu
    and calls the menu handler
-------------------------------------------------*/

UINT32 ui_menu_ui_handler(running_machine *machine, UINT32 state)
{
	/* if we have no menus stacked up, start with the main menu */
	if (menu_stack == NULL)
		ui_menu_stack_push(ui_menu_alloc(machine, menu_main, NULL));

	/* update the menu state */
	if (menu_stack != NULL)
	{
		ui_menu *menu = menu_stack;
		(*menu->handler)(machine, menu, menu->parameter, menu->state);
	}

	/* clear up anything pending to be released */
	ui_menu_clear_free_list(machine);

	/* if the menus are to be hidden, return a cancel here */
	if ((ui_input_pressed(machine, IPT_UI_CONFIGURE) && !ui_menu_is_force_game_select()) || menu_stack == NULL)
		return UI_HANDLER_CANCEL;

	return 0;
}


/*-------------------------------------------------
    ui_slider_ui_handler - pushes the slider
    menu on the stack and hands off to the
    standard menu handler
-------------------------------------------------*/

UINT32 ui_slider_ui_handler(running_machine *machine, UINT32 state)
{
	UINT32 result;

	/* if this is the first call, push the sliders menu */
	if (state)
		ui_menu_stack_push(ui_menu_alloc(machine, menu_sliders, (void *)1));

	/* handle standard menus */
	result = ui_menu_ui_handler(machine, state);

	/* if we are cancelled, pop the sliders menu */
	if (result == UI_HANDLER_CANCEL)
		ui_menu_stack_pop(machine);

	return (menu_stack != NULL && menu_stack->handler == menu_sliders && menu_stack->parameter != NULL) ? 0 : UI_HANDLER_CANCEL;
}


/*-------------------------------------------------
    ui_menu_force_game_select - force the game
    select menu to be visible and inescapable
-------------------------------------------------*/

void ui_menu_force_game_select(running_machine *machine)
{
	char *gamename = (char *)options_get_string(mame_options(), OPTION_GAMENAME);

	/* reset the menu stack */
	ui_menu_stack_reset(machine);

	/* add the quit entry followed by the game select entry */
	ui_menu_stack_push(ui_menu_alloc(machine, menu_quit_game, NULL));
	ui_menu_stack_push(ui_menu_alloc(machine, menu_select_game, gamename));

	/* force the menus on */
	ui_show_menu();

	/* make sure MAME is paused */
	mame_pause(machine, TRUE);
}


/*-------------------------------------------------
    ui_menu_is_force_game_select - return true
    if we are currently in "force game select"
    mode
-------------------------------------------------*/

int ui_menu_is_force_game_select(void)
{
	ui_menu *menu;

	for (menu = menu_stack; menu != NULL; menu = menu->parent)
		if (menu->handler == menu_quit_game && menu->parent == NULL)
			return TRUE;

	return FALSE;
}



/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

static void menu_main(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_main_populate(machine, menu, state);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
		ui_menu_stack_push(ui_menu_alloc(machine, (ui_menu_handler_func)event->itemref, NULL));
}


/*-------------------------------------------------
    menu_main_populate - populate the main menu
-------------------------------------------------*/

static void menu_main_populate(running_machine *machine, ui_menu *menu, void *state)
{
	const input_field_config *field;
	const input_port_config *port;
	int has_categories = FALSE;
	int has_configs = FALSE;
	int has_analog = FALSE;
	int has_dips = FALSE;

	/* scan the input port array to see what options we need to enable */
	for (port = machine->portlist.head; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
		{
			if (field->type == IPT_DIPSWITCH)
				has_dips = TRUE;
			if (field->type == IPT_CONFIG)
				has_configs = TRUE;
			if (field->category > 0)
				has_categories = TRUE;
			if (input_type_is_analog(field->type))
				has_analog = TRUE;
		}

	/* add input menu items */
	ui_menu_item_append(menu, "Input (general)", NULL, 0, (void *)menu_input_groups);
	ui_menu_item_append(menu, "Input (this " CAPSTARTGAMENOUN ")", NULL, 0, (void *)menu_input_specific);

	/* add optional input-related menus */
	if (has_dips)
		ui_menu_item_append(menu, "Dip Switches", NULL, 0, (void *)menu_settings_dip_switches);
	if (has_configs)
		ui_menu_item_append(menu, "Driver Configuration", NULL, 0, (void *)menu_settings_driver_config);
	if (has_categories)
		ui_menu_item_append(menu, "Categories", NULL, 0, (void *)menu_settings_categories);
	if (has_analog)
		ui_menu_item_append(menu, "Analog Controls", NULL, 0, (void *)menu_analog);

#ifndef MESS
  	/* add bookkeeping menu */
	ui_menu_item_append(menu, "Bookkeeping Info", NULL, 0, (void *)menu_bookkeeping);
#endif

	/* add game info menu */
	ui_menu_item_append(menu, CAPSTARTGAMENOUN " Information", NULL, 0, (void *)menu_game_info);

#ifdef MESS
	/* add MESS-specific menus */
	ui_mess_main_menu_populate(machine, menu);
#endif /* MESS */

	/* add sliders menu */
	ui_menu_item_append(menu, "Slider Controls", NULL, 0, (void *)menu_sliders);

	/* add video options menu */
	ui_menu_item_append(menu, "Video Options", NULL, 0, (render_target_get_indexed(1) != NULL) ? (void *)menu_video_targets : (void *)menu_video_options);

	/* add crosshair options menu */
	if (crosshair_get_usage(machine))
		ui_menu_item_append(menu, "Crosshair Options", NULL, 0, (void *)menu_crosshair);

	/* add cheat menu */
	if (options_get_bool(mame_options(), OPTION_CHEAT) && cheat_get_next_menu_entry(machine, NULL, NULL, NULL, NULL) != NULL)
		ui_menu_item_append(menu, "Cheat", NULL, 0, (void *)menu_cheat);

	/* add memory card menu */
	if (machine->config->memcard_handler != NULL)
		ui_menu_item_append(menu, "Memory Card", NULL, 0, (void *)menu_memory_card);

	/* add reset and exit menus */
	ui_menu_item_append(menu, "Select New " CAPSTARTGAMENOUN, NULL, 0, (void *)menu_select_game);
}


/*-------------------------------------------------
    menu_input_groups - handle the input groups
    menu
-------------------------------------------------*/

static void menu_input_groups(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_input_groups_populate(machine, menu, state);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
		ui_menu_stack_push(ui_menu_alloc(machine, menu_input_general, event->itemref));
}


/*-------------------------------------------------
    menu_input_groups_populate - populate the
    input groups menu
-------------------------------------------------*/

static void menu_input_groups_populate(running_machine *machine, ui_menu *menu, void *state)
{
	int player;

	/* build up the menu */
	ui_menu_item_append(menu, "User Interface", NULL, 0, (void *)(IPG_UI + 1));
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		char buffer[40];
		sprintf(buffer, "Player %d Controls", player + 1);
		ui_menu_item_append(menu, buffer, NULL, 0, (void *)(FPTR)(IPG_PLAYER1 + player + 1));
	}
	ui_menu_item_append(menu, "Other Controls", NULL, 0, (void *)(FPTR)(IPG_OTHER + 1));
}



/*-------------------------------------------------
    menu_input_general - handle the general
    input menu
-------------------------------------------------*/

static void menu_input_general(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	menu_input_common(machine, menu, parameter, state);
}


/*-------------------------------------------------
    menu_input_general_populate - populate the
    general input menu
-------------------------------------------------*/

static void menu_input_general_populate(running_machine *machine, ui_menu *menu, input_menu_state *menustate, int group)
{
	astring *tempstring = astring_alloc();
	input_item_data *itemlist = NULL;
	const input_type_desc *typedesc;
	int suborder[SEQ_TYPE_TOTAL];
	int sortorder = 1;

	/* create a mini lookup table for sort order based on sequence type */
	suborder[SEQ_TYPE_STANDARD] = 0;
	suborder[SEQ_TYPE_DECREMENT] = 1;
	suborder[SEQ_TYPE_INCREMENT] = 2;

	/* iterate over the input ports and add menu items */
	for (typedesc = input_type_list(machine); typedesc != NULL; typedesc = typedesc->next)

		/* add if we match the group and we have a valid name */
		if (typedesc->group == group && typedesc->name != NULL && typedesc->name[0] != 0)
		{
			input_seq_type seqtype;

			/* loop over all sequence types */
			sortorder++;
			for (seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; seqtype++)
			{
				/* build an entry for the standard sequence */
				input_item_data *item = (input_item_data *)ui_menu_pool_alloc(menu, sizeof(*item));
				memset(item, 0, sizeof(*item));
				item->ref = typedesc;
				item->seqtype = seqtype;
				item->seq = *input_type_seq(machine, typedesc->type, typedesc->player, seqtype);
				item->defseq = &typedesc->seq[seqtype];
				item->sortorder = sortorder * 4 + suborder[seqtype];
				item->type = input_type_is_analog(typedesc->type) ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
				item->name = typedesc->name;
				item->next = itemlist;
				itemlist = item;

				/* stop after one, unless we're analog */
				if (item->type == INPUT_TYPE_DIGITAL)
					break;
			}
		}

	/* sort and populate the menu in a standard fashion */
	menu_input_populate_and_sort(machine, menu, itemlist, menustate);
	astring_free(tempstring);
}


/*-------------------------------------------------
    menu_input_specific - handle the game-specific
    input menu
-------------------------------------------------*/

static void menu_input_specific(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	menu_input_common(machine, menu, NULL, state);
}


/*-------------------------------------------------
    menu_input_specific_populate - populate
    the input menu with game-specific items
-------------------------------------------------*/

static void menu_input_specific_populate(running_machine *machine, ui_menu *menu, input_menu_state *menustate)
{
	astring *tempstring = astring_alloc();
	input_item_data *itemlist = NULL;
	const input_field_config *field;
	const input_port_config *port;
	int suborder[SEQ_TYPE_TOTAL];

	/* create a mini lookup table for sort order based on sequence type */
	suborder[SEQ_TYPE_STANDARD] = 0;
	suborder[SEQ_TYPE_DECREMENT] = 1;
	suborder[SEQ_TYPE_INCREMENT] = 2;

	/* iterate over the input ports and add menu items */
	for (port = machine->portlist.head; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
		{
			const char *name = input_field_name(field);

			/* add if we match the group and we have a valid name */
			if (name != NULL && input_condition_true(machine, &field->condition) &&
#ifdef MESS
				(field->category == 0 || input_category_active(machine, field->category)) &&
#endif /* MESS */
				((field->type == IPT_OTHER && field->name != NULL) || input_type_group(machine, field->type, field->player) != IPG_INVALID))
			{
				input_seq_type seqtype;
				UINT16 sortorder;

				/* determine the sorting order */
				if (field->type >= IPT_START1 && field->type <= __ipt_analog_end)
					sortorder = (field->type << 2) | (field->player << 12);
				else
					sortorder = field->type | 0xf000;

				/* loop over all sequence types */
				for (seqtype = SEQ_TYPE_STANDARD; seqtype < SEQ_TYPE_TOTAL; seqtype++)
				{
					/* build an entry for the standard sequence */
					input_item_data *item = (input_item_data *)ui_menu_pool_alloc(menu, sizeof(*item));
					memset(item, 0, sizeof(*item));
					item->ref = field;
					item->seqtype = seqtype;
					item->seq = *input_field_seq(field, seqtype);
					item->defseq = get_field_default_seq(field, seqtype);
					item->sortorder = sortorder + suborder[seqtype];
					item->type = input_type_is_analog(field->type) ? (INPUT_TYPE_ANALOG + seqtype) : INPUT_TYPE_DIGITAL;
					item->name = name;
					item->next = itemlist;
					itemlist = item;

					/* stop after one, unless we're analog */
					if (item->type == INPUT_TYPE_DIGITAL)
						break;
				}
			}
		}

	/* sort and populate the menu in a standard fashion */
	menu_input_populate_and_sort(machine, menu, itemlist, menustate);
	astring_free(tempstring);
}


/*-------------------------------------------------
    menu_input - display a menu for inputs
-------------------------------------------------*/

static void menu_input_common(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	input_item_data *seqchangeditem = NULL;
	input_menu_state *menustate;
	const ui_menu_event *event;
	int invalidate = FALSE;

	/* if no state, allocate now */
	if (state == NULL)
		state = ui_menu_alloc_state(menu, sizeof(*menustate), NULL);
	menustate = (input_menu_state *)state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
	{
		if (parameter != NULL)
			menu_input_general_populate(machine, menu, menustate, (FPTR)parameter - 1);
		else
			menu_input_specific_populate(machine, menu, menustate);
	}

	/* process the menu */
	event = ui_menu_process(machine, menu, (menustate->pollingitem != NULL) ? UI_MENU_PROCESS_NOKEYS : 0);

	/* if we are polling, handle as a special case */
	if (menustate->pollingitem != NULL)
	{
		input_item_data *item = menustate->pollingitem;
		input_seq newseq;

		/* if UI_CANCEL is pressed, abort */
		if (ui_input_pressed(machine, IPT_UI_CANCEL))
		{
			menustate->pollingitem = NULL;
			menustate->record_next = FALSE;
			toggle_none_default(&item->seq, &menustate->starting_seq, item->defseq);
			seqchangeditem = item;
		}

		/* poll again; if finished, update the sequence */
		if (input_seq_poll(machine, &newseq))
		{
			menustate->pollingitem = NULL;
			menustate->record_next = TRUE;
			item->seq = newseq;
			seqchangeditem = item;
		}
	}

	/* otherwise, handle the events */
	else if (event != NULL && event->itemref != NULL)
	{
		input_item_data *item = (input_item_data *)event->itemref;
		switch (event->iptkey)
		{
			/* an item was selected: begin polling */
			case IPT_UI_SELECT:
				menustate->pollingitem = item;
				menustate->last_sortorder = item->sortorder;
				menustate->starting_seq = item->seq;
				input_seq_poll_start(machine, (item->type == INPUT_TYPE_ANALOG) ? ITEM_CLASS_ABSOLUTE : ITEM_CLASS_SWITCH, menustate->record_next ? &item->seq : NULL);
				invalidate = TRUE;
				break;

			/* if the clear key was pressed, reset the selected item */
			case IPT_UI_CLEAR:
				toggle_none_default(&item->seq, &item->seq, item->defseq);
				menustate->record_next = FALSE;
				seqchangeditem = item;
				break;
		}

		/* if the selection changed, reset the "record next" flag */
		if (item->sortorder != menustate->last_sortorder)
			menustate->record_next = FALSE;
		menustate->last_sortorder = item->sortorder;
	}

	/* if the sequence changed, update it */
	if (seqchangeditem != NULL)
	{
		/* update a general input */
		if (parameter != NULL)
		{
			const input_type_desc *typedesc = (const input_type_desc *)seqchangeditem->ref;
			input_type_set_seq(machine, typedesc->type, typedesc->player, seqchangeditem->seqtype, &seqchangeditem->seq);
		}

		/* update a game-specific input */
		else
		{
			input_field_user_settings settings;

			input_field_get_user_settings((const input_field_config *)seqchangeditem->ref, &settings);
			settings.seq[seqchangeditem->seqtype] = seqchangeditem->seq;
			input_field_set_user_settings((const input_field_config *)seqchangeditem->ref, &settings);
		}

		/* invalidate the menu to force an update */
		invalidate = TRUE;
	}

	/* if the menu is invalidated, clear it now */
	if (invalidate)
	{
		menustate->pollingref = NULL;
		if (menustate->pollingitem != NULL)
			menustate->pollingref = menustate->pollingitem->ref;
		ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_POSITION);
	}
}


/*-------------------------------------------------
    menu_input_compare_items - compare two
    items for quicksort
-------------------------------------------------*/

static int menu_input_compare_items(const void *i1, const void *i2)
{
	const input_item_data * const *data1 = (const input_item_data * const *)i1;
	const input_item_data * const *data2 = (const input_item_data * const *)i2;
	if ((*data1)->sortorder < (*data2)->sortorder)
		return -1;
	if ((*data1)->sortorder > (*data2)->sortorder)
		return 1;
	return 0;
}


/*-------------------------------------------------
    menu_input_populate_and_sort - take a list
    of input_item_data objects and build up the
    menu from them
-------------------------------------------------*/

static void menu_input_populate_and_sort(running_machine *machine, ui_menu *menu, input_item_data *itemlist, input_menu_state *menustate)
{
	const char *nameformat[INPUT_TYPE_TOTAL] = { 0 };
	input_item_data **itemarray, *item;
	astring *subtext = astring_alloc();
	astring *text = astring_alloc();
	int numitems = 0, curitem;

	/* create a mini lookup table for name format based on type */
	nameformat[INPUT_TYPE_DIGITAL] = "%s";
	nameformat[INPUT_TYPE_ANALOG] = "%s Analog";
	nameformat[INPUT_TYPE_ANALOG_INC] = "%s Analog Inc";
	nameformat[INPUT_TYPE_ANALOG_DEC] = "%s Analog Dec";

	/* first count the number of items */
	for (item = itemlist; item != NULL; item = item->next)
		numitems++;

	/* now allocate an array of items and fill it up */
	itemarray = (input_item_data **)ui_menu_pool_alloc(menu, sizeof(*itemarray) * numitems);
	for (item = itemlist, curitem = 0; item != NULL; item = item->next)
		itemarray[curitem++] = item;

	/* sort it */
	qsort(itemarray, numitems, sizeof(*itemarray), menu_input_compare_items);

	/* build the menu */
	for (curitem = 0; curitem < numitems; curitem++)
	{
		UINT32 flags = 0;

		/* generate the name of the item itself, based off the base name and the type */
		item = itemarray[curitem];
		assert(nameformat[item->type] != NULL);
		astring_printf(text, nameformat[item->type], item->name);

		/* if we're polling this item, use some spaces with left/right arrows */
		if (menustate->pollingref == item->ref)
		{
			astring_cpyc(subtext, "   ");
			flags |= MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW;
		}

		/* otherwise, generate the sequence name and invert it if different from the default */
		else
		{
			input_seq_name(machine, subtext, &item->seq);
			flags |= input_seq_cmp(&item->seq, item->defseq) ? MENU_FLAG_INVERT : 0;
		}

		/* add the item */
		ui_menu_item_append(menu, astring_c(text), astring_c(subtext), flags, item);
	}

	/* free our temporary strings */
	astring_free(subtext);
	astring_free(text);
}


/*-------------------------------------------------
    menu_settings_dip_switches - handle the DIP
    switches menu
-------------------------------------------------*/

static void menu_settings_dip_switches(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	menu_settings_common(machine, menu, state, IPT_DIPSWITCH);
}


/*-------------------------------------------------
    menu_settings_driver_config - handle the
    driver config menu
-------------------------------------------------*/

static void menu_settings_driver_config(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	menu_settings_common(machine, menu, state, IPT_CONFIG);
}


/*-------------------------------------------------
    menu_settings_categories - handle the
    categories menu
-------------------------------------------------*/

static void menu_settings_categories(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	menu_settings_common(machine, menu, state, IPT_CATEGORY);
}


/*-------------------------------------------------
    menu_settings_common - handle one of the
    switches menus
-------------------------------------------------*/

static void menu_settings_common(running_machine *machine, ui_menu *menu, void *state, UINT32 type)
{
	settings_menu_state *menustate;
	const ui_menu_event *event;

	/* if no state, allocate now */
	if (state == NULL)
		state = ui_menu_alloc_state(menu, sizeof(*menustate), NULL);
	menustate = (settings_menu_state *)state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_settings_populate(machine, menu, menustate, type);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);

	/* handle events */
	if (event != NULL && event->itemref != NULL)
	{
		const input_field_config *field = (const input_field_config *)event->itemref;
		input_field_user_settings settings;
		int changed = FALSE;

		switch (event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				input_field_get_user_settings(field, &settings);
				settings.value = field->defvalue;
				input_field_set_user_settings(field, &settings);
				changed = TRUE;
				break;

			/* left goes to previous setting */
			case IPT_UI_LEFT:
				input_field_select_previous_setting(field);
				changed = TRUE;
				break;

			/* right goes to next setting */
			case IPT_UI_RIGHT:
				input_field_select_next_setting(field);
				changed = TRUE;
				break;
		}

		/* if anything changed, rebuild the menu, trying to stay on the same field */
		if (changed)
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_settings_populate - populate one of the
    switches menus
-------------------------------------------------*/

static void menu_settings_populate(running_machine *machine, ui_menu *menu, settings_menu_state *menustate, UINT32 type)
{
	const input_field_config *field;
	const input_port_config *port;
	dip_descriptor **diplist_tailptr;
	int dipcount = 0;

	/* reset the dip switch tracking */
	menustate->diplist = NULL;
	diplist_tailptr = &menustate->diplist;

	/* loop over input ports and set up the current values */
	for (port = machine->portlist.head; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (field->type == type && input_condition_true(machine, &field->condition))
			{
				UINT32 flags = 0;

				/* set the left/right flags appropriately */
				if (input_field_has_previous_setting(field))
					flags |= MENU_FLAG_LEFT_ARROW;
				if (input_field_has_next_setting(field))
					flags |= MENU_FLAG_RIGHT_ARROW;

				/* add the menu item */
				ui_menu_item_append(menu, input_field_name(field), input_field_setting_name(field), flags, (void *)field);

				/* for DIP switches, build up the model */
				if (type == IPT_DIPSWITCH && field->diploclist != NULL)
				{
					const input_field_diplocation *diploc;
					input_field_user_settings settings;
					UINT32 accummask = field->mask;

					/* get current settings */
					input_field_get_user_settings(field, &settings);

					/* iterate over each bit in the field */
					for (diploc = field->diploclist; diploc != NULL; diploc = diploc->next)
					{
						UINT32 mask = accummask & ~(accummask - 1);
						dip_descriptor *dip;

						/* find the matching switch name */
						for (dip = menustate->diplist; dip != NULL; dip = dip->next)
							if (strcmp(dip->name, diploc->swname) == 0)
								break;

						/* allocate new if none */
						if (dip == NULL)
						{
							dip = (dip_descriptor *)ui_menu_pool_alloc(menu, sizeof(*dip));
							dip->next = NULL;
							dip->name = diploc->swname;
							dip->mask = dip->state = 0;
							*diplist_tailptr = dip;
							diplist_tailptr = &dip->next;
							dipcount++;
						}

						/* apply the bits */
						dip->mask |= 1 << (diploc->swnum - 1);
						if (((settings.value & mask) != 0 && !diploc->invert) || ((settings.value & mask) == 0 && diploc->invert))
							dip->state |= 1 << (diploc->swnum - 1);

						/* clear the relevant bit in the accumulated mask */
						accummask &= ~mask;
					}
				}
			}

	/* configure the extra menu */
	if (type == IPT_DIPSWITCH && menustate->diplist != NULL)
		ui_menu_set_custom_render(menu, menu_settings_custom_render, 0.0f, dipcount * (DIP_SWITCH_HEIGHT + DIP_SWITCH_SPACING) + DIP_SWITCH_SPACING);
}


/*-------------------------------------------------
    menu_settings_custom_render - perform our special
    rendering
-------------------------------------------------*/

static void menu_settings_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const input_field_config *field = (const input_field_config *)selectedref;
	settings_menu_state *menustate = (settings_menu_state *)state;
	dip_descriptor *dip;

	/* add borders */
	y1 = y2 + UI_BOX_TB_BORDER;
	y2 = y1 + bottom;

	/* draw extra menu area */
	ui_draw_outlined_box(x1, y1, x2, y2, UI_BACKGROUND_COLOR);
	y1 += (float)DIP_SWITCH_SPACING;

	/* iterate over DIP switches */
	for (dip = menustate->diplist; dip != NULL; dip = dip->next)
	{
		const input_field_diplocation *diploc;
		UINT32 selectedmask = 0;

		/* determine the mask of selected bits */
		if (field != NULL)
			for (diploc = field->diploclist; diploc != NULL; diploc = diploc->next)
				if (strcmp(dip->name, diploc->swname) == 0)
					selectedmask |= 1 << (diploc->swnum - 1);

		/* draw one switch */
		menu_settings_custom_render_one(x1, y1, x2, y1 + DIP_SWITCH_HEIGHT, dip, selectedmask);
		y1 += (float)(DIP_SWITCH_SPACING + DIP_SWITCH_HEIGHT);
	}
}


/*-------------------------------------------------
    menu_settings_custom_render_one - draw a single
    DIP switch
-------------------------------------------------*/

static void menu_settings_custom_render_one(float x1, float y1, float x2, float y2, const dip_descriptor *dip, UINT32 selectedmask)
{
	float switch_field_width = SINGLE_TOGGLE_SWITCH_FIELD_WIDTH * render_get_ui_aspect();
	float switch_width = SINGLE_TOGGLE_SWITCH_WIDTH * render_get_ui_aspect();
	int numtoggles, toggle;
	float switch_toggle_gap;
	float y1_off, y1_on;

	/* determine the number of toggles in the DIP */
	numtoggles = 32 - count_leading_zeros(dip->mask);

	/* center based on the number of switches */
 	x1 += (x2 - x1 - numtoggles * switch_field_width) / 2;

	/* draw the dip switch name */
	ui_draw_text_full(	dip->name,
						0,
						y1 + (DIP_SWITCH_HEIGHT - UI_TARGET_FONT_HEIGHT) / 2,
						x1 - ui_get_string_width(" "),
						JUSTIFY_RIGHT,
						WRAP_NEVER,
						DRAW_NORMAL,
						UI_TEXT_COLOR,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA),
						NULL ,
						NULL);

	/* compute top and bottom for on and off positions */
	switch_toggle_gap = ((DIP_SWITCH_HEIGHT/2) - SINGLE_TOGGLE_SWITCH_HEIGHT)/2;
	y1_off = y1 + UI_LINE_WIDTH + switch_toggle_gap;
	y1_on = y1 + DIP_SWITCH_HEIGHT/2 + switch_toggle_gap;

	/* iterate over toggles */
	for (toggle = 0; toggle < numtoggles; toggle++)
	{
		float innerx1;

		/* first outline the switch */
		ui_draw_outlined_box(x1, y1, x1 + switch_field_width, y2, UI_BACKGROUND_COLOR);

		/* compute x1/x2 for the inner filled in switch */
		innerx1 = x1 + (switch_field_width - switch_width) / 2;

		/* see if the switch is actually used */
		if (dip->mask & (1 << toggle))
		{
			float innery1 = (dip->state & (1 << toggle)) ? y1_on : y1_off;
			render_ui_add_rect(innerx1, innery1, innerx1 + switch_width, innery1 + SINGLE_TOGGLE_SWITCH_HEIGHT,
							   (selectedmask & (1 << toggle)) ? UI_DIPSW_COLOR : UI_TEXT_COLOR,
							   PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}
		else
		{
			render_ui_add_rect(innerx1, y1_off, innerx1 + switch_width, y1_on + SINGLE_TOGGLE_SWITCH_HEIGHT,
							   UI_UNAVAILABLE_COLOR,
							   PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		}

		/* advance to the next switch */
		x1 += switch_field_width;
	}
}


/*-------------------------------------------------
    menu_analog - handle the analog settings menu
-------------------------------------------------*/

static void menu_analog(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_analog_populate(machine, menu);

	/* process the menu */
	event = ui_menu_process(machine, menu, UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (event != NULL && event->itemref != NULL)
	{
		analog_item_data *data = (analog_item_data *)event->itemref;
		int newval = data->cur;

		switch (event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				newval = data->defvalue;
				break;

			/* left decrements */
			case IPT_UI_LEFT:
				newval -= input_code_pressed(machine, KEYCODE_LSHIFT) ? 10 : 1;
				break;

			/* right increments */
			case IPT_UI_RIGHT:
				newval += input_code_pressed(machine, KEYCODE_LSHIFT) ? 10 : 1;
				break;
		}

		/* clamp to range */
		if (newval < data->min)
			newval = data->min;
		if (newval > data->max)
			newval = data->max;

		/* if things changed, update */
		if (newval != data->cur)
		{
			input_field_user_settings settings;

			/* get the settings and set the new value */
			input_field_get_user_settings(data->field, &settings);
			switch (data->type)
			{
				case ANALOG_ITEM_KEYSPEED:		settings.delta = newval;		break;
				case ANALOG_ITEM_CENTERSPEED:	settings.centerdelta = newval;	break;
				case ANALOG_ITEM_REVERSE:		settings.reverse = newval;		break;
				case ANALOG_ITEM_SENSITIVITY:	settings.sensitivity = newval;	break;
			}
			input_field_set_user_settings(data->field, &settings);

			/* rebuild the menu */
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}


/*-------------------------------------------------
    menu_analog_populate - populate the analog
    settings menu
-------------------------------------------------*/

static void menu_analog_populate(running_machine *machine, ui_menu *menu)
{
	astring *subtext = astring_alloc();
	astring *text = astring_alloc();
	const input_field_config *field;
	const input_port_config *port;

	/* loop over input ports and add the items */
	for (port = machine->portlist.head; port != NULL; port = port->next)
		for (field = port->fieldlist; field != NULL; field = field->next)
			if (input_type_is_analog(field->type))
			{
				input_field_user_settings settings;
				int use_autocenter = FALSE;
				int type;

				/* based on the type, determine if we enable autocenter */
				switch (field->type)
				{
					case IPT_POSITIONAL:
					case IPT_POSITIONAL_V:
						if (field->flags & ANALOG_FLAG_WRAPS)
							break;

					case IPT_PEDAL:
					case IPT_PEDAL2:
					case IPT_PEDAL3:
					case IPT_PADDLE:
					case IPT_PADDLE_V:
					case IPT_AD_STICK_X:
					case IPT_AD_STICK_Y:
					case IPT_AD_STICK_Z:
						use_autocenter = TRUE;
						break;
				}

				/* get the user settings */
				input_field_get_user_settings(field, &settings);

				/* iterate over types */
				for (type = 0; type < ANALOG_ITEM_COUNT; type++)
					if (type != ANALOG_ITEM_CENTERSPEED || use_autocenter)
					{
						analog_item_data *data;
						UINT32 flags = 0;

						/* allocate a data item for tracking what this menu item refers to */
						data = (analog_item_data *)ui_menu_pool_alloc(menu, sizeof(*data));
						data->field = field;
						data->type = type;

						/* determine the properties of this item */
						switch (type)
						{
							default:
							case ANALOG_ITEM_KEYSPEED:
								astring_printf(text, "%s Digital Speed", input_field_name(field));
								astring_printf(subtext, "%d", settings.delta);
								data->min = 0;
								data->max = 255;
								data->cur = settings.delta;
								data->defvalue = field->delta;
								break;

							case ANALOG_ITEM_CENTERSPEED:
								astring_printf(text, "%s Autocenter Speed", input_field_name(field));
								astring_printf(subtext, "%d", settings.centerdelta);
								data->min = 0;
								data->max = 255;
								data->cur = settings.centerdelta;
								data->defvalue = field->centerdelta;
								break;

							case ANALOG_ITEM_REVERSE:
								astring_printf(text, "%s Reverse", input_field_name(field));
								astring_cpyc(subtext, settings.reverse ? "On" : "Off");
								data->min = 0;
								data->max = 1;
								data->cur = settings.reverse;
								data->defvalue = ((field->flags & ANALOG_FLAG_REVERSE) != 0);
								break;

							case ANALOG_ITEM_SENSITIVITY:
								astring_printf(text, "%s Sensitivity", input_field_name(field));
								astring_printf(subtext, "%d", settings.sensitivity);
								data->min = 1;
								data->max = 255;
								data->cur = settings.sensitivity;
								data->defvalue = field->sensitivity;
								break;
						}

						/* put on arrows */
						if (data->cur > data->min)
							flags |= MENU_FLAG_LEFT_ARROW;
						if (data->cur < data->max)
							flags |= MENU_FLAG_RIGHT_ARROW;

						/* append a menu item */
						ui_menu_item_append(menu, astring_c(text), astring_c(subtext), flags, data);
					}
			}

	/* release our temporary strings */
	astring_free(subtext);
	astring_free(text);
}


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/

#ifndef MESS
static void menu_bookkeeping(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	attotime *prevtime;
	attotime curtime;

	/* if no state, allocate some */
	if (state == NULL)
		state = ui_menu_alloc_state(menu, sizeof(*prevtime), NULL);
	prevtime = (attotime *)state;

	/* if the time has rolled over another second, regenerate */
	curtime = timer_get_time(machine);
	if (prevtime->seconds != curtime.seconds)
	{
		ui_menu_reset(menu, UI_MENU_RESET_SELECT_FIRST);
		*prevtime = curtime;
		menu_bookkeeping_populate(machine, menu, prevtime);
	}

	/* process the menu */
	ui_menu_process(machine, menu, 0);
}
#endif


/*-------------------------------------------------
    menu_bookkeeping - handle the bookkeeping
    information menu
-------------------------------------------------*/

#ifndef MESS
static void menu_bookkeeping_populate(running_machine *machine, ui_menu *menu, attotime *curtime)
{
	int tickets = get_dispensed_tickets(machine);
	astring *tempstring = astring_alloc();
	int ctrnum;

	/* show total time first */
	if (curtime->seconds >= 60 * 60)
		astring_catprintf(tempstring, "Uptime: %d:%02d:%02d\n\n", curtime->seconds / (60*60), (curtime->seconds / 60) % 60, curtime->seconds % 60);
	else
		astring_catprintf(tempstring, "Uptime: %d:%02d\n\n", (curtime->seconds / 60) % 60, curtime->seconds % 60);

	/* show tickets at the top */
	if (tickets > 0)
		astring_catprintf(tempstring, "Tickets dispensed: %d\n\n", tickets);

	/* loop over coin counters */
	for (ctrnum = 0; ctrnum < COIN_COUNTERS; ctrnum++)
	{
		int count = coin_counter_get_count(machine, ctrnum);

		/* display the coin counter number */
		astring_catprintf(tempstring, "Coin %c: ", ctrnum + 'A');

		/* display how many coins */
		if (count == 0)
			astring_catc(tempstring, "NA");
		else
			astring_catprintf(tempstring, "%d", count);

		/* display whether or not we are locked out */
		if (coin_lockout_get_state(machine, ctrnum))
			astring_catc(tempstring, " (locked)");
		astring_catc(tempstring, "\n");
	}

	/* append the single item */
	ui_menu_item_append(menu, astring_c(tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
	astring_free(tempstring);
}
#endif


/*-------------------------------------------------
    menu_game_info - handle the game information
    menu
-------------------------------------------------*/

static void menu_game_info(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
	{
		astring *tempstring = game_info_astring(machine, astring_alloc());
		ui_menu_item_append(menu, astring_c(tempstring), NULL, MENU_FLAG_MULTILINE, NULL);
		astring_free(tempstring);
	}

	/* process the menu */
	ui_menu_process(machine, menu, 0);
}


/*-------------------------------------------------
    menu_cheat - handle the cheat menu
-------------------------------------------------*/

static void menu_cheat(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_cheat_populate(machine, menu);

	/* process the menu */
	event = ui_menu_process(machine, menu, UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (event != NULL && event->itemref != NULL)
	{
		int changed = FALSE;

		/* clear cheat comment on any movement or keypress */
		popmessage(NULL);

		/* handle reset all + reset all cheats for reload all option */
		if ((FPTR)event->itemref < 3 && event->iptkey == IPT_UI_SELECT)
		{
			void *curcheat;
			for (curcheat = cheat_get_next_menu_entry(machine, NULL, NULL, NULL, NULL);
				 curcheat != NULL;
				 curcheat = cheat_get_next_menu_entry(machine, curcheat, NULL, NULL, NULL))
			{
				changed |= cheat_select_default_state(machine, curcheat);
			}
		}


		/* handle individual cheats */
		else if ((FPTR)event->itemref > 2)
		{
			switch (event->iptkey)
			{
				/* if selected, activate a oneshot */
				case IPT_UI_SELECT:
					changed = cheat_activate(machine, event->itemref);
					break;

				/* if cleared, reset to default value */
				case IPT_UI_CLEAR:
					changed = cheat_select_default_state(machine, event->itemref);
					break;

				/* left decrements */
				case IPT_UI_LEFT:
					changed = cheat_select_previous_state(machine, event->itemref);
					break;

				/* right increments */
				case IPT_UI_RIGHT:
					changed = cheat_select_next_state(machine, event->itemref);
					break;

				/* bring up display comment if one exists */
				case IPT_UI_DISPLAY_COMMENT:
				case IPT_UI_UP:
				case IPT_UI_DOWN:
					if (cheat_get_comment(event->itemref) != NULL)
						popmessage("Cheat Comment:\n%s", astring_c(cheat_get_comment(event->itemref)));
					break;
			}
		}

		/* handle reload all  */
		if ((FPTR)event->itemref == 2 && event->iptkey == IPT_UI_SELECT)
		{
			/* re-init cheat engine and thus reload cheats/cheats have already been turned off by here */
			cheat_reload(machine);

			/* display the reloaded cheats */
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
			popmessage("All cheats reloaded");
		}

		/* if things changed, update */
		if (changed)
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
	}
}


/*-------------------------------------------------
    menu_cheat_populate - populate the cheat menu
-------------------------------------------------*/

static void menu_cheat_populate(running_machine *machine, ui_menu *menu)
{
	const char *text, *subtext;
	void *curcheat;
	UINT32 flags;

	/* iterate over cheats */
	for (curcheat = cheat_get_next_menu_entry(machine, NULL, &text, &subtext, &flags);
		 curcheat != NULL;
		 curcheat = cheat_get_next_menu_entry(machine, curcheat, &text, &subtext, &flags))
	{
		ui_menu_item_append(menu, text, subtext, flags, curcheat);
	}

	/* add a separator */
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a reset all option */
	ui_menu_item_append(menu, "Reset All", NULL, 0, (void *)1);

	/* add a reload all cheats option */
	ui_menu_item_append(menu, "Reload All", NULL, 0, (void *)2);
}


/*-------------------------------------------------
    menu_memory_card - handle the memory card
    menu
-------------------------------------------------*/

static void menu_memory_card(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;
	int *cardnum;

	/* if no state, allocate some */
	if (state == NULL)
		state = ui_menu_alloc_state(menu, sizeof(*cardnum), NULL);
	cardnum = (int *)state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_memory_card_populate(machine, menu, *cardnum);

	/* process the menu */
	event = ui_menu_process(machine, menu, UI_MENU_PROCESS_LR_REPEAT);

	/* if something was selected, act on it */
	if (event != NULL && event->itemref != NULL)
	{
		FPTR item = (FPTR)event->itemref;

		/* select executes actions on some of the items */
		if (event->iptkey == IPT_UI_SELECT)
		{
			switch (item)
			{
				/* handle card loading; if we succeed, clear the menus */
				case MEMCARD_ITEM_LOAD:
					if (memcard_insert(menu->machine, *cardnum) == 0)
					{
						popmessage("Memory card loaded");
						ui_menu_stack_reset(menu->machine);
					}
					else
						popmessage("Error loading memory card");
					break;

				/* handle card ejecting */
				case MEMCARD_ITEM_EJECT:
					memcard_eject(menu->machine);
					popmessage("Memory card ejected");
					break;

				/* handle card creating */
				case MEMCARD_ITEM_CREATE:
					if (memcard_create(menu->machine, *cardnum, FALSE) == 0)
						popmessage("Memory card created");
					else
						popmessage("Error creating memory card\n(Card may already exist)");
					break;
			}
		}

		/* the select item has extra keys */
		else if (item == MEMCARD_ITEM_SELECT)
		{
			switch (event->iptkey)
			{
				/* left decrements the card number */
				case IPT_UI_LEFT:
					*cardnum -= 1;
					ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
					break;

				/* right decrements the card number */
				case IPT_UI_RIGHT:
					*cardnum += 1;
					ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
					break;
			}
		}
	}
}


/*-------------------------------------------------
    menu_memory_card_populate - populate the
    memory card menu
-------------------------------------------------*/

static void menu_memory_card_populate(running_machine *machine, ui_menu *menu, int cardnum)
{
	char tempstring[20];
	UINT32 flags = 0;

	/* add the card select menu */
	sprintf(tempstring, "%d", cardnum);
	if (cardnum > 0)
		flags |= MENU_FLAG_LEFT_ARROW;
	if (cardnum < 1000)
		flags |= MENU_FLAG_RIGHT_ARROW;
	ui_menu_item_append(menu, "Card Number:", tempstring, flags, (void *)MEMCARD_ITEM_SELECT);

	/* add the remaining items */
	ui_menu_item_append(menu, "Load Selected Card", NULL, 0, (void *)MEMCARD_ITEM_LOAD);
	if (memcard_present(machine) != -1)
		ui_menu_item_append(menu, "Eject Current Card", NULL, 0, (void *)MEMCARD_ITEM_EJECT);
	ui_menu_item_append(menu, "Create New Card", NULL, 0, (void *)MEMCARD_ITEM_CREATE);
}


/*-------------------------------------------------
    menu_sliders - handle the sliders menu
-------------------------------------------------*/

static void menu_sliders(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	int menuless_mode = (parameter != NULL);
	const ui_menu_event *event;
	UINT8 *hidden = (UINT8 *)state;

	/* if no state, allocate some */
	if (hidden == NULL)
		hidden = (UINT8 *)ui_menu_alloc_state(menu, sizeof(*hidden), NULL);
	if (menuless_mode)
		*hidden = TRUE;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_sliders_populate(machine, menu, menuless_mode);

	/* process the menu */
	event = ui_menu_process(machine, menu, UI_MENU_PROCESS_LR_REPEAT | (*hidden ? UI_MENU_PROCESS_CUSTOM_ONLY : 0));
	if (event != NULL)
	{
		/* handle keys if there is a valid item selected */
		if (event->itemref != NULL)
		{
			const slider_state *slider = (const slider_state *)event->itemref;
			INT32 curvalue = (*slider->update)(machine, slider->arg, NULL, SLIDER_NOCHANGE);
			INT32 increment = 0;

			switch (event->iptkey)
			{
				/* toggle visibility */
				case IPT_UI_ON_SCREEN_DISPLAY:
					if (menuless_mode)
						ui_menu_stack_pop(machine);
					else
						*hidden = !*hidden;
					break;

				/* decrease value */
				case IPT_UI_LEFT:
					if (input_code_pressed(machine, KEYCODE_LALT) || input_code_pressed(machine, KEYCODE_RALT))
						increment = -1;
					else if (input_code_pressed(machine, KEYCODE_LSHIFT) || input_code_pressed(machine, KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? -(slider->incval / 10) : -1;
					else if (input_code_pressed(machine, KEYCODE_LCONTROL) || input_code_pressed(machine, KEYCODE_RCONTROL))
						increment = -slider->incval * 10;
					else
						increment = -slider->incval;
					break;

				/* increase value */
				case IPT_UI_RIGHT:
					if (input_code_pressed(machine, KEYCODE_LALT) || input_code_pressed(machine, KEYCODE_RALT))
						increment = 1;
					else if (input_code_pressed(machine, KEYCODE_LSHIFT) || input_code_pressed(machine, KEYCODE_RSHIFT))
						increment = (slider->incval > 10) ? (slider->incval / 10) : 1;
					else if (input_code_pressed(machine, KEYCODE_LCONTROL) || input_code_pressed(machine, KEYCODE_RCONTROL))
						increment = slider->incval * 10;
					else
						increment = slider->incval;
					break;

				/* restore default */
				case IPT_UI_SELECT:
					increment = slider->defval - curvalue;
					break;
			}

			/* handle any changes */
			if (increment != 0)
			{
				INT32 newvalue = curvalue + increment;

				/* clamp within bounds */
				if (newvalue < slider->minval)
					newvalue = slider->minval;
				if (newvalue > slider->maxval)
					newvalue = slider->maxval;

				/* update the slider and recompute the menu */
				(*slider->update)(machine, slider->arg, NULL, newvalue);
				ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
			}
		}

		/* if we are selecting an invalid item and we are hidden, skip to the next one */
		else if (*hidden)
		{
			/* if we got here via up or page up, select the previous item */
			if (event->iptkey == IPT_UI_UP || event->iptkey == IPT_UI_PAGE_UP)
			{
				menu->selected = (menu->selected + menu->numitems - 1) % menu->numitems;
				ui_menu_validate_selection(menu, -1);
			}

			/* otherwise select the next item */
			else if (event->iptkey == IPT_UI_DOWN || event->iptkey == IPT_UI_PAGE_DOWN)
			{
				menu->selected = (menu->selected + 1) % menu->numitems;
				ui_menu_validate_selection(menu, 1);
			}
		}
	}
}


/*-------------------------------------------------
    menu_sliders_populate - populate the sliders
    menu
-------------------------------------------------*/

static void menu_sliders_populate(running_machine *machine, ui_menu *menu, int menuless_mode)
{
	astring *tempstring = astring_alloc();
	const slider_state *curslider;

	/* add all sliders */
	for (curslider = ui_get_slider_list(); curslider != NULL; curslider = curslider->next)
	{
		INT32 curval = (*curslider->update)(machine, curslider->arg, tempstring, SLIDER_NOCHANGE);
		UINT32 flags = 0;
		if (curval > curslider->minval)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (curval < curslider->maxval)
			flags |= MENU_FLAG_RIGHT_ARROW;
		ui_menu_item_append(menu, curslider->description, astring_c(tempstring), flags, (void *)curslider);

		if (menuless_mode)
			break;
	}

	ui_menu_set_custom_render(menu, menu_sliders_custom_render, 0.0f, 2.0f * ui_get_line_height() + 2.0f * UI_BOX_TB_BORDER);
	astring_free(tempstring);
}


/*-------------------------------------------------
    menu_sliders_custom_render - perform our special
    rendering
-------------------------------------------------*/

static void menu_sliders_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float x1, float y1, float x2, float y2)
{
	const slider_state *curslider = (const slider_state *)selectedref;
	if (curslider != NULL)
	{
		float bar_left, bar_area_top, bar_width, bar_area_height, bar_top, bar_bottom, default_x, current_x;
		float line_height = ui_get_line_height();
		astring *tempstring = astring_alloc();
		float percentage, default_percentage;
		float text_height;
		INT32 curval;

		/* determine the current value and text */
		curval = (*curslider->update)(machine, curslider->arg, tempstring, SLIDER_NOCHANGE);

		/* compute the current and default percentages */
		percentage = (float)(curval - curslider->minval) / (float)(curslider->maxval - curslider->minval);
		default_percentage = (float)(curslider->defval - curslider->minval) / (float)(curslider->maxval - curslider->minval);

		/* assemble the the text */
		astring_insc(tempstring, 0, " ");
		astring_insc(tempstring, 0, curslider->description);

		/* move us to the bottom of the screen, and expand to full width */
		y2 = 1.0f - UI_BOX_TB_BORDER;
		y1 = y2 - bottom;
		x1 = UI_BOX_LR_BORDER;
		x2 = 1.0f - UI_BOX_LR_BORDER;

		/* draw extra menu area */
		ui_draw_outlined_box(x1, y1, x2, y2, UI_BACKGROUND_COLOR);
		y1 += UI_BOX_TB_BORDER;

		/* determine the text height */
		ui_draw_text_full(astring_c(tempstring), 0, 0, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, NULL, &text_height);

		/* draw the thermometer */
		bar_left = x1 + UI_BOX_LR_BORDER;
		bar_area_top = y1;
		bar_width = x2 - x1 - 2.0f * UI_BOX_LR_BORDER;
		bar_area_height = line_height;

		/* compute positions */
		bar_top = bar_area_top + 0.125f * bar_area_height;
		bar_bottom = bar_area_top + 0.875f * bar_area_height;
		default_x = bar_left + bar_width * default_percentage;
		current_x = bar_left + bar_width * percentage;

		/* fill in the percentage */
		render_ui_add_rect(bar_left, bar_top, current_x, bar_bottom, UI_SLIDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the top and bottom lines */
		render_ui_add_line(bar_left, bar_top, bar_left + bar_width, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		render_ui_add_line(bar_left, bar_bottom, bar_left + bar_width, bar_bottom, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw default marker */
		render_ui_add_line(default_x, bar_area_top, default_x, bar_top, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		render_ui_add_line(default_x, bar_bottom, default_x, bar_area_top + bar_area_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		/* draw the actual text */
		ui_draw_text_full(astring_c(tempstring), x1 + UI_BOX_LR_BORDER, y1 + line_height, x2 - x1 - 2.0f * UI_BOX_LR_BORDER,
					JUSTIFY_CENTER, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, &text_height);

		astring_free(tempstring);
	}
}


/*-------------------------------------------------
    menu_video_targets - handle the video targets
    menu
-------------------------------------------------*/

static void menu_video_targets(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_video_targets_populate(machine, menu);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
		ui_menu_stack_push(ui_menu_alloc(machine, menu_video_options, event->itemref));
}


/*-------------------------------------------------
    menu_video_targets_populate - populate the
    video targets menu
-------------------------------------------------*/

static void menu_video_targets_populate(running_machine *machine, ui_menu *menu)
{
	int targetnum;

	/* find the targets */
	for (targetnum = 0; ; targetnum++)
	{
		render_target *target = render_target_get_indexed(targetnum);
		char buffer[40];

		/* stop when we run out */
		if (target == NULL)
			break;

		/* add a menu item */
		sprintf(buffer, "Screen #%d", targetnum);
		ui_menu_item_append(menu, buffer, NULL, 0, target);
	}
}


/*-------------------------------------------------
    menu_video_options - handle the video options
    menu
-------------------------------------------------*/

static void menu_video_options(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	render_target *target = (parameter != NULL) ? (render_target *)parameter : render_target_get_indexed(0);
	const ui_menu_event *event;
	int changed = FALSE;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_video_options_populate(machine, menu, target);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->itemref != NULL)
	{
		switch ((FPTR)event->itemref)
		{
			/* rotate adds rotation depending on the direction */
			case VIDEO_ITEM_ROTATE:
				if (event->iptkey == IPT_UI_LEFT || event->iptkey == IPT_UI_RIGHT)
				{
					int delta = (event->iptkey == IPT_UI_LEFT) ? ROT270 : ROT90;
					render_target_set_orientation(target, orientation_add(delta, render_target_get_orientation(target)));
					if (target == render_get_ui_target())
					{
						render_container_user_settings settings;
						render_container_get_user_settings(render_container_get_ui(), &settings);
						settings.orientation = orientation_add(delta ^ ROT180, settings.orientation);
						render_container_set_user_settings(render_container_get_ui(), &settings);
					}
					changed = TRUE;
				}
				break;

			/* layer config bitmasks handle left/right keys the same (toggle) */
			case LAYER_CONFIG_ENABLE_BACKDROP:
			case LAYER_CONFIG_ENABLE_OVERLAY:
			case LAYER_CONFIG_ENABLE_BEZEL:
			case LAYER_CONFIG_ZOOM_TO_SCREEN:
				if (event->iptkey == IPT_UI_LEFT || event->iptkey == IPT_UI_RIGHT)
				{
					render_target_set_layer_config(target, render_target_get_layer_config(target) ^ (FPTR)event->itemref);
					changed = TRUE;
				}
				break;

			/* anything else is a view item */
			default:
				if (event->iptkey == IPT_UI_SELECT && (int)(FPTR)event->itemref >= VIDEO_ITEM_VIEW)
				{
					render_target_set_view(target, (FPTR)event->itemref - VIDEO_ITEM_VIEW);
					changed = TRUE;
				}
				break;
		}
	}

	/* if something changed, rebuild the menu */
	if (changed)
		ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
}


/*-------------------------------------------------
    menu_video_options_populate - populate the
    video options menu
-------------------------------------------------*/

static void menu_video_options_populate(running_machine *machine, ui_menu *menu, render_target *target)
{
	int layermask = render_target_get_layer_config(target);
	astring *tempstring = astring_alloc();
	const char *subtext = "";
	int viewnum;
	int enabled;

	/* add items for each view */
	for (viewnum = 0; ; viewnum++)
	{
		const char *name = render_target_get_view_name(target, viewnum);
		if (name == NULL)
			break;

		/* create a string for the item, replacing underscores with spaces */
		astring_replacec(astring_cpyc(tempstring, name), 0, "_", " ");
		ui_menu_item_append(menu, astring_c(tempstring), NULL, 0, (void *)(FPTR)(VIDEO_ITEM_VIEW + viewnum));
	}

	/* add a separator */
	ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);

	/* add a rotate item */
	switch (render_target_get_orientation(target))
	{
		case ROT0:		subtext = "None";					break;
		case ROT90:		subtext = "CW 90" UTF8_DEGREES; 	break;
		case ROT180:	subtext = "180" UTF8_DEGREES; 		break;
		case ROT270:	subtext = "CCW 90" UTF8_DEGREES; 	break;
	}
	ui_menu_item_append(menu, "Rotate", subtext, MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, (void *)VIDEO_ITEM_ROTATE);

	/* backdrop item */
	enabled = layermask & LAYER_CONFIG_ENABLE_BACKDROP;
	ui_menu_item_append(menu, "Backdrops", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)LAYER_CONFIG_ENABLE_BACKDROP);

	/* overlay item */
	enabled = layermask & LAYER_CONFIG_ENABLE_OVERLAY;
	ui_menu_item_append(menu, "Overlays", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)LAYER_CONFIG_ENABLE_OVERLAY);

	/* bezel item */
	enabled = layermask & LAYER_CONFIG_ENABLE_BEZEL;
	ui_menu_item_append(menu, "Bezels", enabled ? "Enabled" : "Disabled", enabled ? MENU_FLAG_LEFT_ARROW : MENU_FLAG_RIGHT_ARROW, (void *)LAYER_CONFIG_ENABLE_BEZEL);

	/* cropping */
	enabled = layermask & LAYER_CONFIG_ZOOM_TO_SCREEN;
	ui_menu_item_append(menu, "View", enabled ? "Cropped" : "Full", enabled ? MENU_FLAG_RIGHT_ARROW : MENU_FLAG_LEFT_ARROW, (void *)LAYER_CONFIG_ZOOM_TO_SCREEN);

	astring_free(tempstring);
}


/*-------------------------------------------------
    menu_crosshair - handle the crosshair settings
    menu
-------------------------------------------------*/

static void menu_crosshair(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	const ui_menu_event *event;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_crosshair_populate(machine, menu);

	/* process the menu */
	event = ui_menu_process(machine, menu, UI_MENU_PROCESS_LR_REPEAT);

	/* handle events */
	if (event != NULL && event->itemref != NULL)
	{
		crosshair_user_settings settings;
		crosshair_item_data *data = (crosshair_item_data *)event->itemref;
		int changed = FALSE;
		int set_def = FALSE;
		int newval = data->cur;

		/* retreive the user settings */
		crosshair_get_user_settings(machine, data->player, &settings);

		switch (event->iptkey)
		{
			/* if selected, reset to default value */
			case IPT_UI_SELECT:
				newval = data->defvalue;
				set_def = TRUE;
				break;

			/* left decrements */
			case IPT_UI_LEFT:
				newval -= input_code_pressed(machine, KEYCODE_LSHIFT) ? 10 : 1;
				break;

			/* right increments */
			case IPT_UI_RIGHT:
				newval += input_code_pressed(machine, KEYCODE_LSHIFT) ? 10 : 1;
				break;
		}

		/* clamp to range */
		if (newval < data->min)
			newval = data->min;
		if (newval > data->max)
			newval = data->max;

		/* if things changed, update */
		if (newval != data->cur)
		{
			switch (data->type)
			{
				/* visibility state */
				case CROSSHAIR_ITEM_VIS:
					settings.mode = newval;
					changed = TRUE;
					break;

				/* auto time */
				case CROSSHAIR_ITEM_AUTO_TIME:
					settings.auto_time = newval;
					changed = TRUE;
					break;
			}
		}

		/* crosshair graphic name */
		if (data->type == CROSSHAIR_ITEM_PIC)
		{
			if (event->iptkey == IPT_UI_SELECT)
			{
				/* clear the name string to reset to default crosshair */
				settings.name[0] = 0;
				changed = TRUE;
			}
			else if (event->iptkey == IPT_UI_LEFT)
			{
				strcpy(settings.name, data->last_name);
				changed = TRUE;
			}
			else if (event->iptkey == IPT_UI_RIGHT)
			{
				strcpy(settings.name, data->next_name);
				changed = TRUE;
			}
		}

		if (changed)
		{
			/* save the user settings */
			crosshair_set_user_settings(machine, data->player, &settings);

			/* rebuild the menu */
			ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_POSITION);
		}
	}
}


/*-------------------------------------------------
    menu_crosshair_populate - populate the
    crosshair settings menu
-------------------------------------------------*/

static void menu_crosshair_populate(running_machine *machine, ui_menu *menu)
{
	crosshair_user_settings settings;
	crosshair_item_data *data;
	char temp_text[16];
	int player;
	UINT8 use_auto = FALSE;
	UINT32 flags = 0;
	mame_path *path;

	/* loop over player and add the manual items */
	for (player = 0; player < MAX_PLAYERS; player++)
	{
		/* get the user settings */
		crosshair_get_user_settings(machine, player, &settings);

		/* add menu items for usable crosshairs */
		if (settings.used)
		{
			/* Make sure to keep these matched to the CROSSHAIR_VISIBILITY_xxx types */
			static const char *const vis_text[] = { "Off", "On", "Auto" };

			/* track if we need the auto time menu */
			if (settings.mode == CROSSHAIR_VISIBILITY_AUTO) use_auto = TRUE;

			/* CROSSHAIR_ITEM_VIS - allocate a data item and fill it */
			data = (crosshair_item_data *)ui_menu_pool_alloc(menu, sizeof(*data));
			data->type = CROSSHAIR_ITEM_VIS;
			data->player = player;
			data->min = CROSSHAIR_VISIBILITY_OFF;
			data->max = CROSSHAIR_VISIBILITY_AUTO;
			data->defvalue = CROSSHAIR_VISIBILITY_DEFAULT;
			data->cur = settings.mode;

			/* put on arrows */
			if (data->cur > data->min)
				flags |= MENU_FLAG_LEFT_ARROW;
			if (data->cur < data->max)
				flags |= MENU_FLAG_RIGHT_ARROW;

			/* add CROSSHAIR_ITEM_VIS menu */
			sprintf(temp_text, "P%d Visibility", player + 1);
			ui_menu_item_append(menu, temp_text, vis_text[settings.mode], flags, data);

			/* CROSSHAIR_ITEM_PIC - allocate a data item and fill it */
			data = (crosshair_item_data *)ui_menu_pool_alloc(menu, sizeof(*data));
			data->type = CROSSHAIR_ITEM_PIC;
			data->player = player;
			data->last_name[0] = 0;
			/* other data item not used by this menu */

			/* search for crosshair graphics */

			/* open a path to the crosshairs */
			path = mame_openpath(mame_options(), OPTION_CROSSHAIRPATH);
			if (path != NULL)
			{
				const osd_directory_entry *dir;

				/* reset search flags */
				int using_default = FALSE;
				int finished = FALSE;
				int found = FALSE;

				/* if we are using the default, then we just need to find the first in the list */
				if (strlen(settings.name) == 0)
					using_default = TRUE;

				/* look for the current name, then remember the name before */
				/* and find the next name */
				while (((dir = mame_readpath(path)) != NULL) && !finished)
				{
					int length = strlen(dir->name);

					/* look for files ending in .png with a name not larger then 9 chars*/
					if ((length > 4) && (length <= CROSSHAIR_PIC_NAME_LENGTH + 4) &&
						dir->name[length - 4] == '.' &&
						tolower((UINT8)dir->name[length - 3]) == 'p' &&
						tolower((UINT8)dir->name[length - 2]) == 'n' &&
						tolower((UINT8)dir->name[length - 1]) == 'g')

					{
						/* remove .png from length */
						length -= 4;

						if (found || using_default)
						{
							/* get the next name */
							strncpy(data->next_name, dir->name, length);
							data->next_name[length] = 0;
							finished = TRUE;
						}
						else if (!strncmp(dir->name, settings.name, length))
						{
							/* we found the current name */
							/* so loop once more to find the next name */
							found = TRUE;
						}
						else
							/* remember last name */
							/* we will do it here in case files get added to the directory */
						{
							strncpy(data->last_name, dir->name, length);
							data->last_name[length] = 0;
						}
					}
				}
				/* if name not found then next item is DEFAULT */
				if (!found && !using_default)
				{
					data->next_name[0] = 0;
					finished = TRUE;
				}
				/* setup the selection flags */
				flags = 0;
				if (finished)
					flags |= MENU_FLAG_RIGHT_ARROW;
				if (found)
					flags |= MENU_FLAG_LEFT_ARROW;

				/* add CROSSHAIR_ITEM_PIC menu */
				sprintf(temp_text, "P%d Crosshair", player + 1);
				ui_menu_item_append(menu, temp_text, using_default ? "DEFAULT" : settings.name, flags, data);
			}
		}
	}
	if (use_auto)
	{
		/* any player can be used to get the autotime */
		crosshair_get_user_settings(machine, 0, &settings);

		/* CROSSHAIR_ITEM_AUTO_TIME - allocate a data item and fill it */
		data = (crosshair_item_data *)ui_menu_pool_alloc(menu, sizeof(*data));
		data->type = CROSSHAIR_ITEM_AUTO_TIME;
		data->min = CROSSHAIR_VISIBILITY_AUTOTIME_MIN;
		data->max = CROSSHAIR_VISIBILITY_AUTOTIME_MAX;
		data->defvalue = CROSSHAIR_VISIBILITY_AUTOTIME_DEFAULT;
		data->cur = settings.auto_time;

		/* put on arrows in visible menu */
		if (data->cur > data->min)
			flags |= MENU_FLAG_LEFT_ARROW;
		if (data->cur < data->max)
			flags |= MENU_FLAG_RIGHT_ARROW;

		/* add CROSSHAIR_ITEM_AUTO_TIME menu */
		sprintf(temp_text, "%d", settings.auto_time);
		ui_menu_item_append(menu, "Visible Delay", temp_text, flags, data);
	}
//  else
//      /* leave a blank filler line when not in auto time so size does not rescale */
//      ui_menu_item_append(menu, "", "", NULL, NULL);
}


/*-------------------------------------------------
    menu_quit_game - handle the "menu" for
    quitting the game
-------------------------------------------------*/

static void menu_quit_game(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	/* request a reset */
	mame_schedule_exit(machine);

	/* reset the menu stack */
	ui_menu_stack_reset(machine);
}


/*-------------------------------------------------
    menu_select_game - handle the game select
    menu
-------------------------------------------------*/

static void menu_select_game(running_machine *machine, ui_menu *menu, void *parameter, void *state)
{
	select_game_state *menustate;
	const ui_menu_event *event;

	/* if no state, allocate some */
	if (state == NULL)
	{
		state = ui_menu_alloc_state(menu, sizeof(*menustate) + sizeof(menustate->driverlist) * driver_list_get_count(drivers), NULL);
		if (parameter != NULL)
			strcpy(((select_game_state *)state)->search, (const char *)parameter);
	}
	menustate = (select_game_state *)state;

	/* if the menu isn't built, populate now */
	if (!ui_menu_populated(menu))
		menu_select_game_populate(machine, menu, menustate);

	/* ignore pause keys by swallowing them before we process the menu */
	ui_input_pressed(machine, IPT_UI_PAUSE);

	/* process the menu */
	event = ui_menu_process(machine, menu, 0);
	if (event != NULL && event->itemref != NULL)
	{
		/* reset the error on any future event */
		if (menustate->error)
			menustate->error = FALSE;

		/* handle selections */
		else if (event->iptkey == IPT_UI_SELECT)
		{
			const game_driver *driver = (const game_driver *)event->itemref;

			/* special case for configure inputs */
			if ((FPTR)driver == 1)
				ui_menu_stack_push(ui_menu_alloc(menu->machine, menu_input_groups, NULL));

			/* anything else is a driver */
			else
			{
				audit_record *audit;
				int audit_records;
				int audit_result;

				/* audit the game first to see if we're going to work */
				audit_records = audit_images(mame_options(), driver, AUDIT_VALIDATE_FAST, &audit);
				audit_result = audit_summary(driver, audit_records, audit, FALSE);
				if (audit_records > 0)
					free(audit);

				/* if everything looks good, schedule the new driver */
				if (audit_result == CORRECT || audit_result == BEST_AVAILABLE)
				{
					mame_schedule_new_driver(machine, driver);
					ui_menu_stack_reset(machine);
				}

				/* otherwise, display an error */
				else
				{
					ui_menu_reset(menu, UI_MENU_RESET_REMEMBER_REF);
					menustate->error = TRUE;
				}
			}
		}

		/* escape pressed with non-empty text clears the text */
		else if (event->iptkey == IPT_UI_CANCEL && menustate->search[0] != 0)
		{
			/* since we have already been popped, we must recreate ourself from scratch */
			ui_menu_stack_push(ui_menu_alloc(menu->machine, menu_select_game, NULL));
		}

		/* typed characters append to the buffer */
		else if (event->iptkey == IPT_SPECIAL)
		{
			int buflen = strlen(menustate->search);

			/* if it's a backspace and we can handle it, do so */
			if ((event->unichar == 8 || event->unichar == 0x7f) && buflen > 0)
			{
				*(char *)utf8_previous_char(&menustate->search[buflen]) = 0;
				menustate->rerandomize = TRUE;
				ui_menu_reset(menu, UI_MENU_RESET_SELECT_FIRST);
			}

			/* if it's any other key and we're not maxed out, update */
			else if (event->unichar >= ' ' && event->unichar < 0x7f)
			{
				buflen += utf8_from_uchar(&menustate->search[buflen], ARRAY_LENGTH(menustate->search) - buflen, event->unichar);
				menustate->search[buflen] = 0;
				ui_menu_reset(menu, UI_MENU_RESET_SELECT_FIRST);
			}
		}
	}

	/* if we're in an error state, overlay an error message */
	if (menustate->error)
		ui_draw_text_box("The selected game is missing one or more required ROM or CHD images. "
		                 "Please select a different game.\n\nPress any key to continue.",
		                 JUSTIFY_CENTER, 0.5f, 0.5f, UI_RED_COLOR);
}


/*-------------------------------------------------
    menu_select_game_populate - populate the game
    select menu
-------------------------------------------------*/

static void menu_select_game_populate(running_machine *machine, ui_menu *menu, select_game_state *menustate)
{
	int matchcount;
	int curitem;

	/* update our driver list if necessary */
	if (menustate->driverlist[0] == NULL)
		menu_select_game_build_driver_list(menu, menustate);
	for (curitem = matchcount = 0; menustate->driverlist[curitem] != NULL && matchcount < VISIBLE_GAMES_IN_LIST; curitem++)
		if (!(menustate->driverlist[curitem]->flags & GAME_NO_STANDALONE))
			matchcount++;

	/* if nothing there, add a single multiline item and return */
	if (matchcount == 0)
	{
		ui_menu_item_append(menu, "No "GAMESNOUN" found. Please check the rompath specified in the "CONFIGNAME".ini file.\n\n"
								  "If this is your first time using "APPNAME", please see the config.txt file in "
								  "the docs directory for information on configuring "APPNAME".", NULL, MENU_FLAG_MULTILINE | MENU_FLAG_REDTEXT, NULL);
		return;
	}

	/* otherwise, rebuild the match list */
	if (menustate->search[0] != 0 || menustate->matchlist[0] == NULL || menustate->rerandomize)
		driver_list_get_approx_matches(menustate->driverlist, menustate->search, matchcount, menustate->matchlist);
	menustate->rerandomize = FALSE;

	/* iterate over entries */
	for (curitem = 0; curitem < matchcount; curitem++)
	{
		const game_driver *driver = menustate->matchlist[curitem];
		if (driver != NULL)
		{
			const game_driver *cloneof = driver_get_clone(driver);
			ui_menu_item_append(menu, driver->name, driver->description, (cloneof == NULL || (cloneof->flags & GAME_IS_BIOS_ROOT)) ? 0 : MENU_FLAG_INVERT, (void *)driver);
		}
	}

	/* if we're forced into this, allow general input configuration as well */
	if (ui_menu_is_force_game_select())
	{
		ui_menu_item_append(menu, MENU_SEPARATOR_ITEM, NULL, 0, NULL);
		ui_menu_item_append(menu, "Configure General Inputs", NULL, 0, (void *)1);
	}

	/* configure the custom rendering */
	ui_menu_set_custom_render(menu, menu_select_game_custom_render, ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER, 4.0f * ui_get_line_height() + 3.0f * UI_BOX_TB_BORDER);
}


/*-------------------------------------------------
    menu_select_game_driver_compare - compare the
    names of two drivers
-------------------------------------------------*/

static int CLIB_DECL menu_select_game_driver_compare(const void *elem1, const void *elem2)
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
    menu_select_game_build_driver_list - build a
    list of available drivers
-------------------------------------------------*/

static void menu_select_game_build_driver_list(ui_menu *menu, select_game_state *menustate)
{
	int driver_count = driver_list_get_count(drivers);
	int drivnum, listnum;
	mame_path *path;
	UINT8 *found;

	/* create a sorted copy of the main driver list */
	memcpy((void *)menustate->driverlist, drivers, driver_count * sizeof(menustate->driverlist[0]));
	qsort((void *)menustate->driverlist, driver_count, sizeof(menustate->driverlist[0]), menu_select_game_driver_compare);

	/* allocate a temporary array to track which ones we found */
	found = (UINT8 *)ui_menu_pool_alloc(menu, (driver_count + 7) / 8);
	memset(found, 0, (driver_count + 7) / 8);

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
				*dst++ = tolower((UINT8)*src);
			*dst = 0;

			/* find it in the array */
			tempdriver.name = drivername;
			tempdriver_ptr = &tempdriver;
			found_driver = (const game_driver **)bsearch(&tempdriver_ptr, menustate->driverlist, driver_count, sizeof(*menustate->driverlist), menu_select_game_driver_compare);

			/* if found, mark the corresponding entry in the array */
			if (found_driver != NULL)
			{
				int index = found_driver - menustate->driverlist;
				found[index / 8] |= 1 << (index % 8);
			}
		}
		mame_closepath(path);
	}

	/* now build the final list */
	for (drivnum = listnum = 0; drivnum < driver_count; drivnum++)
		if (found[drivnum / 8] & (1 << (drivnum % 8)))
			menustate->driverlist[listnum++] = menustate->driverlist[drivnum];

	/* NULL-terminate */
	menustate->driverlist[listnum] = NULL;
}


/*-------------------------------------------------
    menu_select_game_custom_render - perform our
    special rendering
-------------------------------------------------*/

static void menu_select_game_custom_render(running_machine *machine, ui_menu *menu, void *state, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	select_game_state *menustate = (select_game_state *)state;
	const game_driver *driver;
	float width, maxwidth;
	float x1, y1, x2, y2;
	char tempbuf[4][256];
	rgb_t color;
	int line;

	/* display the current typeahead */
	if (menustate->search[0] != 0)
		sprintf(&tempbuf[0][0], "Type name or select: %s_", menustate->search);
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
	y1 = origy1 - top;
	y2 = origy1 - UI_BOX_TB_BORDER;

	/* draw a box */
	ui_draw_outlined_box(x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* take off the borders */
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	/* draw the text within it */
	ui_draw_text_full(&tempbuf[0][0], x1, y1, x2 - x1, JUSTIFY_CENTER, WRAP_TRUNCATE,
					  DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	/* determine the text to render below */
	driver = ((FPTR)selectedref > 1) ? (const game_driver *)selectedref : NULL;
	if ((FPTR)driver > 1)
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
		const char *s = COPYRIGHT;
		int line = 0;
		int col = 0;

		/* first line is version string */
		sprintf(&tempbuf[line++][0], "%s %s", APPLONGNAME, build_version);

		/* output message */
		while (line < ARRAY_LENGTH(tempbuf))
		{
			if (*s == 0 || *s == '\n')
			{
				tempbuf[line++][col] = 0;
				col = 0;
			}
			else
				tempbuf[line][col++] = *s;

			if (*s != 0)
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
	y2 = origy2 + bottom;

	/* draw a box */
	color = UI_BACKGROUND_COLOR;
	if (driver != NULL)
		color = UI_GREEN_COLOR;
	if (driver != NULL && (driver->flags & (GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_IMPERFECT_COLORS | GAME_NO_SOUND | GAME_IMPERFECT_SOUND)) != 0)
		color = UI_YELLOW_COLOR;
	if (driver != NULL && (driver->flags & (GAME_NOT_WORKING | GAME_UNEMULATED_PROTECTION)) != 0)
		color = UI_RED_COLOR;
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
						  DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);
		y1 += ui_get_line_height();
	}
}



/***************************************************************************
    MENU HELPERS
***************************************************************************/

/*-------------------------------------------------
    menu_render_triangle - render a triangle that
    is used for up/down arrows and left/right
    indicators
-------------------------------------------------*/

static void menu_render_triangle(bitmap_t *dest, const bitmap_t *source, const rectangle *sbounds, void *param)
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
