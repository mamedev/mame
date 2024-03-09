// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_neptunex.c
 */

#include "emu.h"
#include "x68k_neptunex.h"

#include "machine/dp8390.h"

#include "multibyte.h"


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
	uint8_t mac[6];
	uint32_t num = machine().rand();
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	memset(m_prom, 0x57, 16);
	mac[2] = 0x1b;
	put_u24be(mac+3, num);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp8390->set_mac(mac);
	m_slot->space().install_readwrite_handler(0xece000,0xece3ff, read16s_delegate(*this, FUNC(x68k_neptune_device::x68k_neptune_port_r)), write16s_delegate(*this, FUNC(x68k_neptune_device::x68k_neptune_port_w)), 0xffffffff);
}

void x68k_neptune_device::device_reset()
{
	memcpy(m_prom, &m_dp8390->get_mac()[0], 6);
}

uint16_t x68k_neptune_device::x68k_neptune_port_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data;

	if(offset >= 0x100+32 || offset < 0x100)
		return 0xffff;
	if(offset < 0x100+16)
	{
		return (m_dp8390->cs_read(offset) << 8)|
				m_dp8390->cs_read(offset+1);
	}
	//if(mem_mask == 0x00ff) offset++;
	switch(offset)
	{
	case 0x100+16:
		data = m_dp8390->remote_read();
		data = swapendian_int16(data);
		return data;
	case 0x100+31:
		m_dp8390->dp8390_reset(CLEAR_LINE);
		return 0;
	default:
		logerror("x68k_neptune: invalid register read %02X\n", offset);
	}
	return 0;
}

void x68k_neptune_device::x68k_neptune_port_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(offset >= 0x100+32 || offset < 0x100)
		return;
	if(offset < 0x100+16)
	{
		if(mem_mask == 0x00ff)
		{
			data <<= 8;
			offset++;
		}
		m_dp8390->cs_write(offset, data>>8);
		if(mem_mask == 0xffff) m_dp8390->cs_write(offset+1, data & 0xff);
		return;
	}
	//if(mem_mask == 0x00ff) offset++;
	switch(offset)
	{
	case 0x100+16:
		m_dp8390->remote_write(swapendian_int16(data));
		return;
	case 0x100+31:
		m_dp8390->dp8390_reset(ASSERT_LINE);
		return;
	default:
		logerror("x68k_neptune: invalid register write %02X\n", offset);
	}
	return;
}

uint8_t x68k_neptune_device::x68k_neptune_mem_read(offs_t offset)
{
	if(offset < 32) return m_prom[offset>>1];
	if((offset < (16*1024)) || (offset >= (32*1024)))
	{
		logerror("x68k_neptune: invalid memory read %04X\n", offset);
		return 0xff;
	}
	return m_board_ram[offset - (16*1024)];
}

void x68k_neptune_device::x68k_neptune_mem_write(offs_t offset, uint8_t data)
{
	if((offset < (16*1024)) || (offset >= (32*1024)))
	{
		logerror("x68k_neptune: invalid memory write %04X\n", offset);
		return;
	}
	m_board_ram[offset - (16*1024)] = data;
}

void x68k_neptune_device::x68k_neptune_irq_w(int state)
{
	m_slot->irq2_w(state);
	logerror("Neptune: IRQ2 set to %i\n",state);
}

uint8_t x68k_neptune_device::iack2()
{
	return NEPTUNE_IRQ_VECTOR;
}
