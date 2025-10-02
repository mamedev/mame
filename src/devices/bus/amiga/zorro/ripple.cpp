// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    RIPPLE IDE

    Zorro-II IDE interface for Amiga 2000/3000/4000

    Notes:
    - See https://github.com/LIV2/RIPPLE-IDE
    - To enable boot from CD-ROM you need CDFileSystem from OS 3.2.1 or
      later, then you can issue the command "LoadModule L:CDFileSystem"
    - If you have the correct version of CDFileSystem you can also flash
      it to the controller with "lideflash -C L:CDFileSystem"

    TODO:
    - Verify cs1 access

***************************************************************************/

#include "emu.h"
#include "ripple.h"

#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(AMIGA_RIPPLE, bus::amiga::zorro::ripple_ide_device, "amiga_ripple", "RIPPLE IDE Interface")

namespace bus::amiga::zorro {

ripple_ide_device::ripple_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AMIGA_RIPPLE, tag, owner, clock),
	device_zorro2_card_interface(mconfig, *this),
	m_ata_0(*this, "ata_0"),
	m_ata_1(*this, "ata_1"),
	m_flash(*this, "flash")
{
}


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ripple_ide_device::mmio_map(address_map &map)
{
	map(0x01000, 0x01fff).rw(FUNC(ripple_ide_device::ide0_cs0_r), FUNC(ripple_ide_device::ide0_cs0_w));
	map(0x02000, 0x02fff).rw(FUNC(ripple_ide_device::ide1_cs0_r), FUNC(ripple_ide_device::ide1_cs0_w));
	map(0x05000, 0x05fff).rw(FUNC(ripple_ide_device::ide0_cs1_r), FUNC(ripple_ide_device::ide0_cs1_w));
	map(0x06000, 0x06fff).rw(FUNC(ripple_ide_device::ide1_cs1_r), FUNC(ripple_ide_device::ide1_cs1_w));
	map(0x08000, 0x08fff).w(FUNC(ripple_ide_device::bank_select_w));
	map(0x10000, 0x1ffff).rw(FUNC(ripple_ide_device::banked_flash_r), FUNC(ripple_ide_device::banked_flash_w)).umask16(0xff00);
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void ripple_ide_device::device_add_mconfig(machine_config &config)
{
	ATA_INTERFACE(config, m_ata_0).options(ata_devices, nullptr, nullptr, false);
	ATA_INTERFACE(config, m_ata_1).options(ata_devices, nullptr, nullptr, false);

	SST_39SF010(config, "flash");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( lide )
	ROM_REGION(0x20000, "flash", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("40.8")
	ROM_SYSTEM_BIOS(0, "40.8", "Release 40.8")
	ROMX_LOAD("lide_40-8.rom", 0x0000, 0x8000, CRC(3f021472) SHA1(83762ef5a883e5c43ad321eaa03fde7454f70785), ROM_BIOS(0))
ROM_END

const tiny_rom_entry *ripple_ide_device::device_rom_region() const
{
	return ROM_NAME( lide );
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ripple_ide_device::device_start()
{
	// register for save states
	save_item(NAME(m_base_address));
	save_item(NAME(m_flash_bank));
}

void ripple_ide_device::busrst_w(int state)
{
	if (state == 0)
		m_flash_bank = 0;
}

void ripple_ide_device::bank_select_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		m_flash_bank = data >> 14;
}

uint8_t ripple_ide_device::banked_flash_r(offs_t offset)
{
	return m_flash->read(m_flash_bank << 15 | offset);
}

void ripple_ide_device::banked_flash_w(offs_t offset, uint8_t data)
{
	m_flash->write(m_flash_bank << 15 | offset, data);
}

uint16_t ripple_ide_device::ide0_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_0->cs0_swap_r(offset >> 8, mem_mask);
}

void ripple_ide_device::ide0_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_0->cs0_swap_w(offset >> 8, data, mem_mask);
}

uint16_t ripple_ide_device::ide0_cs1_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_0->cs1_swap_r(offset >> 8, mem_mask);
}

void ripple_ide_device::ide0_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_0->cs1_swap_w(offset >> 8, data, mem_mask);
}

uint16_t ripple_ide_device::ide1_cs0_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_1->cs0_swap_r(offset >> 8, mem_mask);
}

void ripple_ide_device::ide1_cs0_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_1->cs0_swap_w(offset >> 8, data, mem_mask);
}

uint16_t ripple_ide_device::ide1_cs1_r(offs_t offset, uint16_t mem_mask)
{
	return m_ata_1->cs1_swap_r(offset >> 8, mem_mask);
}

void ripple_ide_device::ide1_cs1_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_ata_1->cs1_swap_w(offset >> 8, data, mem_mask);
}


//**************************************************************************
//  AUTOCONFIG
//**************************************************************************

void ripple_ide_device::autoconfig_base_address(offs_t address)
{
	LOG("autoconfig_base_address received: 0x%06x\n", address);
	LOG("-> installing ripple ide\n");

	// save base address so that the tap can reconfigure our space
	m_base_address = address;

	// stop responding to default autoconfig
	m_zorro->space().unmap_readwrite(0xe80000, 0xe8007f);

	// flash occupies our space until the ide registers are switched in
	m_zorro->space().install_readwrite_handler(address, address + 0x1ffff,
		emu::rw_delegate(m_flash, FUNC(intelfsh8_device::read)),
		emu::rw_delegate(m_flash, FUNC(intelfsh8_device::write)), 0xff00);

	// install write tap to handle switching in ide registers
	m_write_tap.remove();
	m_write_tap = m_zorro->space().install_write_tap(
		address, address + 0x1ffff,
		"flash_disable_w",
		[this] (offs_t offset, uint16_t &data, uint16_t mem_mask)
		{
			m_write_tap.remove();

			// ripple registers are now available
			m_zorro->space().install_device(m_base_address, m_base_address + 0x1ffff, *this, &ripple_ide_device::mmio_map);

			// we need to repeat the write here as this tap won't hit it yet
			// the initial write will instead hit the flash, but it's harmless
			m_zorro->space().write_word(offset, data, mem_mask);
		},
		&m_write_tap
	);

	// we're done
	m_zorro->cfgout_w(0);
}

void ripple_ide_device::cfgin_w(int state)
{
	LOG("cfgin_w (%d)\n", state);

	if (state == 0)
	{
		// setup autoconfig
		autoconfig_board_type(BOARD_TYPE_ZORRO2);
		autoconfig_board_size(BOARD_SIZE_128K);
		autoconfig_link_into_memory(false);
		autoconfig_rom_vector_valid(true);
		autoconfig_multi_device(false);
		autoconfig_8meg_preferred(false);
		autoconfig_can_shutup(true);
		autoconfig_product(7);
		autoconfig_manufacturer(5194);
		autoconfig_serial(0x00000000);
		autoconfig_rom_vector(0x0008);

		// install autoconfig handler
		m_zorro->space().install_readwrite_handler(0xe80000, 0xe8007f,
			read16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_read)),
			write16_delegate(*this, FUNC(amiga_autoconfig::autoconfig_write)), 0xffff);
	}
}

} // namespace bus::amiga::zorro
