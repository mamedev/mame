// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filesel.h

    MESS's clunky built-in file manager

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_FILESEL_H
#define MAME_FRONTEND_UI_FILESEL_H

#include "ui/menu.h"

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

	menu_file_selector(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, result &result);
	virtual ~menu_file_selector() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

protected:
	virtual bool custom_mouse_down() override;

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
		file_selector_entry_type type;
		std::string basename;
		std::string fullpath;
	};

	// internal state
	device_image_interface *    m_image;
	std::string &               m_current_directory;
	std::string &               m_current_file;
	bool                        m_has_empty;
	bool                        m_has_softlist;
	bool                        m_has_create;
	result &                    m_result;
	std::vector<file_selector_entry>	m_entrylist;
	std::string                 m_hover_directory;
	std::string					m_filename;

	// methods
	int compare_entries(const file_selector_entry *e1, const file_selector_entry *e2);
	file_selector_entry &append_entry(file_selector_entry_type entry_type, const std::string &entry_basename, const std::string &entry_fullpath);
	file_selector_entry &append_entry(file_selector_entry_type entry_type, std::string &&entry_basename, std::string &&entry_fullpath);
	file_selector_entry *append_dirent_entry(const osd::directory::entry *dirent);
	void append_entry_menu_item(const file_selector_entry *entry);
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
	menu_select_rw(mame_ui_manager &mui, render_container *container,
						bool can_in_place, result &result);
	virtual ~menu_select_rw() override;
	virtual void populate() override;
	virtual void handle() override;

	static void *itemref_from_result(result result);
	static result result_from_itemref(void *itemref);

private:
	// internal state
	bool        m_can_in_place;
	result &    m_result;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_FILESEL_H
