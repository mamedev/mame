/*********************************************************************

    dvstate.h

    State debugger view.

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

#ifndef __DVSTATE_H__
#define __DVSTATE_H__

#include "debugvw.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// data sources for state views
class debug_view_state_source : public debug_view_source
{
	friend class debug_view_state;

	// construction/destruction
	debug_view_state_source(const char *name, device_t &device);

public:
	// getters
	device_t &device() const { return m_device; }

private:
	// internal state
	device_t &          m_device;               // underlying device
	device_state_interface *m_stateintf;        // state interface
	device_execute_interface *m_execintf;       // execution interface
};


// debug view for state
class debug_view_state : public debug_view
{
	friend resource_pool_object<debug_view_state>::~resource_pool_object();
	friend class debug_view_manager;

	// construction/destruction
	debug_view_state(running_machine &machine, debug_view_osd_update_func osdupdate, void *osdprivate);
	virtual ~debug_view_state();

protected:
	// view overrides
	virtual void view_update();
	virtual void view_notify(debug_view_notification type);

private:
	struct state_item
	{
		state_item(int index, const char *name, UINT8 valuechars);

		state_item *        m_next;             // next item
		UINT64              m_lastval;          // last value
		UINT64              m_currval;          // current value
		int                 m_index;            // index
		UINT8               m_vallen;           // number of value chars
		astring             m_symbol;           // symbol
	};

	// internal helpers
	void enumerate_sources();
	void reset();
	void recompute();

	// internal state
	int                 m_divider;              // dividing column
	UINT64              m_last_update;          // execution counter at last update
	state_item *        m_state_list;           // state data

	// constants
	static const int REG_DIVIDER    = -10;
	static const int REG_CYCLES     = -11;
	static const int REG_BEAMX      = -12;
	static const int REG_BEAMY      = -13;
	static const int REG_FRAME      = -14;
};


#endif
