// license:BSD-3-Clause
// copyright-holders:hap
/*

Hughes HLCD 0515 family LCD Driver

0515: 25 columns(also size of buffer/ram)
0569: 24 columns, display blank has no effect(instead it's external with VDRIVE?)
0530: specifications unknown, pinout seems similar to 0569
0601: specifications unknown, pinout seems similar to 0569

TODO:
- Does DATA OUT pin function the same on each chip? The 0515 datasheet says that
  the 25th column is output first, but on 0569(no datasheet available) it's reversed.

*/

#include "emu.h"
#include "video/hlcd0515.h"


DEFINE_DEVICE_TYPE(HLCD0515, hlcd0515_device, "hlcd0515", "Hughes HLCD 0515 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0569, hlcd0569_device, "hlcd0569", "Hughes HLCD 0569 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0530, hlcd0530_device, "hlcd0530", "Hughes HLCD 0530 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0601, hlcd0601_device, "hlcd0601", "Hughes HLCD 0601 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 colmax) :
	device_t(mconfig, type, tag, owner, clock),
	m_colmax(colmax),
	m_write_cols(*this), m_write_data(*this)
{ }

hlcd0515_device::hlcd0515_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0515, tag, owner, clock, 25)
{ }

hlcd0569_device::hlcd0569_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0569, tag, owner, clock, 24)
{ }

hlcd0530_device::hlcd0530_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0530, tag, owner, clock, 24)
{ }

hlcd0601_device::hlcd0601_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0515_device(mconfig, HLCD0601, tag, owner, clock, 24)
{ }



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0515_device::device_start()
{
	// resolve callbacks
	m_write_cols.resolve_safe();
	m_write_data.resolve_safe();

	// timer
	m_lcd_timer = timer_alloc();
	m_lcd_timer->adjust(attotime::from_hz(clock() / 2), 0, attotime::from_hz(clock() / 2));

	// zerofill
	m_cs = 0;
	m_clk = 0;
	m_data = 0;
	m_dataout = 0;
	m_count = 0;
	m_control = 0;
	m_blank = false;
	m_rowmax = 0;
	m_rowout = 0;
	m_rowsel = 0;
	m_buffer = 0;
	memset(m_ram, 0, sizeof(m_ram));

	// register for savestates
	save_item(NAME(m_cs));
	save_item(NAME(m_clk));
	save_item(NAME(m_data));
	save_item(NAME(m_dataout));
	save_item(NAME(m_count));
	save_item(NAME(m_control));
	save_item(NAME(m_blank));
	save_item(NAME(m_rowmax));
	save_item(NAME(m_rowout));
	save_item(NAME(m_rowsel));
	save_item(NAME(m_buffer));
	save_item(NAME(m_ram));
}



//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void hlcd0515_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (m_rowout > m_rowmax)
		m_rowout = 0;

	// write to COL/ROW pins
	m_write_cols(m_rowout, m_blank ? 0 : m_ram[m_rowout], ~0);
	m_rowout++;
}



//-------------------------------------------------
//  handlers
//-------------------------------------------------

void hlcd0515_device::set_control()
{
	// clock 0,1,2: row select
	m_rowsel = m_control >> 2 & 7;

	// clock 3(,4): initialize
	if (m_control & 2)
	{
		m_rowmax = m_rowsel;
		m_blank = bool(~m_control & 1);
	}

	// clock 4: read/write mode
	if (m_control & 1)
	{
		m_buffer = m_ram[m_rowsel];
		clock_data();
	}
	else
		m_buffer = 0;
}

void hlcd0569_device::set_control()
{
	hlcd0515_device::set_control();
	m_blank = false; // 0569 doesn't support display blanking
}


void hlcd0515_device::clock_data(int col)
{
	if (m_control & 1)
	{
		m_dataout = m_buffer & 1;
		m_write_data(m_dataout);

		m_buffer >>= 1;
	}
	else
	{
		if (col < m_colmax)
			m_buffer >>= 1;

		// always write last column
		u32 mask = 1 << (m_colmax - 1);
		m_buffer = (m_buffer & ~mask) | (m_data ? mask : 0);
	}
}


WRITE_LINE_MEMBER(hlcd0515_device::clock_w)
{
	state = (state) ? 1 : 0;

	// clock/shift data on falling edge
	if (!m_cs && !state && m_clk)
	{
		if (m_count < 5)
		{
			// 5-bit mode/control
			m_control = m_control << 1 | m_data;
			if (m_count == 4)
				set_control();
		}

		else
			clock_data(m_count - 5);

		if (m_count < (m_colmax + 5))
			m_count++;
	}

	m_clk = state;
}


WRITE_LINE_MEMBER(hlcd0515_device::cs_w)
{
	state = (state) ? 1 : 0;

	// finish serial sequence on rising edge
	if (state && !m_cs)
	{
		// transfer to ram
		if (~m_control & 1)
			m_ram[m_rowsel] = m_buffer;

		m_count = 0;
		m_control = 0;
	}

	m_cs = state;
}
