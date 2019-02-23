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
	ROM_DEFAULT_BIOS("exp100")
	ROM_SYSTEM_BIOS(0, "exp100", "ROMBOX+ Expansion 1.00")
	ROMX_LOAD("romboxplus.rom", 0x0000, 0x2000, CRC(0520ab6d) SHA1(2f551bea279a64e09fd4d31024799f7459fb9938), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "presap2", "PRES AP2 Support 1.23")
	ROMX_LOAD("presap2rb_123.rom", 0x0000, 0x2000, CRC(04931d2c) SHA1(84a27fd30adea4e7f7c53e7875f63cf9e6928688), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "exp101", "Slogger Expansion 1.01")
	ROMX_LOAD("exprom101.rom", 0x0000, 0x2000, CRC(6f854419) SHA1(1f3e7e0c2843e1a364b4b3f96c890fe70ef03200), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "exp200", "Slogger Expansion 2.00")
	ROMX_LOAD("elkexp200.rom", 0x0000, 0x2000, CRC(dee02843) SHA1(5c9b940b4ddb46e9a223160310683a32266300c8), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "exp201", "Slogger Expansion 2.01")
	ROMX_LOAD("elkexp201.rom", 0x0000, 0x2000, CRC(0e896892) SHA1(4e0794f1083fe529b01bd4fa100996a533ed8b10), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "exp202", "Slogger Expansion 2.02")
	ROMX_LOAD("elkexp202.rom", 0x0000, 0x2000, CRC(32b440be) SHA1(dbc73e8d919c5615d0241d99db60e06324e16c86), ROM_BIOS(5))

	ROM_SYSTEM_BIOS(6, "exp210", "Slogger Expansion 2.10 (dev)")
	ROMX_LOAD("elkexp210.rom", 0x0000, 0x2000, CRC(12442575) SHA1(eb8609991a9a8fb017b8100bfca4248d65faeea8), ROM_BIOS(6))
ROM_END

//-------------------------------------------------
//  INPUT_PORTS( romboxp )
//-------------------------------------------------

static INPUT_PORTS_START( romboxp )
	PORT_START("OPTION")
	PORT_CONFNAME(0x01, 0x01, "A1") // not implemented
	PORT_CONFSETTING(0x00, "RAM")
	PORT_CONFSETTING(0x01, "ROM")
	PORT_CONFNAME(0x02, 0x02, "A2")
	PORT_CONFSETTING(0x00, "ROM 12-15")
	PORT_CONFSETTING(0x02, "ROM 4-7")
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
	/* printer */
	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(electron_romboxp_device::busy_w));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	/* rom sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 4/12
	m_rom[0]->set_device_load(device_image_load_delegate(&electron_romboxp_device::device_image_load_rom1_load, this));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 5/13
	m_rom[1]->set_device_load(device_image_load_delegate(&electron_romboxp_device::device_image_load_rom2_load, this));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 6/14 also ROM/RAM
	m_rom[2]->set_device_load(device_image_load_delegate(&electron_romboxp_device::device_image_load_rom3_load, this));
	GENERIC_SOCKET(config, m_rom[3], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 7/15
	m_rom[3]->set_device_load(device_image_load_delegate(&electron_romboxp_device::device_image_load_rom4_load, this));

	/* cartridges */
	ELECTRON_CARTSLOT(config, m_cart[0], DERIVED_CLOCK(1, 1), electron_cart, nullptr); // ROM SLOT 0/1
	m_cart[0]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_cart[0]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
	ELECTRON_CARTSLOT(config, m_cart[1], DERIVED_CLOCK(1, 1), electron_cart, nullptr); // ROM SLOT 2/3
	m_cart[1]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
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

electron_romboxp_device::electron_romboxp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ELECTRON_ROMBOXP, tag, owner, clock),
	device_electron_expansion_interface(mconfig, *this),
	m_exp_rom(*this, "exp_rom"),
	m_rom(*this, "rom%u", 1),
	m_cart(*this, "cart%u", 1),
	m_centronics(*this, "centronics"),
	m_cent_data_out(*this, "cent_data_out"),
	m_option(*this, "OPTION"),
	m_romsel(0),
	m_rom_base(0),
	m_centronics_busy(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_romboxp_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_romboxp_device::device_reset()
{
	m_rom_base = (m_option->read() & 0x02) ? 4 : 12;
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_romboxp_device::expbus_r(address_space &space, offs_t offset)
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
			data = m_cart[1]->read(space, offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			data = m_cart[0]->read(space, offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			if (m_rom_base == 4)
			{
				data = m_rom[m_romsel - 4]->read_rom(offset & 0x3fff);
			}
			break;
		case 12:
			data = m_exp_rom->base()[offset & 0x1fff];
			break;
		case 13:
			data &= m_cart[0]->read(space, offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			data &= m_cart[1]->read(space, offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
		case 14:
		case 15:
			if (m_rom_base == 12)
			{
				data = m_rom[m_romsel - 12]->read_rom(offset & 0x3fff);
			}
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			data &= m_cart[0]->read(space, offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(space, offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc72)
			{
				data &= status_r(space, offset);
			}
			break;

		case 0xfd:
			data &= m_cart[0]->read(space, offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(space, offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			break;
		}
	}

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_romboxp_device::expbus_w(address_space &space, offs_t offset, uint8_t data)
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
			m_cart[1]->write(space, offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 2:
		case 3:
			m_cart[0]->write(space, offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			m_cart[0]->write(space, offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);
			m_cart[1]->write(space, offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset == 0xfc71)
			{
				m_cent_data_out->write(data);
			}
			break;

		case 0xfd:
			m_cart[0]->write(space, offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
			m_cart[1]->write(space, offset & 0xff, data, 0, 1, m_romsel & 0x01, 0, 0);
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

READ8_MEMBER(electron_romboxp_device::status_r)
{
	// Status: b7: printer Busy
	return (m_centronics_busy << 7) | 0x7f;
}


WRITE_LINE_MEMBER(electron_romboxp_device::busy_w)
{
	m_centronics_busy = !state;
}


image_init_result electron_romboxp_device::load_rom(device_image_interface &image, generic_slot_device *slot)
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
