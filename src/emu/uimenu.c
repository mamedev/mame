/*********************************************************************

    uimenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "ui.h"
#include "rendutil.h"
#include "uiinput.h"
#include "cheat.h"
#include "uimain.h"
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE		65536
#define UI_MENU_ALLOC_ITEMS		256

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

ui_menu *ui_menu::menu_stack;
ui_menu *ui_menu::menu_free;
bitmap_rgb32 *ui_menu::hilight_bitmap;
render_texture *ui_menu::hilight_texture;
render_texture *ui_menu::arrow_texture;

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    is_selectable - return TRUE if the given
    item is selectable
-------------------------------------------------*/

inline bool ui_menu_item::is_selectable() const
{
	return ((flags & (MENU_FLAG_MULTILINE | MENU_FLAG_DISABLE)) == 0 && strcmp(text, MENU_SEPARATOR_ITEM) != 0);
}


/*-------------------------------------------------
    exclusive_input_pressed - return TRUE if the
    given key is pressed and we haven't already
    reported a key
-------------------------------------------------*/

inline bool ui_menu::exclusive_input_pressed(int key, int repeat)
{
	if (menu_event.iptkey == IPT_INVALID && ui_input_pressed_repeat(machine(), key, repeat))
	{
		menu_event.iptkey = key;
		return true;
	}
	return false;
}



/***************************************************************************
    CORE SYSTEM MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    init - initialize the menu system
-------------------------------------------------*/

void ui_menu::init(running_machine &machine)
{
	int x;

	/* initialize the menu stack */
	ui_menu::stack_reset(machine);

	/* create a texture for hilighting items */
	hilight_bitmap = auto_bitmap_rgb32_alloc(machine, 256, 1);
	for (x = 0; x < 256; x++)
	{
		int alpha = 0xff;
		if (x < 25) alpha = 0xff * x / 25;
		if (x > 256 - 25) alpha = 0xff * (255 - x) / 25;
		hilight_bitmap->pix32(0, x) = MAKE_ARGB(alpha,0xff,0xff,0xff);
	}
	hilight_texture = machine.render().texture_alloc();
	hilight_texture->set_bitmap(*hilight_bitmap, hilight_bitmap->cliprect(), TEXFORMAT_ARGB32);

	/* create a texture for arrow icons */
	arrow_texture = machine.render().texture_alloc(render_triangle);

	/* add an exit callback to free memory */
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_menu::exit), &machine));
}


/*-------------------------------------------------
    exit - clean up after ourselves
-------------------------------------------------*/

void ui_menu::exit(running_machine &machine)
{
	/* free menus */
	ui_menu::stack_reset(machine);
	ui_menu::clear_free_list(machine);

	/* free textures */
	machine.render().texture_free(hilight_texture);
	machine.render().texture_free(arrow_texture);
}



/***************************************************************************
    CORE MENU MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    ui_menu - menu constructor
-------------------------------------------------*/
ui_menu::ui_menu(running_machine &machine, render_container *_container) : m_machine(machine)
{
	special_main_menu = false;
	container = _container;

	reset(UI_MENU_RESET_SELECT_FIRST);
}


/*-------------------------------------------------
    ~ui_menu - menu destructor
-------------------------------------------------*/

ui_menu::~ui_menu()
{
	/* free the pools */
	while (pool)
	{
		ui_menu_pool *ppool = pool;
		pool = pool->next;
		auto_free(machine(), ppool);
	}

	/* free the item array */
	if (item)
		auto_free(machine(), item);
}


/*-------------------------------------------------
    reset - free all items in the menu,
    and all memory allocated from the memory pool
-------------------------------------------------*/

