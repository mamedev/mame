/***********************************************************************************************************

 Super NES/Famicom (LoROM) cartridge emulation (for SNES/SFC)

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.

 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sns_rom.h"


//-------------------------------------------------
//  sns_rom_device - constructor
//-------------------------------------------------

const device_type SNS_LOROM = &device_creator<sns_rom_device>;
const device_type SNS_LOROM_OBC1 = &device_creator<sns_rom_obc1_device>;
// LoROM pirate carts with protection
const device_type SNS_LOROM_POKEMON = &device_creator<sns_rom_pokemon_device>;


sns_rom_device::sns_rom_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, type, name, tag, owner, clock),
						device_sns_cart_interface( mconfig, *this )
{
}

sns_rom_device::sns_rom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, SNS_LOROM, "SNES Cart (LoROM)", tag, owner, clock),
						device_sns_cart_interface( mconfig, *this )
{
}

sns_rom_pokemon_device::sns_rom_pokemon_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_POKEMON, "SNES Pirate Carts with Protection", tag, owner, clock)
{
}

sns_rom_obc1_device::sns_rom_obc1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_OBC1, "SNES Cart (LoROM) + OBC-1", tag, owner, clock)
{
}


void sns_rom_device::device_start()
{
	memset(rom_bank_map, 0, sizeof(rom_bank_map));
}

void sns_rom_pokemon_device::device_start()
{
	m_latch = 0;
	save_item(NAME(m_latch));
}

void sns_rom_obc1_device::device_start()
{
	memset(m_ram, 0xff, sizeof(m_ram));
	// or from rom?
	m_offset  = (m_ram[0x1ff5] & 0x01) ? 0x1800 : 0x1c00;
	m_address = (m_ram[0x1ff6] & 0x7f);
	m_shift   = (m_ram[0x1ff6] & 0x03) << 1;

	save_item(NAME(m_ram));
	save_item(NAME(m_address));
	save_item(NAME(m_offset));
	save_item(NAME(m_shift));
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(sns_rom_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_device::read_h)
{
	UINT8 value = 0xff;
	UINT16 address = offset & 0xffff;

	if (offset < 0x700000)
	{
		int bank = offset / 0x10000;
		value = m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	else
	{
		if (address < 0x8000)
		{
			if (m_nvram_size > 0x8000)
			{
				// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
				int mask = m_nvram_size - 1;
				offset = ((offset - 0x700000) / 0x10000) * 0x8000 + (offset & 0x7fff);
				value = m_nvram[offset & mask];
			}
			else if (m_nvram_size > 0)
			{
				int mask = m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
				value = m_nvram[offset & mask];
			}
			else
				value = 0xff;   // this should never happened...
		}
		else
		{
			int bank = offset / 0x10000;
			value = m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
		}
	}
	return value;
}

WRITE8_MEMBER(sns_rom_device::write_l)
{
	write_h(space, offset, data);
}

WRITE8_MEMBER(sns_rom_device::write_h)
{
	if (offset >= 0x700000) // SRAM
	{
		if (m_nvram_size > 0x8000)
		{
			// In this case, SRAM is mapped in 0x8000 chunks at diff offsets: 0x700000-0x707fff, 0x710000-0x717fff, etc.
			int mask = m_nvram_size - 1;
			offset = ((offset - 0x700000) / 0x10000) * 0x8000 + (offset & 0x7fff);
			m_nvram[offset & mask] = data;
		}
		else if (m_nvram_size > 0)
		{
			int mask = m_nvram_size - 1;   /* Limit SRAM size to what's actually present */
			m_nvram[offset & mask] = data;
		}
	}
}



// Lo-ROM + Protection device

READ8_MEMBER( sns_rom_pokemon_device::chip_read )
{
	return BITSWAP8(m_latch,0,6,7,1,2,3,4,5);
}

WRITE8_MEMBER( sns_rom_pokemon_device::chip_write )
{
	m_latch = data;
}


// Lo-ROM + OBC-1 (used by Metal Combat - Falcon's Revenge)
// same as above but additional read/write handling for the add-on chip

/***************************************************************************

 Based on C++ implementation by Byuu in BSNES.

 Byuu's code is released under GNU General Public License
 version 2 as published by the Free Software Foundation.

 The implementation below is released under the MAME license
 for use in MAME, MESS and derivatives by permission of Byuu

 Copyright (for the implementation below) MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.

 ***********************************************************************************************************/


READ8_MEMBER( sns_rom_obc1_device::chip_read )
{
	UINT16 address = offset & 0x1fff;
	UINT8 value;

	switch (address)
	{
		case 0x1ff0:
			value = m_ram[m_offset + (m_address << 2) + 0];
			break;

		case 0x1ff1:
			value = m_ram[m_offset + (m_address << 2) + 1];
			break;

		case 0x1ff2:
			value = m_ram[m_offset + (m_address << 2) + 2];
			break;

		case 0x1ff3:
			value = m_ram[m_offset + (m_address << 2) + 3];
			break;

		case 0x1ff4:
			value = m_ram[m_offset + (m_address >> 2) + 0x200];
			break;

		default:
			value = m_ram[address];
			break;
	}

	return value;
}


WRITE8_MEMBER( sns_rom_obc1_device::chip_write )
{
	UINT16 address = offset & 0x1fff;
	UINT8 temp;

	switch(address)
	{
		case 0x1ff0:
			m_ram[m_offset + (m_address << 2) + 0] = data;
			break;

		case 0x1ff1:
			m_ram[m_offset + (m_address << 2) + 1] = data;
			break;

		case 0x1ff2:
			m_ram[m_offset + (m_address << 2) + 2] = data;
			break;

		case 0x1ff3:
			m_ram[m_offset + (m_address << 2) + 3] = data;
			break;

		case 0x1ff4:
			temp = m_ram[m_offset + (m_address >> 2) + 0x200];
			temp = (temp & ~(3 << m_shift)) | ((data & 0x03) << m_shift);
			m_ram[m_offset + (m_address >> 2) + 0x200] = temp;
			break;

		case 0x1ff5:
			m_offset = (data & 0x01) ? 0x1800 : 0x1c00;
			m_ram[address & 0x1fff] = data;
			break;

		case 0x1ff6:
			m_address = data & 0x7f;
			m_shift = (data & 0x03) << 1;
			m_ram[address & 0x1fff] = data;
			break;

		default:
			m_ram[address & 0x1fff] = data;
			break;
	}
}
