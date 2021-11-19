// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods

/***************************************************************************

    ui/menuitem.h

    Internal data representation for a UI menu item.

***************************************************************************/

#ifndef MAME_FRONTEND_UI_MENUITEM_H
#define MAME_FRONTEND_UI_MENUITEM_H

#pragma once


#include <cstdint>
#include <string>
#include <utility>


namespace ui {

// special menu item for separators
#define MENU_SEPARATOR_ITEM         "---"

// types of menu items (TODO: please expand)
enum class menu_item_type
{
	UNKNOWN,
	SLIDER,
	SEPARATOR
};

class menu_item
{
public:
	menu_item(menu_item const &) = default;
	menu_item(menu_item &&) = default;
	menu_item &operator=(menu_item const &) = default;
	menu_item &operator=(menu_item &&) = default;

	menu_item(menu_item_type t = menu_item_type::UNKNOWN, void *r = nullptr, uint32_t f = 0) : m_ref(r), m_flags(f), m_type(t)
	{ }

	std::string const &text() const noexcept { return m_text; }
	std::string const &subtext() const noexcept { return m_subtext; }
	void *ref() const noexcept { return m_ref; }
	uint32_t flags() const noexcept { return m_flags; }
	unsigned generation() const noexcept { return m_generation; }
	menu_item_type type() const noexcept { return m_type; }

	template <typename... T> void set_text(T &&... args) { m_text.assign(std::forward<T>(args)...); ++m_generation; }
	template <typename... T> void set_subtext(T &&... args) { m_subtext.assign(std::forward<T>(args)...); ++m_generation; }
	void set_flags(uint32_t f) noexcept { m_flags = f; ++m_generation; }

private:
	std::string     m_text;
	std::string     m_subtext;
	void            *m_ref;
	uint32_t        m_flags;
	unsigned        m_generation = 0;
	menu_item_type  m_type;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_MENUITEM_H
