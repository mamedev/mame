// license:BSD-3-Clause
// copyright-holders:hap
/*

  Hughes HLCD 0538(A)/0539(A) LCD Driver

  0538: 8 rows, 26 columns
  0539: 0 rows, 34 columns

  TODO:
  - the only difference between 0538/0539 is row pins voltage levels?

*/

#include "emu.h"
#include "video/hlcd0538.h"


const device_type HLCD0538 = device_creator<hlcd0538_device>;
const device_type HLCD0539 = device_creator<hlcd0539_device>;

//-------------------------------------------------
//  constructor
//-------------------------------------------------

hlcd0538_device::hlcd0538_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, u32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_write_cols(*this), m_write_interrupt(*this)
{
}

hlcd0538_device::hlcd0538_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: hlcd0538_device(mconfig, HLCD0538, "HLCD 0538 LCD Driver", tag, owner, clock, "hlcd0538", __FILE__)
{
}

hlcd0539_device::hlcd0539_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: hlcd0538_device(mconfig, HLCD0539, "HLCD 0539 LCD Driver", tag, owner, clock, "hlcd0539", __FILE__)
{
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hlcd0538_device::device_start()
{
	// resolve callbacks
	m_write_cols.resolve_safe();
	m_write_interrupt.resolve_safe();

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

WRITE_LINE_MEMBER(hlcd0538_device::write_clk)
{
	state = (state) ? 1 : 0;
	
	// clock in data on falling edge
	if (!state && m_clk)
		m_shift = (m_shift << 1 | m_data) & u64(0x3ffffffff);
	
	m_clk = state;
}

WRITE_LINE_MEMBER(hlcd0538_device::write_lcd)
{
	state = (state) ? 1 : 0;
	
	// transfer to latches on rising edge
	if (state && !m_lcd)
	{
		m_write_cols(0, m_shift, ~u64(0));
		m_shift = 0;
	}

	m_lcd = state;
	
	// interrupt output follows lcd input
	m_write_interrupt(state);
}
