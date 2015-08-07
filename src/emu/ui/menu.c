// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/*********************************************************************

	ui/menu.c

	Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "rendutil.h"
#include "cheat.h"
#include "uiinput.h"
#include "ui/ui.h"
#include "ui/mainmenu.h"
#include "ui/cheatopt.h"
#include "mewui/utils.h"
#include "mewui/menu.c"



/***************************************************************************
	CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE  65536

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

//-------------------------------------------------
//  is_selectable - return TRUE if the given
//  item is selectable
//-------------------------------------------------

inline bool ui_menu_item::is_selectable() const
{
	return ((flags & (MENU_FLAG_MULTILINE | MENU_FLAG_DISABLE)) == 0 && strcmp(text, MENU_SEPARATOR_ITEM) != 0);
}


//-------------------------------------------------
//  exclusive_input_pressed - return TRUE if the
//  given key is pressed and we haven't already
//  reported a key
//-------------------------------------------------

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

//-------------------------------------------------
//  init - initialize the menu system
//-------------------------------------------------

void ui_menu::init(running_machine &machine)
{
	// initialize the menu stack
	ui_menu::stack_reset(machine);

	// create a texture for hilighting items
	hilight_bitmap = auto_bitmap_rgb32_alloc(machine, 256, 1);
	for (int x = 0; x < 256; x++)
	{
		int alpha = 0xff;
		if (x < 25) alpha = 0xff * x / 25;
		if (x > 256 - 25) alpha = 0xff * (255 - x) / 25;
		hilight_bitmap->pix32(0, x) = rgb_t(alpha,0xff,0xff,0xff);
	}
	hilight_texture = machine.render().texture_alloc();
	hilight_texture->set_bitmap(*hilight_bitmap, hilight_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for arrow icons
	arrow_texture = machine.render().texture_alloc(render_triangle);

	// initialize mewui
	init_mewui(machine);

	// add an exit callback to free memory
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(ui_menu::exit), &machine));
}


//-------------------------------------------------
//  exit - clean up after ourselves
//-------------------------------------------------

void ui_menu::exit(running_machine &machine)
{
	// free menus
	ui_menu::stack_reset(machine);
	ui_menu::clear_free_list(machine);

	// free textures
	machine.render().texture_free(hilight_texture);
	machine.render().texture_free(arrow_texture);
	machine.render().texture_free(snapx_texture);
	machine.render().texture_free(hilight_main_texture);
	machine.render().texture_free(bgrnd_texture);
	machine.render().texture_free(star_texture);

	for (int i = 0; i < MAX_ICONS_RENDER; i++)
		machine.render().texture_free(icons_texture[i]);

	for (int i = 0; i < MEWUI_TOOLBAR_BUTTONS; i++)
	{
		machine.render().texture_free(sw_toolbar_texture[i]);
		machine.render().texture_free(toolbar_texture[i]);
	}
}



/***************************************************************************
	CORE MENU MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  ui_menu - menu constructor
//-------------------------------------------------

ui_menu::ui_menu(running_machine &machine, render_container *_container) : m_machine(machine)
{
	m_special_main_menu = false;
	container = _container;

	reset(UI_MENU_RESET_SELECT_FIRST);

	top_line = 0;
}


//-------------------------------------------------
//  ~ui_menu - menu destructor
//-------------------------------------------------

ui_menu::~ui_menu()
{
	// free the pools
	while (pool)
	{
		ui_menu_pool *ppool = pool;
		pool = pool->next;
		auto_free(machine(), ppool);
	}
}


//-------------------------------------------------
//  reset - free all items in the menu,
//  and all memory allocated from the memory pool
//-------------------------------------------------

void ui_menu::reset(ui_menu_reset_options options)
{
	// based on the reset option, set the reset info
	resetpos = 0;
	resetref = NULL;
	if (options == UI_MENU_RESET_REMEMBER_POSITION)
		resetpos = selected;
	else if (options == UI_MENU_RESET_REMEMBER_REF)
		resetref = item[selected].ref;

	// reset all the pools and the item.size() back to 0
	for (ui_menu_pool *ppool = pool; ppool != NULL; ppool = ppool->next)
		ppool->top = (UINT8 *)(ppool + 1);
	item.clear();
	visitems = 0;
	selected = 0;
	std::string backtext;
	strprintf(backtext, "Return to %s", emulator_info::get_capstartgamenoun());

	// add an item to return
	if (parent == NULL)
		item_append(backtext.c_str(), NULL, 0, NULL);
	else if (parent->is_special_main_menu())
		item_append("Exit", NULL, 0, NULL);
	else
		item_append("Return to Previous Menu", NULL, 0, NULL);
}


//-------------------------------------------------
//  is_special_main_menu - returns whether the
//  menu has special needs
//-------------------------------------------------

bool ui_menu::is_special_main_menu() const
{
	return m_special_main_menu;
}


//-------------------------------------------------
//  set_special_main_menu - set whether the
//  menu has special needs
//-------------------------------------------------

void ui_menu::set_special_main_menu(bool special)
{
	m_special_main_menu = special;
}


//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void ui_menu::item_append(const char *text, const char *subtext, UINT32 flags, void *ref)
{
	// only allow multiline as the first item
	if ((flags & MENU_FLAG_MULTILINE) != 0)
		assert(item.size() == 1);

	// only allow a single multi-line item
	else if (item.size() >= 2)
		assert((item[0].flags & MENU_FLAG_MULTILINE) == 0);

	// allocate a new item and populate it
	ui_menu_item pitem;
	pitem.text = (text != NULL) ? pool_strdup(text) : NULL;
	pitem.subtext = (subtext != NULL) ? pool_strdup(subtext) : NULL;
	pitem.flags = flags;
	pitem.ref = ref;

	// append to array
	int index = item.size();
	if (!item.empty())
	{
		item.insert(item.end() - 1, pitem);
		--index;
	}
	else
		item.push_back(pitem);

	// update the selection if we need to
	if (resetpos == index || (resetref != NULL && resetref == ref))
		selected = index;
	if (resetpos == item.size() - 1)
		selected = item.size() - 1;
}


//-------------------------------------------------
//  process - process a menu, drawing it
//  and returning any interesting events
//-------------------------------------------------

const ui_menu_event *ui_menu::process(UINT32 flags)
{
	// reset the menu_event
	menu_event.iptkey = IPT_INVALID;

	// first make sure our selection is valid
//	if (!(flags & UI_MENU_PROCESS_NOINPUT))
		validate_selection(1);

	// draw the menu
	if (item.size() > 1 && (item[0].flags & MENU_FLAG_MULTILINE) != 0)
		draw_text_box();
	else if ((item[0].flags & MENU_FLAG_MEWUI ) != 0 || (item[0].flags & MENU_FLAG_MEWUI_SWLIST ) != 0)
		draw_select_game(flags & UI_MENU_PROCESS_NOINPUT);
	else if ((item[0].flags & MENU_FLAG_MEWUI_PALETTE ) != 0)
		draw_palette_menu();
	else
		draw(flags & UI_MENU_PROCESS_CUSTOM_ONLY, flags & UI_MENU_PROCESS_NOIMAGE, flags & UI_MENU_PROCESS_NOINPUT);

	// process input
	if (!(flags & UI_MENU_PROCESS_NOKEYS) && !(flags & UI_MENU_PROCESS_NOINPUT))
	{
		// read events
		if ((item[0].flags & MENU_FLAG_MEWUI ) != 0 || (item[0].flags & MENU_FLAG_MEWUI_SWLIST ) != 0)
			handle_main_events(flags);
		else
			handle_events(flags);

		// handle the keys if we don't already have an menu_event
		if (menu_event.iptkey == IPT_INVALID)
		{
			if ((item[0].flags & MENU_FLAG_MEWUI ) != 0 || (item[0].flags & MENU_FLAG_MEWUI_SWLIST ) != 0)
				handle_main_keys(flags);
			else
				handle_keys(flags);
		}
	}

	// update the selected item in the menu_event
	if (menu_event.iptkey != IPT_INVALID && selected >= 0 && selected < item.size())
	{
		menu_event.itemref = item[selected].ref;
		return &menu_event;
	}
	return NULL;
}


//-------------------------------------------------
//  m_pool_alloc - allocate temporary memory
//  from the menu's memory pool
//-------------------------------------------------

void *ui_menu::m_pool_alloc(size_t size)
{
	ui_menu_pool *ppool;

	assert(size < UI_MENU_POOL_SIZE);

	// find a pool with enough room
	for (ppool = pool; ppool != NULL; ppool = ppool->next)
		if (ppool->end - ppool->top >= size)
		{
			void *result = ppool->top;
			ppool->top += size;
			return result;
		}

	// allocate a new pool
	ppool = (ui_menu_pool *)auto_alloc_array_clear(machine(), UINT8, sizeof(*ppool) + UI_MENU_POOL_SIZE);

	// wire it up
	ppool->next = pool;
	pool = ppool;
	ppool->top = (UINT8 *)(ppool + 1);
	ppool->end = ppool->top + UI_MENU_POOL_SIZE;
	return m_pool_alloc(size);
}


//-------------------------------------------------
//  pool_strdup - make a temporary string
//  copy in the menu's memory pool
//-------------------------------------------------

const char *ui_menu::pool_strdup(const char *string)
{
	return strcpy((char *)m_pool_alloc(strlen(string) + 1), string);
}


//-------------------------------------------------
//  get_selection - retrieves the index
//  of the currently selected menu item
//-------------------------------------------------

void *ui_menu::get_selection()
{
	return (selected >= 0 && selected < item.size()) ? item[selected].ref : NULL;
}


//-------------------------------------------------
//  set_selection - changes the index
//  of the currently selected menu item
//-------------------------------------------------

void ui_menu::set_selection(void *selected_itemref)
{
	selected = -1;
	for (int itemnum = 0; itemnum < item.size(); itemnum++)
		if (item[itemnum].ref == selected_itemref)
		{
			selected = itemnum;
			break;
		}
}



/***************************************************************************
	INTERNAL MENU PROCESSING
***************************************************************************/

