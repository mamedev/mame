// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/menu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_MENU_H
#define MAME_FRONTEND_UI_MENU_H

#include "ui/ui.h"
#include "ui/menuitem.h"

#include "language.h"
#include "render.h"

#include <memory>


namespace ui {
/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class menu
{
public:
	// flags for menu items
	enum
	{
		FLAG_LEFT_ARROW     = (1 << 0),
		FLAG_RIGHT_ARROW    = (1 << 1),
		FLAG_INVERT         = (1 << 2),
		FLAG_MULTILINE      = (1 << 3),
		FLAG_REDTEXT        = (1 << 4),
		FLAG_DISABLE        = (1 << 5),
		FLAG_UI             = (1 << 6),
		FLAG_UI_DATS        = (1 << 7),
		FLAG_UI_SWLIST      = (1 << 8),
		FLAG_UI_FAVORITE    = (1 << 9),
		FLAG_UI_PALETTE     = (1 << 10),
		FLAG_UI_HEADING     = (1 << 11)
	};

	virtual ~menu();

	mame_ui_manager &ui() const { return m_ui; }
	running_machine &machine() const { return m_ui.machine(); }

	render_container        *container;   // render_container we render to
	int                     resetpos;     // reset position
	void                    *resetref;    // reset reference
	int                     selected;     // which item is selected
	int                     hover;        // which item is being hovered over
	int                     visitems;     // number of visible items
	float                   customtop;    // amount of extra height to add at the top
	float                   custombottom; // amount of extra height to add at the bottom
	std::vector<menu_item>  item;         // array of items

	// append a new item to the end of the menu
	void item_append(const char *text, const char *subtext, UINT32 flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	void item_append(menu_item item);
	void item_append(menu_item_type type);

	// configure the menu for custom rendering
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// allocate temporary memory from the menu's memory pool
	void *m_pool_alloc(size_t size);

	// make a temporary string copy in the menu's memory pool
	const char *pool_strdup(const char *string);

	// retrieves the index of the currently selected menu item
	void *get_selection();

	// changes the index of the currently selected menu item
	void set_selection(void *selected_itemref);

	// request the specific handling of the game selection main menu
	bool is_special_main_menu() const;

	// Global initialization
	static void init(running_machine &machine, ui_options &mopt);
	static void exit(running_machine &machine);

	// reset the menus, clearing everything
	static void stack_reset(running_machine &machine);

	// push a new menu onto the stack
	template <typename T, typename... Params>
	static void stack_push(Params &&... args)
	{
		stack_push(std::unique_ptr<menu>(global_alloc_clear<T>(std::forward<Params>(args)...)));
	}
	template <typename T, typename... Params>
	static void stack_push_special_main(Params &&... args)
	{
		std::unique_ptr<menu> ptr(global_alloc_clear<T>(std::forward<Params>(args)...));
		ptr->set_special_main_menu(true);
		stack_push(std::move(ptr));
	}

	// pop a menu from the stack
	static void stack_pop(running_machine &machine);

	// test if one of the menus in the stack requires hide disable
	static bool stack_has_special_main_menu();

	// highlight
	static void highlight(render_container *container, float x0, float y0, float x1, float y1, rgb_t bgcolor);

	// draw arrow
	static void draw_arrow(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation);

	// master handler
	static UINT32 ui_handler(render_container *container, mame_ui_manager &mui);

	// Used by sliders
	void validate_selection(int scandir);

	void do_handle();

	// To be reimplemented in the menu subclass
	virtual void populate() = 0;

	// To be reimplemented in the menu subclass
	virtual void handle() = 0;

	// test if search is active
	virtual bool menu_has_search_active() { return false; }

private:
	static std::unique_ptr<bitmap_rgb32> hilight_bitmap;
	static render_texture *hilight_texture, *arrow_texture;

	void draw(UINT32 flags, float x0 = 0.0f, float y0 = 0.0f);
	void draw_text_box();
	void handle_events(UINT32 flags);
	void handle_keys(UINT32 flags);

	inline bool exclusive_input_pressed(int key, int repeat);
	static void clear_free_list(running_machine &machine);
	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

public:
	void *m_prev_selected;

	int  visible_items;
	bool ui_error;

	// mouse handling
	bool mouse_hit, mouse_button;
	render_target *mouse_target;
	INT32 mouse_target_x, mouse_target_y;
	float mouse_x, mouse_y;

	// draw toolbar
	void draw_toolbar(float x1, float y1, float x2, float y2, bool software = false);

	// draw left panel
	virtual float draw_left_panel(float x1, float y1, float x2, float y2) { return 0; }

	// draw right panel
	virtual void draw_right_panel(void *selectedref, float origx1, float origy1, float origx2, float origy2) { };

	// draw star
	void draw_star(float x0, float y0);

	// Global initialization
	static void init_ui(running_machine &machine, ui_options &mopt);

	// get arrows status
	template <typename _T1, typename _T2, typename _T3>
	UINT32 get_arrow_flags(_T1 min, _T2 max, _T3 actual)
	{
		if (max == 0)
			return 0;
		else
			return ((actual <= min) ? FLAG_RIGHT_ARROW : (actual >= max ? FLAG_LEFT_ARROW : (FLAG_LEFT_ARROW | FLAG_RIGHT_ARROW)));
	}

protected:
	// flags to pass to process
	enum
	{
		PROCESS_NOKEYS      = 1,
		PROCESS_LR_REPEAT   = 2,
		PROCESS_CUSTOM_ONLY = 4,
		PROCESS_ONLYCHAR    = 8,
		PROCESS_NOINPUT     = 16,
		PROCESS_NOIMAGE     = 32
	};

	// options for reset
	enum class reset_options
	{
		SELECT_FIRST,
		REMEMBER_POSITION,
		REMEMBER_REF
	};

	// tab navigation
	enum class focused_menu
	{
		main,
		left,
		righttop,
		rightbottom
	};

	// menu-related events
	struct event
	{
		void                *itemref;   // reference for the selected item
		menu_item_type      type;       // item type (eventually will go away when itemref is proper ui_menu_item class rather than void*)
		int                 iptkey;     // one of the IPT_* values from inptport.h
		unicode_char        unichar;    // unicode character if iptkey == IPT_SPECIAL
		render_bounds       mouse;      // mouse position if iptkey == IPT_CUSTOM

		bool is_char_printable() const
		{
			return
				!(0x0001f >= unichar) &&                            // C0 control
				!((0x0007f <= unichar) && (0x0009f >= unichar)) &&  // DEL and C1 control
				!((0x0fdd0 <= unichar) && (0x0fddf >= unichar)) &&  // noncharacters
				!(0x0fffe == (unichar & 0x0ffff)) &&                // byte-order detection noncharacter
				!(0x0ffff == (unichar & 0x0ffff));                  // the other noncharacter
		}

		template <std::size_t N>
		bool append_char(char (&buffer)[N], std::size_t offset) const
		{
			auto const chlen = utf8_from_uchar(&buffer[offset], N - offset - 1, unichar);
			if (0 < chlen)
			{
				buffer[offset + chlen] = '\0';
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	int topline_datsview;      // right box top line
	int top_line;              // main box top line
	int l_sw_hover;
	int l_hover;
	int totallines;
	int skip_main_items;

	menu(mame_ui_manager &mui, render_container *container);

	// free all items in the menu, and all memory allocated from the memory pool
	void reset(reset_options options);
	void reset_parent(reset_options options) { m_parent->reset(options); }
	static void reset_topmost(reset_options options) { if (menu_stack) menu_stack->reset(options); }

	// process a menu, drawing it and returning any interesting events
	const event *process(UINT32 flags, float x0 = 0.0f, float y0 = 0.0f);
	void process_parent() { m_parent->process(PROCESS_NOINPUT); }

	focused_menu get_focus() const { return m_focus; }
	void set_focus(focused_menu focus) { m_focus = focus; }

	// draw right box
	float draw_right_box_title(float x1, float y1, float x2, float y2);

	// draw arrow
	void draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title);
	void info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width);

	// images render
	std::string arts_render_common(float origx1, float origy1, float origx2, float origy2);
	void arts_render_images(bitmap_argb32 *bitmap, float origx1, float origy1, float origx2, float origy2);

	// draw header and footer text
	void extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, const char *header, const char *footer);
	void extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
		int direction, float &x1, float &y1, float &x2, float &y2);

	// custom events
	virtual bool custom_mouse_down() { return false; }

	template <typename T>
	static T *topmost_menu() { return dynamic_cast<T *>(menu_stack.get()); }

	int visible_lines;        // main box visible lines
	int right_visible_lines;  // right box lines

	static std::unique_ptr<bitmap_argb32> snapx_bitmap;
	static render_texture *snapx_texture;

	static std::unique_ptr<bitmap_rgb32> hilight_main_bitmap;
	static render_texture *hilight_main_texture;

private:
	struct pool
	{
		pool   *next;    // chain to next one
		UINT8  *top;     // top of the pool
		UINT8  *end;     // end of the pool
	};


	void reset_pressed() { m_pressed = false; m_repeat = 0; }
	bool mouse_pressed() { return (osd_ticks() >= m_repeat); }
	void set_pressed();

	static std::unique_ptr<bitmap_argb32> no_avail_bitmap, bgrnd_bitmap, star_bitmap;
	static render_texture *bgrnd_texture, *star_texture;
	static std::vector<std::unique_ptr<bitmap_argb32>> icons_bitmap;
	static render_texture *icons_texture[];

	// request the specific handling of the game selection main menu
	void set_special_main_menu(bool disable);

	// push a new menu onto the stack
	static void stack_push(std::unique_ptr<menu> &&menu);

	// toolbar
	static std::vector<std::shared_ptr<bitmap_argb32>> toolbar_bitmap, sw_toolbar_bitmap;
	static render_texture *toolbar_texture[], *sw_toolbar_texture[];

	// draw game list
	void draw_select_game(UINT32 flags);

	// draw palette menu
	void draw_palette_menu();

	// draw dats menu
	void draw_dats_menu();

	void get_title_search(std::string &title, std::string &search);

	// handle keys
	void handle_main_keys(UINT32 flags);

	// handle mouse
	void handle_main_events();

	float draw_icon(int linenum, void *selectedref, float x1, float y1);
	void extra_text_draw_box(float origx1, float origx2, float origy, float yspan, const char *text, int direction);

	bool                    m_special_main_menu;
	mame_ui_manager         &m_ui;     // UI we are attached to
	std::unique_ptr<menu>   m_parent;  // pointer to parent menu
	bool                    m_pressed; // mouse button held down
	osd_ticks_t             m_repeat;
	event                   m_event;   // the UI event that occurred
	pool                    *m_pool;   // list of memory pools
	focused_menu            m_focus;
	static std::vector<const game_driver *> m_old_icons;

	static std::unique_ptr<menu> menu_stack;
	static std::unique_ptr<menu> menu_free;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_MENU_H
