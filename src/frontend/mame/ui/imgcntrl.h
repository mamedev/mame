// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/imgcntrl.h

    MAME's clunky built-in file manager

***************************************************************************/

#ifndef MAME_FRONTEND_UI_IMAGECNTRL_H
#define MAME_FRONTEND_UI_IMAGECNTRL_H

#pragma once

#include "ui/menu.h"
#include "ui/swlist.h"


namespace ui {

class menu_control_device_image : public menu
{
public:
	menu_control_device_image(mame_ui_manager &mui, render_target &target, device_image_interface &image);
	virtual ~menu_control_device_image() override;

protected:
	// instance variables - made protected so they can be shared with floppycntrl.cpp
	device_image_interface &        m_image;
	std::string                     m_current_directory;
	std::string                     m_current_file;

	// methods
	virtual void menu_activated() override;
	virtual bool handle(event const *ev) override;

	virtual void create_file(std::string const &directory, std::string const &name);
	void test_create(std::string const &path, bool &can_create, bool &need_confirm);

private:
	enum
	{
		START_FILE, START_OTHER_PART, DONE
	};

	// instance variables
	int                             m_state;
	const software_info *           m_swi;
	const software_part *           m_swp;
	class software_list_device *    m_sld;

	// methods
	virtual void populate() override;

	virtual bool hook_load(std::string const &filename);
	virtual bool hook_create(std::string_view path);

	void start_file();
	void start_softlist();
	void start_midi();
	bool load_software_part(software_list_device &sld, software_info const &swi, software_part const &swp);
	void software_item_selected(software_list_device &sld, std::string_view item);
	void other_part_selected(menu_software_parts::result action, software_part const *swp);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_IMAGECNTRL_H
