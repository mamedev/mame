// license:BSD-3-Clause
// copyright-holders:David Haywood

// this assumes that the regular banked carts don't also have the 4-in-1 logic.

#include "emu.h"
#include "rom.h"

//-------------------------------------------------
//  gamate_rom_device - constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(GAMATE_ROM_PLAIN,         gamate_rom_plain_device,       "gamate_rom_plain",        "GAMATE Cartridge")
DEFINE_DEVICE_TYPE(GAMATE_ROM_BANKED,        gamate_rom_banked_device,      "gamate_rom_banked",       "GAMATE Cartridge with banking")
DEFINE_DEVICE_TYPE(GAMATE_ROM_4IN1,          gamate_rom_4in1_device,        "gamate_rom_4in1",         "GAMATE 4-in-1 Cartridge")


gamate_rom_plain_device::gamate_rom_plain_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock), device_gamate_cart_interface(mconfig, *this),
	m_protection(*this, "protection")
{
}

gamate_rom_plain_device::gamate_rom_plain_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	gamate_rom_plain_device(mconfig, GAMATE_ROM_PLAIN, tag, owner, clock)
{
}

gamate_rom_banked_device::gamate_rom_banked_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	gamate_rom_plain_device(mconfig, GAMATE_ROM_BANKED, tag, owner, clock),
	m_bank(0)
{
}

gamate_rom_banked_device::gamate_rom_banked_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	gamate_rom_plain_device(mconfig, type, tag, owner, clock),
	m_bank(0)
{
}


gamate_rom_4in1_device::gamate_rom_4in1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	gamate_rom_banked_device(mconfig, GAMATE_ROM_4IN1, tag, owner, clock),
	m_multibank(0)
{
}

//-------------------------------------------------
//  mapper specific start/reset
//-------------------------------------------------

void gamate_rom_banked_device::device_start()
{
	save_item(NAME(m_bank));
}

void gamate_rom_4in1_device::device_start()
{
	gamate_rom_banked_device::device_start();
	save_item(NAME(m_multibank));
}


void gamate_rom_banked_device::device_reset()
{
	m_bank = 0;
}

void gamate_rom_4in1_device::device_reset()
{
	gamate_rom_banked_device::device_reset();
	m_multibank = 0;
}

/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/


READ8_MEMBER(gamate_rom_plain_device::read_cart)
{
	if (m_protection->is_protection_passed())
	{
		return read_rom(space, offset, mem_mask);
	}
	else
	{
		return m_protection->prot_r() << 1;
	}

	return 0xff;
}

READ8_MEMBER(gamate_rom_plain_device::read_rom)
{
	return m_rom[offset & (m_rom_size-1)];
}

READ8_MEMBER(gamate_rom_banked_device::read_rom)
{
	if (offset < 0x4000)
	{
		return m_rom[offset & (m_rom_size - 1)];
	}
	else
	{
		return m_rom[((m_bank * 0x4000) + (offset & 0x3fff)) & (m_rom_size - 1)];
	}

	return 0xff;
}

READ8_MEMBER(gamate_rom_4in1_device::read_rom)
{
	if (offset < 0x4000)
	{
		return m_rom[((m_multibank * 0x4000) + (offset & 0x3fff)) & (m_rom_size - 1)];
	}
	else
	{
		return m_rom[((m_bank * 0x4000) + (offset & 0x3fff)) & (m_rom_size - 1)];
	}
}

WRITE8_MEMBER(gamate_rom_plain_device::write_cart)
{
	if (m_protection->is_protection_passed())
	{
		write_rom(space, offset, data, mem_mask);
	}
	else
	{
		m_protection->prot_w((data & 0x04) >> 2);
	}
}

WRITE8_MEMBER(gamate_rom_plain_device::write_rom)
{
	// shouldn't be any write on an unbanked game
	logerror("gamate_rom_plain_device::write_rom %04x %02x\n", offset, data);
}

WRITE8_MEMBER(gamate_rom_banked_device::write_rom)
{
	if (offset == 0x6000)
	{
		m_bank = data;
	}
	else
	{
		logerror("gamate_rom_banked_device::write_rom %04x %02x\n", offset, data);
	}
}

WRITE8_MEMBER(gamate_rom_4in1_device::write_rom)
{
	if (offset == 0x2000)
	{
		m_multibank = data;
	}
	else if (offset == 0x6000)
	{
		m_bank = data;
	}
	else
	{
		logerror("gamate_rom_4in1_device::write_rom %04x %02x\n", offset, data);
	}
}

void gamate_rom_plain_device::device_add_mconfig(machine_config &config)
{
	GAMATE_PROT(config, m_protection, 0);
}


/*-------------------------------------------------
 slot interface
 -------------------------------------------------*/

void gamate_cart(device_slot_interface &device)
{
	device.option_add_internal("plain",       GAMATE_ROM_PLAIN);
	device.option_add_internal("banked",      GAMATE_ROM_BANKED);
	device.option_add_internal("4in1",        GAMATE_ROM_4IN1);
}
