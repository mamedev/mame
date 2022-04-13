// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Computech Integra-B expansion board

    Usage:
    To enable the computer to acknowledge the INTEGRA-β Expansion on the
    first switch-on you must hold down the '@' key whilst switching on
    the computer, or CTRL-@-BREAK to reset.

    If default language has not been configured then reset the board and:
    *CONFIGURE LANG 14

**********************************************************************/


#include "emu.h"
#include "integrab.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(BBC_INTEGRAB, bbc_integrab_device, "bbc_integrab", u8"Computech Integra-β")


//-------------------------------------------------
//  INPUT_PORTS( integrab )
//-------------------------------------------------

static INPUT_PORTS_START(integrab)
	PORT_START("wp")
	PORT_CONFNAME(0x01, 0x00, "Write Protect Sideways RAM Banks 4/5")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x01, DEF_STR(Yes))
	PORT_CONFNAME(0x02, 0x00, "Write Protect Sideways RAM Bank 6/7")
	PORT_CONFSETTING(0x00, DEF_STR(No))
	PORT_CONFSETTING(0x02, DEF_STR(Yes))
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor bbc_integrab_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(integrab);
}

//-------------------------------------------------
//  ROM( integrab )
//-------------------------------------------------

ROM_START(integrab)
	ROM_REGION(0x4000, "ibos", 0)
	ROM_SYSTEM_BIOS(0, "120p", "IBOS 1.20 (Y2K patched)")
	ROMX_LOAD("ibos120p.rom", 0x0000, 0x4000, CRC(21e7a2c2) SHA1(d5ff79da2243aabd363ee26a1e7ad546657fdeb5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "120", "IBOS 1.20")
	ROMX_LOAD("ibos120.rom", 0x0000, 0x4000, CRC(52b1b8c4) SHA1(0f5b2a5bee23808b34eab2f027f7c1e77395c782), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "114", "IBOS 1.14")
	ROMX_LOAD("ibos114.rom", 0x0000, 0x4000, CRC(d05ce376) SHA1(b2b7fc3258936296f83d720759caacba5378edec), ROM_BIOS(2))
ROM_END

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void bbc_integrab_device::device_add_mconfig(machine_config &config)
{
	/* rtc */
	MC146818(config, m_rtc, 32.768_kHz_XTAL); // CDP6818
	m_rtc->irq().set(DEVICE_SELF_OWNER, FUNC(bbc_internal_slot_device::irq_w));

	/* rom/ram sockets */
	BBC_ROMSLOT16(config, m_rom[4], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[5], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[6], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[7], bbc_rom_devices, "nvram").set_fixed_ram(true);
	BBC_ROMSLOT16(config, m_rom[8], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[9], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[10], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[11], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[12], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[13], bbc_rom_devices, nullptr);
	BBC_ROMSLOT16(config, m_rom[14], bbc_rom_devices, nullptr);

	/* battery-backed ram */
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *bbc_integrab_device::device_rom_region() const
{
	return ROM_NAME( integrab );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bbc_integrab_device - constructor
//-------------------------------------------------

bbc_integrab_device::bbc_integrab_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BBC_INTEGRAB, tag, owner, clock)
	, device_bbc_internal_interface(mconfig, *this)
	, m_ibos(*this, "ibos")
	, m_rom(*this, "romslot%u", 0U)
	, m_nvram(*this, "nvram")
	, m_rtc(*this, "rtc")
	, m_wp(*this, "wp")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bbc_integrab_device::device_start()
{
	m_shadow = false;
	m_ram = make_unique_clear<uint8_t[]>(0x8000);
	m_nvram->set_base(m_ram.get(), 0x8000);

	/* move internal roms to board */
	memmove(m_region_swr->base() + 0x2c000, m_region_swr->base(), 0x10000);
	memset(m_region_swr->base(), 0xff, 0x10000);
	memcpy(m_region_swr->base() + 0x3c000, m_ibos->base(), 0x4000);

	/* register for save states */
	save_item(NAME(m_romsel));
	save_item(NAME(m_ramsel));
	save_item(NAME(m_shadow));
	save_pointer(NAME(m_ram), 0x8000);
}

void bbc_integrab_device::device_reset()
{
	m_romsel = 0;
	m_ramsel = 0;
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

uint8_t bbc_integrab_device::romsel_r(offs_t offset)
{
	uint8_t data = 0xfe;

	switch (offset & 0x0c)
	{
	case 0x08:
		data = m_rtc->read(0);
		break;
	case 0x0c:
		data = m_rtc->read(1);
		break;
	}

	return data;
}

void bbc_integrab_device::romsel_w(offs_t offset, uint8_t data)
{
	/*
	 ROMSEL & ROMID
	  Bits 0,3 ROMNUM Sideways ROM/RAM select bits
	  Bits 4,5        Not used
	  Bit 6    PRVEN  Private RAM enable
	  Bit 7    MEMSEL Shadow/Main RAM toggle

	 RAMSEL & RAMID
	  Bits 0,1 AUX0,1 Not used but must be preserved
	  Bits 2,3        Not used
	  Bit 4    PRVS8  Private RAM 8K area select (&9000-&AFFF)
	  Bit 5    PRVS4  Private RAM 4K area select (&8000-&8FFF)
	  Bit 6    PRVS1  Private RAM 1K area select (&8000-&83FF)
	  Bit 7    SHEN   Shadow RAM enable bit
	*/
	switch (offset & 0x0c)
	{
	case 0x00:
		m_romsel = data;
		break;
	case 0x04:
		m_ramsel = data;
		break;
	case 0x08:
		m_rtc->write(0, data);
		break;
	case 0x0c:
		m_rtc->write(1, data);
		break;
	}

	m_shadow = BIT(m_ramsel, 7) && !BIT(m_romsel, 7);
}

uint8_t bbc_integrab_device::ram_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_shadow && offset >= 0x3000)
		data = m_ram[offset];
	else
		data = m_mb_ram->pointer()[offset];

	return data;
}

void bbc_integrab_device::ram_w(offs_t offset, uint8_t data)
{
	if (m_shadow && offset >= 0x3000)
		m_ram[offset] = data;
	else
		m_mb_ram->pointer()[offset] = data;
}

uint8_t bbc_integrab_device::paged_r(offs_t offset)
{
	uint8_t data = 0xff;
	uint8_t romsel = m_romsel & 0x0f;

	switch (romsel)
	{
	case 0: case 1: case 2: case 3:
		/* motherboard rom sockets */
		if (m_mb_rom[romsel] && m_mb_rom[romsel]->present())
		{
			data = m_mb_rom[romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (romsel << 14)];
		}
		break;
	case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
		/* expansion board sockets */
		if (m_rom[romsel] && m_rom[romsel]->present())
		{
			data = m_rom[romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (romsel << 14)];
		}
		break;
	}

	/* private ram */
	if (BIT(m_romsel, 6))
	{
		switch (offset & 0x3c00)
		{
		case 0x0000:
			/* PRVS1 */
			if (m_ramsel & 0x60)
				data = m_ram[offset];
			break;
		case 0x0400: case 0x0800: case 0x0c00:
			/* PRVS4 */
			if (m_ramsel & 0x20)
				data = m_ram[offset];
			break;
		case 0x1000: case 0x1400: case 0x1800: case 0x1c00: case 0x2000: case 0x2400: case 0x2800: case 0x2c00:
			/* PRVS8 */
			if (m_ramsel & 0x10)
				data = m_ram[offset];
			break;
		}
	}

	return data;
}

void bbc_integrab_device::paged_w(offs_t offset, uint8_t data)
{
	uint8_t romsel = m_romsel & 0x0f;

	/* private ram */
	if (BIT(m_romsel, 6))
	{
		switch (offset & 0x3c00)
		{
		case 0x0000:
			/* PRVS1 */
			if (m_ramsel & 0x60)
				m_ram[offset] = data;
			break;
		case 0x0400: case 0x0800: case 0x0c00:
			/* PRVS4 */
			if (m_ramsel & 0x20)
				m_ram[offset] = data;
			break;
		case 0x1000: case 0x1400: case 0x1800: case 0x1c00: case 0x2000: case 0x2400: case 0x2800: case 0x2c00:
			/* PRVS8 */
			if (m_ramsel & 0x10)
				m_ram[offset] = data;
			break;
		}
	}
	else
	{
		switch (romsel)
		{
		case 0: case 1: case 2: case 3:
			/* motherboard rom sockets */
			if (m_mb_rom[romsel])
			{
				m_mb_rom[romsel]->write(offset, data);
			}
			break;
		case 4: case 5:
			/* expansion board sockets */
			if (m_rom[romsel] && !BIT(m_wp->read(), 0))
			{
				m_rom[romsel]->write(offset, data);
			}
			break;
		case 6: case 7:
			/* expansion board sockets */
			if (m_rom[romsel] && !BIT(m_wp->read(), 1))
			{
				m_rom[romsel]->write(offset, data);
			}
			break;
		case 8: case 9: case 10: case 11: case 12: case 13: case 14:
			/* expansion board sockets */
			if (m_rom[romsel])
			{
				m_rom[romsel]->write(offset, data);
			}
			break;
		}
	}
}
