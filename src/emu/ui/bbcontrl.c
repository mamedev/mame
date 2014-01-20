/***************************************************************************

    ui/bbcontrl.c

    MESS's "bit banger" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/menu.h"
#include "ui/bbcontrl.h"
#include "imagedev/bitbngr.h"


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

ui_menu_mess_bitbanger_control::ui_menu_mess_bitbanger_control(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_mess_bitbanger_control::~ui_menu_mess_bitbanger_control()
{
}


//-------------------------------------------------
//  bitbanger_count - returns the number of bitbanger
//  devices in the machine
//-------------------------------------------------

int ui_menu_mess_bitbanger_control::bitbanger_count()
{
	bitbanger_device_iterator iter(machine().root_device());
	return iter.count();
}


//-------------------------------------------------
//  populate - populates the main bitbanger control menu
//-------------------------------------------------

void ui_menu_mess_bitbanger_control::populate()
{
	int count = bitbanger_count();
	UINT32 flags = 0, mode_flags = 0, baud_flags = 0, tune_flags = 0;

	if( count > 0 )
	{
		if( index == (count-1) )
			flags |= MENU_FLAG_LEFT_ARROW;
		else
			flags |= MENU_FLAG_RIGHT_ARROW;
	}

	if ((device != NULL) && (device->exists()))
	{
		bitbanger_device *bitbanger = downcast<bitbanger_device *>(&device->device());

		if (bitbanger->inc_mode(TRUE))
			mode_flags |= MENU_FLAG_RIGHT_ARROW;

		if (bitbanger->dec_mode(TRUE))
			mode_flags |= MENU_FLAG_LEFT_ARROW;

		if (bitbanger->inc_baud(TRUE))
			baud_flags |= MENU_FLAG_RIGHT_ARROW;

		if (bitbanger->dec_baud(TRUE))
			baud_flags |= MENU_FLAG_LEFT_ARROW;

		if (bitbanger->inc_tune(TRUE))
			tune_flags |= MENU_FLAG_RIGHT_ARROW;

		if (bitbanger->dec_tune(TRUE))
			tune_flags |= MENU_FLAG_LEFT_ARROW;

		// name of bitbanger file
		item_append(device->device().name(), device->filename(), flags, BITBANGERCMD_SELECT);
		item_append("Device Mode:", bitbanger->mode_string(), mode_flags, BITBANGERCMD_MODE);
		item_append("Baud:", bitbanger->baud_string(), baud_flags, BITBANGERCMD_BAUD);
		item_append("Baud Tune:", bitbanger->tune_string(), tune_flags, BITBANGERCMD_TUNE);
		item_append("Protocol:", "8-1-N", 0, NULL);
	}
	else
	{
		// no tape loaded
		item_append("No Bitbanger Image loaded", NULL, flags, NULL);
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_mess_bitbanger_control::handle()
{
	// do we have to load the device?
	if (device == NULL)
	{
		bitbanger_device_iterator iter(machine().root_device());
		device = iter.byindex(index);
		reset((ui_menu_reset_options)0);
	}

	// get the bitbanger
	bitbanger_device *bitbanger = downcast<bitbanger_device *>(device);

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
				{
					// left arrow - rotate left through cassette devices
					if (index > 0)
						index--;
					else
						index = bitbanger_count() - 1;
					device = NULL;
				}
				else if (event->itemref==BITBANGERCMD_MODE)
				{
					bitbanger->dec_mode(FALSE);
				}
				else if (event->itemref==BITBANGERCMD_BAUD)
				{
					bitbanger->dec_baud(FALSE);
				}
				else if (event->itemref==BITBANGERCMD_TUNE)
				{
					bitbanger->dec_tune(FALSE);
				}
				break;

			case IPT_UI_RIGHT:
				if (event->itemref==BITBANGERCMD_SELECT)
				{
					// right arrow - rotate right through cassette devices
					if (index < bitbanger_count() - 1)
						index++;
					else
						index = 0;
					device = NULL;
				}
				else if (event->itemref==BITBANGERCMD_MODE)
				{
					bitbanger->inc_mode(FALSE);
				}
				else if (event->itemref==BITBANGERCMD_BAUD)
				{
					bitbanger->inc_baud(FALSE);
				}
				else if (event->itemref==BITBANGERCMD_TUNE)
				{
					bitbanger->inc_tune(FALSE);
				}
				break;
		}
	}
}

