// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/menu.h

    Internal MAME menus for the user interface.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_MENU_H
#define MAME_FRONTEND_UI_MENU_H

#pragma once

#include "ui/ui.h"
#include "ui/menuitem.h"
#include "ui/widgets.h"

#include "language.h"
#include "render.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>


namespace ui {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class menu
{
public:
	// flags for menu items
	enum : unsigned
	{
		FLAG_LEFT_ARROW     = 1U << 0,
		FLAG_RIGHT_ARROW    = 1U << 1,
		FLAG_INVERT         = 1U << 2,
		FLAG_MULTILINE      = 1U << 3,
		FLAG_REDTEXT        = 1U << 4,
		FLAG_DISABLE        = 1U << 5,
		FLAG_UI_DATS        = 1U << 6,
		FLAG_UI_HEADING     = 1U << 7,
		FLAG_COLOR_BOX      = 1U << 8
	};

	virtual ~menu();

	// append a new item to the end of the menu
	void item_append(const std::string &text, const std::string &subtext, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	void item_append(std::string &&text, std::string &&subtext, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	void item_append(menu_item item);
	void item_append(menu_item_type type, uint32_t flags = 0);
	void item_append_on_off(const std::string &text, bool state, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);

	// Global initialization
	static void init(running_machine &machine, ui_options &mopt);

	// reset the menus, clearing everything
	static void stack_reset(running_machine &machine) { get_global_state(machine)->stack_reset(); }

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
	static void stack_pop(running_machine &machine) { get_global_state(machine)->stack_pop(); }

	// test if one of the menus in the stack requires hide disable
	static bool stack_has_special_main_menu(running_machine &machine) { return get_global_state(machine)->stack_has_special_main_menu(); }

	// master handler
	static uint32_t ui_handler(render_container &container, mame_ui_manager &mui);

	// Used by sliders
	void validate_selection(int scandir);

	void do_handle();

private:
	virtual void draw(uint32_t flags);
	void draw_text_box();

protected:
	using cleanup_callback = std::function<void(running_machine &)>;
	using bitmap_ptr = widgets_manager::bitmap_ptr;
	using texture_ptr = widgets_manager::texture_ptr;

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

	// menu-related events
	struct event
	{
		void                *itemref;   // reference for the selected item
		menu_item_type      type;       // item type (eventually will go away when itemref is proper ui_menu_item class rather than void*)
		int                 iptkey;     // one of the IPT_* values from inptport.h
		char32_t            unichar;    // unicode character if iptkey == IPT_SPECIAL
		render_bounds       mouse;      // mouse position if iptkey == IPT_CUSTOM
	};

	int                     hover;        // which item is being hovered over
	std::vector<menu_item>  item;         // array of items

	int top_line;           // main box top line
	int skip_main_items;
	int selected;           // which item is selected

	int m_visible_lines;    // main box visible lines
	int m_visible_items;    // number of visible items

	menu(mame_ui_manager &mui, render_container &container);

	mame_ui_manager &ui() const { return m_ui; }
	running_machine &machine() const { return m_ui.machine(); }
	render_container &container() const { return m_container; }

	// allocate temporary memory from the menu's memory pool
	void *m_pool_alloc(size_t size);

	void reset(reset_options options);
	void reset_parent(reset_options options) { m_parent->reset(options); }

	template <typename T> T *topmost_menu() const { return m_global_state->topmost_menu<T>(); }
	template <typename T> static T *topmost_menu(running_machine &machine) { return get_global_state(machine)->topmost_menu<T>(); }
	void stack_pop() { m_global_state->stack_pop(); }
	void stack_reset() { m_global_state->stack_reset(); }
	bool stack_has_special_main_menu() const { return m_global_state->stack_has_special_main_menu(); }

	void add_cleanup_callback(cleanup_callback &&callback) { m_global_state->add_cleanup_callback(std::move(callback)); }

	// repopulate the menu items
	void repopulate(reset_options options);

	// process a menu, drawing it and returning any interesting events
	const event *process(uint32_t flags, float x0 = 0.0f, float y0 = 0.0f);
	void process_parent() { m_parent->process(PROCESS_NOINPUT); }

	// retrieves the ref of the currently selected menu item or nullptr
	void *get_selection_ref() const { return selection_valid() ? item[selected].ref : nullptr; }

	menu_item &selected_item() { return item[selected]; }
	menu_item const &selected_item() const { return item[selected]; }
	int selected_index() const { return selected; }
	bool selection_valid() const { return (0 <= selected) && (item.size() > selected); }
	bool is_selected(int index) const { return selection_valid() && (selected == index); }
	bool is_first_selected() const { return 0 == selected; }
	bool is_last_selected() const { return (item.size() - 1) == selected; }

	// changes the index of the currently selected menu item
	void set_selection(void *selected_itemref);

	// scroll position control
	void centre_selection() { top_line = selected - (m_visible_lines / 2); }

	// test if the given key is pressed and we haven't already reported a key
	bool exclusive_input_pressed(int &iptkey, int key, int repeat);

	// layout
	float get_customtop() const { return m_customtop; }
	float get_custombottom() const { return m_custombottom; }

	// highlight
	void highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor);
	render_texture *hilight_main_texture() { return m_global_state->hilight_main_texture(); }

	// draw arrow
	void draw_arrow(float x0, float y0, float x1, float y1, rgb_t fgcolor, uint32_t orientation);

	// draw header and footer text
	void extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, const char *header, const char *footer);
	void extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
		int direction, float &x1, float &y1, float &x2, float &y2);

