// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_neptunex.c
 */

#include "emu.h"
#include "x68k_neptunex.h"

#include "machine/dp8390.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X68K_NEPTUNEX, x68k_neptune_device, "x68k_neptunex", "Neptune-X")

// device machine config
void x68k_neptune_device::device_add_mconfig(machine_config &config)
{
	DP8390D(config, m_dp8390, 0);
	m_dp8390->irq_callback().set(FUNC(x68k_neptune_device::x68k_neptune_irq_w));
	m_dp8390->mem_read_callback().set(FUNC(x68k_neptune_device::x68k_neptune_mem_read));
	m_dp8390->mem_write_callback().set(FUNC(x68k_neptune_device::x68k_neptune_mem_write));
}

x68k_neptune_device::x68k_neptune_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X68K_NEPTUNEX, tag, owner, clock)
	, device_x68k_expansion_card_interface(mconfig, *this)
	, m_slot(nullptr)
	, m_dp8390(*this, "dp8390d")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x68k_neptune_device::device_start()
{
	char mac[7];
	uint32_t num = machine().rand();
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	memset(m_prom, 0x57, 16);
	sprintf(mac+2, "\x1b%c%c%c", (num >> 16) & 0xff, (num >> 8) & 0xff, num & 0xff);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp8390->set_mac(mac);
	m_slot->space().install_readwrite_handler(0xece000,0xece3ff, read16_delegate(*this, FUNC(x68k_neptune_device::x68k_neptune_port_r)), write16_delegate(*this, FUNC(x68k_neptune_device::x68k_neptune_port_w)), 0xffffffff);
}

void x68k_neptune_device::device_reset() {
	memcpy(m_prom, m_dp8390->get_mac(), 6);
}

READ16_MEMBER(x68k_neptune_device::x68k_neptune_port_r)
{
	uint16_t data;

	if(offset >= 0x100+32 || offset < 0x100)
		return 0xffff;
	if(offset < 0x100+16)
	{
		m_dp8390->dp8390_cs(CLEAR_LINE);
		return (m_dp8390->dp8390_r(space, offset, 0xff) << 8)|
				m_dp8390->dp8390_r(space, offset+1, 0xff);
	}
	//if(mem_mask == 0x00ff) offset++;
	switch(offset)
	{
	case 0x100+16:
		m_dp8390->dp8390_cs(ASSERT_LINE);
		data = m_dp8390->dp8390_r(space, offset, mem_mask);
		data = ((data & 0x00ff) << 8) | ((data & 0xff00) >> 8);
		return data;
	case 0x100+31:
		m_dp8390->dp8390_reset(CLEAR_LINE);
		return 0;
	default:
		logerror("x68k_neptune: invalid register read %02X\n", offset);
	}
	return 0;
}

WRITE16_MEMBER(x68k_neptune_device::x68k_neptune_port_w)
{
	if(offset >= 0x100+32 || offset < 0x100)
		return;
	if(offset < 0x100+16)
	{
		m_dp8390->dp8390_cs(CLEAR_LINE);
		if(mem_mask == 0x00ff)
		{
			data <<= 8;
			offset++;
		}
		m_dp8390->dp8390_w(space, offset, data>>8, 0xff);
		if(mem_mask == 0xffff) m_dp8390->dp8390_w(space, offset+1, data & 0xff, 0xff);
		return;
	}
	//if(mem_mask == 0x00ff) offset++;
	switch(offset)
	{
	case 0x100+16:
		m_dp8390->dp8390_cs(ASSERT_LINE);
		data = ((data & 0x00ff) << 8) | ((data & 0xff00) >> 8);
		m_dp8390->dp8390_w(space, offset, data, mem_mask);
		return;
	case 0x100+31:
		m_dp8390->dp8390_reset(ASSERT_LINE);
		return;
	default:
		logerror("x68k_neptune: invalid register write %02X\n", offset);
	}
	return;
}

READ8_MEMBER(x68k_neptune_device::x68k_neptune_mem_read)
{
	if(offset < 32) return m_prom[offset>>1];
	if((offset < (16*1024)) || (offset >= (32*1024)))
	{
		logerror("x68k_neptune: invalid memory read %04X\n", offset);
		return 0xff;
	}
	return m_board_ram[offset - (16*1024)];
}

WRITE8_MEMBER(x68k_neptune_device::x68k_neptune_mem_write)
{
	if((offset < (16*1024)) || (offset >= (32*1024)))
	{
		logerror("x68k_neptune: invalid memory write %04X\n", offset);
		return;
	}
	m_board_ram[offset - (16*1024)] = data;
}

WRITE_LINE_MEMBER(x68k_neptune_device::x68k_neptune_irq_w)
{
	m_slot->irq2_w(state);
	logerror("Neptune: IRQ2 set to %i\n",state);
}

uint8_t x68k_neptune_device::iack2()
{
	return NEPTUNE_IRQ_VECTOR;
}
