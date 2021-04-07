// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    diexec.h

    Device execution interfaces.

***************************************************************************/

#ifndef MAME_EMU_DIEXEC_H
#define MAME_EMU_DIEXEC_H

#pragma once

#include "debug/debugcpu.h"


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// suspension reasons for executing devices
constexpr u32 SUSPEND_REASON_HALT       = 0x0001;   // HALT line set (or equivalent)
constexpr u32 SUSPEND_REASON_RESET      = 0x0002;   // RESET line set (or equivalent)
constexpr u32 SUSPEND_REASON_SPIN       = 0x0004;   // currently spinning
constexpr u32 SUSPEND_REASON_TRIGGER    = 0x0008;   // waiting for a trigger
constexpr u32 SUSPEND_REASON_DISABLE    = 0x0010;   // disabled (due to disable flag)
constexpr u32 SUSPEND_REASON_TIMESLICE  = 0x0020;   // waiting for the next timeslice
constexpr u32 SUSPEND_REASON_CLOCK      = 0x0040;   // currently not clocked
constexpr u32 SUSPEND_ANY_REASON        = ~0;       // all of the above


// I/O line states
enum line_state
{
	CLEAR_LINE = 0,             // clear (a fired or held) line
	ASSERT_LINE,                // assert an interrupt immediately
	HOLD_LINE                   // hold interrupt line until acknowledged
};


// I/O line definitions
enum
{
	// input lines
	MAX_INPUT_LINES = 64+3,
	INPUT_LINE_IRQ0 = 0,
	INPUT_LINE_IRQ1 = 1,
	INPUT_LINE_IRQ2 = 2,
	INPUT_LINE_IRQ3 = 3,
	INPUT_LINE_IRQ4 = 4,
	INPUT_LINE_IRQ5 = 5,
	INPUT_LINE_IRQ6 = 6,
	INPUT_LINE_IRQ7 = 7,
	INPUT_LINE_IRQ8 = 8,
	INPUT_LINE_IRQ9 = 9,
	INPUT_LINE_NMI = MAX_INPUT_LINES - 3,

	// special input lines that are implemented in the core
	INPUT_LINE_RESET = MAX_INPUT_LINES - 2,
	INPUT_LINE_HALT = MAX_INPUT_LINES - 1
};



//**************************************************************************
//  MACROS
//**************************************************************************

// IRQ callback to be called by device implementations when an IRQ is actually taken
#define IRQ_CALLBACK_MEMBER(func)       int func(device_t &device, int irqline)

// interrupt generator callback called as a VBLANK or periodic interrupt
#define INTERRUPT_GEN_MEMBER(func)      void func(device_t &device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// interrupt callback for VBLANK and timed interrupts
using device_interrupt_delegate = device_delegate<void (device_t &)>;

// IRQ callback to be called by executing devices when an IRQ is actually taken
using device_irq_acknowledge_delegate = device_delegate<int (device_t &, int)>;



// ======================> device_execute_interface

class device_execute_interface : public device_interface
{
	friend class device_scheduler;
	friend class testcpu_state;
	friend class device_input;

	using execute_delegate = delegate<void ()>;

public:
	// construction/destruction
	device_execute_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_execute_interface();

	// configuration access
	bool disabled() const { return m_disabled; }
	u64 clocks_to_cycles(u64 clocks) const { return execute_clocks_to_cycles(clocks); }
	u64 cycles_to_clocks(u64 cycles) const { return execute_cycles_to_clocks(cycles); }
	u32 min_cycles() const { return execute_min_cycles(); }
	u32 max_cycles() const { return execute_max_cycles(); }
	attotime cycles_to_attotime(u64 cycles) const { return device().clocks_to_attotime(cycles_to_clocks(cycles)); }
	u64 attotime_to_cycles(const attotime &duration) const { return clocks_to_cycles(device().attotime_to_clocks(duration)); }
	u32 input_lines() const { return execute_input_lines(); }
	u32 default_irq_vector(int linenum) const { return execute_default_irq_vector(linenum); }
	bool input_edge_triggered(int linenum) const { return execute_input_edge_triggered(linenum); }

	// inline configuration helpers
	void set_disable() { m_disabled = true; }

