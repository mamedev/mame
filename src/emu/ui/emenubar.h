/***************************************************************************

    emenubar.h

    Internal MAME menu bar for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __UI_EMENUBAR_H__
#define __UI_EMENUBAR_H__

#include "ui/menubar.h"
#include "imagedev/cassette.h"
#include "imagedev/bitbngr.h"

class ui_emu_menubar : public ui_menubar
{
public:
	ui_emu_menubar(running_machine &machine);

	virtual void handle(render_container *container);

protected:
	virtual void menubar_build_menus();
	virtual void menubar_draw_ui_elements();

private:
	// variables
	static device_image_interface *s_softlist_image;
	static astring s_softlist_result;

	// menubar building
	void build_file_menu();
	void build_images_menu();
	bool build_software_list_menus(menu_item &menu, device_image_interface *image);
	void build_options_menu();
	void build_video_target_menu(menu_item &target_menu, render_target &target);
	void build_settings_menu();
	void build_help_menu();

	// miscellaneous
	bool is_softlist_relevant(software_list_device *swlist, const char *interface, astring &list_description);
	void set_ui_handler(UINT32 (*callback)(running_machine &, render_container *, UINT32), UINT32 param);
	void select_new_game();
	void select_from_software_list(device_image_interface *image, software_list_device *swlist);
	void tape_control(cassette_image_device *image);
	void load(device_image_interface *image);
	bool has_images();
	void set_throttle_rate(float throttle_rate);
	void start_menu(ui_menu *menu);
	void debugger_break();

	// template methods
	template<class _Menu>
	void start_menu();
};


#endif // __UI_EMENUBAR_H__
