/***************************************************************************

    ui/tapectrl.c

    MESS's tape control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/tapectrl.h"

#define TAPECMD_NULL            ((void *) 0x0000)
#define TAPECMD_STOP            ((void *) 0x0001)
#define TAPECMD_PLAY            ((void *) 0x0002)
#define TAPECMD_RECORD          ((void *) 0x0003)
#define TAPECMD_REWIND          ((void *) 0x0004)
#define TAPECMD_FAST_FORWARD        ((void *) 0x0005)
#define TAPECMD_SLIDER          ((void *) 0x0006)
#define TAPECMD_SELECT          ((void *) 0x0007)

ui_menu_mess_tape_control::ui_menu_mess_tape_control(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

ui_menu_mess_tape_control::~ui_menu_mess_tape_control()
{
}

/*-------------------------------------------------
    cassette_count - returns the number of cassette
    devices in the machine
-------------------------------------------------*/

int ui_menu_mess_tape_control::cassette_count()
{
	cassette_device_iterator iter(machine().root_device());
	return iter.count();
}



/*-------------------------------------------------
    menu_tape_control_populate - populates the
    main tape control menu
-------------------------------------------------*/

void ui_menu_mess_tape_control::populate()
{
	astring timepos;
	cassette_state state;
	int count = cassette_count();
	UINT32 flags = 0;

	if( count > 0 )
	{
		if( index == (count-1) )
			flags |= MENU_FLAG_LEFT_ARROW;
		else
			flags |= MENU_FLAG_RIGHT_ARROW;
	}

	if ((device != NULL) && (device->exists()))
	{
		double t0, t1;
		UINT32 tapeflags = 0;
		cassette_image_device* cassette = dynamic_cast<cassette_image_device*>(&device->device());

		t0 = cassette->get_position();
		t1 = cassette->get_length();

		if (t1 > 0)
		{
			if (t0 > 0)
				tapeflags |= MENU_FLAG_LEFT_ARROW;
			if (t0 < t1)
				tapeflags |= MENU_FLAG_RIGHT_ARROW;
		}

		/* name of tape */
		item_append(device->device().name(), device->filename(), flags, TAPECMD_SELECT);

		/* state */
		tapecontrol_gettime(timepos, cassette, NULL, NULL);
		state = cassette->get_state();
		item_append(
			(state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED
				?   "stopped"
				:   ((state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY
					? ((state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED ? "playing" : "(playing)")
					: ((state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED ? "recording" : "(recording)")
					),
			timepos,
			tapeflags,
			TAPECMD_SLIDER);

		/* pause or stop */
		item_append("Pause/Stop", NULL, 0, TAPECMD_STOP);

		/* play */
		item_append("Play", NULL, 0, TAPECMD_PLAY);

		/* record */
		item_append("Record", NULL, 0, TAPECMD_RECORD);

		/* rewind */
		item_append("Rewind", NULL, 0, TAPECMD_REWIND);

		/* fast forward */
		item_append("Fast Forward", NULL, 0, TAPECMD_FAST_FORWARD);
	}
	else
	{
		/* no tape loaded */
		item_append("No Tape Image loaded", NULL, flags, NULL);
	}
}


/*-------------------------------------------------
    menu_tape_control - main tape control menu
-------------------------------------------------*/

void ui_menu_mess_tape_control::handle()
{
	/* do we have to load the device? */
	if (device == NULL)
	{
		cassette_device_iterator iter(machine().root_device());
		device = iter.byindex(index);
		reset((ui_menu_reset_options)0);
	}

	/* rebuild the menu - we have to do this so that the counter updates */
	reset(UI_MENU_RESET_REMEMBER_POSITION);
	populate();

	cassette_image_device* cassette = dynamic_cast<cassette_image_device*>(&device->device());

	/* process the menu */
	const ui_menu_event *event = process(UI_MENU_PROCESS_LR_REPEAT);
	if (event != NULL)
	{
		switch(event->iptkey)
		{
			case IPT_UI_LEFT:
				if (event->itemref==TAPECMD_SLIDER)
					cassette->seek(-1, SEEK_CUR);
				else
				if (event->itemref==TAPECMD_SELECT)
				{
					/* left arrow - rotate left through cassette devices */
					if (index > 0)
						index--;
					else
						index = cassette_count() - 1;
					device = NULL;
				}
				break;

			case IPT_UI_RIGHT:
				if (event->itemref==TAPECMD_SLIDER)
					cassette->seek(+1, SEEK_CUR);
				else
				if (event->itemref==TAPECMD_SELECT)
				{
					/* right arrow - rotate right through cassette devices */
					if (index < cassette_count() - 1)
						index++;
					else
						index = 0;
					device = NULL;
				}
				break;

			case IPT_UI_SELECT:
				{
					if (event->itemref==TAPECMD_STOP)
						cassette->change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
					else
					if (event->itemref==TAPECMD_PLAY)
						cassette->change_state(CASSETTE_PLAY, CASSETTE_MASK_UISTATE);
					else
					if (event->itemref==TAPECMD_RECORD)
						cassette->change_state(CASSETTE_RECORD, CASSETTE_MASK_UISTATE);
					else
					if (event->itemref==TAPECMD_REWIND)
						cassette->seek(-30, SEEK_CUR);
					else
					if (event->itemref==TAPECMD_FAST_FORWARD)
						cassette->seek(30, SEEK_CUR);
					else
					if (event->itemref==TAPECMD_SLIDER)
						cassette->seek(0, SEEK_SET);
				}
				break;
		}
	}
}


/*-------------------------------------------------
    tapecontrol_gettime - returns a textual
    representation of the time
-------------------------------------------------*/

astring &ui_menu_mess_tape_control::tapecontrol_gettime(astring &dest, cassette_image_device *cassette, int *curpos, int *endpos)
{
	double t0, t1;

	t0 = cassette->get_position();
	t1 = cassette->get_length();

	if (t1)
		dest.printf("%04d/%04d", (int) t0, (int) t1);
	else
		dest.printf("%04d/%04d", 0, (int) t1);

	if (curpos != NULL)
		*curpos = t0;
	if (endpos != NULL)
		*endpos = t1;

	return dest;
}


