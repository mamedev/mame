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

class machine_static_info
{
public:
	// construction
	machine_static_info(const ui_options &options, machine_config const &config);

	// overall emulation status
	::machine_flags::type machine_flags() const { return m_flags; }
	device_t::feature_type unemulated_features() const { return m_unemulated_features; }
	device_t::feature_type imperfect_features() const { return m_imperfect_features; }

	// has... getters
	bool has_bioses() const { return m_has_bioses; }

	// has input types getters
	bool has_dips() const { return m_has_dips; }
	bool has_configs() const { return m_has_configs; }
	bool has_keyboard() const { return m_has_keyboard; }
	bool has_test_switch() const { return m_has_test_switch; }
	bool has_analog() const { return m_has_analog; }

	// message colour
	rgb_t status_color() const;
	rgb_t warnings_color() const;

protected:
	machine_static_info(const ui_options &options, machine_config const &config, ioport_list const &ports);

private:
	machine_static_info(const ui_options &options, machine_config const &config, ioport_list const *ports);

	const ui_options &		m_options;

	// overall feature status
	::machine_flags::type   m_flags;
	device_t::feature_type  m_unemulated_features;
	device_t::feature_type  m_imperfect_features;

	// has...
	bool                    m_has_bioses;

	// has input types
	bool                    m_has_dips;
	bool                    m_has_configs;
	bool                    m_has_keyboard;
	bool                    m_has_test_switch;
	bool                    m_has_analog;
};


class machine_info : public machine_static_info
{
public:
	// construction
	machine_info(running_machine &machine);

	// text generators
	std::string warnings_string() const;
	std::string game_info_string() const;
	std::string mandatory_images() const;
	std::string get_screen_desc(screen_device &screen) const;

private:
	// reference to machine
	running_machine &   m_machine;
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
