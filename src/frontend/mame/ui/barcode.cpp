// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    ui/barcode.cpp

    "Barcode Reader" control

***************************************************************************/

#include "emu.h"
#include "ui/barcode.h"

#include "ui/ui.h"
#include "ui/utils.h"


namespace ui {

// itemrefs for key menu items
#define ITEMREF_NEW_BARCODE    ((void *) 0x0001)
#define ITEMREF_ENTER_BARCODE  ((void *) 0x0002)
#define ITEMREF_SELECT_READER  ((void *) 0x0003)


/**************************************************

 BARCODE READER MENU

 **************************************************/


//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_barcode_reader::menu_barcode_reader(mame_ui_manager &mui, render_container &container, barcode_reader_device *device)
	: menu_device_control<barcode_reader_device>(mui, container, device)
{
	set_heading(_("Barcode Reader"));
	set_process_flags(PROCESS_LR_REPEAT);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_barcode_reader::~menu_barcode_reader()
{
}

//-------------------------------------------------
//  populate - populates the barcode input menu
//-------------------------------------------------

void menu_barcode_reader::populate()
{
	if (current_device())
	{
		// selected device
		item_append(std::string(current_display_name()), std::string(current_device()->tag() + 1), current_display_flags(), ITEMREF_SELECT_READER);

		// append the "New Barcode" item
		item_append(_("New Barcode:"), m_barcode_buffer, 0, ITEMREF_NEW_BARCODE);

		// finish up the menu
		item_append(_("Enter Code"), 0, ITEMREF_ENTER_BARCODE);
		item_append(menu_item_type::SEPARATOR);
	}
}


//-------------------------------------------------
//  handle - manages inputs in the barcode input menu
//-------------------------------------------------

bool menu_barcode_reader::handle(event const *ev)
{
	if (!ev)
		return false;

	switch (ev->iptkey)
	{
	case IPT_UI_LEFT:
		if (ev->itemref == ITEMREF_SELECT_READER)
			return previous();
		break;

	case IPT_UI_RIGHT:
		if (ev->itemref == ITEMREF_SELECT_READER)
			return next();
		break;

	case IPT_UI_SELECT:
		if (ev->itemref == ITEMREF_ENTER_BARCODE)
		{
			//osd_printf_verbose("code %s\n", m_barcode_buffer);
			if (!current_device()->is_valid(m_barcode_buffer.length()))
			{
				ui().popup_time(5, "%s", _("Barcode length invalid!"));
			}
			else
			{
				current_device()->write_code(m_barcode_buffer.c_str(), m_barcode_buffer.length());
				// if sending was successful, reset char buffer
				m_barcode_buffer.clear();
				reset(reset_options::REMEMBER_POSITION);
			}
		}
		break;

	case IPT_UI_CLEAR:
		if (ev->itemref == ITEMREF_NEW_BARCODE)
		{
			m_barcode_buffer.clear();
			ev->item->set_subtext(m_barcode_buffer);
			return true;
		}
		break;

	case IPT_UI_PASTE:
		if (get_selection_ref() == ITEMREF_NEW_BARCODE)
		{
			if (paste_text(m_barcode_buffer, uchar_is_digit))
			{
				ev->item->set_subtext(m_barcode_buffer);
				return true;
			}
		}
		break;

	case IPT_SPECIAL:
		if (get_selection_ref() == ITEMREF_NEW_BARCODE)
		{
			if (input_character(m_barcode_buffer, ev->unichar, uchar_is_digit))
			{
				ev->item->set_subtext(m_barcode_buffer);
				return true;
			}
		}
		break;
	}

	return false;
}

} // namespace ui
