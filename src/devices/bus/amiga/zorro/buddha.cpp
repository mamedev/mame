// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Buddha

    Zorro-II IDE controller

    The 'speed' register is used to select the IDE timing according to
    the following table (bits 7-5 are used):

    0    497ns   7c  to select, IOR/IOW after 172ns  2c
    1    639ns   9c  to select, IOR/IOW after 243ns  3c
    2    781ns  11c  to select, IOR/IOW after 314ns  4c
    3    355ns   5c  to select, IOR/IOW after 101ns  1c
    4    355ns   5c  to select, IOR/IOW after 172ns  2c
    5    355ns   5c  to select, IOR/IOW after 243ns  3c
    6   1065ns  15c  to select, IOR/IOW after 314ns  4c
    7    355ns   5c  to select, IOR/IOW after 101ns  1c

    c = clock cycles. This isn't emulated.

***************************************************************************/

#include "emu.h"
#include "buddha.h"

//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define VERBOSE 1


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BUDDHA, buddha_device, "buddha", "Buddha IDE controller")

//-------------------------------------------------
//  mmio_map - device-specific memory mapped I/O
//-------------------------------------------------

void buddha_device::mmio_map(address_map &map)
{
	map(0x7fe, 0x7ff).rw(FUNC(buddha_device::speed_r), FUNC(buddha_device::speed_w));
	map(0x800, 0x8ff).rw(FUNC(buddha_device::ide_0_cs0_r), FUNC(buddha_device::ide_0_cs0_w));
	map(0x900, 0x9ff).rw(FUNC(buddha_device::ide_0_cs1_r), FUNC(buddha_device::ide_0_cs1_w));
	map(0xa00, 0xaff).rw(FUNC(buddha_device::ide_1_cs0_r), FUNC(buddha_device::ide_1_cs0_w));
	map(0xb00, 0xbff).rw(FUNC(buddha_device::ide_1_cs1_r), FUNC(buddha_device::ide_1_cs1_w));
	map(0xf00, 0xf3f).r(FUNC(buddha_device::ide_0_interrupt_r));
	map(0xf40, 0xf7f).r(FUNC(buddha_device::ide_1_interrupt_r));
	map(0xfc0, 0xfff).w(FUNC(buddha_device::ide_interrupt_enable_w));
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void buddha_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata_0).options(ata_devices, nullptr, nullptr, false);
	m_ata_0->irq_handler().set(FUNC(buddha_device::ide_0_interrupt_w));

	ATA_INTERFACE(config, m_ata_1).options(ata_devices, nullptr, nullptr, false);
	m_ata_1->irq_handler().set(FUNC(buddha_device::ide_1_interrupt_w));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( buddha )
	ROM_REGION16_BE(0x10000, "bootrom", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("v103-17")
	ROM_SYSTEM_BIOS(0, "v103-8", "Version 103.8")
	ROMX_LOAD("buddha_103-8.rom",  0x0000, 0x8000, CRC(44f81426) SHA1(95555c6690b5c697e1cdca2726e47c1c6c194d7c), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v103-17", "Version 103.17")
	ROMX_LOAD("buddha_103-17.rom", 0x0000, 0x8000, CRC(2b7b24e0) SHA1(ec17a58962c373a2892090ec9b1722d2c326d631), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

const tiny_rom_entry *buddha_device::device_rom_region() const
{
	return ROM_NAME( buddha );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  buddha_device - constructor
//-------------------------------------------------

buddha_device::buddha_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BUDDHA, tag, owner, clock),
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

	save_item(NAME(m_ide_interrupts_enabled));
	save_item(NAME(m_ide_0_interrupt));
	save_item(NAME(m_ide_1_interrupt));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void buddha_device::device_reset()
{
	m_ide_interrupts_enabled = false;
	m_ide_0_interrupt = 0;
	m_ide_1_interrupt = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void buddha_device::autoconfig_base_address(offs_t address)
{
	if (VERBOSE)
		logerror("autoconfig_base_address received: 0x%06x\n", address);

	if (VERBOSE)
		logerror("-> installing buddha\n");

	// stop responding to default autoconfig
	m_slot->space().unmap_readwrite(0xe80000, 0xe8007f);

	// buddha registers
	m_slot->space().install_device(address, address + 0xfff, *this, &buddha_device::mmio_map);

	// install autoconfig handler to new location
	m_slot->space().install_readwrite_handler(address, address + 0x7f,
		read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
		write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);

	// install access to the rom space
	m_slot->space().install_rom(address + 0x1000, address + 0xffff, memregion("bootrom")->base() + 0x1000);

	// we're done
	m_slot->cfgout_w(0);
}

WRITE_LINE_MEMBER( buddha_device::cfgin_w )
{
	if (VERBOSE)
		logerror("configin_w (%d)\n", state);

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
		m_slot->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(FUNC(amiga_autoconfig::autoconfig_read), static_cast<amiga_autoconfig *>(this)),
			write16_delegate(FUNC(amiga_autoconfig::autoconfig_write), static_cast<amiga_autoconfig *>(this)), 0xffff);
	}
}

READ16_MEMBER( buddha_device::speed_r )
{
	uint16_t data = 0xffff;

	if (VERBOSE)
		logerror("speed_r %04x [mask = %04x]\n", data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::speed_w )
{
	if (VERBOSE)
		logerror("speed_w %04x [mask = %04x]\n", data, mem_mask);
}

WRITE_LINE_MEMBER( buddha_device::ide_0_interrupt_w)
{
	if (VERBOSE)
		logerror("ide_0_interrupt_w (%d)\n", state);

	m_ide_0_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_slot->int2_w(state);
}

WRITE_LINE_MEMBER( buddha_device::ide_1_interrupt_w)
{
	if (VERBOSE)
		logerror("ide_1_interrupt_w (%d)\n", state);

	m_ide_1_interrupt = state;

	if (m_ide_interrupts_enabled)
		m_slot->int2_w(state);
}

READ16_MEMBER( buddha_device::ide_0_interrupt_r )
{
	uint16_t data;

	data = m_ide_0_interrupt << 15;

//	if (VERBOSE)
//		logerror("ide_0_interrupt_r %04x [mask = %04x]\n", data, mem_mask);

	return data;
}

READ16_MEMBER( buddha_device::ide_1_interrupt_r )
{
	uint16_t data;

	data = m_ide_1_interrupt << 15;

//	if (VERBOSE)
//		logerror("ide_1_interrupt_r %04x [mask = %04x]\n", data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_interrupt_enable_w )
{
	if (VERBOSE)
		logerror("ide_interrupt_enable_w %04x [mask = %04x]\n", data, mem_mask);

	// writing any value here enables ide interrupts to the zorro slot
	m_ide_interrupts_enabled = true;
}

READ16_MEMBER( buddha_device::ide_0_cs0_r )
{
	uint16_t data = m_ata_0->read_cs0((offset >> 1) & 0x07, (mem_mask << 8) | (mem_mask >> 8));
	data = (data << 8) | (data >> 8);

	if (VERBOSE)
		logerror("ide_0_cs0_r(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_0_cs0_w )
{
	if (VERBOSE)
		logerror("ide_0_cs0_w(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_0->write_cs0((offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_0_cs1_r )
{
	uint16_t data = m_ata_0->read_cs1((offset >> 1) & 0x07, (mem_mask << 8) | (mem_mask >> 8));
	data = (data << 8) | (data >> 8);

	if (VERBOSE)
		logerror("ide_0_cs1_r(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_0_cs1_w )
{
	if (VERBOSE)
		logerror("ide_0_cs1_w(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_0->write_cs1((offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_1_cs0_r )
{
	uint16_t data = m_ata_1->read_cs0((offset >> 1) & 0x07, (mem_mask << 8) | (mem_mask >> 8));
	data = (data << 8) | (data >> 8);

	if (VERBOSE)
		logerror("ide_1_cs0_r(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_1_cs0_w )
{
	if (VERBOSE)
		logerror("ide_1_cs0_w(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_1->write_cs0((offset >> 1) & 0x07, data, mem_mask);
}

READ16_MEMBER( buddha_device::ide_1_cs1_r )
{
	uint16_t data = m_ata_1->read_cs1((offset >> 1) & 0x07, (mem_mask << 8) | (mem_mask >> 8));
	data = (data << 8) | (data >> 8);

	if (VERBOSE)
		logerror("ide_1_cs1_r(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	return data;
}

WRITE16_MEMBER( buddha_device::ide_1_cs1_w )
{
	if (VERBOSE)
		logerror("ide_1_cs1_w(%04x) %04x [mask = %04x]\n", offset, data, mem_mask);

	mem_mask = (mem_mask << 8) | (mem_mask >> 8);
	data = (data << 8) | (data >> 8);

	m_ata_1->write_cs1((offset >> 1) & 0x07, data, mem_mask);
}
