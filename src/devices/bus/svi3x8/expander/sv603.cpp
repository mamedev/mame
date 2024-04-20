// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SV-603 Coleco Game Adapter for SVI-318/328

***************************************************************************/

#include "emu.h"
#include "sv603.h"
#include "softlist_dev.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SV603, sv603_device, "sv603", "SV-603 Coleco Game Adapter")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( sv603 )
	ROM_REGION(0x2000, "bios", 0)
	ROM_LOAD("sv603.ic10", 0x0000, 0x2000, CRC(19e91b82) SHA1(8a30abe5ffef810b0f99b86db38b1b3c9d259b78))
ROM_END

const tiny_rom_entry *sv603_device::device_rom_region() const
{
	return ROM_NAME( sv603 );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sv603_device::device_add_mconfig(machine_config &config)
{
	SN76489A(config, m_snd, XTAL(10'738'635) / 3);
	m_snd->add_route(ALL_OUTPUTS, DEVICE_SELF_OWNER, 1.0);

	// controller ports
	COLECOVISION_CONTROL_PORT(config, m_joy[0], colecovision_control_port_devices, "hand");
	m_joy[0]->irq().set(FUNC(sv603_device::joy_irq_w<0>));
	COLECOVISION_CONTROL_PORT(config, m_joy[1], colecovision_control_port_devices, nullptr);
	m_joy[1]->irq().set(FUNC(sv603_device::joy_irq_w<1>));

	// cartridge slot
	COLECOVISION_CARTRIDGE_SLOT(config, m_cart, colecovision_cartridges, nullptr);
	SOFTWARE_LIST(config, "cart_list").set_original("coleco");
	SOFTWARE_LIST(config, "homebrew_list").set_original("coleco_homebrew");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sv603_device - constructor
//-------------------------------------------------

sv603_device::sv603_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SV603, tag, owner, clock),
	device_svi_expander_interface(mconfig, *this),
	m_bios(*this, "bios"),
	m_snd(*this, "snd"),
	m_joy{ {*this, "joy1"}, {*this, "joy2"} },
	m_cart(*this, COLECOVISION_CARTRIDGE_SLOT_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sv603_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sv603_device::device_reset()
{
	m_expander->ctrl1_w(0);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

template<int N>
void sv603_device::joy_irq_w(int state)
{
	m_expander->int_w(state);
}

uint8_t sv603_device::mreq_r(offs_t offset)
{
	uint8_t data = 0xff;

	// ls138 (active low)
	const int ccs1 = ((offset >> 13) == 0) ? 0 : 1;
	const int ccs2 = ((offset >> 13) == 1) ? 0 : 1;
	const int ccs3 = ((offset >> 13) == 2) ? 0 : 1;
	const int ccs4 = ((offset >> 13) == 3) ? 0 : 1;
	const int bios = ((offset >> 13) == 4) ? 0 : 1;
	// 5, 6, 7: not connected

	m_expander->romdis_w(0);
	m_expander->ramdis_w(bios);

	data &= m_cart->read(offset, ccs1, ccs2, ccs3, ccs4);

	if (bios == 0)
		data &= m_bios->as_u8(offset & 0x1fff);

	return data;
}

void sv603_device::mreq_w(offs_t offset, uint8_t data)
{
	const int ccs1 = ((offset >> 13) == 0) ? 0 : 1;
	const int ccs2 = ((offset >> 13) == 1) ? 0 : 1;
	const int ccs3 = ((offset >> 13) == 2) ? 0 : 1;
	const int ccs4 = ((offset >> 13) == 3) ? 0 : 1;
	const int bios = ((offset >> 13) == 4) ? 0 : 1;

	m_expander->romdis_w(0);
	m_expander->ramdis_w(bios);

	m_cart->write(offset, data, ccs1, ccs2, ccs3, ccs4);
}

uint8_t sv603_device::iorq_r(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset & 0xe0)
	{
	case 0xa0:
		data = m_expander->excs_r(offset);
		break;

	case 0xe0:
		data = m_joy[BIT(offset, 1)]->read();
		break;
	}

	return data;
}

void sv603_device::iorq_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xe0)
	{
	case 0x80:
		// keypad mode
		m_joy[0]->common0_w(1);
		m_joy[0]->common1_w(0);
		m_joy[1]->common0_w(1);
		m_joy[1]->common1_w(0);
		break;

	case 0xa0:
		m_expander->excs_w(offset, data);
		break;

	case 0xc0:
		// joystick mode
		m_joy[0]->common0_w(0);
		m_joy[0]->common1_w(1);
		m_joy[1]->common0_w(0);
		m_joy[1]->common1_w(1);
		break;

	case 0xe0:
		m_snd->write(data);
		break;
	}
}
