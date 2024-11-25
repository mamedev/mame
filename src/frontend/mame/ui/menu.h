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

#include "interface/uievents.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


/***************************************************************************
    FORWARD DECLARATIONS
***************************************************************************/

struct ui_event;


namespace ui {

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class menu
{
public:
	// flags for menu items
	enum : uint32_t
	{
		FLAG_LEFT_ARROW     = 1U << 0,
		FLAG_RIGHT_ARROW    = 1U << 1,
		FLAG_INVERT         = 1U << 2,
		FLAG_DISABLE        = 1U << 4,
		FLAG_UI_HEADING     = 1U << 5,
		FLAG_COLOR_BOX      = 1U << 6
	};

	virtual ~menu();

	// setting menu heading
	template <typename... T>
	void set_heading(T &&... args)
	{
		if (!m_heading)
			m_heading.emplace(std::forward<T>(args)...);
		else
			m_heading->assign(std::forward<T>(args)...);
	}

	// append a new item to the end of the menu
	int item_append(const std::string &text, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN) { return item_append(std::string(text), std::string(), flags, ref, type); }
	int item_append(const std::string &text, const std::string &subtext, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN) { return item_append(std::string(text), std::string(subtext), flags, ref, type); }
	int item_append(std::string &&text, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN) { return item_append(text, std::string(), flags, ref, type); }
	int item_append(std::string &&text, std::string &&subtext, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);
	int item_append(menu_item item) { return item_append(item.text(), item.subtext(), item.flags(), item.ref(), item.type()); }
	int item_append(menu_item_type type, uint32_t flags = 0);
	int item_append_on_off(const std::string &text, bool state, uint32_t flags, void *ref, menu_item_type type = menu_item_type::UNKNOWN);

	// set space required for drawing extra content
	void set_custom_space(float top, float bottom);

	// reset the menus, clearing everything
	static void stack_reset(mame_ui_manager &ui) { get_global_state(ui).stack_reset(); }

	// push a new menu onto the stack
	template <typename T, typename... Params>
	static void stack_push(Params &&... args)
	{
		stack_push(std::make_unique<T>(std::forward<Params>(args)...));
	}
	template <typename T, typename... Params>
	static void stack_push_special_main(Params &&... args)
	{
		std::unique_ptr<menu> ptr(std::make_unique<T>(std::forward<Params>(args)...));
		ptr->set_special_main_menu(true);
		stack_push(std::move(ptr));
	}

	// pop a menu from the stack
	static void stack_pop(mame_ui_manager &ui) { get_global_state(ui).stack_pop(); }

	// test if one of the menus in the stack requires hide disable
	static bool stack_has_special_main_menu(mame_ui_manager &ui) { return get_global_state(ui).stack_has_special_main_menu(); }

	// master handler
	static delegate<uint32_t (render_container &)> get_ui_handler(mame_ui_manager &mui);

	// Used by sliders
	void validate_selection(int scandir);

	bool do_handle();

protected:
	using bitmap_ptr = widgets_manager::bitmap_ptr;
	using texture_ptr = widgets_manager::texture_ptr;

	// flags to pass to set_process_flags
	enum
	{
		PROCESS_NOKEYS      = 1 << 0,
		PROCESS_LR_ALWAYS   = 1 << 1,
		PROCESS_LR_REPEAT   = 1 << 2,
		PROCESS_CUSTOM_NAV  = 1 << 3,
		PROCESS_CUSTOM_ONLY = 1 << 4,
		PROCESS_ONLYCHAR    = 1 << 5,
		PROCESS_NOINPUT     = 1 << 6,
		PROCESS_IGNOREPAUSE = 1 << 7
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
		void        *itemref;   // reference for the selected item or nullptr
		menu_item   *item;      // selected item or nullptr
		int         iptkey;     // one of the IPT_* values from inpttype.h
		char32_t    unichar;    // unicode character if iptkey == IPT_SPECIAL
	};

	menu(mame_ui_manager &mui, render_container &container);

	mame_ui_manager &ui() const { return m_ui; }
	running_machine &machine() const { return m_ui.machine(); }
	render_container &container() const { return m_container; }

	bool is_special_main_menu() const { return m_special_main_menu; }
	bool is_one_shot() const { return m_one_shot; }
	bool is_active() const { return m_active; }

	void set_one_shot(bool oneshot) { m_one_shot = oneshot; }
	void set_needs_prev_menu_item(bool needs) { m_needs_prev_menu_item = needs; }
	void reset(reset_options options);
	void reset_parent(reset_options options) { m_parent->reset(options); }

	template <typename T> T *topmost_menu() const { return m_global_state.topmost_menu<T>(); }
	template <typename T> static T *topmost_menu(mame_ui_manager &ui) { return get_global_state(ui).topmost_menu<T>(); }
	void stack_pop() { m_global_state.stack_pop(); }
	void stack_reset() { m_global_state.stack_reset(); }
	bool stack_has_special_main_menu() const { return m_global_state.stack_has_special_main_menu(); }

	menu_item &item(int index) { return m_items[index]; }
	menu_item const &item(int index) const { return m_items[index]; }
	int item_count() const { return m_items.size(); }

	// retrieves the ref of the currently selected menu item or nullptr
	void *get_selection_ref() const { return selection_valid() ? m_items[m_selected].ref() : nullptr; }

	menu_item &selected_item() { return m_items[m_selected]; }
	menu_item const &selected_item() const { return m_items[m_selected]; }
	int selected_index() const { return m_selected; }
	bool selection_valid() const { return (0 <= m_selected) && (m_items.size() > m_selected); }
	bool is_selected(int index) const { return selection_valid() && (m_selected == index); }
	bool is_first_selected() const { return 0 == m_selected; }
	bool is_last_selected() const { return (m_items.size() - 1) == m_selected; }

	// changes the index of the currently selected menu item
	void set_selection(void *selected_itemref);
	void set_selected_index(int index) { m_selected = index; }
	void select_first_item();
	void select_last_item();

	// scroll position control
	void set_top_line(int index) { top_line = (0 < index) ? (index - 1) : index; }
	void centre_selection() { top_line = m_selected - (m_visible_lines / 2); }

	// test if the given key is pressed and we haven't already reported a key
	bool exclusive_input_pressed(int &iptkey, int key, int repeat);

	// layout
	float get_customtop() const { return m_customtop; }
	float get_custombottom() const { return m_custombottom; }

	std::pair<uint32_t, uint32_t> target_size() const { return m_last_size; }
	float x_aspect() const { return m_last_aspect; }
	float line_height() const { return m_line_height; }
	float gutter_width() const { return m_gutter_width; }
	float tb_border() const { return m_tb_border; }
	float lr_border() const { return m_lr_border; }
	float lr_arrow_width() const { return m_lr_arrow_width; }
	float ud_arrow_width() const { return m_ud_arrow_width; }

	float get_string_width(std::string_view s) { return ui().get_string_width(s, line_height()); }
	text_layout create_layout(float width = 1.0, text_layout::text_justify justify = text_layout::text_justify::LEFT, text_layout::word_wrapping wrap = text_layout::word_wrapping::WORD);

	void draw_text_normal(
			std::string_view text,
			float x, float y, float width,
			text_layout::text_justify justify, text_layout::word_wrapping wrap,
			rgb_t color)
	{
		ui().draw_text_full(
				container(),
				text,
				x, y, width, justify, wrap,
				mame_ui_manager::NORMAL, color, ui().colors().text_bg_color(),
				nullptr, nullptr,
				line_height());
	}

	float get_text_width(
			std::string_view text,
			float x, float y, float width,
			text_layout::text_justify justify, text_layout::word_wrapping wrap)
	{
		float result;
		ui().draw_text_full(
				container(),
				text,
				x, y, width, justify, wrap,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&result, nullptr,
				line_height());
		return result;
	}

	std::pair<float, float> get_text_dimensions(
			std::string_view text,
			float x, float y, float width,
			text_layout::text_justify justify, text_layout::word_wrapping wrap)
	{
		std::pair<float, float> result;
		ui().draw_text_full(
				container(),
				text,
				x, y, width, justify, wrap,
				mame_ui_manager::NONE, rgb_t::white(), rgb_t::black(),
				&result.first, &result.second,
				line_height());
		return result;
	}

	// highlight
	void highlight(float x0, float y0, float x1, float y1, rgb_t bgcolor);
	render_texture *hilight_main_texture() { return m_global_state.hilight_main_texture(); }

	// draw arrow
	void draw_arrow(float x0, float y0, float x1, float y1, rgb_t fgcolor, uint32_t orientation);

	// draw header and footer text
	void extra_text_render(float top, float bottom, float origx1, float origy1, float origx2, float origy2, std::string_view header, std::string_view footer);
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
		float const origwidth(origx2 - origx1 - (2.0f * lr_border()));
		float maxwidth(origwidth);
		for (Iter it = begin; it != end; ++it)
		{
			std::string_view const &line(*it);
			if (!line.empty())
			{
				text_layout layout(*ui().get_font(), text_size * x_aspect(), text_size, 1.0, justify, wrap);
				layout.add_text(line, rgb_t::white(), rgb_t::black());
				maxwidth = (std::max)(layout.actual_width(), maxwidth);
			}
		}
		if (scale && (origwidth < maxwidth))
		{
			text_size *= origwidth / maxwidth;
			maxwidth = origwidth;
		}

		// draw containing box
		float const boxleft(0.5f - (maxwidth * 0.5f) - lr_border());
		float boxright(0.5f + (maxwidth * 0.5f) + lr_border());
		ui().draw_outlined_box(container(), boxleft, y1, boxright, y2, bgcolor);

		// inset box and draw content
		float const textleft(0.5f - (maxwidth * 0.5f));
		y1 += tb_border();
		for (Iter it = begin; it != end; ++it)
		{
			ui().draw_text_full(
					container(), std::string_view(*it),
					textleft, y1, maxwidth, justify, wrap,
					mame_ui_manager::NORMAL, fgcolor, ui().colors().text_bg_color(),
					nullptr, nullptr, text_size);
			y1 += text_size;
		}

		// in case you want another box of similar width
		return maxwidth;
	}

