// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    mame.h

    Controls execution of the core MAME system.
***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MAME_H__
#define __MAME_H__

#include <time.h>
#include "pluginopts.h"

class osd_interface;

//**************************************************************************
//    CONSTANTS
//**************************************************************************

// return values from run_game
enum
{
	MAMERR_NONE             = 0,    /* no error */
	MAMERR_FAILED_VALIDITY  = 1,    /* failed validity checks */
	MAMERR_MISSING_FILES    = 2,    /* missing files */
	MAMERR_FATALERROR       = 3,    /* some other fatal error */
	MAMERR_DEVICE           = 4,    /* device initialization error (MESS-specific) */
	MAMERR_NO_SUCH_GAME     = 5,    /* game was specified but doesn't exist */
	MAMERR_INVALID_CONFIG   = 6,    /* some sort of error in configuration */
	MAMERR_IDENT_NONROMS    = 7,    /* identified all non-ROM files */
	MAMERR_IDENT_PARTIAL    = 8,    /* identified some files but not all */
	MAMERR_IDENT_NONE       = 9     /* identified no files */
};




//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

class emulator_info
{
public:
	// construction/destruction
	emulator_info() {};

	static const char * get_appname();
	static const char * get_appname_lower();
	static const char * get_configname();
	static const char * get_copyright();
	static const char * get_copyright_info();
};

class lua_engine;

// ======================> machine_manager

class machine_manager
{
	DISABLE_COPYING(machine_manager);
private:
	// construction/destruction
	machine_manager(emu_options &options, osd_interface &osd);
public:
	static machine_manager *instance(emu_options &options, osd_interface &osd);
	static machine_manager *instance();
	~machine_manager();

	osd_interface &osd() const;
	emu_options &options() const { return m_options; }
	plugin_options &plugins() const { return *m_plugins; }
	lua_engine *lua() { return m_lua; }

	running_machine *machine() { return m_machine; }

	void set_machine(running_machine *machine) { m_machine = machine; }

	void update_machine();

	/* execute as configured by the OPTION_SYSTEMNAME option on the specified options */
	int execute();
	void start_luaengine();
	void schedule_new_driver(const game_driver &driver);
private:
	osd_interface &         m_osd;                  // reference to OSD system
	emu_options &           m_options;              // reference to options
	std::unique_ptr<plugin_options> m_plugins;              // pointer to plugin options
	lua_engine *            m_lua;

	const game_driver *     m_new_driver_pending;   // pointer to the next pending driver

	running_machine *m_machine;
	static machine_manager* m_manager;
};

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern const char build_version[];
extern const char bare_build_version[];

#endif  /* __MAME_H__ */
