// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ACP Advanced Plus 5

**********************************************************************/


#include "emu.h"
#include "ap5.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ELECTRON_AP5, electron_ap5_device, "electron_ap5", "P.R.E.S. Advanced Plus 5")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void electron_ap5_device::device_add_mconfig(machine_config &config)
{
	/* rom sockets */
	GENERIC_SOCKET(config, m_romslot[0], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 14
	m_romslot[0]->set_device_load(device_image_load_delegate(&electron_ap5_device::device_image_load_rom1_load, this));
	GENERIC_SOCKET(config, m_romslot[1], generic_plain_slot, "electron_rom", "bin,rom"); // ROM SLOT 15
	m_romslot[1]->set_device_load(device_image_load_delegate(&electron_ap5_device::device_image_load_rom2_load, this));

	/* via */
	VIA6522(config, m_via, DERIVED_CLOCK(1, 16));
	m_via->readpb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_r));
	m_via->writepb_handler().set(m_userport, FUNC(bbc_userport_slot_device::pb_w));
	m_via->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));

	/* user port */
	BBC_USERPORT_SLOT(config, m_userport, bbc_userport_devices, nullptr);
	m_userport->cb1_handler().set(m_via, FUNC(via6522_device::write_cb1));
	m_userport->cb2_handler().set(m_via, FUNC(via6522_device::write_cb2));

	/* 1mhz bus port */
	BBC_1MHZBUS_SLOT(config, m_1mhzbus, DERIVED_CLOCK(1, 16), bbc_1mhzbus_devices, nullptr);
	m_1mhzbus->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
	m_1mhzbus->nmi_handler().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::nmi_w));

	/* tube port */
	BBC_TUBE_SLOT(config, m_tube, electron_tube_devices, nullptr);
	m_tube->irq_handler().set(DEVICE_SELF_OWNER, FUNC(electron_cartslot_device::irq_w));
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  electron_ap5_device - constructor
//-------------------------------------------------

electron_ap5_device::electron_ap5_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ELECTRON_AP5, tag, owner, clock)
	, device_electron_cart_interface(mconfig, *this)
	, m_via(*this, "via6522")
	, m_tube(*this, "tube")
	, m_1mhzbus(*this, "1mhzbus")
	, m_userport(*this, "userport")
	, m_romslot(*this, "rom%u", 1)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void electron_ap5_device::device_start()
{
}

//-------------------------------------------------
//  read - cartridge data read
//-------------------------------------------------

uint8_t electron_ap5_device::read(offs_t offset, int infc, int infd, int romqa, int oe, int oe2)
{
	uint8_t data = 0xff;

	if (infc)
	{
		data = m_1mhzbus->fred_r(offset);

		switch (offset & 0xf0)
		{
		case 0xb0:
			data &= m_via->read(offset & 0x0f);
			break;

		case 0xe0:
			data &= m_tube->host_r(offset & 0x0f);
			break;
		}
	}
	else if (infd)
	{
		data = m_1mhzbus->jim_r(offset);
	}
	else if (oe)
	{
		data = m_romslot[romqa]->read_rom(offset & 0x3fff);
	}
	else if (oe2)
	{
		data = m_rom[offset & 0x1fff];
	}

	return data;
}

//-------------------------------------------------
//  write - cartridge data write
//-------------------------------------------------

void electron_ap5_device::write(offs_t offset, uint8_t data, int infc, int infd, int romqa, int oe, int oe2)
{
	if (infc)
	{
		m_1mhzbus->fred_w(offset, data);

		switch (offset & 0xf0)
		{
		case 0xb0:
			m_via->write(offset & 0x0f, data);
			break;

		case 0xe0:
			m_tube->host_w(offset & 0x0f, data);
			break;
		}
	}
	else if (infd)
	{
		m_1mhzbus->jim_w(offset, data);
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

image_init_result electron_ap5_device::load_rom(device_image_interface &image, generic_slot_device *slot)
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
