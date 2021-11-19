// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Rombox Plus

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Slogger_RomBoxPlus.html

    The Electron Rombox+ by Slogger has been designed to be
    compatible with the Acorn Plus 1 but with the added facility to allow
    the popular ROM based software to be used on the Electron microcomputer.
    The Rombox+ offers the following facilities
    - 2 cartridge slots
    - 4 Sideways ROM / RAM slots
    - Centronics Printer interface

**********************************************************************/


#include "emu.h"
#include "romboxp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ROMBOXP, electron_romboxp_device, "electron_romboxp", "Slogger Rombox+")

//-------------------------------------------------
//  ROM( romboxp )
//-------------------------------------------------

ROM_START( romboxp )
	// Bank 12 Expansion module operating system
	ROM_REGION(0x2000, "exp_rom", 0)
	ROM_DEFAULT_BIOS("exp202")
	ROM_SYSTEM_BIOS(0, "exp100", "ROMBOX+ Expansion 1.00")
	ROMX_LOAD("romboxplus.rom", 0x0000, 0x2000, CRC(0520ab6d) SHA1(2f551bea279a64e09fd4d31024799f7459fb9938), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "exp101", "Slogger Expansion 1.01")
	ROMX_LOAD("exprom101.rom", 0x0000, 0x2000, CRC(6f854419) SHA1(1f3e7e0c2843e1a364b4b3f96c890fe70ef03200), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "exp200", "Slogger Expansion 2.00")
	ROMX_LOAD("elkexp200.rom", 0x0000, 0x2000, CRC(dee02843) SHA1(5c9b940b4ddb46e9a223160310683a32266300c8), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "exp201", "Slogger Expansion 2.01")
	ROMX_LOAD("elkexp201.rom", 0x0000, 0x2000, CRC(0e896892) SHA1(4e0794f1083fe529b01bd4fa100996a533ed8b10), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "exp202", "Slogger Expansion 2.02")
	ROMX_LOAD("elkexp202.rom", 0x0000, 0x2000, CRC(32b440be) SHA1(dbc73e8d919c5615d0241d99db60e06324e16c86), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "exp210", "Slogger Expansion 2.10 (dev)")
	ROMX_LOAD("elkexp210.rom", 0x0000, 0x2000, CRC(12442575) SHA1(eb8609991a9a8fb017b8100bfca4248d65faeea8), ROM_BIOS(5))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( romboxp )
//-------------------------------------------------

static INPUT_PORTS_START( romboxp )
	PORT_START("OPTION")
	PORT_CONFNAME(0x01, 0x01, "Sideways ROM Pages")
	PORT_CONFSETTING(0x00, "12-15")
	PORT_CONFSETTING(0x01, "4-7")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_romboxp_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( romboxp );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_romboxp_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set([this](int state) { m_centronics_busy = state; });
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* rom sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 4/12
	m_rom[0]->set_device_load(FUNC(electron_romboxp_device::rom1_load));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 5/13
	m_rom[1]->set_device_load(FUNC(electron_romboxp_device::rom2_load));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 6/14 also ROM/RAM
	m_rom[2]->set_device_load(FUNC(electron_romboxp_device::rom3_load));
	GENERIC_SOCKET(config, m_rom[3], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 7/15
	m_rom[3]->set_device_load(FUNC(electron_romboxp_device::rom4_load));

	/* cartridges */
	ELECTRON_CARTSLOT(config, m_cart[0], DERIVED_CLOCK(1, 1), electron_cart, nullptr); // ROM SLOT 0/1
	m_cart[0]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));
	m_cart[0]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
	ELECTRON_CARTSLOT(config, m_cart[1], DERIVED_CLOCK(1, 1), electron_cart, nullptr); // ROM SLOT 2/3
	m_cart[1]->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_cart[1]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}

const tiny_rom_entry *electron_romboxp_device::device_rom_region() const
{
	return ROM_NAME( romboxp );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_romboxp_device - constructor
//-------------------------------------------------

electron_romboxp_device::electron_romboxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ROMBOXP, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_irqs(*this, "irqs")
	, m_exp_rom(*this, "exp_rom")
	, m_rom(*this, "rom%u", 1)
	, m_cart(*this, "cart%u", 1)
	, m_centronics(*this, "centronics")
	, m_cent_data_out(*this, "cent_data_out")
	, m_option(*this, "OPTION")
	, m_romsel(0)
	, m_rom_base(0)
	, m_centronics_busy(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_romboxp_device::device_start()
{
	m_ram = make_unique_clear<uint8_t[]>(0x10000);
	memset(m_ram.get(), 0xff, 0x10000);

	/* register for save states */
	save_item(NAME(m_romsel));
	save_pointer(NAME(m_ram), 0x10000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_romboxp_device::device_reset()
{
	m_rom_base = m_option->read() ? 4 : 12;
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_romboxp_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset >> 12)
	{
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
		switch (m_romsel)
		{
		case 0:
		case 1:
			data = m_cart[1]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			data = m_cart[0]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			if (m_rom_base == 4)
			{
				if (m_rom[m_romsel - 4]->exists())
					data = m_rom[m_romsel - 4]->read_rom(offset & 0x3fff);
				else
					data = m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)];
			}
			break;
		case 12:
			data = m_exp_rom->base()[offset & 0x1fff];
			break;
		case 13:
			data &= m_cart[0]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			data &= m_cart[1]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			[[fallthrough]];
		case 14:
		case 15:
			if (m_rom_base == 12)
			{
				if (m_rom[m_romsel - 12]->exists())
					data = m_rom[m_romsel - 12]->read_rom(offset & 0x3fff);
				else
					data = m_ram[(m_romsel - 12) << 14 | (offset & 0x3fff)];
			}
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			data &= m_cart[0]->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc72)
			{
				data &= (m_centronics_busy << 7) | 0x7f;
			}
			break;

		case 0xfd:
			data &= m_cart[0]->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_romboxp_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
		switch (m_romsel)
		{
		case 0:
		case 1:
			m_cart[1]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			m_cart[0]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			if (m_rom_base == 4)
			{
				if (!m_rom[m_romsel - 4]->exists())
					m_ram[(m_romsel - 4) << 14 | (offset & 0x3fff)] = data;
			}
			break;
		case 12:
			break;
		case 13:
			m_cart[0]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 0, 1);
			m_cart[1]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 0, 1);
			[[fallthrough]];
		case 14:
		case 15:
			if (m_rom_base == 12)
			{
				if (!m_rom[m_romsel - 12]->exists())
					m_ram[(m_romsel - 12) << 14 | (offset & 0x3fff)] = data;
			}
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			m_cart[0]->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);
			m_cart[1]->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc71)
			{
				m_cent_data_out->write(data);
				m_centronics->write_strobe(0);
				m_centronics->write_strobe(1);
			}
			break;

		case 0xfd:
			m_cart[0]->write(offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
			m_cart[1]->write(offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
			break;

		case 0xfe:
			if (offset == 0xfe05)
			{
				m_romsel = data & 0x0f;
			}
			break;
		}
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_romboxp_device::load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(image_error::INVALIDIMAGE, "Invalid size: Only 8K/16K is supported");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(0x4000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	// mirror 8K ROMs
	uint8_t *crt = slot->get_rom_base();
	if (size <= 0x2000) memcpy(crt + 0x2000, crt, 0x2000);

	return image_init_result::PASS;
}
