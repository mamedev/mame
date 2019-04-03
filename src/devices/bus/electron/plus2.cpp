// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Slogger Plus 2

    The Plus 2 interface from Slogger has been designed to compliment
    the Slogger Rombox Plus and Acorn Plus 1 by offering further
    expansion capabilities. This has been achieved by providing two
    extra cartridge slots, three ROM sockets and connections for a
    Usr Port and even further expansion via two expansion points.

**********************************************************************/


#include "emu.h"
#include "plus2.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_PLUS2, electron_plus2_device, "electron_plus2", "Slogger Plus 2 Expansion")



//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_plus2_device::device_add_mconfig(machine_config &config)
{
	/* rom sockets */
	GENERIC_SOCKET(config, m_rom[0], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 13
	m_rom[0]->set_device_load(device_image_load_delegate(&electron_plus2_device::device_image_load_rom1_load, this));
	GENERIC_SOCKET(config, m_rom[1], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 14
	m_rom[1]->set_device_load(device_image_load_delegate(&electron_plus2_device::device_image_load_rom2_load, this));
	GENERIC_SOCKET(config, m_rom[2], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 15
	m_rom[2]->set_device_load(device_image_load_delegate(&electron_plus2_device::device_image_load_rom3_load, this));

	/* cartridges */
	ELECTRON_CARTSLOT(config, m_cart[0], DERIVED_CLOCK(1, 1), electron_cart, nullptr);
	m_cart[0]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_cart[0]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
	ELECTRON_CARTSLOT(config, m_cart[1], DERIVED_CLOCK(1, 1), electron_cart, nullptr);
	m_cart[1]->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_cart[1]->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));

	/* via */
	VIA6522(config, m_via, DERIVED_CLOCK(1, 16));
	m_via->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));

	/* pass-through */
	ELECTRON_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), electron_expansion_devices, nullptr);
	m_exp->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::irq_w));
	m_exp->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_expansion_slot_device::nmi_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_plus2_device - constructor
//-------------------------------------------------

electron_plus2_device::electron_plus2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_PLUS2, tag, owner, clock)
	, device_electron_expansion_interface(mconfig, *this)
	, m_exp(*this, "exp")
	, m_via(*this, "via6522")
	, m_rom(*this, "rom%u", 1)
	, m_cart(*this, "cart%u", 1)
	, m_userport(*this, "userport")
	, m_romsel(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_plus2_device::device_start()
{
}


//-------------------------------------------------
//  expbus_r - expansion data read
//-------------------------------------------------

uint8_t electron_plus2_device::expbus_r(offs_t offset)
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
		case 4:
		case 5:
			data = m_cart[1]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 6:
		case 7:
			data = m_cart[0]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 13:
			data &= m_cart[0]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
			data &= m_cart[1]->read(offset & 0x3fff, 0, 0, m_romsel & 0x01, 0, 1);
		case 14:
		case 15:
			data &= m_rom[m_romsel - 13]->read_rom(offset & 0x3fff);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			data &= m_cart[0]->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(offset & 0xff, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset >= 0xfcb0 && offset < 0xfcc0)
			{
				data &= m_via->read(offset & 0x0f);
			}
			break;

		case 0xfd:
			data &= m_cart[0]->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			data &= m_cart[1]->read(offset & 0xff, 0, 1, m_romsel & 0x01, 0, 0);
			break;
		}
	}

	data &= m_exp->expbus_r(offset);

	return data;
}


//-------------------------------------------------
//  expbus_w - expansion data write
//-------------------------------------------------

void electron_plus2_device::expbus_w(offs_t offset, uint8_t data)
{
	switch (offset >> 12)
	{
	case 0x8:
	case 0x9:
	case 0xa:
	case 0xb:
		switch (m_romsel)
		{
		case 4:
		case 5:
			m_cart[1]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		case 6:
		case 7:
			m_cart[0]->write(offset & 0x3fff, data, 0, 0, m_romsel & 0x01, 1, 0);
			break;
		}
		break;

	case 0xf:
		switch (offset >> 8)
		{
		case 0xfc:
			m_cart[0]->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);
			m_cart[1]->write(offset & 0xff, data, 1, 0, m_romsel & 0x01, 0, 0);

			if (offset >= 0xfcb0 && offset < 0xfcc0)
			{
				m_via->write(offset & 0x0f, data);
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

	m_exp->expbus_w(offset, data);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_plus2_device::load_rom(device_image_interface &image, generic_slot_device *slot)
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
