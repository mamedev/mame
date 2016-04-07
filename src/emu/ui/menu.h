// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/menu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#pragma once

#ifndef __UI_MENU_H__
#define __UI_MENU_H__

#include "render.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

// flags for menu items
#define MENU_FLAG_LEFT_ARROW        (1 << 0)
#define MENU_FLAG_RIGHT_ARROW       (1 << 1)
#define MENU_FLAG_INVERT            (1 << 2)
#define MENU_FLAG_MULTILINE         (1 << 3)
#define MENU_FLAG_REDTEXT           (1 << 4)
#define MENU_FLAG_DISABLE           (1 << 5)
#define MENU_FLAG_UI                (1 << 6)
#define MENU_FLAG_UI_DATS           (1 << 7)
#define MENU_FLAG_UI_SWLIST         (1 << 8)
#define MENU_FLAG_UI_FAVORITE       (1 << 9)
#define MENU_FLAG_UI_PALETTE        (1 << 10)
#define MENU_FLAG_UI_HEADING        (1 << 11)

// special menu item for separators
#define MENU_SEPARATOR_ITEM         "---"

// flags to pass to ui_menu_process
#define UI_MENU_PROCESS_NOKEYS      1
#define UI_MENU_PROCESS_LR_REPEAT   2
#define UI_MENU_PROCESS_CUSTOM_ONLY 4
#define UI_MENU_PROCESS_ONLYCHAR    8
#define UI_MENU_PROCESS_NOINPUT     16
#define UI_MENU_PROCESS_NOIMAGE     32

// options for ui_menu_reset
enum ui_menu_reset_options
{
	UI_MENU_RESET_SELECT_FIRST,
	UI_MENU_RESET_REMEMBER_POSITION,
	UI_MENU_RESET_REMEMBER_REF
};



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// menu-related events
struct ui_menu_event
{
	void         *itemref;   // reference for the selected item
	int          iptkey;     // one of the IPT_* values from inptport.h
	unicode_char unichar;    // unicode character if iptkey == IPT_SPECIAL
};

struct ui_menu_pool
{
	ui_menu_pool   *next;    // chain to next one
	UINT8          *top;     // top of the pool
	UINT8          *end;     // end of the pool
};


class ui_menu_item
{
public:
	const char  *text;
	const char  *subtext;
	UINT32      flags;
	void        *ref;

	inline bool is_selectable() const;
};

class ui_menu
{
public:
	ui_menu(running_machine &machine, render_container *container);
	virtual ~ui_menu();

	running_machine &machine() const { return m_machine; }

	render_container            *container;   // render_container we render to
	ui_menu_event               menu_event;   // the UI menu_event that occurred
	ui_menu                     *parent;      // pointer to parent menu
	int                         resetpos;     // reset position
	void                        *resetref;    // reset reference
	int                         selected;     // which item is selected
	int                         hover;        // which item is being hovered over
	int                         visitems;     // number of visible items
	float                       customtop;    // amount of extra height to add at the top
	float                       custombottom; // amount of extra height to add at the bottom
	ui_menu_pool                *pool;        // list of memory pools
	std::vector<ui_menu_item>   item;         // array of items

	// free all items in the menu, and all memory allocated from the memory pool
	void reset(ui_menu_reset_options options);

	// append a new item to the end of the menu
	void item_append(const char *text, const char *subtext, UINT32 flags, void *ref);

	// process a menu, drawing it and returning any interesting events
	const ui_menu_event *process(UINT32 flags);

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
	void set_special_main_menu(bool disable);

	// Global initialization
	static void init(running_machine &machine);
	static void exit(running_machine &machine);

	// reset the menus, clearing everything
	static void stack_reset(running_machine &machine);

	// push a new menu onto the stack
	static void stack_push(ui_menu *menu);

	// pop a menu from the stack
	static void stack_pop(running_machine &machine);

	// test if one of the menus in the stack requires hide disable
	static bool stack_has_special_main_menu();

	// highlight
	static void highlight(render_container *container, float x0, float y0, float x1, float y1, rgb_t bgcolor);

	// draw arrow
	static void draw_arrow(render_container *container, float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation);

