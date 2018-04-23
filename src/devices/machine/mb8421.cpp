// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

    MB84x2 lacks interrupt pins, it's basically as simple as AM_RAM AM_SHARE("x")
    MB843x is same as MB842x, except that it supports slave mode for 16-bit or
    32-bit expansion. It makes sure there are no clashes with the _BUSY pin.

**********************************************************************/

#include "emu.h"
#include "machine/mb8421.h"


DEFINE_DEVICE_TYPE(MB8421, mb8421_device, "mb8421", "MB8421 8-bit Dual-Port SRAM")
DEFINE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device, "mb8421_mb8431_16", "MB8421/MB8431 16-bit Dual-Port SRAM")

//-------------------------------------------------
//  mb8421_master_device - constructor
//-------------------------------------------------

mb8421_master_device::mb8421_master_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock),
		m_intl_handler(*this),
		m_intr_handler(*this)
{
}

//-------------------------------------------------
//  mb8421_device - constructor
//-------------------------------------------------

mb8421_device::mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb8421_master_device(mconfig, MB8421, tag, owner, clock)
{
}

//-------------------------------------------------
//  mb8421_mb8431_16_device - constructor
//-------------------------------------------------

mb8421_mb8431_16_device::mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb8421_master_device(mconfig, MB8421_MB8431_16BIT, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb8421_master_device::device_start()
{
	// resolve callbacks
	m_intl_handler.resolve_safe();
	m_intr_handler.resolve_safe();
}

void mb8421_device::device_start()
{
	mb8421_master_device::device_start();

	m_ram = make_unique_clear<u8[]>(0x800);

	// state save
	save_pointer(NAME(m_ram.get()), 0x800);
}

void mb8421_mb8431_16_device::device_start()
{
	mb8421_master_device::device_start();

	m_ram = make_unique_clear<u16[]>(0x800);

	// state save
	save_pointer(NAME(m_ram.get()), 0x800);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb8421_master_device::device_reset()
{
	m_intl_handler(0);
	m_intr_handler(0);
}

//-------------------------------------------------
//  update_intr - update interrupt lines upon
//  read or write accesses to special locations
//-------------------------------------------------

template<read_or_write row, bool is_right>
void mb8421_master_device::update_intr(offs_t offset)
{
	if (machine().side_effects_disabled())
		return;

	if (row == read_or_write::WRITE && offset == (is_right ? 0x7fe : 0x7ff))
		(is_right ? m_intl_handler : m_intr_handler)(1);
	else if (row == read_or_write::READ && offset == (is_right ? 0x7ff : 0x7fe))
		(is_right ? m_intr_handler : m_intl_handler)(0);
}

//-------------------------------------------------
//  left_w - write access for left-side bus
//  (write to 7FF asserts INTR)
//-------------------------------------------------

WRITE8_MEMBER(mb8421_device::left_w)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, false>(offset);
}

WRITE16_MEMBER(mb8421_mb8431_16_device::left_w)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, false>(offset);
}

//-------------------------------------------------
//  left_r - read access for left-side bus
//  (read from 7FE acknowledges INTL)
//-------------------------------------------------

READ8_MEMBER(mb8421_device::left_r)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

READ16_MEMBER(mb8421_mb8431_16_device::left_r)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

//-------------------------------------------------
//  right_w - write access for right-side bus
//  (write to 7FE asserts INTL)
//-------------------------------------------------

WRITE8_MEMBER(mb8421_device::right_w)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, true>(offset);
}

WRITE16_MEMBER(mb8421_mb8431_16_device::right_w)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, true>(offset);
}

//-------------------------------------------------
//  right_r - read access for right-side bus
//  (read from 7FF acknowledges INTR)
//-------------------------------------------------

READ8_MEMBER(mb8421_device::right_r)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}

READ16_MEMBER(mb8421_mb8431_16_device::right_r)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}
