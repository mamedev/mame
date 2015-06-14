/*********************************************************************

    mewui/auditmenu.c

    Internal MEWUI user interface.

*********************************************************************/

#include "emu.h"
#include "ui/ui.h"
#include "audit.h"
#include "mewui/auditmenu.h"
#include "mewui/utils.h"

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
		if (cx == -1 || (cx != -1 && ((driver_list::driver(cx).flags & GAME_IS_BIOS_ROOT) != 0)))
			clonex = false;
	}

	if (cloney)
	{
		cy = driver_list::find(y->parent);
		if (cy == -1 || (cy != -1 && ((driver_list::driver(cy).flags & GAME_IS_BIOS_ROOT) != 0)))
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

ui_menu_audit::ui_menu_audit(running_machine &machine, render_container *container, std::vector<const game_driver *> &available, std::vector<const game_driver *> &unavailable, std::vector<const game_driver *> &availablesorted, std::vector<const game_driver *> &unavailablesorted,  int _audit_mode)
	: ui_menu(machine, container), m_available(available), m_unavailable(unavailable),
	  m_availablesorted(availablesorted), m_unavailablesorted(unavailablesorted)
{
	m_audit_mode = _audit_mode;
}

ui_menu_audit::~ui_menu_audit()
{
}

//-------------------------------------------------
//  handle
//-------------------------------------------------

void ui_menu_audit::handle()
{
	static bool steps = false;

	machine().ui().draw_text_box(container, "Audit in progress...", JUSTIFY_CENTER, 0.5f, 0.5f, UI_GREEN_COLOR);
	if (!steps)
	{
		steps = true;
		return;
	}

	std::vector<UINT8> m_included(m_unavailable.size(), 0);

	for (size_t x = 0; x < m_unavailable.size(); ++x)
	{
		driver_enumerator enumerator(machine().options(), m_unavailable[x]->name);
		enumerator.next();
		media_auditor auditor(enumerator);
		media_auditor::summary summary = auditor.audit_media(AUDIT_VALIDATE_FAST);

		// if everything looks good, include the driver
		if (summary == media_auditor::CORRECT || summary == media_auditor::BEST_AVAILABLE || summary == media_auditor::NONE_NEEDED)
		{
			m_available.push_back(m_unavailable[x]);
			m_included[x] = 1;
		}
	}

	std::vector<const game_driver *> m_remove;
	for (size_t x = 0; x < m_unavailable.size(); ++x)
		if (!m_included[x])
			m_remove.push_back(m_unavailable[x]);

	m_unavailable = m_remove;

	// sort
	m_availablesorted = m_available;
	std::stable_sort(m_availablesorted.begin(), m_availablesorted.end(), sorted_game_list);
	m_unavailablesorted = m_unavailable;
	std::stable_sort(m_unavailablesorted.begin(), m_unavailablesorted.end(), sorted_game_list);

	ui_menu::menu_stack->parent->reset(UI_MENU_RESET_SELECT_FIRST);
	ui_menu::stack_pop(machine());
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_audit::populate()
{
}
