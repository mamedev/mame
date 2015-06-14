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
//  ctor / dtor
//-------------------------------------------------

ui_menu_audit(running_machine &machine, render_container *container, std::vector<const game_driver *> &available, std::vector<const game_driver *> &unavailable, int _audit_mode)
	: ui_menu(machine, container), m_available(available), m_unavailable(unavailable)
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
}

//-------------------------------------------------
//  populate
//-------------------------------------------------

void ui_menu_audit::populate()
{
}