//-------------------------------------------------
//  draw - draw a menu
//-------------------------------------------------

void ui_menu::draw(bool customonly, bool noimage, bool noinput)
{
	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;

	int selected_subitem_too_big = FALSE;
	int itemnum, linenum;
	bool mouse_hit, mouse_button;
	float mouse_x = -1, mouse_y = -1;
	bool history_flag = ((item[0].flags & MENU_FLAG_MEWUI_HISTORY) != 0);

	if (machine().options().use_background_image() && machine().options().system() == NULL && bgrnd_bitmap->valid() && !noimage)
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, ARGB_WHITE, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// compute the width and height of the full menu
	float visible_width = 0;
	float visible_main_menu_height = 0;
	for (itemnum = 0; itemnum < item.size(); itemnum++)
	{
		const ui_menu_item &pitem = item[itemnum];
		float total_width;

		// compute width of left hand side
		total_width = gutter_width + machine().ui().get_string_width(pitem.text) + gutter_width;

		// add in width of right hand side
		if (pitem.subtext)
			total_width += 2.0f * gutter_width + machine().ui().get_string_width(pitem.subtext);

		// track the maximum
		if (total_width > visible_width)
			visible_width = total_width;

		// track the height as well
		visible_main_menu_height += line_height;
	}

	// account for extra space at the top and bottom
	float visible_extra_menu_height = customtop + custombottom;

	// add a little bit of slop for rounding
	visible_width += 0.01f;
	visible_main_menu_height += 0.01f;

	// if we are too wide or too tall, clamp it down
	if (visible_width + 2.0f * UI_BOX_LR_BORDER > 1.0f)
		visible_width = 1.0f - 2.0f * UI_BOX_LR_BORDER;

	// if the menu and extra menu won't fit, take away part of the regular menu, it will scroll
	if (visible_main_menu_height + visible_extra_menu_height + 2.0f * UI_BOX_TB_BORDER > 1.0f)
		visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;

	int visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)visible_lines * line_height;

	// compute top/left of inner menu area by centering
	float visible_left = (1.0f - visible_width) * 0.5f;
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += customtop;

	// first add us a box
	float x1 = visible_left - UI_BOX_LR_BORDER;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	if (!customonly)
		machine().ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// determine the first visible line based on the current selection
	int top_line = selected - visible_lines / 2;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= item.size())
		top_line = item.size() - visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	// locate mouse
	mouse_hit = false;
	mouse_button = false;
	if (!customonly && !noinput)
	{
		INT32 mouse_target_x, mouse_target_y;
		render_target *mouse_target = ui_input_find_mouse(machine(), &mouse_target_x, &mouse_target_y, &mouse_button);
		if (mouse_target != NULL)
			if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
				mouse_hit = true;
	}

	// loop over visible lines
	hover = item.size() + 1;
	float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
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
			float line_y0 = line_y;
			float line_y1 = line_y + line_height;

			// set the hover if this is our item
			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
				hover = itemnum;

			// if we're selected, draw with a different background
			if (itemnum == selected && (pitem.flags & MENU_FLAG_MEWUI_HISTORY) == 0)
			{
				fgcolor = UI_SELECTED_COLOR;
				bgcolor = UI_SELECTED_BG_COLOR;
				fgcolor2 = UI_SELECTED_COLOR;
				fgcolor3 = UI_SELECTED_COLOR;
			}

			// else if the mouse is over this item, draw with a different background
			else if (itemnum == hover && (((pitem.flags & MENU_FLAG_MEWUI_HISTORY) == 0) || (linenum == 0 && top_line != 0)
			         || (linenum == visible_lines - 1 && itemnum != item.size() - 1)))
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor2 = UI_MOUSEOVER_COLOR;
				fgcolor3 = UI_MOUSEOVER_COLOR;
			}

			// if we have some background hilighting to do, add a quad behind everything else
			if (bgcolor != UI_TEXT_BG_COLOR)
				highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);

			// if we're on the top line, display the up arrow
			if (linenum == 0 && top_line != 0)
			{
				draw_arrow(container,
									0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
									line_y + 0.25f * line_height,
									0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
									line_y + 0.75f * line_height,
									fgcolor,
									ROT0);
				if (hover == itemnum)
					hover = HOVER_ARROW_UP;
			}

			// if we're on the bottom line, display the down arrow
			else if (linenum == visible_lines - 1 && itemnum != item.size() - 1)
			{
				draw_arrow(container,
									0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
									line_y + 0.25f * line_height,
									0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
									line_y + 0.75f * line_height,
									fgcolor,
									ROT0 ^ ORIENTATION_FLIP_Y);
				if (hover == itemnum)
					hover = HOVER_ARROW_DOWN;
			}

			// if we're just a divider, draw a line
			else if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
				container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// draw the subitem left-justified
			else if (pitem.subtext == NULL && (pitem.flags & MENU_FLAG_MEWUI_HISTORY) != 0)
				machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				                              JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);

			// if we don't have a subitem, just draw the string centered
			else if (pitem.subtext == NULL)
				machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				                              JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, NULL, NULL);

			// otherwise, draw the item on the left and the subitem text on the right
			else
			{
				int subitem_invert = pitem.flags & MENU_FLAG_INVERT;
				const char *subitem_text = pitem.subtext;
				float item_width, subitem_width;

				// draw the left-side text
				machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				                              JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, &item_width, NULL);

				// give 2 spaces worth of padding
				item_width += 2.0f * gutter_width;

				// if the subitem doesn't fit here, display dots
				if (machine().ui().get_string_width(subitem_text) > effective_width - item_width)
				{
					subitem_text = "...";
					if (itemnum == selected)
						selected_subitem_too_big = TRUE;
				}

				// customize subitem text color
				if (!core_stricmp(subitem_text, "On"))
					fgcolor2 = rgb_t(0xff,0x00,0xff,0x00);

				if (!core_stricmp(subitem_text, "Off"))
					fgcolor2 = rgb_t(0xff,0xff,0x00,0x00);

				if (!core_stricmp(subitem_text, "Auto"))
					fgcolor2 = rgb_t(0xff,0xff,0xff,0x00);

				// draw the subitem right-justified
				machine().ui().draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
				                              JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor, &subitem_width, NULL);

				// apply arrows
				if (itemnum == selected && (pitem.flags & MENU_FLAG_LEFT_ARROW))
				{
					draw_arrow(container,
										effective_left + effective_width - subitem_width - gutter_width,
										line_y + 0.1f * line_height,
										effective_left + effective_width - subitem_width - gutter_width + lr_arrow_width,
										line_y + 0.9f * line_height,
										fgcolor,
										ROT90 ^ ORIENTATION_FLIP_X);
				}
				if (itemnum == selected && (pitem.flags & MENU_FLAG_RIGHT_ARROW))
				{
					draw_arrow(container,
										effective_left + effective_width + gutter_width - lr_arrow_width,
										line_y + 0.1f * line_height,
										effective_left + effective_width + gutter_width,
										line_y + 0.9f * line_height,
										fgcolor,
										ROT90);
				}
			}
		}

	// if the selected subitem is too big, display it in a separate offset box
	if (selected_subitem_too_big)
	{
		const ui_menu_item &pitem = item[selected];
		int subitem_invert = pitem.flags & MENU_FLAG_INVERT;
		linenum = selected - top_line;
		float line_y = visible_top + (float)linenum * line_height;
		float target_width, target_height;
		float target_x, target_y;

		// compute the multi-line target width/height
		machine().ui().draw_text_full(container, pitem.subtext, 0, 0, visible_width * 0.75f,
		                              JUSTIFY_RIGHT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);

		// determine the target location
		target_x = visible_left + visible_width - target_width - UI_BOX_LR_BORDER;
		target_y = line_y + line_height + UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > visible_main_menu_height)
			target_y = line_y - target_height - UI_BOX_TB_BORDER;

		// add a box around that
		machine().ui().draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
							target_y - UI_BOX_TB_BORDER,
							target_x + target_width + UI_BOX_LR_BORDER,
										 target_y + target_height + UI_BOX_TB_BORDER,
										 subitem_invert ? UI_SELECTED_BG_COLOR : UI_BACKGROUND_COLOR);
		machine().ui().draw_text_full(container, pitem.subtext, target_x, target_y, target_width,
		                              JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : NULL, customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != item.size());
	if (history_flag && (top_line + visible_lines >= item.size()))
		selected = item.size() - 1;
}