	template <typename... T> void set_vblank_int(const char *tag, T &&... args)
	{
		m_vblank_interrupt.set(std::forward<T>(args)...);
		m_vblank_interrupt_screen = tag;
	}
	void remove_vblank_int()
	{
		m_vblank_interrupt = device_interrupt_delegate(*this);
		m_vblank_interrupt_screen = nullptr;
	}

	template <typename F> void set_periodic_int(F &&cb, const char *name, const attotime &rate)
	{
		m_timed_interrupt.set(std::forward<F>(cb), name);
		m_timed_interrupt_period = rate;
	}
	template <typename T, typename F> void set_periodic_int(T &&target, F &&cb, const char *name, const attotime &rate)
	{
		m_timed_interrupt.set(std::forward<T>(target), std::forward<F>(cb), name);
		m_timed_interrupt_period = rate;
	}
	void remove_periodic_int()
	{
		m_timed_interrupt = device_interrupt_delegate(*this);
		m_timed_interrupt_period = attotime();
	}

	template <typename... T> void set_irq_acknowledge_callback(T &&... args)
	{
		m_driver_irq.set(std::forward<T>(args)...);
	}
	void remove_irq_acknowledge_callback()
	{
		m_driver_irq = device_irq_acknowledge_delegate(*this);
	}

	// execution management
	device_scheduler &scheduler() const noexcept { assert(m_scheduler != nullptr); return *m_scheduler; }
	bool executing() const noexcept { return scheduler().currently_executing() == this; }
	s32 cycles_remaining() const noexcept { return executing() ? *m_icountptr : 0; } // cycles remaining in this timeslice
	void eat_cycles(int cycles) noexcept { if (executing()) *m_icountptr = (cycles > *m_icountptr) ? 0 : (*m_icountptr - cycles); }
	void adjust_icount(int delta) noexcept { if (executing()) *m_icountptr += delta; }
	void abort_timeslice() noexcept;

	// input and interrupt management
	void set_input_line(int linenum, int state) { m_input[linenum].set_state_synced(state); }
	void set_input_line_vector(int linenum, int vector) { m_input[linenum].set_vector(vector); }
	void set_input_line_and_vector(int linenum, int state, int vector) { m_input[linenum].set_state_synced(state, vector); }
	int input_state(int linenum) const { return m_input[linenum].m_curstate; }
	void pulse_input_line(int irqline, const attotime &duration);

	// suspend/resume
	void suspend(u32 reason, bool eatcycles);
	void resume(u32 reason);
	bool suspended(u32 reason = SUSPEND_ANY_REASON) const noexcept { return (m_nextsuspend & reason) != 0; }
	void yield() { suspend(SUSPEND_REASON_TIMESLICE, false); }
	void spin() { suspend(SUSPEND_REASON_TIMESLICE, true); }
	void spin_until_trigger(int trigid) { suspend_until_trigger(trigid, true); }
	void spin_until_time(const attotime &duration);
	void spin_until_interrupt() { spin_until_trigger(m_inttrigger); }

	// triggers
	void suspend_until_trigger(int trigid, bool eatcycles);
	void trigger(int trigid);
	void signal_interrupt_trigger() { trigger(m_inttrigger); }

	// time and cycle accounting
	attotime local_time() noexcept;
	u64 total_cycles() const noexcept;
	attotime minimum_quantum_time() const { return attotime(0, minimum_quantum()); }

	// required operation overrides
	void run() { execute_run(); }

	// deliberately ambiguous functions; if you have the execute interface
	// just use it
	device_execute_interface &execute() { return *this; }

protected:
	// clock and cycle information getters
	virtual u64 execute_clocks_to_cycles(u64 clocks) const noexcept;
	virtual u64 execute_cycles_to_clocks(u64 cycles) const noexcept;
	virtual u32 execute_min_cycles() const noexcept;
	virtual u32 execute_max_cycles() const noexcept;

	// input line information getters
	virtual u32 execute_input_lines() const noexcept;
	virtual u32 execute_default_irq_vector(int linenum) const noexcept;
	virtual bool execute_input_edge_triggered(int linenum) const noexcept;

	// optional operation overrides
	virtual void execute_run() = 0;
	virtual void execute_burn(s32 cycles);
	virtual void execute_set_input(int linenum, int state);

	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;
	virtual void interface_post_reset() override;
	virtual void interface_clock_changed() override;

