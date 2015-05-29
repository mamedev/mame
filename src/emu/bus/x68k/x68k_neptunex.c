// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_neptunex.c
 */

#include "emu.h"
#include "machine/dp8390.h"
#include "x68k_neptunex.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type X68K_NEPTUNEX = &device_creator<x68k_neptune_device>;

// device machine config
static MACHINE_CONFIG_FRAGMENT( x68k_neptunex )
	MCFG_DEVICE_ADD("dp8390d", DP8390D, 0)
	MCFG_DP8390D_IRQ_CB(WRITELINE(x68k_neptune_device, x68k_neptune_irq_w))
	MCFG_DP8390D_MEM_READ_CB(READ8(x68k_neptune_device, x68k_neptune_mem_read))
	MCFG_DP8390D_MEM_WRITE_CB(WRITE8(x68k_neptune_device, x68k_neptune_mem_write))
MACHINE_CONFIG_END

machine_config_constructor x68k_neptune_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( x68k_neptunex );
}

x68k_neptune_device::x68k_neptune_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, X68K_NEPTUNEX, "Neptune-X", tag, owner, clock, "x68k_neptunex", __FILE__),
		device_x68k_expansion_card_interface(mconfig, *this),
		m_dp8390(*this, "dp8390d")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void x68k_neptune_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	char mac[7];
	UINT32 num = rand();
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	memset(m_prom, 0x57, 16);
	sprintf(mac+2, "\x1b%c%c%c", (num >> 16) & 0xff, (num >> 8) & 0xff, num & 0xff);
	mac[0] = 0; mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp8390->set_mac(mac);
	space.install_readwrite_handler(0xece000,0xece3ff,read16_delegate(FUNC(x68k_neptune_device::x68k_neptune_port_r),this),write16_delegate(FUNC(x68k_neptune_device::x68k_neptune_port_w),this),0xffffffff);
}

void x68k_neptune_device::device_reset() {
	memcpy(m_prom, m_dp8390->get_mac(), 6);
}

READ16_MEMBER(x68k_neptune_device::x68k_neptune_port_r)
{
	UINT16 data;

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
	machine().device("maincpu")->execute().set_input_line_vector(2, NEPTUNE_IRQ_VECTOR);
	m_slot->irq2_w(state);
	logerror("Neptune: IRQ2 set to %i\n",state);
}