void ui_menu::reset(ui_menu_reset_options options)
{
	/* based on the reset option, set the reset info */
	resetpos = 0;
	resetref = NULL;
	if (options == UI_MENU_RESET_REMEMBER_POSITION)
		resetpos = selected;
	else if (options == UI_MENU_RESET_REMEMBER_REF)
		resetref = item[selected].ref;

	/* reset all the pools and the numitems back to 0 */
	for (ui_menu_pool *ppool = pool; ppool != NULL; ppool = ppool->next)
		ppool->top = (UINT8 *)(ppool + 1);
	numitems = 0;
	visitems = 0;
	selected = 0;
	astring backtext;
	backtext.printf("Return to %s",emulator_info::get_capstartgamenoun());

	/* add an item to return */
	if (parent == NULL)
		item_append(backtext.cstr(), NULL, 0, NULL);
	else if (parent->is_special_main_menu())
		item_append("Exit", NULL, 0, NULL);
	else
		item_append("Return to Prior Menu", NULL, 0, NULL);
}


/*-------------------------------------------------
    is_special_main_menu - returns whether the
    menu has special needs
-------------------------------------------------*/
bool ui_menu::is_special_main_menu() const
{
	return special_main_menu;
}


/*-------------------------------------------------
    set_special_main_menu - set whether the
    menu has special needs
-------------------------------------------------*/
void ui_menu::set_special_main_menu(bool special)
{
	special_main_menu = special;
}


/*-------------------------------------------------
    populated - returns true if the menu
    has any non-default items in it
-------------------------------------------------*/

bool ui_menu::populated()
{
	return numitems > 1;
}


/*-------------------------------------------------
    item_append - append a new item to the
    end of the menu
-------------------------------------------------*/

void ui_menu::item_append(const char *text, const char *subtext, UINT32 flags, void *ref)
{
	ui_menu_item *pitem;
	int index;

	/* only allow multiline as the first item */
	if ((flags & MENU_FLAG_MULTILINE) != 0)
		assert(numitems == 1);

	/* only allow a single multi-line item */
	else if (numitems >= 2)
		assert((item[0].flags & MENU_FLAG_MULTILINE) == 0);

	/* realloc the item array if necessary */
	if (numitems >= allocitems)
	{
		int olditems = allocitems;
		allocitems += UI_MENU_ALLOC_ITEMS;
		ui_menu_item *newitems = auto_alloc_array(machine(), ui_menu_item, allocitems);
		for (int itemnum = 0; itemnum < olditems; itemnum++)
			newitems[itemnum] = item[itemnum];
		auto_free(machine(), item);
		item = newitems;
	}
	index = numitems++;

	/* copy the previous last item to the next one */
	if (index != 0)
	{
		index--;
		item[index + 1] = item[index];
	}

	/* allocate a new item and populate it */
	pitem = &item[index];
	pitem->text = (text != NULL) ? pool_strdup(text) : NULL;
	pitem->subtext = (subtext != NULL) ? pool_strdup(subtext) : NULL;
	pitem->flags = flags;
	pitem->ref = ref;

	/* update the selection if we need to */
	if (resetpos == index || (resetref != NULL && resetref == ref))
		selected = index;
	if (resetpos == numitems - 1)
		selected = numitems - 1;
}


/*-------------------------------------------------
    process - process a menu, drawing it
    and returning any interesting events
-------------------------------------------------*/

const ui_menu_event *ui_menu::process(UINT32 flags)
{
	/* reset the menu_event */
	menu_event.iptkey = IPT_INVALID;

	/* first make sure our selection is valid */
	validate_selection(1);

	/* draw the menu */
	if (numitems > 1 && (item[0].flags & MENU_FLAG_MULTILINE) != 0)
		draw_text_box();
	else
		draw(flags & UI_MENU_PROCESS_CUSTOM_ONLY);

	/* process input */
	if (!(flags & UI_MENU_PROCESS_NOKEYS))
	{
		/* read events */
		handle_events();

		/* handle the keys if we don't already have an menu_event */
		if (menu_event.iptkey == IPT_INVALID)
			handle_keys(flags);
	}

	/* update the selected item in the menu_event */
	if (menu_event.iptkey != IPT_INVALID && selected >= 0 && selected < numitems)
	{
		menu_event.itemref = item[selected].ref;
		return &menu_event;
	}
	return NULL;
}


