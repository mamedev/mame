// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*********************************************************************

    dvbpoints.h

    Breakpoint debugger view.

***************************************************************************/

#ifndef __DVBPOINTS_H__
#define __DVBPOINTS_H__

#include "debugvw.h"
#include "debugcpu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// debug view for breakpoints
class debug_view_breakpoints : public debug_view
{
	friend resource_pool_object<debug_view_breakpoints>::~resource_pool_object();
	friend class debug_view_manager;

	// construction/destruction
	debug_view_breakpoints(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_breakpoints();

public:
	enum SortMode
	{
		SORT_NONE,
		SORT_INDEX_ASCENDING,
		SORT_INDEX_DESCENDING,
		SORT_ENABLED_ASCENDING,
		SORT_ENABLED_DESCENDING,
		SORT_CPU_ASCENDING,
		SORT_CPU_DESCENDING,
		SORT_ADDRESS_ASCENDING,
		SORT_ADDRESS_DESCENDING,
		SORT_CONDITION_ASCENDING,
		SORT_CONDITION_DESCENDING,
		SORT_ACTION_ASCENDING,
		SORT_ACTION_DESCENDING
	};

	// getters
	// setters

protected:
	// view overrides
	virtual void view_update();
	virtual void view_notify(debug_view_notification type);
	virtual void view_char(int chval);
	virtual void view_click(const int button, const debug_view_xy& pos);

private:
	// internal helpers
	void enumerate_sources();
	bool recompute(offs_t pc, int startline, int lines);
	void pad_astring_to_length(astring& str, int len);
	int breakpoints(SortMode sort, device_debug::breakpoint**& bpList);


	// internal state
	SortMode m_sortType;
};


#endif
