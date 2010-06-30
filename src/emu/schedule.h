/***************************************************************************

    schedule.h

    Core device execution and scheduling engine.

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

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __SCHEDULE_H__
#define __SCHEDULE_H__


//**************************************************************************
//  MACROS
//**************************************************************************

// these must be macros because we are included before the running_machine
#define cpuexec_describe_context(mach)				(mach)->describe_context()
#define cpuexec_boost_interleave(mach, slice, dur)	(mach)->scheduler().boost_interleave(slice, dur)
#define cpuexec_trigger(mach, trigid)				(mach)->scheduler().trigger(trigid)
#define cpuexec_triggertime(mach, trigid, dur)		(mach)->scheduler().trigger(trigid, dur)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_scheduler

class device_scheduler
{
	friend class device_execute_interface;

public:
	device_scheduler(running_machine &machine);
	~device_scheduler();

	void timeslice();

	void trigger(int trigid, attotime after = attotime_zero);

	void boost_interleave(attotime timeslice_time, attotime boost_duration);
	void abort_timeslice();

	device_execute_interface *currently_executing() const { return m_executing_device; }

	// for timer system only!
	attotime override_local_time(attotime default_time);

	// for emergencies only!
	void eat_all_cycles();

private:
	void compute_perfect_interleave();
	void rebuild_execute_list();

	static TIMER_CALLBACK( static_timed_trigger );

	running_machine &			m_machine;				// reference to our owner
	bool						m_quantum_set;			// have we set the scheduling quantum yet?
	device_execute_interface *	m_executing_device;		// pointer to currently executing device
	device_execute_interface *	m_execute_list;			// list of devices to be executed
};


#endif	/* __SCHEDULE_H__ */
