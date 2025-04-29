// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    main.h

    Controls execution of the core emulator system.
***************************************************************************/
#ifndef MAME_EMU_MAIN_H
#define MAME_EMU_MAIN_H

#pragma once

#include "emufwd.h"

#include <map>
#include <memory>
#include <string>
#include <vector>


//**************************************************************************
//    CONSTANTS
//**************************************************************************

constexpr int EMU_ERR_NONE             = 0;    // no error
constexpr int EMU_ERR_FAILED_VALIDITY  = 1;    // failed validity checks
constexpr int EMU_ERR_MISSING_FILES    = 2;    // missing files
constexpr int EMU_ERR_FATALERROR       = 3;    // some other fatal error
constexpr int EMU_ERR_DEVICE           = 4;    // device initialization error
constexpr int EMU_ERR_NO_SUCH_SYSTEM   = 5;    // system was specified but doesn't exist
constexpr int EMU_ERR_INVALID_CONFIG   = 6;    // some sort of error in configuration
constexpr int EMU_ERR_IDENT_NONROMS    = 7;    // identified all non-ROM files
constexpr int EMU_ERR_IDENT_PARTIAL    = 8;    // identified some files but not all
constexpr int EMU_ERR_IDENT_NONE       = 9;    // identified no files


//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************

class emulator_info
{
public:
	// construction/destruction
	emulator_info() = default;

	static const char *get_appname();
	static const char *get_appname_lower();
	static const char *get_configname();
	static const char *get_copyright();
	static const char *get_copyright_info();
	static const char *get_bare_build_version();
	static const char *get_build_version();
	static void display_ui_chooser(running_machine &machine);
	static int start_frontend(emu_options &options, osd_interface &osd, std::vector<std::string> &args);
	static int start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[]);
	static bool draw_user_interface(running_machine& machine);
	static void periodic_check();
	static bool frame_hook();
	static void sound_hook(const std::map<std::string, std::vector<std::pair<const float *, int>>> &sound); // Can't use sound_stream::sample_t sadly
	static void layout_script_cb(layout_file &file, const char *script);
	static bool standalone();
};


class machine_manager
{
protected:
	// construction/destruction
	machine_manager(emu_options &options, osd_interface &osd);
	machine_manager(machine_manager const &) = delete;

public:
	virtual ~machine_manager();

	osd_interface &osd() const { return m_osd; }
	emu_options &options() const { return m_options; }

	running_machine *machine() const { return m_machine; }

	void set_machine(running_machine *machine) { m_machine = machine; }

	virtual ui_manager* create_ui(running_machine& machine) { return nullptr;  }
	virtual void create_custom(running_machine& machine) { }
	virtual void load_cheatfiles(running_machine& machine) { }
	virtual void ui_initialize(running_machine& machine) { }
	virtual void before_load_settings(running_machine &machine) { }

	virtual void update_machine() { }

	http_manager *http();
	void start_http_server();

protected:
	osd_interface &               m_osd;                  // reference to OSD system
	emu_options &                 m_options;              // reference to options
	running_machine *             m_machine;
	std::unique_ptr<http_manager> m_http;
};

#endif // MAME_EMU_MAIN_H
