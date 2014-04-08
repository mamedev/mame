/***************************************************************************

    ui/barcode.c

    MESS's "barcode reader" control

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "ui/barcode.h"


// itemrefs for key menu items
#define ITEMREF_NEW_BARCODE    ((void *) 0x0001)
#define ITEMREF_ENTER          ((void *) 0x0002)


/**************************************************

 BARCODE INPUT MENU

 **************************************************/


//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_barcode_code::ui_menu_barcode_code(running_machine &machine, render_container *container, barcode_reader_device *reader)
						: ui_menu(machine, container)
{
	m_reader = reader;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_barcode_code::~ui_menu_barcode_code()
{
}

//-------------------------------------------------
//  populate - populates the barcode input menu
//-------------------------------------------------

void ui_menu_barcode_code::populate()
{
	astring buffer;
	const char *new_barcode;

	// append the "New Barcode" item
	if (get_selection() == ITEMREF_NEW_BARCODE)
	{
		buffer.cat(m_barcode_buffer);
		new_barcode = buffer;
	}
	else
	{
		new_barcode = m_barcode_buffer;
	}

	item_append("New Barcode:", new_barcode, 0, ITEMREF_NEW_BARCODE);

	// finish up the menu
	item_append(MENU_SEPARATOR_ITEM, NULL, 0, NULL);
	item_append("Enter Code", NULL, 0, ITEMREF_ENTER);

	customtop = machine().ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  handle - manages inputs in the barcode input menu
//-------------------------------------------------

void ui_menu_barcode_code::handle()
{
	// process the menu
	const ui_menu_event *event = process(0);

	// process the event
	if (event != NULL)
	{
		// handle selections
		switch (event->iptkey)
		{
			case IPT_UI_SELECT:
				if (event->itemref == ITEMREF_ENTER)
				{
					astring tmp_file(m_barcode_buffer);
					//printf("code %s\n", m_barcode_buffer);
					if (!m_reader->is_valid(tmp_file.len()))
						machine().ui().popup_time(5, "Barcode length invalid!");
					else
					{
						m_reader->write_code(tmp_file.cstr(), tmp_file.len());
						// if sending was successful, reset char buffer
						if (m_barcode_buffer[0] != '\0')
							memset(m_barcode_buffer, '\0', ARRAY_LENGTH(m_barcode_buffer));
						reset(UI_MENU_RESET_REMEMBER_POSITION);
					}
				}
				break;

			case IPT_SPECIAL:
				if (get_selection() == ITEMREF_NEW_BARCODE)
				{
					int buflen = strlen(m_barcode_buffer);

					// if it's a backspace and we can handle it, do so
					if ((event->unichar == 8 || event->unichar == 0x7f) && buflen > 0)
						*(char *)utf8_previous_char(&m_barcode_buffer[buflen]) = 0;
					else if (event->unichar >= '0' && event->unichar <= '9')
					{
						buflen += utf8_from_uchar(&m_barcode_buffer[buflen], ARRAY_LENGTH(m_barcode_buffer) - buflen, event->unichar);
						m_barcode_buffer[buflen] = 0;
					}
					reset(UI_MENU_RESET_REMEMBER_POSITION);
				}
				break;

			case IPT_UI_CANCEL:
				// reset the char buffer also in this case
				if (m_barcode_buffer[0] != '\0')
					memset(m_barcode_buffer, '\0', ARRAY_LENGTH(m_barcode_buffer));
				break;
		}
	}
}



/**************************************************

 READER MENU

**************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

ui_menu_barcode_reader::ui_menu_barcode_reader(running_machine &machine, render_container *container)
						: ui_menu(machine, container)
{
}

//-------------------------------------------------
//  dtor
//-------------------------------------------------

ui_menu_barcode_reader::~ui_menu_barcode_reader()
{
}


//-------------------------------------------------
//  populate - populates the barcode reader menu
//-------------------------------------------------

void ui_menu_barcode_reader::populate()
{
	astring buffer;

	barcode_reader_device_iterator iter(machine().config().root_device());
	for (const barcode_reader_device *bcreader = iter.first(); bcreader != NULL; bcreader = iter.next())
	{
		char label[0x400];
		sprintf(label,"[%s (%s)]",bcreader->name(),bcreader->basetag());
		item_append(label, NULL, 0, (void *)bcreader);
	}
}


//-------------------------------------------------
//  handle - manages inputs in the barcode reader menu
//-------------------------------------------------

void ui_menu_barcode_reader::handle()
{
	// process the menu
	const ui_menu_event *event = process(0);

	// process the event
	if (event != NULL && event->iptkey == IPT_UI_SELECT)
	{
		if (event->itemref != NULL)
		{
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_barcode_code(machine(), container, (barcode_reader_device *)event->itemref)));
		}

	}
}
