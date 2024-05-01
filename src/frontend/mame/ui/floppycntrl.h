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


namespace ui {

class menu_control_floppy_image : public menu_control_device_image
{
public:
	menu_control_floppy_image(mame_ui_manager &ui, render_container &container, device_image_interface &image);
	virtual ~menu_control_floppy_image() override;

protected:
	virtual void menu_activated() override;

private:
	enum { SELECT_FORMAT = LAST_ID, SELECT_MEDIA, SELECT_INIT };

	floppy_image_device &fd;
	const floppy_image_format_t *input_format, *output_format;
	const floppy_image_device::fs_info *create_fs;
	std::string input_filename, output_filename;

	void do_load_create();
	virtual void hook_load(const std::string &filename) override;
	static bool can_format(const floppy_image_device::fs_info &fs);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_FLOPPYCNTRL_H
