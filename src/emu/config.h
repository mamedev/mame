// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    config.h

    Wrappers for handling MAME configuration files

***************************************************************************/

#ifndef MAME_EMU_CONFIG_H
#define MAME_EMU_CONFIG_H

#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>


enum class config_type : int
{
	INIT,           // opportunity to initialize things first
	CONTROLLER,     // loading from controller file
	DEFAULT,        // loading from default.cfg
	SYSTEM,         // loading from system.cfg
	FINAL           // opportunity to finish initialization
};

enum class config_level : int
{
	DEFAULT,
	SOURCE,
	BIOS,
	PARENT,
	SYSTEM
};


class configuration_manager
{
public:
	typedef delegate<void (config_type, config_level, util::xml::data_node const *)> load_delegate;
	typedef delegate<void (config_type, util::xml::data_node *)> save_delegate;

	static inline constexpr int CONFIG_VERSION = 10;

	// construction/destruction
	configuration_manager(running_machine &machine);
	~configuration_manager();

	void config_register(std::string_view name, load_delegate &&load, save_delegate &&save);

	bool load_settings();
	void save_settings();

private:
	struct config_handler
	{
		load_delegate load;
		save_delegate save;
	};

	running_machine &machine() const { return m_machine; }

	bool attempt_load(game_driver const &system, emu_file &file, std::string_view name, config_type which_type);

	bool load_xml(game_driver const &system, emu_file &file, config_type which_type);
	bool save_xml(emu_file &file, config_type which_type);

	void save_unhandled(std::unique_ptr<util::xml::file> &unhandled, util::xml::data_node const &systemnode);
	void restore_unhandled(util::xml::file const &unhandled, util::xml::data_node &systemnode);

	// internal state
	running_machine &m_machine;
	std::multimap<std::string, config_handler> m_typelist;
	std::unique_ptr<util::xml::file> m_unhandled_default;
	std::unique_ptr<util::xml::file> m_unhandled_system;
};

#endif // MAME_EMU_CONFIG_H
