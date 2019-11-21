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

template class dual_port_mailbox_ram_base<u8, 10, 8>;
template class dual_port_mailbox_ram_base<u8, 11, 8>;
template class dual_port_mailbox_ram_base<u16, 11, 16>;

template <typename Type, unsigned AddrBits, unsigned DataBits>
constexpr Type dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::DATA_MASK;
template <typename Type, unsigned AddrBits, unsigned DataBits>
constexpr size_t dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::RAM_SIZE;
template <typename Type, unsigned AddrBits, unsigned DataBits>
constexpr offs_t dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::ADDR_MASK;
template <typename Type, unsigned AddrBits, unsigned DataBits>
constexpr offs_t dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::INT_ADDR_LEFT;
template <typename Type, unsigned AddrBits, unsigned DataBits>
constexpr offs_t dual_port_mailbox_ram_base<Type, AddrBits, DataBits>::INT_ADDR_RIGHT;

// 1Kx8
DEFINE_DEVICE_TYPE(CY7C131,             cy7c131_device,          "cy7c131",          "Cypress CY7C131 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(IDT7130,             idt7130_device,          "idt7130",          "IDT 7130 8-bit Dual-Port SRAM with Interrupts")
// 2Kx8
DEFINE_DEVICE_TYPE(IDT71321,            idt71321_device,         "idt71321",         "IDT 71321 8-bit Dual-Port SRAM with Interrupts")
DEFINE_DEVICE_TYPE(MB8421,              mb8421_device,           "mb8421",           "Fujitsu MB8421 8-bit Dual-Port SRAM with Interrupts")
// 2Kx16
DEFINE_DEVICE_TYPE(MB8421_MB8431_16BIT, mb8421_mb8431_16_device, "mb8421_mb8431_16", "Fujitsu MB8421/MB8431 16-bit Dual-Port SRAM with Interrupts")
