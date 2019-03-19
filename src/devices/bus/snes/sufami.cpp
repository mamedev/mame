// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

 Bandai Sufami Turbo cartridge emulation (for SNES/SFC)

 This is basically a standard LoROM cart with two slots for ST minicarts
 The content of each slot (with ROM and RAM) is mapped to a separate memory range
 Slot 1: ROM [20-3f][8000-ffff], RAM [60-63][8000-ffff]
 Slot 2: ROM [40-5f][8000-ffff], RAM [70-73][8000-ffff]

 ***********************************************************************************************************/


#include "emu.h"
#include "sufami.h"


//-------------------------------------------------
//  sns_rom_sufami_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(SNS_LOROM_SUFAMI, sns_rom_sufami_device, "sns_rom_sufami", "SNES Sufami Turbo Cart")
DEFINE_DEVICE_TYPE(SNS_STROM,        sns_rom_strom_device,  "sns_strom",      "SNES Sufami Turbo Minicart")


sns_rom_sufami_device::sns_rom_sufami_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, SNS_LOROM_SUFAMI, tag, owner, clock)
	, m_slot1(*this, "st_slot1")
	, m_slot2(*this, "st_slot2")
{
}

sns_rom_strom_device::sns_rom_strom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sns_rom_device(mconfig, SNS_STROM, tag, owner, clock)
{
}


void sns_rom_sufami_device::device_start()
{
}

void sns_rom_strom_device::device_start()
{
}


static void sufamiturbo_cart(device_slot_interface &device)
{
	device.option_add_internal("strom",  SNS_STROM);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sns_rom_sufami_device::device_add_mconfig(machine_config &config)
{
	SNS_SUFAMI_CART_SLOT(config, m_slot1, sufamiturbo_cart, nullptr);
	SNS_SUFAMI_CART_SLOT(config, m_slot2, sufamiturbo_cart, nullptr);
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

uint8_t sns_rom_sufami_device::read_l(offs_t offset)
{
	return read_h(offset);
}

uint8_t sns_rom_sufami_device::read_h(offs_t offset)
{
	int bank;

	if (offset < 0x200000)      // SUFAMI TURBO ROM
	{
		bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	if (offset >= 0x200000 && offset < 0x400000)    // SLOT1 STROM
	{
		return m_slot1->read_l(offset - 0x200000);
	}
	if (offset >= 0x400000 && offset < 0x600000)    // SLOT2 STROM
	{
		return m_slot2->read_l(offset - 0x400000);
	}
	if (offset >= 0x600000 && offset < 0x640000)    // SLOT1 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x600000;
			bank = offset / 0x10000;
			return m_slot1->read_ram(bank * 0x8000 + (offset & 0x7fff));
		}
	}
	if (offset >= 0x700000 && offset < 0x740000)    // SLOT2 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x700000;
			bank = offset / 0x10000;
			return m_slot2->read_ram(bank * 0x8000 + (offset & 0x7fff));
		}
	}

	return 0xff;
}

void sns_rom_sufami_device::write_l(offs_t offset, uint8_t data)
{
	write_h(offset, data);
}

void sns_rom_sufami_device::write_h(offs_t offset, uint8_t data)
{
	int bank;
	if (offset >= 0x600000 && offset < 0x640000)    // SLOT1 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x600000;
			bank = offset / 0x10000;
			m_slot1->write_ram(bank * 0x8000 + (offset & 0x7fff), data);
		}
	}

	if (offset >= 0x700000 && offset < 0x740000)    // SLOT2 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x700000;
			bank = offset / 0x10000;
			m_slot2->write_ram(bank * 0x8000 + (offset & 0x7fff), data);
		}
	}

}

/*-------------------------------------------------
 Sufami Turbo 'minicart' emulation
 -------------------------------------------------*/

uint8_t sns_rom_strom_device::read_l(offs_t offset)
{
	if (offset < 0x200000)
	{
		int bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	return 0xff;
}
