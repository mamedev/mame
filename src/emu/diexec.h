// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    diexec.h

    Device execution interfaces.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DIEXEC_H__
#define __DIEXEC_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// suspension reasons for executing devices
const UINT32 SUSPEND_REASON_HALT        = 0x0001;   // HALT line set (or equivalent)
const UINT32 SUSPEND_REASON_RESET       = 0x0002;   // RESET line set (or equivalent)
const UINT32 SUSPEND_REASON_SPIN        = 0x0004;   // currently spinning
const UINT32 SUSPEND_REASON_TRIGGER     = 0x0008;   // waiting for a trigger
const UINT32 SUSPEND_REASON_DISABLE     = 0x0010;   // disabled (due to disable flag)
const UINT32 SUSPEND_REASON_TIMESLICE   = 0x0020;   // waiting for the next timeslice
const UINT32 SUSPEND_REASON_CLOCK       = 0x0040;   // currently not clocked
const UINT32 SUSPEND_ANY_REASON         = ~0;       // all of the above


// I/O line states
enum line_state
{
	CLEAR_LINE = 0,             // clear (a fired or held) line
	ASSERT_LINE,                // assert an interrupt immediately
	HOLD_LINE,                  // hold interrupt line until acknowledged
	PULSE_LINE                  // pulse interrupt line instantaneously (only for NMI, RESET)
};


// I/O line definitions
enum
{
	// input lines
	MAX_INPUT_LINES = 32+3,
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

#define TIMER_CALLBACK(name)            void name(running_machine &machine, void *ptr, int param)
#define TIMER_CALLBACK_MEMBER(name)     void name(void *ptr, INT32 param)

// IRQ callback to be called by device implementations when an IRQ is actually taken
#define IRQ_CALLBACK_MEMBER(func)       int func(device_t &device, int irqline)

// interrupt generator callback called as a VBLANK or periodic interrupt
#define INTERRUPT_GEN_MEMBER(func)      void func(device_t &device)



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_DEVICE_DISABLE() \
	device_execute_interface::static_set_disable(*device);
#define MCFG_DEVICE_VBLANK_INT_DRIVER(_tag, _class, _func) \
	device_execute_interface::static_set_vblank_int(*device, device_interrupt_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0), _tag);
