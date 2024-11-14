// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/filemngr.h

    MESS's clunky built-in file manager

***************************************************************************/
#ifndef MAME_FRONTEND_UI_FILEMNGR_H
#define MAME_FRONTEND_UI_FILEMNGR_H

#pragma once

#include "ui/menu.h"

#include "notifier.h"

#include <string>
#include <vector>


namespace ui {

class menu_file_manager : public menu
{
public:
	static void force_file_manager(mame_ui_manager &mui, render_container &container, const char *warnings);

	menu_file_manager(mame_ui_manager &mui, render_container &container, const char *warnings);
	virtual ~menu_file_manager();

protected:
	virtual void recompute_metrics(uint32_t width, uint32_t height, float aspect) override;
	virtual void custom_render(uint32_t flags, void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;

	void fill_image_line(device_image_interface &img, std::string &instance, std::string &filename);

	std::string const m_warnings;
	std::vector<util::notifier_subscription> m_notifiers;
	device_image_interface *m_selected_device;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_FILEMNGR_H
