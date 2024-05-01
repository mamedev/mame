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
#include "ui/utils.h"

#include "formats/flopimg.h"

#include "path.h"
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

/***************************************************************************
CONFIRM SAVE AS MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_confirm_save_as::menu_confirm_save_as(mame_ui_manager &mui, render_container &container, bool &yes)
	: menu(mui, container)
	, m_yes(yes)
{
	m_yes = false;
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
	item_append(_("File Already Exists - Override?"), FLAG_DISABLE, nullptr);
	item_append(menu_item_type::SEPARATOR);
	item_append(_("No"), 0, ITEMREF_NO);
	item_append(_("Yes"), 0, ITEMREF_YES);
}

//-------------------------------------------------
//  handle - confirm save as menu
//-------------------------------------------------

bool menu_confirm_save_as::handle(event const *ev)
{
	// process the event
	if (ev && (ev->iptkey == IPT_UI_SELECT))
	{
		if (ev->itemref == ITEMREF_YES)
			m_yes = true;

		// no matter what, pop out
		stack_pop();
	}

	return false;
}



/***************************************************************************
FILE CREATE MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_file_create::menu_file_create(mame_ui_manager &mui, render_container &container, device_image_interface *image, std::string &current_directory, std::string &current_file, bool &ok)
	: menu(mui, container)
	, m_ok(ok)
	, m_current_directory(current_directory)
	, m_current_file(current_file)
	, m_current_format(nullptr)
{
	m_image = image;
	m_ok = false;

	m_filename.reserve(1024);
	m_filename = core_filename_extract_base(current_file);
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_file_create::~menu_file_create()
{
}


//-------------------------------------------------
//  recompute_metrics - recompute metrics
//-------------------------------------------------

void menu_file_create::recompute_metrics(uint32_t width, uint32_t height, float aspect)
{
	menu::recompute_metrics(width, height, aspect);

	set_custom_space(line_height() + 3.0F * tb_border(), 0.0F);
}


//-------------------------------------------------
//  custom_render - perform our special rendering
//-------------------------------------------------

void menu_file_create::custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2)
{
	extra_text_render(top, bottom, origx1, origy1, origx2, origy2,
			m_current_directory,
			std::string_view());
}


//-------------------------------------------------
//  custom_ui_back - override back handling
//-------------------------------------------------

bool menu_file_create::custom_ui_back()
{
	return (get_selection_ref() == ITEMREF_NEW_IMAGE_NAME) && !m_filename.empty();
}


//-------------------------------------------------
//  populate - populates the file creator menu
//-------------------------------------------------

void menu_file_create::populate()
{
	std::string buffer;
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
	image_device_format const *const format = ENABLE_FORMATS ? m_image->formatlist().front().get() : nullptr;
	if (format)
	{
		// FIXME: is this in the right order?  It reassigns m_current_format after reading it.
		item_append(_("Image Format:"), m_current_format->description(), 0, ITEMREF_FORMAT);
		m_current_format = format;
	}

	// finish up the menu
	item_append(menu_item_type::SEPARATOR);
	item_append(_("Create"), 0, ITEMREF_CREATE);
}


//-------------------------------------------------
//  handle - file creator menu
//-------------------------------------------------

bool menu_file_create::handle(event const *ev)
{
	if (!ev)
		return false;

	// handle selections
	switch (ev->iptkey)
	{
	case IPT_UI_SELECT:
		if ((ev->itemref == ITEMREF_CREATE) || (ev->itemref == ITEMREF_NEW_IMAGE_NAME))
		{
			std::string tmp_file(m_filename);
			if (tmp_file.find('.') != -1 && tmp_file.find('.') < tmp_file.length() - 1)
			{
				m_current_file = m_filename;
				m_ok = true;
				stack_pop();
			}
			else
			{
				ui().popup_time(1, "%s", _("Please enter a file extension too"));
			}
		}
		break;

	case IPT_UI_PASTE:
		if (ev->itemref == ITEMREF_NEW_IMAGE_NAME)
		{
			if (paste_text(m_filename, &osd_is_valid_filename_char))
			{
				ev->item->set_subtext(m_filename + "_");
				return true;
			}
		}
		break;

	case IPT_SPECIAL:
		if (ev->itemref == ITEMREF_NEW_IMAGE_NAME)
		{
			if (input_character(m_filename, ev->unichar, &osd_is_valid_filename_char))
			{
				ev->item->set_subtext(m_filename + "_");
				return true;
			}
		}
		break;

	case IPT_UI_CANCEL:
		if ((ev->itemref == ITEMREF_NEW_IMAGE_NAME) && !m_filename.empty())
		{
			m_filename.clear();
			ev->item->set_subtext("_");
			return true;
		}
		break;
	}

	return false;
}


/***************************************************************************
SELECT FORMAT MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_format::menu_select_format(mame_ui_manager &mui, render_container &container, const std::vector<const floppy_image_format_t *> &formats, int ext_match, const floppy_image_format_t **result)
	: menu(mui, container)
{
	m_formats = formats;
	m_ext_match = ext_match;
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
	item_append(_("Select image format"), FLAG_DISABLE, nullptr);
	for (unsigned int i = 0; i != m_formats.size(); i++)
	{
		const floppy_image_format_t *fmt = m_formats[i];

		if (i && i == m_ext_match)
			item_append(menu_item_type::SEPARATOR);
		item_append(fmt->description(), fmt->name(), 0, const_cast<floppy_image_format_t *>(fmt));
	}
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_format::handle(event const *ev)
{
	// process the menu
	if (ev && ev->iptkey == IPT_UI_SELECT)
	{
		*m_result = (floppy_image_format_t *)ev->itemref;
		stack_pop();
	}

	return false;
}


/***************************************************************************
SELECT FORMAT MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_floppy_init::menu_select_floppy_init(mame_ui_manager &mui, render_container &container, std::vector<std::reference_wrapper<const floppy_image_device::fs_info>> &&fs, int *result)
	: menu(mui, container),
	  m_fs(std::move(fs)),
	  m_result(result)

{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_select_floppy_init::~menu_select_floppy_init()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_select_floppy_init::populate()
{
	item_append(_("Select initial contents"), FLAG_DISABLE, nullptr);
	int id = 0;
	for (const floppy_image_device::fs_info &fmt : m_fs)
		item_append(fmt.m_description, fmt.m_name, 0, (void *)(uintptr_t)(id++));
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_floppy_init::handle(event const *ev)
{
	// process the menu
	if (ev && ev->iptkey == IPT_UI_SELECT)
	{
		*m_result = int(uintptr_t(ev->itemref));
		stack_pop();
	}

	return false;
}


} // namespace ui
