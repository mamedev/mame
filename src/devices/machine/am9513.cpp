// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Am9513A/Am9513 System Timing Controller (STC)

    The Am9513 is a five-channel counter/timer circuit introduced by
    AMD around 1980. (It was also sold as the AmZ8073, apparently due
    to a licensing deal with Zilog to develop Z8000 peripherals. No
    company is known to have second-sourced the device, however.)
    Clock source, edge selection, gating and retriggering are
    programmable for each channel. There is also a frequency divider
    which can take any of the 15 normal counter inputs and divide it
    by any number between 1 and 16.

    All internal counters are 16 bits wide (except the internal 4-bit
    counter for the FOUT divider, which is not externally accessible).
    The device defaults to an 8-bit external interface after being
    powered on or a programmed master reset, but can be configured to
    work more efficiently with an 16-bit data bus.

    There is no reset line, though the device does reset itself when
    it powers on, and there is a "master reset" software command.

    For a full description of each counter mode, see the datasheet.

**********************************************************************/

#include "emu.h"
#include "am9513.h"

#define LOG_MODE  (1U << 1)
#define LOG_INPUT (1U << 2)
#define LOG_TC    (1U << 3)
#define LOG_WARN    (1U << 4)

//#define VERBOSE (LOG_GENERAL | LOG_MODE)

#include "logmacro.h"

#define LOGWARN(...)	LOGMASKED(LOG_WARN, "WARNING: " __VA_ARGS__)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AM9513, am9513_device, "am9513", "Am9513 STC")
DEFINE_DEVICE_TYPE(AM9513A, am9513a_device, "am9513a", "Am9513A STC")


//**************************************************************************
//  DEVICE CONSTRUCTION AND INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  am9513_device - constructor
//-------------------------------------------------

am9513_device::am9513_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, bool is_am9513a)
	: device_t(mconfig, type, tag, owner, clock)
	, m_out_cb(*this)
	, m_fout_cb(*this)
	, m_is_am9513a(is_am9513a)
{
}

am9513_device::am9513_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: am9513_device(mconfig, AM9513, tag, owner, clock, false)
{
}

am9513a_device::am9513a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: am9513_device(mconfig, AM9513A, tag, owner, clock, true)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void am9513_device::device_start()
{
	// Power-on reset
	m_dpr = 0x1f;
	m_mmr = 0;
	m_write_prefetch = true;
	m_status = 0;
	m_f = 0;
	m_fout = true;
	m_fout_counter = 16;
	std::fill(std::begin(m_count), std::end(m_count), 0);
	std::fill(std::begin(m_counter_load), std::end(m_counter_load), 0);
	std::fill(std::begin(m_counter_hold), std::end(m_counter_hold), 0);
	std::fill(std::begin(m_counter_mode), std::end(m_counter_mode), 0x0b00);
	std::fill(std::begin(m_alarm), std::end(m_alarm), 0);
	std::fill(std::begin(m_counter_armed), std::end(m_counter_armed), false);
	std::fill(std::begin(m_counter_running), std::end(m_counter_running), false);
	std::fill(std::begin(m_alternate_count), std::end(m_alternate_count), false);
	std::fill(std::begin(m_tc), std::end(m_tc), false);
	std::fill(std::begin(m_toggle), std::end(m_toggle), false);

	// Unused SRC and GATE inputs are typically grounded
	std::fill(std::begin(m_src), std::end(m_src), false);
	std::fill(std::begin(m_gate), std::end(m_gate), false);
	std::fill(std::begin(m_gate_active), std::end(m_gate_active), false);

	// Alternate gate inputs should be tied high if not used
	std::fill(std::begin(m_gate_alt), std::end(m_gate_alt), true);

	// Set up frequency timers
	for (int f = 0; f < 5; f++)
	{
		m_freq_timer[f] = timer_alloc(FUNC(am9513_device::timer_tick), this);
		m_freq_timer_selected[f] = (f == 0) ? (m_fout_cb.isunset() ? 0x3e : 0x3f) : 0;
		m_freq_timer_cycle[f] = 0;
	}

	// Save device state
	save_item(NAME(m_dpr));
	save_item(NAME(m_mmr));
	save_item(NAME(m_status));
	save_item(NAME(m_write_prefetch));
	save_item(NAME(m_count));
	save_item(NAME(m_counter_load));
	save_item(NAME(m_counter_hold));
	save_item(NAME(m_counter_mode));
	save_item(NAME(m_alarm));
	save_item(NAME(m_counter_armed));
	save_item(NAME(m_counter_running));
	save_item(NAME(m_alternate_count));
	save_item(NAME(m_src));
	save_item(NAME(m_gate));
	save_item(NAME(m_gate_alt));
	save_item(NAME(m_gate_active));
	save_item(NAME(m_tc));
	save_item(NAME(m_toggle));
	save_item(NAME(m_f));
	save_item(NAME(m_freq_timer_selected));
	save_item(NAME(m_freq_timer_cycle));
	save_item(NAME(m_fout));
	save_item(NAME(m_fout_counter));

	// Synchronize clearing of OUT n
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(am9513_device::clear_outputs), this));
}


//-------------------------------------------------
//  clear_outputs - output initial clear state
//  (delayed until all devices have started)
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(am9513_device::clear_outputs)
{
	for (int c = 0; c < 5; c++)
		m_out_cb[c](0);
}


//-------------------------------------------------
//  master_reset - software-controlled reset
//-------------------------------------------------

void am9513_device::master_reset()
{
	LOGMASKED(LOG_MODE, "Master reset\n");

	// Clear master mode register
	set_master_mode(0);

	// Enable prefetch for write
	m_write_prefetch = true;

	// Clear TC state
	std::fill(std::begin(m_tc), std::end(m_tc), false);
	std::fill(std::begin(m_toggle), std::end(m_toggle), false);

	// Initialize counter mode, load and hold registers
	for (int c = 0; c < 5; c++)
	{
		set_counter_mode(c, 0x0b00);
		m_counter_load[c] = 0;
		m_counter_hold[c] = 0;
	}
}


