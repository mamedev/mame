// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*********************************************************************

    dvrpoints.h

    Registerpoint debugger view.

***************************************************************************/
#ifndef MAME_EMU_DEBUG_DVRPOINTS_H
#define MAME_EMU_DEBUG_DVRPOINTS_H

#pragma once

#include "debugvw.h"

#include <utility>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for breakpoints
class debug_view_registerpoints : public debug_view
{
	friend class debug_view_manager;

	// construction/destruction
	debug_view_registerpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_registerpoints();

protected:
	// view overrides
	virtual void view_update() override;
	virtual void view_click(int button, debug_view_xy const &pos) override;

private:
	using point_pair = std::pair<device_t *, debug_registerpoint const *>;

	// internal helpers
	void enumerate_sources();
	void pad_ostream_to_length(std::ostream& str, int len);
	void gather_registerpoints();

	// internal state
	bool (*m_sort_type)(point_pair const &, point_pair const &);
	std::vector<point_pair> m_buffer;
};

#endif // MAME_EMU_DEBUG_DVBPOINTS_H
