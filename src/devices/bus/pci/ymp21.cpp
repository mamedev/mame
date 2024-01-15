// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "ymp21.h"

ymp21_device::ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock),
	  m_mailbox_buffer(*this, "mailbox", 0x80 * 5, ENDIANNESS_LITTLE)
{
}

void ymp21_device::device_start()
{
	pci_card_device::device_start();
	add_map(0x40000, M_MEM, FUNC(ymp21_device::map));
	intr_pin = 0x01;

	save_item(NAME(m_mailbox_size));
}

void ymp21_device::device_reset()
{
	pci_card_device::device_reset();
	std::fill(m_mailbox_size.begin(), m_mailbox_size.end(), 0);
}

void ymp21_device::map(address_map &map)
{
	map(0x3e000, 0x3e000).rw(FUNC(ymp21_device::uart_status_r), FUNC(ymp21_device::uart_ctrl_w)).select(2);
	map(0x3e001, 0x3e001).rw(FUNC(ymp21_device::uart_data_r), FUNC(ymp21_device::uart_data_w)).select(2);

	map(0x3f000, 0x3f07f).w(FUNC(ymp21_device::mailbox_w)).select(0x00700);
	map(0x3f080, 0x3f083).rw(FUNC(ymp21_device::mailbox_size_r), FUNC(ymp21_device::mailbox_size_w)).select(0x00700);
	map(0x3f084, 0x3f087).w(FUNC(ymp21_device::mailbox_address_w)).select(0x00700);

	map(0x3ff00, 0x3ff03).rw(FUNC(ymp21_device::port0_r), FUNC(ymp21_device::port0_w));
	map(0x3ff04, 0x3ff07).rw(FUNC(ymp21_device::status_r), FUNC(ymp21_device::irq_w));
	map(0x3ff10, 0x3ff13).w(FUNC(ymp21_device::port0_w));
}

void ymp21_device::uart_data_w(offs_t offset, u8 data)
{
	logerror("uart_data_w %d, %02x\n", offset, data);
}

u8 ymp21_device::uart_data_r(offs_t offset)
{
	logerror("uart_data_r %d\n", offset);
	return 0;
}

void ymp21_device::uart_ctrl_w(offs_t offset, u8 data)
{
	logerror("uart_ctrl_w %d, %02x\n", offset, data);
}

u8 ymp21_device::uart_status_r(offs_t offset)
{
	logerror("uart_status_r %d\n", offset);
	return 0;
}

void ymp21_device::port0_w(u32 data)
{
	logerror("port0_w %08x\n", data);
}

u32 ymp21_device::port0_r()
{
	logerror("port0_r\n");
	return 0;
}

void ymp21_device::port1_w(u32 data)
{
	logerror("port1_w %08x\n", data);
}

void ymp21_device::irq_w(u32 data)
{
	logerror("irq_w %08x\n", data);
}

u32 ymp21_device::status_r()
{
	logerror("status_r\n");
	return 0;
}

void ymp21_device::mailbox_w(offs_t offset, u32 data, u32 mem_mask)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	COMBINE_DATA(&m_mailbox_buffer[(slot << 5) | offset]);
}

void ymp21_device::mailbox_size_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	m_mailbox_size[slot] = data;
}

u32 ymp21_device::mailbox_size_r(offs_t offset)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return 0;

	return m_mailbox_size[slot] | 0x200000;
}

void ymp21_device::mailbox_address_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	logerror("mailbox upload %d %x.%04x %02x\n", slot, (data >> 8) & 0xff, data >> 16, m_mailbox_size[slot] >> 16);
	m_mailbox_size[slot] = 0;
}
