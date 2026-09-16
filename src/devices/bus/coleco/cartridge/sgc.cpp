// license: BSD-3-Clause
// copyright-holders: Dirk Best
/**********************************************************************

    ColecoVision 'Super Game Cartridge' emulation

    Cartridge designed by Opcode Games

    Hardware:
    - Flash ROM for storage
    - ATF1502AS CPLD

    Notes:
    - Emulated here is what gradius needs. Other games might use
      more or different features.

**********************************************************************/

#include "emu.h"
#include "sgc.h"

#define LOG_WRITES  (1U << 1)
#define LOG_FLASH   (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_WRITES | LOG_FLASH)

#include "logmacro.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COLECOVISION_SGC_1MBIT, colecovision_sgc_1mbit_cartridge_device, "coleco_sgc_1mbit", "ColecoVision Super Game Cartridge (1 MBit)")
DEFINE_DEVICE_TYPE(COLECOVISION_SGC_2MBIT, colecovision_sgc_2mbit_cartridge_device, "coleco_sgc_2mbit", "ColecoVision Super Game Cartridge (2 MBit)")
DEFINE_DEVICE_TYPE(COLECOVISION_SGC_4MBIT, colecovision_sgc_4mbit_cartridge_device, "coleco_sgc_4mbit", "ColecoVision Super Game Cartridge (4 MBit)")

colecovision_sgc_cartridge_device::colecovision_sgc_cartridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_colecovision_cartridge_interface(mconfig, *this),
	m_flash(*this, "flash"),
	m_flash_region(*this, "flash"),
	m_slot0_bank(0),
	m_slot1_bank(0),
	m_slot2_bank(0),
	m_slot3_bank(0),
	m_flash_a16(0)
{
}

colecovision_sgc_1mbit_cartridge_device::colecovision_sgc_1mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	colecovision_sgc_cartridge_device(mconfig, COLECOVISION_SGC_1MBIT, tag, owner, clock)
{
}

colecovision_sgc_2mbit_cartridge_device::colecovision_sgc_2mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	colecovision_sgc_cartridge_device(mconfig, COLECOVISION_SGC_2MBIT, tag, owner, clock)
{
}

colecovision_sgc_4mbit_cartridge_device::colecovision_sgc_4mbit_cartridge_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	colecovision_sgc_cartridge_device(mconfig, COLECOVISION_SGC_4MBIT, tag, owner, clock)
{
}


//**************************************************************************
//  MEMORY REGIONS
//**************************************************************************

ROM_START( sgc_1mbit_flash )
	ROM_REGION(0x20000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *colecovision_sgc_1mbit_cartridge_device::device_rom_region() const
{
	return ROM_NAME( sgc_1mbit_flash );
}

ROM_START( sgc_2mbit_flash )
	ROM_REGION(0x40000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *colecovision_sgc_2mbit_cartridge_device::device_rom_region() const
{
	return ROM_NAME( sgc_2mbit_flash );
}

ROM_START( sgc_4mbit_flash )
	ROM_REGION(0x80000, "flash", ROMREGION_ERASEFF)
ROM_END

const tiny_rom_entry *colecovision_sgc_4mbit_cartridge_device::device_rom_region() const
{
	return ROM_NAME( sgc_4mbit_flash );
}


//**************************************************************************
//  MACHINE DEFINITIONS
//**************************************************************************

void colecovision_sgc_1mbit_cartridge_device::device_add_mconfig(machine_config &config)
{
	SST_39SF010(config, m_flash);
}

void colecovision_sgc_2mbit_cartridge_device::device_add_mconfig(machine_config &config)
{
	SST_39SF020(config, m_flash);
}

void colecovision_sgc_4mbit_cartridge_device::device_add_mconfig(machine_config &config)
{
	SST_39SF040(config, m_flash);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void colecovision_sgc_cartridge_device::device_start()
{
	// register for save states
	save_item(NAME(m_slot0_bank));
	save_item(NAME(m_slot1_bank));
	save_item(NAME(m_slot2_bank));
	save_item(NAME(m_slot3_bank));
	save_item(NAME(m_flash_a16));
}

void colecovision_sgc_cartridge_device::device_reset()
{
	m_slot0_bank = 0;
	m_slot1_bank = 0;
	m_slot2_bank = 0;
	m_slot3_bank = 0;
	m_flash_a16 = 0;
}

void colecovision_sgc_cartridge_device::load_done()
{
	if (m_rom_size > m_flash_region->bytes())
		fatalerror("SGC: ROM size %u larger than flash size %u\n", m_rom_size, m_flash_region->bytes());

	// initialize flash with provided rom data
	memcpy(m_flash_region->base(), m_rom, m_rom_size);
}

offs_t colecovision_sgc_cartridge_device::banked_address(offs_t offset)
{
	if (offset < 0xa000)
		return m_slot0_bank << 13 | (offset & 0x1fff);
	else if (offset < 0xc000)
		return m_slot1_bank << 13 | (offset & 0x1fff);
	else if (offset < 0xe000)
		return m_slot2_bank << 13 | (offset & 0x1fff);
	else
		return m_slot3_bank << 13 | (offset & 0x1fff);
}

uint8_t colecovision_sgc_cartridge_device::read(offs_t offset, int _8000, int _a000, int _c000, int _e000)
{
	return m_flash->read(banked_address(0x8000 | offset));
}

void colecovision_sgc_cartridge_device::write(offs_t offset, uint8_t data, int _8000, int _a000, int _c000, int _e000)
{
	offset |= 0x8000;
	uint8_t max_banks = m_rom_size / 0x2000;

	LOGMASKED(LOG_WRITES, "write: %04x = %02x\n", offset, data);

	switch (offset)
	{
		case 0xfffc:
			if (data < max_banks)
				m_slot1_bank = data;
			break;

		case 0xfffd:
			if (data < max_banks)
				m_slot2_bank = data;
			break;

		case 0xfffe:
			// gradius writes 0xaa here once, purpose?
			if (data < max_banks)
				m_slot3_bank = data;
			break;

		case 0xffff:
			// bit 0 = flash memory a16 (only when writing?)
			// other bits unknown (if actually used)
			m_flash_a16 = data;
			break;

		default:
			// write to flash
			offs_t addr = (BIT(m_flash_a16, 0) << 16) | banked_address(offset);
			LOGMASKED(LOG_FLASH, "flash write: %05x = %02x\n", addr, data);
			m_flash->write(addr, data);
			break;
	}
}
