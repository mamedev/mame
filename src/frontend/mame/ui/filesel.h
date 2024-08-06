// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filesel.h

    MESS's clunky built-in file manager

***************************************************************************/

#ifndef MAME_FRONTEND_UI_FILESEL_H
#define MAME_FRONTEND_UI_FILESEL_H

#pragma once

#include "ui/menu.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


namespace ui {

// ======================> menu_file_selector

class menu_file_selector : public menu
{
public:
	enum class result
	{
		INVALID = -1,
		EMPTY = 0x1000,
		SOFTLIST,
		CREATE,
		FILE
	};

	using handler_function = std::function<void (result result, std::string &&directory, std::string &&file)>;

	menu_file_selector(
			mame_ui_manager &mui,
			render_container &container,
			device_image_interface *image,
			std::string_view directory,
			std::string_view file,
			bool has_empty,
			bool has_softlist,
			bool has_create,
			handler_function &&handler);
	virtual ~menu_file_selector() override;

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual bool custom_ui_back() override { return !m_filename.empty(); }
	virtual std::tuple<int, bool, bool> custom_pointer_updated(bool changed, ui_event const &uievt) override;
	virtual void menu_activated() override;

private:
	enum file_selector_entry_type
	{
		SELECTOR_ENTRY_TYPE_EMPTY,
		SELECTOR_ENTRY_TYPE_CREATE,
		SELECTOR_ENTRY_TYPE_SOFTWARE_LIST,
		SELECTOR_ENTRY_TYPE_DRIVE,
		SELECTOR_ENTRY_TYPE_DIRECTORY,
		SELECTOR_ENTRY_TYPE_FILE
	};

	struct file_selector_entry
	{
		file_selector_entry() = default;
		file_selector_entry(file_selector_entry &&) = default;
		file_selector_entry &operator=(file_selector_entry &&) = default;

		file_selector_entry_type type = SELECTOR_ENTRY_TYPE_EMPTY;
		std::string basename;
		std::string fullpath;
	};

	// internal state
	handler_function const              m_handler;
	device_image_interface *const       m_image;
	std::string                         m_current_directory;
	std::string                         m_current_file;
	std::optional<text_layout>          m_path_layout;
	std::pair<float, float>             m_path_position;
	result                              m_result;
	bool const                          m_has_empty;
	bool const                          m_has_softlist;
	bool const                          m_has_create;
	std::vector<file_selector_entry>    m_entrylist;
	std::string                         m_filename;
	std::pair<size_t, size_t>           m_clicked_directory;

	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	// methods
	file_selector_entry &append_entry(file_selector_entry_type entry_type, const std::string &entry_basename, const std::string &entry_fullpath);
	file_selector_entry &append_entry(file_selector_entry_type entry_type, std::string &&entry_basename, std::string &&entry_fullpath);
	file_selector_entry *append_dirent_entry(const osd::directory::entry *dirent);
	void append_entry_menu_item(const file_selector_entry *entry);
	void select_item(const file_selector_entry &entry);
	void update_search();
	std::pair<size_t, size_t> get_directory_range(float x, float y);
};


// ======================> menu_select_rw

class menu_select_rw : public menu
{
public:
	enum class result
	{
		INVALID = -1,
		READONLY = 0x3000,
		READWRITE,
		WRITE_OTHER,
		WRITE_DIFF
	};

	using handler_function = std::function<void (result result)>;

	menu_select_rw(
			mame_ui_manager &mui,
			render_container &container,
			bool can_in_place,
			handler_function &&handler);
	virtual ~menu_select_rw() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	static void *itemref_from_result(result result);
	static result result_from_itemref(void *itemref);

	// internal state
	handler_function const  m_handler;
	bool const              m_can_in_place;
	result                  m_result;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_FILESEL_H
