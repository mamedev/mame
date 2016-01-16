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

const device_type SNS_LOROM_SUFAMI = &device_creator<sns_rom_sufami_device>;
const device_type SNS_STROM = &device_creator<sns_rom_strom_device>;


sns_rom_sufami_device::sns_rom_sufami_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_SUFAMI, "SNES Sufami Turbo Cart", tag, owner, clock, "sns_rom_sufami", __FILE__),
						m_slot1(*this, "st_slot1"),
						m_slot2(*this, "st_slot2")
{
}

sns_rom_strom_device::sns_rom_strom_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_STROM, "SNES Sufami Turbo Minicart", tag, owner, clock, "sns_strom", __FILE__)
{
}


void sns_rom_sufami_device::device_start()
{
}

void sns_rom_strom_device::device_start()
{
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( st_slot )
//-------------------------------------------------

static SLOT_INTERFACE_START(sufamiturbo_cart)
	SLOT_INTERFACE_INTERNAL("strom",  SNS_STROM)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( st_slot )
	MCFG_SNS_SUFAMI_CARTRIDGE_ADD("st_slot1", sufamiturbo_cart, nullptr)
	MCFG_SNS_SUFAMI_CARTRIDGE_ADD("st_slot2", sufamiturbo_cart, nullptr)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor sns_rom_sufami_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( st_slot );
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

READ8_MEMBER(sns_rom_sufami_device::read_l)
{
	return read_h(space, offset);
}

READ8_MEMBER(sns_rom_sufami_device::read_h)
{
	int bank;

	if (offset < 0x200000)      // SUFAMI TURBO ROM
	{
		bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	if (offset >= 0x200000 && offset < 0x400000)    // SLOT1 STROM
	{
		return m_slot1->read_l(space, offset - 0x200000);
	}
	if (offset >= 0x400000 && offset < 0x600000)    // SLOT2 STROM
	{
		return m_slot2->read_l(space, offset - 0x400000);
	}
	if (offset >= 0x600000 && offset < 0x640000)    // SLOT1 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x600000;
			bank = offset / 0x10000;
			return m_slot1->read_ram(space, bank * 0x8000 + (offset & 0x7fff));
		}
	}
	if (offset >= 0x700000 && offset < 0x740000)    // SLOT2 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x700000;
			bank = offset / 0x10000;
			return m_slot2->read_ram(space, bank * 0x8000 + (offset & 0x7fff));
		}
	}

	return 0xff;
}

WRITE8_MEMBER(sns_rom_sufami_device::write_l)
{
	write_h(space, offset, data);
}

WRITE8_MEMBER(sns_rom_sufami_device::write_h)
{
	int bank;
	if (offset >= 0x600000 && offset < 0x640000)    // SLOT1 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x600000;
			bank = offset / 0x10000;
			m_slot1->write_ram(space, bank * 0x8000 + (offset & 0x7fff), data);
		}
	}

	if (offset >= 0x700000 && offset < 0x740000)    // SLOT2 RAM
	{
		if ((offset & 0xffff) >= 0x8000)
		{
			offset -= 0x700000;
			bank = offset / 0x10000;
			m_slot2->write_ram(space, bank * 0x8000 + (offset & 0x7fff), data);
		}
	}

}

/*-------------------------------------------------
 Sufami Turbo 'minicart' emulation
 -------------------------------------------------*/

READ8_MEMBER(sns_rom_strom_device::read_l)
{
	if (offset < 0x200000)
	{
		int bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	return 0xff;
}
