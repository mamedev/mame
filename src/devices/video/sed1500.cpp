// license:BSD-3-Clause
// copyright-holders:hap
/*

Epson SED1500 series LCD Driver
128 bytes internal RAM.

SED1500: 8 commons, 42 segments
SED1501: 10 commons, 40 segments
SED1502: 16 commons, 34 segments
SED1503: 8 commons, 42 segments, needs multiple of 2 chips to function

The default input OSC frequency is 32768Hz, the frame output frequency is
divided by 64 and by number of commons, eg. 64Hz on a SED1500.

TODO:
- bus mode (only mode 3 now)
- EI pin (master/slave mode)
- SYNC pin, used for frame synchronizing if multiple chips are used
- SED1503 only has 8 COM pins, the extra 8 outputs are from the slave chip

*/

#include "emu.h"
#include "video/sed1500.h"


DEFINE_DEVICE_TYPE(SED1500, sed1500_device, "sed1500", "Epson SED1500 LCD Driver")
DEFINE_DEVICE_TYPE(SED1501, sed1501_device, "sed1501", "Epson SED1501 LCD Driver")
DEFINE_DEVICE_TYPE(SED1502, sed1502_device, "sed1502", "Epson SED1502 LCD Driver")
DEFINE_DEVICE_TYPE(SED1503, sed1503_device, "sed1503", "Epson SED1503 LCD Driver")

//-------------------------------------------------
//  constructor
//-------------------------------------------------

sed1500_device::sed1500_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, u8 cmax, u8 smax) :
	device_t(mconfig, type, tag, owner, clock),
	m_cmax(cmax), m_smax(smax),
	m_write_segs(*this)
{ }

sed1500_device::sed1500_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sed1500_device(mconfig, SED1500, tag, owner, clock, 8, 42)
{ }

sed1501_device::sed1501_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sed1500_device(mconfig, SED1501, tag, owner, clock, 10, 40)
{ }

sed1502_device::sed1502_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sed1500_device(mconfig, SED1502, tag, owner, clock, 16, 34)
{ }

sed1503_device::sed1503_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	sed1500_device(mconfig, SED1503, tag, owner, clock, 8+8, 42)
{ }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed1500_device::device_start()
{
	// resolve callbacks
	m_write_segs.resolve_safe();

	// timer
	m_lcd_timer = timer_alloc(FUNC(sed1500_device::update_segs), this);
	attotime period = attotime::from_hz(clock() / 64);
	m_lcd_timer->adjust(period, 0, period);

	// zerofill
	m_mode = 0;
	m_cout = 0;
	std::fill_n(m_ram, std::size(m_ram), 0);

	// register for savestates
	save_item(NAME(m_mode));
	save_item(NAME(m_cout));
	save_item(NAME(m_ram));
}


//-------------------------------------------------
//  handlers
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(sed1500_device::update_segs)
{
	u64 data = 0;

	for (int i = m_smax-1; i >= 0; i--)
		data = data << 1 | BIT(m_ram[i | 0x40] << 8 | m_ram[i], m_cout);

	// transfer segments to output
	m_write_segs(m_cout, data);
	m_cout = (m_cout + 1) % m_cmax;
}

void sed1500_device::write(offs_t offset, u8 data)
{
	offset &= 0x7f;
	m_ram[offset] = data;

	// bus mode command:
	// 0 = 4-bit addr, 4-bit data, combined
	// 1 = 7-bit addr, 4-bit data, separate
	// 2 = 7-bit addr, 8-bit data, combined
	// 3 = 7-bit addr, 8-bit data, separate
	if ((offset & 0x3f) == 0x3f && ~data & 1)
		m_mode = data >> 1 & 3;
}

u8 sed1500_device::read(offs_t offset)
{
	return m_ram[offset & 0x7f];
}
