// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles, Nathan Woods
/***************************************************************************

    ui/info.h

    System and image info screens

***************************************************************************/

#ifndef MAME_FRONTEND_UI_INFO_H
#define MAME_FRONTEND_UI_INFO_H

#pragma once

#include "ui/textbox.h"

#include "notifier.h"

#include <string>
#include <vector>


namespace ui {

class machine_static_info
{
public:
	// construction
	machine_static_info(const ui_options &options, machine_config const &config);

	// overall emulation status
	::machine_flags::type machine_flags() const noexcept { return m_flags; }
	device_t::flags_type emulation_flags() const noexcept { return m_emulation_flags; }
	device_t::feature_type unemulated_features() const noexcept { return m_unemulated_features; }
	device_t::feature_type imperfect_features() const noexcept { return m_imperfect_features; }

	// has... getters
	bool has_nonworking_devices() const noexcept { return m_has_nonworking_devices; }
	bool has_bioses() const noexcept { return m_has_bioses; }

	// has input types getters
	bool has_dips() const noexcept { return m_has_dips; }
	bool has_configs() const noexcept { return m_has_configs; }
	bool has_keyboard() const noexcept { return m_has_keyboard; }
	bool has_test_switch() const noexcept { return m_has_test_switch; }
	bool has_analog() const noexcept { return m_has_analog; }

	// warning severity indications
	bool has_warnings() const noexcept;
	bool has_severe_warnings() const noexcept;
	rgb_t status_color() const noexcept;
	rgb_t warnings_color() const noexcept;

protected:
	machine_static_info(const ui_options &options, machine_config const &config, ioport_list const &ports);

private:
	machine_static_info(const ui_options &options, machine_config const &config, ioport_list const *ports);

	const ui_options &      m_options;

	// overall feature status
	::machine_flags::type   m_flags;
	device_t::flags_type    m_emulation_flags;
	device_t::feature_type  m_unemulated_features;
	device_t::feature_type  m_imperfect_features;

	// has...
	bool                    m_has_nonworking_devices;
	bool                    m_has_bioses;

	// has input types
	bool                    m_has_dips;
	bool                    m_has_configs;
	bool                    m_has_keyboard;
	bool                    m_has_test_switch;
	bool                    m_has_analog;

	// media loading issues
	bool                    m_media_warnings;
	bool                    m_severe_media_warnings;
};


class machine_info : public machine_static_info
{
public:
	// construction
	machine_info(running_machine &machine);

	// text generators
	std::string warnings_string() const;
	std::string game_info_string() const;
	std::string get_screen_desc(screen_device &screen) const;

private:
	// reference to machine
	running_machine &   m_machine;
};


class menu_game_info : public menu_textbox
{
public:
	menu_game_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_game_info() override;

protected:
	virtual void menu_activated() override;
	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate() override;
};


class menu_warn_info : public menu_textbox
{
public:
	menu_warn_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_warn_info() override;

protected:
	virtual void populate_text(std::optional<text_layout> &layout, float &width, int &lines) override;

private:
	virtual void populate() override;
};


class menu_image_info : public menu
{
public:
	menu_image_info(mame_ui_manager &mui, render_container &container);
	virtual ~menu_image_info() override;

protected:
	virtual void menu_activated() override;

private:
	virtual void populate() override;
	virtual bool handle(event const *ev) override;
	void image_info(device_image_interface &image);
	void reload(device_image_interface::media_change_event ev);

	std::vector<util::notifier_subscription> m_notifiers;
};

} // namespace ui

#endif // MAME_FRONTEND_UI_INFO_H
