// license:BSD-3-Clause
// copyright-holders:hap
/*

  National Semiconductor MM5445 series VFD Driver

  TODO:
  - brightness control input pin (sets output voltage level)

*/

#include "emu.h"
#include "video/mm5445.h"


DEFINE_DEVICE_TYPE(MM5445, mm5445_device, "mm5445", "MM5445 VFD Driver")
DEFINE_DEVICE_TYPE(MM5446, mm5446_device, "mm5446", "MM5446 VFD Driver")
DEFINE_DEVICE_TYPE(MM5447, mm5447_device, "mm5447", "MM5447 VFD Driver")
DEFINE_DEVICE_TYPE(MM5448, mm5448_device, "mm5448", "MM5448 VFD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

mm5445_device::mm5445_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 outpins) :
	device_t(mconfig, type, tag, owner, clock),
	m_outmask((u64(1) << outpins) - 1),
	m_write_output(*this)
{ }

mm5445_device::mm5445_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm5445_device(mconfig, MM5445, tag, owner, clock, 33)
{ }

mm5446_device::mm5446_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm5445_device(mconfig, MM5446, tag, owner, clock, 34)
{ }

mm5447_device::mm5447_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm5445_device(mconfig, MM5447, tag, owner, clock, 34)
{ }

mm5448_device::mm5448_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	mm5445_device(mconfig, MM5448, tag, owner, clock, 35)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mm5445_device::device_start()
{
	// resolve callbacks
	m_write_output.resolve_safe();

	// zerofill
	m_clk = 0;
	m_enable = 0;
	m_data = 0;
	m_shiftreg = 0;
	m_shiftcount = 0;

	// register for savestates
	save_item(NAME(m_clk));
	save_item(NAME(m_enable));
	save_item(NAME(m_data));
	save_item(NAME(m_shiftreg));
	save_item(NAME(m_shiftcount));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

WRITE_LINE_MEMBER(mm5445_device::clock_w)
{
	state = (state) ? 1 : 0;
	bool rise = state && !m_clk;
	m_clk = state;

	// clock on rising edge
	if (rise)
	{
		u64 data_in = u64(m_data & ~m_enable) << 34;
		u64 lead_in = ~m_shiftreg & data_in;
		m_shiftreg = (m_shiftreg & ((u64(1) << 34) - 1)) | data_in;

		// leading 1 triggers shift start
		if (m_shiftcount == 0 && !lead_in)
			return;

		// output on 35th clock
		if (m_shiftcount == 35)
		{
			m_shiftcount = 0;
			m_write_output(0, m_shiftreg & m_outmask, ~u64(0));
		}
		else
		{
			m_shiftreg >>= 1;
			m_shiftcount++;
		}
	}
}