#define MCFG_DEVICE_VBLANK_INT_DEVICE(_tag, _devtag, _class, _func) \
	device_execute_interface::static_set_vblank_int(*device, device_interrupt_delegate(&_class::_func, #_class "::" #_func, _devtag, (_class *)0), _tag);
#define MCFG_DEVICE_VBLANK_INT_REMOVE()  \
	device_execute_interface::static_set_vblank_int(*device, device_interrupt_delegate(), NULL);
#define MCFG_DEVICE_PERIODIC_INT_DRIVER(_class, _func, _rate) \
	device_execute_interface::static_set_periodic_int(*device, device_interrupt_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0), attotime::from_hz(_rate));
#define MCFG_DEVICE_PERIODIC_INT_DEVICE(_devtag, _class, _func, _rate) \
	device_execute_interface::static_set_periodic_int(*device, device_interrupt_delegate(&_class::_func, #_class "::" #_func, _devtag, (_class *)0), attotime::from_hz(_rate));
#define MCFG_DEVICE_PERIODIC_INT_REMOVE()  \
	device_execute_interface::static_set_periodic_int(*device, device_interrupt_delegate(), attotime());
#define MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(_class, _func) \
	device_execute_interface::static_set_irq_acknowledge_callback(*device, device_irq_acknowledge_delegate(&_class::_func, #_class "::" #_func, DEVICE_SELF, (_class *)0));
#define MCFG_DEVICE_IRQ_ACKNOWLEDGE_DEVICE(_devtag, _class, _func) \
	device_execute_interface::static_set_irq_acknowledge_callback(*device, device_irq_acknowledge_delegate(&_class::_func, #_class "::" #_func, _devtag, (_class *)0));
#define MCFG_DEVICE_IRQ_ACKNOWLEDGE_REMOVE()  \
	device_execute_interface::static_set_irq_acknowledge_callback(*device, device_irq_acknowledge_delegate());


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class emu_timer;
class screen_device;
class device_scheduler;


// interrupt callback for VBLANK and timed interrupts
typedef device_delegate<void (device_t &)> device_interrupt_delegate;

// IRQ callback to be called by executing devices when an IRQ is actually taken
typedef device_delegate<int (device_t &, int)> device_irq_acknowledge_delegate;



// ======================> device_execute_interface

class device_execute_interface : public device_interface
{
	friend class device_scheduler;
	friend class testcpu_state;

public:
	// construction/destruction
	device_execute_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_execute_interface();

	// configuration access
	bool disabled() const { return m_disabled; }
	UINT64 clocks_to_cycles(UINT64 clocks) const { return execute_clocks_to_cycles(clocks); }
	UINT64 cycles_to_clocks(UINT64 cycles) const { return execute_cycles_to_clocks(cycles); }
	UINT32 min_cycles() const { return execute_min_cycles(); }
	UINT32 max_cycles() const { return execute_max_cycles(); }
	attotime cycles_to_attotime(UINT64 cycles) const { return device().clocks_to_attotime(cycles_to_clocks(cycles)); }
	UINT64 attotime_to_cycles(const attotime &duration) const { return clocks_to_cycles(device().attotime_to_clocks(duration)); }
	UINT32 input_lines() const { return execute_input_lines(); }
	UINT32 default_irq_vector() const { return execute_default_irq_vector(); }

	// static inline configuration helpers
	static void static_set_disable(device_t &device);
	static void static_set_vblank_int(device_t &device, device_interrupt_delegate function, const char *tag, int rate = 0);
	static void static_set_periodic_int(device_t &device, device_interrupt_delegate function, const attotime &rate);
	static void static_set_irq_acknowledge_callback(device_t &device, device_irq_acknowledge_delegate callback);

	// execution management
	device_scheduler &scheduler() const { assert(m_scheduler != nullptr); return *m_scheduler; }
	bool executing() const;
	INT32 cycles_remaining() const;
	void eat_cycles(int cycles);
	void adjust_icount(int delta);
	void abort_timeslice();

	// input and interrupt management
	void set_input_line(int linenum, int state) { m_input[linenum].set_state_synced(state); }
	void set_input_line_vector(int linenum, int vector) { m_input[linenum].set_vector(vector); }
	void set_input_line_and_vector(int linenum, int state, int vector) { m_input[linenum].set_state_synced(state, vector); }
	int input_state(int linenum) const { return m_input[linenum].m_curstate; }

	// suspend/resume
	void suspend(UINT32 reason, bool eatcycles);
	void resume(UINT32 reason);
	bool suspended(UINT32 reason = SUSPEND_ANY_REASON) const { return (m_nextsuspend & reason) != 0; }
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
	attotime local_time() const;
	UINT64 total_cycles() const;

	// required operation overrides
	void run() { execute_run(); }

	// deliberately ambiguous functions; if you have the execute interface
	// just use it
	device_execute_interface &execute() { return *this; }

protected:
	// clock and cycle information getters
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const;
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const;
	virtual UINT32 execute_min_cycles() const;
	virtual UINT32 execute_max_cycles() const;

	// input line information getters
	virtual UINT32 execute_input_lines() const;
	virtual UINT32 execute_default_irq_vector() const;

	// optional operation overrides
	virtual void execute_run() = 0;
	virtual void execute_burn(INT32 cycles);
	virtual void execute_set_input(int linenum, int state);

	// interface-level overrides
	virtual void interface_validity_check(validity_checker &valid) const override;
	virtual void interface_pre_start() override;
	virtual void interface_post_start() override;
	virtual void interface_pre_reset() override;
	virtual void interface_post_reset() override;
	virtual void interface_clock_changed() override;

	// for use by devcpu for now...
	IRQ_CALLBACK_MEMBER(standard_irq_callback_member);
	int standard_irq_callback(int irqline);

	// internal information about the state of inputs
	class device_input
	{
		static const int USE_STORED_VECTOR = 0xff000000;

	public:
		device_input();

		void start(device_execute_interface *execute, int linenum);
		void reset();

		void set_state_synced(int state, int vector = USE_STORED_VECTOR);
		void set_vector(int vector) { m_stored_vector = vector; }
		int default_irq_callback();

		device_execute_interface *m_execute;// pointer to the execute interface
		int             m_linenum;          // which input line we are

		INT32           m_stored_vector;    // most recently written vector
		INT32           m_curvector;        // most recently processed vector
		UINT8           m_curstate;         // most recently processed state
		INT32           m_queue[32];        // queue of pending events
		int             m_qindex;           // index within the queue

	private:
		TIMER_CALLBACK_MEMBER(empty_event_queue);
	};

	// scheduler
	device_scheduler *      m_scheduler;                // pointer to the machine scheduler

	// configuration
	bool                    m_disabled;                 // disabled from executing?
	device_interrupt_delegate m_vblank_interrupt;       // for interrupts tied to VBLANK
	const char *            m_vblank_interrupt_screen;  // the screen that causes the VBLANK interrupt
	device_interrupt_delegate m_timed_interrupt;        // for interrupts not tied to VBLANK
	attotime                m_timed_interrupt_period;   // period for periodic interrupts

	// execution lists
	device_execute_interface *m_nextexec;               // pointer to the next device to execute, in order

	// input states and IRQ callbacks
	device_irq_acknowledge_delegate m_driver_irq;       // driver-specific IRQ callback
	device_input            m_input[MAX_INPUT_LINES];   // data about inputs
	emu_timer *             m_timedint_timer;           // reference to this device's periodic interrupt timer

	// cycle counting and executing
	profile_type            m_profiler;                 // profiler tag
	int *                   m_icountptr;                // pointer to the icount
	int                     m_cycles_running;           // number of cycles we are executing
	int                     m_cycles_stolen;            // number of cycles we artificially stole

	// suspend states
	UINT32                  m_suspend;                  // suspend reason mask (0 = not suspended)
	UINT32                  m_nextsuspend;              // pending suspend reason mask
	UINT8                   m_eatcycles;                // true if we eat cycles while suspended
	UINT8                   m_nexteatcycles;            // pending value
	INT32                   m_trigger;                  // pending trigger to release a trigger suspension
	INT32                   m_inttrigger;               // interrupt trigger index

	// clock and timing information
	UINT64                  m_totalcycles;              // total device cycles executed
	attotime                m_localtime;                // local time, relative to the timer system's global time
	INT32                   m_divisor;                  // 32-bit attoseconds_per_cycle divisor
	UINT8                   m_divshift;                 // right shift amount to fit the divisor into 32 bits
	UINT32                  m_cycles_per_second;        // cycles per second, adjusted for multipliers
	attoseconds_t           m_attoseconds_per_cycle;    // attoseconds per adjusted clock cycle

private:
	// callbacks
	TIMER_CALLBACK_MEMBER(timed_trigger_callback);

	void on_vblank(screen_device &screen, bool vblank_state);

	TIMER_CALLBACK_MEMBER(trigger_periodic_interrupt);
	void suspend_resume_changed();

	attoseconds_t minimum_quantum() const;
};

// iterator
typedef device_interface_iterator<device_execute_interface> execute_interface_iterator;


#endif  /* __DIEXEC_H__ */