/*-------------------------------------------------
    m_pool_alloc - allocate temporary memory
    from the menu's memory pool
-------------------------------------------------*/

void *ui_menu::m_pool_alloc(size_t size)
{
	ui_menu_pool *ppool;

	assert(size < UI_MENU_POOL_SIZE);

	/* find a pool with enough room */
	for (ppool = pool; ppool != NULL; ppool = ppool->next)
		if (ppool->end - ppool->top >= size)
		{
			void *result = ppool->top;
			ppool->top += size;
			return result;
		}

	/* allocate a new pool */
	ppool = (ui_menu_pool *)auto_alloc_array_clear(machine(), UINT8, sizeof(*ppool) + UI_MENU_POOL_SIZE);

	/* wire it up */
	ppool->next = pool;
	pool = ppool;
	ppool->top = (UINT8 *)(ppool + 1);
	ppool->end = ppool->top + UI_MENU_POOL_SIZE;
	return m_pool_alloc(size);
}


/*-------------------------------------------------
    pool_strdup - make a temporary string
    copy in the menu's memory pool
-------------------------------------------------*/

const char *ui_menu::pool_strdup(const char *string)
{
	return strcpy((char *)m_pool_alloc(strlen(string) + 1), string);
}


/*-------------------------------------------------
    get_selection - retrieves the index
    of the currently selected menu item
-------------------------------------------------*/

void *ui_menu::get_selection()
{
	return (selected >= 0 && selected < numitems) ? item[selected].ref : NULL;
}


/*-------------------------------------------------
    set_selection - changes the index
    of the currently selected menu item
-------------------------------------------------*/

void ui_menu::set_selection(void *selected_itemref)
{
	int itemnum;

	selected = -1;
	for (itemnum = 0; itemnum < numitems; itemnum++)
		if (item[itemnum].ref == selected_itemref)
		{
			selected = itemnum;
			break;
		}
}



/***************************************************************************
    INTERNAL MENU PROCESSING
***************************************************************************/

/*-------------------------------------------------
    draw - draw a menu
-------------------------------------------------*/

