// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	ui/state.h

	Menus for saving and loading state

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_STATE_H
#define MAME_FRONTEND_UI_STATE_H

#include "ui/menu.h"

namespace ui {

// ======================> menu_load_save_state_base

class menu_load_save_state_base : public menu
{
public:
	virtual ~menu_load_save_state_base() override;
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

protected:
	menu_load_save_state_base(mame_ui_manager &mui, render_container &container, const char *header, const char *footer, bool must_exist);
	virtual void process_file(std::string &&file_name) = 0;

private:
	class file_entry
	{
	public:
		file_entry() = delete;
		file_entry(const file_entry &) = delete;
		file_entry(file_entry &&) = default;
		file_entry(char32_t entry_char, const std::chrono::system_clock::time_point &last_modified);

		char32_t entry_char() const { return m_entry_char; }
		const std::chrono::system_clock::time_point &last_modified() const { return m_last_modified; }

	private:
		char32_t								m_entry_char;
		std::chrono::system_clock::time_point   m_last_modified;
	};

	static char32_t								s_last_file_selected;

	std::unordered_map<char32_t, file_entry>	m_file_entries;
	const char *								m_header;
	const char *								m_footer;
	bool										m_must_exist;
	bool										m_was_paused;

	static void *itemref_from_file_entry(const file_entry &entry);
	static const file_entry &file_entry_from_itemref(void *itemref);
	void slot_selected(char32_t entry_char);
	std::string state_directory() const;
	bool is_present(char32_t entry_char) const;
};

// ======================> menu_load_state

class menu_load_state : public menu_load_save_state_base
{
public:
	menu_load_state(mame_ui_manager &mui, render_container &container);

protected:
	virtual void process_file(std::string &&file_name) override;
};

// ======================> menu_save_state

class menu_save_state : public menu_load_save_state_base
{
public:
	menu_save_state(mame_ui_manager &mui, render_container &container);

protected:
	virtual void process_file(std::string &&file_name) override;
};

};

#endif // MAME_FRONTEND_UI_STATE_H
