// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  consolewininfo.h - Win32 debug window handling
//
//============================================================

#ifndef __DEBUG_WIN_CONSOLE_WIN_INFO_H__
#define __DEBUG_WIN_CONSOLE_WIN_INFO_H__

#include "debugwin.h"

#include "disasmbasewininfo.h"


class consolewin_info : public disasmbasewin_info
{
public:
	consolewin_info(debugger_windows_interface &debugger);
	virtual ~consolewin_info();

	void set_cpu(device_t &device);

protected:
	virtual void recompute_children() override;
	virtual void update_menu() override;
	virtual bool handle_command(WPARAM wparam, LPARAM lparam) override;

private:
	enum
	{
		DEVOPTION_OPEN,
		DEVOPTION_CREATE,
		DEVOPTION_CLOSE,
		DEVOPTION_CASSETTE_STOPPAUSE,
		DEVOPTION_CASSETTE_PLAY,
		DEVOPTION_CASSETTE_RECORD,
		DEVOPTION_CASSETTE_REWIND,
		DEVOPTION_CASSETTE_FASTFORWARD,
		DEVOPTION_MAX
	};

	virtual void process_string(char const *string) override;

	static void build_generic_filter(device_image_interface *img, bool is_save, std::string &filter);
	static void add_filter_entry(std::string &dest, char const *description, char const *extensions);
	static void copy_extension_list(std::string &dest, char const *extensions);

	HMENU   m_devices_menu;
};

#endif
