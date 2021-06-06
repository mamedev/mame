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


#define VERIFY_INPUT_LINE_IN_RANGE (1)


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// suspension reasons for executing devices
constexpr u32 SUSPEND_REASON_HALT          = 0x0001;   // HALT line set (or equivalent)
constexpr u32 SUSPEND_REASON_RESET         = 0x0002;   // RESET line set (or equivalent)
constexpr u32 SUSPEND_REASON_DISABLE       = 0x0004;   // disabled (due to disable flag)
constexpr u32 SUSPEND_REASON_CLOCK         = 0x0008;   // currently not clocked
constexpr u32 SUSPEND_REASON_SPIN          = 0x0010;   // currently spinning
constexpr u32 SUSPEND_REASON_SPIN_TRIGGER  = 0x0020;   // spinning until a trigger
constexpr u32 SUSPEND_REASON_SPIN_SLICE    = 0x0040;   // spinning until the next timeslice
constexpr u32 SUSPEND_REASON_YIELD_TRIGGER = 0x0100;   // yielding until a trigger
constexpr u32 SUSPEND_REASON_YIELD_SLICE   = 0x0200;   // yielding until the next timeslice

constexpr u32 SUSPEND_ANY_REASON           = ~0;       // all of the above
constexpr u32 SUSPEND_YIELD_REASONS        = SUSPEND_REASON_YIELD_TRIGGER | SUSPEND_REASON_YIELD_SLICE;
constexpr u32 SUSPEND_TRIGGER_REASONS      = SUSPEND_REASON_SPIN_TRIGGER | SUSPEND_REASON_YIELD_TRIGGER;
constexpr u32 SUSPEND_SLICE_REASONS        = SUSPEND_REASON_SPIN_SLICE | SUSPEND_REASON_YIELD_SLICE;


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

	// special input lines that are implemented in the core
	INPUT_LINES_INTERNAL = 3,
	INPUT_LINE_NMI = -1,
	INPUT_LINE_RESET = -2,
	INPUT_LINE_HALT = -3,
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

	// internal information about the state of inputs
	struct device_input
	{
		attotime m_last_event_time; // time of last enqueued event
		s32 m_stored_vector;        // most recently written vector
		s32 m_live_vector;          // most recently processed vector
		u8 m_live_state;            // most recently processed state
	};

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
		m_periodic_interrupt.set(std::forward<F>(cb), name);
		m_periodic_interrupt_period = rate;
	}
	template <typename T, typename F> void set_periodic_int(T &&target, F &&cb, const char *name, const attotime &rate)
	{
		m_periodic_interrupt.set(std::forward<T>(target), std::forward<F>(cb), name);
		m_periodic_interrupt_period = rate;
	}
	void remove_periodic_int()
	{
		m_periodic_interrupt = device_interrupt_delegate(*this);
		m_periodic_interrupt_period = attotime();
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
	void set_input_line(int linenum, int state) { enqueue_input_line_change(linenum, state, input_from_line(linenum).m_stored_vector); }
	void set_input_line_vector(int linenum, int vector) { input_from_line(linenum).m_stored_vector = vector; }
	void set_input_line_and_vector(int linenum, int state, int vector) { enqueue_input_line_change(linenum, state, vector); }
	int input_state(int linenum) const { return input_from_line(linenum).m_live_state; }
	void pulse_input_line(int irqline, const attotime &duration);

	// suspend/resume
	void suspend(u32 reason);
	void resume(u32 reason);
	bool suspended(u32 reason = SUSPEND_ANY_REASON) const noexcept { return (m_nextsuspend & reason) != 0; }
	void yield() { suspend(SUSPEND_REASON_YIELD_SLICE); }
	void spin() { suspend(SUSPEND_REASON_SPIN_SLICE); }
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
	subseconds minimum_quantum() const;

	// required operation overrides
	void run() { execute_run(); }

	// deliberately ambiguous functions; if you have the execute interface
	// just use it
	device_execute_interface &execute() { return *this; }

	// debugger hooks
	bool debugger_enabled() const { return device().machine().debug_enabled(); }
	void debugger_instruction_hook(offs_t curpc)
	{
		if (device().machine().debug_flags() & DEBUG_FLAG_CALL_HOOK)
			device().debug()->instruction_hook(curpc);
	}
	void debugger_exception_hook(int exception)
	{
		if (device().machine().debug_enabled())
			device().debug()->exception_hook(exception);
	}

	void debugger_privilege_hook()
	{
		if (device().machine().debug_enabled())
			device().debug()->privilege_hook();
	}

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
	bool update_suspend();

	// for use by devcpu for now...
	int current_input_state(unsigned linenum) const { return input_from_line(linenum).m_live_state; }
	void set_icountptr(int &icount) { assert(!m_icountptr); m_icountptr = &icount; }
	IRQ_CALLBACK_MEMBER(standard_irq_callback_member);
	int standard_irq_callback(int irqline);

private:
	void suspend_resume_changed();

	void run_debug();
	void run_suspend();

	void on_vblank(screen_device &screen, bool vblank_state);

	void periodic_interrupt(timer_instance const &timer);
	void process_input_event(timer_instance const &timer);

	device_input &input_from_line(int linenum)
	{
		int index = linenum + INPUT_LINES_INTERNAL;
		if (VERIFY_INPUT_LINE_IN_RANGE && (index < 0 || index >= m_input.size()))
			throw emu_fatalerror("%s: input_from_line(%d) failure", device().tag(), linenum);
		return m_input[index];
	}
	device_input const &input_from_line(int linenum) const
	{
		int index = linenum + INPUT_LINES_INTERNAL;
		if (VERIFY_INPUT_LINE_IN_RANGE && (index < 0 || index >= m_input.size()))
			throw emu_fatalerror("%s: input_from_line(%d) failure", device().tag(), linenum);
		return m_input[index];
	}
	void enqueue_input_line_change(int line, int state, int vector);

	// core execution state: keep all these members close to the top
	// so they live within the first 128 bytes of the object; this helps
	// the super-hot execution loop stay lean & mean on x64 systems
	device_execute_interface *m_nextexec;               // pointer to the next device to execute, in order
	int *                   m_icountptr;                // pointer to the icount
	union
	{
		u64                 combined;                   // running+stolen as a single 64-bit value
		struct
		{
#ifdef LSB_FIRST
			u32             running;                    // number of cycles we are executing
			s32             stolen;                     // number of cycles we artificially stole
#else
			s32             stolen;                     // number of cycles we artificially stole
			u32             running;                    // number of cycles we are executing
#endif
		} separate;
	} m_cycles;
	u32                     m_cycles_per_second;        // cycles per second, adjusted for multipliers
	subseconds              m_subseconds_per_cycle;     // subseconds per adjusted clock cycle
	u64                     m_totalcycles;              // total device cycles executed
	device_scheduler::basetime_relative m_localtime;    // local time, relative to the scheduler's base
	execute_delegate        m_run_delegate;             // currently active run delegate
	profile_type            m_profiler;                 // profiler tag
	// end core execution state

	// scheduler
	device_scheduler *      m_scheduler;                // pointer to the machine scheduler

	// suspend states
	u32                     m_suspend;                  // suspend reason mask (0 = not suspended)
	u32                     m_nextsuspend;              // pending suspend reason mask
	s32                     m_trigger;                  // pending trigger to release a trigger suspension
	s32                     m_inttrigger;               // interrupt trigger index

	// configuration
	bool                    m_disabled;                 // disabled from executing?
	device_interrupt_delegate m_vblank_interrupt;       // for interrupts tied to VBLANK
	const char *            m_vblank_interrupt_screen;  // the screen that causes the VBLANK interrupt
	device_interrupt_delegate m_periodic_interrupt;     // for interrupts not tied to VBLANK
	attotime                m_periodic_interrupt_period;// period for periodic interrupts

	// execution delegates
	execute_delegate        m_run_fast_delegate;        // normal run delegate
	execute_delegate        m_run_debug_delegate;       // debugging run delegate
	execute_delegate        m_suspend_delegate;         // suspend delegate

	// timers
	transient_timer_factory m_timed_trigger;            // timer for signalling triggers
	transient_timer_factory m_set_input_line;           // timer for setting input lines
	transient_timer_factory m_process_input_event;      // timer for processing input events
	persistent_timer        m_periodic_interrupt_timer; // timer for generating periodic interrupts

	// input states and IRQ callbacks
	device_irq_acknowledge_delegate m_driver_irq;       // driver-specific IRQ callback
	std::vector<device_input> m_input;                  // data about inputs
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

	// compute how many cycles we want to execute, rounding up; note that we
	// pre-cache subseconds per cycle prior to execution, since the clock can
	// be changed dynamically; we also keep it in raw unsigned form since we
	// know it is positive, and the compiler does smarter things with unsigned
	// values in this critical loop
	u64 subseconds_per_cycle = m_subseconds_per_cycle.raw();

	// in a similar vein, ensure that the subseconds value we are passaed is
	// also positive
	scheduler_assert(subs.raw() >= 0);

	// compute the number of cycles; we could divide subseconds by the
	// subseconds per cycle value we cached above, but that's a 64/64-bit
	// divide and expensive; instead, we take advantage of the fact that
	// subseconds is just 2 bits away from being a 64-bit fractional value;
	// so we just left-shift it by 2 to get it into that form, and then do
	// a much cheaper 64x64 multiply by the cycles per second value (which
	// subseconds per cycle is derived from), keeping the upper half as our
	// final result
	u64 ran64 = mulu_64x64_hi(u64(subs.raw()) << 2, m_cycles_per_second) + 1;

	// ran64 will fit in 32 bits, so we take advantage of that fact to write
	// to 2 neighboring 32-bit values at once, with the lower 32 bits setting
	// the number of cycles we will run, while the zeroed upper 32 bits will
	// reset the count of cycles stolen
	m_cycles.combined = ran64;

	// set the device's icount value to the number of cycles we want; again
	// we pre-cache the pointer so that we don't have to re-fetch it when we
	// come back from execution; the fact that we have a direct pointer to the
	// icount is an artifact of the original MAME design; it would also better
	// be an explicit s32 instead of an int
	auto *icountptr = m_icountptr;
	u32 ran = u32(ran64);
	*icountptr = ran;

	// now run the device for the number of cycles; note that m_run_delegate is
	// dynamically switched based on the suspend and debugging states
	m_run_delegate();

	// now let's see how many cycles we actually ran; if the device's icount is
	// negative, then we ran more than requested (this is both allowed and
	// expected), so the subtract here typically will increase ran
	scheduler_assert(int(ran) >= *icountptr);
	ran -= *icountptr;

	// if cycles were stolen (i.e., icount was artificially decremented), then
	// ran isn't actually correct, so remove the number of cycles that we did
	// that for
	scheduler_assert(s32(ran) >= m_cycles.separate.stolen);
	ran -= m_cycles.separate.stolen;

	// time should never go backwards, nor should we ever attempt to execute
	// more than a full second (minimum quantum prevents that); special case if
	// the clock is 0, as rounding guarantees we ask for at least 1 cycle
	scheduler_assert(ran < m_cycles_per_second || (ran <= 1 && m_cycles_per_second == 0));

	// update our count of total cycles executed with the true number of cycles
	m_totalcycles += ran;

	// update our local time so that it represents an integral number of cycles
	u64 ran_subseconds = subseconds_per_cycle * ran;
	m_localtime.add(subseconds::from_raw(ran_subseconds));

	g_profiler.stop();

	// return the current localtime as a basetime-relative value
	return m_localtime.relative();
}


//-------------------------------------------------
//  update_suspend - clock the pending suspension
//  states forward, updating execution delegates
//  along the way; return true if a timeslice
//  suspend is active
//-------------------------------------------------

inline bool device_execute_interface::update_suspend()
{
	// update suspend state
	u32 delta = m_suspend ^ m_nextsuspend;
	m_suspend = m_nextsuspend;

	// clear the timeslice reasons always for the next round
	m_nextsuspend &= ~SUSPEND_SLICE_REASONS;

	// update the execution delegate
	if (delta != 0)
	{
		if (m_suspend != 0)
			m_run_delegate = m_suspend_delegate;
		else if (!debugger_enabled())
			m_run_delegate = m_run_fast_delegate;
		else
			m_run_delegate = m_run_debug_delegate;
	}

	// return true if a timeslice suspend is active
	return (m_suspend != m_nextsuspend);
}

#endif // MAME_EMU_DIEXEC_H
