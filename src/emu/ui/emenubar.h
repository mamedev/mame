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

class ui_emu_menubar : public ui_menubar
{
public:
	ui_emu_menubar(running_machine &machine, render_container *container);

protected:
	virtual void menubar_build_menus();

private:
	// dummy declaration for now, so that development is easier
	char			m_dummy[256];

	// menubar building
	void build_file_menu();
	void build_images_menu();
	void build_options_menu();
	void build_settings_menu();
	void build_help_menu();

	// miscellaneous
	void select_new_game();
	void throttle(float f);
	void set_natural_keyboard(bool use_natural_keyboard);
	void video_options();
};


#endif // __UI_EMENUBAR_H__
