// license:BSD-3-Clause
// copyright-holders:Andrew Gardner, Vas Crabb
/*********************************************************************

    dvbpoints.h

    Breakpoint debugger view.

***************************************************************************/
#ifndef MAME_EMU_DEBUG_DVBPOINTS_H
#define MAME_EMU_DEBUG_DVBPOINTS_H

#pragma once

#include "debugvw.h"

#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for breakpoints
class debug_view_breakpoints : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_breakpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_breakpoints();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_click(const int button, const debug_view_xy& pos) override;

private:
	// internal helpers
	void enumerate_sources();
	void pad_ostream_to_length(std::ostream& str, int len);
	void gather_breakpoints();

	// internal state
	bool (*m_sortType)(const debug_breakpoint *, const debug_breakpoint *);
	std::vector<const debug_breakpoint *> m_buffer;
};

#endif // MAME_EMU_DEBUG_DVBPOINTS_H
