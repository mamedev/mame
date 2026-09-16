// license:BSD-3-Clause
// copyright-holders:hap
/*

  Texas Instruments TMS1024/TMS1025 I/O expander

  No documentation was available, just a pinout. But documentation
  is available for pin-compatible Mitsubishi M50780SP/M50781SP.

  Other than more port pins, TMS1025 is assumed to be same as TMS1024.

*/

#include "emu.h"
#include "tms1024.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TMS1024, tms1024_device, "tms1024", "TMS1024 I/O Expander")
DEFINE_DEVICE_TYPE(TMS1025, tms1025_device, "tms1025", "TMS1025 I/O Expander")


//-------------------------------------------------
//  constructor
//-------------------------------------------------

tms1024_device::tms1024_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_h(0), m_s(0), m_std(0), m_ms(0)
	, m_read_port(*this, 0)
	, m_write_port(*this)
{
}

tms1024_device::tms1024_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1024_device(mconfig, TMS1024, tag, owner, clock)
{
}

tms1025_device::tms1025_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tms1024_device(mconfig, TMS1025, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tms1024_device::device_start()
{
	// register for savestates
	save_item(NAME(m_h));
	save_item(NAME(m_s));
	save_item(NAME(m_std));
	save_item(NAME(m_ms));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

void tms1024_device::write_h(u8 data)
{
	// H1,2,3,4: data for outputs A,B,C,D
	m_h = data & 0xf;
}

u8 tms1024_device::read_h()
{
	if (!m_ms)
	{
		// read selected port data
		if (m_s != 0)
		{
			m_h = (m_read_port[m_s-1])((offs_t)(m_s-1)) & 0xf;
			LOG("%s: Read %d%d%d%d from P%d\n", machine().describe_context(), BIT(m_h, 3), BIT(m_h, 2), BIT(m_h, 1), BIT(m_h, 0), m_s);
		}

		// high-impedance otherwise
	}

	return m_h;
}

void tms1024_device::write_s(uint8_t data)
{
	// S0,1,2: select port
	m_s = data & 7;
}

void tms1024_device::write_std(int state)
{
	state = (state) ? 1 : 0;

	// output on falling edge
	if (m_ms && !state && m_std)
	{
		if (m_s != 0)
		{
			LOG("%s: Write %d%d%d%d to P%d\n", machine().describe_context(), BIT(m_h, 3), BIT(m_h, 2), BIT(m_h, 1), BIT(m_h, 0), m_s);
			(m_write_port[m_s-1])((offs_t)(m_s-1), m_h);
		}

		else
		{
			// reset all ports
			LOG("%s: Reset all ports\n", machine().describe_context());
			for (int i = PORT1; i <= PORT7; i++)
				(m_write_port[i])(offs_t(i), 0);
		}
	}

	m_std = state;
}

void tms1024_device::write_ms(int state)
{
	// 0: multiplexer(read) mode, 1: latch(write) mode
	m_ms = (state) ? 1 : 0;
}
