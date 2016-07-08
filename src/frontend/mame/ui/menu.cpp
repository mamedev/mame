// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods, Maurizio Petrarota
/*********************************************************************

    ui/menu.cpp

    Internal MAME menus for the user interface.

*********************************************************************/

#include "emu.h"

#include "ui/menu.h"

#include "ui/ui.h"
#include "ui/mainmenu.h"
#include "ui/utils.h"
#include "ui/defimg.h"
#include "ui/starimg.h"
#include "ui/icorender.h"
#include "ui/toolbar.h"
#include "ui/miscmenu.h"

#include "cheat.h"
#include "drivenum.h"
#include "mame.h"
#include "rendutil.h"
#include "uiinput.h"


namespace ui {
/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE  65536
#define MAX_ICONS_RENDER   40

static std::vector<std::pair<const char *, const char *>> arts_info =
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
};

static const char *hover_msg[] = {
	__("Add or remove favorites"),
	__("Export displayed list to file"),
	__("Show DATs view"),
};

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

std::unique_ptr<menu> menu::menu_stack;
std::unique_ptr<menu> menu::menu_free;
std::unique_ptr<bitmap_rgb32> menu::hilight_bitmap;
render_texture *menu::hilight_texture;
render_texture *menu::arrow_texture;
render_texture *menu::snapx_texture;
render_texture *menu::hilight_main_texture;
render_texture *menu::bgrnd_texture;
render_texture *menu::star_texture;
render_texture *menu::toolbar_texture[UI_TOOLBAR_BUTTONS];
render_texture *menu::sw_toolbar_texture[UI_TOOLBAR_BUTTONS];
render_texture *menu::icons_texture[MAX_ICONS_RENDER];
std::unique_ptr<bitmap_argb32> menu::snapx_bitmap;
std::unique_ptr<bitmap_argb32> menu::no_avail_bitmap;
std::unique_ptr<bitmap_argb32> menu::star_bitmap;
std::unique_ptr<bitmap_argb32> menu::bgrnd_bitmap;
std::vector<std::unique_ptr<bitmap_argb32>> menu::icons_bitmap;
std::unique_ptr<bitmap_rgb32> menu::hilight_main_bitmap;
std::vector<std::shared_ptr<bitmap_argb32>> menu::toolbar_bitmap;
std::vector<std::shared_ptr<bitmap_argb32>> menu::sw_toolbar_bitmap;
std::vector<const game_driver *> menu::m_old_icons;

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  is_selectable - return TRUE if the given
//  item is selectable
//-------------------------------------------------