//**************************************************************************
//  FREQUENCY SCALER
//**************************************************************************

//-------------------------------------------------
//  init_freq_timer - set up one of the F1-F5
//  frequency timers
//-------------------------------------------------

void am9513_device::init_freq_timer(int f)
{
	u32 scale = 1;
	for (int n = 0; n < f; n++)
		scale *= BIT(m_mmr, 15) ? 10 : 16;

	attotime freq = clocks_to_attotime(scale);
	if (m_freq_timer_cycle[f] == 0)
		m_freq_timer[f]->adjust(freq, f, freq);
	else
		m_freq_timer[f]->adjust(freq / 2, f, freq / 2);
	m_freq_timer[f]->enable(m_freq_timer_selected[f] != 0);

	LOG("F%d = %f Hz (%s cycle emulation)\n", f + 1, double(clock()) / scale,
			m_freq_timer_selected[f] == 0 ? "no" : m_freq_timer_cycle[f] == 0 ? "partial" : "full");
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way
//-------------------------------------------------

void am9513_device::device_clock_changed()
{
	for (int f = 0; f < 5; f++)
		init_freq_timer(f);
}


//-------------------------------------------------
//  timer_tick - advance our counters
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(am9513_device::timer_tick)
{
	int cycle = m_freq_timer_cycle[param] == 0 ? 2 : 1;
	while (cycle-- > 0)
	{
		m_f ^= 1 << param;
		bool level = BIT(m_f, param);

		int source = param + 11;
		for (int c = 0; c < 5; c++)
		{
			if ((m_counter_mode[c] & 0x0f00) >> 8 == source && BIT(m_counter_mode[c], 12) == !level)
				count_edge(c);
		}

		// FOUT Source = Fn
		if ((m_mmr & 0x00f0) >> 4 == source || ((m_mmr & 0x00f0) == 0 && param == 0))
			fout_tick();
	}
}


//-------------------------------------------------
//  select_freq_timer - keep track of which of the
//  F1-F5 timers are actually in use
//-------------------------------------------------

void am9513_device::select_freq_timer(int f, int c, bool selected, bool cycle)
{
	assert(f >= 0 && f < 5);

	if (selected)
		m_freq_timer_selected[f] |= 1 << c;
	else
		m_freq_timer_selected[f] &= ~(1 << c);

	if (selected && cycle)
		m_freq_timer_cycle[f] |= 1 << c;
	else
		m_freq_timer_cycle[f] &= ~(1 << c);

	init_freq_timer(f);
}


//**************************************************************************
//  MASTER MODE SELECTION
//**************************************************************************

//-------------------------------------------------
//  set_master_mode - master mode control
//-------------------------------------------------

void am9513_device::set_master_mode(u16 data)
{
	u16 old_mmr = m_mmr;
	m_mmr = data;

	// Time-of-Day options
	if ((m_mmr & 0x0003) != (old_mmr & 0x0003))
	{
		switch (m_mmr & 0x0003)
		{
		case 0: LOGMASKED(LOG_MODE, "TOD Disabled\n"); break;
		case 1: LOGMASKED(LOG_MODE, "TOD Enabled (/5 Input)\n"); break;
		case 2: LOGMASKED(LOG_MODE, "TOD Enabled (/6 Input)\n"); break;
		case 3: LOGMASKED(LOG_MODE, "TOD Enabled (/10 Input)\n"); break;
		}
	}

	// Comparator enable
	if (BIT(m_mmr, 2) != BIT(old_mmr, 2))
		LOGMASKED(LOG_MODE, "Compare 1 %s\n", BIT(m_mmr, 2) ? "Enabled" : "Disabled");
	if (BIT(m_mmr, 3) != BIT(old_mmr, 3))
		LOGMASKED(LOG_MODE, "Compare 2 %s\n", BIT(m_mmr, 3) ? "Enabled" : "Disabled");

	// FOUT Source/Divider selection
	if ((m_mmr & 0x0ff0) != (old_mmr & 0x0ff0))
	{
		int source = (m_mmr >> 4) & 15;
		int divider = (m_mmr >> 8) & 15;
		if (source == 0)
			source = 11;
		if (divider == 0)
			divider = 16;

		int old_source = (old_mmr >> 4) & 15;
		if (old_source == 0)
			old_source = 11;
		if (old_source >= 11 && old_source <= 15 && source != old_source)
			select_freq_timer(old_source - 11, 0, false, false);

		if (source >= 11 && source <= 15)
		{
			LOGMASKED(LOG_MODE, "FOUT = F%d / %d\n", source - 10, divider);
			select_freq_timer(source - 11, 0, !m_fout_cb.isunset(), BIT(divider, 0));
		}
		else if (source >= 6 && source <= 10)
			LOGMASKED(LOG_MODE, "FOUT = GATE %d / %d\n", source - 5, divider);
		else
			LOGMASKED(LOG_MODE, "FOUT = SRC %d / %d\n", source, divider);
	}

	// MM12: FOUT gate control
	if (BIT(m_mmr, 12) != BIT(old_mmr, 12))
		LOGMASKED(LOG_MODE, "FOUT: Gate %s\n", BIT(m_mmr, 12) ? "Off" : "On");

	// MM13: Data bus width
	if (BIT(m_mmr, 13) != BIT(old_mmr, 13))
		LOGMASKED(LOG_MODE, "Data Bus Width = %d-Bit\n", BIT(m_mmr, 13) ? 16 : 8);

	// MM14: Data pointer sequencing
	if (BIT(m_mmr, 14) != BIT(old_mmr, 14))
		LOGMASKED(LOG_MODE, "%s Data Pointer Increment\n", BIT(m_mmr, 14) ? "Disable" : "Enable");

	// MM15: Scaler ratio control
	if (BIT(m_mmr, 15) != BIT(old_mmr, 15))
	{
		LOGMASKED(LOG_MODE, "%s Frequency Division\n", BIT(m_mmr, 15) ? "BCD" : "Binary");
		for (int f = 1; f < 5; f++)
			init_freq_timer(f);
	}
}


//**************************************************************************
//  COUNTER CONTROL
//**************************************************************************

//-------------------------------------------------
//  counter_is_mode_x - return true if the counter
//  is configured for the special Mode X
//-------------------------------------------------

bool am9513_device::counter_is_mode_x(int c) const
{
	// Am9513A only: CM7-CM5 = 1, CM15-CM13 = EDGE
	return m_is_am9513a && (m_counter_mode[c] & 0xc0e0) == 0xc0e0;
}


//-------------------------------------------------
//  compare_count - determine comparator output
//  for Counter 1 or Counter 2
//-------------------------------------------------

bool am9513_device::compare_count(int c) const
{
	assert(c == 0 || c == 1);

	// TOD special case: Comparator 2 does 32-bit comparison when Comparator 1 is also activated
	if (c == 1 && BIT(m_mmr, 2) && (m_mmr & 0x0003) != 0)
		return m_count[0] == m_alarm[0] && m_count[1] == m_alarm[1];
	else
		return m_count[c] == m_alarm[c];
}


//-------------------------------------------------
//  set_counter_mode - handle counter mode changes
//-------------------------------------------------

void am9513_device::set_counter_mode(int c, u16 data)
{
	if ((data & 0xe0e0) != (m_counter_mode[c] & 0xe0e0))
	{
		// CM15-CM13, CM7-CM5: Mode selection and gating control
		int mode = ((data >> 5) & 7) * 3;
		switch (data & 0xe000)
		{
		case 0x0000:
			mode += 'A';
			LOGMASKED(LOG_MODE, "Counter %d: Mode %c selected (no gating)\n", c + 1, mode);
			break;

		case 0x2000:
			mode += 'B';
			LOGMASKED(LOG_MODE, "Counter %d: Mode %c selected (active high TC%d)\n", c + 1, mode, c);
			break;

		case 0x4000:
		case 0x6000:
			mode += 'B';
			LOGMASKED(LOG_MODE, "Counter %d: Mode %c selected (active high GATE %d)\n", c + 1, mode, BIT(data, 13) ? c + 2 : c);
			break;

		case 0x8000:
		case 0xa000:
			mode += 'B';
			LOGMASKED(LOG_MODE, "Counter %d: Mode %c selected (active %s GATE %d)\n", c + 1, mode, BIT(data, 13) ? "low" : "high", c + 1);
			break;

		case 0xc000:
		case 0xe000:
			mode += 'C';
			LOGMASKED(LOG_MODE, "Counter %d: Mode %c selected (%s edge GATE %d)\n", c + 1, mode, BIT(data, 13) ? "falling" : "rising", c + 1);
			break;
		}
	}

	if ((data & 0x1f00) != (m_counter_mode[c] & 0x1f00))
	{
		// CM11-CM8: Source selection
		// CM12: Source edge control
		int source = (data >> 8) & 15;
		int old_source = (m_counter_mode[c] >> 8) & 15;
		if (old_source >= 11 && old_source <= 15 && source != old_source)
			select_freq_timer(old_source - 11, c + 1, false, false);
		if (source >= 11 && source <= 15)
		{
			select_freq_timer(source - 11, c + 1, true, BIT(data, 12));
			LOGMASKED(LOG_MODE, "Counter %d: Count on %s edge of F%d\n", c + 1, BIT(data, 12) ? "falling" : "rising", source - 10);
		}
		else if (source >= 6 && source <= 10)
			LOGMASKED(LOG_MODE, "Counter %d: Count on %s edge of GATE %d\n", c + 1, BIT(data, 12) ? "falling" : "rising", source - 5);
		else if (source == 0)
			LOGMASKED(LOG_MODE, "Counter %d: Count on %s edge of TC%d\n", c + 1, BIT(data, 12) ? "falling" : "rising", c);
		else
			LOGMASKED(LOG_MODE, "Counter %d: Count on %s edge of SRC %d\n", c + 1, BIT(data, 12) ? "falling" : "rising", source);
	}

	if ((data & 0x0018) != (m_counter_mode[c] & 0x0018))
		LOGMASKED(LOG_MODE, "Counter %d: %s %s count %s\n", c + 1, BIT(data, 4) ? "BCD" : "Binary", BIT(data, 3) ? "up" : "down", BIT(data, 5) ? "repetitively" : "once");

	if ((data & 0x0007) != (m_counter_mode[c] & 0x0007))
	{
		// CM2-CM0: Output form
		switch (data & 0x0007)
		{
		case 0x0000:
			LOGMASKED(LOG_MODE, "Counter %d: Output low (inactive)\n", c + 1);
			set_output(c, false);
			break;

		case 0x0004:
			LOGMASKED(LOG_MODE, "Counter %d: Output high impedance (inactive)\n", c + 1);
			break;

		case 0x0001:
			if (c < 2 && BIT(m_mmr, c + 2))
				set_output(c, compare_count(c));
			else
			{
				LOGMASKED(LOG_MODE, "Counter %d: Output active high TC pulse\n", c + 1);
				set_output(c, m_tc[c]);
			}
			break;

		case 0x0005:
			if (c < 2 && BIT(m_mmr, c + 2))
				set_output(c, !compare_count(c));
			else
			{
				LOGMASKED(LOG_MODE, "Counter %d: Output active low TC pulse\n", c + 1);
				set_output(c, !m_tc[c]);
			}
			break;

		case 0x0002:
		case 0x0003: // SCP-300F sets this up; why?
			if (c < 2 && BIT(m_mmr, c + 2))
				set_output(c, compare_count(c));
			else
			{
				LOGMASKED(LOG_MODE, "Counter %d: Output toggle on TC\n", c + 1);
				set_output(c, m_toggle[c]);
			}
			break;

		default:
			LOGMASKED(LOG_MODE, "Counter %d: Output mode %d (illegal)\n", c + 1, data & 0x0007);
			break;
		}
	}

	m_counter_mode[c] = data;
}


//-------------------------------------------------
//  arm_counter - arm a particular counter
//-------------------------------------------------

void am9513_device::arm_counter(int c)
{
	if (!m_counter_armed[c])
	{
		LOG("Counter %d: Arming counter\n", c + 1);
		m_counter_armed[c] = true;

		// Count starts upon first active gate edge after arming in Modes C, F, I, L, O, R, X
		m_counter_running[c] = (m_counter_mode[c] & 0xc000) != 0xc000;
	}
	m_alternate_count[c] = false;
}


//-------------------------------------------------
//  disarm_counter - disarm a particular counter
//-------------------------------------------------

void am9513_device::disarm_counter(int c)
{
	if (m_counter_armed[c])
	{
		LOG("Counter %d: Disarming counter\n", c + 1);
		m_counter_armed[c] = false;
		m_counter_running[c] = false;
	}
}


//-------------------------------------------------
//  save_counter - capture the current count
//-------------------------------------------------

void am9513_device::save_counter(int c)
{
	m_counter_hold[c] = m_count[c];
	LOG("Counter %d: Count %u saved\n", c + 1, m_count[c]);
}


//**************************************************************************
//  OUTPUT CONTROL
//**************************************************************************


//-------------------------------------------------
//  set_output - set one of the 5 main outputs
//-------------------------------------------------

void am9513_device::set_output(int c, bool state)
{
	// SR1-SR5 track the output state
	if (BIT(m_status, c + 1) == state)
		return;

	if (state)
		m_status |= 1 << (c + 1);
	else
		m_status &= ~(1 << (c + 1));
	m_out_cb[c](state);
}


//-------------------------------------------------
//  set_toggle - output level control for TC
//  toggle mode
//-------------------------------------------------

void am9513_device::set_toggle(int c, bool state)
{
	m_toggle[c] = state;
	if ((m_counter_mode[c] & 0x0006) == 0x0002)
		set_output(c, state);
}


//-------------------------------------------------
//  set_tc - register terminal count status
//-------------------------------------------------

void am9513_device::set_tc(int c, bool state)
{
	m_tc[c] = state;

	// TC output is disabled when comparator is enabled
	if (c >= 2 || !BIT(m_mmr, 2 + c))
	{
		switch (m_counter_mode[c] & 0x0007)
		{
		case 0x0001:
			// Active high TC pulse
			set_output(c, state);
			break;
		case 0x0002:
		case 0x0003: // SCP-300F sets this up; why?
			// TC toggled output
			if (!state)
				set_toggle(c, !m_toggle[c]);
			break;
		case 0x0005:
			// Active low TC pulse
			set_output(c, !state);
			break;
		}
	}

	// TCn-1 = TC5 for Counter 1
	int d = (c + 1) % 5;

	// TC cascading
	if ((m_counter_mode[d] & 0x1f00) == (state ? 0x0000 : 0x1000))
	{
		LOGMASKED(LOG_TC, "Counter %d: TC cascade Next Count %u \n", c + 1, m_count[d]);
	
		count_edge(d);

	}
	
	// TC gating
	if ((m_counter_mode[d] & 0xe000) == 0x2000)
		gate_count(d, state && (bus_is_16_bit() || m_gate_alt[d]));
}


//**************************************************************************
//  SOURCE INPUTS
//**************************************************************************

//-------------------------------------------------
//  write_source - register state changes on SRC
//  input lines
//-------------------------------------------------

void am9513_device::write_source(int s, bool level)
{
	if (level == m_src[s])
		return;

	m_src[s] = level;
	LOGMASKED(LOG_INPUT, "Source %d: %s edge\n", s + 1, level ? "Rising" : "Falling");

	for (int c = 0; c < 5; c++)
	{
		if ((m_counter_mode[c] & 0x0f00) >> 8 == (s + 1) && BIT(m_counter_mode[c], 12) == !level)
			count_edge(c);
	}

	// FOUT Source = SRC n
	if ((m_mmr & 0x00f0) >> 4 == (s + 1))
		fout_tick();
}


//**************************************************************************
//  COUNTER OPERATION
//**************************************************************************

//-------------------------------------------------
//  count_edge - gate and count active edges
//-------------------------------------------------

void am9513_device::count_edge(int c)
{
	// Counting cannot be disabled or gated during TC
	if (!m_tc[c])
	{
		if (!m_counter_running[c])
			return;

		if (!m_gate_active[c])
		{
			// Modes B, E, H, K, N & Q: Count only during active gate level
			int gating = (m_counter_mode[c] >> 13) & 7;
			if (gating >= 1 && gating <= 5)
				return;
		}
	}

	step_counter(c, false);
}


//-------------------------------------------------
//  reload_from_hold - return true if the current
//  reload source of the counter is the hold
//  register rather than the load register
//-------------------------------------------------

bool am9513_device::reload_from_hold(int c) const
{
	if (counter_is_mode_x(c))
		return false;

	// Modes S & V: Reload from hold register when gate is high
	else if ((m_counter_mode[c] & 0x00c0) == 0x00c0)
		return m_gate_active[c];

	// Modes G, H, I, J, K & L: Alternating reload
	else if ((m_counter_mode[c] & 0x00c0) == 0x0040)
		return m_alternate_count[c];

	else
		return false;
}


//-------------------------------------------------
//  step_counter - advance the counter by one step
//  and/or reload it from the specified register
//  N.B. The "load counter" command works by
//  causing an internal step and so can affect TC
//-------------------------------------------------

void am9513_device::step_counter(int c, bool force_load)
{
	if (BIT(m_counter_mode[c], 3))
	{
		// CM3 = 1: Count up
		++m_count[c];

		// CM4 = 1: BCD adjustment
		if (BIT(m_counter_mode[c], 4))
		{
			if ((m_count[c] & 0x000f) >= 0x000a)
				m_count[c] += 0x0006;

			if (c == 0 && (m_mmr & 0x0003) == 0x0001)
			{
				// TOD: 50Hz
				if ((m_count[c] & 0x00f0) >= 0x0050)
					m_count[c] += 0x00b0;
			}
			else if (c == 0 && (m_mmr & 0x0003) == 0x0002)
			{
				// TOD: 60Hz
				if ((m_count[c] & 0x00f0) >= 0x0060)
					m_count[c] += 0x00a0;
			}
			else if (c == 1 && (m_mmr & 0x0003) != 0)
			{
				// TOD: minutes
				if ((m_count[c] & 0x00f0) >= 0x0060)
					m_count[c] += 0x00a0;
			}
			else
			{
				if ((m_count[c] & 0x00f0) >= 0x00a0)
					m_count[c] += 0x0060;
			}

			if ((m_count[c] & 0x0f00) >= 0x0a00)
				m_count[c] += 0x0600;

			if (c == 0 && (m_mmr & 0x0003) != 0)
			{
				// TOD: seconds
				if ((m_count[c] & 0xf000) >= 0x6000)
					m_count[c] += 0xa000;
			}
			else if (c == 1 && (m_mmr & 0x0003) != 0)
			{
				// TOD: hours
				if ((m_count[c] & 0xff00) >= 0x2400)
					m_count[c] += 0xdc00;
			}
			else
			{
				if ((m_count[c] & 0xf000) >= 0xa000)
					m_count[c] += 0x6000;
			}
		}
	}
	else
	{
		// CM3 = 0: Count down
		--m_count[c];

		// CM4 = 1: BCD adjustment
		if (BIT(m_counter_mode[c], 4))
		{
			if ((m_count[c] & 0x000f) >= 0x000a)
				m_count[c] -= 0x0006;

			if (c == 0 && (m_mmr & 0x0003) == 0x0001)
			{
				// TOD: 50Hz
				if ((m_count[c] & 0x00f0) >= 0x0050)
					m_count[c] -= 0x00b0;
			}
			else if (c == 0 && (m_mmr & 0x0003) == 0x0002)
			{
				// TOD: 60Hz
				if ((m_count[c] & 0x00f0) >= 0x0060)
					m_count[c] -= 0x00a0;
			}
			else if (c == 1 && (m_mmr & 0x0003) != 0)
			{
				// TOD: minutes
				if ((m_count[c] & 0x00f0) >= 0x0060)
					m_count[c] -= 0x00a0;
			}
			else
			{
				if ((m_count[c] & 0x00f0) >= 0x00a0)
					m_count[c] -= 0x0060;
			}

			if ((m_count[c] & 0x0f00) >= 0x0a00)
				m_count[c] -= 0x0600;

			if (c == 0 && (m_mmr & 0x0003) != 0)
			{
				// TOD: seconds
				if ((m_count[c] & 0xf000) >= 0x6000)
					m_count[c] -= 0xa000;
			}
			else if (c == 1 && (m_mmr & 0x0003) != 0)
			{
				// TOD: hours
				if ((m_count[c] & 0xff00) >= 0x2400)
					m_count[c] -= 0xdc00;
			}
			else
			{
				if ((m_count[c] & 0xf000) >= 0xa000)
					m_count[c] -= 0x6000;
			}
		}
	}

	if (m_count[c] == 0)
	{
		set_tc(c, true);
		m_count[c] = reload_from_hold(c) ? m_counter_hold[c] : m_counter_load[c];
		m_alternate_count[c] = !m_alternate_count[c];
		LOGMASKED(LOG_TC, "Counter %d: Terminal count (%u reloaded)\n", c + 1, m_count[c]);

		// Modes A, B, C, N & O: disarm counter after counting to TC once
		if ((m_counter_mode[c] & 0x0060) == 0x0000)
			disarm_counter(c);

		// Modes G, H, I & S: disarm counter after counting to TC twice
		if ((m_counter_mode[c] & 0x0060) == 0x0040 && !m_alternate_count[c])
			disarm_counter(c);

		// Modes C, F, O, R & X: retriggering required after first TC
		// Modes I & L: retriggering required after second TC
		if ((m_counter_mode[c] & 0xc000) == 0xc000 && ((m_counter_mode[c] & 0x00c0) != 0x0040 || !m_alternate_count[c]))
			m_counter_running[c] = false;
	}
	else
	{
		// Drive counter out of TC
		if (m_tc[c])
			set_tc(c, false);

		// Load if requested
		if (force_load)
		{
			m_count[c] = reload_from_hold(c) ? m_counter_hold[c] : m_counter_load[c];
			LOG("Counter %d: %u loaded\n", c + 1, m_count[c]);
		}
	}

	// Active comparators
	if (c < 2 && BIT(m_mmr, c + 2))
	{
		switch (m_counter_mode[c] & 0x0007)
		{
		case 0x0001:
		case 0x0002:
			// Active high comparator output
			set_output(c, compare_count(c));
			break;

		case 0x0005:
			// Active low comparator output
			set_output(c, !compare_count(c));
			break;
		}
	}
}


//**************************************************************************
//  HARDWARE GATING
//**************************************************************************

//-------------------------------------------------
//  gate_count - track active gate state for each
//  counter
//-------------------------------------------------

void am9513_device::gate_count(int c, bool state)
{
	// Active low gating when CM15 = CM13 = 1
	if ((m_counter_mode[c] & 0xa000) == 0xa000)
		state = !state;

	if (m_gate_active[c] == state)
		return;

	m_gate_active[c] = state;
	LOGMASKED(LOG_INPUT, "Counter %d: Gate %sactive\n", c + 1, state ? "" : "in");

	// Active gate edge handling for armed counters
	if (state && m_counter_armed[c])
	{
		// Mode X: transfer counter into Hold register but continue counting
		if (counter_is_mode_x(c))
			m_counter_hold[c] = m_count[c];

		// Modes N, O, Q & R: hardware retriggering
		if ((m_counter_mode[c] & 0x00c0) == 0x0080)
		{
			m_counter_hold[c] = m_count[c];
			m_count[c] = m_counter_load[c];
		}

		// Modes C, F, I, L, O, R & X: start count on active gate edge
		if ((m_counter_mode[c] & 0xc000) == 0xc000)
			m_counter_running[c] = true;
	}
}


//-------------------------------------------------
//  write_gate - register state changes on the
//  GATE n inputs
//-------------------------------------------------

void am9513_device::write_gate(int g, bool level)
{
	if (level == m_gate[g])
		return;

	m_gate[g] = level;

	// Check for selection of GATE n as source for any counter
	for (int c = 0; c < 5; c++)
	{
		if ((m_counter_mode[c] & 0x0f00) >> 8 == (g + 6) && BIT(m_counter_mode[c], 12) == !level)
			count_edge(c);
	}

	// Check for selection of GATE n as control for same-numbered counter
	if (BIT(m_counter_mode[g], 15))
		gate_count(g, level && (bus_is_16_bit() || m_gate_alt[g]));

	// Check for selection of GATE n as control for previous counter
	if ((m_counter_mode[(g + 4) % 5] & 0xe000) == 0x4000)
		gate_count((g + 4) % 5, level && (bus_is_16_bit() || m_gate_alt[(g + 4) % 5]));

	// Check for selection of GATE n as control for next counter
	if ((m_counter_mode[(g + 1) % 5] & 0xe000) == 0x6000)
		gate_count((g + 1) % 5, level && (bus_is_16_bit() || m_gate_alt[(g + 1) % 5]));

	// Check for selection of GATE n as source for FOUT
	if ((m_mmr & 0x00f0) >> 4 == (g + 6))
		fout_tick();
}


//-------------------------------------------------
//  write_gate_alt - register state on GATE nA
//  inputs (only available in 8-bit mode)
//-------------------------------------------------

void am9513_device::write_gate_alt(int c, bool level)
{
	if (bus_is_16_bit())
	{
		LOGWARN("Gate %dA written when configured as DB%d\n", c + 1, c + 8);
		return;
	}

	m_gate_alt[c] = level;

	switch (m_counter_mode[c] & 0xe000)
	{
	case 0x2000:
		gate_count(c, level && m_tc[(c + 4) % 5]);
		break;

	case 0x4000:
		gate_count((c + 1) % 5, level && m_gate[(c + 1) % 5]);
		break;

	case 0x6000:
		gate_count((c + 4) % 5, level && m_gate[(c + 4) % 5]);
		break;

	case 0x8000:
	case 0xa000:
	case 0xc000:
	case 0xe000:
		gate_count(c, level && m_gate[c]);
		break;
	}
}


//**************************************************************************
//  DATA PORT MUX
//**************************************************************************

//-------------------------------------------------
//  describe_register - diagnostic helper
//-------------------------------------------------

std::string am9513_device::describe_register() const
{
	switch (m_dpr)
	{
	case 0x17:
		return std::string("Master Mode");
	case 0x1f:
		return std::string("Status");
	case 0x07:
	case 0x0f:
		return string_format("Counter %d Alarm", BIT(m_dpr, 3) ? 2 : 1);
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
		return string_format("Counter %d Mode", m_dpr & 7);
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
		return string_format("Counter %d Load", m_dpr & 7);
	case 0x11: case 0x19:
	case 0x12: case 0x1a:
	case 0x13: case 0x1b:
	case 0x14: case 0x1c:
	case 0x15: case 0x1d:
		return string_format("Counter %d Hold", m_dpr & 7);
	default:
		return string_format("Illegal %02X", m_dpr);
	}
}


//-------------------------------------------------
//  internal_read - read from register selected by
//  the data pointer
//-------------------------------------------------

u16 am9513_device::internal_read() const
{
	switch (m_dpr)
	{
	case 0x17: // Master mode register
		return m_mmr;
	case 0x1f: // Status register
		return m_status | 0xff00;
	case 0x07: // Alarm 1 register
	case 0x0f: // Alarm 2 register
		return m_alarm[BIT(m_dpr, 3)];
	case 0x01: // Counter 1 mode register
	case 0x02: // Counter 2 mode register
	case 0x03: // Counter 3 mode register
	case 0x04: // Counter 4 mode register
	case 0x05: // Counter 5 mode register
		return m_counter_mode[(m_dpr & 7) - 1];
	case 0x09: // Counter 1 load register
	case 0x0a: // Counter 2 load register
	case 0x0b: // Counter 3 load register
	case 0x0c: // Counter 4 load register
	case 0x0d: // Counter 5 load register
		return m_counter_load[(m_dpr & 7) - 1];
	case 0x11: case 0x19: // Counter 1 hold register
	case 0x12: case 0x1a: // Counter 2 hold register
	case 0x13: case 0x1b: // Counter 3 hold register
	case 0x14: case 0x1c: // Counter 4 hold register
	case 0x15: case 0x1d: // Counter 5 hold register
		return m_counter_hold[(m_dpr & 7) - 1];
	default: // Invalid register
		return 0xffff;
	}
}


//-------------------------------------------------
//  internal_read - write to register selected by
//  the data pointer
//-------------------------------------------------

void am9513_device::internal_write(u16 data)
{
	switch (m_dpr)
	{
	case 0x17: // Master mode register
		set_master_mode(data);
		break;
	case 0x1f: // Status register (read only?)
		LOGWARN("Writing %04X to status register\n", data);
		break;
	case 0x07: // Alarm 1 register
	case 0x0f: // Alarm 2 register
		if (m_alarm[BIT(m_dpr, 3)] != data)
			LOG("Counter %d: Alarm = %u\n", BIT(m_dpr, 3) ? 2 : 1, data);
		m_alarm[BIT(m_dpr, 3)] = data;
		break;
	case 0x01: // Counter 1 mode register
	case 0x02: // Counter 2 mode register
	case 0x03: // Counter 3 mode register
	case 0x04: // Counter 4 mode register
	case 0x05: // Counter 5 mode register
		if (m_counter_mode[(m_dpr & 7) - 1] != data)
			LOG("Counter %d: Mode = %04X\n", m_dpr & 7, data);
		set_counter_mode((m_dpr & 7) - 1, data);
		break;
	case 0x09: // Counter 1 load register
	case 0x0a: // Counter 2 load register
	case 0x0b: // Counter 3 load register
	case 0x0c: // Counter 4 load register
	case 0x0d: // Counter 5 load register
		if (m_counter_load[(m_dpr & 7) - 1] != data)
			LOG("Counter %d: Load = %u\n", m_dpr & 7, data);
		m_counter_load[(m_dpr & 7) - 1] = data;
		break;
	case 0x11: case 0x19: // Counter 1 hold register
	case 0x12: case 0x1a: // Counter 2 hold register
	case 0x13: case 0x1b: // Counter 3 hold register
	case 0x14: case 0x1c: // Counter 4 hold register
	case 0x15: case 0x1d: // Counter 5 hold register
		if (m_counter_hold[(m_dpr & 7) - 1] != data)
			LOG("Counter %d: Hold = %u\n", m_dpr & 7, data);
		m_counter_hold[(m_dpr & 7) - 1] = data;
		break;
	default: // Invalid register
		LOGWARN("Writing %04X to register %02X\n", data, m_dpr);
		break;
	}
}


//**************************************************************************
//  PREFETCH LATCH
//**************************************************************************

//-------------------------------------------------
//  advance_dpr - advance the data pointer through
//  one of four cycles after a data read or write
//-------------------------------------------------

void am9513_device::advance_dpr()
{
	if (machine().side_effects_disabled())
		return;

	if (bus_is_16_bit() || !BIT(m_status, 0))
	{
		// Least significant byte (or 16-bit word) transferred next
		m_status |= 0x01;
		if (BIT(m_mmr, 14))
			return;
	}
	else
	{
		// Most significant byte transferred next
		m_status &= 0xfe;
		return;
	}

	// Cycle within group
	switch (m_dpr & 0x18)
	{
	case 0x00: // Mode/Alarm 1 -> Load/Alarm 2
	case 0x08: // Load/Alarm 2 -> Hold/Master Mode
		m_dpr += 0x08;
		return;
	case 0x10: // Hold/Master Mode -> Mode (next counter)/Alarm 1
		m_dpr -= 0x10;
		break;
	case 0x18:
		break;
	}

	// Cycle through groups
	switch (m_dpr & 0x07)
	{
	case 0x05: // Counter 5 -> Counter 1
		m_dpr -= 0x04;
		break;
	case 0x07: // In control group/status cycle
		break;
	default:
		m_dpr += 0x01;
		break;
	}
}


//**************************************************************************
//  CONTROL PORT
//**************************************************************************

//-------------------------------------------------
//  command_write - decode writes to control
//  register
//-------------------------------------------------

void am9513_device::command_write(u8 data)
{
	switch (data & 0xe0)
	{
	case 0x00:
		if ((data & 0x07) == 0x00 || (data & 0x07) == 0x06)
		{
			LOGWARN("Invalid register selected: %02X\n", data);
			break;
		}

		// Load data pointer
		m_dpr = data;

		// Automatic prefetch
		m_status |= 0x01;
		break;

	case 0x20: // Arm
	case 0x40: // Load
	case 0x60: // Load and Arm
		for (int c = 0; c < 5; c++)
		{
			if (BIT(data, c))
			{
				if (BIT(data, 6))
					step_counter(c, true);
				if (BIT(data, 5))
				{
					LOGMASKED(LOG_MODE, "Arm Counter %d\n", c + 1);
					arm_counter(c);
				}
			}
		}
		break;

	case 0x80: // Disarm and Save
	case 0xa0: // Save
	case 0xc0: // Disarm
		for (int c = 0; c < 5; c++)
		{
			if (BIT(data, c))
			{
				if (!BIT(data, 5))
				{
					LOGMASKED(LOG_MODE, "Disarm Counter %d\n", c + 1);
					disarm_counter(c);
				}
				if (!BIT(data, 6))
				{
					LOGMASKED(LOG_MODE, "Save Counter %d\n", c + 1);
					save_counter(c);
				}
			}
		}
		break;

	default:
		switch (data)
		{
		case 0xe0: case 0xe8: // Clear/set MM14 (Enable/Disable data pointer sequencing)
			LOGMASKED(LOG_MODE, "%s Data Pointer Increment\n", BIT(data, 3) ? "Disable" : "Enable");
			m_mmr = ((m_mmr & ~(1 << 14)) | BIT(data, 3) << 14);
			break;
		case 0xe1: case 0xe9: // Clear/set toggle out for counter 1
		case 0xe2: case 0xea: // Clear/set toggle out for counter 2
		case 0xe3: case 0xeb: // Clear/set toggle out for counter 3
		case 0xe4: case 0xec: // Clear/set toggle out for counter 4
		case 0xe5: case 0xed: // Clear/set toggle out for counter 5
			LOGMASKED(LOG_MODE, "Counter %d: %s output\n", data & 7, BIT(data, 3) ? "Set" : "Clear");
			set_toggle((data & 7) - 1, BIT(data, 3));
			break;
		case 0xe6: case 0xee: // Clear/set MM12 (FOUT gate on/FOUT gate off)
			LOGMASKED(LOG_MODE, "FOUT: Gate %s\n", BIT(data, 3) ? "Off" : "On");
			m_mmr = ((m_mmr & ~(1 << 12)) | BIT(data, 3) << 12);
			break;
		case 0xe7: case 0xef: // Clear/set MM13 (8-bit bus/16-bit bus)
			LOGMASKED(LOG_MODE, "Data Bus Width = %d-Bit\n", BIT(data, 3) ? 16 : 8);
			m_mmr = ((m_mmr & ~(1 << 13)) | BIT(data, 3) << 13);
			break;
		case 0xf1: // Step counter 1
		case 0xf2: // Step counter 2
		case 0xf3: // Step counter 3
		case 0xf4: // Step counter 4
		case 0xf5: // Step counter 5
			LOGMASKED(LOG_MODE, "Counter %d: Step\n", data & 7);
			step_counter((data & 7) - 1, false);
			break;
		case 0xff: // Master reset
			master_reset();
			break;
		case 0xf8: case 0xf9: // Enable/disable prefetch for write (Am9513A only)
			if (m_is_am9513a)
			{
				LOGMASKED(LOG_MODE, "Prefetch %s for write operations\n", BIT(data, 0) ? "disabled" : "enabled");
				m_write_prefetch = !BIT(data, 0);
				break;
			}
			[[fallthrough]];
		default:
			LOGWARN("Invalid command: %02X\n", data);
			break;
		}
		break;
	}
}


//-------------------------------------------------
//  status_read - return the status register
//-------------------------------------------------

u8 am9513_device::status_read() const
{
	return m_status;
}


//**************************************************************************
//  DATA BUS MUX
//**************************************************************************

//-------------------------------------------------
//  bus_is_16_bit - determine whether or not the
//  device is configured for a 16-bit bus
//-------------------------------------------------

bool am9513_device::bus_is_16_bit() const
{
	return BIT(m_mmr, 13);
}


//-------------------------------------------------
//  data_read - generic data read access handler
//-------------------------------------------------

u16 am9513_device::data_read()
{
	u16 result = internal_read();
	if (!bus_is_16_bit() && !BIT(m_status, 0))
		result = (result >> 8) | 0xff00;

	// update the data pointer
	advance_dpr();
	return result;
}


//-------------------------------------------------
//  data_write - generic data write access handler
//-------------------------------------------------

void am9513_device::data_write(u16 data)
{
	if (!bus_is_16_bit())
	{
		data &= 0x00ff;
		if (BIT(m_status, 0))
			data |= internal_read() & 0xff00;
		else
			data = (data << 8) | (internal_read() & 0x00ff);
	}
	internal_write(data);

	// update the data pointer
	if (m_write_prefetch)
		advance_dpr();
}


//-------------------------------------------------
//  read8 - 8-bit read access
//-------------------------------------------------

u8 am9513_device::read8(offs_t offset)
{
	if (BIT(offset, 0))
		return status_read();
	else
		return data_read() & 0x00ff;
}


//-------------------------------------------------
//  write8 - 8-bit write access
//-------------------------------------------------

void am9513_device::write8(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		if (data == 0xef)
			LOGWARN("16-bit data bus selected with 8-bit write\n");
		command_write(data);
	}
	else
		data_write(data | 0xff00);
}


