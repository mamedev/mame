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

// 1Kx8
DEFINE_DEVICE_TYPE(CY7C131,             cy7c131_device,          "cy7c131",          "Cypress CY7C131 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(IDT7130,             idt7130_device,          "idt7130",          "IDT 7130 8-bit Dual-Port SRAM with Interrupts")
// 2Kx8
DEFINE_DEVICE_TYPE(IDT71321,            idt71321_device,         "idt71321",         "IDT 71321 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(MB8421,              mb8421_device,           "mb8421",           "Fujitsu MB8421 8-bit Dual-Port SRAM with Interrupts")
// 2Kx16
DEFINE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device, "mb8421_mb8431_16", "Fujitsu MB8421/MB8431 16-bit Dual-Port SRAM with Interrupts")

//-------------------------------------------------
//  dual_port_mailbox_ram_base - constructor
//-------------------------------------------------

template <typename Type, unsigned AddrBits, unsigned DataBits>
dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::dual_port_mailbox_ram_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_intl_callback(*this)
	, m_intr_callback(*this)
	, m_ram(nullptr)
{
}

//-------------------------------------------------
//  cy7c131_device - constructor
//-------------------------------------------------

cy7c131_device::cy7c131_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_base<u8, 10, 8>(mconfig, CY7C131, tag, owner, clock) // 1kx8
{
}

//-------------------------------------------------
//  idt7130_device - constructor
//-------------------------------------------------

idt7130_device::idt7130_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_base<u8, 10, 8>(mconfig, IDT7130, tag, owner, clock) // 1kx8
{
}

//-------------------------------------------------
//  idt71321_device - constructor
//-------------------------------------------------

idt71321_device::idt71321_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_base<u8, 11, 8>(mconfig, IDT71321, tag, owner, clock) // 2kx8
{
}

//-------------------------------------------------
//  mb8421_device - constructor
//-------------------------------------------------

mb8421_device::mb8421_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_base<u8, 11, 8>(mconfig, MB8421, tag, owner, clock) // 2kx8
{
}

//-------------------------------------------------
//  mb8421_mb8431_16_device - constructor
//-------------------------------------------------

mb8421_mb8431_16_device::mb8421_mb8431_16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dual_port_mailbox_ram_base<u16, 11, 16>(mconfig, MB8421_MB8431_16BIT, tag, owner, clock) // 2kx16
{
}

//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

template <typename Type, unsigned AddrBits, unsigned DataBits>
void dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::device_resolve_objects()
{
	// resolve callbacks
	m_intl_callback.resolve_safe();
	m_intr_callback.resolve_safe();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

template <typename Type, unsigned AddrBits, unsigned DataBits>
void dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::device_start()
{
	m_ram = make_unique_clear<Type[]>(RAM_SIZE);

	// state save
	save_pointer(NAME(m_ram), RAM_SIZE);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

template <typename Type, unsigned AddrBits, unsigned DataBits>
void dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::device_reset()
{
	m_intl_callback(CLEAR_LINE);
	m_intr_callback(CLEAR_LINE);
}
