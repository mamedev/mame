/***************************************************************************

    uiimage.h

    Internal MAME user interface image.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UIIMAGE_H__
#define __UIIMAGE_H__

class ui_menu_image_info : public ui_menu {
public:
	ui_menu_image_info(running_machine &machine, render_container *container);
	virtual ~ui_menu_image_info();
	virtual void populate();
	virtual void handle();
};

class ui_menu_file_manager : public ui_menu {
public:
	astring current_directory;
	astring current_file;
	device_image_interface *selected_device;

	ui_menu_file_manager(running_machine &machine, render_container *container);
	virtual ~ui_menu_file_manager();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	void fix_working_directory(device_image_interface *image);
};

class ui_menu_mess_tape_control : public ui_menu {
public:
	ui_menu_mess_tape_control(running_machine &machine, render_container *container);
	virtual ~ui_menu_mess_tape_control();
	virtual void populate();
	virtual void handle();

private:
	int index;
	device_image_interface *device;
	int cassette_count();
};

class ui_menu_mess_bitbanger_control : public ui_menu {
public:
	ui_menu_mess_bitbanger_control(running_machine &machine, render_container *container);
	virtual ~ui_menu_mess_bitbanger_control();
	virtual void populate();
	virtual void handle();

private:
	int index;
	device_image_interface *device;
	int bitbanger_count();
};

class ui_menu_confirm_save_as : public ui_menu {
public:
	ui_menu_confirm_save_as(running_machine &machine, render_container *container, int *yes);
	virtual ~ui_menu_confirm_save_as();
	virtual void populate();
	virtual void handle();

private:
	int *yes;
};

class ui_menu_file_create : public ui_menu {
public:
	ui_menu_file_create(running_machine &machine, render_container *container, ui_menu_file_manager *parent);
	virtual ~ui_menu_file_create();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	ui_menu_file_manager *manager;
	const image_device_format *current_format;
	int confirm_save_as_yes;
	char filename_buffer[1024];

	int create_new_image(device_image_interface *image, const char *directory, const char *filename, int *yes);
};

class ui_menu_file_selector : public ui_menu {
public:
	ui_menu_file_selector(running_machine &machine, render_container *container, ui_menu_file_manager *parent);
	virtual ~ui_menu_file_selector();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
	enum file_selector_entry_type {
		SELECTOR_ENTRY_TYPE_EMPTY,
		SELECTOR_ENTRY_TYPE_CREATE,
		SELECTOR_ENTRY_TYPE_SOFTWARE_LIST,
		SELECTOR_ENTRY_TYPE_DRIVE,
		SELECTOR_ENTRY_TYPE_DIRECTORY,
		SELECTOR_ENTRY_TYPE_FILE
	};

	struct file_selector_entry {
		file_selector_entry *next;

		file_selector_entry_type type;
		const char *basename;
		const char *fullpath;
	};

	ui_menu_file_manager *manager;
	file_selector_entry *entrylist;
	char filename_buffer[1024];

	int compare_entries(const file_selector_entry *e1, const file_selector_entry *e2);
	file_selector_entry *append_entry(file_selector_entry_type entry_type, const char *entry_basename, const char *entry_fullpath);
	file_selector_entry *append_dirent_entry(const osd_directory_entry *dirent);
	void append_entry_menu_item(const file_selector_entry *entry);
};

#endif	/* __UIIMAGE_H__ */