	template <typename Iter>
	float draw_text_box(
			Iter begin, Iter end,
			float origx1, float origx2, float y1, float y2,
			ui::text_layout::text_justify justify, ui::text_layout::word_wrapping wrap, bool scale,
			rgb_t fgcolor, rgb_t bgcolor)
	{
		return draw_text_box(begin, end, origx1, origx2, y1, y2, justify, wrap, scale, fgcolor, bgcolor, line_height());
	}

	void draw_background();

	// draw additional menu content
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect);
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2);

	// access to pointer state
	bool have_pointer() const noexcept { return m_global_state.have_pointer(); }
	bool pointer_idle() const noexcept { return (track_pointer::IDLE == m_pointer_state) && have_pointer(); }
	bool pointer_in_rect(float x0, float y0, float x1, float y1) const noexcept { return m_global_state.pointer_in_rect(x0, y0, x1, y1); }
	std::pair<float, float> pointer_location() const noexcept { return m_global_state.pointer_location(); }
	osd::ui_event_handler::pointer pointer_type() const noexcept { return m_global_state.pointer_type(); }

	// derived classes that override handle_events need to call these for pointer events
	std::pair<int, bool> handle_pointer_update(uint32_t flags, ui_event const &uievt);
	std::pair<int, bool> handle_pointer_leave(uint32_t flags, ui_event const &uievt);
	std::pair<int, bool> handle_pointer_abort(uint32_t flags, ui_event const &uievt);

	// overridable event handling
	void set_process_flags(uint32_t flags) { m_process_flags = flags; }
	virtual bool handle_events(uint32_t flags, event &ev);
	virtual bool handle_keys(uint32_t flags, int &iptkey);
	virtual bool custom_ui_back();
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt);
	virtual bool custom_mouse_scroll(int lines);

	// event notifications
	virtual void menu_activated();
	virtual void menu_deactivated();
	virtual void menu_dismissed();

	static bool is_selectable(menu_item const &item)
	{
		return (!(item.flags() & menu::FLAG_DISABLE) && (item.type() != menu_item_type::SEPARATOR));
	}

	// get arrows status
	template <typename T>
	static uint32_t get_arrow_flags(T min, T max, T actual)
	{
		return ((actual > min) ? FLAG_LEFT_ARROW : 0) | ((actual < max) ? FLAG_RIGHT_ARROW : 0);
	}

	static bool reentered_rect(float x0, float y0, float x1, float y1, float l, float t, float r, float b)
	{
		return
				((x0 < l) || (x0 >= r) || (y0 < t) || (y0 >= b)) &&
				((x1 >= l) && (x1 < r) && (y1 >= t) && (y1 < b));
	}

	std::pair<bool, bool> check_drag_conversion(float x, float y, float x_base, float y_base, float threshold) const
	{
		float const dx(std::abs((x - x_base) / x_aspect()));
		float const dy(std::abs(y - y_base));
		if ((dx > dy) && (dx >= threshold))
			return std::make_pair(true, false);
		else if ((dy >= dx) && (dy > threshold))
			return std::make_pair(false, true);
		else
			return std::make_pair(false, false);
	}

	template <typename T>
	static T drag_scroll(float location, float base, float &last, float unit, T start, T min, T max)
	{
		// set thresholds depending on the direction for hysteresis and clamp to valid range
		T const target((location - (std::abs(unit) * ((location > last) ? 0.3F : -0.3F)) - base) / unit);
		last = base + (float(target) * unit);
		return std::clamp(start + target, min, max);
	}

