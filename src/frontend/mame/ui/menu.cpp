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
#include "ui/miscmenu.h"

#include "cheat.h"
#include "mame.h"

#include "drivenum.h"
#include "rendutil.h"
#include "uiinput.h"

#include <algorithm>
#include <cmath>
#include <utility>


namespace ui {
/***************************************************************************
    CONSTANTS
***************************************************************************/

#define UI_MENU_POOL_SIZE  65536

/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

std::mutex menu::s_global_state_guard;
menu::global_state_map menu::s_global_states;
std::unique_ptr<bitmap_rgb32> menu::hilight_bitmap;
render_texture *menu::hilight_texture;
render_texture *menu::snapx_texture;
render_texture *menu::hilight_main_texture;
std::unique_ptr<bitmap_argb32> menu::snapx_bitmap;
std::unique_ptr<bitmap_rgb32> menu::hilight_main_bitmap;

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline menu::global_state_ptr menu::get_global_state(running_machine &machine)
{
	std::lock_guard<std::mutex> guard(s_global_state_guard);
	auto const it(s_global_states.find(&machine));
	return (it != s_global_states.end()) ? it->second : global_state_ptr();

}

//-------------------------------------------------
//  exclusive_input_pressed - return TRUE if the
//  given key is pressed and we haven't already
//  reported a key
//-------------------------------------------------

bool menu::exclusive_input_pressed(int &iptkey, int key, int repeat)
{
	if ((iptkey == IPT_INVALID) && machine().ui_input().pressed_repeat(key, repeat))
	{
		iptkey = key;
		return true;
	}
	else
	{
		return false;
	}
}



/***************************************************************************
    CORE SYSTEM MANAGEMENT
***************************************************************************/

menu::global_state::global_state(running_machine &machine, ui_options const &options)
	: m_machine(machine)
	, m_cleanup_callbacks()
	, m_bgrnd_bitmap()
	, m_bgrnd_texture()
	, m_stack()
	, m_free()
{
	render_manager &render(machine.render());
	auto const texture_free([&render](render_texture *texture) { render.texture_free(texture); });

	// create a texture for arrow icons
	m_arrow_texture = texture_ptr(render.texture_alloc(render_triangle), texture_free);

	// create a texture for main menu background
	m_bgrnd_texture = texture_ptr(render.texture_alloc(render_texture::hq_scale), texture_free);
	if (options.use_background_image() && (&machine.system() == &GAME_NAME(___empty)))
	{
		m_bgrnd_bitmap = std::make_unique<bitmap_argb32>(0, 0);
		emu_file backgroundfile(".", OPEN_FLAG_READ);
		render_load_jpeg(*m_bgrnd_bitmap, backgroundfile, nullptr, "background.jpg");

		if (!m_bgrnd_bitmap->valid())
			render_load_png(*m_bgrnd_bitmap, backgroundfile, nullptr, "background.png");

		if (m_bgrnd_bitmap->valid())
			m_bgrnd_texture->set_bitmap(*m_bgrnd_bitmap, m_bgrnd_bitmap->cliprect(), TEXFORMAT_ARGB32);
		else
			m_bgrnd_bitmap->reset();
	}
}


menu::global_state::~global_state()
{
	stack_reset();
	clear_free_list();

	for (auto const &callback : m_cleanup_callbacks)
		callback(m_machine);
}


void menu::global_state::add_cleanup_callback(cleanup_callback &&callback)
{
	m_cleanup_callbacks.emplace_back(std::move(callback));
}


void menu::global_state::stack_push(std::unique_ptr<menu> &&menu)
{
	menu->m_parent = std::move(m_stack);
	m_stack = std::move(menu);
	m_stack->reset(reset_options::SELECT_FIRST);
	m_stack->machine().ui_input().reset();
}


void menu::global_state::stack_pop()
{
	if (m_stack)
	{
		std::unique_ptr<menu> menu(std::move(m_stack));
		m_stack = std::move(menu->m_parent);
		menu->m_parent = std::move(m_free);
		m_free = std::move(menu);
		m_machine.ui_input().reset();
	}
}


void menu::global_state::stack_reset()
{
	while (m_stack)
		stack_pop();
}


void menu::global_state::clear_free_list()
{
	while (m_free)
		m_free = std::move(m_free->m_parent);
}


bool menu::global_state::stack_has_special_main_menu() const
{
	for (auto menu = m_stack.get(); menu != nullptr; menu = menu->m_parent.get())
	{
		if (menu->is_special_main_menu())
			return true;
	}
	return false;
}




//-------------------------------------------------
//  init - initialize the menu system
//-------------------------------------------------

void menu::init(running_machine &machine, ui_options &mopt)
{
	// initialize the menu stack
	{
		std::lock_guard<std::mutex> guard(s_global_state_guard);
		auto const ins(s_global_states.emplace(&machine, std::make_shared<global_state>(machine, mopt)));
		if (ins.second)
			machine.add_notifier(MACHINE_NOTIFY_EXIT, machine_notify_delegate(FUNC(menu::exit), &machine)); // add an exit callback to free memory
		else
			ins.first->second->stack_reset();
	}

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

	// initialize ui
	init_ui(machine, mopt);
}


//-------------------------------------------------
//  exit - clean up after ourselves
//-------------------------------------------------

void menu::exit(running_machine &machine)
{
	// free menus
	{
		std::lock_guard<std::mutex> guard(s_global_state_guard);
		s_global_states.erase(&machine);
	}

	// free textures
	render_manager &mre = machine.render();
	mre.texture_free(hilight_texture);
	mre.texture_free(snapx_texture);
	mre.texture_free(hilight_main_texture);
}



/***************************************************************************
    CORE MENU MANAGEMENT
***************************************************************************/

//-------------------------------------------------
//  menu - menu constructor
//-------------------------------------------------

menu::menu(mame_ui_manager &mui, render_container *_container)
	: m_visible_lines(0)
	, m_visible_items(0)
	, m_global_state(get_global_state(mui.machine()))
	, m_special_main_menu(false)
	, m_ui(mui)
	, m_parent()
	, m_event()
	, m_pool(nullptr)
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
	m_visible_items = 0;
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
			item_append(_("Exit"), "", FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, nullptr);
	}
	else
	{
		if (machine().options().ui() != emu_options::UI_SIMPLE && stack_has_special_main_menu())
			item_append(_("Return to Previous Menu"), "", FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW, nullptr);
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
	else if ((item[0].flags & FLAG_UI_PALETTE) != 0)
		draw_palette_menu();
	else
		draw(flags);

	// process input
	if (!(flags & PROCESS_NOKEYS) && !(flags & PROCESS_NOINPUT))
	{
		// read events
		handle_events(flags, m_event);

		// handle the keys if we don't already have an event
		if (m_event.iptkey == IPT_INVALID)
			handle_keys(flags, m_event.iptkey);
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

void menu::draw(UINT32 flags)
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

	if (&machine().system() == &GAME_NAME(___empty) && !noimage)
		draw_background();

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

	m_visible_lines = std::min(int(std::floor(visible_main_menu_height / line_height)), int(unsigned(item.size())));
	visible_main_menu_height = float(m_visible_lines) * line_height;

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
	if (selected >= (top_line + m_visible_lines))
		top_line = selected - (m_visible_lines / 2);
	if ((top_line > (item.size() - m_visible_lines)) || (selected == (item.size() - 1)))
		top_line = item.size() - m_visible_lines;

	// if scrolling, show arrows
	bool const show_top_arrow((item.size() > m_visible_lines) && !first_item_visible());
	bool const show_bottom_arrow((item.size() > m_visible_lines) && !last_item_visible());

	// set the number of visible lines, minus 1 for top arrow and 1 for bottom arrow
	m_visible_items = m_visible_lines - (show_top_arrow ? 1 : 0) - (show_bottom_arrow ? 1 : 0);

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
		for (int linenum = 0; linenum < m_visible_lines; linenum++)
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

			if (linenum == 0 && show_top_arrow)
			{
				// if we're on the top line, display the up arrow
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
			else if (linenum == m_visible_lines - 1 && show_bottom_arrow)
			{
				// if we're on the bottom line, display the down arrow
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
			else if (pitem.type == menu_item_type::SEPARATOR)
			{
				// if we're just a divider, draw a line
				container->add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
			}
			else if (pitem.subtext.empty())
			{
				// if we don't have a subitem, just draw the string centered
				if (pitem.flags & FLAG_UI_HEADING)
				{
					float heading_width = ui().get_string_width(itemtext);
					container->add_line(visible_left, line_y0 + 0.5f * line_height, visible_left + ((visible_width - heading_width) / 2) - UI_BOX_LR_BORDER, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
					container->add_line(visible_left + visible_width - ((visible_width - heading_width) / 2) + UI_BOX_LR_BORDER, line_y0 + 0.5f * line_height, visible_left + visible_width, line_y0 + 0.5f * line_height, UI_LINE_WIDTH, UI_BORDER_COLOR, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
				}
				ui().draw_text_full(container, itemtext, effective_left, line_y0, effective_width,
					ui::text_layout::CENTER, ui::text_layout::TRUNCATE, mame_ui_manager::NORMAL, fgcolor, bgcolor, nullptr, nullptr);
			}
			else
			{
				// otherwise, draw the item on the left and the subitem text on the right
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

void menu::handle_events(UINT32 flags, event &ev)
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
							top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
							return;
						}
						selected -= m_visible_items;
						if (selected < 0)
							selected = 0;
						top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
					}
					else if (hover == HOVER_ARROW_DOWN)
					{
						if ((flags & FLAG_UI_DATS) != 0)
						{
							top_line += m_visible_lines - 2;
							return;
						}
						selected += m_visible_lines - 2 + (selected == 0);
						if (selected > item.size() - 1)
							selected = item.size() - 1;
						top_line += m_visible_lines - 2;
					}
				}
				break;

			// if we are hovering over a valid item, fake a UI_SELECT with a double-click
			case UI_EVENT_MOUSE_DOUBLE_CLICK:
				if (!(flags & PROCESS_ONLYCHAR) && hover >= 0 && hover < item.size())
				{
					selected = hover;
					ev.iptkey = IPT_UI_SELECT;
					if (is_last_selected())
					{
						ev.iptkey = IPT_UI_CANCEL;
						stack_pop();
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
						if (selected <= top_line && m_visible_items != m_visible_lines)
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
						top_line += (selected >= top_line + m_visible_items + (top_line != 0));
						if (selected >= (top_line + m_visible_items + (top_line != 0)))
							top_line += local_menu_event.num_lines;
					}
				}
				break;

			// translate CHAR events into specials
			case UI_EVENT_CHAR:
				ev.iptkey = IPT_SPECIAL;
				ev.unichar = local_menu_event.ch;
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

void menu::handle_keys(UINT32 flags, int &iptkey)
{
	bool ignorepause = stack_has_special_main_menu();
	int code;

	// bail if no items
	if (item.empty())
		return;

	// if we hit select, return TRUE or pop the stack, depending on the item
	if (exclusive_input_pressed(iptkey, IPT_UI_SELECT, 0))
	{
		if (is_last_selected())
		{
			iptkey = IPT_UI_CANCEL;
			stack_pop();
		}
		return;
	}

	// bail out
	if ((flags & PROCESS_ONLYCHAR))
		return;

	// hitting cancel also pops the stack
	if (exclusive_input_pressed(iptkey, IPT_UI_CANCEL, 0))
	{
		if (!menu_has_search_active())
			stack_pop();
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
	if (!ignoreleft && exclusive_input_pressed(iptkey, IPT_UI_LEFT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;
	if (!ignoreright && exclusive_input_pressed(iptkey, IPT_UI_RIGHT, (flags & PROCESS_LR_REPEAT) ? 6 : 0))
		return;

	// up backs up by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_UP, 6))
	{
		if ((item[0].flags & FLAG_UI_DATS))
		{
			top_line--;
			return;
		}
		is_first_selected() ? selected = top_line = item.size() - 1 : --selected;
		validate_selection(-1);
		top_line -= (selected <= top_line && top_line != 0);
		if (selected <= top_line && m_visible_items != m_visible_lines)
			top_line--;
	}

	// down advances by one item
	if (exclusive_input_pressed(iptkey, IPT_UI_DOWN, 6))
	{
		if ((item[0].flags & FLAG_UI_DATS))
		{
			top_line++;
			return;
		}
		is_last_selected() ? selected = top_line = 0 : ++selected;
		validate_selection(1);
		top_line += (selected >= top_line + m_visible_items + (top_line != 0));
		if (selected >= (top_line + m_visible_items + (top_line != 0)))
			top_line++;
	}

	// page up backs up by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_UP, 6))
	{
		selected -= m_visible_items;
		top_line -= m_visible_items - (last_item_visible() ? 1 : 0);
		if (selected < 0)
			selected = 0;
		validate_selection(1);
	}

	// page down advances by m_visible_items
	if (exclusive_input_pressed(iptkey, IPT_UI_PAGE_DOWN, 6))
	{
		selected += m_visible_lines - 2 + (selected == 0);
		top_line += m_visible_lines - 2;

		if (selected > item.size() - 1)
			selected = item.size() - 1;
		validate_selection(-1);
	}

	// home goes to the start
	if (exclusive_input_pressed(iptkey, IPT_UI_HOME, 0))
	{
		selected = top_line = 0;
		validate_selection(1);
	}

	// end goes to the last
	if (exclusive_input_pressed(iptkey, IPT_UI_END, 0))
	{
		selected = top_line = item.size() - 1;
		validate_selection(-1);
	}

	// pause enables/disables pause
	if (!ignorepause && exclusive_input_pressed(iptkey, IPT_UI_PAUSE, 0))
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
	if (iptkey == IPT_INVALID)
	{
		for (code = IPT_UI_FIRST + 1; code < IPT_UI_LAST; code++)
		{
			if (code == IPT_UI_CONFIGURE || (code == IPT_UI_LEFT && ignoreleft) || (code == IPT_UI_RIGHT && ignoreright) || (code == IPT_UI_PAUSE && ignorepause))
				continue;
			if (exclusive_input_pressed(iptkey, code, 0))
				break;
		}
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



/***************************************************************************
    MENU STACK MANAGEMENT
***************************************************************************/

void menu::do_handle()
{
	if (item.size() < 2)
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
	global_state_ptr const state(get_global_state(mui.machine()));

	// if we have no menus stacked up, start with the main menu
	if (!state->topmost_menu<menu>())
		state->stack_push(std::unique_ptr<menu>(global_alloc_clear<menu_main>(mui, container)));

	// update the menu state
	if (state->topmost_menu<menu>())
		state->topmost_menu<menu>()->do_handle();

	// clear up anything pending to be released
	state->clear_free_list();

	// if the menus are to be hidden, return a cancel here
	if (mui.is_menu_active() && ((mui.machine().ui_input().pressed(IPT_UI_CONFIGURE) && !state->stack_has_special_main_menu()) || !state->topmost_menu<menu>()))
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
	container->add_quad(x0, y0, x1, y1, fgcolor, m_global_state->arrow_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_TEXORIENT(orientation) | PRIMFLAG_PACKABLE);
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

	if (&machine().system() == &GAME_NAME(___empty))
		draw_background();

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
	m_visible_items = visible_lines - (top_line != 0) - (top_line + visible_lines != item.size());
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


void menu::draw_background()
{
	// draw background image if available
	if (ui().options().use_background_image() && m_global_state->bgrnd_bitmap() && m_global_state->bgrnd_bitmap()->valid())
		container->add_quad(0.0f, 0.0f, 1.0f, 1.0f, rgb_t::white, m_global_state->bgrnd_texture(), PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA));
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
