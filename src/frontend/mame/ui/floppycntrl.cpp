// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.cpp

***************************************************************************/

#include "emu.h"

#include "ui/filesel.h"
#include "ui/filecreate.h"
#include "ui/floppycntrl.h"

#include "zippath.h"


namespace ui {
/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

menu_control_floppy_image::menu_control_floppy_image(mame_ui_manager &mui, render_container &container, device_image_interface &image) : menu_control_device_image(mui, container, image)
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(&m_image);
	const floppy_image_format_t *fif_list = fd->get_formats();
	int fcnt = 0;
	for(const floppy_image_format_t *i = fif_list; i; i = i->next)
		fcnt++;

	format_array = global_alloc_array(floppy_image_format_t *, fcnt);
	input_format = output_format = nullptr;
	input_filename = output_filename = "";
}

menu_control_floppy_image::~menu_control_floppy_image()
{
	global_free_array(format_array);
}

void menu_control_floppy_image::do_load_create()
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(&m_image);
	if(input_filename.compare("")==0) {
		image_init_result err = fd->create(output_filename.c_str(), nullptr, nullptr);
		if (err != image_init_result::PASS) {
			machine().popmessage("Error: %s", fd->error());
			return;
		}
		fd->setup_write(output_format);
	} else {
		image_init_result err = fd->load(input_filename);
		if ((err == image_init_result::PASS) && (output_filename.compare("") != 0))
			err = fd->reopen_for_write(output_filename.c_str()) ? image_init_result::FAIL : image_init_result::PASS;
		if (err != image_init_result::PASS) {
			machine().popmessage("Error: %s", fd->error());
			return;
		}
		if(output_format)
			fd->setup_write(output_format);
	}
}

void menu_control_floppy_image::hook_load(const std::string &filename, bool softlist)
{
	if (softlist)
	{
		machine().popmessage("When loaded from software list, the disk is Read-only.\n");
		m_image.load_software(filename);
		stack_pop();
		return;
	}

	input_filename = filename;
	input_format = static_cast<floppy_image_device &>(m_image).identify(filename);

	if (!input_format)
	{
		machine().popmessage("Error: %s\n", m_image.error());
		stack_pop();
		return;
	}

	bool can_in_place = input_format->supports_save();
	if(can_in_place) {
		osd_file::error filerr;
		std::string tmp_path;
		util::core_file::ptr tmp_file;
		// attempt to open the file for writing but *without* create
		filerr = util::zippath_fopen(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE, tmp_file, tmp_path);
		if(filerr == osd_file::error::NONE)
			tmp_file.reset();
		else
			can_in_place = false;
	}
	m_submenu_result.rw = menu_select_rw::result::INVALID;
	menu::stack_push<menu_select_rw>(ui(), container(), can_in_place, m_submenu_result.rw);
	m_state = SELECT_RW;
}

void menu_control_floppy_image::handle()
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(&m_image);
	switch (m_state) {
	case DO_CREATE: {
		floppy_image_format_t *fif_list = fd->get_formats();
			int ext_match;
			int total_usable = 0;
			for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if (i->extension_matches(m_current_file.c_str()))
				format_array[total_usable++] = i;
		}
		ext_match = total_usable;
		for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if (!i->extension_matches(m_current_file.c_str()))
				format_array[total_usable++] = i;
		}
		m_submenu_result.i = -1;
		menu::stack_push<menu_select_format>(ui(), container(), format_array, ext_match, total_usable, &m_submenu_result.i);

		m_state = SELECT_FORMAT;
		break;
	}

	case SELECT_FORMAT:
		if(m_submenu_result.i == -1) {
			m_state = START_FILE;
			handle();
		} else {
			output_filename = util::zippath_combine(m_current_directory, m_current_file);
			output_format = format_array[m_submenu_result.i];
			do_load_create();
			stack_pop();
		}
		break;

	case SELECT_RW:
		switch(m_submenu_result.rw) {
		case menu_select_rw::result::READONLY:
			do_load_create();
			stack_pop();
			break;

		case menu_select_rw::result::READWRITE:
			output_format = input_format;
			do_load_create();
			stack_pop();
			break;

		case menu_select_rw::result::WRITE_DIFF:
			machine().popmessage("Sorry, diffs are not supported yet\n");
			stack_pop();
			break;

		case menu_select_rw::result::WRITE_OTHER:
			menu::stack_push<menu_file_create>(ui(), container(), &m_image, m_current_directory, m_current_file, m_create_ok);
			m_state = CHECK_CREATE;
			break;

		case menu_select_rw::result::INVALID:
			m_state = START_FILE;
			break;
		}
		break;

	default:
		menu_control_device_image::handle();
	}
}

} // namespace ui
