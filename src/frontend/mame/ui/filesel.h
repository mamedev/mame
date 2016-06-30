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

class floppy_image_format_t;

namespace ui {
// ======================> menu_confirm_save_as

class menu_confirm_save_as : public menu
{
public:
	menu_confirm_save_as(mame_ui_manager &mui, render_container *container, bool *yes);
	virtual ~menu_confirm_save_as() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	bool *m_yes;
};


// ======================> menu_file_create

class menu_file_create : public menu
{
public:
	menu_file_create(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool *ok);
	virtual ~menu_file_create() override;
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

private:
	device_image_interface *        m_image;
	std::string &                   m_current_directory;
	std::string &                   m_current_file;
	const image_device_format *     m_current_format;
	char                            m_filename_buffer[1024];

protected:
	bool *                          m_ok;
};


// ======================> menu_file_selector

class menu_file_selector : public menu
{
public:
	enum { R_EMPTY, R_SOFTLIST, R_CREATE, R_FILE };
	menu_file_selector(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, int *result);
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
	int *                       m_result;
	std::vector<file_selector_entry>	m_entrylist;
	std::string                 m_hover_directory;
	char                        m_filename_buffer[1024];

	// methods
	int compare_entries(const file_selector_entry *e1, const file_selector_entry *e2);
	file_selector_entry *append_entry(file_selector_entry_type entry_type, const char *entry_basename, const char *entry_fullpath);
	file_selector_entry *append_dirent_entry(const osd::directory::entry *dirent);
	void append_entry_menu_item(const file_selector_entry *entry);
};


// ======================> menu_select_format

class menu_select_format : public menu
{
public:
	menu_select_format(mame_ui_manager &mui, render_container *container,
							floppy_image_format_t **formats, int ext_match, int total_usable, int *result);
	virtual ~menu_select_format() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	// internal state
	floppy_image_format_t **    m_formats;
	int                         m_ext_match;
	int                         m_total_usable;
	int *                       m_result;
};


// ======================> menu_select_rw

class menu_select_rw : public menu
{
public:
	enum { READONLY, READWRITE, WRITE_OTHER, WRITE_DIFF };
	menu_select_rw(mame_ui_manager &mui, render_container *container,
						bool can_in_place, int *result);
	virtual ~menu_select_rw() override;
	virtual void populate() override;
	virtual void handle() override;

private:
	// internal state
	bool        m_can_in_place;
	int *       m_result;
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_FILESEL_H */
