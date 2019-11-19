// license:BSD-3-Clause
// copyright-holders:hap,AJR
/**********************************************************************

    Dual port RAM with Mailbox emulation

    Fujitsu MB8421/22/31/32-90/-90L/-90LL/-12/-12L/-12LL
    CMOS 16K-bit (2KB) dual-port SRAM

    MB84x2 lacks interrupt pins, it's basically as simple as ram().share("x")
    MB843x is same as MB842x, except that it supports slave mode for 16-bit or
    32-bit expansion. It makes sure there are no clashes with the _BUSY pin.

    IDT71321 is function compatible, but not pin compatible with MB8421
    IDT7130 is 1KB variation of IDT71321
	CY7C131 is similar as IDT7130

**********************************************************************/

#include "emu.h"
#include "machine/mb8421.h"


DEFINE_DEVICE_TYPE(MB8421, mb8421_device, "mb8421", "MB8421 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(IDT7130, idt7130_device, "idt7130", "IDT7130 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(CY7C131, cy7c131_device, "cy7c131", "CY7C131 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(IDT71321, idt71321_device, "idt71321", "IDT71321 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device, "mb8421_mb8431_16", "MB8421/MB8431 16-bit Dual-Port SRAM with Interrupts")

//-------------------------------------------------
//  dual_port_mailbox_ram_base - constructor
//-------------------------------------------------

dual_port_mailbox_ram_base::dual_port_mailbox_ram_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, size_t ram_size, u8 data_bits)
	: device_t(mconfig, type, tag, owner, clock)
	, m_data_mask((1 << data_bits) - 1)
	, m_ram_size(ram_size)
	, m_ram_mask(m_ram_size - 1)
	, m_int_addr_left(m_ram_mask - 1) // max RAM word size - 2
	, m_int_addr_right(m_ram_mask) // max RAM word size - 1
	, m_intl_callback(*this)
	, m_intr_callback(*this)
{
}

//-------------------------------------------------
//  dual_port_mailbox_ram_8bit_base - constructor
//-------------------------------------------------

dual_port_mailbox_ram_8bit_base::dual_port_mailbox_ram_8bit_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, size_t ram_size, u8 data_bits)
	: dual_port_mailbox_ram_base(mconfig, type, tag, owner, clock, ram_size, data_bits)
{
}

//-------------------------------------------------
//  dual_port_mailbox_ram_16bit_base - constructor
//-------------------------------------------------

dual_port_mailbox_ram_16bit_base::dual_port_mailbox_ram_16bit_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, size_t ram_size, u8 data_bits)
	: dual_port_mailbox_ram_base(mconfig, type, tag, owner, clock, ram_size >> 1, data_bits)
{
}

//-------------------------------------------------
//  cy7c131_device - constructor
//-------------------------------------------------

cy7c131_device::cy7c131_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_8bit_base(mconfig, CY7C131, tag, owner, clock, 0x400)
{
}

//-------------------------------------------------
//  idt7130_device - constructor
//-------------------------------------------------

idt7130_device::idt7130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_8bit_base(mconfig, IDT7130, tag, owner, clock, 0x400)
{
}

//-------------------------------------------------
//  mb8421_device - constructor
//-------------------------------------------------

mb8421_device::mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_8bit_base(mconfig, MB8421, tag, owner, clock, 0x800)
{
}

//-------------------------------------------------
//  idt71321_device - constructor
//-------------------------------------------------

idt71321_device::idt71321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_8bit_base(mconfig, IDT71321, tag, owner, clock, 0x800)
{
}

//-------------------------------------------------
//  mb8421_mb8431_16_device - constructor
//-------------------------------------------------

mb8421_mb8431_16_device::mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_16bit_base(mconfig, MB8421_MB8431_16BIT, tag, owner, clock, 0x1000)
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void dual_port_mailbox_ram_base::device_resolve_objects()
{
	// resolve callbacks
	m_intl_callback.resolve_safe();
	m_intr_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dual_port_mailbox_ram_8bit_base::device_start()
{
	m_ram = make_unique_clear<u8[]>(m_ram_size);

	// state save
	save_pointer(NAME(m_ram), m_ram_size);
}

void dual_port_mailbox_ram_16bit_base::device_start()
{
	m_ram = make_unique_clear<u16[]>(m_ram_size);

	// state save
	save_pointer(NAME(m_ram), m_ram_size);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dual_port_mailbox_ram_base::device_reset()
{
	m_intl_callback(CLEAR_LINE);
	m_intr_callback(CLEAR_LINE);
}

//-------------------------------------------------
//  update_intr - update interrupt lines upon
//  read or write accesses to special locations
//-------------------------------------------------

template<read_or_write row, bool is_right>
void dual_port_mailbox_ram_base::update_intr(offs_t offset)
{
	if (machine().side_effects_disabled())
		return;

	if (row == read_or_write::WRITE && offset == (is_right ? m_int_addr_left : m_int_addr_right))
		(is_right ? m_intl_callback : m_intr_callback)(ASSERT_LINE);
	else if (row == read_or_write::READ && offset == (is_right ? m_int_addr_right : m_int_addr_left))
		(is_right ? m_intr_callback : m_intl_callback)(CLEAR_LINE);
}

//-------------------------------------------------
//  left_w - write access for left-side bus
//  (write to 7FF asserts INTR)
//-------------------------------------------------

void dual_port_mailbox_ram_8bit_base::left_w(offs_t offset, u8 data)
{
	offset &= m_ram_mask;
	data &= m_data_mask;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, false>(offset);
}

void dual_port_mailbox_ram_16bit_base::left_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= m_ram_mask;
	data &= m_data_mask;
	COMBINE_DATA(&m_ram[offset]);
	update_intr<read_or_write::WRITE, false>(offset);
}

//-------------------------------------------------
//  left_r - read access for left-side bus
//  (read from 7FE acknowledges INTL)
//-------------------------------------------------

u8 dual_port_mailbox_ram_8bit_base::left_r(offs_t offset)
{
	offset &= m_ram_mask;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

u16 dual_port_mailbox_ram_16bit_base::left_r(offs_t offset, u16 mem_mask)
{
	offset &= m_ram_mask;
	update_intr<read_or_write::READ, false>(offset);
	return m_ram[offset];
}

//-------------------------------------------------
//  right_w - write access for right-side bus
//  (write to 7FE asserts INTL)
//-------------------------------------------------

void dual_port_mailbox_ram_8bit_base::right_w(offs_t offset, u8 data)
{
	offset &= m_ram_mask;
	data &= m_data_mask;
	m_ram[offset] = data;
	update_intr<read_or_write::WRITE, true>(offset);
}

void dual_port_mailbox_ram_16bit_base::right_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset &= m_ram_mask;
	data &= m_data_mask;
	COMBINE_DATA(&m_ram[offset]);
	update_intr<read_or_write::WRITE, true>(offset);
}

//-------------------------------------------------
//  right_r - read access for right-side bus
//  (read from 7FF acknowledges INTR)
//-------------------------------------------------

u8 dual_port_mailbox_ram_8bit_base::right_r(offs_t offset)
{
	offset &= m_ram_mask;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}

u16 dual_port_mailbox_ram_16bit_base::right_r(offs_t offset, u16 mem_mask)
{
	offset &= m_ram_mask;
	update_intr<read_or_write::READ, true>(offset);
	return m_ram[offset];
}