	// master handler
	static UINT32 ui_handler(running_machine &machine, render_container *container, UINT32 state);

	// Used by sliders
	void validate_selection(int scandir);
	static ui_menu *menu_stack;

	void do_handle();

	// To be reimplemented in the menu subclass
	virtual void populate() = 0;

	// To be reimplemented in the menu subclass
	virtual void handle() = 0;

	// test if search is active
	virtual bool menu_has_search_active() { return false; }

private:
	static ui_menu *menu_free;
	static std::unique_ptr<bitmap_rgb32> hilight_bitmap;
	static render_texture *hilight_texture, *arrow_texture;

	bool m_special_main_menu;
	running_machine &m_machine;  // machine we are attached to

	void draw(bool customonly, bool noimage, bool noinput);
	void draw_text_box();
	void handle_events(UINT32 flags);
	void handle_keys(UINT32 flags);

	inline bool exclusive_input_pressed(int key, int repeat);
	static void clear_free_list(running_machine &machine);
	static void render_triangle(bitmap_argb32 &dest, bitmap_argb32 &source, const rectangle &sbounds, void *param);

public:
	// tab navigation
	enum class focused_menu
	{
		main,
		left,
		righttop,
		rightbottom
	};

	focused_menu m_focus;
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
	static void init_ui(running_machine &machine);

	// get arrows status
	template <typename _T1, typename _T2, typename _T3>
	UINT32 get_arrow_flags(_T1 min, _T2 max, _T3 actual)
	{
		if (max == 0)
			return 0;
		else
			return ((actual <= min) ? MENU_FLAG_RIGHT_ARROW : (actual >= max ? MENU_FLAG_LEFT_ARROW : (MENU_FLAG_LEFT_ARROW | MENU_FLAG_RIGHT_ARROW)));
	}

protected:
	int topline_datsview;      // right box top line
	int top_line;              // main box top line
	int l_sw_hover;
	int l_hover;
	int totallines;
	int skip_main_items;

	// draw right box
	float draw_right_box_title(float x1, float y1, float x2, float y2);

	// draw arrow
	void draw_common_arrow(float origx1, float origy1, float origx2, float origy2, int current, int dmin, int dmax, float title);
	void info_arrow(int ub, float origx1, float origx2, float oy1, float line_height, float text_size, float ud_arrow_width);

	// images render
	std::string arts_render_common(float origx1, float origy1, float origx2, float origy2);
	void arts_render_images(bitmap_argb32 *bitmap, float origx1, float origy1, float origx2, float origy2, bool software);

	int visible_lines;        // main box visible lines
	int right_visible_lines;  // right box lines

	static std::unique_ptr<bitmap_argb32> snapx_bitmap;
	static render_texture *snapx_texture;

	static std::unique_ptr<bitmap_rgb32> hilight_main_bitmap;
	static render_texture *hilight_main_texture;
private:

	// mouse button held down
	bool m_pressed = false;
	osd_ticks_t m_repeat = 0;
	void reset_pressed() { m_pressed = false; m_repeat = 0; }
	bool mouse_pressed() { return (osd_ticks() >= m_repeat); }
	void set_pressed();

	static std::unique_ptr<bitmap_argb32> no_avail_bitmap, bgrnd_bitmap, star_bitmap;
	static render_texture *bgrnd_texture, *star_texture;
	static bitmap_argb32 *icons_bitmap[];
	static render_texture *icons_texture[];

	// toolbar
	static bitmap_argb32 *toolbar_bitmap[], *sw_toolbar_bitmap[];
	static render_texture *toolbar_texture[], *sw_toolbar_texture[];

	// draw game list
	void draw_select_game(bool noinput);

	// draw palette menu
	void draw_palette_menu();

	// draw dats menu
	void draw_dats_menu();

	void get_title_search(std::string &title, std::string &search);

	// handle keys
	void handle_main_keys(UINT32 flags);

	// handle mouse
	void handle_main_events(UINT32 flags);

	void draw_icon(int linenum, void *selectedref, float x1, float y1);
};

#endif  // __UI_MENU_H__