private:
	// pointer tracking state
	enum class track_pointer
	{
		IDLE,
		IGNORED,
		COMPLETED,
		CUSTOM,
		TRACK_LINE,
		SCROLL,
		ADJUST
	};

	class global_state : public widgets_manager
	{
	public:
		global_state(mame_ui_manager &ui);
		global_state(global_state const &) = delete;
		global_state(global_state &&) = delete;
		~global_state();

		bitmap_argb32 *bgrnd_bitmap() { return m_bgrnd_bitmap.get(); }
		render_texture *bgrnd_texture() { return m_bgrnd_texture.get(); }

		template <typename T>
		T *topmost_menu() const { return dynamic_cast<T *>(m_stack.get()); }

		void stack_push(std::unique_ptr<menu> &&menu);
		void stack_pop();
		void stack_reset();
		void clear_free_list();
		bool stack_has_special_main_menu() const;

		void hide_menu() { m_hide = true; }

		uint32_t ui_handler(render_container &container);

		bool have_pointer() const noexcept
		{
			return 0 <= m_current_pointer;
		}
		bool pointer_in_rect(float x0, float y0, float x1, float y1) const noexcept
		{
			return (m_pointer_x >= x0) && (m_pointer_x < x1) && (m_pointer_y >= y0) && (m_pointer_y < y1);
		}
		std::pair<float, float> pointer_location() const noexcept
		{
			return std::make_pair(m_pointer_x, m_pointer_y);
		}
		osd::ui_event_handler::pointer pointer_type() const noexcept
		{
			return m_pointer_type;
		}

		std::pair<bool, bool> use_pointer(render_target &target, render_container &container, ui_event const &event);

	protected:
		mame_ui_manager                 &m_ui;

	private:
		bitmap_ptr                      m_bgrnd_bitmap;
		texture_ptr                     m_bgrnd_texture;

		std::unique_ptr<menu>           m_stack;
		std::unique_ptr<menu>           m_free;

		bool                            m_hide;

		s32                             m_current_pointer;      // current active pointer ID or -1 if none
		osd::ui_event_handler::pointer  m_pointer_type;         // current pointer type
		u32                             m_pointer_buttons;      // depressed buttons for current pointer
		float                           m_pointer_x;
		float                           m_pointer_y;
		bool                            m_pointer_hit;
	};

	// this is to satisfy the std::any requirement that objects be copyable
	class global_state_wrapper : public global_state
	{
	public:
		global_state_wrapper(mame_ui_manager &ui) : global_state(ui) { }
		global_state_wrapper(global_state_wrapper const &that) : global_state(that.m_ui) { }
	};

	// process a menu, returning any interesting events
	std::pair<int, bool> handle_primary_down(uint32_t flags, ui_event const &uievt);
	std::pair<int, bool> update_line_click(ui_event const &uievt);
	bool update_drag_scroll(ui_event const &uievt);
	std::pair<int, bool> update_drag_adjust(ui_event const &uievt);
	std::pair<int, bool> check_touch_drag(ui_event const &uievt);

	// drawing the menu
	void do_draw_menu();
	virtual void draw(uint32_t flags);

	// request the specific handling of the game selection main menu
	void set_special_main_menu(bool disable);

	// to be implemented in derived classes
	virtual void populate() = 0;

	// to be implemented in derived classes
	virtual bool handle(event const *ev) = 0;

	void extra_text_draw_box(float origx1, float origx2, float origy, float yspan, std::string_view text, int direction);

	void activate_menu();
	bool check_metrics();
	bool do_rebuild();
	bool first_item_visible() const { return top_line <= 0; }
	bool last_item_visible() const { return (top_line + m_visible_lines) >= m_items.size(); }
	void force_visible_selection();

	// push a new menu onto the stack
	static void stack_push(std::unique_ptr<menu> &&menu) { menu->m_global_state.stack_push(std::move(menu)); }

	static global_state &get_global_state(mame_ui_manager &ui);

