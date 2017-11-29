// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    main.cpp

    Controls execution of the core ZEXALL system.

***************************************************************************/

#include "emu.h"

#include "ui/uimain.h"

#include "emuopts.h"
#include "drivenum.h"

#include "xmlfile.h"

#include "modules/lib/osdobj_common.h"


GAME_EXTERN(zexall);

const game_driver * const driver_list::s_drivers_sorted[2] =
{
	&GAME_NAME(___empty),
	&GAME_NAME(zexall),
};

std::size_t const driver_list::s_driver_count = 2;

// ======================> zexall_machine_manager

class zexall_machine_manager : public machine_manager
{
private:
	DISABLE_COPYING(zexall_machine_manager);
	// construction/destruction
	zexall_machine_manager(emu_options &options, osd_interface &osd) : machine_manager(options, osd) { }
public:
	static zexall_machine_manager *instance(emu_options &options, osd_interface &osd)
	{
		if (!m_manager)
		{
			m_manager = new zexall_machine_manager(options, osd);
		}
		return m_manager;
	}

	static zexall_machine_manager *instance() { return m_manager; }

	~zexall_machine_manager() { delete m_manager;  m_manager = nullptr; }

	int execute()
	{
		auto system = &GAME_NAME(zexall);
		machine_config config(*system, m_options);
		running_machine machine(config, *this);
		return machine.run(false);
	}

	virtual ui_manager* create_ui(running_machine& machine) override {
		m_ui = std::make_unique<ui_manager>(machine);
		return m_ui.get();
	}

private:
	static zexall_machine_manager* m_manager;
	std::unique_ptr<ui_manager> m_ui;
};

//**************************************************************************
//  MACHINE MANAGER
//**************************************************************************

zexall_machine_manager* zexall_machine_manager::m_manager = nullptr;

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, std::vector<std::string> &args)
{
	options.set_value(OSDOPTION_VIDEO, "none", OPTION_PRIORITY_MAXIMUM);
	options.set_value(OSDOPTION_SOUND, "none", OPTION_PRIORITY_MAXIMUM);
	//options.set_value(OPTION_DEBUG, true, OPTION_PRIORITY_MAXIMUM);
	options.set_value(OPTION_THROTTLE, false, OPTION_PRIORITY_MAXIMUM);

	zexall_machine_manager::instance(options,osd)->start_http_server();
	zexall_machine_manager::instance(options,osd)->execute();
	return 0;
}

int emulator_info::start_frontend(emu_options &options, osd_interface &osd, int argc, char *argv[])
{
	std::vector<std::string> args(argv, argv + argc);
	return start_frontend(options, osd, args);
}

const char * emulator_info::get_bare_build_version() { return nullptr; }

const char * emulator_info::get_build_version() { return nullptr; }

void emulator_info::display_ui_chooser(running_machine& machine) { }

void emulator_info::draw_user_interface(running_machine& machine) { }

void emulator_info::periodic_check() { }

bool emulator_info::frame_hook() { return false; }

void emulator_info::layout_file_cb(util::xml::data_node const &layout) { }

const char * emulator_info::get_appname() { return nullptr; }

const char * emulator_info::get_appname_lower() { return "zexall"; }

const char * emulator_info::get_configname() { return nullptr; }

const char * emulator_info::get_copyright() { return nullptr; }

const char * emulator_info::get_copyright_info() { return nullptr; }

bool emulator_info::standalone() { return true; }
