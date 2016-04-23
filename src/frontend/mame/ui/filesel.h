// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filesel.h

    MESS's clunky built-in file manager

***************************************************************************/

#pragma once

#ifndef __UI_FILESEL_H__
#define __UI_FILESEL_H__

// ======================> ui_menu_confirm_save_as

class ui_menu_confirm_save_as : public ui_menu
{
public:
	ui_menu_confirm_save_as(running_machine &machine, render_container *container, bool *yes);
	virtual ~ui_menu_confirm_save_as();
	virtual void populate() override;
	virtual void handle() override;

private:
	bool *m_yes;
};


// ======================> ui_menu_file_create

class ui_menu_file_create : public ui_menu
{
public:
	ui_menu_file_create(running_machine &machine, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool *ok);
	virtual ~ui_menu_file_create();
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


// ======================> ui_menu_file_selector

class ui_menu_file_selector : public ui_menu
{
public:
	enum { R_EMPTY, R_SOFTLIST, R_CREATE, R_FILE };
	ui_menu_file_selector(running_machine &machine, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool has_empty, bool has_softlist, bool has_create, int *result);
	virtual ~ui_menu_file_selector();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

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
		file_selector_entry *next;

		file_selector_entry_type type;
		const char *basename;
		const char *fullpath;
	};

	// internal state
	device_image_interface *    m_image;
	std::string &               m_current_directory;
	std::string &               m_current_file;
	bool                        m_has_empty;
	bool                        m_has_softlist;
	bool                        m_has_create;
	int *                       m_result;
	file_selector_entry *       m_entrylist;
	char                        m_filename_buffer[1024];

	// methods
	int compare_entries(const file_selector_entry *e1, const file_selector_entry *e2);
	file_selector_entry *append_entry(file_selector_entry_type entry_type, const char *entry_basename, const char *entry_fullpath);
	file_selector_entry *append_dirent_entry(const osd_directory_entry *dirent);
	void append_entry_menu_item(const file_selector_entry *entry);
};


// ======================> ui_menu_select_format

class ui_menu_select_format : public ui_menu
{
public:
	ui_menu_select_format(running_machine &machine, render_container *container,
							class floppy_image_format_t **formats, int ext_match, int total_usable, int *result);
	virtual ~ui_menu_select_format();
	virtual void populate() override;
	virtual void handle() override;

private:
	// internal state
	floppy_image_format_t **    m_formats;
	int                         m_ext_match;
	int                         m_total_usable;
	int *                       m_result;
};


// ======================> ui_menu_select_rw

class ui_menu_select_rw : public ui_menu
{
public:
	enum { READONLY, READWRITE, WRITE_OTHER, WRITE_DIFF };
	ui_menu_select_rw(running_machine &machine, render_container *container,
						bool can_in_place, int *result);
	virtual ~ui_menu_select_rw();
	virtual void populate() override;
	virtual void handle() override;

private:
	// internal state
	bool        m_can_in_place;
	int *       m_result;
};

// helper
void extra_text_render(render_container *container, float top, float bottom,
	float origx1, float origy1, float origx2, float origy2,
	const char *header, const char *footer);

#endif /* __UI_FILESEL_H__ */
