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

#include "zippath.h"

#include <cstring>


namespace ui {

/***************************************************************************
CONSTANTS
***************************************************************************/

// conditional compilation to enable choosing of image formats - this is not
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

menu_confirm_save_as::menu_confirm_save_as(
		mame_ui_manager &mui,
		render_target &target,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
{
	set_needs_prev_menu_item(false);
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
	item_append(_("File Already Exists - Overwrite?"), FLAG_DISABLE, nullptr);
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
			m_handler();
		else
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

menu_file_create::menu_file_create(
		mame_ui_manager &mui,
		render_target &target,
		device_image_interface &image,
		std::string_view current_directory,
		std::string &&starting_name,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_image(image)
	, m_current_directory(current_directory)
	, m_current_format(nullptr)
	, m_filename(std::move(starting_name))
{
	m_filename.reserve(1024);
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
	image_device_format const *const format = ENABLE_FORMATS ? m_image.formatlist().front().get() : nullptr;
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
				m_handler(m_filename);
			else
				ui().popup_time(1, "%s", _("Please enter a file name extension, too"));
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

menu_select_format::menu_select_format(
		mame_ui_manager &mui,
		render_target &target,
		floppy_image_device &fd,
		std::string_view name,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
{
	m_formats.reserve(fd.get_formats().size());
	for (floppy_image_format_t const *i : fd.get_formats())
	{
		if (i->supports_save() && i->extension_matches(name))
			m_formats.emplace_back(i);
	}
	m_ext_match = m_formats.size();
	for (floppy_image_format_t const *i : fd.get_formats())
	{
		if (i->supports_save() && !i->extension_matches(name))
			m_formats.emplace_back(i);
	}

	set_heading(_("Select image format"));
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
	for (unsigned int i = 0; i != m_formats.size(); i++)
	{
		const floppy_image_format_t *fmt = m_formats[i];

		if (i && (i == m_ext_match))
			item_append(menu_item_type::SEPARATOR);
		item_append(fmt->description(), fmt->name(), 0, const_cast<floppy_image_format_t *>(fmt));
	}

	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_format::handle(event const *ev)
{
	// process the menu
	if (ev && (ev->iptkey == IPT_UI_SELECT))
		m_handler(*reinterpret_cast<floppy_image_format_t const *>(ev->itemref));

	return false;
}


/***************************************************************************
SELECT FORMAT MENU
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_select_floppy_init::menu_select_floppy_init(
		mame_ui_manager &mui,
		render_target &target,
		std::vector<std::reference_wrapper<floppy_image_device::fs_info const> > &&fs,
		handler_function &&handler)
	: menu(mui, target)
	, m_handler(std::move(handler))
	, m_fs(std::move(fs))
{
	set_heading(_("Select initial contents"));
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
	for (floppy_image_device::fs_info const &fs : m_fs)
		item_append(fs.m_description, fs.m_name, 0, &const_cast<floppy_image_device::fs_info &>(fs));
	item_append(menu_item_type::SEPARATOR);
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_select_floppy_init::handle(event const *ev)
{
	// process the menu
	if (ev && ev->iptkey == IPT_UI_SELECT)
		m_handler(*reinterpret_cast<floppy_image_device::fs_info const *>(ev->itemref));

	return false;
}


} // namespace ui
