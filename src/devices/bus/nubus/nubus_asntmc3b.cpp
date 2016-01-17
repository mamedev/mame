// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Asante MC3NB NuBus Ethernet card (DP83902)
  Apple NuBus Ethernet Card (DP8390)

  Based on National Semiconductor DP8390 family chips

  FssD0000 - 64k RAM buffer (used as DP83902 DMA target)
  FssE0000 - DP83902 registers

***************************************************************************/

#include "emu.h"
#include "nubus_asntmc3b.h"

#define MAC8390_ROM_REGION  "asntm3b_rom"
#define MAC8390_839X  "dp83902"


MACHINE_CONFIG_FRAGMENT( asntm3b )
	MCFG_DEVICE_ADD(MAC8390_839X, DP8390D, 0)
	MCFG_DP8390D_IRQ_CB(WRITELINE(nubus_mac8390_device, dp_irq_w))
	MCFG_DP8390D_MEM_READ_CB(READ8(nubus_mac8390_device, dp_mem_read))
	MCFG_DP8390D_MEM_WRITE_CB(WRITE8(nubus_mac8390_device, dp_mem_write))
MACHINE_CONFIG_END

ROM_START( asntm3nb )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "asante_mc3b.bin", 0x000000, 0x004000, CRC(4f86d451) SHA1(d0a41df667e6b51fbc63f9251d593f4fc49104ba) )
ROM_END

ROM_START( appleenet )
	ROM_REGION(0x4000, MAC8390_ROM_REGION, 0)
	ROM_LOAD( "aenet1",       0x000000, 0x004000, CRC(e3ae8c26) SHA1(01ddc15ee84b17128203cb812f29bac6b20fd642) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type NUBUS_ASNTMC3NB = &device_creator<nubus_asntmc3nb_device>;
const device_type NUBUS_APPLEENET = &device_creator<nubus_appleenet_device>;


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nubus_mac8390_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( asntm3b );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nubus_mac8390_device::device_rom_region() const
{
	return ROM_NAME( asntm3nb );
}

const rom_entry *nubus_appleenet_device::device_rom_region() const
{
	return ROM_NAME( appleenet );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  nubus_mac8390_device - constructor
//-------------------------------------------------

nubus_mac8390_device::nubus_mac8390_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
		device_t(mconfig, NUBUS_ASNTMC3NB, name, tag, owner, clock, shortname, source),
		device_nubus_card_interface(mconfig, *this),
		m_dp83902(*this, MAC8390_839X)
{
}

nubus_asntmc3nb_device::nubus_asntmc3nb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		nubus_mac8390_device(mconfig, NUBUS_ASNTMC3NB, "Asante MC3NB Ethernet card", tag, owner, clock, "nb_amc3b", __FILE__)
{
}

nubus_appleenet_device::nubus_appleenet_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
		nubus_mac8390_device(mconfig, NUBUS_APPLEENET, "Apple NuBus Ethernet card", tag, owner, clock, "nb_aenet", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void nubus_mac8390_device::device_start()
{
	UINT32 slotspace;
	char mac[7];
	UINT32 num = rand();
	memset(m_prom, 0x57, 16);
	sprintf(mac+2, "\x1b%c%c%c", (num >> 16) & 0xff, (num >> 8) & 0xff, num & 0xff);
	mac[0] = mac[1] = 0;  // avoid gcc warning
	memcpy(m_prom, mac, 6);
	m_dp83902->set_mac(mac);
	// set_nubus_device makes m_slot valid
	set_nubus_device();
	install_declaration_rom(this, MAC8390_ROM_REGION, true);

	slotspace = get_slotspace();

//  printf("[ASNTMC3NB %p] slotspace = %x\n", this, slotspace);

	// TODO: move 24-bit mirroring down into nubus.c
	UINT32 ofs_24bit = m_slot<<20;
	m_nubus->install_device(slotspace+0xd0000, slotspace+0xdffff, read8_delegate(FUNC(nubus_mac8390_device::asntm3b_ram_r), this), write8_delegate(FUNC(nubus_mac8390_device::asntm3b_ram_w), this));
	m_nubus->install_device(slotspace+0xe0000, slotspace+0xe003f, read32_delegate(FUNC(nubus_mac8390_device::en_r), this), write32_delegate(FUNC(nubus_mac8390_device::en_w), this));
	m_nubus->install_device(slotspace+0xd0000+ofs_24bit, slotspace+0xdffff+ofs_24bit, read8_delegate(FUNC(nubus_mac8390_device::asntm3b_ram_r), this), write8_delegate(FUNC(nubus_mac8390_device::asntm3b_ram_w), this));
	m_nubus->install_device(slotspace+0xe0000+ofs_24bit, slotspace+0xe003f+ofs_24bit, read32_delegate(FUNC(nubus_mac8390_device::en_r), this), write32_delegate(FUNC(nubus_mac8390_device::en_w), this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void nubus_mac8390_device::device_reset()
{
	m_dp83902->dp8390_reset(0);
	m_dp83902->dp8390_cs(0);
	memcpy(m_prom, m_dp83902->get_mac(), 6);
}

WRITE8_MEMBER( nubus_mac8390_device::asntm3b_ram_w )
{
//    printf("MC3NB: CPU wrote %02x to RAM @ %x\n", data, offset);
	m_ram[offset] = data;
}

READ8_MEMBER( nubus_mac8390_device::asntm3b_ram_r )
{
//    printf("MC3NB: CPU read %02x @ RAM %x\n", m_ram[offset], offset);
	return m_ram[offset];
}

WRITE32_MEMBER( nubus_mac8390_device::en_w )
{
	if (mem_mask == 0xff000000)
	{
//        printf("%02x to 8390 @ %x\n", data>>24, 0xf-offset);
		m_dp83902->dp8390_w(space, 0xf-offset, data>>24);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_dp83902->dp8390_cs(1);
		m_dp83902->dp8390_w(space, 0xf-offset, data>>16);
		m_dp83902->dp8390_cs(0);
	}
	else
	{
		fatalerror("asntmc3nb: write %08x to DP83902 @ %x with unhandled mask %08x (PC=%x)\n", data, offset, mem_mask, space.device().safe_pc());
	}
}

READ32_MEMBER( nubus_mac8390_device::en_r )
{
	if (mem_mask == 0xff000000)
	{
		return (m_dp83902->dp8390_r(space, 0xf-offset)<<24);
	}
	else if (mem_mask == 0xffff0000)
	{
		m_dp83902->dp8390_cs(1);
		return (m_dp83902->dp8390_r(space, 0xf-offset)<<16);
		m_dp83902->dp8390_cs(0);
	}
	else
	{
		fatalerror("asntmc3nb: read DP83902 @ %x with unhandled mask %08x (PC=%x)\n", offset, mem_mask, space.device().safe_pc());
	}

	return 0;
}

WRITE_LINE_MEMBER( nubus_mac8390_device::dp_irq_w )
{
	if (state)
	{
		raise_slot_irq();
	}
	else
	{
		lower_slot_irq();
	}
}

READ8_MEMBER( nubus_mac8390_device::dp_mem_read )
{
//    printf("MC3NB: 8390 read RAM @ %x = %02x\n", offset, m_ram[offset]);
	return m_ram[offset];
}

WRITE8_MEMBER( nubus_mac8390_device::dp_mem_write )
{
//    printf("MC3NB: 8390 wrote %02x to RAM @ %x\n", data, offset);
	m_ram[offset] = data;
}
