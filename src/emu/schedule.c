/***************************************************************************

    schedule.c

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

#include "emu.h"
#include "profiler.h"
#include "debugger.h"


//**************************************************************************
//  DEBUGGING
//**************************************************************************

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define TEMPLOG	0



//**************************************************************************
//  CONSTANTS
//**************************************************************************

// internal trigger IDs
enum
{
	TRIGGER_INT 		= -2000,
	TRIGGER_YIELDTIME	= -3000,
	TRIGGER_SUSPENDTIME = -4000
};



//**************************************************************************
//  MACROS
//**************************************************************************

// these are macros to ensure inlining in device_scheduler::timeslice
#define ATTOTIME_LT(a,b)		((a).seconds < (b).seconds || ((a).seconds == (b).seconds && (a).attoseconds < (b).attoseconds))
#define ATTOTIME_NORMALIZE(a)	do { if ((a).attoseconds >= ATTOSECONDS_PER_SECOND) { (a).seconds++; (a).attoseconds -= ATTOSECONDS_PER_SECOND; } } while (0)



//**************************************************************************
//  CORE CPU EXECUTION
//**************************************************************************

//-------------------------------------------------
//  device_scheduler - constructor
//-------------------------------------------------

device_scheduler::device_scheduler(running_machine &machine) :
	m_machine(machine),
	m_quantum_set(false),
	m_executing_device(NULL),
	m_execute_list(NULL)
{
}


//-------------------------------------------------
//  device_scheduler - destructor
//-------------------------------------------------

device_scheduler::~device_scheduler()
{
}


//-------------------------------------------------
//  timeslice - execute all devices for a single
//  timeslice
//-------------------------------------------------

void device_scheduler::timeslice()
{
	bool call_debugger = ((m_machine.debug_flags & DEBUG_FLAG_ENABLED) != 0);
	timer_execution_state *timerexec = timer_get_execution_state(&m_machine);
if (TEMPLOG) printf("Timeslice start\n");

	// build the execution list if we don't have one yet
	if (m_execute_list == NULL)
		rebuild_execute_list();

	// loop until we hit the next timer
	while (ATTOTIME_LT(timerexec->basetime, timerexec->nextfire))
	{
if (TEMPLOG)
{
	void timer_print_first_timer(running_machine *machine);
	printf("Timeslice loop: basetime=%15.6f\n", attotime_to_double(timerexec->basetime));
	timer_print_first_timer(&m_machine);
}


		// by default, assume our target is the end of the next quantum
		attotime target;
		target.seconds = timerexec->basetime.seconds;
		target.attoseconds = timerexec->basetime.attoseconds + timerexec->curquantum;
		ATTOTIME_NORMALIZE(target);

		// however, if the next timer is going to fire before then, override
		assert(attotime_sub(timerexec->nextfire, target).seconds <= 0);
		if (ATTOTIME_LT(timerexec->nextfire, target))
			target = timerexec->nextfire;

		LOG(("------------------\n"));
		LOG(("cpu_timeslice: target = %s\n", attotime_string(target, 9)));

		// apply pending suspension changes
		UINT32 suspendchanged = 0;
		for (device_execute_interface *exec = m_execute_list; exec != NULL; exec = exec->m_nextexec)
		{
			suspendchanged |= (exec->m_suspend ^ exec->m_nextsuspend);
			exec->m_suspend = exec->m_nextsuspend;
			exec->m_nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
			exec->m_eatcycles = exec->m_nexteatcycles;
		}

		// recompute the execute list if any CPUs changed their suspension state
		if (suspendchanged != 0)
			rebuild_execute_list();

		// loop over non-suspended CPUs
		for (device_execute_interface *exec = m_execute_list; exec != NULL; exec = exec->m_nextexec)
		{
			// only process if our target is later than the CPU's current time (coarse check)
			if (target.seconds >= exec->m_localtime.seconds)
			{
				// compute how many attoseconds to execute this CPU
				attoseconds_t delta = target.attoseconds - exec->m_localtime.attoseconds;
				if (delta < 0 && target.seconds > exec->m_localtime.seconds)
					delta += ATTOSECONDS_PER_SECOND;
				assert(delta == attotime_to_attoseconds(attotime_sub(target, exec->m_localtime)));

				// if we have enough for at least 1 cycle, do the math
				if (delta >= exec->m_attoseconds_per_cycle)
				{
					// compute how many cycles we want to execute
					int ran = exec->m_cycles_running = divu_64x32((UINT64)delta >> exec->m_divshift, exec->m_divisor);
					LOG(("  cpu '%s': %d cycles\n", exec->device().tag(), exec->m_cycles_running));

					// if we're not suspended, actually execute
					if (exec->m_suspend == 0)
					{
						profiler_mark_start(exec->m_profiler);

						// note that this global variable cycles_stolen can be modified
						// via the call to cpu_execute
						exec->m_cycles_stolen = 0;
if (TEMPLOG) printf("Executing %s for %d cycles\n", exec->device().tag(), ran);
						m_executing_device = exec;
						*exec->m_icount = exec->m_cycles_running;
						if (!call_debugger)
							exec->execute_run();
						else
						{
							debugger_start_cpu_hook(&exec->device(), target);
							exec->execute_run();
							debugger_stop_cpu_hook(&exec->device());
						}

						// adjust for any cycles we took back
						assert(ran >= *exec->m_icount);
						ran -= *exec->m_icount;
						assert(ran >= exec->m_cycles_stolen);
						ran -= exec->m_cycles_stolen;
						profiler_mark_end();
					}
else
if (TEMPLOG) printf("Skipping %s for %d cycles\n", exec->device().tag(), ran);

					// account for these cycles
					exec->m_totalcycles += ran;

					// update the local time for this CPU
					attoseconds_t actualdelta = exec->m_attoseconds_per_cycle * ran;
					exec->m_localtime.attoseconds += actualdelta;
					ATTOTIME_NORMALIZE(exec->m_localtime);
					LOG(("         %d ran, %d total, time = %s\n", ran, (INT32)exec->m_totalcycles, attotime_string(exec->m_localtime, 9)));

					// if the new local CPU time is less than our target, move the target up
					if (ATTOTIME_LT(exec->m_localtime, target))
					{
						assert(attotime_compare(exec->m_localtime, target) < 0);
						target = exec->m_localtime;

						// however, if this puts us before the base, clamp to the base as a minimum
						if (ATTOTIME_LT(target, timerexec->basetime))
						{
							assert(attotime_compare(target, timerexec->basetime) < 0);
							target = timerexec->basetime;
						}
						LOG(("         (new target)\n"));
					}
				}
			}
		}
		m_executing_device = NULL;

		// update the base time
		timerexec->basetime = target;
	}
if (TEMPLOG) printf("Timeslice end\n");

	// execute timers
	timer_execute_timers(&m_machine);
}


//-------------------------------------------------
//  boost_interleave - temporarily boosts the
//  interleave factor
//-------------------------------------------------

void device_scheduler::boost_interleave(attotime timeslice_time, attotime boost_duration)
{
	// ignore timeslices > 1 second
	if (timeslice_time.seconds > 0)
		return;
	timer_add_scheduling_quantum(&m_machine, timeslice_time.attoseconds, boost_duration);
}


//-------------------------------------------------
//  eat_all_cycles - eat a ton of cycles on all
//  CPUs to force a quick exit
//-------------------------------------------------

void device_scheduler::eat_all_cycles()
{
	for (device_execute_interface *exec = m_execute_list; exec != NULL; exec = exec->m_nextexec)
		exec->eat_cycles(1000000000);
}



//**************************************************************************
//  GLOBAL HELPERS
//**************************************************************************

//-------------------------------------------------
//  cpuexec_abort_timeslice - abort execution
//  for the current timeslice
//-------------------------------------------------

void device_scheduler::abort_timeslice()
{
	if (m_executing_device != NULL)
		m_executing_device->abort_timeslice();
}


//-------------------------------------------------
//  trigger - generate a global trigger
//-------------------------------------------------

void device_scheduler::trigger(int trigid, attotime after)
{
	// ensure we have a list of executing devices
	if (m_execute_list == NULL)
		rebuild_execute_list();

	// if we have a non-zero time, schedule a timer
	if (after.attoseconds != 0 || after.seconds != 0)
		timer_set(&m_machine, after, (void *)this, trigid, static_timed_trigger);

	// send the trigger to everyone who cares
	else
		for (device_execute_interface *exec = m_execute_list; exec != NULL; exec = exec->m_nextexec)
			exec->trigger(trigid);
}


//-------------------------------------------------
//  static_timed_trigger - generate a trigger
//  after a given amount of time
//-------------------------------------------------

TIMER_CALLBACK( device_scheduler::static_timed_trigger )
{
	reinterpret_cast<device_scheduler *>(ptr)->trigger(param);
}


//-------------------------------------------------
//  compute_perfect_interleave - compute the
//  "perfect" interleave interval
//-------------------------------------------------

void device_scheduler::compute_perfect_interleave()
{
	// ensure we have a list of executing devices
	if (m_execute_list == NULL)
		rebuild_execute_list();

	// start with the first one
	device_execute_interface *first = m_execute_list;
	if (first != NULL)
	{
		attoseconds_t smallest = first->minimum_quantum();
		attoseconds_t perfect = ATTOSECONDS_PER_SECOND - 1;

		// start with a huge time factor and find the 2nd smallest cycle time
		for (device_execute_interface *exec = first->m_nextexec; exec != NULL; exec = exec->m_nextexec)
		{
			attoseconds_t curquantum = exec->minimum_quantum();

			// find the 2nd smallest cycle interval
			if (curquantum < smallest)
			{
				perfect = smallest;
				smallest = curquantum;
			}
			else if (curquantum < perfect)
				perfect = curquantum;
		}

		// adjust the final value
		timer_set_minimum_quantum(&m_machine, perfect);

		LOG(("Perfect interleave = %.9f, smallest = %.9f\n", ATTOSECONDS_TO_DOUBLE(perfect), ATTOSECONDS_TO_DOUBLE(smallest)));
	}
}


//-------------------------------------------------
//  rebuild_execute_list - rebuild the list of
//  executing CPUs, moving suspended CPUs to the
//  end
//-------------------------------------------------

void device_scheduler::rebuild_execute_list()
{
	// if we haven't yet set a scheduling quantum, do it now
	if (!m_quantum_set)
	{
		// set the core scheduling quantum
		attotime min_quantum = m_machine.config->minimum_quantum;

		// if none specified default to 60Hz
		if (attotime_compare(min_quantum, attotime_zero) == 0)
			min_quantum = ATTOTIME_IN_HZ(60);

		// if the configuration specifies a device to make perfect, pick that as the minimum
		if (m_machine.config->perfect_cpu_quantum != NULL)
		{
			device_t *device = m_machine.device(m_machine.config->perfect_cpu_quantum);
			if (device == NULL)
				fatalerror("Device '%s' specified for perfect interleave is not present!", m_machine.config->perfect_cpu_quantum);

			device_execute_interface *exec;
			if (!device->interface(exec))
				fatalerror("Device '%s' specified for perfect interleave is not an executing device!", m_machine.config->perfect_cpu_quantum);

			attotime cpu_quantum = attotime_make(0, exec->minimum_quantum());
			min_quantum = attotime_min(cpu_quantum, min_quantum);
		}

		// inform the timer system of our decision
		assert(min_quantum.seconds == 0);
		timer_add_scheduling_quantum(&m_machine, min_quantum.attoseconds, attotime_never);
if (TEMPLOG) printf("Setting quantum: %08X%08X\n", (UINT32)(min_quantum.attoseconds >> 32), (UINT32)min_quantum.attoseconds);
		m_quantum_set = true;
	}

	// start with an empty list
	device_execute_interface **active_tailptr = &m_execute_list;
	*active_tailptr = NULL;

	// also make an empty list of suspended devices
	device_execute_interface *suspend_list = NULL;
	device_execute_interface **suspend_tailptr = &suspend_list;

	// iterate over all devices
	device_execute_interface *exec = NULL;
	for (bool gotone = m_machine.devicelist.first(exec); gotone; gotone = exec->next(exec))
	{
		// append to the appropriate list
		exec->m_nextexec = NULL;
		if (exec->m_suspend == 0)
		{
			*active_tailptr = exec;
			active_tailptr = &exec->m_nextexec;
		}
		else
		{
			*suspend_tailptr = exec;
			suspend_tailptr = &exec->m_nextexec;
		}
	}

	// append the suspend list to the end of the active list
	*active_tailptr = suspend_list;
if (TEMPLOG)
{
	printf("Execute list:");
	for (exec = m_execute_list; exec != NULL; exec = exec->m_nextexec)
		printf(" %s", exec->device().tag());
	printf("\n");
}
}
