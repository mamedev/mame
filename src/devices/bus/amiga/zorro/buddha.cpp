// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

***************************************************************************/

#include "buddha.h"

//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type BUDDHA = &device_creator<buddha_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( buddha )
	MCFG_ATA_INTERFACE_ADD("ata_0", ata_devices, nullptr, nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(buddha_device, ide_0_interrupt_w))
	MCFG_ATA_INTERFACE_ADD("ata_1", ata_devices, nullptr, nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(buddha_device, ide_1_interrupt_w))
MACHINE_CONFIG_END

machine_config_constructor buddha_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( buddha );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( buddha )
	ROM_REGION16_BE(0x10000, "bootrom", 0)
	ROM_DEFAULT_BIOS("v103-17")
	ROM_SYSTEM_BIOS(0, "v103-8", "Version 103.8")
	ROMX_LOAD("buddha_103-8.rom", 0x0000, 0x8000, CRC(44f81426) SHA1(95555c6690b5c697e1cdca2726e47c1c6c194d7c), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v103-17", "Version 103.17")
	ROMX_LOAD("buddha_103-17.rom", 0x0000, 0x8000, CRC(2b7b24e0) SHA1(ec17a58962c373a2892090ec9b1722d2c326d631), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

const rom_entry *buddha_device::device_rom_region() const
{
	return ROM_NAME( buddha );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  buddha_device - constructor
//-------------------------------------------------

buddha_device::buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, BUDDHA, "Buddha IDE controller", tag, owner, clock, "buddha", __FILE__),
	device_zorro2_card_interface(mconfig, *this),
	m_ata_0(*this, "ata_0"),
	m_ata_1(*this, "ata_1"),
	m_ide_interrupts_enabled(false),
	m_ide_0_interrupt(0),
	m_ide_1_interrupt(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void buddha_device::device_start()
{
	set_zorro_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buddha_device::device_reset()
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void buddha_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("%s('%s'): autoconfig_base_address received: 0x%06x\n", shortname(), basetag(), address);

	if (VERBOSE)
		logerror("-> installing buddha\n");

	// stop responding to default autoconfig
	m_slot->m_space->unmap_readwrite(0xe80000, 0xe8007f);

	// install autoconfig handler to new location
	m_slot->m_space->install_readwrite_handler(address, address + 0x7f,
		read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
		write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);

	// buddha registers
	m_slot->m_space->install_readwrite_handler(address + 0x7fe, address + 0x7ff,
		read16_delegate(FUNC(buddha_device::speed_r), this),
		write16_delegate(FUNC(buddha_device::speed_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0x800, address + 0x8ff,
		read16_delegate(FUNC(buddha_device::ide_0_cs0_r), this),
		write16_delegate(FUNC(buddha_device::ide_0_cs0_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0x900, address + 0x9ff,
		read16_delegate(FUNC(buddha_device::ide_0_cs1_r), this),
		write16_delegate(FUNC(buddha_device::ide_0_cs1_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0xa00, address + 0xaff,
		read16_delegate(FUNC(buddha_device::ide_0_cs0_r), this),
		write16_delegate(FUNC(buddha_device::ide_0_cs0_w), this), 0xffff);

	m_slot->m_space->install_readwrite_handler(address + 0xb00, address + 0xbff,
		read16_delegate(FUNC(buddha_device::ide_0_cs1_r), this),
		write16_delegate(FUNC(buddha_device::ide_0_cs1_w), this), 0xffff);

	m_slot->m_space->install_read_handler(address + 0xf00, address + 0xf3f,
		read16_delegate(FUNC(buddha_device::ide_0_interrupt_r), this), 0xffff);

	m_slot->m_space->install_read_handler(address + 0xf40, address + 0xf7f,
		read16_delegate(FUNC(buddha_device::ide_1_interrupt_r), this), 0xffff);

	m_slot->m_space->install_write_handler(address + 0xfc0, address + 0xfff,
		write16_delegate(FUNC(buddha_device::ide_interrupt_enable_w), this), 0xffff);

	// install access to the rom space
	m_slot->m_space->install_rom(address + 0x1000, address + 0xffff, memregion("bootrom")->base() + 0x1000);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( buddha_device::cfgin_w )
{
	if (VERBOSE)
		logerror("%s('%s'): configin_w (%d)\n", shortname(), basetag(), state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_64K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(true);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(0x00);
		autoconfig_manufacturer(0x1212);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x1000);

		// install autoconfig handler
		m_slot->m_space->install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
			write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);
	}
}

READ16_MEMBER( buddha_device::speed_r )
{
	UINT16 data = 0xffff;

	if (VERBOSE)
		logerror("%s('%s'): ide_0_interrupt_r %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::speed_w )
{
	if (VERBOSE)
		logerror("%s('%s'): speed_w %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);
}

WRITE_LINE_MEMBER( buddha_device::ide_0_interrupt_w)
{
	if (VERBOSE)
		logerror("%s('%s'): ide_0_interrupt_w (%d)\n", shortname(), basetag(), state);

	m_ide_0_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_slot->int2_w(state);
}

WRITE_LINE_MEMBER( buddha_device::ide_1_interrupt_w)
{
	if (VERBOSE)
		logerror("%s('%s'): ide_1_interrupt_w (%d)\n", shortname(), basetag(), state);

	m_ide_1_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_slot->int2_w(state);
}

READ16_MEMBER( buddha_device::ide_0_interrupt_r )
{
	UINT16 data;

	data = m_ide_0_interrupt << 15;

	if (VERBOSE)
		logerror("%s('%s'): ide_0_interrupt_r %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	logerror("%s\n", device().machine().describe_context());

	return data;
}

READ16_MEMBER( buddha_device::ide_1_interrupt_r )
{
	UINT16 data;

	data = m_ide_1_interrupt << 15;

	if (VERBOSE)
		logerror("%s('%s'): ide_1_interrupt_r %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_interrupt_enable_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ide_interrupt_enable_w %04x [mask = %04x]\n", shortname(), basetag(), data, mem_mask);

	// writing any value here enables ide interrupts to the zorro slot
	m_ide_interrupts_enabled = true;
}

READ16_MEMBER( buddha_device::ide_0_cs0_r )
{
	UINT16 data;

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = m_ata_0->read_cs0(space, (offset >> 1) & 0x07, mem_mask);

	if (VERBOSE)
		logerror("%s('%s'): ide_0_cs0_r(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	return (data << 8) | (data >> 8);
}

WRITE16_MEMBER( buddha_device::ide_0_cs0_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ide_0_cs0_w(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_0->write_cs0(space, (offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_0_cs1_r )
{
	UINT16 data;

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = m_ata_0->read_cs1(space, (offset >> 1) & 0x07, mem_mask);

	if (VERBOSE)
		logerror("%s('%s'): ide_0_cs1_r(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	return (data << 8) | (data >> 8);
}

WRITE16_MEMBER( buddha_device::ide_0_cs1_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ide_0_cs1_w(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_0->write_cs1(space, (offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_1_cs0_r )
{
	UINT16 data;

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = m_ata_1->read_cs0(space, (offset >> 1) & 0x07, mem_mask);

	if (VERBOSE)
		logerror("%s('%s'): ide_1_cs0_r(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	return (data << 8) | (data >> 8);
}

WRITE16_MEMBER( buddha_device::ide_1_cs0_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ide_1_cs0_w(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_1->write_cs0(space, (offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_1_cs1_r )
{
	UINT16 data;

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = m_ata_1->read_cs1(space, (offset >> 1) & 0x07, mem_mask);

	if (VERBOSE)
		logerror("%s('%s'): ide_1_cs1_r(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	return (data << 8) | (data >> 8);
}

WRITE16_MEMBER( buddha_device::ide_1_cs1_w )
{
	if (VERBOSE)
		logerror("%s('%s'): ide_1_cs1_w(%04x) %04x [mask = %04x]\n", shortname(), basetag(), offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_1->write_cs1(space, (offset >> 1) & 0x07, data, mem_mask);
}
