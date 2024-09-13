// license:BSD-3-Clause
// copyright-holders:hap
/*

Philips PCF2100 family LCD Driver

PCF2100: 2*20 LCD segments
PCF2110: 2*30 LCD segments, S31,S32 are LED outputs
PCF2111: 2*32 LCD segments
PCF2112: 1*32 LCD segments

OSC is R/C, 2100/10/11: 60-100Hz, 2112: 30-50Hz
Actual segment output frequency is divided by number of LCD commons.

*/

#include "emu.h"
#include "pcf2100.h"


DEFINE_DEVICE_TYPE(PCF2100, pcf2100_device, "pcf2100", "Philips PCF2100 LCD Driver")
DEFINE_DEVICE_TYPE(PCF2110, pcf2110_device, "pcf2110", "Philips PCF2110 LCD Driver")
DEFINE_DEVICE_TYPE(PCF2111, pcf2111_device, "pcf2111", "Philips PCF2111 LCD Driver")
DEFINE_DEVICE_TYPE(PCF2112, pcf2112_device, "pcf2112", "Philips PCF2112 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

pcf2100_device::pcf2100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 bpmax, u8 smax) :
	device_t(mconfig, type, tag, owner, clock),
	m_bpmax(bpmax), m_smax(smax), m_write_segs(*this)
{ }

pcf2100_device::pcf2100_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pcf2100_device(mconfig, PCF2100, tag, owner, clock, 2, 20)
{ }

pcf2110_device::pcf2110_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pcf2100_device(mconfig, PCF2110, tag, owner, clock, 2, 32)
{ }

pcf2111_device::pcf2111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pcf2100_device(mconfig, PCF2111, tag, owner, clock, 2, 32)
{ }

pcf2112_device::pcf2112_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	pcf2100_device(mconfig, PCF2112, tag, owner, clock, 1, 32)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pcf2100_device::device_start()
{
	// timer
	m_lcd_timer = timer_alloc(FUNC(pcf2100_device::update_output_latches), this);
	attotime period = attotime::from_hz(clock());
	m_lcd_timer->adjust(period, 0, period);

	// zerofill
	m_shift = 0;
	m_count = 0;
	m_bpout = 0;
	std::fill_n(m_latch, std::size(m_latch), 0);
	m_clb = 0;
	m_data = 0;
	m_dlen = 0;

	// register for savestates
	save_item(NAME(m_shift));
	save_item(NAME(m_count));
	save_item(NAME(m_bpout));
	save_item(NAME(m_latch));
	save_item(NAME(m_clb));
	save_item(NAME(m_data));
	save_item(NAME(m_dlen));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(pcf2100_device::update_output_latches)
{
	// transfer latches to output
	m_write_segs(m_bpout, m_latch[m_bpout]);
	m_bpout = (m_bpout + 1) % m_bpmax;
}

void pcf2100_device::clb_w(int state)
{
	state = (state) ? 1 : 0;
	bool rise = state && !m_clb;
	m_clb = state;

	if (!rise)
		return;

	if (m_dlen)
	{
		// before shifting data, test leading 0 at start
		if ((m_data && m_count == 0) || m_count > (m_smax + 2))
			return;

		m_shift |= u64(m_data) << m_count;
		m_count++;
	}
	else
	{
		if (m_count == (m_smax + 2))
		{
			// transfer to latches
			int dest = BIT(m_shift, m_smax + 1) ? 0 : 1;
			u64 mask = (u64(1) << m_smax) - 1;
			m_latch[dest] = m_shift >> 1 & mask;
		}

		m_shift = 0;
		m_count = 0;
	}
}
