// license:BSD-3-Clause
// copyright-holders:Maurizio Petrarota
/*********************************************************************

    ui/auditmenu.cpp

    Internal UI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "ui/menu.h"
#include "audit.h"
#include "ui/auditmenu.h"
#include <algorithm>

extern const char UI_VERSION_TAG[];

//-------------------------------------------------
//  sort
//-------------------------------------------------

inline int cs_stricmp(const char *s1, const char *s2)
{
	for (;;)
	{
		int c1 = tolower((UINT8)*s1++);
		int c2 = tolower((UINT8)*s2++);
		if (c1 == 0 || c1 != c2)
			return c1 - c2;
	}
}

bool sorted_game_list(const game_driver *x, const game_driver *y)
{
	bool clonex = strcmp(x->parent, "0");
	bool cloney = strcmp(y->parent, "0");

	if (!clonex && !cloney)
		return (cs_stricmp(x->description, y->description) < 0);

	int cx = -1, cy = -1;
	if (clonex)
	{
		cx = driver_list::find(x->parent);
		if (cx == -1 || (driver_list::driver(cx).flags & MACHINE_IS_BIOS_ROOT) != 0)
			clonex = false;
	}

	if (cloney)
	{
		cy = driver_list::find(y->parent);
		if (cy == -1 || (driver_list::driver(cy).flags & MACHINE_IS_BIOS_ROOT) != 0)
			cloney = false;
	}

	if (!clonex && !cloney)
		return (cs_stricmp(x->description, y->description) < 0);
	else if (clonex && cloney)
	{
		if (!cs_stricmp(x->parent, y->parent))
			return (cs_stricmp(x->description, y->description) < 0);
		else
			return (cs_stricmp(driver_list::driver(cx).description, driver_list::driver(cy).description) < 0);
	}
	else if (!clonex && cloney)
	{
		if (!cs_stricmp(x->name, y->parent))
			return true;
		else
			return (cs_stricmp(x->description, driver_list::driver(cy).description) < 0);
	}
	else
	{
		if (!cs_stricmp(x->parent, y->name))
			return false;
		else
			return (cs_stricmp(driver_list::driver(cx).description, y->description) < 0);
	}
}

//-------------------------------------------------
//  ctor / dtor
//-------------------------------------------------

ui_menu_audit::ui_menu_audit(running_machine &machine, render_container *container, vptr_game &availablesorted, vptr_game &unavailablesorted,  int _audit_mode)
	: ui_menu(machine, container)
	, m_availablesorted(availablesorted)
	, m_unavailablesorted(unavailablesorted)
	, m_audit_mode(_audit_mode)
	, m_first(true)
{
	if (m_audit_mode == 2)
	{
		m_availablesorted.clear();
		m_unavailablesorted.clear();
	}
}

ui_menu_audit::~ui_menu_audit()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_audit::handle()
{
	process(UI_MENU_PROCESS_CUSTOM_ONLY);

	if (m_first)
	{
		machine().ui().draw_text_box(container, _("Audit in progress..."), JUSTIFY_CENTER, 0.5f, 0.5f, UI_GREEN_COLOR);
		m_first = false;
		return;
	}

	if (m_audit_mode == 1)
	{
		vptr_game::iterator iter = m_unavailablesorted.begin();
		while (iter != m_unavailablesorted.end())
		{
			driver_enumerator enumerator(machine().options(), (*iter)->name);
			enumerator.next();
			media_auditor auditor(enumerator);
			media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

			// if everything looks good, include the driver
			if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
			{
				m_availablesorted.push_back((*iter));
				iter = m_unavailablesorted.erase(iter);
			}
			else
				++iter;
		}
	}
	else
	{
		driver_enumerator enumerator(machine().options());
		media_auditor auditor(enumerator);
		while (enumerator.next())
		{
			media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

			// if everything looks good, include the driver
			if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
				m_availablesorted.push_back(&enumerator.driver());
			else
				m_unavailablesorted.push_back(&enumerator.driver());
		}
	}

	// sort
	std::stable_sort(m_availablesorted.begin(), m_availablesorted.end(), sorted_game_list);
	std::stable_sort(m_unavailablesorted.begin(), m_unavailablesorted.end(), sorted_game_list);
	save_available_machines();
	ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
	ui_menu::stack_pop(machine());
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_audit::populate()
{
	item_append("Dummy", nullptr, 0, (void *)(FPTR)1);
}

//-------------------------------------------------
//  save drivers infos to file
//-------------------------------------------------

void ui_menu_audit::save_available_machines()
{
	// attempt to open the output file
	emu_file file(machine().ui().options().ui_path(), OPEN_FLAG_WRITE | OPEN_FLAG_CREATE | OPEN_FLAG_CREATE_PATHS);
	if (file.open(emulator_info::get_configname(), "_avail.ini") == osd_file::error::NONE)
	{
		// generate header
		std::ostringstream buffer;
		buffer << "#\n" << UI_VERSION_TAG << bare_build_version << "\n#\n\n";
		util::stream_format(buffer, "%d\n", m_availablesorted.size());
		util::stream_format(buffer, "%d\n", m_unavailablesorted.size());

		// generate available list
		for (size_t x = 0; x < m_availablesorted.size(); ++x)
		{
			int find = driver_list::find(m_availablesorted[x]->name);
			util::stream_format(buffer, "%d\n", find);
		}

		// generate unavailable list
		for (size_t x = 0; x < m_unavailablesorted.size(); ++x)
		{
			int find = driver_list::find(m_unavailablesorted[x]->name);
			util::stream_format(buffer, "%d\n", find);
		}
		file.puts(buffer.str().c_str());
		file.close();
	}
}
