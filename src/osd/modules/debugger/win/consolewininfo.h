// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  consolewininfo.h - Win32 debug window handling
//
//============================================================
#ifndef MAME_DEBUGGER_WIN_CONSOLEWININFO_H
#define MAME_DEBUGGER_WIN_CONSOLEWININFO_H

#pragma once

#include "debugwin.h"

#include "disasmbasewininfo.h"
#include "sourcewininfo.h"


namespace osd::debugger::win {

class consolewin_info : public sourcewin_info
{
public:
	consolewin_info(debugger_windows_interface &debugger);
	virtual ~consolewin_info();

	void set_cpu(device_t &device);

protected:
	virtual void recompute_children() override;
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;
	virtual void save_configuration_to_node(util::xml::data_node &node) override;

private:
	enum
	{
		DEVOPTION_OPEN,
		DEVOPTION_CREATE,
		DEVOPTION_CLOSE,
		DEVOPTION_ITEM,
		DEVOPTION_CASSETTE_STOPPAUSE,
		DEVOPTION_CASSETTE_PLAY,
		DEVOPTION_CASSETTE_RECORD,
		DEVOPTION_CASSETTE_REWIND,
		DEVOPTION_CASSETTE_FASTFORWARD,
		DEVOPTION_CASSETTE_MOTOR,
		DEVOPTION_CASSETTE_SOUND,
		DEVOPTION_MAX
	};

	virtual void process_string(std::string const &string) override;

	void open_image_file(device_image_interface &device);
	void create_image_file(device_image_interface &device);
	bool get_softlist_info(device_image_interface &img);

	device_t *m_current_cpu;
	HMENU m_devices_menu;
	std::map<std::string,std::string> slmap;
};

} // namespace osd::debugger::win

#endif // MAME_DEBUGGER_WIN_CONSOLEWININFO_H
