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
	enum
	{
		FLAG_LEFT_ARROW     = (1 << 0),
		FLAG_RIGHT_ARROW    = (1 << 1),
		FLAG_INVERT         = (1 << 2),
		FLAG_MULTILINE      = (1 << 3),
		FLAG_REDTEXT        = (1 << 4),
		FLAG_DISABLE        = (1 << 5),
		FLAG_UI_DATS        = (1 << 6),
		FLAG_UI_FAVORITE    = (1 << 7),
		FLAG_UI_HEADING     = (1 << 8)
	};

	virtual ~menu();

	int                     hover;        // which item is being hovered over
	float                   customtop;    // amount of extra height to add at the top
	float                   custombottom; // amount of extra height to add at the bottom
	std::vector<menu_item>  item;         // array of items

	// append a new item to the end of the menu
	void item_append(const std::string &text, const std::string &subtext, UINT32 flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	void item_append(std::string &&text, std::string &&subtext, UINT32 flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	void item_append(menu_item item);
	void item_append(menu_item_type type, UINT32 flags = 0);

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
	static UINT32 ui_handler(render_container &container, mame_ui_manager &mui);

	// Used by sliders
	void move_selection(int delta, UINT32 flags = 0);
	void validate_selection(int scandir);

	void do_handle();

private:
	virtual void draw(UINT32 flags);
	void draw_text_box();

public:
	// mouse handling
	bool mouse_hit, mouse_button;
	render_target *mouse_target;
	INT32 mouse_target_x, mouse_target_y;
	float mouse_x, mouse_y;

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

	int top_line;           // main box top line
	int l_sw_hover;
	int l_hover;
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
	void reset_topmost(reset_options options) { m_global_state->reset_topmost(options); }

	template <typename T> T *topmost_menu() const { return m_global_state->topmost_menu<T>(); }
	template <typename T> static T *topmost_menu(running_machine &machine) { return get_global_state(machine)->topmost_menu<T>(); }
	void stack_pop() { m_global_state->stack_pop(); }
	void stack_reset() { m_global_state->stack_reset(); }
	bool stack_has_special_main_menu() const { return m_global_state->stack_has_special_main_menu(); }

	void add_cleanup_callback(cleanup_callback &&callback) { m_global_state->add_cleanup_callback(std::move(callback)); }

	// process a menu, drawing it and returning any interesting events
	const event *process(UINT32 flags, float x0 = 0.0f, float y0 = 0.0f);
	void process_parent() { m_parent->process(PROCESS_NOINPUT); }

	// retrieves the ref of the currently selected menu item or nullptr
	void *get_selection_ref() const { return selection_valid() ? item[selected].ref : nullptr; }
	bool selection_valid() const { return (0 <= selected) && (item.size() > selected); }
	bool is_first_selected() const { return 0 == selected; }
	bool is_last_selected() const { return (item.size() - 1) == selected; }

	// changes the index of the currently selected menu item
	void set_selection(void *selected_itemref);

	// scroll position control
	void centre_selection() { top_line = selected - (m_visible_lines / 2); }

	// test if the given key is pressed and we haven't already reported a key
	bool exclusive_input_pressed(int &iptkey, int key, int repeat);

	// highlight
	void highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor);
	render_texture *hilight_main_texture() { return m_global_state->hilight_main_texture(); }

	// draw arrow
	void draw_arrow(float x0, float y0, float x1, float y1, rgb_t fgcolor, UINT32 orientation);

	// draw header and footer text
	void extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, const char *header, const char *footer);
	void extra_text_position(float origx1, float origx2, float origy, float yspan, text_layout &layout,
		int direction, float &x1, float &y1, float &x2, float &y2);

	void draw_background();

	// configure the menu for custom rendering
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	// overridable event handling
	virtual void handle_events(UINT32 flags, event &ev);
	virtual void handle_keys(UINT32 flags, int &iptkey);
	virtual bool custom_mouse_down() { return false; }
	virtual void selection_changed() { }

	// test if search is active
	virtual bool menu_has_search_active() { return false; }

	static bool is_selectable(menu_item const &item)
	{
		return ((item.flags & (menu::FLAG_MULTILINE | menu::FLAG_DISABLE)) == 0 && item.type != menu_item_type::SEPARATOR);
	}

	// get arrows status
	template <typename T>
	UINT32 get_arrow_flags(T min, T max, T actual)
	{
		return ((actual > min) ? FLAG_LEFT_ARROW : 0) | ((actual < max) ? FLAG_RIGHT_ARROW : 0);
	}

	int right_visible_lines;  // right box lines

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

		void reset_topmost(reset_options options) { if (m_stack) m_stack->reset(options); }

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
		pool   *next;    // chain to next one
		UINT8  *top;     // top of the pool
		UINT8  *end;     // end of the pool
	};

	// request the specific handling of the game selection main menu
	bool is_special_main_menu() const;
	void set_special_main_menu(bool disable);

	// To be reimplemented in the menu subclass
	virtual void populate() = 0;

	// To be reimplemented in the menu subclass
	virtual void handle() = 0;

	// push a new menu onto the stack
	static void stack_push(std::unique_ptr<menu> &&menu) { get_global_state(menu->machine())->stack_push(std::move(menu)); }

	void extra_text_draw_box(float origx1, float origx2, float origy, float yspan, const char *text, int direction);

	bool first_item_visible() const { return top_line <= 0; }
	bool last_item_visible() const { return (top_line + m_visible_lines) >= item.size(); }

	static void exit(running_machine &machine);
	static global_state_ptr get_global_state(running_machine &machine);

	global_state_ptr const  m_global_state;
	bool                    m_special_main_menu;
	mame_ui_manager         &m_ui;              // UI we are attached to
	render_container        &m_container;       // render_container we render to
	std::unique_ptr<menu>   m_parent;           // pointer to parent menu
	event                   m_event;            // the UI event that occurred
	pool                    *m_pool;            // list of memory pools

	int                     m_resetpos;         // reset position
	void                    *m_resetref;        // reset reference

	static std::mutex       s_global_state_guard;
	static global_state_map s_global_states;
};

} // namespace ui

#endif  // MAME_FRONTEND_UI_MENU_H
