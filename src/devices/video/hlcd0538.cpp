// license:BSD-3-Clause
// copyright-holders:hap
/*

Hughes HLCD 0538(A)/0539(A)/0607(A) LCD Driver

0538: 8 rows, 26 columns
0539: 0 rows, 34 columns
0607: 4 rows, 30 columns

"LCD" pin can be used in 2 modes, either direct drive, or as an oscillator.
In latter case, output frequency is approximately 1/RC.

TODO:
- the only difference between 0538/0607 and 0539 is row pins voltage levels?

*/

#include "emu.h"
#include "hlcd0538.h"


DEFINE_DEVICE_TYPE(HLCD0538, hlcd0538_device, "hlcd0538", "Hughes HLCD 0538 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0539, hlcd0539_device, "hlcd0539", "Hughes HLCD 0539 LCD Driver")
DEFINE_DEVICE_TYPE(HLCD0607, hlcd0607_device, "hlcd0607", "Hughes HLCD 0607 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0538_device::hlcd0538_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_write_cols(*this), m_write_interrupt(*this)
{ }

hlcd0538_device::hlcd0538_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0538_device(mconfig, HLCD0538, tag, owner, clock)
{ }

hlcd0539_device::hlcd0539_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0538_device(mconfig, HLCD0539, tag, owner, clock)
{ }

hlcd0607_device::hlcd0607_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	hlcd0538_device(mconfig, HLCD0607, tag, owner, clock)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0538_device::device_start()
{
	// timer (when LCD pin is oscillator)
	m_lcd_timer = timer_alloc(FUNC(hlcd0538_device::toggle_lcd), this);
	attotime period = (clock() != 0) ? attotime::from_hz(2 * clock()) : attotime::never;
	m_lcd_timer->adjust(period, 0, period);

	// zerofill
	m_lcd = 0;
	m_clk = 0;
	m_data = 0;
	m_shift = 0;

	// register for savestates
	save_item(NAME(m_lcd));
	save_item(NAME(m_clk));
	save_item(NAME(m_data));
	save_item(NAME(m_shift));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(hlcd0538_device::toggle_lcd)
{
	lcd_w(!m_lcd);
}

void hlcd0538_device::clk_w(int state)
{
	state = (state) ? 1 : 0;

	// clock in data on falling edge
	if (!state && m_clk)
		m_shift = (m_shift << 1 | m_data) & u64(0x3ffffffff);

	m_clk = state;
}

void hlcd0538_device::lcd_w(int state)
{
	state = (state) ? 1 : 0;

	// transfer to latches on rising edge
	if (state && !m_lcd)
	{
		m_write_cols(0, m_shift);
		m_shift = 0;
	}

	m_lcd = state;

	// interrupt output follows lcd input
	m_write_interrupt(state);
}
