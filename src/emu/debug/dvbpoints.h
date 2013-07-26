/*********************************************************************

    dvbpoints.h

    Breakpoint debugger view.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
