// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.h

***************************************************************************/
#ifndef MAME_FRONTEND_UI_FLOPPYCNTRL_H
#define MAME_FRONTEND_UI_FLOPPYCNTRL_H

#pragma once

#include "ui/imgcntrl.h"

#include "imagedev/floppy.h"

#include <memory>
#include <string>
#include <string_view>


namespace ui {

class menu_control_floppy_image : public menu_control_device_image
{
public:
	menu_control_floppy_image(mame_ui_manager &ui, render_target &target, device_image_interface &image);
	virtual ~menu_control_floppy_image() override;

private:
	floppy_image_device &m_fd;
	const floppy_image_format_t *m_input_format, *m_output_format;
	const floppy_image_device::fs_info *m_create_fs;
	std::string m_input_filename, m_output_filename;

	virtual bool hook_load(const std::string &filename) override;
	virtual bool hook_create(std::string_view path) override;

	void create_format_selected(std::string_view path, floppy_image_format_t const &format);
	void output_format_selected(std::string_view path, floppy_image_format_t const &format);
	void create_write_other(std::string const &name);
	bool do_load_create();

	static bool can_format(const floppy_image_device::fs_info &fs);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_FLOPPYCNTRL_H
