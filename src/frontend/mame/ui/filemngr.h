// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filemngr.h

    MESS's clunky built-in file manager

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_FILEMNGR_H
#define MAME_FRONTEND_UI_FILEMNGR_H

namespace ui {
class menu_file_manager : public menu
{
public:
	std::string current_directory;
	std::string current_file;
	device_image_interface *selected_device;

	static void force_file_manager(mame_ui_manager &mui, render_container *container, const char *warnings);

	menu_file_manager(mame_ui_manager &mui, render_container *container, const char *warnings);
	virtual ~menu_file_manager();
	virtual void populate() override;
	virtual void handle() override;
	virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2) override;

	void fill_image_line(device_image_interface *img, std::string &instance, std::string &filename);

private:
	std::string m_warnings;
	bool m_curr_selected;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_FILEMNGR_H */
