// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Generic binary ripple counter emulation

    This device emulates basic ripple counter logic ICs with falling-
    edge clocks and a synchronous reset inputs such as CD4040 and
    74LS393.

    The optional 8-bit ROM interface is intended to help stream ROM
    data to sound chips that lack memory interfaces of their own
    (e.g. MSM5205, TMS5110).

**********************************************************************/

#include "emu.h"
#include "machine/ripple_counter.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(RIPPLE_COUNTER, ripple_counter_device, "ripple_counter", "Generic ripple counter")


//**************************************************************************
//  RIPPLE COUNTER DEVICE
//**************************************************************************

//-------------------------------------------------
//  ripple_counter_device - constructor
//-------------------------------------------------

ripple_counter_device::ripple_counter_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RIPPLE_COUNTER, tag, owner, clock),
		device_rom_interface(mconfig, *this),
		m_count_out_cb(*this),
		m_rom_out_cb(*this),
		m_count_timer(nullptr),
		m_count_mask(0),
		m_count(1),
		m_clk(false),
		m_reset(false)
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector ripple_counter_device::memory_space_config() const
{
	if (m_rom_out_cb.isnull())
		return space_config_vector();
	else
		return device_rom_interface::memory_space_config();
}


//-------------------------------------------------
//  device_validity_check - validate a device after
//  the configuration has been constructed
//-------------------------------------------------

void ripple_counter_device::device_validity_check(validity_checker &valid) const
{
	if (m_count_mask == 0)
		osd_printf_error("No counting stages configured\n");
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void ripple_counter_device::device_resolve_objects()
{
	// resolve callbacks
	m_count_out_cb.resolve_safe();
	m_rom_out_cb.resolve();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ripple_counter_device::device_start()
{
	// initialize timers
	m_count_timer = timer_alloc(TIMER_COUNT);

	// register internal state
	save_item(NAME(m_count));
	save_item(NAME(m_clk));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  device_clock_changed - called when the
//  device clock is altered in any way
//-------------------------------------------------

void ripple_counter_device::device_clock_changed()
{
	attotime freq = m_reset ? attotime::never : clocks_to_attotime(1);
	m_count_timer->adjust(freq, 0, freq);
}


//-------------------------------------------------
//  rom_bank_updated - called when the ROM bank
//  is changed
//-------------------------------------------------

void ripple_counter_device::rom_bank_updated()
{
	m_rom_out_cb(read_byte(m_count));
}


//-------------------------------------------------
//  set_count - update the count and associated
//  outputs
//-------------------------------------------------

void ripple_counter_device::set_count(u32 count)
{
	m_count = count;
	m_count_out_cb(count);
	if (!m_rom_out_cb.isnull())
		m_rom_out_cb(read_byte(count));
}


//-------------------------------------------------
//  clock_w - handle falling-edge clock input
//-------------------------------------------------

WRITE_LINE_MEMBER(ripple_counter_device::clock_w)
{
	if (m_clk != bool(state))
	{
		m_clk = bool(state);
		if (!state && !m_reset)
			set_count((m_count + 1) & m_count_mask);
	}
}


//-------------------------------------------------
//  reset_w - handle active-high reset input
//-------------------------------------------------

WRITE_LINE_MEMBER(ripple_counter_device::reset_w)
{
	if (m_reset != bool(state))
	{
		m_reset = bool(state);
		if (state && m_count != 0)
			set_count(0);

		// stop or start the count timer as required
		notify_clock_changed();
	}
}


//-------------------------------------------------
//  device_timer - called whenever a device timer
//  fires
//-------------------------------------------------

void ripple_counter_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_COUNT:
		set_count((m_count + 1) & m_count_mask);
		break;
	}
}
