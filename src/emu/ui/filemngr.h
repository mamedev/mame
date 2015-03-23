/***************************************************************************

    ui/filemngr.h

    MESS's clunky built-in file manager

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_FILEMNGR_H__
#define __UI_FILEMNGR_H__

class ui_menu_file_manager : public ui_menu {
public:
	astring current_directory;
	astring current_file;
	device_image_interface *selected_device;

	static void force_file_manager(running_machine &machine, render_container *container, const char *warnings);

	ui_menu_file_manager(running_machine &machine, render_container *container, const char *warnings);
	virtual ~ui_menu_file_manager();
	virtual void populate();
	virtual void handle();
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

	void fill_image_line(device_image_interface *img, astring &instance, astring &filename);

private:
	astring m_warnings;
	bool m_curr_selected;
};

#endif  /* __UI_FILEMNGR_H__ */
