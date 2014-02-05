/***************************************************************************

    menubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_MENUBAR_H__
#define __UI_MENUBAR_H__

#include "render.h"


//**************************************************************************
//  MENU BAR
//**************************************************************************

class ui_menubar
{
public:
	ui_menubar(running_machine &machine);
	virtual ~ui_menubar();

	virtual void reset();
	void handle(render_container *container);

	// getters
	bool is_visible() const { return m_menubar_visibility != MENUBAR_VISIBILITY_INVISIBLE; }

protected:
	class menu_item
	{
	public:
		menu_item(ui_menubar &menubar, const char *text = NULL, menu_item *parent = NULL, bool is_invokable = false, int shortcut = 0);
		virtual ~menu_item();

		// methods
		menu_item &append(const char *text);
		void append_separator();
		bool is_child_of(menu_item *that) const;
		virtual void invoke();
		void clear_area_recursive();
		menu_item *find_point(float x, float y);
		const char *shortcut_text();
		float shortcut_text_width();
		void sensible_seq_name(astring &text, const input_seq &seq);

		// template methods; look I tried to use delegate.h but I got humbled...
		template<class _Target> menu_item &append(const char *text, void (_Target::*callback)(), _Target &obj, int shortcut = 0)
		{
			menu_item *child = new invokable_menu_item<_Target>(m_menubar, text, this, callback, obj, shortcut);
			initialize(*child);
			return *child;
		}
		template<class _Target, typename _Arg> menu_item &append(const char *text, void (_Target::*callback)(_Arg), _Target &obj, _Arg arg, int shortcut = 0)
		{
			menu_item *child = new arg_invokable_menu_item<_Target, _Arg>(m_menubar, text, this, callback, obj, arg, shortcut);
			initialize(*child);
			return *child;
		}
		template<class _Target, typename _Arg1, typename _Arg2> menu_item &append(const char *text, void (_Target::*callback)(_Arg1, _Arg2), _Target &obj, _Arg1 arg1, _Arg2 arg2, int shortcut = 0)
		{
			menu_item *child = new arg2_invokable_menu_item<_Target, _Arg1, _Arg2>(m_menubar, text, this, callback, obj, arg1, arg2, shortcut);
			initialize(*child);
			return *child;
		}
		template<class _Target> menu_item &append(const char *text, void (_Target::*set_callback)(bool), bool (_Target::*get_callback)() const, _Target &obj, int shortcut = 0)
		{
			// tailored for a toggle
			bool current_value = ((obj).*(get_callback))();
			menu_item &menu = append(text, set_callback, obj, !current_value, shortcut);
			menu.set_checked(current_value);
			return menu;
		}
		template<class _Target, typename _Arg> menu_item &append(const char *text, void (_Target::*set_callback)(_Arg), _Arg (_Target::*get_callback)() const, _Target &obj, _Arg arg, int shortcut = 0)
		{
			// tailored for a set operation
			_Arg current_value = ((obj).*(get_callback))();
			menu_item &menu = append(text, set_callback, obj, arg, shortcut);
			menu.set_checked(current_value == arg);
			return menu;
		}

		// getters
		bool is_empty() const { return !m_first_child; }
		bool is_invokable() const { return m_is_invokable; }
		bool is_checked() const { return m_is_checked; }
		bool is_enabled() const { return m_is_enabled; }
		bool is_separator() const { return m_is_separator; }
		bool has_children() const { return m_first_child ? true : false; }
		int shortcut() const { return m_shortcut; }
		const astring &text() const { return m_text; }
		menu_item *parent() { return m_parent; }
		menu_item *child() { return m_first_child; }
		menu_item *last_child() { return m_last_child; }
		menu_item *previous() { return m_previous; }
		menu_item *next() { return m_next; }
		menu_item *next_with_shortcut() { return m_next_with_shortcut; }
		bool is_sub_menu() const { return m_parent && m_parent->m_parent; }

		// setters
		void set_area(float x0, float y0, float x1, float y1);
		void clear_area() { set_area(-1, -1, -1, -1); }
		void set_checked(bool checked) { m_is_checked = checked; }
		void set_enabled(bool enabled) { m_is_enabled = enabled; }
		void set_text(const char *text) { m_text.cpy(text); }
		void set_next_with_shortcut(menu_item *item) { m_next_with_shortcut = item; }

	private:
		// private variables
		ui_menubar &	m_menubar;
		astring			m_text;
		int				m_shortcut;
		astring			m_shortcut_text;
		float			m_shortcut_text_width;
		bool			m_is_invokable;
		bool			m_is_checked;
		bool			m_is_enabled;
		bool			m_is_separator;
		menu_item *		m_parent;
		menu_item *		m_first_child;
		menu_item *		m_last_child;
		menu_item *		m_previous;
		menu_item *		m_next;
		menu_item *		m_next_with_shortcut;
		float			m_x0;
		float			m_y0;
		float			m_x1;
		float			m_y1;

		// private methods
		void initialize(menu_item &child);
	};

	// implemented by child classes
	virtual void menubar_build_menus() = 0;
	virtual void menubar_draw_ui_elements() = 0;

	// accessors
	running_machine &machine() { return m_machine; }
	render_container *container() { return m_container; }
	menu_item &root_menu() { return m_menus; }

private:
	// classes
	template<class _Target>
	class invokable_menu_item : public menu_item
	{
	public:
		invokable_menu_item(ui_menubar &menubar, const char *name, menu_item *parent, void (_Target::*callback)(), _Target &obj, int shortcut)
			: menu_item(menubar, name, parent, true, shortcut), m_callback(callback), m_obj(obj)
		{
		}

		virtual void invoke()	{ ((m_obj).*(m_callback))(); }

	private:
		void (_Target::*m_callback)();
		_Target &m_obj;
	};

	template<class _Target, typename _Arg>
	class arg_invokable_menu_item : public menu_item
	{
	public:
		arg_invokable_menu_item(ui_menubar &menubar, const char *name, menu_item *parent, void (_Target::*callback)(_Arg), _Target &obj, _Arg arg, int shortcut)
			: menu_item(menubar, name, parent, true, shortcut), m_callback(callback), m_obj(obj), m_arg(arg)
		{
		}

		virtual void invoke()	{ ((m_obj).*(m_callback))(m_arg); }

	private:
		void (_Target::*m_callback)(_Arg);
		_Target &m_obj;
		_Arg m_arg;
	};

	template<class _Target, typename _Arg1, typename _Arg2>
	class arg2_invokable_menu_item : public menu_item
	{
	public:
		arg2_invokable_menu_item(ui_menubar &menubar, const char *name, menu_item *parent, void (_Target::*callback)(_Arg1, _Arg2), _Target &obj, _Arg1 arg1, _Arg2 arg2, int shortcut)
			: menu_item(menubar, name, parent, true, shortcut), m_callback(callback), m_obj(obj), m_arg1(arg1), m_arg2(arg2)
		{
		}

		virtual void invoke()	{ ((m_obj).*(m_callback))(m_arg1, m_arg2); }

	private:
		void (_Target::*m_callback)(_Arg1, _Arg2);
		_Target &m_obj;
		_Arg1 m_arg1;
		_Arg2 m_arg2;
	};

	// menubar visibility
	enum menubar_visibility_t
	{
		MENUBAR_VISIBILITY_INVISIBLE,
		MENUBAR_VISIBILITY_TRANSLUCENT,
		MENUBAR_VISIBILITY_VISIBLE
	};

	// instance variables
	running_machine &		m_machine;
	render_container *		m_container;
	menu_item				m_menus;					// the root menu item
	menu_item *				m_shortcuted_menu_items;	// list of menu items with shortcuts
	menu_item *				m_selected_item;			// current selection
	menu_item *				m_active_item;				// active menu item
	bool					m_dragged;					// have we dragged over at least one item?
	float					m_mouse_x, m_mouse_y;
	bool					m_mouse_button;
	float					m_checkmark_width;
	osd_ticks_t				m_last_mouse_move;
	menubar_visibility_t	m_menubar_visibility;

	// selection walking
	bool walk_selection_previous();
	bool walk_selection_next();
	bool walk_selection_child();
	bool walk_selection_parent();
	bool walk_selection_previous_peer();
	bool walk_selection_next_peer();

	// miscellaneous
	void draw_child_menu(menu_item *menu, float x, float y);
	bool is_child_menu_visible(menu_item *menu) const;
	void draw_menu_item_text(menu_item *mi, float x0, float y0, float x1, float y1, bool decorations, const float *column_widths = NULL);
	bool event_loop();
	bool poll_navigation_keys();
	bool poll_shortcut_keys();
	bool input_pressed_safe(int key);
	void toggle_selection();
	void invoke(menu_item *menu);
	bool find_mouse(float &mouse_x, float &mouse_y, bool &mouse_button);
	menubar_visibility_t get_menubar_visibility();
	rgb_t adjust_color(rgb_t color);
};


#endif /* __UI_MENUBAR_H__ */
