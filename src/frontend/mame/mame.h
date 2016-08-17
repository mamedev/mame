// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    mame.h

    Controls execution of the core MAME system.
***************************************************************************/

#pragma once

#ifndef __MAME_H__
#define __MAME_H__

class plugin_options;
class osd_interface;

//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************
class lua_engine;
class cheat_manager;
class inifile_manager;
class favorite_manager;
class mame_ui_manager;

namespace ui {
class datfile_manager;
} // namespace ui

// ======================> machine_manager

class mame_machine_manager : public machine_manager
{
	DISABLE_COPYING(mame_machine_manager);
private:
	// construction/destruction
	mame_machine_manager(emu_options &options, osd_interface &osd);
public:
	static mame_machine_manager *instance(emu_options &options, osd_interface &osd);
	static mame_machine_manager *instance();
	~mame_machine_manager();

	plugin_options &plugins() const { return *m_plugins; }
	lua_engine *lua() { return m_lua; }

	virtual void update_machine() override;

	void reset();
	TIMER_CALLBACK_MEMBER(autoboot_callback);

	virtual ui_manager* create_ui(running_machine& machine) override;

	virtual void create_custom(running_machine& machine) override;

	virtual void ui_initialize(running_machine& machine) override;

	/* execute as configured by the OPTION_SYSTEMNAME option on the specified options */
	int execute();
	void start_luaengine();
	void schedule_new_driver(const game_driver &driver);
	mame_ui_manager& ui() const { assert(m_ui != nullptr); return *m_ui; }
	cheat_manager &cheat() const { assert(m_cheat != nullptr); return *m_cheat; }
	ui::datfile_manager &datfile() const { assert(m_datfile != nullptr); return *m_datfile; }
	inifile_manager &inifile() const { assert(m_inifile != nullptr); return *m_inifile; }
	favorite_manager &favorite() const { assert(m_favorite != nullptr); return *m_favorite; }
private:
	std::unique_ptr<plugin_options> m_plugins;              // pointer to plugin options
	lua_engine *            m_lua;

	const game_driver *     m_new_driver_pending;   // pointer to the next pending driver
	bool                    m_firstrun;

	static mame_machine_manager* m_manager;
	emu_timer               *m_autoboot_timer;      // autoboot timer
	std::unique_ptr<mame_ui_manager> m_ui;                  // internal data from ui.cpp
	std::unique_ptr<cheat_manager> m_cheat;            // internal data from cheat.cpp
	std::unique_ptr<ui::datfile_manager>   m_datfile;      // internal data from datfile.c
	std::unique_ptr<inifile_manager>   m_inifile;      // internal data from inifile.c for INIs
	std::unique_ptr<favorite_manager>  m_favorite;     // internal data from inifile.c for favorites

};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char build_version[];
extern const char bare_build_version[];

#endif  /* __MAME_H__ */
