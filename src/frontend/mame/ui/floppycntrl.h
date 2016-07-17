// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.h

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_FLOPPYCNTRL_H
#define MAME_FRONTEND_UI_FLOPPYCNTRL_H

#include "ui/imgcntrl.h"

#include "imagedev/floppy.h"
#include "formats/flopimg.h"

namespace ui {
class menu_control_floppy_image : public menu_control_device_image
{
public:
	menu_control_floppy_image(mame_ui_manager &ui, render_container &container, device_image_interface *image);
	virtual ~menu_control_floppy_image() override;

private:
	enum { SELECT_FORMAT = LAST_ID, SELECT_MEDIA, SELECT_RW };

	floppy_image_format_t **format_array;
	floppy_image_format_t *input_format, *output_format;
	std::string input_filename, output_filename;

	virtual void handle() override;

	void do_load_create();
	virtual void hook_load(std::string filename, bool softlist) override;
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_FLOPPYCNTRL_H */
