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
#include "ui/stackable.h"


//**************************************************************************
//  MENU BAR
//**************************************************************************

class ui_menubar : public ui_stackable
{
public:
	ui_menubar(running_machine &machine, render_container *container);
	~ui_menubar();

	virtual void reset();
	virtual void do_handle();

	// template methods
	template<class _Stackable>
	void push_menu()
	{
		stack_push(auto_alloc_clear(machine(), _Stackable(machine(), container)));
	}

protected:
	class menu_item
	{
	public:
		menu_item(const char *name = NULL, menu_item *parent = NULL, bool is_invokable = false);
		virtual ~menu_item();

		// methods
		menu_item &append(const char *name);
		bool is_child_of(menu_item *that) const;
		virtual void invoke();
		void clear_area_recursive();
		menu_item *find_point(float x, float y);

		// template methods
		template<class _Target> menu_item &append(const char *name, void (_Target::*callback)(), _Target &obj)
		{
			menu_item *child = new invokable_menu_item<_Target>(name, this, callback, obj);
			initialize(*child);
			return *child;
		}
		template<class _Target, typename _Arg> menu_item &append(const char *name, void (_Target::*callback)(_Arg), _Target &obj, _Arg arg)
		{
			menu_item *child = new arg_invokable_menu_item<_Target, _Arg>(name, this, callback, obj, arg);
			initialize(*child);
			return *child;
		}

		// accessors
		bool is_empty() const { return !m_first_child; }
		bool is_invokable() const { return m_is_invokable; }
		bool is_checked() const { return m_is_checked; }
		bool has_children() const { return m_first_child ? true : false; }
		const astring &name() const { return m_name; }
		menu_item *parent() { return m_parent; }
		menu_item *child() { return m_first_child; }
		menu_item *last_child() { return m_last_child; }
		menu_item *previous() { return m_previous; }
		menu_item *next() { return m_next; }
		bool is_sub_menu() const { return m_parent && m_parent->m_parent; }
		void set_area(float x0, float y0, float x1, float y1);
		void clear_area() { set_area(-1, -1, -1, -1); }
		void set_checked(bool checked) { m_is_checked = checked; }

	private:
		// private variables
		astring			m_name;
		bool			m_is_invokable;
		bool			m_is_checked;
		menu_item *		m_parent;
		menu_item *		m_first_child;
		menu_item *		m_last_child;
		menu_item *		m_previous;
		menu_item *		m_next;
		float			m_x0;
		float			m_y0;
		float			m_x1;
		float			m_y1;

		// private methods
		void initialize(menu_item &child);
	};

	// implemented by child classes
	virtual void menubar_build_menus() = 0;

	// accessors
	menu_item &root_menu() { return m_menus; }

private:
	// classes
	template<class _Target>
	class invokable_menu_item : public menu_item
	{
	public:
		invokable_menu_item(const char *name, menu_item *parent, void (_Target::*callback)(), _Target &obj)
			: menu_item(name, parent, true), m_callback(callback), m_obj(obj)
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
		arg_invokable_menu_item(const char *name, menu_item *parent, void (_Target::*callback)(_Arg), _Target &obj, _Arg arg)
			: menu_item(name, parent, true), m_callback(callback), m_obj(obj), m_arg(arg)
		{
		}

		virtual void invoke()	{ ((m_obj).*(m_callback))(m_arg); }

	private:
		void (_Target::*m_callback)(_Arg);
		_Target &m_obj;
		_Arg m_arg;
	};

	// instance variables
	menu_item		m_menus;
	menu_item *		m_selected;
	float			m_mouse_x, m_mouse_y;
	float			m_checkmark_width;
	float			m_submenu_expander_width;

	// selection walking
	bool walk_selection_previous();
	bool walk_selection_next();
	bool walk_selection_child();
	bool walk_selection_parent();
	bool walk_selection_previous_peer();
	bool walk_selection_next_peer();

	// miscellaneous
	void draw_child_menu(menu_item *menu, float x, float y);
	void draw_menu_item_text(menu_item *mi, float x0, float y0, float x1, float y1, bool decorations);
	bool input_pressed_safe(int key);
	void invoke_selection();
};


#endif /* __UI_MENUBAR_H__ */