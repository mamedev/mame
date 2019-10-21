// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

    MB84x2 lacks interrupt pins, it's basically as simple as ram().share("x")
    MB843x is same as MB842x, except that it supports slave mode for 16-bit or
    32-bit expansion. It makes sure there are no clashes with the _BUSY pin.

    IDT71321 is function compatible, but not pin compatible with MB8421

**********************************************************************/

#include "emu.h"
#include "machine/mb8421.h"


DEFINE_DEVICE_TYPE(MB8421, mb8421_device, "mb8421", "MB8421 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(IDT71321, idt71321_device, "idt71321", "IDT71321 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device, "mb8421_mb8431_16", "MB8421/MB8431 16-bit Dual-Port SRAM with Interrupts")

//-------------------------------------------------
//  mb8421_master_device - constructor
//-------------------------------------------------

mb8421_master_device::mb8421_master_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_intl_callback(*this)
	, m_intr_callback(*this)
{
}

//-------------------------------------------------
//  mb8421_device - constructor
//-------------------------------------------------

mb8421_device::mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb8421_master_device(mconfig, MB8421, tag, owner, clock)
{
}

mb8421_device::mb8421_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: mb8421_master_device(mconfig, type, tag, owner, clock)
{
}

//-------------------------------------------------
//  idt71321_device - constructor
//-------------------------------------------------

idt71321_device::idt71321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: mb8421_device(mconfig, IDT71321, tag, owner, clock)
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
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mb8421_master_device::device_resolve_objects()
{
	// resolve callbacks
	m_intl_callback.resolve_safe();
	m_intr_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb8421_device::device_start()
{
	m_ram = make_unique_clear<u8[]>(0x800);

	// state save
	save_pointer(NAME(m_ram), 0x800);
}

void mb8421_mb8431_16_device::device_start()
{
	m_ram = make_unique_clear<u16[]>(0x800);

	// state save
	save_pointer(NAME(m_ram), 0x800);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb8421_master_device::device_reset()
{
	m_intl_callback(CLEAR_LINE);
	m_intr_callback(CLEAR_LINE);
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
		(is_right ? m_intl_callback : m_intr_callback)(ASSERT_LINE);
	else if (row == read_or_write::READ && offset == (is_right ? 0x7ff : 0x7fe))
		(is_right ? m_intr_callback : m_intl_callback)(CLEAR_LINE);
}

//-------------------------------------------------
//  left_w - write access for left-side bus
//  (write to 7FF asserts INTR)
//-------------------------------------------------

void mb8421_device::left_w(offs_t offset, u8 data)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, false>(offset);
}

void mb8421_mb8431_16_device::left_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= 0x7ff;
	COMBINE_DATA(&m_ram[offset]);
	update_intr<read_or_write::WRITE, false>(offset);
}

//-------------------------------------------------
//  left_r - read access for left-side bus
//  (read from 7FE acknowledges INTL)
//-------------------------------------------------

u8 mb8421_device::left_r(offs_t offset)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

u16 mb8421_mb8431_16_device::left_r(offs_t offset, u16 mem_mask)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

//-------------------------------------------------
//  right_w - write access for right-side bus
//  (write to 7FE asserts INTL)
//-------------------------------------------------

void mb8421_device::right_w(offs_t offset, u8 data)
{
	offset &= 0x7ff;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, true>(offset);
}

void mb8421_mb8431_16_device::right_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= 0x7ff;
	COMBINE_DATA(&m_ram[offset]);
	update_intr<read_or_write::WRITE, true>(offset);
}

//-------------------------------------------------
//  right_r - read access for right-side bus
//  (read from 7FF acknowledges INTR)
//-------------------------------------------------

u8 mb8421_device::right_r(offs_t offset)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}

u16 mb8421_mb8431_16_device::right_r(offs_t offset, u16 mem_mask)
{
	offset &= 0x7ff;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}