void ui_menu::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
}

//-------------------------------------------------
//  draw_text_box - draw a multiline
//  word-wrapped text box with a menu item at the
//  bottom
//-------------------------------------------------

void ui_menu::draw_text_box()
{
	const char *text = item[0].text;
	const char *backtext = item[1].text;
	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width;
	float target_width, target_height, prior_width;
	float target_x, target_y;

	// compute the multi-line target width/height
	machine().ui().draw_text_full(container, text, 0, 0, 1.0f - 2.0f * UI_BOX_LR_BORDER - 2.0f * gutter_width,
	                              JUSTIFY_LEFT, WRAP_WORD, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &target_width, &target_height);
	target_height += 2.0f * line_height;
	if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
		target_height = floorf((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;

	// maximum against "return to prior menu" text
	prior_width = machine().ui().get_string_width(backtext) + 2.0f * gutter_width;
	target_width = MAX(target_width, prior_width);

	// determine the target location
	target_x = 0.5f - 0.5f * target_width;
	target_y = 0.5f - 0.5f * target_height;

	// make sure we stay on-screen
	if (target_x < UI_BOX_LR_BORDER + gutter_width)
		target_x = UI_BOX_LR_BORDER + gutter_width;
	if (target_x + target_width + gutter_width + UI_BOX_LR_BORDER > 1.0f)
		target_x = 1.0f - UI_BOX_LR_BORDER - gutter_width - target_width;
	if (target_y < UI_BOX_TB_BORDER)
		target_y = UI_BOX_TB_BORDER;
	if (target_y + target_height + UI_BOX_TB_BORDER > 1.0f)
		target_y = 1.0f - UI_BOX_TB_BORDER - target_height;

	// add a box around that
	machine().ui().draw_outlined_box(container, target_x - UI_BOX_LR_BORDER - gutter_width,
                                     target_y - UI_BOX_TB_BORDER,
                                     target_x + target_width + gutter_width + UI_BOX_LR_BORDER,
                                     target_y + target_height + UI_BOX_TB_BORDER,
                                     (item[0].flags & MENU_FLAG_REDTEXT) ?  UI_RED_COLOR : UI_BACKGROUND_COLOR);
	machine().ui().draw_text_full(container, text, target_x, target_y, target_width,
	                              JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, NULL, NULL);

	// draw the "return to prior menu" text with a hilight behind it
	highlight(container,
              target_x + 0.5f * UI_LINE_WIDTH,
              target_y + target_height - line_height,
              target_x + target_width - 0.5f * UI_LINE_WIDTH,
              target_y + target_height,
              UI_SELECTED_BG_COLOR);
	machine().ui().draw_text_full(container, backtext, target_x, target_y + target_height - line_height, target_width,
	                              JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, NULL, NULL);

	// artificially set the hover to the last item so a double-click exits
	hover = item.size() - 1;
}


//-------------------------------------------------
//  handle_events - generically handle
//  input events for a menu
//-------------------------------------------------

void ui_menu::handle_events(UINT32 flags)
{
	int stop = FALSE;
	ui_event local_menu_event;

	// loop while we have interesting events
	while (!stop && ui_input_pop_event(machine(), &local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
			// if we are hovering over a valid item, select it with a single click
			case UI_EVENT_MOUSE_DOWN:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
				{
					if (hover >= 0 && hover < item.size())
						selected = hover;
					else if (hover == HOVER_ARROW_UP)
					{
						selected -= visitems - 1;
						validate_selection(1);
					}
					else if (hover == HOVER_ARROW_DOWN)
					{
						selected += visitems - 1;
						validate_selection(1);
					}
				}
				break;

			// if we are hovering over a valid item, fake a UI_SELECT with a double-click
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
				{
					if (hover >= 0 && hover < item.size())
					{
						selected = hover;
						if (local_menu_event.event_type == UI_EVENT_MOUSE_DOUBLE_CLICK)
						{
							menu_event.iptkey = IPT_UI_SELECT;
							if (selected == item.size() - 1)
							{
								menu_event.iptkey = IPT_UI_CANCEL;
								ui_menu::stack_pop(machine());
							}
						}
						stop = TRUE;
					}
				}
				break;

			// caught scroll event
			case UI_EVENT_MOUSE_SCROLL:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
				{
					if (local_menu_event.zdelta > 0)
					{
						if (selected >= visible_items || selected == 0 || ui_error)
							break;
						selected -= local_menu_event.num_lines;
						if (selected < top_line + (top_line != 0))
							top_line -= local_menu_event.num_lines;
					}
					else
					{
						if (selected >= visible_items - 1 || ui_error)
							break;
						selected += local_menu_event.num_lines;
						if (selected > visible_items - 1)
							selected = visible_items - 1;
						if (selected >= top_line + visitems + (top_line != 0))
							top_line += local_menu_event.num_lines;
					}
				}
				break;

			// translate CHAR events into specials
			case UI_EVENT_CHAR:
				menu_event.iptkey = IPT_SPECIAL;
				menu_event.unichar = local_menu_event.ch;
				stop = TRUE;
				break;

			// ignore everything else
			default:
				break;
		}
	}
}


//-------------------------------------------------
//  handle_keys - generically handle
//  keys for a menu
//-------------------------------------------------

void ui_menu::handle_keys(UINT32 flags)
{
	int ignorepause = ui_menu::stack_has_special_main_menu();
	int ignoreright = FALSE;
	int ignoreleft = FALSE;
	int code;

	// bail if no items
	if (item.empty())
		return;
	bool historyflag = ((item[0].flags & MENU_FLAG_MEWUI_HISTORY) != 0);


	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (selected == item.size() - 1)
		{
			menu_event.iptkey = IPT_UI_CANCEL;
			ui_menu::stack_pop(machine());
		}
		return;
	}

	// bail out
	if ((flags & UI_MENU_PROCESS_ONLYCHAR) != 0)
		return;

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		if (!menu_has_search_active())
			ui_menu::stack_pop(machine());
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	ignoreleft = ((item[selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	ignoreright = ((item[selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		if (historyflag && selected <= (visitems / 2))
			return;
		else if (historyflag && visitems == item.size())
		{
			selected = item.size() - 1;
			return;
		}
		else if (historyflag && selected == item.size() - 1)
			selected = (item.size() - 1) - (visitems / 2);

		selected = (selected + item.size() - 1) % item.size();
		validate_selection(-1);
	}

	// down advances by one item
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		if (historyflag && (selected < visitems / 2))
			selected = visitems / 2;
		else if (historyflag && (selected + (visitems / 2) >= item.size()))
		{
			selected = item.size() - 1;
			return;
		}

		selected = (selected + 1) % item.size();
		validate_selection(1);
	}

	// page up backs up by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_UP, 6))
	{
		selected -= visitems - 1;
		validate_selection(1);
	}

	// page down advances by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_DOWN, 6))
	{
		selected += visitems - 1;
		validate_selection(-1);
	}

	// home goes to the start
	if (exclusive_input_pressed(IPT_UI_HOME, 0))
	{
		selected = 0;
		validate_selection(1);
	}

	// end goes to the last
	if (exclusive_input_pressed(IPT_UI_END, 0))
	{
		selected = item.size() - 1;
		validate_selection(-1);
	}

	// pause enables/disables pause
	if (!ignorepause && exclusive_input_pressed(IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// handle a toggle cheats request
	if (ui_input_pressed_repeat(machine(), IPT_UI_TOGGLE_CHEAT, 0))
		machine().cheat().set_enable(!machine().cheat().enabled());

	// see if any other UI keys are pressed
	if (menu_event.iptkey == IPT_INVALID)
		for (code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft) || (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;
			if (exclusive_input_pressed(code, 0))
				break;
		}
}


//-------------------------------------------------
//  validate_selection - validate the
//  current selection and ensure it is on a
//  correct item
//-------------------------------------------------

void ui_menu::validate_selection(int scandir)
{
	// clamp to be in range
	if (selected < 0)
		selected = 0;
	else if (selected >= item.size())
		selected = item.size() - 1;

	// skip past unselectable items
	while (!item[selected].is_selectable())
		selected = (selected + item.size() + scandir) % item.size();
}



//-------------------------------------------------
//  clear_free_list - clear out anything
//  accumulated in the free list
//-------------------------------------------------

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

//-------------------------------------------------
//  ui_menu::stack_reset - reset the menu stack
//-------------------------------------------------

void ui_menu::stack_reset(running_machine &machine)
{
	while (menu_stack != NULL)
		ui_menu::stack_pop(machine);
}


//-------------------------------------------------
//  stack_push - push a new menu onto the
//  stack
//-------------------------------------------------

void ui_menu::stack_push(ui_menu *menu)
{
	menu->parent = menu_stack;
	menu_stack = menu;
	menu->reset(UI_MENU_RESET_SELECT_FIRST);
	ui_input_reset(menu->machine());
}


//-------------------------------------------------
//  stack_pop - pop a menu from the stack
//-------------------------------------------------

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


//-------------------------------------------------
//  ui_menu::stack_has_special_main_menu -
//  check in the special main menu is in the stack
//-------------------------------------------------

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
	if(item.size() < 2)
		populate();
	handle();
}


/***************************************************************************
	UI SYSTEM INTERACTION
***************************************************************************/

//-------------------------------------------------
//  ui_menu_ui_handler - displays the current menu
//  and calls the menu handler
//-------------------------------------------------

UINT32 ui_menu::ui_handler(running_machine &machine, render_container *container, UINT32 state)
{
	// if we have no menus stacked up, start with the main menu
	if (menu_stack == NULL)
		stack_push(auto_alloc_clear(machine, ui_menu_main(machine, container)));

	// update the menu state
	if (menu_stack != NULL)
		menu_stack->do_handle();

	// clear up anything pending to be released
	clear_free_list(machine);

	// if the menus are to be hidden, return a cancel here
	if (machine.ui().is_menu_active() && ((ui_input_pressed(machine, IPT_UI_CONFIGURE) && !stack_has_special_main_menu()) || menu_stack == NULL))
		return UI_HANDLER_CANCEL;

	return 0;
}

/***************************************************************************
	MENU HELPERS
***************************************************************************/

//-------------------------------------------------
//  render_triangle - render a triangle that
//  is used for up/down arrows and left/right
//  indicators
//-------------------------------------------------

void ui_menu::render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
{
	int halfwidth = dest.width() / 2;
	int height = dest.height();
	int x, y;

	// start with all-transparent
	dest.fill(rgb_t(0x00,0x00,0x00,0x00));

	// render from the tip to the bottom
	for (y = 0; y < height; y++)
	{
		int linewidth = (y * (halfwidth - 1) + (height / 2)) * 255 * 2 / height;
		UINT32 *target = &dest.pix32(y, halfwidth);

		// don't antialias if height < 12
		if (dest.height() < 12)
		{
			int pixels = (linewidth + 254) / 255;
			if (pixels % 2 == 0) pixels++;
			linewidth = pixels * 255;
		}

		// loop while we still have data to generate
		for (x = 0; linewidth > 0; x++)
		{
			int dalpha;

			// first column we only consume one pixel
			if (x == 0)
			{
				dalpha = MIN(0xff, linewidth);
				target[x] = rgb_t(dalpha,0xff,0xff,0xff);
			}

			// remaining columns consume two pixels, one on each side
			else
			{
				dalpha = MIN(0x1fe, linewidth);
				target[x] = target[-x] = rgb_t(dalpha/2,0xff,0xff,0xff);
			}

			// account for the weight we consumed */
			linewidth -= dalpha;
		}
	}
}


//-------------------------------------------------
//  highlight
//-------------------------------------------------

void ui_menu::highlight(render_container *container, float x0, float y0, float x1, float y1, rgb_t bgcolor)
{
	container->add_quad(x0, y0, x1, y1, bgcolor, hilight_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
}


//-------------------------------------------------
//  draw_arrow
//-------------------------------------------------

void ui_menu::draw_arrow(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation)
{
	container->add_quad(x0, y0, x1, y1, fgcolor, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(orientation));
}
