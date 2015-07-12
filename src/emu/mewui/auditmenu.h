/***************************************************************************

    mewui/auditmenu.h

    Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MEWUI_AUDIT_H__
#define __MEWUI_AUDIT_H__

//-------------------------------------------------
//  class audit menu
//-------------------------------------------------

class ui_menu_audit : public ui_menu
{
public:
	ui_menu_audit(running_machine &machine, render_container *container, std::vector<const game_driver *> &available, std::vector<const game_driver *> &unavailable, std::vector<const game_driver *> &availablesorted, std::vector<const game_driver *> &unavailablesorted, int audit_mode);
	virtual ~ui_menu_audit();
	virtual void populate();
	virtual void handle();

private:
	std::vector<const game_driver *> &m_available;
	std::vector<const game_driver *> &m_unavailable;
	std::vector<const game_driver *> &m_availablesorted;
	std::vector<const game_driver *> &m_unavailablesorted;

	int m_audit_mode, x;
	size_t m_size;
	void save_available_machines();
};

#endif /* __MEWUI_AUDIT_H__ */
