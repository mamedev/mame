// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.h

    System and image info screens

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INFO_H
#define MAME_FRONTEND_UI_INFO_H

#pragma once

#include "ui/menu.h"

namespace ui {

class machine_info
{
public:
	// construction
	machine_info(running_machine &machine);

	// has... getters
	bool has_configs() const { return m_has_configs; }
	bool has_analog() const { return m_has_analog; }
	bool has_dips() const { return m_has_dips; }
	bool has_bioses() const { return m_has_bioses; }
	bool has_keyboard() const { return m_has_keyboard; }
	bool has_test_switch() const { return m_has_test_switch; }

	// text generators
	std::string warnings_string() const;
	std::string game_info_string() const;
	std::string mandatory_images() const;
	std::string get_screen_desc(screen_device &screen) const;

	// message colour
	rgb_t warnings_color() const;

private:
	// reference to machine
	running_machine &       m_machine;

	// overall feature status
	machine_flags::type     m_flags;
	device_t::feature_type  m_unemulated_features;
	device_t::feature_type  m_imperfect_features;

	// has...
	bool                    m_has_configs;
	bool                    m_has_analog;
	bool                    m_has_dips;
	bool                    m_has_bioses;
	bool                    m_has_keyboard;
	bool                    m_has_test_switch;
};

class menu_game_info : public menu
{
public:
	menu_game_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_game_info() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
};


class menu_image_info : public menu
{
public:
	menu_image_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_image_info() override;

private:
	virtual void populate(float &customtop, float &custombottom) override;
	virtual void handle() override;
	void image_info(device_image_interface *image);
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INFO_H
