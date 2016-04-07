// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Maurizio Petrarota
/*********************************************************************

    ui/menu.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"
#include "rendutil.h"
#include "cheat.h"
#include "uiinput.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/mainmenu.h"
#include "ui/utils.h"
#include "ui/defimg.h"
#include "ui/starimg.h"
#include "ui/optsmenu.h"
#include "ui/datfile.h"
#include "rendfont.h"
#include "ui/custmenu.h"
#include "ui/icorender.h"
#include "ui/toolbar.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE  65536
#define MAX_ICONS_RENDER   40

struct ui_arts_info
{
	const char *title, *path;
};

static const ui_arts_info arts_info[] =
{
	{ __("Snapshots"), OPTION_SNAPSHOT_DIRECTORY },
	{ __("Cabinets"), OPTION_CABINETS_PATH },
	{ __("Control Panels"), OPTION_CPANELS_PATH },
	{ __("PCBs"), OPTION_PCBS_PATH },
	{ __("Flyers"), OPTION_FLYERS_PATH },
	{ __("Titles"), OPTION_TITLES_PATH },
	{ __("Ends"), OPTION_ENDS_PATH },
	{ __("Artwork Preview"), OPTION_ARTPREV_PATH },
	{ __("Bosses"), OPTION_BOSSES_PATH },
	{ __("Logos"), OPTION_LOGOS_PATH },
	{ __("Versus"), OPTION_VERSUS_PATH },
	{ __("Game Over"), OPTION_GAMEOVER_PATH },
	{ __("HowTo"), OPTION_HOWTO_PATH },
	{ __("Scores"), OPTION_SCORES_PATH },
	{ __("Select"), OPTION_SELECT_PATH },
	{ __("Marquees"), OPTION_MARQUEES_PATH },
	{ __("Covers"), OPTION_COVER_PATH },
	{ nullptr }
};

static const char *hover_msg[] = {
	__("Add or remove favorites"),
	__("Export displayed list to file"),
	__("Show DATs view"),
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

ui_menu *ui_menu::menu_stack;
ui_menu *ui_menu::menu_free;
std::unique_ptr<bitmap_rgb32> ui_menu::hilight_bitmap;
render_texture *ui_menu::hilight_texture;
render_texture *ui_menu::arrow_texture;
render_texture *ui_menu::snapx_texture;
render_texture *ui_menu::hilight_main_texture;
render_texture *ui_menu::bgrnd_texture;
render_texture *ui_menu::star_texture;
render_texture *ui_menu::toolbar_texture[UI_TOOLBAR_BUTTONS];
render_texture *ui_menu::sw_toolbar_texture[UI_TOOLBAR_BUTTONS];
render_texture *ui_menu::icons_texture[MAX_ICONS_RENDER];
std::unique_ptr<bitmap_argb32> ui_menu::snapx_bitmap;
std::unique_ptr<bitmap_argb32> ui_menu::no_avail_bitmap;
std::unique_ptr<bitmap_argb32> ui_menu::star_bitmap;
std::unique_ptr<bitmap_argb32> ui_menu::bgrnd_bitmap;
bitmap_argb32 *ui_menu::icons_bitmap[MAX_ICONS_RENDER];
std::unique_ptr<bitmap_rgb32> ui_menu::hilight_main_bitmap;
bitmap_argb32 *ui_menu::toolbar_bitmap[UI_TOOLBAR_BUTTONS];
bitmap_argb32 *ui_menu::sw_toolbar_bitmap[UI_TOOLBAR_BUTTONS];

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
	if (menu_event.iptkey == IPT_INVALID && machine().ui_input().pressed_repeat(key, repeat))
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
	hilight_bitmap = std::make_unique<bitmap_rgb32>(256, 1);
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

	// initialize ui
	init_ui(machine);

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
	render_manager &mre = machine.render();
	mre.texture_free(hilight_texture);
	mre.texture_free(arrow_texture);
	mre.texture_free(snapx_texture);
	mre.texture_free(hilight_main_texture);
	mre.texture_free(bgrnd_texture);
	mre.texture_free(star_texture);

	for (auto & elem : icons_texture)
		mre.texture_free(elem);

	for (int i = 0; i < UI_TOOLBAR_BUTTONS; i++)
	{
		mre.texture_free(sw_toolbar_texture[i]);
		mre.texture_free(toolbar_texture[i]);
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
		global_free(ppool);
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
	resetref = nullptr;
	if (options == UI_MENU_RESET_REMEMBER_POSITION)
		resetpos = selected;
	else if (options == UI_MENU_RESET_REMEMBER_REF)
		resetref = item[selected].ref;

	// reset all the pools and the item.size() back to 0
	for (ui_menu_pool *ppool = pool; ppool != nullptr; ppool = ppool->next)
		ppool->top = (UINT8 *)(ppool + 1);
	item.clear();
	visitems = 0;
	selected = 0;

	// add an item to return
	if (parent == nullptr)
		item_append(_("Return to Machine"), nullptr, 0, nullptr);
	else if (parent->is_special_main_menu())
	{
		if (strcmp(machine().options().ui(), "simple") == 0)
			item_append(_("Exit"), nullptr, 0, nullptr);
		else
			item_append(_("Exit"), nullptr, MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, nullptr);
	}
	else
	{
		if (strcmp(machine().options().ui(), "simple") != 0 && ui_menu::stack_has_special_main_menu())
			item_append(_("Return to Previous Menu"), nullptr, MENU_FLAG_UI | MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW, nullptr);
		else
			item_append(_("Return to Previous Menu"), nullptr, 0, nullptr);
	}

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
	pitem.text = (text != nullptr) ? pool_strdup(text) : nullptr;
	pitem.subtext = (subtext != nullptr) ? pool_strdup(subtext) : nullptr;
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
	if (resetpos == index || (resetref != nullptr && resetref == ref))
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
//  if (!(flags & UI_MENU_PROCESS_NOINPUT))
		validate_selection(1);

	// draw the menu
	if (item.size() > 1 && (item[0].flags & MENU_FLAG_MULTILINE) != 0)
		draw_text_box();
	else if ((item[0].flags & MENU_FLAG_UI ) != 0 || (item[0].flags & MENU_FLAG_UI_SWLIST ) != 0)
		draw_select_game(flags & UI_MENU_PROCESS_NOINPUT);
	else if ((item[0].flags & MENU_FLAG_UI_PALETTE ) != 0)
		draw_palette_menu();
	else if ((item[0].flags & MENU_FLAG_UI_DATS) != 0)
		draw_dats_menu();
	else
		draw(flags & UI_MENU_PROCESS_CUSTOM_ONLY, flags & UI_MENU_PROCESS_NOIMAGE, flags & UI_MENU_PROCESS_NOINPUT);

	// process input
	if (!(flags & UI_MENU_PROCESS_NOKEYS) && !(flags & UI_MENU_PROCESS_NOINPUT))
	{
		// read events
		if ((item[0].flags & MENU_FLAG_UI ) != 0 || (item[0].flags & MENU_FLAG_UI_SWLIST ) != 0)
			handle_main_events(flags);
		else
			handle_events(flags);

		// handle the keys if we don't already have an menu_event
		if (menu_event.iptkey == IPT_INVALID)
		{
			if ((item[0].flags & MENU_FLAG_UI ) != 0 || (item[0].flags & MENU_FLAG_UI_SWLIST ) != 0)
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
	return nullptr;
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
	for (ppool = pool; ppool != nullptr; ppool = ppool->next)
		if (ppool->end - ppool->top >= size)
		{
			void *result = ppool->top;
			ppool->top += size;
			return result;
		}

	// allocate a new pool
	ppool = (ui_menu_pool *)global_alloc_array_clear<UINT8>(sizeof(*ppool) + UI_MENU_POOL_SIZE);

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
	return (selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr;
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
	// first draw the FPS counter
	if (machine().ui().show_fps_counter())
	{
		machine().ui().draw_text_full(container, machine().video().speed_text().c_str(), 0.0f, 0.0f, 1.0f,
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_OPAQUE, ARGB_WHITE, ARGB_BLACK, nullptr, nullptr);
	}

	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;

	bool selected_subitem_too_big = false;
	int itemnum, linenum;
	bool mouse_hit, mouse_button;
	float mouse_x = -1, mouse_y = -1;

	if (machine().ui().options().use_background_image() && &machine().system() == &GAME_NAME(___empty) && bgrnd_bitmap->valid() && !noimage)
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

	visible_lines = floor(visible_main_menu_height / line_height);
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

	if (top_line < 0 || selected == 0)
		top_line = 0;
	if (top_line > item.size() - visible_lines || selected == (item.size() - 1))
		top_line = item.size() - visible_lines;

	bool show_top_arrow = false;
	bool show_bottom_arrow = false;

	// if scrolling, show arrows
	if (item.size() > visible_lines) {
		if (top_line > 0)
			show_top_arrow = true;
		if (top_line != item.size() - visible_lines)
			show_bottom_arrow = true;
	}

	// set the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - show_top_arrow - show_bottom_arrow;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	// locate mouse
	mouse_hit = false;
	mouse_button = false;
	if (!customonly && !noinput)
	{
		INT32 mouse_target_x, mouse_target_y;
		render_target *mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &mouse_button);
		if (mouse_target != nullptr)
			if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
				mouse_hit = true;
	}

	// loop over visible lines
	hover = item.size() + 1;
	float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
	if (!customonly)
	{
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
			if (itemnum == selected)
			{
				fgcolor = UI_SELECTED_COLOR;
				bgcolor = UI_SELECTED_BG_COLOR;
				fgcolor2 = UI_SELECTED_COLOR;
				fgcolor3 = UI_SELECTED_COLOR;
			}

			// else if the mouse is over this item, draw with a different background
			else if (itemnum == hover)
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
			if (linenum == 0 && show_top_arrow)
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
			else if (linenum == visible_lines - 1 && show_bottom_arrow)
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

			// if we don't have a subitem, just draw the string centered
			else if (pitem.subtext == nullptr)
			{
				if (pitem.flags & MENU_FLAG_UI_HEADING)
				{
					float heading_width = machine().ui().get_string_width(itemtext);
					container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + ((visible_width - heading_width) / 2) - UI_BOX_LR_BORDER, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container->add_line(visible_left + visible_width - ((visible_width - heading_width) / 2) + UI_BOX_LR_BORDER, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
					JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);
			}

			// otherwise, draw the item on the left and the subitem text on the right
			else
			{
				int subitem_invert = pitem.flags & MENU_FLAG_INVERT;
				const char *subitem_text = pitem.subtext;
				float item_width, subitem_width;

				// draw the left-side text
				machine().ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
							JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, &item_width, nullptr);

				// give 2 spaces worth of padding
				item_width += 2.0f * gutter_width;

				// if the subitem doesn't fit here, display dots
				if (machine().ui().get_string_width(subitem_text) > effective_width - item_width)
				{
					subitem_text = "...";
					if (itemnum == selected)
						selected_subitem_too_big = true;
				}

				// customize subitem text color
				if (!core_stricmp(subitem_text, _("On")))
					fgcolor2 = rgb_t(0xff,0x00,0xff,0x00);

				if (!core_stricmp(subitem_text, _("Off")))
					fgcolor2 = rgb_t(0xff,0xff,0x00,0x00);

				if (!core_stricmp(subitem_text, _("Auto")))
					fgcolor2 = rgb_t(0xff,0xff,0xff,0x00);

				// draw the subitem right-justified
				machine().ui().draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
							JUSTIFY_RIGHT, WRAP_TRUNCATE, DRAW_NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor, &subitem_width, nullptr);

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
					JUSTIFY_RIGHT, WRAP_WORD, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, nullptr, nullptr);
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr, customtop, custombottom, x1, y1, x2, y2);
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
				JUSTIFY_LEFT, WRAP_WORD, DRAW_NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// draw the "return to prior menu" text with a hilight behind it
	highlight(container,
				target_x + 0.5f * UI_LINE_WIDTH,
				target_y + target_height - line_height,
				target_x + target_width - 0.5f * UI_LINE_WIDTH,
				target_y + target_height,
				UI_SELECTED_BG_COLOR);
	machine().ui().draw_text_full(container, backtext, target_x, target_y + target_height - line_height, target_width,
				JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, nullptr, nullptr);

	// artificially set the hover to the last item so a double-click exits
	hover = item.size() - 1;
}


//-------------------------------------------------
//  handle_events - generically handle
//  input events for a menu
//-------------------------------------------------

void ui_menu::handle_events(UINT32 flags)
{
	bool stop = false;
	ui_event local_menu_event;

	// loop while we have interesting events
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
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
						if ((flags & MENU_FLAG_UI_DATS) != 0)
						{
							top_line -= visitems - (top_line + visible_lines == item.size() - 1);
							return;
						}
						selected -= visitems;
						if (selected < 0)
							selected = 0;
						top_line -= visitems - (top_line + visible_lines == item.size() - 1);
					}
					else if (hover == HOVER_ARROW_DOWN)
					{
						if ((flags & MENU_FLAG_UI_DATS) != 0)
						{
							top_line += visible_lines - 2;
							return;
						}
						selected += visible_lines - 2 + (selected == 0);
						if (selected > item.size() - 1)
							selected = item.size() - 1;
						top_line += visible_lines - 2;
					}
				}
				break;

			// if we are hovering over a valid item, fake a UI_SELECT with a double-click
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0 && hover >= 0 && hover < item.size())
				{
					selected = hover;
					menu_event.iptkey = IPT_UI_SELECT;
					if (selected == item.size() - 1)
					{
						menu_event.iptkey = IPT_UI_CANCEL;
						ui_menu::stack_pop(machine());
					}
					stop = true;
				}
				break;

			// caught scroll event
			case UI_EVENT_MOUSE_WHEEL:
				if ((flags & UI_MENU_PROCESS_ONLYCHAR) == 0)
				{
					if (local_menu_event.zdelta > 0)
					{
						if ((flags & MENU_FLAG_UI_DATS) != 0)
						{
							top_line -= local_menu_event.num_lines;
							return;
						}
						selected -= local_menu_event.num_lines;
						validate_selection(-1);
						if (selected < top_line + (top_line != 0))
							top_line -= local_menu_event.num_lines;
					}
					else
					{
						if ((flags & MENU_FLAG_UI_DATS) != 0)
						{
							top_line += local_menu_event.num_lines;
							return;
						}
						selected += local_menu_event.num_lines;
						validate_selection(1);
						if (selected > item.size() - 1)
							selected = item.size() - 1;
						if (selected >= top_line + visitems + (top_line != 0))
							top_line += local_menu_event.num_lines;
					}
				}
				break;

			// translate CHAR events into specials
			case UI_EVENT_CHAR:
				menu_event.iptkey = IPT_SPECIAL;
				menu_event.unichar = local_menu_event.ch;
				stop = true;
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
	bool ignorepause = ui_menu::stack_has_special_main_menu();
	int code;

	// bail if no items
	if (item.empty())
		return;

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
	bool ignoreleft = ((item[selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	bool ignoreright = ((item[selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);

	if ((item[0].flags & MENU_FLAG_UI_DATS) != 0)
		ignoreleft = ignoreright = false;

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		if ((item[0].flags & MENU_FLAG_UI_DATS) != 0)
		{
			top_line--;
			return;
		}
		(selected == 0) ? selected = top_line = item.size() - 1 : --selected;
		validate_selection(-1);
		top_line -= (selected <= top_line && top_line != 0);
		if (selected <= top_line && visitems != visible_lines)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		if ((item[0].flags & MENU_FLAG_UI_DATS) != 0)
		{
			top_line++;
			return;
		}
		(selected == item.size() - 1) ? selected = top_line = 0 : ++selected;
		validate_selection(1);
		top_line += (selected >= top_line + visitems + (top_line != 0));
		if (selected >= (top_line + visitems + (top_line != 0)))
			top_line++;
	}

	// page up backs up by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_UP, 6))
	{
		selected -= visitems;
		top_line -= visitems - (top_line + visible_lines == item.size() - 1);
		if (selected < 0)
			selected = 0;
		validate_selection(1);
	}

	// page down advances by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_DOWN, 6))
	{
		selected += visible_lines - 2 + (selected == 0);
		top_line += visible_lines - 2;

		if (selected > item.size() - 1)
			selected = item.size() - 1;
		validate_selection(-1);
	}

	// home goes to the start
	if (exclusive_input_pressed(IPT_UI_HOME, 0))
	{
		selected = top_line = 0;
		validate_selection(1);
	}

	// end goes to the last
	if (exclusive_input_pressed(IPT_UI_END, 0))
	{
		selected = top_line = item.size() - 1;
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
	if (machine().ui_input().pressed_repeat(IPT_UI_TOGGLE_CHEAT, 0))
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
	while (menu_free != nullptr)
	{
		ui_menu *menu = menu_free;
		menu_free = menu->parent;
		global_free(menu);
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
	while (menu_stack != nullptr)
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
	menu->machine().ui_input().reset();
}


//-------------------------------------------------
//  stack_pop - pop a menu from the stack
//-------------------------------------------------

void ui_menu::stack_pop(running_machine &machine)
{
	if (menu_stack != nullptr)
	{
		ui_menu *menu = menu_stack;
		menu_stack = menu->parent;
		menu->parent = menu_free;
		menu_free = menu;
		machine.ui_input().reset();
	}
}


//-------------------------------------------------
//  ui_menu::stack_has_special_main_menu -
//  check in the special main menu is in the stack
//-------------------------------------------------

bool ui_menu::stack_has_special_main_menu()
{
	ui_menu *menu;

	for (menu = menu_stack; menu != nullptr; menu = menu->parent)
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
	if (menu_stack == nullptr)
		stack_push(global_alloc_clear<ui_menu_main>(machine, container));

	// update the menu state
	if (menu_stack != nullptr)
		menu_stack->do_handle();

	// clear up anything pending to be released
	clear_free_list(machine);

	// if the menus are to be hidden, return a cancel here
	if (machine.ui().is_menu_active() && ((machine.ui_input().pressed(IPT_UI_CONFIGURE) && !stack_has_special_main_menu()) || menu_stack == nullptr))
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
	container->add_quad(x0, y0, x1, y1, bgcolor, hilight_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE) | PRIMFLAG_PACKABLE);
}


//-------------------------------------------------
//  draw_arrow
//-------------------------------------------------

void ui_menu::draw_arrow(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation)
{
	container->add_quad(x0, y0, x1, y1, fgcolor, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(orientation) | PRIMFLAG_PACKABLE);
}

//-------------------------------------------------
//  init - initialize the ui menu system
//-------------------------------------------------

void ui_menu::init_ui(running_machine &machine)
{
	render_manager &mrender = machine.render();
	// create a texture for hilighting items in main menu
	hilight_main_bitmap = std::make_unique<bitmap_rgb32>(1, 26);
	int r1 = 0, g1 = 169, b1 = 255; //Any start color
	int r2 = 0, g2 = 39, b2 = 130; //Any stop color
	for (int y = 0; y < 26; y++)
	{
		int r = r1 + (y * (r2 - r1) / 26);
		int g = g1 + (y * (g2 - g1) / 26);
		int b = b1 + (y * (b2 - b1) / 26);
		hilight_main_bitmap->pix32(y, 0) = rgb_t(0xff, r, g, b);
	}

	hilight_main_texture = mrender.texture_alloc();
	hilight_main_texture->set_bitmap(*hilight_main_bitmap, hilight_main_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// create a texture for snapshot
	snapx_bitmap = std::make_unique<bitmap_argb32>(0, 0);
	snapx_texture = mrender.texture_alloc(render_texture::hq_scale);

	// allocates and sets the default "no available" image
	no_avail_bitmap = std::make_unique<bitmap_argb32>(256, 256);
	UINT32 *dst = &no_avail_bitmap->pix32(0);
	memcpy(dst, no_avail_bmp, 256 * 256 * sizeof(UINT32));

	// allocates and sets the favorites star image
	star_bitmap = std::make_unique<bitmap_argb32>(32, 32);
	dst = &star_bitmap->pix32(0);
	memcpy(dst, favorite_star_bmp, 32 * 32 * sizeof(UINT32));
	star_texture = mrender.texture_alloc();
	star_texture->set_bitmap(*star_bitmap, star_bitmap->cliprect(), TEXFORMAT_ARGB32);

	// allocate icons
	for (int i = 0; i < MAX_ICONS_RENDER; i++)
	{
		icons_bitmap[i] = auto_alloc(machine, bitmap_argb32);
		icons_texture[i] = mrender.texture_alloc();
	}

	// create a texture for main menu background
	bgrnd_bitmap = std::make_unique<bitmap_argb32>(0, 0);
	bgrnd_texture = mrender.texture_alloc(render_texture::hq_scale);

	ui_options &mopt = machine.ui().options();
	if (mopt.use_background_image() && &machine.system() == &GAME_NAME(___empty))
	{
		emu_file backgroundfile(".", OPEN_FLAG_READ);
		render_load_jpeg(*bgrnd_bitmap, backgroundfile, nullptr, "background.jpg");

		if (!bgrnd_bitmap->valid())
			render_load_png(*bgrnd_bitmap, backgroundfile, nullptr, "background.png");

		if (bgrnd_bitmap->valid())
			bgrnd_texture->set_bitmap(*bgrnd_bitmap, bgrnd_bitmap->cliprect(), TEXFORMAT_ARGB32);
		else
			bgrnd_bitmap->reset();
	}
	else
		bgrnd_bitmap->reset();

	// create a texture for toolbar
	for (int x = 0; x < UI_TOOLBAR_BUTTONS; ++x)
	{
		toolbar_bitmap[x] = auto_alloc(machine, bitmap_argb32(32, 32));
		sw_toolbar_bitmap[x] = auto_alloc(machine, bitmap_argb32(32, 32));
		toolbar_texture[x] = mrender.texture_alloc();
		sw_toolbar_texture[x] = mrender.texture_alloc();
		UINT32 *dst = &toolbar_bitmap[x]->pix32(0);
		memcpy(dst, toolbar_bitmap_bmp[x], 32 * 32 * sizeof(UINT32));
		if (toolbar_bitmap[x]->valid())
			toolbar_texture[x]->set_bitmap(*toolbar_bitmap[x], toolbar_bitmap[x]->cliprect(), TEXFORMAT_ARGB32);
		else
			toolbar_bitmap[x]->reset();

		if (x == 0 || x == 2)
		{
			dst = &sw_toolbar_bitmap[x]->pix32(0);
			memcpy(dst, toolbar_bitmap_bmp[x], 32 * 32 * sizeof(UINT32));
			if (sw_toolbar_bitmap[x]->valid())
				sw_toolbar_texture[x]->set_bitmap(*sw_toolbar_bitmap[x], sw_toolbar_bitmap[x]->cliprect(), TEXFORMAT_ARGB32);
			else
				sw_toolbar_bitmap[x]->reset();
		}
		else
			sw_toolbar_bitmap[x]->reset();

	}
}


//-------------------------------------------------
//  draw main menu
//-------------------------------------------------

void ui_menu::draw_select_game(bool noinput)
{
	float line_height = machine().ui().get_line_height();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = 0.52f * line_height * machine().render().ui_aspect();
	mouse_x = -1, mouse_y = -1;
	float right_panel_size = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL) ? 2.0f * UI_BOX_LR_BORDER : 0.3f;
	float visible_width = 1.0f - 4.0f * UI_BOX_LR_BORDER;
	float primary_left = (1.0f - visible_width) * 0.5f;
	float primary_width = visible_width;
	bool is_swlist = ((item[0].flags & MENU_FLAG_UI_SWLIST) != 0);
	bool is_favorites = ((item[0].flags & MENU_FLAG_UI_FAVORITE) != 0);
	ui_manager &mui = machine().ui();

	// draw background image if available
	if (machine().ui().options().use_background_image() && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, ARGB_WHITE, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	hover = item.size() + 1;
	visible_items = (is_swlist) ? item.size() - 2 : item.size() - 2 - skip_main_items;
	float extra_height = (is_swlist) ? 2.0f * line_height : (2.0f + skip_main_items) * line_height;
	float visible_extra_menu_height = customtop + custombottom + extra_height;

	// locate mouse
	mouse_hit = false;
	mouse_button = false;
	if (!noinput)
	{
		mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &mouse_button);
		if (mouse_target != nullptr)
			if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
				mouse_hit = true;
	}

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;
	visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)(visible_lines * line_height);

	if (!is_swlist)
		ui_globals::visible_main_lines = visible_lines;
	else
		ui_globals::visible_sw_lines = visible_lines;

	// compute top/left of inner menu area by centering
	float visible_left = primary_left;
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += customtop;

	// compute left box size
	float x1 = visible_left - UI_BOX_LR_BORDER;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = x1 + 2.0f * UI_BOX_LR_BORDER;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER + extra_height;

	// add left box
	visible_left = draw_left_panel(x1, y1, x2, y2);
	visible_width -= right_panel_size + visible_left - 2.0f * UI_BOX_LR_BORDER;

	// compute and add main box
	x1 = visible_left - UI_BOX_LR_BORDER;
	x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	float line = visible_top + (float)(visible_lines * line_height);

	//machine().ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);
	if (visible_items < visible_lines)
		visible_lines = visible_items;
	if (top_line < 0 || selected == 0)
		top_line = 0;
	if (selected < visible_items && top_line + visible_lines >= visible_items)
		top_line = visible_items - visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	int n_loop = (visible_items >= visible_lines) ? visible_lines : visible_items;
	if (m_prev_selected != nullptr && m_focus == focused_menu::main && selected < visible_items)
		m_prev_selected = nullptr;

	for (int linenum = 0; linenum < n_loop; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		int itemnum = top_line + linenum;
		const ui_menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		rgb_t fgcolor3 = UI_CLONE_COLOR;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line_y;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = itemnum;

		// if we're selected, draw with a different background
		if (itemnum == selected && m_focus == focused_menu::main)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			fgcolor3 = rgb_t(0xff, 0xcc, 0xcc, 0x00);
			mui.draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}
		// else if the mouse is over this item, draw with a different background
		else if (itemnum == hover)
		{
			fgcolor = fgcolor3 = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
		}
		else if (pitem.ref == m_prev_selected)
		{
			fgcolor = fgcolor3 = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			mui.draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}

		// if we're on the top line, display the up arrow
		if (linenum == 0 && top_line != 0)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0);

			if (hover == itemnum)
				hover = HOVER_ARROW_UP;
		}
		// if we're on the bottom line, display the down arrow
		else if (linenum == visible_lines - 1 && itemnum != visible_items - 1)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);

			if (hover == itemnum)
				hover = HOVER_ARROW_DOWN;
		}
		// if we're just a divider, draw a line
		else if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
			container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		// draw the item centered
		else if (pitem.subtext == nullptr)
		{
			int item_invert = pitem.flags & MENU_FLAG_INVERT;
			float space = 0.0f;

			if (ui_globals::has_icons && !is_swlist)
			{
				if (is_favorites)
				{
					ui_software_info *soft = (ui_software_info *)item[itemnum].ref;
					if (soft->startempty == 1)
						draw_icon(linenum, (void *)soft->driver, effective_left, line_y);
				}
				else
					draw_icon(linenum, item[itemnum].ref, effective_left, line_y);

				space = mui.get_line_height() * container->manager().ui_aspect() * 1.5f;
			}
			mui.draw_text_full(container, itemtext, effective_left + space, line_y, effective_width - space, JUSTIFY_LEFT, WRAP_TRUNCATE,
				DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, nullptr, nullptr);
		}
		else
		{
			int item_invert = pitem.flags & MENU_FLAG_INVERT;
			const char *subitem_text = pitem.subtext;
			float item_width, subitem_width;

			// compute right space for subitem
			mui.draw_text_full(container, subitem_text, effective_left, line_y, machine().ui().get_string_width(pitem.subtext),
				JUSTIFY_RIGHT, WRAP_NEVER, DRAW_NONE, item_invert ? fgcolor3 : fgcolor, bgcolor, &subitem_width, nullptr);
			subitem_width += gutter_width;

			// draw the item left-justified
			mui.draw_text_full(container, itemtext, effective_left, line_y, effective_width - subitem_width,
				JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, &item_width, nullptr);

			// draw the subitem right-justified
			mui.draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
				JUSTIFY_RIGHT, WRAP_NEVER, DRAW_NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, nullptr, nullptr);
		}
	}

	for (size_t count = visible_items; count < item.size(); count++)
	{
		const ui_menu_item &pitem = item[count];
		const char *itemtext = pitem.text;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = count;

		// if we're selected, draw with a different background
		if (count == selected && m_focus == focused_menu::main)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			mui.draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(255, 43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}
		// else if the mouse is over this item, draw with a different background
		else if (count == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
		}

		if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
			container->add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
			mui.draw_text_full(container, itemtext, effective_left, line, effective_width, JUSTIFY_CENTER, WRAP_TRUNCATE,
				DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		line += line_height;
	}

	x1 = x2;
	x2 += right_panel_size;

	draw_right_panel((selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr, x1, y1, x2, y2);

	x1 = primary_left - UI_BOX_LR_BORDER;
	x2 = primary_left + primary_width + UI_BOX_LR_BORDER;

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr, customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != visible_items);

	// reset redraw icon stage
	if (!is_swlist)
		ui_globals::redraw_icon = false;

	// noinput
	if (noinput)
	{
		int alpha = (1.0f - machine().options().pause_brightness()) * 255.0f;
		if (alpha > 255)
			alpha = 255;
		if (alpha >= 0)
			container->add_rect(0.0f, 0.0f, 1.0f, 1.0f, rgb_t(alpha, 0x00, 0x00, 0x00), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
	}
}

//-------------------------------------------------
//  get title and search path for right panel
//-------------------------------------------------

void ui_menu::get_title_search(std::string &snaptext, std::string &searchstr)
{
	// get arts title text
	snaptext.assign(_(arts_info[ui_globals::curimage_view].title));

	// get search path
	std::string addpath;
	if (ui_globals::curimage_view == SNAPSHOT_VIEW)
	{
		emu_options moptions;
		searchstr = machine().options().value(arts_info[ui_globals::curimage_view].path);
		addpath = moptions.value(arts_info[ui_globals::curimage_view].path);
	}
	else
	{
		ui_options moptions;
		searchstr = machine().ui().options().value(arts_info[ui_globals::curimage_view].path);
		addpath = moptions.value(arts_info[ui_globals::curimage_view].path);
	}

	std::string tmp(searchstr);
	path_iterator path(tmp.c_str());
	path_iterator path_iter(addpath.c_str());
	std::string c_path, curpath;

	// iterate over path and add path for zipped formats
	while (path.next(curpath))
	{
		path_iter.reset();
		while (path_iter.next(c_path))
			searchstr.append(";").append(curpath).append(PATH_SEPARATOR).append(c_path);
	}
}

//-------------------------------------------------
//  handle keys for main menu
//-------------------------------------------------

void ui_menu::handle_main_keys(UINT32 flags)
{
	bool ignorepause = ui_menu::stack_has_special_main_menu();

	// bail if no items
	if (item.size() == 0)
		return;

	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (selected == item.size() - 1 && m_focus == focused_menu::main)
		{
			menu_event.iptkey = IPT_UI_CANCEL;
			ui_menu::stack_pop(machine());
		}
		return;
	}

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		if (!ui_error && !menu_has_search_active())
			ui_menu::stack_pop(machine());
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool ignoreleft = ((item[selected].flags & MENU_FLAG_LEFT_ARROW) == 0);
	bool ignoreright = ((item[selected].flags & MENU_FLAG_RIGHT_ARROW) == 0);
	bool leftclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_LEFT_PANEL);
	bool rightclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL);

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::righttop)
			menu_event.iptkey = IPT_UI_LEFT_PANEL;
		return;
	}

	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & UI_MENU_PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::righttop)
			menu_event.iptkey = IPT_UI_RIGHT_PANEL;
		return;
	}

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		// Filter
		if (!leftclose && m_focus == focused_menu::left)
		{
			menu_event.iptkey = IPT_UI_UP_FILTER;
			return;
		}

		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_UP_PANEL;
			topline_datsview--;
			return;
		}

		if (selected == visible_items + 1 || selected == 0 || ui_error)
			return;

		selected--;

		if (selected == top_line && top_line != 0)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		// Filter
		if (!leftclose && m_focus == focused_menu::left)
		{
			menu_event.iptkey = IPT_UI_DOWN_FILTER;
			return;
		}

		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview++;
			return;
		}

		if (selected == item.size() - 1 || selected == visible_items - 1 || ui_error)
			return;

		selected++;

		if (selected == top_line + visitems + (top_line != 0))
			top_line++;
	}

	// page up backs up by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_UP, 6))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview -= right_visible_lines - 1;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected -= visitems;

			if (selected < 0)
				selected = 0;

			top_line -= visitems - (top_line + visible_lines == visible_items);
		}
	}

	// page down advances by visitems
	if (exclusive_input_pressed(IPT_UI_PAGE_DOWN, 6))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview += right_visible_lines - 1;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected += visible_lines - 2 + (selected == 0);

			if (selected >= visible_items)
				selected = visible_items - 1;

			top_line += visible_lines - 2;
		}
	}

	// home goes to the start
	if (exclusive_input_pressed(IPT_UI_HOME, 0))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview = 0;
			return;
		}

		if (selected < visible_items && !ui_error)
		{
			selected = 0;
			top_line = 0;
		}
	}

	// end goes to the last
	if (exclusive_input_pressed(IPT_UI_END, 0))
	{
		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			menu_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview = totallines;
			return;
		}

		if (selected < visible_items && !ui_error)
			selected = top_line = visible_items - 1;
	}

	// pause enables/disables pause
	if (!ui_error && !ignorepause && exclusive_input_pressed(IPT_UI_PAUSE, 0))
	{
		if (machine().paused())
			machine().resume();
		else
			machine().pause();
	}

	// handle a toggle cheats request
	if (!ui_error && machine().ui_input().pressed_repeat(IPT_UI_TOGGLE_CHEAT, 0))
		machine().cheat().set_enable(!machine().cheat().enabled());

	// see if any other UI keys are pressed
	if (menu_event.iptkey == IPT_INVALID)
		for (int code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (ui_error || code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft) || (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;

			if (exclusive_input_pressed(code, 0))
				break;
		}
}

//-------------------------------------------------
//  handle input events for main menu
//-------------------------------------------------

void ui_menu::handle_main_events(UINT32 flags)
{
	bool stop = false;
	ui_event local_menu_event;

	if (m_pressed)
	{
		bool pressed = mouse_pressed();
		INT32 m_target_x, m_target_y;
		bool m_button;
		render_target *mouse_target = machine().ui_input().find_mouse(&m_target_x, &m_target_y, &m_button);
		if (mouse_target != nullptr && m_button && (hover == HOVER_ARROW_DOWN || hover == HOVER_ARROW_UP))
		{
			if (pressed)
				machine().ui_input().push_mouse_down_event(mouse_target, m_target_x, m_target_y);
		}
		else
			reset_pressed();
	}

	// loop while we have interesting events
	while (!stop && machine().ui_input().pop_event(&local_menu_event))
	{
		switch (local_menu_event.event_type)
		{
			// if we are hovering over a valid item, select it with a single click
		case UI_EVENT_MOUSE_DOWN:
		{
			if (ui_error)
			{
				menu_event.iptkey = IPT_OTHER;
				stop = true;
			}
			else
			{
				if (hover >= 0 && hover < item.size())
				{
					if (hover >= visible_items - 1 && selected < visible_items)
						m_prev_selected = item[selected].ref;
					selected = hover;
					m_focus = focused_menu::main;
				}
				else if (hover == HOVER_ARROW_UP)
				{
					selected -= visitems;
					if (selected < 0)
						selected = 0;
					top_line -= visitems - (top_line + visible_lines == visible_items);
					set_pressed();
				}
				else if (hover == HOVER_ARROW_DOWN)
				{
					selected += visible_lines - 2 + (selected == 0);
					if (selected >= visible_items)
						selected = visible_items - 1;
					top_line += visible_lines - 2;
					set_pressed();
				}
				else if (hover == HOVER_UI_RIGHT)
					menu_event.iptkey = IPT_UI_RIGHT;
				else if (hover == HOVER_UI_LEFT)
					menu_event.iptkey = IPT_UI_LEFT;
				else if (hover == HOVER_DAT_DOWN)
					topline_datsview += right_visible_lines - 1;
				else if (hover == HOVER_DAT_UP)
					topline_datsview -= right_visible_lines - 1;
				else if (hover == HOVER_LPANEL_ARROW)
				{
					if (ui_globals::panels_status == HIDE_LEFT_PANEL)
						ui_globals::panels_status = SHOW_PANELS;
					else if (ui_globals::panels_status == HIDE_BOTH)
						ui_globals::panels_status = HIDE_RIGHT_PANEL;
					else if (ui_globals::panels_status == SHOW_PANELS)
						ui_globals::panels_status = HIDE_LEFT_PANEL;
					else if (ui_globals::panels_status == HIDE_RIGHT_PANEL)
						ui_globals::panels_status = HIDE_BOTH;
				}
				else if (hover == HOVER_RPANEL_ARROW)
				{
					if (ui_globals::panels_status == HIDE_RIGHT_PANEL)
						ui_globals::panels_status = SHOW_PANELS;
					else if (ui_globals::panels_status == HIDE_BOTH)
						ui_globals::panels_status = HIDE_LEFT_PANEL;
					else if (ui_globals::panels_status == SHOW_PANELS)
						ui_globals::panels_status = HIDE_RIGHT_PANEL;
					else if (ui_globals::panels_status == HIDE_LEFT_PANEL)
						ui_globals::panels_status = HIDE_BOTH;
				}
				else if (hover == HOVER_B_FAV)
				{
					menu_event.iptkey = IPT_UI_FAVORITES;
					stop = true;
				}
				else if (hover == HOVER_B_EXPORT)
				{
					menu_event.iptkey = IPT_UI_EXPORT;
					stop = true;
				}
				else if (hover == HOVER_B_DATS)
				{
					menu_event.iptkey = IPT_UI_DATS;
					stop = true;
				}
				else if (hover >= HOVER_RP_FIRST && hover <= HOVER_RP_LAST)
				{
					ui_globals::rpanel = (HOVER_RP_FIRST - hover) * (-1);
					stop = true;
				}
				else if (hover >= HOVER_SW_FILTER_FIRST && hover <= HOVER_SW_FILTER_LAST)
				{
					l_sw_hover = (HOVER_SW_FILTER_FIRST - hover) * (-1);
					menu_event.iptkey = IPT_OTHER;
					stop = true;
				}
				else if (hover >= HOVER_FILTER_FIRST && hover <= HOVER_FILTER_LAST)
				{
					l_hover = (HOVER_FILTER_FIRST - hover) * (-1);
					menu_event.iptkey = IPT_OTHER;
					stop = true;
				}
			}
			break;
		}

		// if we are hovering over a valid item, fake a UI_SELECT with a double-click
		case UI_EVENT_MOUSE_DOUBLE_CLICK:
			if (hover >= 0 && hover < item.size())
			{
				selected = hover;
				menu_event.iptkey = IPT_UI_SELECT;
			}

			if (selected == item.size() - 1)
			{
				menu_event.iptkey = IPT_UI_CANCEL;
				ui_menu::stack_pop(machine());
			}
			stop = true;
			break;

			// caught scroll event
		case UI_EVENT_MOUSE_WHEEL:
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
			break;

			// translate CHAR events into specials
		case UI_EVENT_CHAR:
			if (exclusive_input_pressed(IPT_UI_CONFIGURE, 0))
			{
				menu_event.iptkey = IPT_UI_CONFIGURE;
				stop = true;
			}
			else
			{
				menu_event.iptkey = IPT_SPECIAL;
				menu_event.unichar = local_menu_event.ch;
				stop = true;
			}
			break;

			// ignore everything else
		default:
			break;
		}
	}
}

//-------------------------------------------------
//  draw right box title
//-------------------------------------------------

float ui_menu::draw_right_box_title(float x1, float y1, float x2, float y2)
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();
	float midl = (x2 - x1) * 0.5f;

	// add outlined box for options
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// add separator line
	container->add_line(x1 + midl, y1, x1 + midl, y1 + line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	std::string buffer[RP_LAST + 1];
	buffer[RP_IMAGES] = _("Images");
	buffer[RP_INFOS] = _("Infos");

	// check size
	float text_size = 1.0f;
	for (auto & elem : buffer)
	{
		float textlen = mui.get_string_width(elem.c_str()) + 0.01f;
		float tmp_size = (textlen > midl) ? (midl / textlen) : 1.0f;
		text_size = MIN(text_size, tmp_size);
	}

	for (int cells = RP_FIRST; cells <= RP_LAST; ++cells)
	{
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		rgb_t fgcolor = UI_TEXT_COLOR;

		if (mouse_hit && x1 <= mouse_x && x1 + midl > mouse_x && y1 <= mouse_y && y1 + line_height > mouse_y)
		{
			if (ui_globals::rpanel != cells)
			{
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				fgcolor = UI_MOUSEOVER_COLOR;
				hover = HOVER_RP_FIRST + cells;
			}
		}

		if (ui_globals::rpanel != cells)
		{
			container->add_line(x1, y1 + line_height, x1 + midl, y1 + line_height, UI_LINE_WIDTH,
				UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			if (fgcolor != UI_MOUSEOVER_COLOR)
				fgcolor = UI_CLONE_COLOR;
		}

		if (m_focus == focused_menu::righttop && ui_globals::rpanel == cells)
		{
			fgcolor = rgb_t(0xff, 0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff, 0xff);
			mui.draw_textured_box(container, x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
				bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}
		else if (bgcolor == UI_MOUSEOVER_BG_COLOR)
			container->add_rect(x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
				bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		mui.draw_text_full(container, buffer[cells].c_str(), x1 + UI_LINE_WIDTH, y1, midl - UI_LINE_WIDTH,
			JUSTIFY_CENTER, WRAP_NEVER, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
		x1 += midl;
	}

	return (y1 + line_height + UI_LINE_WIDTH);
}

//-------------------------------------------------
//  common function for images render
//-------------------------------------------------

std::string ui_menu::arts_render_common(float origx1, float origy1, float origx2, float origy2)
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();
	std::string snaptext, searchstr;
	float title_size = 0.0f;
	float txt_lenght = 0.0f;
	float gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;

	get_title_search(snaptext, searchstr);

	// apply title to right panel
	for (int x = FIRST_VIEW; x < LAST_VIEW; x++)
	{
		mui.draw_text_full(container, _(arts_info[x].title), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER,
			WRAP_TRUNCATE, DRAW_NONE, ARGB_WHITE, ARGB_BLACK, &txt_lenght, nullptr);
		txt_lenght += 0.01f;
		title_size = MAX(txt_lenght, title_size);
	}

	rgb_t fgcolor = (m_focus == focused_menu::rightbottom) ? rgb_t(0xff, 0xff, 0xff, 0x00) : UI_TEXT_COLOR;
	rgb_t bgcolor = (m_focus == focused_menu::rightbottom) ? rgb_t(0xff, 0xff, 0xff, 0xff) : UI_TEXT_BG_COLOR;
	float middle = origx2 - origx1;

	// check size
	float sc = title_size + 2.0f * gutter_width;
	float tmp_size = (sc > middle) ? ((middle - 2.0f * gutter_width) / sc) : 1.0f;
	title_size *= tmp_size;

	if (bgcolor != UI_TEXT_BG_COLOR)
		mui.draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
			origy1 + line_height, bgcolor, rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

	mui.draw_text_full(container, snaptext.c_str(), origx1, origy1, origx2 - origx1, JUSTIFY_CENTER, WRAP_TRUNCATE,
		DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr, tmp_size);

	draw_common_arrow(origx1, origy1, origx2, origy2, ui_globals::curimage_view, FIRST_VIEW, LAST_VIEW, title_size);

	return searchstr;
}

//-------------------------------------------------
//  draw favorites star
//-------------------------------------------------

void ui_menu::draw_star(float x0, float y0)
{
	float y1 = y0 + machine().ui().get_line_height();
	float x1 = x0 + machine().ui().get_line_height() * container->manager().ui_aspect();
	container->add_quad(x0, y0, x1, y1, ARGB_WHITE, star_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_PACKABLE);
}

//-------------------------------------------------
//  draw toolbar
//-------------------------------------------------

void ui_menu::draw_toolbar(float x1, float y1, float x2, float y2, bool software)
{
	ui_manager &mui = machine().ui();
	// draw a box
	mui.draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	render_texture **t_texture = (software) ? sw_toolbar_texture : toolbar_texture;
	bitmap_argb32 **t_bitmap = (software) ? sw_toolbar_bitmap : toolbar_bitmap;

	int m_valid = 0;
	for (int x = 0; x < UI_TOOLBAR_BUTTONS; ++x)
	{
		if (t_bitmap[x]->valid())
		{
			m_valid++;
		}
	}

	float space_x = (y2 - y1) * container->manager().ui_aspect();
	float total = (m_valid * space_x) + ((m_valid - 1) * 0.001f);
	x1 = ((x2 - x1) * 0.5f) - (total / 2);
	x2 = x1 + space_x;

	for (int z = 0; z < UI_TOOLBAR_BUTTONS; ++z)
	{
		if (t_bitmap[z]->valid())
		{
			rgb_t color(0xEFEFEFEF);
			if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y2 > mouse_y)
			{
				hover = HOVER_B_FAV + z;
				color = ARGB_WHITE;
				float ypos = y2 + machine().ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
				mui.draw_text_box(container, _(hover_msg[z]), JUSTIFY_CENTER, 0.5f, ypos, UI_BACKGROUND_COLOR);
			}

			container->add_quad(x1, y1, x2, y2, color, t_texture[z], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			x1 += space_x + ((z < UI_TOOLBAR_BUTTONS - 1) ? 0.001f : 0.0f);
			x2 = x1 + space_x;
		}
	}
}


//-------------------------------------------------
//  perform rendering of image
//-------------------------------------------------

void ui_menu::arts_render_images(bitmap_argb32 *tmp_bitmap, float origx1, float origy1, float origx2, float origy2, bool software)
{
	bool no_available = false;
	float line_height = machine().ui().get_line_height();

	// if it fails, use the default image
	if (!tmp_bitmap->valid())
	{
		tmp_bitmap->allocate(256, 256);
		for (int x = 0; x < 256; x++)
			for (int y = 0; y < 256; y++)
				tmp_bitmap->pix32(y, x) = no_avail_bitmap->pix32(y, x);
		no_available = true;
	}

	if (tmp_bitmap->valid())
	{
		float panel_width = origx2 - origx1 - 0.02f;
		float panel_height = origy2 - origy1 - 0.02f - (2.0f * UI_BOX_TB_BORDER) - (2.0f * line_height);
		int screen_width = machine().render().ui_target().width();
		int screen_height = machine().render().ui_target().height();

		if (machine().render().ui_target().orientation() & ORIENTATION_SWAP_XY)
			std::swap(screen_height, screen_width);

		int panel_width_pixel = panel_width * screen_width;
		int panel_height_pixel = panel_height * screen_height;

		// Calculate resize ratios for resizing
		float ratioW = (float)panel_width_pixel / tmp_bitmap->width();
		float ratioH = (float)panel_height_pixel / tmp_bitmap->height();
		float ratioI = (float)tmp_bitmap->height() / tmp_bitmap->width();
		int dest_xPixel = tmp_bitmap->width();
		int dest_yPixel = tmp_bitmap->height();

		// force 4:3 ratio min
		if (machine().ui().options().forced_4x3_snapshot() && ratioI < 0.75f && ui_globals::curimage_view == SNAPSHOT_VIEW)
		{
			// smaller ratio will ensure that the image fits in the view
			dest_yPixel = tmp_bitmap->width() * 0.75f;
			ratioH = (float)panel_height_pixel / dest_yPixel;
			float ratio = MIN(ratioW, ratioH);
			dest_xPixel = tmp_bitmap->width() * ratio;
			dest_yPixel *= ratio;
		}
		// resize the bitmap if necessary
		else if (ratioW < 1 || ratioH < 1 || (machine().ui().options().enlarge_snaps() && !no_available))
		{
			// smaller ratio will ensure that the image fits in the view
			float ratio = MIN(ratioW, ratioH);
			dest_xPixel = tmp_bitmap->width() * ratio;
			dest_yPixel = tmp_bitmap->height() * ratio;
		}

		bitmap_argb32 *dest_bitmap;

		// resample if necessary
		if (dest_xPixel != tmp_bitmap->width() || dest_yPixel != tmp_bitmap->height())
		{
			dest_bitmap = auto_alloc(machine(), bitmap_argb32);
			dest_bitmap->allocate(dest_xPixel, dest_yPixel);
			render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
			render_resample_argb_bitmap_hq(*dest_bitmap, *tmp_bitmap, color, true);
		}
		else
			dest_bitmap = tmp_bitmap;

		snapx_bitmap->allocate(panel_width_pixel, panel_height_pixel);
		int x1 = (0.5f * panel_width_pixel) - (0.5f * dest_xPixel);
		int y1 = (0.5f * panel_height_pixel) - (0.5f * dest_yPixel);

		for (int x = 0; x < dest_xPixel; x++)
			for (int y = 0; y < dest_yPixel; y++)
				snapx_bitmap->pix32(y + y1, x + x1) = dest_bitmap->pix32(y, x);

		auto_free(machine(), dest_bitmap);

		// apply bitmap
		snapx_texture->set_bitmap(*snapx_bitmap, snapx_bitmap->cliprect(), TEXFORMAT_ARGB32);
	}
	else
		snapx_bitmap->reset();
}

//-------------------------------------------------
//  draw common arrows
//-------------------------------------------------

void ui_menu::draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title_size)
{
	float line_height = machine().ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;

	// set left-right arrows dimension
	float ar_x0 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width - lr_arrow_width;
	float ar_y0 = origy1 + 0.1f * line_height;
	float ar_x1 = 0.5f * (origx2 + origx1) + 0.5f * title_size + gutter_width;
	float ar_y1 = origy1 + 0.9f * line_height;

	float al_x0 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width;
	float al_y0 = origy1 + 0.1f * line_height;
	float al_x1 = 0.5f * (origx2 + origx1) - 0.5f * title_size - gutter_width + lr_arrow_width;
	float al_y1 = origy1 + 0.9f * line_height;

	rgb_t fgcolor_right, fgcolor_left;
	fgcolor_right = fgcolor_left = UI_TEXT_COLOR;

	// set hover
	if (mouse_hit && ar_x0 <= mouse_x && ar_x1 > mouse_x && ar_y0 <= mouse_y && ar_y1 > mouse_y && current != dmax)
	{
		machine().ui().draw_textured_box(container, ar_x0 + 0.01f, ar_y0, ar_x1 - 0.01f, ar_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(255, 43, 43, 43),
			hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = HOVER_UI_RIGHT;
		fgcolor_right = UI_MOUSEOVER_COLOR;
	}
	else if (mouse_hit && al_x0 <= mouse_x && al_x1 > mouse_x && al_y0 <= mouse_y && al_y1 > mouse_y && current != dmin)
	{
		machine().ui().draw_textured_box(container, al_x0 + 0.01f, al_y0, al_x1 - 0.01f, al_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(255, 43, 43, 43),
			hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = HOVER_UI_LEFT;
		fgcolor_left = UI_MOUSEOVER_COLOR;
	}

	// apply arrow
	if (current == dmin)
		draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, ROT90);
	else if (current == dmax)
		draw_arrow(container, al_x0, al_y0, al_x1, al_y1, fgcolor_left, ROT90 ^ ORIENTATION_FLIP_X);
	else
	{
		draw_arrow(container, ar_x0, ar_y0, ar_x1, ar_y1, fgcolor_right, ROT90);
		draw_arrow(container, al_x0, al_y0, al_x1, al_y1, fgcolor_left, ROT90 ^ ORIENTATION_FLIP_X);
	}
}

//-------------------------------------------------
//  draw icons
//-------------------------------------------------

void ui_menu::draw_icon(int linenum, void *selectedref, float x0, float y0)
{
	static const game_driver *olddriver[MAX_ICONS_RENDER] = { nullptr };
	float x1 = x0 + machine().ui().get_line_height() * container->manager().ui_aspect(container);
	float y1 = y0 + machine().ui().get_line_height();
	const game_driver *driver = (const game_driver *)selectedref;

	if (olddriver[linenum] != driver || ui_globals::redraw_icon)
	{
		olddriver[linenum] = driver;

		// set clone status
		bool cloneof = strcmp(driver->parent, "0");
		if (cloneof)
		{
			int cx = driver_list::find(driver->parent);
			if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
				cloneof = false;
		}

		// get search path
		path_iterator path(machine().ui().options().icons_directory());
		std::string curpath;
		std::string searchstr(machine().ui().options().icons_directory());

		// iterate over path and add path for zipped formats
		while (path.next(curpath))
			searchstr.append(";").append(curpath.c_str()).append(PATH_SEPARATOR).append("icons");

		bitmap_argb32 *tmp = auto_alloc(machine(), bitmap_argb32);
		emu_file snapfile(searchstr.c_str(), OPEN_FLAG_READ);
		std::string fullname = std::string(driver->name).append(".ico");
		render_load_ico(*tmp, snapfile, nullptr, fullname.c_str());

		if (!tmp->valid() && cloneof)
		{
			fullname.assign(driver->parent).append(".ico");
			render_load_ico(*tmp, snapfile, nullptr, fullname.c_str());
		}

		if (tmp->valid())
		{
			float panel_width = x1 - x0;
			float panel_height = y1 - y0;
			int screen_width = machine().render().ui_target().width();
			int screen_height = machine().render().ui_target().height();

			if (machine().render().ui_target().orientation() & ORIENTATION_SWAP_XY)
				std::swap(screen_height, screen_width);

			int panel_width_pixel = panel_width * screen_width;
			int panel_height_pixel = panel_height * screen_height;

			// Calculate resize ratios for resizing
			float ratioW = (float)panel_width_pixel / tmp->width();
			float ratioH = (float)panel_height_pixel / tmp->height();
			int dest_xPixel = tmp->width();
			int dest_yPixel = tmp->height();

			if (ratioW < 1 || ratioH < 1)
			{
				// smaller ratio will ensure that the image fits in the view
				float ratio = MIN(ratioW, ratioH);
				dest_xPixel = tmp->width() * ratio;
				dest_yPixel = tmp->height() * ratio;
			}

			bitmap_argb32 *dest_bitmap;
			dest_bitmap = auto_alloc(machine(), bitmap_argb32);

			// resample if necessary
			if (dest_xPixel != tmp->width() || dest_yPixel != tmp->height())
			{
				dest_bitmap->allocate(dest_xPixel, dest_yPixel);
				render_color color = { 1.0f, 1.0f, 1.0f, 1.0f };
				render_resample_argb_bitmap_hq(*dest_bitmap, *tmp, color, true);
			}
			else
				dest_bitmap = tmp;

			icons_bitmap[linenum]->reset();
			icons_bitmap[linenum]->allocate(panel_width_pixel, panel_height_pixel);

			for (int x = 0; x < dest_xPixel; x++)
				for (int y = 0; y < dest_yPixel; y++)
					icons_bitmap[linenum]->pix32(y, x) = dest_bitmap->pix32(y, x);

			auto_free(machine(), dest_bitmap);

			icons_texture[linenum]->set_bitmap(*icons_bitmap[linenum], icons_bitmap[linenum]->cliprect(), TEXFORMAT_ARGB32);
		}
		else if (icons_bitmap[linenum] != nullptr)
			icons_bitmap[linenum]->reset();

		auto_free(machine(), tmp);
	}

	if (icons_bitmap[linenum] != nullptr && icons_bitmap[linenum]->valid())
		container->add_quad(x0, y0, x1, y1, ARGB_WHITE, icons_texture[linenum], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
}

//-------------------------------------------------
//  draw info arrow
//-------------------------------------------------

void ui_menu::info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width)
{
	rgb_t fgcolor = UI_TEXT_COLOR;
	UINT32 orientation = (!ub) ? ROT0 : ROT0 ^ ORIENTATION_FLIP_Y;

	if (mouse_hit && origx1 <= mouse_x && origx2 > mouse_x && oy1 <= mouse_y && oy1 + (line_height * text_size) > mouse_y)
	{
		machine().ui().draw_textured_box(container, origx1 + 0.01f, oy1, origx2 - 0.01f, oy1 + (line_height * text_size), UI_MOUSEOVER_BG_COLOR,
			rgb_t(255, 43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = (!ub) ? HOVER_DAT_UP : HOVER_DAT_DOWN;
		fgcolor = UI_MOUSEOVER_COLOR;
	}

	draw_arrow(container, 0.5f * (origx1 + origx2) - 0.5f * (ud_arrow_width * text_size), oy1 + 0.25f * (line_height * text_size),
		0.5f * (origx1 + origx2) + 0.5f * (ud_arrow_width * text_size), oy1 + 0.75f * (line_height * text_size), fgcolor, orientation);
}

//-------------------------------------------------
//  draw - draw palette menu
//-------------------------------------------------

void ui_menu::draw_palette_menu()
{
	ui_manager &mui = machine().ui();
	float line_height = mui.get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width * 1.3f;
	int itemnum, linenum;

	if (machine().ui().options().use_background_image() && machine().options().system() == nullptr && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, ARGB_WHITE, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// compute the width and height of the full menu
	float visible_width = 0;
	float visible_main_menu_height = 0;
	for (itemnum = 0; itemnum < item.size(); itemnum++)
	{
		const ui_menu_item &pitem = item[itemnum];

		// compute width of left hand side
		float total_width = gutter_width + mui.get_string_width(pitem.text) + gutter_width;

		// add in width of right hand side
		if (pitem.subtext)
			total_width += 2.0f * gutter_width + mui.get_string_width(pitem.subtext);

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
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

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
	mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != nullptr)
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
			mouse_hit = true;

	// loop over visible lines
	hover = item.size() + 1;
	float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;

	for (linenum = 0; linenum < visible_lines; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		itemnum = top_line + linenum;
		const ui_menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		float line_y0 = line_y;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = itemnum;

		// if we're selected, draw with a different background
		if (itemnum == selected)
		{
			fgcolor = UI_SELECTED_COLOR;
			bgcolor = UI_SELECTED_BG_COLOR;
		}

		// else if the mouse is over this item, draw with a different background
		else if (itemnum == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
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

		// if we don't have a subitem, just draw the string centered
		else if (pitem.subtext == nullptr)
			mui.draw_text_full(container, itemtext, effective_left, line_y, effective_width,
			JUSTIFY_CENTER, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);

		// otherwise, draw the item on the left and the subitem text on the right
		else
		{
			const char *subitem_text = pitem.subtext;
			rgb_t color = rgb_t((UINT32)strtoul(subitem_text, nullptr, 16));

			// draw the left-side text
			mui.draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				JUSTIFY_LEFT, WRAP_TRUNCATE, DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);

			// give 2 spaces worth of padding
			float subitem_width = mui.get_string_width("FF00FF00");

			mui.draw_outlined_box(container, effective_left + effective_width - subitem_width, line_y0,
				effective_left + effective_width, line_y1, color);
		}
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr, customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != item.size());
}

//-------------------------------------------------
//  draw - draw dats menu
//-------------------------------------------------

void ui_menu::draw_dats_menu()
{
	float line_height = machine().ui().get_line_height();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = 0.52f * line_height * machine().render().ui_aspect();
	mouse_x = -1, mouse_y = -1;
	float visible_width = 1.0f - 2.0f * UI_BOX_LR_BORDER;
	float visible_left = (1.0f - visible_width) * 0.5f;
	ui_manager &mui = machine().ui();

	// draw background image if available
	if (machine().ui().options().use_background_image() && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, ARGB_WHITE, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	hover = item.size() + 1;
	visible_items = item.size() - 2;
	float extra_height = 2.0f * line_height;
	float visible_extra_menu_height = customtop + custombottom + extra_height;

	// locate mouse
	mouse_hit = false;
	mouse_button = false;
	mouse_target = machine().ui_input().find_mouse(&mouse_target_x, &mouse_target_y, &mouse_button);
	if (mouse_target != nullptr)
		if (mouse_target->map_point_container(mouse_target_x, mouse_target_y, *container, mouse_x, mouse_y))
			mouse_hit = true;

	// account for extra space at the top and bottom
	float visible_main_menu_height = 1.0f - 2.0f * UI_BOX_TB_BORDER - visible_extra_menu_height;
	visible_lines = floor(visible_main_menu_height / line_height);
	visible_main_menu_height = (float)(visible_lines * line_height);

	// compute top/left of inner menu area by centering
	float visible_top = (1.0f - (visible_main_menu_height + visible_extra_menu_height)) * 0.5f;

	// if the menu is at the bottom of the extra, adjust
	visible_top += customtop;

	// compute left box size
	float x1 = visible_left;
	float y1 = visible_top - UI_BOX_TB_BORDER;
	float x2 = x1 + visible_width;
	float y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER + extra_height;
	float line = visible_top + (float)(visible_lines * line_height);

	//machine().ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));
	mui.draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	if (visible_items < visible_lines)
		visible_lines = visible_items;
	if (top_line < 0)
		top_line = 0;
	if (top_line + visible_lines >= visible_items)
		top_line = visible_items - visible_lines;

	// determine effective positions taking into account the hilighting arrows
	float effective_width = visible_width - 2.0f * gutter_width;
	float effective_left = visible_left + gutter_width;

	int n_loop = (visible_items >= visible_lines) ? visible_lines : visible_items;

	for (int linenum = 0; linenum < n_loop; linenum++)
	{
		float line_y = visible_top + (float)linenum * line_height;
		int itemnum = top_line + linenum;
		const ui_menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line_y;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line_y + line_height;

		// if we're on the top line, display the up arrow
		if (linenum == 0 && top_line != 0)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0);

			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y)
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
				hover = HOVER_ARROW_UP;
			}
		}
		// if we're on the bottom line, display the down arrow
		else if (linenum == visible_lines - 1 && itemnum != visible_items - 1)
		{
			draw_arrow(container, 0.5f * (x1 + x2) - 0.5f * ud_arrow_width, line_y + 0.25f * line_height,
				0.5f * (x1 + x2) + 0.5f * ud_arrow_width, line_y + 0.75f * line_height, fgcolor, ROT0 ^ ORIENTATION_FLIP_Y);

			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y)
			{
				fgcolor = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
				highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
				hover = HOVER_ARROW_DOWN;
			}
		}

		// draw dats text
		else if (pitem.subtext == nullptr)
		{
			mui.draw_text_full(container, itemtext, effective_left, line_y, effective_width, JUSTIFY_LEFT, WRAP_NEVER,
				DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		}
	}

	for (size_t count = visible_items; count < item.size(); count++)
	{
		const ui_menu_item &pitem = item[count];
		const char *itemtext = pitem.text;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = UI_SELECTED_COLOR;
		rgb_t bgcolor = UI_SELECTED_BG_COLOR;

		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && pitem.is_selectable())
			hover = count;

		if (strcmp(itemtext, MENU_SEPARATOR_ITEM) == 0)
			container->add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
		{
			highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
			mui.draw_text_full(container, itemtext, effective_left, line, effective_width, JUSTIFY_CENTER, WRAP_TRUNCATE,
				DRAW_NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		}
		line += line_height;
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render((selected >= 0 && selected < item.size()) ? item[selected].ref : nullptr, customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != visible_items);
}

void ui_menu::set_pressed()
{
	(m_repeat == 0) ? m_repeat = osd_ticks() + osd_ticks_per_second() / 2 : m_repeat = osd_ticks() + osd_ticks_per_second() / 4;
	m_pressed = true;
}
