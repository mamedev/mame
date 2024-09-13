// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.cpp

***************************************************************************/

#include "emu.h"

#include "ui/filesel.h"
#include "ui/filecreate.h"
#include "ui/floppycntrl.h"

#include "formats/flopimg.h"
#include "formats/fsmgr.h"

#include "zippath.h"

#include <tuple>


namespace ui {

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

menu_control_floppy_image::menu_control_floppy_image(mame_ui_manager &mui, render_container &container, device_image_interface &image) :
	menu_control_device_image(mui, container, image),
	fd(dynamic_cast<floppy_image_device &>(image)),
	input_format(nullptr),
	output_format(nullptr),
	create_fs(nullptr),
	input_filename(),
	output_filename()
{
}

menu_control_floppy_image::~menu_control_floppy_image()
{
}

void menu_control_floppy_image::do_load_create()
{
	if(input_filename.empty()) {
		auto [err, message] = fd.create(output_filename, nullptr, nullptr);
		if (err) {
			machine().popmessage(_("Error creating floppy image: %1$s"), !message.empty() ? message : err.message());
			return;
		}
		if (create_fs) {
			// HACK: ensure the floppy_image structure is created since device_image_interface may not otherwise do so during "init phase"
			err = fd.finish_load().first;
			if (!err) {
				fs::meta_data meta;
				fd.init_fs(create_fs, meta);
			}
		}
	} else {
		auto [err, message] = fd.load(input_filename);
		if (!err && !output_filename.empty()) {
			message.clear();
			err = fd.reopen_for_write(output_filename);
		}
		if (err) {
			machine().popmessage(_("Error opening floppy image: %1$s"), !message.empty() ? message : err.message());
			return;
		}
	}
	if(output_format)
		fd.setup_write(output_format);
}

void menu_control_floppy_image::hook_load(const std::string &filename)
{
	std::error_condition err;
	input_filename = filename;
	std::tie(err, input_format) = static_cast<floppy_image_device &>(m_image).identify(filename);

	if (!input_format)
	{
		machine().popmessage("Error: %s", err.message());
		stack_pop();
	}
	else
	{
		bool can_in_place = input_format->supports_save();
		if (can_in_place)
		{
			std::string tmp_path;
			util::core_file::ptr tmp_file;
			// attempt to open the file for writing but *without* create
			std::error_condition const filerr = util::zippath_fopen(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE, tmp_file, tmp_path);
			if(!filerr)
				tmp_file.reset();
			else
				can_in_place = false;
		}
		menu::stack_push<menu_select_rw>(
				ui(),
				container(),
				can_in_place,
				[this] (menu_select_rw::result result)
				{
					switch (result)
					{
					case menu_select_rw::result::READONLY:
						do_load_create();
						fd.setup_write(nullptr);
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
				});
	}
}

bool menu_control_floppy_image::can_format(const floppy_image_device::fs_info &fs)
{
	return !fs.m_manager || fs.m_manager->can_format();
}

void menu_control_floppy_image::menu_activated()
{
	switch (m_state) {
	case DO_CREATE: {
		std::vector<const floppy_image_format_t *> format_array;
		for(const floppy_image_format_t *i : fd.get_formats()) {
			if(!i->supports_save())
				continue;
			if (i->extension_matches(m_current_file.c_str()))
				format_array.push_back(i);
		}
		int ext_match = format_array.size();
		for(const floppy_image_format_t *i : fd.get_formats()) {
			if(!i->supports_save())
				continue;
			if (!i->extension_matches(m_current_file.c_str()))
				format_array.push_back(i);
		}
		output_format = nullptr;
		menu::stack_push<menu_select_format>(ui(), container(), format_array, ext_match, &output_format);

		m_state = SELECT_FORMAT;
		break;
	}

	case SELECT_FORMAT:
		if(!output_format) {
			m_state = START_FILE;
			menu_activated();
		} else {
			// get all formatable file systems
			std::vector<std::reference_wrapper<const floppy_image_device::fs_info>> fs;
			for (const auto &this_fs : fd.get_fs()) {
				if (can_format(this_fs))
					fs.emplace_back(std::ref(this_fs));
			}

			output_filename = util::zippath_combine(m_current_directory, m_current_file);
			if(fs.size() == 1) {
				create_fs = &(fs[0].get());
				do_load_create();
				stack_pop();
			} else {
				m_submenu_result.i = -1;
				menu::stack_push<menu_select_floppy_init>(ui(), container(), std::move(fs), &m_submenu_result.i);
				m_state = SELECT_INIT;
			}
		}
		break;

	case SELECT_INIT:
		// figure out which (if any) create file system was selected
		create_fs = nullptr;
		if(m_submenu_result.i >= 0) {
			int i = 0;
			for (const auto &this_fs : fd.get_fs()) {
				if (can_format(this_fs)) {
					if (i == m_submenu_result.i) {
						create_fs = &this_fs;
						break;
					}
					i++;
				}
			}
		}

		if(!create_fs) {
			m_state = START_FILE;
			menu_activated();
		} else {
			do_load_create();
			stack_pop();
		}
		break;

	default:
		menu_control_device_image::menu_activated();
	}
}

} // namespace ui