void ui_menu::draw(bool customonly)
{
	float line_height = ui_get_line_height(machine());
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
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
	for (itemnum = 0; itemnum < numitems; itemnum++)
	{
		const ui_menu_item &pitem = item[itemnum];
		float total_width;

		/* compute width of left hand side */
		total_width = gutter_width + ui_get_string_width(machine(), pitem.text) + gutter_width;

		/* add in width of right hand side */
		if (pitem.subtext)
			total_width += 2.0f * gutter_width + ui_get_string_width(machine(), pitem.subtext);

		/* track the maximum */
		if (total_width > visible_width)
			visible_width = total_width;

		/* track the height as well */
		visible_main_menu_height += line_height;
	}

	/* account for extra space at the top and bottom */
	visible_extra_menu_height = customtop + custombottom;

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
	visible_top += customtop;

	/* first add us a box */
	x1 = visible_left - UI_BOX_LR_BORDER;
	y1 = visible_top - UI_BOX_TB_BORDER;
	x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	if (!customonly)
		ui_draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	/* determine the first visible line based on the current selection */
	top_line = selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= numitems)
		top_line = numitems - visible_lines;

	/* determine effective positions taking into account the hilighting arrows */
	effective_width = visible_width - 2.0f * gutter_width;
	effective_left = visible_left + gutter_width;

	/* locate mouse */
	mouse_hit = FALSE;
	mouse_button = FALSE;
	if (!customonly)
	{
		mouse_target = ui_input_find_mouse(machine(), &mouse_target_x, &mouse_target_y, &mouse_button);
		if (mouse_target != NULL)
			if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
				mouse_hit = TRUE;
	}

	/* loop over visible lines */
	hover = numitems + 1;
	if (!customonly)
		for (linenum = 0; linenum < visible_lines; linenum++)
		{
			float line_y = visible_top + (float)linenum * line_height;
			itemnum = top_line + linenum;
			const ui_menu_item &pitem = item[itemnum];
			const char *itemtext = pitem.text;
			rgb_t fgcolor = UI_TEXT_COLOR;
			rgb_t bgcolor = UI_TEXT_BG_COLOR;
			rgb_t fgcolor2 = UI_SUBITEM_COLOR;
			rgb_t fgcolor3 = UI_CLONE_COLOR;
			float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
			float line_y0 = line_y;
			float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
			float line_y1 = line_y + line_height;

			/* set the hover if this is our item */
			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
				hover = itemnum;

			/* if we're selected, draw with a different background */
			if (itemnum == selected)
			{
				fgcolor = UI_SELECTED_COLOR;
				bgcolor = UI_SELECTED_BG_COLOR;
				fgcolor2 = UI_SELECTED_COLOR;
				fgcolor3 = UI_SELECTED_COLOR;
			}

			/* else if the mouse is over this item, draw with a different background */
			else if (itemnum == hover)
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor2 = UI_MOUSEOVER_COLOR;
				fgcolor3 = UI_MOUSEOVER_COLOR;
			}

			/* if we have some background hilighting to do, add a quad behind everything else */
			if (bgcolor != UI_TEXT_BG_COLOR)
				container->add_quad(line_x0, line_y0, line_x1, line_y1, bgcolor, hilight_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

			/* if we're on the top line, display the up arrow */
			if (linenum == 0 && top_line != 0)
			{
				container->add_quad(
						            0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
									line_y + 0.25f * line_height,
									0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
									line_y + 0.75f * line_height,
									fgcolor,
									arrow_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT0));
				if (hover == itemnum)
					hover = -2;
			}

			/* if we're on the bottom line, display the down arrow */
			else if (linenum == visible_lines - 1 && itemnum != numitems - 1)
			{
				container->add_quad(
									0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
									line_y + 0.25f * line_height,
									0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
									line_y + 0.75f * line_height,
									fgcolor,
									arrow_texture,
									PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT0 ^ ORIENTATION_FLIP_Y));
				if (hover == itemnum)
					hover = -1;
			}

			/* if we're just a divider, draw a line */
			else if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
				container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			/* if we don't have a subitem, just draw the string centered */
			else if (pitem.subtext == NULL)
				ui_draw_text_full(container, itemtext, effective_left, line_y, effective_width,
							JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);

			/* otherwise, draw the item on the left and the subitem text on the right */
			else
			{
				int subitem_invert = pitem.flags & MENU_FLAG_INVERT;
				const char *subitem_text = pitem.subtext;
				float item_width, subitem_width;

				/* draw the left-side text */
				ui_draw_text_full(container, itemtext, effective_left, line_y, effective_width,
							JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, &item_width, NULL);

				/* give 2 spaces worth of padding */
				item_width += 2.0f * gutter_width;

				/* if the subitem doesn't fit here, display dots */
				if (ui_get_string_width(machine(), subitem_text) > effective_width - item_width)
				{
					subitem_text = "...";
					if (itemnum == selected)
						selected_subitem_too_big = TRUE;
				}

				/* draw the subitem right-justified */
				ui_draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
							JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor, &subitem_width, NULL);

				/* apply arrows */
				if (itemnum == selected && (pitem.flags & MENU_FLAG_LEFT_ARROW))
				{
					container->add_quad(
										effective_left + effective_width - subitem_width - gutter_width,
										line_y + 0.1f * line_height,
										effective_left + effective_width - subitem_width - gutter_width + lr_arrow_width,
										line_y + 0.9f * line_height,
										fgcolor,
										arrow_texture,
										PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(ROT90 ^ ORIENTATION_FLIP_X));
				}
				if (itemnum == selected && (pitem.flags & MENU_FLAG_RIGHT_ARROW))
				{
					container->add_quad(
										effective_left + effective_width + gutter_width - lr_arrow_width,
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
		const ui_menu_item &pitem = item[selected];
		int subitem_invert = pitem.flags & MENU_FLAG_INVERT;
		linenum = selected - top_line;
		float line_y = visible_top + (float)linenum * line_height;
		float target_width, target_height;
		float target_x, target_y;

		/* compute the multi-line target width/height */
		ui_draw_text_full(container, pitem.subtext, 0, 0, visible_width * 0.75f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);

		/* determine the target location */
		target_x = visible_left + visible_width - target_width - UI_BOX_LR_BORDER;
		target_y = line_y + line_height + UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > visible_main_menu_height)
			target_y = line_y - target_height - UI_BOX_TB_BORDER;

		/* add a box around that */
		ui_draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
						 target_y - UI_BOX_TB_BORDER,
						 target_x + target_width + UI_BOX_LR_BORDER,
						 target_y + target_height + UI_BOX_TB_BORDER, subitem_invert ? UI_SELECTED_BG_COLOR : UI_BACKGROUND_COLOR);
		ui_draw_text_full(container, pitem.subtext, target_x, target_y, target_width,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);
	}

	/* if there is something special to add, do it by calling the virtual method */
	custom_render((selected >= 0 && selected < numitems) ? item[selected].ref : NULL, customtop, custombottom, x1, y1, x2, y2);

	/* return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow */
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != numitems);
}