	// device_scheduler helpers
	subseconds run_for(subseconds subs);
	u32 update_suspend();

	// for use by devcpu for now...
	int current_input_state(unsigned i) const { return m_input[i].m_curstate; }
	void set_icountptr(int &icount) { assert(!m_icountptr); m_icountptr = &icount; }
	IRQ_CALLBACK_MEMBER(standard_irq_callback_member);
	int standard_irq_callback(int irqline);

	// debugger hooks
	bool debugger_enabled() const { return bool(device().machine().debug_flags & DEBUG_FLAG_ENABLED); }
	void debugger_instruction_hook(offs_t curpc)
	{
		if (device().machine().debug_flags & DEBUG_FLAG_CALL_HOOK)
			device().debug()->instruction_hook(curpc);
	}
	void debugger_exception_hook(int exception)
	{
		if (device().machine().debug_flags & DEBUG_FLAG_ENABLED)
			device().debug()->exception_hook(exception);
	}

	void debugger_privilege_hook()
	{
		if (device().machine().debug_flags & DEBUG_FLAG_ENABLED)
			device().debug()->privilege_hook();
	}

private:
	void suspend_resume_changed();
	subseconds minimum_quantum() const;

	void run_debug();
	void run_suspend();

	void on_vblank(screen_device &screen, bool vblank_state);

	TIMER_CALLBACK_MEMBER(trigger_periodic_interrupt);
	TIMER_CALLBACK_MEMBER(irq_pulse_clear) { set_input_line(int(param), CLEAR_LINE); }
	TIMER_CALLBACK_MEMBER(empty_event_queue) { m_input[param].empty_event_queue(); }

	// internal information about the state of inputs
	class device_input
	{
		static constexpr int USE_STORED_VECTOR = 0xff000000;

	public:
		device_input();

		void start(device_execute_interface &execute, int linenum);
		void reset();

		void set_state_synced(int state, int vector = USE_STORED_VECTOR);
		void set_vector(int vector) { m_stored_vector = vector; }
		int default_irq_callback();
		void empty_event_queue();

		device_execute_interface *m_execute;// pointer to the execute interface
		int             m_linenum;          // which input line we are

		s32             m_stored_vector;    // most recently written vector
		s32             m_curvector;        // most recently processed vector
		u8              m_curstate;         // most recently processed state
		s32             m_queue[32];        // queue of pending events
		int             m_qindex;           // index within the queue
	};

	void synchronize_event_queue(int line) { m_empty_event_queue.synchronize(line); }

	// internal debugger hooks
	void debugger_start_cpu_hook(const attotime &endtime)
	{
		if (device().machine().debug_flags & DEBUG_FLAG_ENABLED)
			device().debug()->start_hook(endtime);
	}
	void debugger_stop_cpu_hook()
	{
		if (device().machine().debug_flags & DEBUG_FLAG_ENABLED)
			device().debug()->stop_hook();
	}

	// scheduler
	device_scheduler *      m_scheduler;                // pointer to the machine scheduler

	// core execution state: keep all these members close to the top
	// so they live within the first 128 bytes of the object; this helps
	// the super-hot execution loop stay lean & mean on x64 systems
	device_execute_interface *m_nextexec;               // pointer to the next device to execute, in order
	execute_delegate *      m_run_delegate;             // currently active run delegate
	int *                   m_icountptr;                // pointer to the icount
	int                     m_cycles_running;           // number of cycles we are executing
	int                     m_cycles_stolen;            // number of cycles we artificially stole
	u32                     m_cycles_per_second;        // cycles per second, adjusted for multipliers
	subseconds              m_subseconds_per_cycle;     // subseconds per adjusted clock cycle
	u64                     m_totalcycles;              // total device cycles executed
	device_scheduler::basetime_relative m_localtime;    // local time, relative to the scheduler's base
	profile_type            m_profiler;                 // profiler tag
	// end core execution state

	// suspend states
	u32                     m_suspend;                  // suspend reason mask (0 = not suspended)
	u32                     m_nextsuspend;              // pending suspend reason mask
	u8                      m_eatcycles;                // true if we eat cycles while suspended
	u8                      m_nexteatcycles;            // pending value
	s32                     m_trigger;                  // pending trigger to release a trigger suspension
	s32                     m_inttrigger;               // interrupt trigger index

