// license:BSD-3-Clause
// copyright-holders:AJR,m1macrophage
/***************************************************************************

    Roland MB63H114 Multiple Address Counter

    The multiplexed outputs of eight 13-bit counters internal to this
    64-pin QFP CMOS gate array are used to play percussion samples.

    The on-chip clock generator must be externally strapped by connecting
    SCO1 (36) to CLK0 (37) to obtain the standard 1.6 MHz master clock.
    This is divided to produce 100, 50, 25 and 12.5 kHz outputs on A (3),
    B (5), C (7) and D (4). These outputs are normally connected to the
    XCK0–XCK7 inputs (56–57, 59–64), upper address lines and chip selects
    of sample mask ROMs, and 40H151 multiplexers and demultiplexers, but
    may also be used to drive other circuits.

    The XST0–XST7 (38–41, 44–47) counter start inputs, strobed with XSTA
    (50), are normally connected to CPU address outputs rather than the
    data bus, perhaps due to long setup/hold requirements. All these are
    specified as active low.

***************************************************************************/

#include "emu.h"
#include "mb63h114.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB63H114, mb63h114_device, "mb63h114", "Roland MB63H114 Multiple Address Counter")

//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  mb63h114_device - constructor
//-------------------------------------------------

mb63h114_device::mb63h114_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB63H114, tag, owner, clock)
	, m_counter_func(*this)
	, m_timer(nullptr)
	, m_xst(0xff)
	, m_xck(0)
	, m_active_counter(0)
	, m_d(0)
{
	std::fill(m_counters.begin(), m_counters.end(), MAX_COUNT);
}


//-------------------------------------------------
//  timer tick
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(mb63h114_device::timer_tick)
{
	const u8 dcba_output = (m_d << 3) | m_active_counter;
	m_counter_func(dcba_output, m_counters[m_active_counter]);

	const u8 next_output = dcba_output + 1;
	m_active_counter = next_output & 0x07;
	m_d = BIT(next_output, 3);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb63h114_device::device_start()
{
	save_item(NAME(m_xst));
	save_item(NAME(m_xck));
	save_item(NAME(m_active_counter));
	save_item(NAME(m_d));
	save_item(NAME(m_counters));

	m_timer = timer_alloc(FUNC(mb63h114_device::timer_tick), this);
	const attotime period = attotime::from_hz(clock() / 8.0);
	m_timer->adjust(period, 0, period);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb63h114_device::device_reset()
{
	m_xst = 0xff;
	m_xck = 0;
	m_active_counter = 0;
	m_d = 0;
	std::fill(m_counters.begin(), m_counters.end(), MAX_COUNT);
}


//-------------------------------------------------
//  xst_w - write counter start inputs
//-------------------------------------------------

void mb63h114_device::xst_w(u8 data)
{
	for (int i = 0; i < COUNTERS; ++i)
		// Not sure if level- or edge-triggered. Treating as negative edge-triggered.
		if (BIT(m_xst, i) && !BIT(data, i))
			m_counters[i] = 0;
	m_xst = data;
}


//-------------------------------------------------
//  xck_w -  write clock inputs for the counters
//-------------------------------------------------

void mb63h114_device::xck_w(u8 data)
{
	for (int i = 0; i < COUNTERS; ++i)
		if (BIT(m_xck, i) && !BIT(data, i))  // Negative edge-triggered.
			if (m_counters[i] < MAX_COUNT)
				++m_counters[i];
	m_xck = data;
}
