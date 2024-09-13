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
#include <tuple>
#include <unordered_map>


namespace ui {

class menu_load_save_state_base : public autopause_menu<>
{
public:
	virtual ~menu_load_save_state_base() override;

protected:
	menu_load_save_state_base(
			mame_ui_manager &mui,
			render_container &container,
			std::string_view header,
			std::string_view footer,
			bool must_exist,
			bool one_shot);

	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool handle_keys(uint32_t flags, int &iptkey) override;
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

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
	std::string_view const                          m_footer;
	std::string                                     m_delete_prompt;
	std::string                                     m_confirm_prompt;
	file_entry const *                              m_confirm_delete;
	bool const                                      m_must_exist;
	bool                                            m_keys_released;
	input_code                                      m_slot_selected;

	static void *itemref_from_file_entry(const file_entry &entry);
	static const file_entry &file_entry_from_itemref(void *itemref);

	bool try_select_slot(std::string &&name);
	void slot_selected(std::string &&name);
	std::string state_directory() const;
	bool is_present(const std::string &name) const;
	std::string poll_inputs(input_code &code);
	std::string get_visible_name(const std::string &file_name);
};


class menu_load_state : public menu_load_save_state_base
{
public:
	menu_load_state(mame_ui_manager &mui, render_container &container, bool one_shot);

protected:
	virtual void process_file(std::string &&file_name) override;
};


class menu_save_state : public menu_load_save_state_base
{
public:
	menu_save_state(mame_ui_manager &mui, render_container &container, bool one_shot);

protected:
	virtual void process_file(std::string &&file_name) override;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_STATE_H
