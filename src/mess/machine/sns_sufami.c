/***********************************************************************************************************
 
 Bandai Sufami Turbo cartridge emulation (for SNES/SFC)
 
 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.
 
 This is basically a standard LoROM cart with two slots for ST minicarts
 The content of each slot (with ROM and RAM) is mapped to a separate memory range
 Slot 1: ROM [20-3f][8000-ffff], RAM [60-63][8000-ffff]
 Slot 2: ROM [40-5f][8000-ffff], RAM [70-73][8000-ffff]
 
 ***********************************************************************************************************/


#include "emu.h"
#include "machine/sns_sufami.h"


//-------------------------------------------------
//  sns_rom_sufami_device - constructor
//-------------------------------------------------

const device_type SNS_LOROM_SUFAMI = &device_creator<sns_rom_sufami_device>;
const device_type SNS_STROM = &device_creator<sns_rom_strom_device>;


sns_rom_sufami_device::sns_rom_sufami_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_LOROM_SUFAMI, "SNES Sufami Turbo Cart", tag, owner, clock),
						m_slot1(*this, "st_slot1"),
						m_slot2(*this, "st_slot2")
{
}

sns_rom_strom_device::sns_rom_strom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: sns_rom_device(mconfig, SNS_STROM, "SNES Sufami Turbo Minicart", tag, owner, clock)
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
	MCFG_SNS_SUFAMI_CARTRIDGE_ADD("st_slot1", sufamiturbo_cart, NULL, NULL)
	MCFG_SNS_SUFAMI_CARTRIDGE_ADD("st_slot2", sufamiturbo_cart, NULL, NULL)
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
	if (offset < 0x200000)		// SUFAMI TURBO ROM
	{
		int bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	if (offset >= 0x200000 && offset < 0x400000)	// SLOT1 STROM
	{
		if (m_slot1->m_cart)
			return m_slot1->m_cart->read_l(space, offset - 0x200000);
	}
	if (offset >= 0x400000 && offset < 0x600000)	// SLOT2 STROM
	{
		if (m_slot2->m_cart)
			return m_slot2->m_cart->read_l(space, offset - 0x400000);
	}
	if (offset >= 0x600000 && offset < 0x640000)	// SLOT1 RAM
	{
		if (m_slot1->m_cart && (offset & 0xffff) > 0x8000)
			return m_slot1->m_cart->read_h(space, offset - 0x600000);
	}
	if (offset >= 0x700000 && offset < 0x740000)	// SLOT2 RAM
	{
		if (m_slot2->m_cart && (offset & 0xffff) > 0x8000)
			return m_slot2->m_cart->read_h(space, offset - 0x700000);
	}
	
	return 0xff;
}

WRITE8_MEMBER(sns_rom_sufami_device::write_l)
{
	write_h(space, offset, data);
}

WRITE8_MEMBER(sns_rom_sufami_device::write_h)
{
	if (offset >= 0x600000 && offset < 0x640000)	// SLOT1 RAM
	{
		if (m_slot1->m_cart && (offset & 0xffff) > 0x8000)
			return m_slot1->m_cart->write_h(space, offset - 0x600000, data);
	}
	
	if (offset >= 0x700000 && offset < 0x740000)	// SLOT2 RAM
	{
		if (m_slot2->m_cart && (offset & 0xffff) > 0x8000)
			return m_slot2->m_cart->write_h(space, offset - 0x700000, data);
	}
	
}

/*-------------------------------------------------
 Sufami Turbo 'minicart' emulation
 -------------------------------------------------*/

// Here we're cheating a bit, for the moment, to avoid the need of ST carts as a completely different device
// which would require separate loading routines
// Hence, we use low r/w handlers for ROM access and hi r/w handlers for RAM access...
// Eventually, it might be better to create a separate device for these, with rom_r and ram_r/ram_w handlers

READ8_MEMBER(sns_rom_strom_device::read_l)
{
	if (offset < 0x200000)
	{
		int bank = offset / 0x10000;
		return m_rom[rom_bank_map[bank] * 0x8000 + (offset & 0x7fff)];
	}
	return 0xff;
}

READ8_MEMBER(sns_rom_strom_device::read_h)
{
	if (offset < 0x40000)
	{
		int bank = offset / 0x10000;
		return m_nvram[bank * 0x8000 + (offset & 0x7fff)];
	}
	return 0xff;
}

WRITE8_MEMBER(sns_rom_strom_device::write_l)
{
}

WRITE8_MEMBER(sns_rom_strom_device::write_h)
{
	if (offset < 0x40000)
	{
		int bank = offset / 0x10000;
		m_nvram[bank * 0x8000 + (offset & 0x7fff)] = data;
	}
}


