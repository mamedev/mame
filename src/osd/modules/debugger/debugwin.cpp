// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Vas Crabb
//============================================================
//
//  debugwin.c - Win32 debug window handling
//
//============================================================

#include "debug_module.h"
#include "modules/osdmodule.h"

#if defined(OSD_WINDOWS) /*|| defined(SDLMAME_WIN32)*/

#include "win/debugwin.h"

#include "win/consolewininfo.h"
#include "win/debugwininfo.h"
#include "win/disasmwininfo.h"
#include "win/logwininfo.h"
#include "win/memorywininfo.h"
#include "win/pointswininfo.h"
#include "win/uimetrics.h"

#include "emu.h"
#include "debugger.h"

#include "window.h"
#include "../../windows/input.h"


class debugger_windows : public osd_module, public debug_module, protected debugger_windows_interface
{
public:
	debugger_windows() :
		osd_module(OSD_DEBUG_PROVIDER, "windows"),
		debug_module(),
		m_machine(NULL),
		m_metrics(),
		m_waiting_for_debugger(false),
		m_window_list(),
		m_main_console(NULL)
	{
	}

	virtual ~debugger_windows() { }

	virtual int init(const osd_options &options) { return 0; }
	virtual void exit();

	virtual void init_debugger(running_machine &machine);
	virtual void wait_for_debugger(device_t &device, bool firststop);
	virtual void debugger_update();

protected:
	virtual running_machine &machine() const { return *m_machine; }

	virtual ui_metrics &metrics() const { return *m_metrics; }

	virtual bool const &waiting_for_debugger() const { return m_waiting_for_debugger; }
	virtual bool seq_pressed() const;

	virtual void create_memory_window() { create_window<memorywin_info>(); }
	virtual void create_disasm_window() { create_window<disasmwin_info>(); }
	virtual void create_log_window() { create_window<logwin_info>(); }
	virtual void create_points_window() { create_window<pointswin_info>(); }
	virtual void remove_window(debugwin_info &info);

	virtual void show_all();
	virtual void hide_all();

private:
	template <typename T> T *create_window();

	running_machine             *m_machine;
	auto_pointer<ui_metrics>    m_metrics;
	bool                        m_waiting_for_debugger;
	simple_list<debugwin_info>  m_window_list;
	consolewin_info             *m_main_console;
};


void debugger_windows::exit()
{
	// loop over windows and free them
	while (m_window_list.first() != NULL)
		m_window_list.first()->destroy();

	m_main_console = NULL;
	m_metrics.reset();
	m_machine = NULL;
}


void debugger_windows::init_debugger(running_machine &machine)
{
	m_machine = &machine;
	m_metrics.reset(global_alloc(ui_metrics(downcast<osd_options &>(m_machine->options()))));
}


void debugger_windows::wait_for_debugger(device_t &device, bool firststop)
{
	// create a console window
	if (m_main_console == NULL)
		m_main_console = create_window<consolewin_info>();

	// update the views in the console to reflect the current CPU
	if (m_main_console != NULL)
		m_main_console->set_cpu(device);

	// when we are first stopped, adjust focus to us
	if (firststop && (m_main_console != NULL))
	{
		m_main_console->set_foreground();
		if (winwindow_has_focus())
			m_main_console->set_default_focus();
	}

	// make sure the debug windows are visible
	m_waiting_for_debugger = true;
	show_all();

	// run input polling to ensure that our status is in sync
	wininput_poll(*m_machine);

	// get and process messages
	MSG message;
	GetMessage(&message, NULL, 0, 0);

	switch (message.message)
	{
	// check for F10 -- we need to capture that ourselves
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		if (message.wParam == VK_F4 && message.message == WM_SYSKEYDOWN)
			SendMessage(GetAncestor(GetFocus(), GA_ROOT), WM_CLOSE, 0, 0);
		if (message.wParam == VK_F10)
			SendMessage(GetAncestor(GetFocus(), GA_ROOT), (message.message == WM_SYSKEYDOWN) ? WM_KEYDOWN : WM_KEYUP, message.wParam, message.lParam);
		break;

	// process everything else
	default:
		winwindow_dispatch_message(*m_machine, &message);
		break;
	}

	// mark the debugger as active
	m_waiting_for_debugger = false;
}


void debugger_windows::debugger_update()
{
	// if we're running live, do some checks
	if (!winwindow_has_focus() && m_machine && !debug_cpu_is_stopped(*m_machine) && (m_machine->phase() == MACHINE_PHASE_RUNNING))
	{
		// see if the interrupt key is pressed and break if it is
		if (seq_pressed())
		{
			HWND const focuswnd = GetFocus();

			debug_cpu_get_visible_cpu(*m_machine)->debug()->halt_on_next_instruction("User-initiated break\n");

			// if we were focused on some window's edit box, reset it to default
			for (debugwin_info *info = m_window_list.first(); info != NULL; info = info->next())
				info->restore_field(focuswnd);
		}
	}
}


bool debugger_windows::seq_pressed() const
{
	input_seq const &seq = m_machine->ioport().type_seq(IPT_UI_DEBUG_BREAK);
	bool result = false;
	bool invert = false;
	bool first = true;

	// iterate over all of the codes
	for (int codenum = 0, length = seq.length(); codenum < length; codenum++)
	{
		input_code code = seq[codenum];

		if (code == input_seq::not_code)
		{
			// handle NOT
			invert = true;
		}
		else if (code == input_seq::or_code || code == input_seq::end_code)
		{
			// handle OR and END

			// if we have a positive result from the previous set, we're done
			if (result || code == input_seq::end_code)
				break;

			// otherwise, reset our state
			result = false;
			invert = false;
			first = true;
		}
		else
		{
			// handle everything else as a series of ANDs
			int const vkey = wininput_vkey_for_mame_code(code);
			bool const pressed = (vkey != 0) && ((GetAsyncKeyState(vkey) & 0x8000) != 0);

			if (first)          // if this is the first in the sequence, result is set equal
				result = pressed ^ invert;
			else if (result)    // further values are ANDed
				result = result && (pressed ^ invert);

			// no longer first, and clear the invert flag
			first = invert = false;
		}
	}

	// return the result if we queried at least one switch
	return result;
}


void debugger_windows::remove_window(debugwin_info &info)
{
	m_window_list.remove(info);
}


void debugger_windows::show_all()
{
	for (debugwin_info *info = m_window_list.first(); info != NULL; info = info->next())
		info->show();
}


void debugger_windows::hide_all()
{
	SetForegroundWindow(win_window_list->m_hwnd);
	for (debugwin_info *info = m_window_list.first(); info != NULL; info = info->next())
		info->hide();
}


template <typename T> T *debugger_windows::create_window()
{
	// allocate memory
	T *info = global_alloc(T(*this));
	if (info->is_valid())
	{
		m_window_list.prepend(*info);
		return info;
	}
	else
	{
		global_free(info);
		return NULL;
	}
}


#else /* not windows */
MODULE_NOT_SUPPORTED(debugger_windows, OSD_DEBUG_PROVIDER, "windows")
#endif

MODULE_DEFINITION(DEBUG_WINDOWS, debugger_windows)