inline bool is_selectable(menu_item const &item)
{
	return ((item.flags & (menu::FLAG_MULTILINE | menu::FLAG_DISABLE)) == 0 && item.type != menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  exclusive_input_pressed - return TRUE if the
//  given key is pressed and we haven't already
//  reported a key
//-------------------------------------------------

inline bool menu::exclusive_input_pressed(int key, int repeat)
{
	if (m_event.iptkey == IPT_INVALID && machine().ui_input().pressed_repeat(key, repeat))
	{
		m_event.iptkey = key;
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

void menu::init(running_machine &machine, ui_options &mopt)
{
	// initialize the menu stack
	menu::stack_reset(machine);

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
	init_ui(machine, mopt);

	// add an exit callback to free memory
	machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(menu::exit), &machine));
}


//-------------------------------------------------
//  exit - clean up after ourselves
//-------------------------------------------------

void menu::exit(running_machine &machine)
{
	// free menus
	menu::stack_reset(machine);
	menu::clear_free_list(machine);

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

	for (int i = 0; i < UI_TOOLBAR_BUTTONS; ++i)
	{
		mre.texture_free(sw_toolbar_texture[i]);
		mre.texture_free(toolbar_texture[i]);
	}

	icons_bitmap.clear();
	sw_toolbar_bitmap.clear();
	toolbar_bitmap.clear();
	m_old_icons.clear();
}



/***************************************************************************
    CORE MENU MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  menu - menu constructor
//-------------------------------------------------

menu::menu(mame_ui_manager &mui, render_container *_container)
	: m_special_main_menu(false)
	, m_ui(mui)
	, m_parent()
	, m_pressed(false)
	, m_repeat(0)
	, m_event()
	, m_pool(nullptr)
	, m_focus(focused_menu::main)
	, m_resetpos(0)
	, m_resetref(nullptr)
{
	container = _container;

	reset(reset_options::SELECT_FIRST);

	top_line = 0;
}


//-------------------------------------------------
//  ~menu - menu destructor
//-------------------------------------------------

menu::~menu()
{
	// free the pools
	while (m_pool)
	{
		pool *const ppool = m_pool;
		m_pool = m_pool->next;
		global_free(ppool);
	}
}


//-------------------------------------------------
//  reset - free all items in the menu,
//  and all memory allocated from the memory pool
//-------------------------------------------------

void menu::reset(reset_options options)
{
	// based on the reset option, set the reset info
	m_resetpos = 0;
	m_resetref = nullptr;
	if (options == reset_options::REMEMBER_POSITION)
		m_resetpos = selected;
	else if (options == reset_options::REMEMBER_REF)
		m_resetref = get_selection_ref();

	// reset all the pools and the item.size() back to 0
	for (pool *ppool = m_pool; ppool != nullptr; ppool = ppool->next)
		ppool->top = (UINT8 *)(ppool + 1);
	item.clear();
	visitems = 0;
	selected = 0;

	// add an item to return
	if (!m_parent)
	{
		item_append(_("Return to Machine"), "", 0, nullptr);
	}
	else if (m_parent->is_special_main_menu())
	{
		if (machine().options().ui() == emu_options::UI_SIMPLE)
			item_append(_("Exit"), "", 0, nullptr);
		else
			item_append(_("Exit"), "", FLAG_UI | FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, nullptr);
	}
	else
	{
		if (machine().options().ui() != emu_options::UI_SIMPLE && menu::stack_has_special_main_menu())
			item_append(_("Return to Previous Menu"), "", FLAG_UI | FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, nullptr);
		else
			item_append(_("Return to Previous Menu"), "", 0, nullptr);
	}

}


//-------------------------------------------------
//  is_special_main_menu - returns whether the
//  menu has special needs
//-------------------------------------------------

bool menu::is_special_main_menu() const
{
	return m_special_main_menu;
}


//-------------------------------------------------
//  set_special_main_menu - set whether the
//  menu has special needs
//-------------------------------------------------

void menu::set_special_main_menu(bool special)
{
	m_special_main_menu = special;
}


//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void menu::item_append(menu_item item)
{
	item_append(item.text, item.subtext, item.flags, item.ref, item.type);
}

//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void menu::item_append(menu_item_type type, UINT32 flags)
{
	if (type == menu_item_type::SEPARATOR)
		item_append(MENU_SEPARATOR_ITEM, "", flags, nullptr, menu_item_type::SEPARATOR);
}

//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void menu::item_append(const std::string &text, const std::string &subtext, UINT32 flags, void *ref, menu_item_type type)
{
	item_append(std::string(text), std::string(subtext), flags, ref, type);
}

//-------------------------------------------------
//  item_append - append a new item to the
//  end of the menu
//-------------------------------------------------

void menu::item_append(std::string &&text, std::string &&subtext, UINT32 flags, void *ref, menu_item_type type)
{
	// only allow multiline as the first item
	if ((flags & FLAG_MULTILINE) != 0)
		assert(item.size() == 1);

	// only allow a single multi-line item
	else if (item.size() >= 2)
		assert((item[0].flags & FLAG_MULTILINE) == 0);

	// allocate a new item and populate it
	menu_item pitem;
	pitem.text = std::move(text);
	pitem.subtext = std::move(subtext);
	pitem.flags = flags;
	pitem.ref = ref;
	pitem.type = type;

	// append to array
	auto index = item.size();
	if (!item.empty())
	{
		item.emplace(item.end() - 1, std::move(pitem));
		--index;
	}
	else
		item.emplace_back(std::move(pitem));

	// update the selection if we need to
	if (m_resetpos == index || (m_resetref != nullptr && m_resetref == ref))
		selected = index;
	if (m_resetpos == (item.size() - 1))
		selected = item.size() - 1;
}


//-------------------------------------------------
//  process - process a menu, drawing it
//  and returning any interesting events
//-------------------------------------------------

const menu::event *menu::process(UINT32 flags, float x0, float y0)
{
	// reset the event
	m_event.iptkey = IPT_INVALID;

	// first make sure our selection is valid
	validate_selection(1);

	// draw the menu
	if (item.size() > 1 && (item[0].flags & FLAG_MULTILINE) != 0)
		draw_text_box();
	else if ((item[0].flags & FLAG_UI) != 0 || (item[0].flags & FLAG_UI_SWLIST) != 0)
		draw_select_game((flags & PROCESS_NOINPUT));
	else if ((item[0].flags & FLAG_UI_PALETTE) != 0)
		draw_palette_menu();
	else if ((item[0].flags & FLAG_UI_DATS) != 0)
		draw_dats_menu();
	else
		draw(flags, x0, y0);

	// process input
	if (!(flags & PROCESS_NOKEYS) && !(flags & PROCESS_NOINPUT))
	{
		// read events
		if ((item[0].flags & FLAG_UI ) != 0 || (item[0].flags & FLAG_UI_SWLIST ) != 0)
			handle_main_events();
		else
			handle_events(flags);

		// handle the keys if we don't already have an event
		if (m_event.iptkey == IPT_INVALID)
		{
			if ((item[0].flags & FLAG_UI) != 0 || (item[0].flags & FLAG_UI_SWLIST) != 0)
				handle_main_keys(flags);
			else
				handle_keys(flags);
		}
	}

	// update the selected item in the event
	if ((m_event.iptkey != IPT_INVALID) && selection_valid())
	{
		m_event.itemref = item[selected].ref;
		m_event.type = item[selected].type;
		return &m_event;
	}
	return nullptr;
}


//-------------------------------------------------
//  m_pool_alloc - allocate temporary memory
//  from the menu's memory pool
//-------------------------------------------------

void *menu::m_pool_alloc(size_t size)
{
	assert(size < UI_MENU_POOL_SIZE);

	// find a pool with enough room
	for (pool *ppool = m_pool; ppool != nullptr; ppool = ppool->next)
	{
		if (ppool->end - ppool->top >= size)
		{
			void *result = ppool->top;
			ppool->top += size;
			return result;
		}
	}

	// allocate a new pool
	pool *ppool = (pool *)global_alloc_array_clear<UINT8>(sizeof(*ppool) + UI_MENU_POOL_SIZE);

	// wire it up
	ppool->next = m_pool;
	m_pool = ppool;
	ppool->top = (UINT8 *)(ppool + 1);
	ppool->end = ppool->top + UI_MENU_POOL_SIZE;
	return m_pool_alloc(size);
}


//-------------------------------------------------
//  set_selection - changes the index
//  of the currently selected menu item
//-------------------------------------------------

void menu::set_selection(void *selected_itemref)
{
	selected = -1;
	for (int itemnum = 0; itemnum < item.size(); itemnum++)
	{
		if (item[itemnum].ref == selected_itemref)
		{
			selected = itemnum;
			break;
		}
	}
}



/***************************************************************************
    INTERNAL MENU PROCESSING
***************************************************************************/

//-------------------------------------------------
//  draw - draw a menu
//-------------------------------------------------

void menu::draw(UINT32 flags, float origx0, float origy0)
{
	// first draw the FPS counter
	if (ui().show_fps_counter())
	{
		ui().draw_text_full(container, machine().video().speed_text().c_str(), 0.0f, 0.0f, 1.0f,
				ui::text_layout::RIGHT, ui::text_layout::WORD, mame_ui_manager::OPAQUE_, rgb_t::white, rgb_t::black, nullptr, nullptr);
	}

	bool const customonly = (flags & PROCESS_CUSTOM_ONLY);
	bool const noimage = (flags & PROCESS_NOIMAGE);
	bool const noinput = (flags & PROCESS_NOINPUT);
	float const line_height = ui().get_line_height();
	float const lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float const ud_arrow_width = line_height * machine().render().ui_aspect();
	float const gutter_width = lr_arrow_width * 1.3f;

	if (ui().options().use_background_image() && &machine().system() == &GAME_NAME(___empty) && bgrnd_bitmap->valid() && !noimage)
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// compute the width and height of the full menu
	float visible_width = 0;
	float visible_main_menu_height = 0;
	for (auto const &pitem : item)
	{
		// compute width of left hand side
		float total_width = gutter_width + ui().get_string_width(pitem.text.c_str()) + gutter_width;

		// add in width of right hand side
		if (!pitem.subtext.empty())
			total_width += 2.0f * gutter_width + ui().get_string_width(pitem.subtext.c_str());

		// track the maximum
		if (total_width > visible_width)
			visible_width = total_width;

		// track the height as well
		visible_main_menu_height += line_height;
	}

	// account for extra space at the top and bottom
	float const visible_extra_menu_height = customtop + custombottom;

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
	if (visible_lines > item.size()) visible_lines = item.size();
	visible_main_menu_height = (float)visible_lines * line_height;

	// compute top/left of inner menu area by centering
	float const visible_left = (1.0f - visible_width) * 0.5f;
	float const visible_top = ((1.0f - visible_main_menu_height - visible_extra_menu_height) * 0.5f) + customtop;

	// first add us a box
	float const x1 = visible_left - UI_BOX_LR_BORDER;
	float const y1 = visible_top - UI_BOX_TB_BORDER;
	float const x2 = visible_left + visible_width + UI_BOX_LR_BORDER;
	float const y2 = visible_top + visible_main_menu_height + UI_BOX_TB_BORDER;
	if (!customonly)
		ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	if (top_line < 0 || is_first_selected())
		top_line = 0;
	if (selected >= (top_line + visible_lines))
		top_line = selected - (visible_lines / 2);
	if ((top_line > (item.size() - visible_lines)) || (selected == (item.size() - 1)))
		top_line = item.size() - visible_lines;

	// if scrolling, show arrows
	bool const show_top_arrow((item.size() > visible_lines) && (top_line > 0));
	bool const show_bottom_arrow((item.size() > visible_lines) && (top_line != (item.size() - visible_lines)));

	// set the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (show_top_arrow ? 1 : 0) - (show_bottom_arrow ? 1 : 0);

	// determine effective positions taking into account the hilighting arrows
	float const effective_width = visible_width - 2.0f * gutter_width;
	float const effective_left = visible_left + gutter_width;

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
	bool selected_subitem_too_big = false;
	float const line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
	float const line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
	if (!customonly)
	{
		for (int linenum = 0; linenum < visible_lines; linenum++)
		{
			auto const itemnum = top_line + linenum;
			menu_item const &pitem = item[itemnum];
			char const *const itemtext = pitem.text.c_str();
			rgb_t fgcolor = UI_TEXT_COLOR;
			rgb_t bgcolor = UI_TEXT_BG_COLOR;
			rgb_t fgcolor2 = UI_SUBITEM_COLOR;
			rgb_t fgcolor3 = UI_CLONE_COLOR;
			float const line_y0 = visible_top + (float)linenum * line_height;
			float const line_y1 = line_y0 + line_height;

			// set the hover if this is our item
			if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
				hover = itemnum;

			// if we're selected, draw with a different background
			if (itemnum == selected)
			{
				fgcolor = fgcolor2 = fgcolor3 = UI_SELECTED_COLOR;
				bgcolor = UI_SELECTED_BG_COLOR;
			}

			// else if the mouse is over this item, draw with a different background
			else if (itemnum == hover)
			{
				fgcolor = fgcolor2 = fgcolor3 = UI_MOUSEOVER_COLOR;
				bgcolor = UI_MOUSEOVER_BG_COLOR;
			}

			// if we have some background hilighting to do, add a quad behind everything else
			if (bgcolor != UI_TEXT_BG_COLOR)
				highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);

			// if we're on the top line, display the up arrow
			if (linenum == 0 && show_top_arrow)
			{
				draw_arrow(container,
							0.5f * (x1 + x2) - 0.5f * ud_arrow_width,
							line_y0 + 0.25f * line_height,
							0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
							line_y0 + 0.75f * line_height,
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
							line_y0 + 0.25f * line_height,
							0.5f * (x1 + x2) + 0.5f * ud_arrow_width,
							line_y0 + 0.75f * line_height,
							fgcolor,
							ROT0 ^ ORIENTATION_FLIP_Y);
				if (hover == itemnum)
					hover = HOVER_ARROW_DOWN;
			}

			// if we're just a divider, draw a line
			else if (pitem.type == menu_item_type::SEPARATOR)
				container->add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

			// if we don't have a subitem, just draw the string centered
			else if (pitem.subtext.empty())
			{
				if (pitem.flags & FLAG_UI_HEADING)
				{
					float heading_width = ui().get_string_width(itemtext);
					container->add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + ((visible_width - heading_width) / 2) - UI_BOX_LR_BORDER, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container->add_line(visible_left + visible_width - ((visible_width - heading_width) / 2) + UI_BOX_LR_BORDER, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				ui().draw_text_full(container, itemtext, effective_left, line_y0, effective_width,
					ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
			}

			// otherwise, draw the item on the left and the subitem text on the right
			else
			{
				bool const subitem_invert(pitem.flags & FLAG_INVERT);
				char const *subitem_text(pitem.subtext.c_str());
				float item_width, subitem_width;

				// draw the left-side text
				ui().draw_text_full(container, itemtext, effective_left, line_y0, effective_width,
					ui::text_layout::LEFT, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, &item_width, nullptr);

				// give 2 spaces worth of padding
				item_width += 2.0f * gutter_width;

				// if the subitem doesn't fit here, display dots
				if (ui().get_string_width(subitem_text) > effective_width - item_width)
				{
					subitem_text = "...";
					if (itemnum == selected)
						selected_subitem_too_big = true;
				}

				// customize subitem text color
				if (!core_stricmp(subitem_text, _("On")))
					fgcolor2 = rgb_t(0x00,0xff,0x00);

				if (!core_stricmp(subitem_text, _("Off")))
					fgcolor2 = rgb_t(0xff,0x00,0x00);

				if (!core_stricmp(subitem_text, _("Auto")))
					fgcolor2 = rgb_t(0xff,0xff,0x00);

				// draw the subitem right-justified
				ui().draw_text_full(container, subitem_text, effective_left + item_width, line_y0, effective_width - item_width,
							ui::text_layout::RIGHT, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, subitem_invert ? fgcolor3 : fgcolor2, bgcolor, &subitem_width, nullptr);

				// apply arrows
				if (itemnum == selected && (pitem.flags & FLAG_LEFT_ARROW))
				{
					draw_arrow(container,
								effective_left + effective_width - subitem_width - gutter_width,
								line_y0 + 0.1f * line_height,
								effective_left + effective_width - subitem_width - gutter_width + lr_arrow_width,
								line_y0 + 0.9f * line_height,
								fgcolor,
								ROT90 ^ ORIENTATION_FLIP_X);
				}
				if (itemnum == selected && (pitem.flags & FLAG_RIGHT_ARROW))
				{
					draw_arrow(container,
								effective_left + effective_width + gutter_width - lr_arrow_width,
								line_y0 + 0.1f * line_height,
								effective_left + effective_width + gutter_width,
								line_y0 + 0.9f * line_height,
								fgcolor,
								ROT90);
				}
			}
		}
	}

	// if the selected subitem is too big, display it in a separate offset box
	if (selected_subitem_too_big)
	{
		menu_item const &pitem = item[selected];
		bool const subitem_invert(pitem.flags & FLAG_INVERT);
		auto const linenum = selected - top_line;
		float const line_y = visible_top + (float)linenum * line_height;
		float target_width, target_height;

		// compute the multi-line target width/height
		ui().draw_text_full(container, pitem.subtext.c_str(), 0, 0, visible_width * 0.75f,
			ui::text_layout::RIGHT, ui::text_layout::WORD, mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &target_width, &target_height);

		// determine the target location
		float const target_x = visible_left + visible_width - target_width - UI_BOX_LR_BORDER;
		float target_y = line_y + line_height + UI_BOX_TB_BORDER;
		if (target_y + target_height + UI_BOX_TB_BORDER > visible_main_menu_height)
			target_y = line_y - target_height - UI_BOX_TB_BORDER;

		// add a box around that
		ui().draw_outlined_box(container, target_x - UI_BOX_LR_BORDER,
				target_y - UI_BOX_TB_BORDER,
				target_x + target_width + UI_BOX_LR_BORDER,
				target_y + target_height + UI_BOX_TB_BORDER,
				subitem_invert ? UI_SELECTED_BG_COLOR : UI_BACKGROUND_COLOR);

		ui().draw_text_full(container, pitem.subtext.c_str(), target_x, target_y, target_width,
				ui::text_layout::RIGHT, ui::text_layout::WORD, mame_ui_manager::NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, nullptr, nullptr);
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), customtop, custombottom, x1, y1, x2, y2);
}

void menu::custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2)
{
}

//-------------------------------------------------
//  draw_text_box - draw a multiline
//  word-wrapped text box with a menu item at the
//  bottom
//-------------------------------------------------

void menu::draw_text_box()
{
	const char *text = item[0].text.c_str();
	const char *backtext = item[1].text.c_str();
	float line_height = ui().get_line_height();
	float lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	float gutter_width = lr_arrow_width;
	float target_width, target_height, prior_width;
	float target_x, target_y;

	// compute the multi-line target width/height
	ui().draw_text_full(container, text, 0, 0, 1.0f - 2.0f * UI_BOX_LR_BORDER - 2.0f * gutter_width,
		ui::text_layout::LEFT, ui::text_layout::WORD, mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &target_width, &target_height);
	target_height += 2.0f * line_height;
	if (target_height > 1.0f - 2.0f * UI_BOX_TB_BORDER)
		target_height = floorf((1.0f - 2.0f * UI_BOX_TB_BORDER) / line_height) * line_height;

	// maximum against "return to prior menu" text
	prior_width = ui().get_string_width(backtext) + 2.0f * gutter_width;
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
	ui().draw_outlined_box(container, target_x - UI_BOX_LR_BORDER - gutter_width,
							target_y - UI_BOX_TB_BORDER,
							target_x + target_width + gutter_width + UI_BOX_LR_BORDER,
							target_y + target_height + UI_BOX_TB_BORDER,
							(item[0].flags & FLAG_REDTEXT) ?  UI_RED_COLOR : UI_BACKGROUND_COLOR);
	ui().draw_text_full(container, text, target_x, target_y, target_width,
		ui::text_layout::LEFT, ui::text_layout::WORD, mame_ui_manager::NORMAL, UI_TEXT_COLOR, UI_TEXT_BG_COLOR, nullptr, nullptr);

	// draw the "return to prior menu" text with a hilight behind it
	highlight(container,
				target_x + 0.5f * UI_LINE_WIDTH,
				target_y + target_height - line_height,
				target_x + target_width - 0.5f * UI_LINE_WIDTH,
				target_y + target_height,
				UI_SELECTED_BG_COLOR);
	ui().draw_text_full(container, backtext, target_x, target_y + target_height - line_height, target_width,
		ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, UI_SELECTED_COLOR, UI_SELECTED_BG_COLOR, nullptr, nullptr);

	// artificially set the hover to the last item so a double-click exits
	hover = item.size() - 1;
}


//-------------------------------------------------
//  handle_events - generically handle
//  input events for a menu
//-------------------------------------------------

void menu::handle_events(UINT32 flags)
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
				if (custom_mouse_down())
					return;

				if ((flags & PROCESS_ONLYCHAR) == 0)
				{
					if (hover >= 0 && hover < item.size())
						selected = hover;
					else if (hover == HOVER_ARROW_UP)
					{
						if ((flags & FLAG_UI_DATS) != 0)
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
						if ((flags & FLAG_UI_DATS) != 0)
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
				if (!(flags & PROCESS_ONLYCHAR) && hover >= 0 && hover < item.size())
				{
					selected = hover;
					m_event.iptkey = IPT_UI_SELECT;
					if (is_last_selected())
					{
						m_event.iptkey = IPT_UI_CANCEL;
						menu::stack_pop(machine());
					}
					stop = true;
				}
				break;

			// caught scroll event
			case UI_EVENT_MOUSE_WHEEL:
				if (!(flags & PROCESS_ONLYCHAR))
				{
					if (local_menu_event.zdelta > 0)
					{
						if ((flags & FLAG_UI_DATS) != 0)
						{
							top_line -= local_menu_event.num_lines;
							return;
						}
						is_first_selected() ? selected = top_line = item.size() - 1 : selected -= local_menu_event.num_lines;
						validate_selection(-1);
						top_line -= (selected <= top_line && top_line != 0);
						if (selected <= top_line && visitems != visible_lines)
							top_line -= local_menu_event.num_lines;
					}
					else
					{
						if ((flags & FLAG_UI_DATS))
						{
							top_line += local_menu_event.num_lines;
							return;
						}
						is_last_selected() ? selected = top_line = 0 : selected += local_menu_event.num_lines;
						validate_selection(1);
						top_line += (selected >= top_line + visitems + (top_line != 0));
						if (selected >= (top_line + visitems + (top_line != 0)))
							top_line += local_menu_event.num_lines;
					}
				}
				break;

			// translate CHAR events into specials
			case UI_EVENT_CHAR:
				m_event.iptkey = IPT_SPECIAL;
				m_event.unichar = local_menu_event.ch;
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

void menu::handle_keys(UINT32 flags)
{
	bool ignorepause = menu::stack_has_special_main_menu();
	int code;

	// bail if no items
	if (item.empty())
		return;

	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (is_last_selected())
		{
			m_event.iptkey = IPT_UI_CANCEL;
			menu::stack_pop(machine());
		}
		return;
	}

	// bail out
	if ((flags & PROCESS_ONLYCHAR))
		return;

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		if (!menu_has_search_active())
			menu::stack_pop(machine());
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool ignoreleft = ((item[selected].flags & FLAG_LEFT_ARROW) == 0);
	bool ignoreright = ((item[selected].flags & FLAG_RIGHT_ARROW) == 0);

	if ((item[0].flags & FLAG_UI_DATS))
		ignoreleft = ignoreright = false;

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		if ((item[0].flags & FLAG_UI_DATS))
		{
			top_line--;
			return;
		}
		is_first_selected() ? selected = top_line = item.size() - 1 : --selected;
		validate_selection(-1);
		top_line -= (selected <= top_line && top_line != 0);
		if (selected <= top_line && visitems != visible_lines)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(IPT_UI_DOWN, 6))
	{
		if ((item[0].flags & FLAG_UI_DATS))
		{
			top_line++;
			return;
		}
		is_last_selected() ? selected = top_line = 0 : ++selected;
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
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled());

	// see if any other UI keys are pressed
	if (m_event.iptkey == IPT_INVALID)
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

void menu::validate_selection(int scandir)
{
	// clamp to be in range
	if (selected < 0)
		selected = 0;
	else if (selected >= item.size())
		selected = item.size() - 1;

	// skip past unselectable items
	while (!is_selectable(item[selected]))
		selected = (selected + item.size() + scandir) % item.size();
}



//-------------------------------------------------
//  clear_free_list - clear out anything
//  accumulated in the free list
//-------------------------------------------------

void menu::clear_free_list(running_machine &machine)
{
	while (menu_free)
		menu_free = std::move(menu_free->m_parent);
}



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  menu::stack_reset - reset the menu stack
//-------------------------------------------------

void menu::stack_reset(running_machine &machine)
{
	while (menu_stack)
		menu::stack_pop(machine);
}


//-------------------------------------------------
//  stack_push - push a new menu onto the
//  stack
//-------------------------------------------------

void menu::stack_push(std::unique_ptr<menu> &&menu)
{
	menu->m_parent = std::move(menu_stack);
	menu_stack = std::move(menu);
	menu_stack->reset(reset_options::SELECT_FIRST);
	menu_stack->machine().ui_input().reset();
}


//-------------------------------------------------
//  stack_pop - pop a menu from the stack
//-------------------------------------------------

void menu::stack_pop(running_machine &machine)
{
	if (menu_stack)
	{
		std::unique_ptr<menu> menu(std::move(menu_stack));
		menu_stack = std::move(menu->m_parent);
		menu->m_parent = std::move(menu_free);
		menu_free = std::move(menu);
		machine.ui_input().reset();
	}
}


//-------------------------------------------------
//  menu::stack_has_special_main_menu -
//  check in the special main menu is in the stack
//-------------------------------------------------

bool menu::stack_has_special_main_menu()
{
	for (auto menu = menu_stack.get(); menu != nullptr; menu = menu->m_parent.get())
		if (menu->is_special_main_menu())
			return true;

	return false;
}

void menu::do_handle()
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

UINT32 menu::ui_handler(render_container *container, mame_ui_manager &mui)
{
	// if we have no menus stacked up, start with the main menu
	if (!menu_stack)
		stack_push(std::unique_ptr<menu>(global_alloc_clear<menu_main>(mui, container)));

	// update the menu state
	if (menu_stack)
		menu_stack->do_handle();

	// clear up anything pending to be released
	clear_free_list(mui.machine());

	// if the menus are to be hidden, return a cancel here
	if (mui.is_menu_active() && ((mui.machine().ui_input().pressed(IPT_UI_CONFIGURE) && !stack_has_special_main_menu()) || menu_stack == nullptr))
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

void menu::render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param)
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

void menu::highlight(render_container *container, float x0, float y0, float x1, float y1, rgb_t bgcolor)
{
	container->add_quad(x0, y0, x1, y1, bgcolor, hilight_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE) | PRIMFLAG_PACKABLE);
}


//-------------------------------------------------
//  draw_arrow
//-------------------------------------------------

void menu::draw_arrow(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation)
{
	container->add_quad(x0, y0, x1, y1, fgcolor, arrow_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(orientation) | PRIMFLAG_PACKABLE);
}

//-------------------------------------------------
//  init - initialize the ui menu system
//-------------------------------------------------

void menu::init_ui(running_machine &machine, ui_options &mopt)
{
	render_manager &mrender = machine.render();
	// create a texture for hilighting items in main menu
	hilight_main_bitmap = std::make_unique<bitmap_rgb32>(1, 128);
	int r1 = 0, g1 = 169, b1 = 255; //Any start color
	int r2 = 0, g2 = 39, b2 = 130; //Any stop color
	for (int y = 0; y < 128; y++)
	{
		int r = r1 + (y * (r2 - r1) / 128);
		int g = g1 + (y * (g2 - g1) / 128);
		int b = b1 + (y * (b2 - b1) / 128);
		hilight_main_bitmap->pix32(y, 0) = rgb_t(r, g, b);
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
	for (auto & icons : icons_texture)
	{
		m_old_icons.emplace_back(nullptr);
		icons_bitmap.emplace_back(std::make_unique<bitmap_argb32>());
		icons = mrender.texture_alloc();
	}

	// create a texture for main menu background
	bgrnd_bitmap = std::make_unique<bitmap_argb32>(0, 0);
	bgrnd_texture = mrender.texture_alloc(render_texture::hq_scale);

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
		toolbar_bitmap.emplace_back(std::make_shared<bitmap_argb32>(32, 32));
		sw_toolbar_bitmap.emplace_back(std::make_shared<bitmap_argb32>(32, 32));
		toolbar_texture[x] = mrender.texture_alloc();
		sw_toolbar_texture[x] = mrender.texture_alloc();
		UINT32 *texture_dst = &toolbar_bitmap.back()->pix32(0);
		memcpy(texture_dst, toolbar_bitmap_bmp[x], 32 * 32 * sizeof(UINT32));
		if (toolbar_bitmap.back()->valid())
			toolbar_texture[x]->set_bitmap(*toolbar_bitmap.back(), toolbar_bitmap.back()->cliprect(), TEXFORMAT_ARGB32);
		else
			toolbar_bitmap.back()->reset();

		if (x == 0 || x == 2)
		{
			texture_dst = &sw_toolbar_bitmap[x]->pix32(0);
			memcpy(texture_dst, toolbar_bitmap_bmp[x], 32 * 32 * sizeof(UINT32));
			if (sw_toolbar_bitmap.back()->valid())
				sw_toolbar_texture[x]->set_bitmap(*sw_toolbar_bitmap.back(), sw_toolbar_bitmap.back()->cliprect(), TEXFORMAT_ARGB32);
			else
				sw_toolbar_bitmap.back()->reset();
		}
		else
			sw_toolbar_bitmap.back()->reset();

	}
}


//-------------------------------------------------
//  draw main menu
//-------------------------------------------------

void menu::draw_select_game(UINT32 flags)
{
	bool noinput = (flags & PROCESS_NOINPUT);
	float line_height = ui().get_line_height();
	float ud_arrow_width = line_height * machine().render().ui_aspect();
	float gutter_width = 0.52f * ud_arrow_width;
	mouse_x = -1, mouse_y = -1;
	float right_panel_size = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL) ? 2.0f * UI_BOX_LR_BORDER : 0.3f;
	float visible_width = 1.0f - 4.0f * UI_BOX_LR_BORDER;
	float primary_left = (1.0f - visible_width) * 0.5f;
	float primary_width = visible_width;
	bool is_swlist = (item[0].flags & FLAG_UI_SWLIST);

	// draw background image if available
	if (ui().options().use_background_image() && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

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
		if (mouse_target)
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
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	if (visible_items < visible_lines)
		visible_lines = visible_items;
	if (top_line < 0 || is_first_selected())
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
		const menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text.c_str();
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		rgb_t fgcolor3 = UI_CLONE_COLOR;
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line_y;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
			hover = itemnum;

		// if we're selected, draw with a different background
		if (itemnum == selected && m_focus == focused_menu::main)
		{
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			fgcolor3 = rgb_t(0xcc, 0xcc, 0x00);
			ui().draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(43, 43, 43),
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
			ui().draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(43, 43, 43),
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
		else if (pitem.type == menu_item_type::SEPARATOR)
			container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// draw the item centered
		else if (pitem.subtext.empty())
		{
			int item_invert = pitem.flags & FLAG_INVERT;
			auto icon = draw_icon(linenum, item[itemnum].ref, effective_left, line_y);
			ui().draw_text_full(container, itemtext, effective_left + icon, line_y, effective_width - icon, ui::text_layout::LEFT, ui::text_layout::TRUNCATE,
				mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, nullptr, nullptr);
		}
		else
		{
			auto item_invert = pitem.flags & FLAG_INVERT;
			const char *subitem_text = pitem.subtext.c_str();
			float item_width, subitem_width;

			// compute right space for subitem
			ui().draw_text_full(container, subitem_text, effective_left, line_y, ui().get_string_width(pitem.subtext.c_str()),
				ui::text_layout::RIGHT, ui::text_layout::NEVER, mame_ui_manager::NONE, item_invert ? fgcolor3 : fgcolor, bgcolor, &subitem_width, nullptr);
			subitem_width += gutter_width;

			// draw the item left-justified
			ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width - subitem_width,
				ui::text_layout::LEFT, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, &item_width, nullptr);

			// draw the subitem right-justified
			ui().draw_text_full(container, subitem_text, effective_left + item_width, line_y, effective_width - item_width,
				ui::text_layout::RIGHT, ui::text_layout::NEVER, mame_ui_manager::NORMAL, item_invert ? fgcolor3 : fgcolor, bgcolor, nullptr, nullptr);
		}
	}

	for (size_t count = visible_items; count < item.size(); count++)
	{
		const menu_item &pitem = item[count];
		const char *itemtext = pitem.text.c_str();
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;

		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
			hover = count;

		// if we're selected, draw with a different background
		if (count == selected && m_focus == focused_menu::main)
		{
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(container, line_x0 + 0.01f, line_y0, line_x1 - 0.01f, line_y1, bgcolor, rgb_t(43, 43, 43),
				hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}
		// else if the mouse is over this item, draw with a different background
		else if (count == hover)
		{
			fgcolor = UI_MOUSEOVER_COLOR;
			bgcolor = UI_MOUSEOVER_BG_COLOR;
			highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
		}

		if (pitem.type == menu_item_type::SEPARATOR)
			container->add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
			ui().draw_text_full(container, itemtext, effective_left, line, effective_width, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		line += line_height;
	}

	x1 = x2;
	x2 += right_panel_size;

	draw_right_panel(get_selection_ref(), x1, y1, x2, y2);

	x1 = primary_left - UI_BOX_LR_BORDER;
	x2 = primary_left + primary_width + UI_BOX_LR_BORDER;

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != visible_items);

	// reset redraw icon stage
	if (!is_swlist) ui_globals::redraw_icon = false;

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

void menu::get_title_search(std::string &snaptext, std::string &searchstr)
{
	// get arts title text
	snaptext.assign(_(arts_info[ui_globals::curimage_view].first));

	// get search path
	std::string addpath;
	if (ui_globals::curimage_view == SNAPSHOT_VIEW)
	{
		emu_options moptions;
		searchstr = machine().options().value(arts_info[ui_globals::curimage_view].second);
		addpath = moptions.value(arts_info[ui_globals::curimage_view].second);
	}
	else
	{
		ui_options moptions;
		searchstr = ui().options().value(arts_info[ui_globals::curimage_view].second);
		addpath = moptions.value(arts_info[ui_globals::curimage_view].second);
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

void menu::handle_main_keys(UINT32 flags)
{
	auto ignorepause = menu::stack_has_special_main_menu();

	// bail if no items
	if (item.size() == 0)
		return;

	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(IPT_UI_SELECT, 0))
	{
		if (is_last_selected() && m_focus == focused_menu::main)
		{
			m_event.iptkey = IPT_UI_CANCEL;
			menu::stack_pop(machine());
		}
		return;
	}

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(IPT_UI_CANCEL, 0))
	{
		if (!ui_error && !menu_has_search_active())
			menu::stack_pop(machine());
		return;
	}

	// validate the current selection
	validate_selection(1);

	// swallow left/right keys if they are not appropriate
	bool ignoreleft = ((item[selected].flags & FLAG_LEFT_ARROW) == 0);
	bool ignoreright = ((item[selected].flags & FLAG_RIGHT_ARROW) == 0);
	bool leftclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_LEFT_PANEL);
	bool rightclose = (ui_globals::panels_status == HIDE_BOTH || ui_globals::panels_status == HIDE_RIGHT_PANEL);

	// accept left/right keys as-is with repeat
	if (!ignoreleft && exclusive_input_pressed(IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::righttop)
			m_event.iptkey = IPT_UI_LEFT_PANEL;
		return;
	}

	if (!ignoreright && exclusive_input_pressed(IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
	{
		// Swap the right panel
		if (m_focus == focused_menu::righttop)
			m_event.iptkey = IPT_UI_RIGHT_PANEL;
		return;
	}

	// up backs up by one item
	if (exclusive_input_pressed(IPT_UI_UP, 6))
	{
		// Filter
		if (!leftclose && m_focus == focused_menu::left)
		{
			m_event.iptkey = IPT_UI_UP_FILTER;
			return;
		}

		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			m_event.iptkey = IPT_UI_UP_PANEL;
			topline_datsview--;
			return;
		}

		if (selected == visible_items + 1 || is_first_selected() || ui_error)
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
			m_event.iptkey = IPT_UI_DOWN_FILTER;
			return;
		}

		// Infos
		if (!rightclose && m_focus == focused_menu::rightbottom)
		{
			m_event.iptkey = IPT_UI_DOWN_PANEL;
			topline_datsview++;
			return;
		}

		if (is_last_selected() || selected == visible_items - 1 || ui_error)
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
			m_event.iptkey = IPT_UI_DOWN_PANEL;
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
			m_event.iptkey = IPT_UI_DOWN_PANEL;
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
			m_event.iptkey = IPT_UI_DOWN_PANEL;
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
			m_event.iptkey = IPT_UI_DOWN_PANEL;
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
		mame_machine_manager::instance()->cheat().set_enable(!mame_machine_manager::instance()->cheat().enabled());

	// see if any other UI keys are pressed
	if (m_event.iptkey == IPT_INVALID)
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

void menu::handle_main_events()
{
	auto stop = false;
	ui_event local_menu_event;

	if (m_pressed)
	{
		bool pressed = mouse_pressed();
		INT32 m_target_x, m_target_y;
		bool m_button;
		auto mouse_target = machine().ui_input().find_mouse(&m_target_x, &m_target_y, &m_button);
		if (mouse_target && m_button && (hover == HOVER_ARROW_DOWN || hover == HOVER_ARROW_UP))
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
					m_event.iptkey = IPT_OTHER;
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
						m_event.iptkey = IPT_UI_RIGHT;
					else if (hover == HOVER_UI_LEFT)
						m_event.iptkey = IPT_UI_LEFT;
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
						m_event.iptkey = IPT_UI_FAVORITES;
						stop = true;
					}
					else if (hover == HOVER_B_EXPORT)
					{
						m_event.iptkey = IPT_UI_EXPORT;
						stop = true;
					}
					else if (hover == HOVER_B_DATS)
					{
						m_event.iptkey = IPT_UI_DATS;
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
						m_event.iptkey = IPT_OTHER;
						stop = true;
					}
					else if (hover >= HOVER_FILTER_FIRST && hover <= HOVER_FILTER_LAST)
					{
						l_hover = (HOVER_FILTER_FIRST - hover) * (-1);
						m_event.iptkey = IPT_OTHER;
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
					m_event.iptkey = IPT_UI_SELECT;
				}

				if (selected == item.size() - 1)
				{
					m_event.iptkey = IPT_UI_CANCEL;
					menu::stack_pop(machine());
				}
				stop = true;
				break;

			// caught scroll event
			case UI_EVENT_MOUSE_WHEEL:
				if (hover >= 0 && hover < item.size() - skip_main_items - 1)
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
				if (exclusive_input_pressed(IPT_UI_CONFIGURE, 0))
				{
					m_event.iptkey = IPT_UI_CONFIGURE;
					stop = true;
				}
				else
				{
					m_event.iptkey = IPT_SPECIAL;
					m_event.unichar = local_menu_event.ch;
					stop = true;
				}
				break;

			case UI_EVENT_MOUSE_RDOWN:
				if (hover >= 0 && hover < item.size() - skip_main_items - 1)
				{
					selected = hover;
					m_prev_selected = item[selected].ref;
					m_focus = focused_menu::main;
					m_event.iptkey = IPT_CUSTOM;
					m_event.mouse.x0 = local_menu_event.mouse_x;
					m_event.mouse.y0 = local_menu_event.mouse_y;
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

float menu::draw_right_box_title(float x1, float y1, float x2, float y2)
{
	auto line_height = ui().get_line_height();
	float midl = (x2 - x1) * 0.5f;

	// add outlined box for options
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// add separator line
	container->add_line(x1 + midl, y1, x1 + midl, y1 + line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	std::string buffer[RP_LAST + 1];
	buffer[RP_IMAGES] = _("Images");
	buffer[RP_INFOS] = _("Infos");

	// check size
	float text_size = 1.0f;
	for (auto & elem : buffer)
	{
		auto textlen = ui().get_string_width(elem.c_str()) + 0.01f;
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
			fgcolor = rgb_t(0xff, 0xff, 0x00);
			bgcolor = rgb_t(0xff, 0xff, 0xff);
			ui().draw_textured_box(container, x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
				bgcolor, rgb_t(43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		}
		else if (bgcolor == UI_MOUSEOVER_BG_COLOR)
			container->add_rect(x1 + UI_LINE_WIDTH, y1 + UI_LINE_WIDTH, x1 + midl - UI_LINE_WIDTH, y1 + line_height,
				bgcolor, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

		ui().draw_text_full(container, buffer[cells].c_str(), x1 + UI_LINE_WIDTH, y1, midl - UI_LINE_WIDTH,
			ui::text_layout::CENTER, ui::text_layout::NEVER, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, text_size);
		x1 += midl;
	}

	return (y1 + line_height + UI_LINE_WIDTH);
}

//-------------------------------------------------
//  common function for images render
//-------------------------------------------------

std::string menu::arts_render_common(float origx1, float origy1, float origx2, float origy2)
{
	auto line_height = ui().get_line_height();
	std::string snaptext, searchstr;
	auto title_size = 0.0f;
	auto txt_lenght = 0.0f;
	auto gutter_width = 0.4f * line_height * machine().render().ui_aspect() * 1.3f;

	get_title_search(snaptext, searchstr);

	// apply title to right panel
	for (int x = FIRST_VIEW; x < LAST_VIEW; x++)
	{
		ui().draw_text_full(container, _(arts_info[x].first), origx1, origy1, origx2 - origx1, ui::text_layout::CENTER,
			ui::text_layout::TRUNCATE, mame_ui_manager::NONE, rgb_t::white, rgb_t::black, &txt_lenght, nullptr);
		txt_lenght += 0.01f;
		title_size = MAX(txt_lenght, title_size);
	}

	rgb_t fgcolor = (m_focus == focused_menu::rightbottom) ? rgb_t(0xff, 0xff, 0x00) : UI_TEXT_COLOR;
	rgb_t bgcolor = (m_focus == focused_menu::rightbottom) ? rgb_t(0xff, 0xff, 0xff) : UI_TEXT_BG_COLOR;
	float middle = origx2 - origx1;

	// check size
	float sc = title_size + 2.0f * gutter_width;
	float tmp_size = (sc > middle) ? ((middle - 2.0f * gutter_width) / sc) : 1.0f;
	title_size *= tmp_size;

	if (bgcolor != UI_TEXT_BG_COLOR)
		ui().draw_textured_box(container, origx1 + ((middle - title_size) * 0.5f), origy1, origx1 + ((middle + title_size) * 0.5f),
			origy1 + line_height, bgcolor, rgb_t(43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));

	ui().draw_text_full(container, snaptext.c_str(), origx1, origy1, origx2 - origx1, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
		mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr, tmp_size);

	draw_common_arrow(origx1, origy1, origx2, origy2, ui_globals::curimage_view, FIRST_VIEW, LAST_VIEW, title_size);

	return searchstr;
}

//-------------------------------------------------
//  draw favorites star
//-------------------------------------------------

void menu::draw_star(float x0, float y0)
{
	float y1 = y0 + ui().get_line_height();
	float x1 = x0 + ui().get_line_height() * container->manager().ui_aspect();
	container->add_quad(x0, y0, x1, y1, rgb_t::white, star_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_PACKABLE);
}

//-------------------------------------------------
//  draw toolbar
//-------------------------------------------------

void menu::draw_toolbar(float x1, float y1, float x2, float y2, bool software)
{
	// draw a box
	ui().draw_outlined_box(container, x1, y1, x2, y2, rgb_t(0xEF, 0x12, 0x47, 0x7B));

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	x2 -= UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;
	y2 -= UI_BOX_TB_BORDER;

	render_texture **t_texture = (software) ? sw_toolbar_texture : toolbar_texture;
	auto t_bitmap = (software) ? sw_toolbar_bitmap : toolbar_bitmap;

	int m_valid = 0;
	for (auto & e : t_bitmap)
		if (e->valid()) m_valid++;

	float space_x = (y2 - y1) * container->manager().ui_aspect(container);
	auto total = (float)(m_valid * space_x) + ((float)(m_valid - 1) * 0.001f);
	x1 += (x2 - x1) * 0.5f - total * 0.5f;
	x2 = x1 + space_x;

	for (int z = 0; z < UI_TOOLBAR_BUTTONS; ++z)
		if (t_bitmap[z]->valid())
		{
			rgb_t color(0xEFEFEFEF);
			if (mouse_hit && x1 <= mouse_x && x2 > mouse_x && y1 <= mouse_y && y2 > mouse_y)
			{
				hover = HOVER_B_FAV + z;
				color = rgb_t::white;
				float ypos = y2 + ui().get_line_height() + 2.0f * UI_BOX_TB_BORDER;
				ui().draw_text_box(container, _(hover_msg[z]), ui::text_layout::CENTER, 0.5f, ypos, UI_BACKGROUND_COLOR);
			}

			container->add_quad(x1, y1, x2, y2, color, t_texture[z], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			x1 += space_x + ((z < UI_TOOLBAR_BUTTONS - 1) ? 0.001f : 0.0f);
			x2 = x1 + space_x;
		}
}


//-------------------------------------------------
//  perform rendering of image
//-------------------------------------------------

void menu::arts_render_images(bitmap_argb32 *tmp_bitmap, float origx1, float origy1, float origx2, float origy2)
{
	bool no_available = false;
	float line_height = ui().get_line_height();

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
		auto ratioW = (float)panel_width_pixel / tmp_bitmap->width();
		auto ratioH = (float)panel_height_pixel / tmp_bitmap->height();
		auto ratioI = (float)tmp_bitmap->height() / tmp_bitmap->width();
		auto dest_xPixel = tmp_bitmap->width();
		auto dest_yPixel = tmp_bitmap->height();

		// force 4:3 ratio min
		if (ui().options().forced_4x3_snapshot() && ratioI < 0.75f && ui_globals::curimage_view == SNAPSHOT_VIEW)
		{
			// smaller ratio will ensure that the image fits in the view
			dest_yPixel = tmp_bitmap->width() * 0.75f;
			ratioH = (float)panel_height_pixel / dest_yPixel;
			float ratio = MIN(ratioW, ratioH);
			dest_xPixel = tmp_bitmap->width() * ratio;
			dest_yPixel *= ratio;
		}
		// resize the bitmap if necessary
		else if (ratioW < 1 || ratioH < 1 || (ui().options().enlarge_snaps() && !no_available))
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

void menu::draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title_size)
{
	auto line_height = ui().get_line_height();
	auto lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	auto gutter_width = lr_arrow_width * 1.3f;

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
		ui().draw_textured_box(container, ar_x0 + 0.01f, ar_y0, ar_x1 - 0.01f, ar_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(43, 43, 43),
			hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = HOVER_UI_RIGHT;
		fgcolor_right = UI_MOUSEOVER_COLOR;
	}
	else if (mouse_hit && al_x0 <= mouse_x && al_x1 > mouse_x && al_y0 <= mouse_y && al_y1 > mouse_y && current != dmin)
	{
		ui().draw_textured_box(container, al_x0 + 0.01f, al_y0, al_x1 - 0.01f, al_y1, UI_MOUSEOVER_BG_COLOR, rgb_t(43, 43, 43),
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

float menu::draw_icon(int linenum, void *selectedref, float x0, float y0)
{
	if (!ui_globals::has_icons || (item[0].flags & FLAG_UI_SWLIST))
		return 0.0f;

	float ud_arrow_width = ui().get_line_height() * container->manager().ui_aspect(container);
	const game_driver *driver = nullptr;

	if (item[0].flags & FLAG_UI_FAVORITE)
	{
		ui_software_info *soft = (ui_software_info *)selectedref;
		if (soft->startempty == 1)
			driver = soft->driver;
	}
	else
		driver = (const game_driver *)selectedref;

	auto x1 = x0 + ud_arrow_width;
	auto y1 = y0 + ui().get_line_height();

	if (m_old_icons[linenum] != driver || ui_globals::redraw_icon)
	{
		m_old_icons[linenum] = driver;

		// set clone status
		bool cloneof = strcmp(driver->parent, "0");
		if (cloneof)
		{
			auto cx = driver_list::find(driver->parent);
			if (cx != -1 && ((driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0))
				cloneof = false;
		}

		// get search path
		path_iterator path(ui().options().icons_directory());
		std::string curpath;
		std::string searchstr(ui().options().icons_directory());

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
			auto screen_width = machine().render().ui_target().width();
			auto screen_height = machine().render().ui_target().height();

			if (machine().render().ui_target().orientation() & ORIENTATION_SWAP_XY)
				std::swap(screen_height, screen_width);

			int panel_width_pixel = panel_width * screen_width;
			int panel_height_pixel = panel_height * screen_height;

			// Calculate resize ratios for resizing
			auto ratioW = (float)panel_width_pixel / tmp->width();
			auto ratioH = (float)panel_height_pixel / tmp->height();
			auto dest_xPixel = tmp->width();
			auto dest_yPixel = tmp->height();

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
		container->add_quad(x0, y0, x1, y1, rgb_t::white, icons_texture[linenum], PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	return ud_arrow_width * 1.5f;
}

//-------------------------------------------------
//  draw info arrow
//-------------------------------------------------

void menu::info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width)
{
	rgb_t fgcolor = UI_TEXT_COLOR;
	UINT32 orientation = (!ub) ? ROT0 : ROT0 ^ ORIENTATION_FLIP_Y;

	if (mouse_hit && origx1 <= mouse_x && origx2 > mouse_x && oy1 <= mouse_y && oy1 + (line_height * text_size) > mouse_y)
	{
		ui().draw_textured_box(container, origx1 + 0.01f, oy1, origx2 - 0.01f, oy1 + (line_height * text_size), UI_MOUSEOVER_BG_COLOR,
			rgb_t(43, 43, 43), hilight_main_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXWRAP(TRUE));
		hover = (!ub) ? HOVER_DAT_UP : HOVER_DAT_DOWN;
		fgcolor = UI_MOUSEOVER_COLOR;
	}

	draw_arrow(container, 0.5f * (origx1 + origx2) - 0.5f * (ud_arrow_width * text_size), oy1 + 0.25f * (line_height * text_size),
		0.5f * (origx1 + origx2) + 0.5f * (ud_arrow_width * text_size), oy1 + 0.75f * (line_height * text_size), fgcolor, orientation);
}

//-------------------------------------------------
//  draw - draw palette menu
//-------------------------------------------------

void menu::draw_palette_menu()
{
	auto line_height = ui().get_line_height();
	auto lr_arrow_width = 0.4f * line_height * machine().render().ui_aspect();
	auto ud_arrow_width = line_height * machine().render().ui_aspect();
	auto gutter_width = lr_arrow_width * 1.3f;
	int itemnum, linenum;

	if (ui().options().use_background_image() && &machine().system() == &GAME_NAME(___empty) && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

	// compute the width and height of the full menu
	auto visible_width = 0.0f;
	auto visible_main_menu_height = 0.0f;
	for (auto & pitem : item)
	{
		// compute width of left hand side
		auto total_width = gutter_width + ui().get_string_width(pitem.text.c_str()) + gutter_width;

		// add in width of right hand side
		if (!pitem.subtext.empty())
			total_width += 2.0f * gutter_width + ui().get_string_width(pitem.subtext.c_str());

		// track the maximum
		if (total_width > visible_width)
			visible_width = total_width;

		// track the height as well
		visible_main_menu_height += line_height;
	}

	// account for extra space at the top and bottom
	auto visible_extra_menu_height = customtop + custombottom;

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
	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

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
		const menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text.c_str();
		rgb_t fgcolor = UI_TEXT_COLOR;
		rgb_t bgcolor = UI_TEXT_BG_COLOR;
		float line_y0 = line_y;
		float line_y1 = line_y + line_height;

		// set the hover if this is our item
		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
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
		else if (pitem.type == menu_item_type::SEPARATOR)
			container->add_line(visible_left, line_y + 0.5f * line_height, visible_left + visible_width, line_y + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

		// if we don't have a subitem, just draw the string centered
		else if (pitem.subtext.empty())
			ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);

		// otherwise, draw the item on the left and the subitem text on the right
		else
		{
			const char *subitem_text = pitem.subtext.c_str();
			rgb_t color = rgb_t((UINT32)strtoul(subitem_text, nullptr, 16));

			// draw the left-side text
			ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width,
				ui::text_layout::LEFT, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);

			// give 2 spaces worth of padding
			float subitem_width = ui().get_string_width("FF00FF00");

			ui().draw_outlined_box(container, effective_left + effective_width - subitem_width, line_y0,
				effective_left + effective_width, line_y1, color);
		}
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != item.size());
}

//-------------------------------------------------
//  draw - draw dats menu
//-------------------------------------------------

void menu::draw_dats_menu()
{
	auto line_height = ui().get_line_height();
	auto ud_arrow_width = line_height * machine().render().ui_aspect();
	auto gutter_width = 0.52f * line_height * machine().render().ui_aspect();
	mouse_x = -1, mouse_y = -1;
	float visible_width = 1.0f - 2.0f * UI_BOX_LR_BORDER;
	float visible_left = (1.0f - visible_width) * 0.5f;

	// draw background image if available
	if (ui().options().use_background_image() && bgrnd_bitmap->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white, bgrnd_texture, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));

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

	ui().draw_outlined_box(container, x1, y1, x2, y2, UI_BACKGROUND_COLOR);

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
		const menu_item &pitem = item[itemnum];
		const char *itemtext = pitem.text.c_str();
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
		else if (pitem.subtext.empty())
		{
			ui().draw_text_full(container, itemtext, effective_left, line_y, effective_width, ui::text_layout::LEFT, ui::text_layout::NEVER,
				mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		}
	}

	for (size_t count = visible_items; count < item.size(); count++)
	{
		const menu_item &pitem = item[count];
		const char *itemtext = pitem.text.c_str();
		float line_x0 = x1 + 0.5f * UI_LINE_WIDTH;
		float line_y0 = line;
		float line_x1 = x2 - 0.5f * UI_LINE_WIDTH;
		float line_y1 = line + line_height;
		rgb_t fgcolor = UI_SELECTED_COLOR;
		rgb_t bgcolor = UI_SELECTED_BG_COLOR;

		if (mouse_hit && line_x0 <= mouse_x && line_x1 > mouse_x && line_y0 <= mouse_y && line_y1 > mouse_y && is_selectable(pitem))
			hover = count;

		if (pitem.type == menu_item_type::SEPARATOR)
			container->add_line(visible_left, line + 0.5f * line_height, visible_left + visible_width, line + 0.5f * line_height,
				UI_LINE_WIDTH, UI_TEXT_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
		else
		{
			highlight(container, line_x0, line_y0, line_x1, line_y1, bgcolor);
			ui().draw_text_full(container, itemtext, effective_left, line, effective_width, ui::text_layout::CENTER, ui::text_layout::TRUNCATE,
				mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
		}
		line += line_height;
	}

	// if there is something special to add, do it by calling the virtual method
	custom_render(get_selection_ref(), customtop, custombottom, x1, y1, x2, y2);

	// return the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	visitems = visible_lines - (top_line != 0) - (top_line + visible_lines != visible_items);
}

void menu::set_pressed()
{
	(m_repeat == 0) ? m_repeat = osd_ticks() + osd_ticks_per_second() / 2 : m_repeat = osd_ticks() + osd_ticks_per_second() / 4;
	m_pressed = true;
}


//-------------------------------------------------
//  extra_text_draw_box - generically adds header
//  or footer text
//-------------------------------------------------

void menu::extra_text_draw_box(float origx1, float origx2, float origy, float yspan, const char *text, int direction)
{
	// get the size of the text
	auto layout = ui().create_layout(container);
	layout.add_text(text);

	// position this extra text
	float x1, y1, x2, y2;
	extra_text_position(origx1, origx2, origy, yspan, layout, direction, x1, y1, x2, y2);

	// draw a box
	ui().draw_outlined_box(container,x1, y1, x2, y2, UI_BACKGROUND_COLOR);

	// take off the borders
	x1 += UI_BOX_LR_BORDER;
	y1 += UI_BOX_TB_BORDER;

	// draw the text within it
	layout.emit(container, x1, y1);
}


//-------------------------------------------------
//  extra_text_position - given extra text that has
//  been put into a layout, position it
//-------------------------------------------------

void menu::extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
	int direction, float &x1, float &y1, float &x2, float &y2)
{
	float width = layout.actual_width() + (2 * UI_BOX_LR_BORDER);
	float maxwidth = MAX(width, origx2 - origx1);

	// compute our bounds
	x1 = 0.5f - 0.5f * maxwidth;
	x2 = x1 + maxwidth;
	y1 = origy + (yspan * direction);
	y2 = origy + (UI_BOX_TB_BORDER * direction);

	if (y1 > y2)
		std::swap(y1, y2);
}


//-------------------------------------------------
//  extra_text_render - generically adds header
//  and footer text
//-------------------------------------------------

void menu::extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, const char *header, const char *footer)
{
	header = ((header != nullptr) && (header[0] != '\0')) ? header : nullptr;
	footer = ((footer != nullptr) && (footer[0] != '\0')) ? footer : nullptr;

	if (header != nullptr)
		extra_text_draw_box(origx1, origx2, origy1, top, header, -1);
	if (footer != nullptr)
		extra_text_draw_box(origx1, origx2, origy2, bottom, footer, +1);
}

} // namespace ui
