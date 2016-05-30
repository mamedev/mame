// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.cpp

***************************************************************************/

#include "emu.h"

#include "ui/filesel.h"
#include "ui/floppycntrl.h"

#include "zippath.h"


namespace ui {

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

menu_control_floppy_image::menu_control_floppy_image(mame_ui_manager &mui, render_container *container, device_image_interface *_image) : menu_control_device_image(mui, container, _image)
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
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
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
	if(input_filename.compare("")==0) {
		int err = fd->create(output_filename.c_str(), nullptr, nullptr);
		if (err != 0) {
			machine().popmessage("Error: %s", fd->error());
			return;
		}
		fd->setup_write(output_format);
	} else {
		int err = fd->load(input_filename.c_str());
		if (!err && output_filename.compare("") != 0)
			err = fd->reopen_for_write(output_filename.c_str());
		if(err != 0) {
			machine().popmessage("Error: %s", fd->error());
			return;
		}
		if(output_format)
			fd->setup_write(output_format);
	}
}

void menu_control_floppy_image::hook_load(std::string filename, bool softlist)
{
	if (softlist)
	{
		machine().popmessage("When loaded from software list, the disk is Read-only.\n");
		image->load(filename.c_str());
		menu::stack_pop(machine());
		return;
	}

	input_filename = filename;
	input_format = static_cast<floppy_image_device *>(image)->identify(filename);

	if (!input_format)
	{
		machine().popmessage("Error: %s\n", image->error());
		menu::stack_pop(machine());
		return;
	}

	bool can_in_place = input_format->supports_save();
	if(can_in_place) {
		osd_file::error filerr;
		std::string tmp_path;
		util::core_file::ptr tmp_file;
		// attempt to open the file for writing but *without* create
		filerr = util::zippath_fopen(filename.c_str(), OPEN_FLAG_READ | OPEN_FLAG_WRITE, tmp_file, tmp_path);
		if(filerr == osd_file::error::NONE)
			tmp_file.reset();
		else
			can_in_place = false;
	}
	submenu_result = -1;
	menu::stack_push<menu_select_rw>(ui(), container, can_in_place, &submenu_result);
	state = SELECT_RW;
}

void menu_control_floppy_image::handle()
{
	floppy_image_device *fd = static_cast<floppy_image_device *>(image);
	switch (state) {
	case DO_CREATE: {
		floppy_image_format_t *fif_list = fd->get_formats();
			int ext_match;
			int total_usable = 0;
			for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if (i->extension_matches(current_file.c_str()))
				format_array[total_usable++] = i;
		}
		ext_match = total_usable;
		for(floppy_image_format_t *i = fif_list; i; i = i->next) {
			if(!i->supports_save())
				continue;
			if (!i->extension_matches(current_file.c_str()))
				format_array[total_usable++] = i;
		}
		submenu_result = -1;
		menu::stack_push<menu_select_format>(ui(), container, format_array, ext_match, total_usable, &submenu_result);

		state = SELECT_FORMAT;
		break;
	}

	case SELECT_FORMAT:
		if(submenu_result == -1) {
			state = START_FILE;
			handle();
		} else {
			util::zippath_combine(output_filename, current_directory.c_str(), current_file.c_str());
			output_format = format_array[submenu_result];
			do_load_create();
			menu::stack_pop(machine());
		}
		break;

	case SELECT_RW:
		switch(submenu_result) {
		case menu_select_rw::READONLY:
			do_load_create();
			menu::stack_pop(machine());
			break;

		case menu_select_rw::READWRITE:
			output_format = input_format;
			do_load_create();
			menu::stack_pop(machine());
			break;

		case menu_select_rw::WRITE_DIFF:
			machine().popmessage("Sorry, diffs are not supported yet\n");
			menu::stack_pop(machine());
			break;

		case menu_select_rw::WRITE_OTHER:
			menu::stack_push<menu_file_create>(ui(), container, image, current_directory, current_file, &create_ok);
			state = CHECK_CREATE;
			break;

		case -1:
			state = START_FILE;
			break;
		}
		break;

	default:
		menu_control_device_image::handle();
	}
}

} // namespace ui
