// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Rombox

    The Rombox offers the following facilities
    - 8 Sideways ROM / RAM slots

**********************************************************************/


#include "emu.h"
#include "rombox.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_ROMBOX, electron_rombox_device, "electron_rombox", "Slogger Rombox")


//-------------------------------------------------
//  INPUT_PORTS( rombox )
//-------------------------------------------------

static INPUT_PORTS_START( rombox )
	PORT_START("OPTION")
	PORT_CONFNAME(0x01, 0x00, "A1") // not implemented
	PORT_CONFSETTING(0x00, "ROM")
	PORT_CONFSETTING(0x01, "RAM")
	PORT_CONFNAME(0x02, 0x00, "A2") // not implemented
	PORT_CONFSETTING(0x00, "8K or 16K")
	PORT_CONFSETTING(0x02, "4K")
	PORT_CONFNAME(0x04, 0x00, "B")
	PORT_CONFSETTING(0x00, "ROM 0,1,2,3")
	PORT_CONFSETTING(0x04, "ROM 12,13,14,15")
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor electron_rombox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( rombox );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_rombox_device::device_add_mconfig(machine_config &config)
{
	/* rom sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[0]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom1_load, this));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[1]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom2_load, this));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[2]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom3_load, this));
	GENERIC_SOCKET(config, m_rom[3], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[3]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom4_load, this));
	GENERIC_SOCKET(config, m_rom[4], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[4]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom5_load, this));
	GENERIC_SOCKET(config, m_rom[5], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[5]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom6_load, this));
	GENERIC_SOCKET(config, m_rom[6], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[6]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom7_load, this));
	GENERIC_SOCKET(config, m_rom[7], generic_plain_slot, "electron_rom", "bin,rom");
	m_rom[7]->set_device_load(device_image_load_delegate(&electron_rombox_device::device_image_load_rom8_load, this));

	/* pass-through */
	ELECTRON_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), electron_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_rombox_device - constructor
//-------------------------------------------------

electron_rombox_device::electron_rombox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_ROMBOX, tag, owner, clock),
		device_electron_expansion_interface(mconfig, *this),
	m_exp(*this, "exp"),
	m_rom(*this, "rom%u", 1),
	m_option(*this, "OPTION"),
	m_romsel(0),
	m_rom_base(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_rombox_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void electron_rombox_device::device_reset()
{
	m_rom_base = (m_option->read() & 0x04) ? 12 : 0;
}

//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_rombox_device::expbus_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (offset >= 0x8000 && offset < 0xc000)
	{
		switch (m_romsel)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			if (m_rom_base == 0 && m_rom[m_romsel + 4]->exists())
			{
				data = m_rom[m_romsel + 4]->read_rom(offset & 0x3fff);
			}
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			if (m_rom[m_romsel - 4]->exists())
			{
				data = m_rom[m_romsel - 4]->read_rom(offset & 0x3fff);
			}
			break;
		case 12:
		case 13:
		case 14:
		case 15:
			if (m_rom_base == 12 && m_rom[m_romsel - 8]->exists())
			{
				data = m_rom[m_romsel - 8]->read_rom(offset & 0x3fff);
			}
			break;
		}
	}

	data &= m_exp->expbus_r(offset);

	return data;
}

//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_rombox_device::expbus_w(offs_t offset, uint8_t data)
{
	m_exp->expbus_w(offset, data);

	if (offset == 0xfe05)
	{
		m_romsel = data & 0x0f;
	}
}

//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_rombox_device::load_rom(device_image_interface &image, generic_slot_device *slot)
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
