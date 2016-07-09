// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

	ui/filecreate.cpp

	MAME's clunky built-in file manager

	TODO
		- Support image creation arguments

***************************************************************************/

#include "emu.h"

#include "ui/filecreate.h"
#include "ui/ui.h"

#include "imagedev/floppy.h"

#include "zippath.h"

#include <cstring>


namespace ui {
/***************************************************************************
CONSTANTS
***************************************************************************/

// conditional compilation to enable chosing of image formats - this is not
// yet fully implemented
#define ENABLE_FORMATS          0

// itemrefs for key menu items
#define ITEMREF_NEW_IMAGE_NAME  ((void *) 0x0001)
#define ITEMREF_CREATE          ((void *) 0x0002)
#define ITEMREF_FORMAT          ((void *) 0x0003)
#define ITEMREF_NO              ((void *) 0x0004)
#define ITEMREF_YES             ((void *) 0x0005)


/***************************************************************************
MENU HELPERS
***************************************************************************/

//-------------------------------------------------
//  input_character - inputs a typed character
//  into a buffer
//-------------------------------------------------

template <typename F>
static void input_character(std::string &buffer, unicode_char unichar, F &&filter)
{
	auto buflen = buffer.size();

	if ((unichar == 8) || (unichar == 0x7f))
	{
		// backspace
		if (0 < buflen)
		{
			auto buffer_oldend = buffer.c_str() + buflen;
			auto buffer_newend = utf8_previous_char(buffer_oldend);
			buffer.resize(buffer_newend - buffer.c_str());
		}
	}
	else if ((unichar >= ' ') && (!filter || filter(unichar)))
	{
		// append this character
		buffer += utf8_from_uchar(unichar);
	}
}


/***************************************************************************
CONFIRM SAVE AS MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_confirm_save_as::menu_confirm_save_as(mame_ui_manager &mui, render_container *container, bool *yes)
	: menu(mui, container)
{
	m_yes = yes;
	*m_yes = false;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_confirm_save_as::~menu_confirm_save_as()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_confirm_save_as::populate()
{
	item_append(_("File Already Exists - Override?"), "", FLAG_DISABLE, nullptr);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("No"), "", 0, ITEMREF_NO);
	item_append(_("Yes"), "", 0, ITEMREF_YES);
}

//-------------------------------------------------
//  handle - confirm save as menu
//-------------------------------------------------

void menu_confirm_save_as::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if ((event != nullptr) && (event->iptkey == IPT_UI_SELECT))
	{
		if (event->itemref == ITEMREF_YES)
			*m_yes = true;

		// no matter what, pop out
		stack_pop();
	}
}



/***************************************************************************
FILE CREATE MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_create::menu_file_create(mame_ui_manager &mui, render_container *container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool &ok)
	: menu(mui, container)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_current_format(nullptr)
	, m_ok(ok)
{
	m_image = image;
	m_ok = true;
	auto const sep = current_file.rfind(PATH_SEPARATOR);

	m_filename.reserve(1024);
	m_filename = sep != std::string::npos
		? current_file.substr(sep + strlen(PATH_SEPARATOR), current_file.size() - sep - strlen(PATH_SEPARATOR))
		: current_file;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_create::~menu_file_create()
{
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_create::custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2,
		m_current_directory.c_str(),
		nullptr);
}


//-------------------------------------------------
//  populate - populates the file creator menu
//-------------------------------------------------

void menu_file_create::populate()
{
	std::string buffer;
	const image_device_format *format;
	const std::string *new_image_name;

	// append the "New Image Name" item
	if (get_selection_ref() == ITEMREF_NEW_IMAGE_NAME)
	{
		buffer = m_filename + "_";
		new_image_name = &buffer;
	}
	else
	{
		new_image_name = &m_filename;
	}
	item_append(_("New Image Name:"), *new_image_name, 0, ITEMREF_NEW_IMAGE_NAME);

	// do we support multiple formats?
	if (ENABLE_FORMATS) format = m_image->formatlist().front().get();
	if (ENABLE_FORMATS && (format != nullptr))
	{
		item_append(_("Image Format:"), m_current_format->description(), 0, ITEMREF_FORMAT);
		m_current_format = format;
	}

	// finish up the menu
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Create"), "", 0, ITEMREF_CREATE);

	customtop = ui().get_line_height() + 3.0f * UI_BOX_TB_BORDER;
}


//-------------------------------------------------
//  handle - file creator menu
//-------------------------------------------------

void menu_file_create::handle()
{
	// process the menu
	const event *event = process(0);

	// process the event
	if (event)
	{
		// handle selections
		switch (event->iptkey)
		{
		case IPT_UI_SELECT:
			if ((event->itemref == ITEMREF_CREATE) || (event->itemref == ITEMREF_NEW_IMAGE_NAME))
			{
				std::string tmp_file(m_filename);
				if (tmp_file.find(".") != -1 && tmp_file.find(".") < tmp_file.length() - 1)
				{
					m_current_file = m_filename;
					stack_pop();
				}
				else
					ui().popup_time(1, "%s", _("Please enter a file extension too"));
			}
			break;

		case IPT_SPECIAL:
			if (get_selection_ref() == ITEMREF_NEW_IMAGE_NAME)
			{
				input_character(m_filename, event->unichar, &osd_is_valid_filename_char);
				reset(reset_options::REMEMBER_POSITION);
			}
			break;
		case IPT_UI_CANCEL:
			m_ok = false;
			break;
		}
	}
}


/***************************************************************************
SELECT FORMAT MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_format::menu_select_format(mame_ui_manager &mui, render_container *container, floppy_image_format_t **formats, int ext_match, int total_usable, int *result)
	: menu(mui, container)
{
	m_formats = formats;
	m_ext_match = ext_match;
	m_total_usable = total_usable;
	m_result = result;
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_format::~menu_select_format()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_format::populate()
{
	item_append(_("Select image format"), "", FLAG_DISABLE, nullptr);
	for (int i = 0; i < m_total_usable; i++)
	{
		const floppy_image_format_t *fmt = m_formats[i];

		if (i && i == m_ext_match)
			item_append(menu_item_type::SEPARATOR);
		item_append(fmt->description(), fmt->name(), 0, (void *)(FPTR)i);
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

void menu_select_format::handle()
{
	// process the menu
	const event *event = process(0);
	if (event != nullptr && event->iptkey == IPT_UI_SELECT)
	{
		*m_result = int(FPTR(event->itemref));
		stack_pop();
	}
}


} // namespace ui