void ui_menu::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
}

/*-------------------------------------------------
    draw_text_box - draw a multiline
    word-wrapped text box with a menu item at the
    bottom
-------------------------------------------------*/

void ui_menu::draw_text_box()
{
	const char *text = item[0].text;
	const char *backtext = item[1].text;
	float line_height = ui_get_line_height(machine());
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width;
	float target_width, target_height, prior_width;
	float target_x, target_y;

	/* compute the multi-line target width/height */
	ui_draw_text_full(container, text, 0, 0, 1.0f - 2.0f * UI_BOX_LR_BORDER - 2.0f * gutter_width,
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);
	target_height += 2.0f * line_height;
	if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
		target_height = floor((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;

	/* maximum against "return to prior menu" text */
	prior_width = ui_get_string_width(machine(), backtext) + 2.0f * gutter_width;
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
	ui_draw_outlined_box(container, target_x - UI_BOX_LR_BORDER - gutter_width,
					 target_y - UI_BOX_TB_BORDER,
					 target_x + target_width + gutter_width + UI_BOX_LR_BORDER,
					 target_y + target_height + UI_BOX_TB_BORDER, (item[0].flags & MENU_FLAG_REDTEXT) ?  UI_RED_COLOR : UI_BACKGROUND_COLOR);
	ui_draw_text_full(container, text, target_x, target_y, target_width,
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	/* draw the "return to prior menu" text with a hilight behind it */
	container->add_quad(
						target_x + 0.5f * UI_LINE_WIDTH,
						target_y + target_height - line_height,
						target_x + target_width - 0.5f * UI_LINE_WIDTH,
						target_y + target_height,
						UI_SELECTED_BG_COLOR,
						hilight_texture,
						PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
	ui_draw_text_full(container, backtext, target_x, target_y + target_height - line_height, target_width,
				JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);

	/* artificially set the hover to the last item so a double-click exits */
	hover = numitems - 1;
}


/*-------------------------------------------------
    handle_events - generically handle
    input events for a menu
-------------------------------------------------*/

void ui_menu::handle_events()
{
	int stop = FALSE;
	ui_event local_menu_event;

	/* loop while we have interesting events */
	while (ui_input_pop_event(machine(), &local_menu_event) && !stop)
	{
		switch (local_menu_event.event_type)
		{
			/* if we are hovering over a valid item, select it with a single click */
			case UI_EVENT_MOUSE_DOWN:
				if (hover >= 0 && hover < numitems)
					selected = hover;
				else if (hover == -2)
				{
					selected -= visitems - 1;
					validate_selection(1);
				}
				else if (hover == -1)
				{
					selected += visitems - 1;
					validate_selection(1);
				}
				break;

			/* if we are hovering over a valid item, fake a UI_SELECT with a double-click */
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if (hover >= 0 && hover < numitems)
				{
					selected = hover;
					if (local_menu_event.event_type == UI_EVENT_MOUSE_DOUBLE_CLICK)
					{
						menu_event.iptkey = IPT_UI_SELECT;
						if (selected == numitems - 1)
						{
							menu_event.iptkey = IPT_UI_CANCEL;
							ui_menu::stack_pop(machine());
						}
					}
					stop = TRUE;
				}
				break;

			/* translate CHAR events into specials */
			case UI_EVENT_CHAR:
				menu_event.iptkey = IPT_SPECIAL;
				menu_event.unichar = local_menu_event.ch;
				stop = TRUE;
				break;

			/* ignore everything else */
			default:
				break;
		}
	}
}


/*-------------------------------------------------
    handle_keys - generically handle
    keys for a menu
-------------------------------------------------*/

void ui_menu::handle_keys(UINT32 flags)
{
	int ignorepause = ui_menu::stack_has_special_main_menu();
	int ignoreright = FALSE;
	int ignoreleft = FALSE;
	int code;

	/* bail if no items */
	if (numitems == 0)
		return;

	/* if we hit select, return TRUE or pop the stack, depending on the item */
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (selected == numitems - 1)
		{
			menu_event.iptkey = IPT_UI_CANCEL;
			ui_menu::stack_pop(machine());
		}
		return;
	}

	/* hitting cancel also pops the stack */
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		ui_menu::stack_pop(machine());
		return;
	}

	/* validate the current selection */
	validate_selection(1);

	/* swallow left/right keys if they are not appropriate */
	ignoreleft = ((item[selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	ignoreright = ((item[selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);

	/* accept left/right keys as-is with repeat */
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	/* up backs up by one item */
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		selected = (selected + numitems - 1) % numitems;
		validate_selection(-1);
	}

	/* down advances by one item */
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		selected = (selected + 1) % numitems;
		validate_selection(1);
	}

	/* page up backs up by visitems */
	if (exclusive_input_pressed(IPT_UI_PAGE_UP, 6))
	{
		selected -= visitems - 1;
		validate_selection(1);
	}

	/* page down advances by visitems */
	if (exclusive_input_pressed(IPT_UI_PAGE_DOWN, 6))
	{
		selected += visitems - 1;
		validate_selection(-1);
	}

	/* home goes to the start */
	if (exclusive_input_pressed(IPT_UI_HOME, 0))
	{
		selected = 0;
		validate_selection(1);
	}

	/* end goes to the last */
	if (exclusive_input_pressed(IPT_UI_END, 0))
	{
		selected = numitems - 1;
		validate_selection(-1);
	}

	/* pause enables/disables pause */
	if (!ignorepause && exclusive_input_pressed(IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	/* handle a toggle cheats request */
	if (ui_input_pressed_repeat(machine(), IPT_UI_TOGGLE_CHEAT, 0))
		machine().cheat().set_enable(!machine().cheat().enabled());

	/* see if any other UI keys are pressed */
	if (menu_event.iptkey == IPT_INVALID)
		for (code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft) || (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;
			if (exclusive_input_pressed(code, 0))
				break;
		}
}


/*-------------------------------------------------
    validate_selection - validate the
    current selection and ensure it is on a
    correct item
-------------------------------------------------*/

void ui_menu::validate_selection(int scandir)
{
	/* clamp to be in range */
	if (selected < 0)
		selected = 0;
	else if (selected >= numitems)
		selected = numitems - 1;

	/* skip past unselectable items */
	while (!item[selected].is_selectable())
		selected = (selected + numitems + scandir) % numitems;
}



/*-------------------------------------------------
    clear_free_list - clear out anything
    accumulated in the free list
-------------------------------------------------*/

void ui_menu::clear_free_list(running_machine &machine)
{
	while (menu_free != NULL)
	{
		ui_menu *menu = menu_free;
		menu_free = menu->parent;
		auto_free(machine, menu);
	}
}



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    ui_menu::stack_reset - reset the menu stack
-------------------------------------------------*/

void ui_menu::stack_reset(running_machine &machine)
{
	while (menu_stack != NULL)
		ui_menu::stack_pop(machine);
}


/*-------------------------------------------------
    stack_push - push a new menu onto the
    stack
-------------------------------------------------*/

void ui_menu::stack_push(ui_menu *menu)
{
	menu->parent = menu_stack;
	menu_stack = menu;
	menu->reset(UI_MENU_RESET_SELECT_FIRST);
	ui_input_reset(menu->machine());
}


/*-------------------------------------------------
    stack_pop - pop a menu from the stack
-------------------------------------------------*/

void ui_menu::stack_pop(running_machine &machine)
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


/*-------------------------------------------------
    ui_menu::stack_has_special_main_menu -
    check in the special main menu is in the stack
-------------------------------------------------*/

bool ui_menu::stack_has_special_main_menu()
{
	ui_menu *menu;

	for (menu = menu_stack; menu != NULL; menu = menu->parent)
		if (menu->is_special_main_menu())
			return true;

	return false;
}

void ui_menu::do_handle()
{
	if(!populated())
		populate();
	handle();
}


/***************************************************************************
    UI SYSTEM INTERACTION
***************************************************************************/

/*-------------------------------------------------
    ui_menu_ui_handler - displays the current menu
    and calls the menu handler
-------------------------------------------------*/

UINT32 ui_menu::ui_handler(running_machine &machine, render_container *container, UINT32 state)
{
	/* if we have no menus stacked up, start with the main menu */
	if (menu_stack == NULL)
		stack_push(auto_alloc_clear(machine, ui_menu_main(machine, container)));

	/* update the menu state */
	if (menu_stack != NULL)
		menu_stack->do_handle();

	/* clear up anything pending to be released */
	clear_free_list(machine);

	/* if the menus are to be hidden, return a cancel here */
	if ((ui_input_pressed(machine, IPT_UI_CONFIGURE) && !stack_has_special_main_menu()) || menu_stack == NULL)
		return UI_HANDLER_CANCEL;

	return 0;
}

/***************************************************************************
    MENU HELPERS
***************************************************************************/

/*-------------------------------------------------
    render_triangle - render a triangle that
    is used for up/down arrows and left/right
    indicators
-------------------------------------------------*/

void ui_menu::render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	int halfwidth = dest.width() / 2;
	int height = dest.height();
	int x, y;

	/* start with all-transparent */
	dest.fill(MAKE_ARGB(0x00,0x00,0x00,0x00));

	/* render from the tip to the bottom */
	for (y = 0; y < height; y++)
	{
		int linewidth = (y * (halfwidth - 1) + (height / 2)) * 255 * 2 / height;
		UINT32 *target = &dest.pix32(y, halfwidth);

		/* don't antialias if height < 12 */
		if (dest.height() < 12)
		{
			int pixels = (linewidth + 254) / 255;
			if (pixels % 2 == 0) pixels++;
			linewidth = pixels * 255;
		}

		/* loop while we still have data to generate */
		for (x = 0; linewidth > 0; x++)
		{
			int dalpha;

			/* first column we only consume one pixel */
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
