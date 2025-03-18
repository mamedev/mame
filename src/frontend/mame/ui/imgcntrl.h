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

// ======================> menu_control_device_image

class menu_control_device_image : public menu
{
public:
	menu_control_device_image(mame_ui_manager &mui, render_container &container, device_image_interface &image);
	virtual ~menu_control_device_image() override;

protected:
	enum
	{
		START_FILE, START_OTHER_PART, START_SOFTLIST, START_MIDI,
		SELECT_PARTLIST, SELECT_ONE_PART, SELECT_OTHER_PART,
		CREATE_FILE, CREATE_CONFIRM, CHECK_CREATE, DO_CREATE, SELECT_SOFTLIST, SELECT_MIDI,
		LAST_ID
	};

	// this is a single union that contains all of the different types of
	// results we could get from child menus
	union
	{
		menu_software_parts::result swparts;
		int i;
	} m_submenu_result;

	// instance variables - made protected so they can be shared with floppycntrl.cpp
	int                             m_state;
	device_image_interface &        m_image;
	std::string                     m_current_directory;
	std::string                     m_current_file;
	bool                            m_create_ok;

	// methods
	virtual void menu_activated() override;
	virtual bool handle(event const *ev) override;
	virtual void hook_load(const std::string &filename);

private:
	// instance variables
	bool                            m_create_confirmed;
	const software_info *           m_swi;
	const software_part *           m_swp;
	class software_list_device *    m_sld;
	std::string                     m_software_info_name;
	std::string                     m_midi;

	// methods
	virtual void populate() override;
	void test_create(bool &can_create, bool &need_confirm);
	void load_software_part();
};

} // namespace ui

#endif // MAME_FRONTEND_UI_IMAGECNTRL_H