//-------------------------------------------------
//  read16 - 16-bit read access
//-------------------------------------------------

u16 am9513_device::read16(offs_t offset)
{
	if (BIT(offset, 0))
		return status_read() | 0xff00;
	else
	{
		if (!bus_is_16_bit())
			LOGWARN("16-bit data read in 8-bit bus mode\n");
		return data_read();
	}
}


//-------------------------------------------------
//  write16 - 16-bit write access
//-------------------------------------------------

void am9513_device::write16(offs_t offset, u16 data)
{
	if ((!bus_is_16_bit() || BIT(offset, 0)) && (data & 0xff00) != 0xff00)
		LOGWARN("Errant write of %02X to upper byte of %s register in %d-bit bus mode\n",
				(data & 0xff00) >> 8,
				BIT(offset, 0) ? "control" : "data",
				bus_is_16_bit() ? 16 : 8);

	if (BIT(offset, 0))
	{
		command_write(data & 0x00ff);

		// NB testing afterwards because this command may have been changing bus width
		if ((data & 0x00ff) == 0x00e7)
			LOGWARN("8-bit data bus selected with 16-bit write\n");
		else if ((data & 0x00ff) == 0x00ef && !bus_is_16_bit())
			LOGWARN("16-bit data bus selected\n");
	}
	else
		data_write(data);
}


//**************************************************************************
//  DIVIDED FREQUENCY OUTPUT
//**************************************************************************

void am9513_device::fout_tick()
{
	--m_fout_counter;
	if (m_fout_counter > 0)
		return;

	// Toggle the output
	m_fout = !m_fout;

	// Check whether the FOUT gate is on
	if (!BIT(m_mmr, 12) && !m_fout_cb.isunset())
		m_fout_cb(m_fout);

	// Reload the counter
	m_fout_counter = (m_mmr >> 8) & 15;
	if (m_fout_counter == 0)
		m_fout_counter = 16;
}
