/***************************************************************************

    ui/bbcontrl.c

    MESS's "bit banger" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/bbcontrl.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define BITBANGERCMD_SELECT         ((void *) 0x0000)
#define BITBANGERCMD_MODE           ((void *) 0x0001)
#define BITBANGERCMD_BAUD           ((void *) 0x0002)
#define BITBANGERCMD_TUNE           ((void *) 0x0003)


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_mess_bitbanger_control::ui_menu_mess_bitbanger_control(running_machine &machine, render_container *container, bitbanger_device *device)
	: ui_menu_device_control<bitbanger_device>(machine, container, device)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_mess_bitbanger_control::~ui_menu_mess_bitbanger_control()
{
}


//-------------------------------------------------
//  populate - populates the main bitbanger control menu
//-------------------------------------------------

void ui_menu_mess_bitbanger_control::populate()
{
	UINT32 flags = 0, mode_flags = 0, baud_flags = 0, tune_flags = 0;

	if( count() > 0 )
	{
		int index = current_index();

		if( index == (count()-1) )
			flags |= MENU_FLAG_LEFT_ARROW;
		else
			flags |= MENU_FLAG_RIGHT_ARROW;
	}

	if ((current_device() != NULL) && (current_device()->exists()))
	{
		if (current_device()->inc_mode(TRUE))
			mode_flags |= MENU_FLAG_RIGHT_ARROW;

		if (current_device()->dec_mode(TRUE))
			mode_flags |= MENU_FLAG_LEFT_ARROW;

		if (current_device()->inc_baud(TRUE))
			baud_flags |= MENU_FLAG_RIGHT_ARROW;

		if (current_device()->dec_baud(TRUE))
			baud_flags |= MENU_FLAG_LEFT_ARROW;

		if (current_device()->inc_tune(TRUE))
			tune_flags |= MENU_FLAG_RIGHT_ARROW;

		if (current_device()->dec_tune(TRUE))
			tune_flags |= MENU_FLAG_LEFT_ARROW;

		// name of bitbanger file
		item_append(current_device()->device().name(), current_device()->filename(), flags, BITBANGERCMD_SELECT);
		item_append("Device Mode:", current_device()->mode_string(), mode_flags, BITBANGERCMD_MODE);
		item_append("Baud:", current_device()->baud_string(), baud_flags, BITBANGERCMD_BAUD);
		item_append("Baud Tune:", current_device()->tune_string(), tune_flags, BITBANGERCMD_TUNE);
		item_append("Protocol:", "8-1-N", 0, NULL);
	}
	else
	{
		// no bitbanger loaded
		item_append("No Bitbanger Image loaded", NULL, flags, NULL);
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_mess_bitbanger_control::handle()
{
	// rebuild the menu
	reset(UI_MENU_RESET_REMEMBER_POSITION);
	populate();

	// process the menu
	const ui_menu_event *event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (event != NULL)
	{
		switch(event->iptkey)
		{
			case IPT_UI_LEFT:
				if (event->itemref==BITBANGERCMD_SELECT)
					previous();
				else if (event->itemref==BITBANGERCMD_MODE)
					current_device()->dec_mode(false);
				else if (event->itemref==BITBANGERCMD_BAUD)
					current_device()->dec_baud(false);
				else if (event->itemref==BITBANGERCMD_TUNE)
					current_device()->dec_tune(false);
				break;

			case IPT_UI_RIGHT:
				if (event->itemref==BITBANGERCMD_SELECT)
					next();
				else if (event->itemref==BITBANGERCMD_MODE)
					current_device()->inc_mode(false);
				else if (event->itemref==BITBANGERCMD_BAUD)
					current_device()->inc_baud(false);
				else if (event->itemref==BITBANGERCMD_TUNE)
					current_device()->inc_tune(false);
				break;
		}
	}
}