	// configuration
	bool                    m_disabled;                 // disabled from executing?
	device_interrupt_delegate m_vblank_interrupt;       // for interrupts tied to VBLANK
	const char *            m_vblank_interrupt_screen;  // the screen that causes the VBLANK interrupt
	device_interrupt_delegate m_timed_interrupt;        // for interrupts not tied to VBLANK
	attotime                m_timed_interrupt_period;   // period for periodic interrupts

	// execution delegates
	execute_delegate        m_run_fast_delegate;        // normal run delegate
	execute_delegate        m_run_debug_delegate;       // debugging run delegate
	execute_delegate        m_suspend_delegate;         // suspend delegate

	// timers
	transient_timer_factory m_timed_trigger_callback;
	transient_timer_factory m_irq_pulse_clear;
	transient_timer_factory m_empty_event_queue;

	// input states and IRQ callbacks
	persistent_timer        m_timedint_timer;           // reference to this device's periodic interrupt timer
	device_irq_acknowledge_delegate m_driver_irq;       // driver-specific IRQ callback
	device_input            m_input[MAX_INPUT_LINES];   // data about inputs
};

// iterator
using execute_interface_enumerator = device_interface_enumerator<device_execute_interface>;


//-------------------------------------------------
//  run_for - execute for the given number of
//  subseconds; note that this function is super
//  hot, so be extremely careful making any
//  changes here
//-------------------------------------------------

inline subseconds device_execute_interface::run_for(subseconds subs)
{
	g_profiler.start(m_profiler);

	// compute how many cycles we want to execute, rounding up
	// note that we pre-cache subseconds per cycle
	subseconds subseconds_per_cycle = m_subseconds_per_cycle;
	s32 ran = subs / subseconds_per_cycle + 1;

	// store the number of cycles we've requested in the executing
	// device
	// TODO: do we need to do this?
	m_cycles_running = ran;

	// set the device's icount value to the number of cycles we want
	// the fact that we have a direct point to this is an artifact of
	// the original MAME design
	auto *icountptr = m_icountptr;
	*icountptr = ran;

	// clear m_cycles_stolen, which gets updated if the timeslice
	// is aborted (due to synchronization or setting a new timer to
	// expire before the original timeslice end)
	m_cycles_stolen = 0;

	// now run the device for the number of cycles
	(*m_run_delegate)();

	// now let's see how many cycles we actually ran; if the device's
	// icount is negative, then we ran more than requested (this is both
	// allowed and expected), so the subtract here typically will
	// increase ran
	scheduler_assert(ran >= *icountptr);
	ran -= *icountptr;

	// if cycles were stolen (i.e., icount was artificially decremented)
	// then ran isn't actually correct, so remove the number of cycles
	// that we did that for
	scheduler_assert(ran >= m_cycles_stolen);
	ran -= m_cycles_stolen;

	// time should never go backwards, nor should we ever attempt to
	// execute more than a full second (minimum quantum prevents that)
	scheduler_assert(ran >= 0 && ran < m_cycles_per_second);

	// update the device's count of total cycles executed with the
	// true number of cycles
	m_totalcycles += ran;

	// update the local time for the device so that it represents an
	// integral number of cycles
	m_localtime.add(subseconds_per_cycle * ran);

	g_profiler.stop();

	// return the current localtime as a basetime-relative value
	return m_localtime.relative();
}


//-------------------------------------------------
//  update_suspend - clock the pending suspension
//  states forward, updating execution delegates
//  along the way
//-------------------------------------------------

inline u32 device_execute_interface::update_suspend()
{
	// update the suspend and eatcycles states
	u32 delta = m_suspend ^ m_nextsuspend;
	m_suspend = m_nextsuspend;
	m_nextsuspend &= ~SUSPEND_REASON_TIMESLICE;
	m_eatcycles = m_nexteatcycles;

	// update the execution delegate
	if (m_suspend != 0)
		m_run_delegate = &m_suspend_delegate;
	else if (!m_scheduler->machine().debug_enabled())
		m_run_delegate = &m_run_fast_delegate;
	else
		m_run_delegate = &m_run_debug_delegate;

	return delta;
}

#endif // MAME_EMU_DIEXEC_H
