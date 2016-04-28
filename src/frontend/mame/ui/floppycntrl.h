// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    ui/floppycntrl.h

***************************************************************************/

#pragma once

#ifndef __UI_FLOPPY_IMAGE_H__
#define __UI_FLOPPY_IMAGE_H__

#include "imagedev/floppy.h"
#include "formats/flopimg.h"

class ui_menu_control_floppy_image : public ui_menu_control_device_image {
public:
	ui_menu_control_floppy_image(mame_ui_manager &ui, render_container *container, device_image_interface *image);
	virtual ~ui_menu_control_floppy_image();

	virtual void handle() override;

protected:
	enum { SELECT_FORMAT = LAST_ID, SELECT_MEDIA, SELECT_RW };

	floppy_image_format_t **format_array;
	floppy_image_format_t *input_format, *output_format;
	std::string input_filename, output_filename;

	void do_load_create();
	virtual void hook_load(std::string filename, bool softlist) override;
};


#endif /* __UI_FLOPPY_IMAGE_H__ */