	// draw a box of text - used for the custom boxes above/below menus
	template <typename Iter>
	float draw_text_box(
			Iter begin, Iter end,
			float origx1, float origx2, float y1, float y2,
			ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, bool scale,
			rgb_t fgcolor, rgb_t bgcolor, float text_size)
	{
		// size up the text
		float maxwidth(origx2 - origx1);
		for (Iter it = begin; it != end; ++it)
		{
			float width;
			ui().draw_text_full(
					container(), get_c_str(*it),
					0.0f, 0.0f, 1.0f, justify, wrap,
					mame_ui_manager::NONE, rgb_t::black(), rgb_t::white(),
					&width, nullptr, text_size);
			width += 2.0f * UI_BOX_LR_BORDER;
			maxwidth = (std::max)(maxwidth, width);
		}
		if (scale && ((origx2 - origx1) < maxwidth))
		{
			text_size *= ((origx2 - origx1) / maxwidth);
			maxwidth = origx2 - origx1;
		}

		// draw containing box
		float x1(0.5f * (1.0f - maxwidth));
		float x2(x1 + maxwidth);
		ui().draw_outlined_box(container(), x1, y1, x2, y2, bgcolor);

		// inset box and draw content
		x1 += UI_BOX_LR_BORDER;
		x2 -= UI_BOX_LR_BORDER;
		y1 += UI_BOX_TB_BORDER;
		y2 -= UI_BOX_TB_BORDER;
		for (Iter it = begin; it != end; ++it)
		{
			ui().draw_text_full(
					container(), get_c_str(*it),
					x1, y1, x2 - x1, justify, wrap,
					mame_ui_manager::NORMAL, fgcolor, UI_TEXT_BG_COLOR,
					nullptr, nullptr, text_size);
			y1 += ui().get_line_height();
		}

		// in case you want another box of similar width
		return maxwidth;
	}

	void draw_background();

	// draw additional menu content
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// map mouse to menu coordinates
	void map_mouse();

	// clear the mouse position
	void ignore_mouse();

	bool is_mouse_hit() const { return m_mouse_hit; }   // is mouse pointer inside menu's render container?
	float get_mouse_x() const { return m_mouse_x; }     // mouse x location in menu coordinates
	float get_mouse_y() const { return m_mouse_y; }     // mouse y location in menu coordinates

	// mouse hit test - checks whether mouse_x is in [x0, x1) and mouse_y is in [y0, y1)
	bool mouse_in_rect(float x0, float y0, float x1, float y1) const
	{
		return m_mouse_hit && (m_mouse_x >= x0) && (m_mouse_x < x1) && (m_mouse_y >= y0) && (m_mouse_y < y1);
	}

