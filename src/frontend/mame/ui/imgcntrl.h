// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/imgcntrl.h

    MESS's clunky built-in file manager

***************************************************************************/

#pragma once

#ifndef MAME_FRONTEND_UI_IMAGECNTRL_H
#define MAME_FRONTEND_UI_IMAGECNTRL_H

#include "ui/menu.h"
#include "ui/filesel.h"
#include "ui/swlist.h"

namespace ui {
// ======================> menu_control_device_image

class menu_control_device_image : public menu
{
public:
	menu_control_device_image(mame_ui_manager &mui, render_container &container, device_image_interface *image);
	virtual ~menu_control_device_image() override;

protected:
	enum {
		START_FILE, START_OTHER_PART, START_SOFTLIST,
		SELECT_PARTLIST, SELECT_ONE_PART, SELECT_OTHER_PART,
		SELECT_FILE, CREATE_FILE, CREATE_CONFIRM, CHECK_CREATE, DO_CREATE, SELECT_SOFTLIST,
		LAST_ID
	};

	// protected instance variables
	int state;
	device_image_interface *image;

	// this is a single union that contains all of the different types of
	// results we could get from child menus
	union
	{
		menu_file_selector::result filesel;
		menu_software_parts::result swparts;
		menu_select_rw::result rw;
		int i;
	} submenu_result;

	std::string m_current_directory;
	std::string m_current_file;

	// methods
	virtual void hook_load(std::string filename, bool softlist);
	virtual void handle() override;

	bool create_ok;

private:
	virtual void populate() override;

	// instance variables
	bool create_confirmed;
	//bool softlist_done;
	const software_info *swi;
	const software_part *swp;
	class software_list_device *sld;
	std::string software_info_name;

	// methods
	void test_create(bool &can_create, bool &need_confirm);
	void load_software_part();
};

} // namespace ui

#endif /* MAME_FRONTEND_UI_IMAGECNTRL_H */
