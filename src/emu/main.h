// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    main.h

    Controls execution of the core emulator system.
***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MAIN_H__
#define __MAIN_H__

#include <time.h>

//**************************************************************************
//    CONSTANTS
//**************************************************************************

enum
{
	EMU_ERR_NONE             = 0,    /* no error */
	EMU_ERR_FAILED_VALIDITY  = 1,    /* failed validity checks */
	EMU_ERR_MISSING_FILES    = 2,    /* missing files */
	EMU_ERR_FATALERROR       = 3,    /* some other fatal error */
	EMU_ERR_DEVICE           = 4,    /* device initialization error (MESS-specific) */
	EMU_ERR_NO_SUCH_GAME     = 5,    /* game was specified but doesn't exist */
	EMU_ERR_INVALID_CONFIG   = 6,    /* some sort of error in configuration */
	EMU_ERR_IDENT_NONROMS    = 7,    /* identified all non-ROM files */
	EMU_ERR_IDENT_PARTIAL    = 8,    /* identified some files but not all */
	EMU_ERR_IDENT_NONE       = 9     /* identified no files */
};

//**************************************************************************
//    TYPE DEFINITIONS
//**************************************************************************
class osd_interface;

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
	static const char * get_bare_build_version();
	static const char * get_build_version();
	static void display_ui_chooser(running_machine& machine);
	static int start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[]);
	static void draw_user_interface(running_machine& machine);
	static void periodic_check();
	static bool frame_hook();
};

// ======================> machine_manager
class ui_manager;

class machine_manager
{
	DISABLE_COPYING(machine_manager);
protected:
	// construction/destruction
	machine_manager(emu_options &options, osd_interface &osd) : m_osd(osd), m_options(options), m_machine(nullptr) { }
public:
	virtual  ~machine_manager() { }

	osd_interface &osd() const { return m_osd; }
	emu_options &options() const { return m_options; }

	running_machine *machine() const { return m_machine; }

	void set_machine(running_machine *machine) { m_machine = machine; }

	virtual ui_manager* create_ui(running_machine& machine) { return nullptr;  }
	virtual void ui_initialize(running_machine& machine,bool firstrun) { }

	virtual void update_machine() { }
protected:
	osd_interface &         m_osd;                  // reference to OSD system
	emu_options &           m_options;              // reference to options
	running_machine *		m_machine;
};


#endif  /* __MAIN_H__ */
