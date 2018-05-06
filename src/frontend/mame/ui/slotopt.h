// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/slotopt.h

    Internal menu for the slot options.

***************************************************************************/
#ifndef MAME_FRONTEND_UI_SLOTOPT_H
#define MAME_FRONTEND_UI_SLOTOPT_H

#pragma once

#include "ui/menu.h"

#include <unordered_map>

namespace ui {
class menu_slot_devices : public menu
{
public:
	menu_slot_devices(mame_ui_manager &mui, render_container &container);
	virtual ~menu_slot_devices() override;

private:
	enum class step_t
	{
		NEXT,
		PREVIOUS
	};

	virtual void populate(float &customtop, float &custombottom) override;
	virtual void custom_render(void *selectedref, float top, float bottom, float origx1, float origy1, float origx2, float origy2) override;
	virtual void handle() override;

	device_slot_interface::slot_option const *get_current_option(device_slot_interface &slot) const;
	void set_slot_device(device_slot_interface &slot, const char *val);
	void record_current_options();
	bool try_refresh_current_options();
	void rotate_slot_device(device_slot_interface &slot, step_t step);

	// variables
	std::unique_ptr<machine_config>                 m_config;
	std::unordered_map<std::string, std::string>    m_slot_options;
	std::string                                     m_current_option_list_slot_tag;
	std::vector<std::string>                        m_current_option_list;
	std::vector<std::string>::const_iterator        m_current_option_list_iter;
};

} // namespace ui

#endif  /* MAME_FRONTEND_UI_SLOTOPT_H */
