// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/state.h

    Menus for saving and loading state

***************************************************************************/
#ifndef MAME_FRONTEND_UI_STATE_H
#define MAME_FRONTEND_UI_STATE_H

#pragma once

#include "ui/menu.h"

#include "iptseqpoll.h"

#include <chrono>
#include <unordered_map>


namespace ui {

class menu_load_save_state_base : public menu
{
public:
	virtual ~menu_load_save_state_base() override;

protected:
	menu_load_save_state_base(mame_ui_manager &mui, render_container &container, std::string_view header, std::string_view footer, bool must_exist);

	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;
	virtual void handle_keys(uint32_t flags, int &iptkey) override;
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;

	virtual void process_file(std::string &&file_name) = 0;

private:
	class file_entry
	{
	public:
		file_entry() = delete;
		file_entry(const file_entry &) = delete;
		file_entry(file_entry &&) = default;
		file_entry(std::string &&file_name, std::string &&visible_name, const std::chrono::system_clock::time_point &last_modified);

		const std::string &file_name() const { return m_file_name; }
		const std::string &visible_name() const { return m_visible_name; }
		const std::chrono::system_clock::time_point &last_modified() const { return m_last_modified; }

	private:
		std::string                             m_file_name;            // filename for the state itself
		std::string                             m_visible_name;         // how it appears in the dialog
		std::chrono::system_clock::time_point   m_last_modified;
	};

	static std::string                              s_last_file_selected;

	switch_code_poller                              m_switch_poller;
	std::unordered_map<std::string, file_entry>     m_file_entries;
	std::unordered_map<std::string, std::string>    m_filename_to_code_map;
	std::string_view const                          m_header;
	std::string_view const                          m_footer;
	std::string                                     m_delete_prompt;
	std::string                                     m_confirm_prompt;
	file_entry const *                              m_confirm_delete;
	bool const                                      m_must_exist;
	bool                                            m_first_time;
	bool                                            m_was_paused;
	bool                                            m_keys_released;

	static void *itemref_from_file_entry(const file_entry &entry);
	static const file_entry &file_entry_from_itemref(void *itemref);

	void try_select_slot(std::string &&name);
	void slot_selected(std::string &&name);
	std::string state_directory() const;
	bool is_present(const std::string &name) const;
	std::string poll_inputs();
	std::string get_visible_name(const std::string &file_name);
};


class menu_load_state : public menu_load_save_state_base
{
public:
	menu_load_state(mame_ui_manager &mui, render_container &container);

protected:
	virtual void process_file(std::string &&file_name) override;
};


class menu_save_state : public menu_load_save_state_base
{
public:
	menu_save_state(mame_ui_manager &mui, render_container &container);

protected:
	virtual void process_file(std::string &&file_name) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_STATE_H