protected: // TODO: remove need to expose these - only used here and in selmenu.cpp
	int top_line;           // main box top line
	int m_visible_lines;    // main box visible lines
	int m_visible_items;    // number of visible items

private:
	global_state            &m_global_state;        // reference to global state for session
	mame_ui_manager         &m_ui;                  // UI we are attached to
	render_container        &m_container;           // render_container we render to
	std::unique_ptr<menu>   m_parent;               // pointer to parent menu in the stack

	std::optional<std::string> m_heading;           // menu heading
	std::vector<menu_item>  m_items;                // array of items
	bool                    m_rebuilding;           // ensure items are only added during rebuild

	std::pair<uint32_t, uint32_t> m_last_size;      // pixel size of UI container when metrics were computed
	float                   m_last_aspect;          // aspect ratio of UI container when metrics were computed
	float                   m_line_height;
	float                   m_gutter_width;
	float                   m_tb_border;
	float                   m_lr_border;
	float                   m_lr_arrow_width;
	float                   m_ud_arrow_width;

	float                   m_items_left;           // left of the area where the items are drawn
	float                   m_items_right;          // right of the area where the items are drawn
	float                   m_items_top;            // top of the area where the items are drawn
	float                   m_adjust_top;           // top of the "increase"/"decrease" arrows
	float                   m_adjust_bottom;        // bottom of the "increase"/"decrease" arrows
	float                   m_decrease_left;        // left of the "decrease" arrow
	float                   m_increase_left;        // left of the "increase" arrow
	bool                    m_show_up_arrow;        // are we showing the "scroll up" arrow?
	bool                    m_show_down_arrow;      // are we showing the "scroll down" arrow?
	bool                    m_items_drawn;          // have we drawn the items at least once?

	track_pointer           m_pointer_state;        // tracking state for currently active pointer
	std::pair<float, float> m_pointer_down;         // start location of tracked pointer action
	std::pair<float, float> m_pointer_updated;      // location where pointer tracking was updated
	int                     m_pointer_line;         // the line we're tracking pointer motion in
	std::chrono::steady_clock::time_point m_pointer_repeat;
	int                     m_accumulated_wheel;    // accumulated scroll wheel/gesture movement

	uint32_t                m_process_flags;        // event processing options
	int                     m_selected;             // which item is selected
	bool                    m_special_main_menu;    // true if no real emulation running under the menu
	bool                    m_one_shot;             // true for menus outside the normal stack
	bool                    m_needs_prev_menu_item; // true to automatically create item to dismiss menu
	bool                    m_active;               // whether the menu is currently visible and topmost

	float                   m_customtop;            // amount of extra height to add at the top
	float                   m_custombottom;         // amount of extra height to add at the bottom

	int                     m_resetpos;             // item index to select after repopulating
	void                    *m_resetref;            // item reference value to select after repopulating
};


template <typename Base = menu>
class autopause_menu : public Base
{
protected:
	using Base::Base;

	virtual void menu_activated() override
	{
		m_was_paused = this->machine().paused();
		if (m_was_paused)
			m_unpaused = false;
		else if (!m_unpaused)
			this->machine().pause();
		Base::menu_activated();
	}

	virtual void menu_deactivated() override
	{
		m_unpaused = !this->machine().paused();
		if (!m_was_paused && !m_unpaused)
			this->machine().resume();
		Base::menu_deactivated();
	}

private:
	bool m_was_paused = false;
	bool m_unpaused = false;
};


} // namespace ui

#endif  // MAME_FRONTEND_UI_MENU_H
