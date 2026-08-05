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

menu_control_floppy_image::menu_control_floppy_image(mame_ui_manager &mui, render_target &target, device_image_interface &image) :
	menu_control_device_image(mui, target, image),
	m_fd(dynamic_cast<floppy_image_device &>(image)),
	m_input_format(nullptr),
	m_output_format(nullptr),
	m_create_fs(nullptr),
	m_input_filename(),
	m_output_filename()
{
}

menu_control_floppy_image::~menu_control_floppy_image()
{
}

void menu_control_floppy_image::create_format_selected(std::string_view path, floppy_image_format_t const &format)
{
	// get all formatable file systems
	std::vector<std::reference_wrapper<floppy_image_device::fs_info const> > filesystems;
	for (auto const &this_fs : m_fd.get_fs())
	{
		if (can_format(this_fs))
			filesystems.emplace_back(std::ref(this_fs));
	}

	if (filesystems.size() == 1)
	{
		stack_pop(); // pop the format selection menu
		m_output_format = &format;
		m_output_filename = path;
		m_create_fs = &filesystems[0].get();
		if (do_load_create())
		{
			stack_pop(); // pop the create file menu
			stack_pop(); // pop the file selection menu
			stack_pop(); // pop the fake menu that orchestrates the other menus
		}
	}
	else
	{
		menu::stack_push<menu_select_floppy_init>(
				ui(),
				target(),
				std::move(filesystems),
				[this, &format, p = std::string(path)] (floppy_image_device::fs_info const &fs)
				{
					stack_pop(); // pop the file system selection menu
					stack_pop(); // pop the format selection menu
					m_output_format = &format;
					m_output_filename = std::move(p);
					m_create_fs = &fs;
					if (do_load_create())
					{
						stack_pop(); // pop the create file menu
						stack_pop(); // pop the file selection menu
						stack_pop(); // pop the fake menu that orchestrates the other menus
					}
				});
	}
}

void menu_control_floppy_image::output_format_selected(std::string_view path, floppy_image_format_t const &format)
{
	stack_pop(); // pop the format selection menu
	stack_pop(); // pop the create file menu
	stack_pop(); // pop the access mode menu
	m_output_filename = path;
	m_output_format = &format;
	m_create_fs = nullptr;
	if (do_load_create())
	{
		stack_pop(); // pop the file selection menu
		stack_pop(); // pop the fake menu that orchestrates the other menus
	}
}

void menu_control_floppy_image::create_write_other(std::string const &name)
{
	auto path = util::zippath_combine(m_current_directory, name);
	bool can_create, need_confirm;
	test_create(path, can_create, need_confirm);
	if (!can_create)
		return;

	if (need_confirm)
	{
		menu::stack_push<menu_confirm_save_as>(
				ui(),
				target(),
				[this, path] ()
				{
					stack_pop(); // pop the confirm overwrite menu
					menu::stack_push<menu_select_format>(
							ui(),
							target(),
							m_fd,
							path,
							[this, path] (floppy_image_format_t const &format) { output_format_selected(path, format); });
				});
	}
	else
	{
		menu::stack_push<menu_select_format>(
				ui(),
				target(),
				m_fd,
				path,
				[this, path] (floppy_image_format_t const &format) { output_format_selected(path, format); });
	}
}

bool menu_control_floppy_image::do_load_create()
{
	if (m_input_filename.empty())
	{
		auto [err, message] = m_fd.create(m_output_filename, nullptr, nullptr);
		if (err)
		{
			machine().popmessage(_("Error creating floppy image: %1$s"), !message.empty() ? message : err.message());
			return false;
		}
		if (m_create_fs)
		{
			// HACK: ensure the floppy_image structure is created since device_image_interface may not otherwise do so during "init phase"
			err = m_fd.finish_load().first;
			if (!err)
			{
				fs::meta_data meta;
				m_fd.init_fs(m_create_fs, meta);
			}
		}
		// TODO: what should we do if the image was created but formatting failed?
	}
	else
	{
		auto [err, message] = m_fd.load(m_input_filename);
		if (!err && !m_output_filename.empty())
		{
			message.clear();
			err = m_fd.reopen_for_write(m_output_filename);
			// TODO: is any cleanup needed if it failed here?
		}
		if (err)
		{
			machine().popmessage(_("Error opening floppy image: %1$s"), !message.empty() ? message : err.message());
			return false;
		}
	}
	if (m_output_format)
		m_fd.setup_write(m_output_format);
	return true;
}

bool menu_control_floppy_image::hook_load(const std::string &filename)
{
	m_input_filename = filename;
	m_output_filename.clear();
	m_output_format = nullptr;
	std::error_condition err;
	std::tie(err, m_input_format) = static_cast<floppy_image_device &>(m_image).identify(filename);
	if (!m_input_format)
	{
		machine().popmessage(_("Error identifying image format: %1$s"), err.message());
		return false;
	}

	bool can_in_place = m_input_format->supports_save();
	if (can_in_place)
	{
		std::string tmp_path;
		util::core_file::ptr tmp_file;
		// attempt to open the file for writing but *without* create
		std::error_condition const filerr = util::zippath_fopen(filename, OPEN_FLAG_READ | OPEN_FLAG_WRITE, tmp_file, tmp_path);
		if (filerr)
			can_in_place = false;
	}

	menu::stack_push<menu_select_rw>(
			ui(),
			target(),
			can_in_place,
			[this] (menu_select_rw::result result)
			{
				switch (result)
				{
				case menu_select_rw::result::READONLY:
					{
						stack_pop(); // pop the access mode menu
						bool const result = do_load_create();
						m_fd.setup_write(nullptr);
						if (result)
						{
							stack_pop(); // pop the file selection menu
							stack_pop(); // pop the fake menu that orchestrates the other menus
						}
					}
					break;

				case menu_select_rw::result::READWRITE:
					stack_pop(); // pop the access mode menu
					m_output_format = m_input_format;
					if (do_load_create())
					{
						stack_pop(); // pop the file selection menu
						stack_pop(); // pop the fake menu that orchestrates the other menus
					}
					break;

				case menu_select_rw::result::WRITE_DIFF:
					machine().popmessage(_("Sorry, diffs are not supported yet."));
					break;

				case menu_select_rw::result::WRITE_OTHER:
					menu::stack_push<menu_file_create>(
							ui(), target(),
							m_image,
							m_current_directory,
							std::string(),
							[this] (std::string const &name) { create_write_other(name); });
					break;
				}
			});
	return false;
}

bool menu_control_floppy_image::hook_create(std::string_view path)
{
	m_input_format = nullptr;
	m_input_filename.clear();
	menu::stack_push<menu_select_format>(
			ui(),
			target(),
			m_fd,
			path,
			[this, p = std::string(path)] (floppy_image_format_t const &format) { create_format_selected(p, format); });

	return false;
}

bool menu_control_floppy_image::can_format(floppy_image_device::fs_info const &fs)
{
	return !fs.m_manager || fs.m_manager->can_format();
}

} // namespace ui
