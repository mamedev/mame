// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    ui/prscntrl.cpp

    MAME's clunky built-in preset selection in a fixed list

***************************************************************************/

#include "emu.h"

#include "ui/prscntrl.h"
#include "ui/ui.h"

namespace ui {

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  ctor
//-------------------------------------------------

menu_control_device_preset::menu_control_device_preset(mame_ui_manager &mui, render_container &container, device_image_interface &image)
	: menu(mui, container)
	, m_image(image)
{
}


//-------------------------------------------------
//  dtor
//-------------------------------------------------

menu_control_device_preset::~menu_control_device_preset()
{
}


//-------------------------------------------------
//  populate
//-------------------------------------------------

void menu_control_device_preset::populate()
{
	auto presets = m_image.preset_images_list();
	for(uintptr_t i = 0; i != uintptr_t(presets.size()); i++)
		item_append(presets[i], 0, reinterpret_cast<void *>(i));
	set_selection(reinterpret_cast<void *>(uintptr_t(m_image.current_preset_image_id())));
}


//-------------------------------------------------
//  handle
//-------------------------------------------------

bool menu_control_device_preset::handle(event const *ev)
{
	if (ev && (ev->iptkey == IPT_UI_SELECT)) {
		int id = reinterpret_cast<uintptr_t>(ev->itemref);
		m_image.switch_preset_image(id);
		stack_pop();
	}
	return false;
}

} // namespace ui