	// overridable event handling
	virtual void handle_events(uint32_t flags, event &ev);
	virtual void handle_keys(uint32_t flags, int &iptkey);
	virtual bool custom_mouse_down() { return false; }

	// test if search is active
	virtual bool menu_has_search_active() { return false; }

	static bool is_selectable(menu_item const &item)
	{
		return ((item.flags & (menu::FLAG_MULTILINE | menu::FLAG_DISABLE)) == 0 && item.type != menu_item_type::SEPARATOR);
	}

	// get arrows status
	template <typename T>
	static uint32_t get_arrow_flags(T min, T max, T actual)
	{
		return ((actual > min) ? FLAG_LEFT_ARROW : 0) | ((actual < max) ? FLAG_RIGHT_ARROW : 0);
	}

private:
	class global_state : public widgets_manager
	{
	public:
		global_state(running_machine &machine, ui_options const &options);
		global_state(global_state const &) = delete;
		global_state(global_state &&) = delete;
		~global_state();

		void add_cleanup_callback(cleanup_callback &&callback);

		bitmap_argb32 *bgrnd_bitmap() { return m_bgrnd_bitmap.get(); }
		render_texture *bgrnd_texture() { return m_bgrnd_texture.get(); }

		template <typename T>
		T *topmost_menu() const { return dynamic_cast<T *>(m_stack.get()); }

		void stack_push(std::unique_ptr<menu> &&menu);
		void stack_pop();
		void stack_reset();
		void clear_free_list();
		bool stack_has_special_main_menu() const;

	private:
		using cleanup_callback_vector = std::vector<cleanup_callback>;

		running_machine         &m_machine;
		cleanup_callback_vector m_cleanup_callbacks;

		bitmap_ptr              m_bgrnd_bitmap;
		texture_ptr             m_bgrnd_texture;

		std::unique_ptr<menu>   m_stack;
		std::unique_ptr<menu>   m_free;
	};
	using global_state_ptr = std::shared_ptr<global_state>;
	using global_state_map = std::map<running_machine *, global_state_ptr>;

	struct pool
	{
		pool       *next;    // chain to next one
		uint8_t    *top;     // top of the pool
		uint8_t    *end;     // end of the pool
	};

	// request the specific handling of the game selection main menu
	bool is_special_main_menu() const;
	void set_special_main_menu(bool disable);

	// To be reimplemented in the menu subclass
	virtual void populate(float &customtop, float &custombottom) = 0;

	// To be reimplemented in the menu subclass
	virtual void handle() = 0;

	// push a new menu onto the stack
	static void stack_push(std::unique_ptr<menu> &&menu) { get_global_state(menu->machine())->stack_push(std::move(menu)); }

	void extra_text_draw_box(float origx1, float origx2, float origy, float yspan, const char *text, int direction);

	bool first_item_visible() const { return top_line <= 0; }
	bool last_item_visible() const { return (top_line + m_visible_lines) >= item.size(); }

	static void exit(running_machine &machine);
	static global_state_ptr get_global_state(running_machine &machine);

	static char const *get_c_str(std::string const &str) { return str.c_str(); }
	static char const *get_c_str(char const *str) { return str; }

	global_state_ptr const  m_global_state;
	bool                    m_special_main_menu;
	mame_ui_manager         &m_ui;              // UI we are attached to
	render_container        &m_container;       // render_container we render to
	std::unique_ptr<menu>   m_parent;           // pointer to parent menu
	event                   m_event;            // the UI event that occurred
	pool                    *m_pool;            // list of memory pools

	float                   m_customtop;        // amount of extra height to add at the top
	float                   m_custombottom;     // amount of extra height to add at the bottom

	int                     m_resetpos;         // reset position
	void                    *m_resetref;        // reset reference

	bool                    m_mouse_hit;
	bool                    m_mouse_button;
	float                   m_mouse_x;
	float                   m_mouse_y;

	static std::mutex       s_global_state_guard;
	static global_state_map s_global_states;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_MENU_H
