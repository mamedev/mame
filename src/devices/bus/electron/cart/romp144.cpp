// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    JAFA Systems ROMPlus-144

**********************************************************************/


#include "emu.h"
#include "romp144.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ROMP144, electron_romp144_device, "electron_romp144", "JAFA Systems ROMPlus-144")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_romp144_device::device_add_mconfig(machine_config &config)
{
	/* rom sockets */
	GENERIC_SOCKET(config, m_romslot[0], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[0]->set_device_load(FUNC(electron_romp144_device::rom0), this);
	GENERIC_SOCKET(config, m_romslot[1], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[1]->set_device_load(FUNC(electron_romp144_device::rom1), this);
	GENERIC_SOCKET(config, m_romslot[2], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[2]->set_device_load(FUNC(electron_romp144_device::rom2), this);
	GENERIC_SOCKET(config, m_romslot[3], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[3]->set_device_load(FUNC(electron_romp144_device::rom3), this);
	GENERIC_SOCKET(config, m_romslot[4], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[4]->set_device_load(FUNC(electron_romp144_device::rom4), this);
	GENERIC_SOCKET(config, m_romslot[5], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[5]->set_device_load(FUNC(electron_romp144_device::rom5), this);
	GENERIC_SOCKET(config, m_romslot[6], generic_plain_slot, "electron_rom", "bin,rom");
	m_romslot[6]->set_device_load(FUNC(electron_romp144_device::rom6), this);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_romp144_device - constructor
//-------------------------------------------------

electron_romp144_device::electron_romp144_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ROMP144, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_romslot(*this, "rom%u", 7)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_romp144_device::device_start()
{
	m_rom_select = 0xff;
	m_rom_latch = 0xff;

	save_item(NAME(m_rom_select));
	save_item(NAME(m_rom_latch));
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_romp144_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (oe)
	{
		if (romqa)
		{
			if ((m_rom_select & 0x07) == 0x00)
				data = m_ram[offset & 0x3fff];
			else
				data = m_romslot[(m_rom_select & 0x07) - 1]->read_rom(offset & 0x3fff);
		}
		else
		{
			if ((m_rom_select & 0x0f) == 0x08)
				data = m_ram[(offset & 0x3fff) | 0x4000];
			else
				data = m_rom[offset & 0x1fff];

			/* roms selected with a read to latch */
			if ((offset & 0x3f00) == 0x3f00)
			{
				m_rom_latch = offset & 0x0f;
			}
		}
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_romp144_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (oe)
	{
		if (romqa)
		{
			if ((m_rom_select & 0x07) == 0x00)
				m_ram[offset & 0x3fff] = data;
		}
		else
		{
			if ((m_rom_select & 0x0f) == 0x08)
				m_ram[(offset & 0x3fff) | 0x4000] = data;

			/* roms selected with a write to select */
			if ((offset & 0x3f00) == 0x3f00)
			{
				/* does the write match the read (upper RAM cannot be de-selected to protect *RSUBSTITUTE) */
				if (m_rom_latch == (offset & 0x0f) && m_rom_select != 0x08)
				{
					m_rom_select = m_rom_latch;
				}
			}
		}
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_romp144_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Invalid size: Only 8K/16K is supported");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *crt = slot->get_rom_base();
	if (size <= 0x2000) memcpy(crt + 0x2000, crt, 0x2000);

	return image_init_result::PASS;
}
