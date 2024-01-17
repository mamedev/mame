// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Banks (with size)
// 0:0240 = bcr1MEL
// 1:0240 = deqc1
// 2:0240 = deqm1
// 3:0240 = dspc1
// 4:0120 = dspa1
// 5:0240 = dspm1
// 6:0240 = ?
// 7:0040 = n1mod0
// 8:0006 = n1creg0
// 9:0020 = n1fgtim0
// a:0040 = n1fguram0

// Registers (with address)
// 1:00e0 = dsp run (0x40000000)
// 7:0040 = n1mod0KeyOnOff (0/0xffff)
// f:c100 = TRWF
// f:c101 = TRWFO

// CLDW/CCK/CDO0 is the three pins of the ym3437c


#include "emu.h"
#include "ymp21.h"

ymp21_device::ymp21_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock),
	  m_tx_cb(*this),
	  m_dsp3_buffer(*this, "dsp3", 0x80 * 5, ENDIANNESS_LITTLE)
{
}

void ymp21_device::device_start()
{
	pci_card_device::device_start();

	add_map(0x40000, M_MEM, FUNC(ymp21_device::map));
	intr_pin = 0x01;

	save_item(NAME(m_dsp3_ctrl1));
	save_item(NAME(m_dsp3_glob1));
	save_item(NAME(m_dsp3_glob2));
	save_item(NAME(m_dsp3_glob3));

	m_rx_timer[0] = timer_alloc(FUNC(ymp21_device::uart_rx), this);
	m_rx_timer[1] = timer_alloc(FUNC(ymp21_device::uart_rx), this);
	m_tx_timer[0] = timer_alloc(FUNC(ymp21_device::uart_tx), this);
	m_tx_timer[1] = timer_alloc(FUNC(ymp21_device::uart_tx), this);
}

void ymp21_device::device_reset()
{
	pci_card_device::device_reset();

	std::fill(m_dsp3_ctrl1.begin(), m_dsp3_ctrl1.end(), 0);
	std::fill(m_dsp3_glob1.begin(), m_dsp3_glob1.end(), 0);
	std::fill(m_dsp3_glob2.begin(), m_dsp3_glob2.end(), 0);
	std::fill(m_dsp3_glob3.begin(), m_dsp3_glob3.end(), 0);
}

void ymp21_device::map(address_map &map)
{
	map(0x3e000, 0x3e000).rw(FUNC(ymp21_device::uart_status_r), FUNC(ymp21_device::uart_ctrl_w)).select(2);
	map(0x3e001, 0x3e001).rw(FUNC(ymp21_device::uart_data_r), FUNC(ymp21_device::uart_data_w)).select(2);

	map(0x3f000, 0x3f07f).w(FUNC(ymp21_device::dsp3_buffer_w)).select(0x00700);
	map(0x3f080, 0x3f083).rw(FUNC(ymp21_device::dsp3_status_r), FUNC(ymp21_device::dsp3_ctrl1_w)).select(0x00700);
	map(0x3f084, 0x3f087).w(FUNC(ymp21_device::dsp3_ctrl2_w)).select(0x00700);
	map(0x3f088, 0x3f08b).w(FUNC(ymp21_device::dsp3_glob1_w)).select(0x00700);
	map(0x3f08c, 0x3f08f).w(FUNC(ymp21_device::dsp3_glob2_w)).select(0x00700);
	map(0x3f090, 0x3f093).w(FUNC(ymp21_device::dsp3_glob3_w)).select(0x00700);

	map(0x3ff00, 0x3ff03).rw(FUNC(ymp21_device::port0_r), FUNC(ymp21_device::port0_w));
	map(0x3ff04, 0x3ff07).rw(FUNC(ymp21_device::status_r), FUNC(ymp21_device::irq_w));
	map(0x3ff10, 0x3ff13).w(FUNC(ymp21_device::port1_w));
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
	logerror("port1_w %08x -%s%s%s%s%s%s%s\n", data,
			 ((data & 0x00030000) == 0x00000000) ? " 48000" : ((data & 0x00030000) == 0x00010000) ? " 44100" : ((data & 0x00030000) == 0x00000000) ? " 32000": "",
			 data & 0x00800000 ? "" : " reset0",
			 data & 0x02000000 ? " mute" : "",
			 data & 0x08000000 ? " CDO0" : "",
			 data & 0x10000000 ? " CLDW" : "",
			 data & 0x20000000 ? " CCK" : "",
			 (data & 0xc0000000) == 0xc0000000 ? " running" : ""
			 );
}

void ymp21_device::irq_w(u32 data)
{
	logerror("irq_w %08x\n", data);
	irq_pin_w(0, data & 0x04000000);
}

u32 ymp21_device::status_r()
{
	machine().debug_break();
	logerror("status_r\n");
	return 0;
}

void ymp21_device::dsp3_buffer_w(offs_t offset, u32 data, u32 mem_mask)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	COMBINE_DATA(&m_dsp3_buffer[(slot << 5) | offset]);
}

u32 ymp21_device::dsp3_status_r(offs_t offset)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return 0;

	// bit 31 = busy

	return 0;
}

void ymp21_device::dsp3_ctrl1_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	m_dsp3_ctrl1[slot] = data;
}

void ymp21_device::dsp3_ctrl2_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	logerror("dsp3 upload %d %x:%08x %02x\n", slot, (data >> 8) & 0xff, (m_dsp3_ctrl1[slot] << 16) | (data >> 16), m_dsp3_ctrl1[slot] >> 16);
}

void ymp21_device::dsp3_glob1_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	m_dsp3_glob1[slot] = data;
}

void ymp21_device::dsp3_glob2_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	m_dsp3_glob2[slot] = data;
}

void ymp21_device::dsp3_glob3_w(offs_t offset, u32 data)
{
	offs_t slot = offset >> 6;
	if(slot >= 5)
		return;

	m_dsp3_glob3[slot] = data;

	logerror("dsp3 %d globals: %04x %04x %08x %04x\n",
			 slot,
			 m_dsp3_glob1[slot] >> 16,
			 m_dsp3_glob1[slot] & 0xffff,
			 m_dsp3_glob2[slot],
			 m_dsp3_glob3[slot] >> 16);
}

TIMER_CALLBACK_MEMBER(ymp21_device::uart_rx)
{
}

TIMER_CALLBACK_MEMBER(ymp21_device::uart_tx)
{
}

