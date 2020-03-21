// license:BSD-3-Clause
// copyright-holders:Roberto Fernandez,Nigel Barnes
// thanks-to:Ed Snider
/***************************************************************************

    coco_psg.cpp

    Code for emulating the CoCo PSG. A banked switched ROM and RAM cartridge
    with a YM2149 programmable sound generator and two digital Atari-compatible
    joystick ports.

    Cartridge by Ed Snider.

    CoCo PSG site:
      https://thezippsterzone.com/2018/05/08/coco-psg/

    CoCo PSG manual:
      https://thezippsterzone.com/wp-content/uploads/2018/05/coco-psg-users-manual.pdf

***************************************************************************/

#include "emu.h"
#include "coco_psg.h"
#include "speaker.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_PSG, device_cococart_interface, coco_psg_device, "coco_psg", "CoCo PSG")


//-------------------------------------------------
//  ROM( cocopsg )
//-------------------------------------------------

ROM_START(cocopsg)
	ROM_REGION(0x80000, "flash", 0)
	ROM_LOAD("psg_firmware_v1.bin", 0x0000, 0x80000, CRC(5de614f8) SHA1(9c7c8a5cc419ca1aca4d1c5e9f1195997c2bc95c))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_psg_device::device_rom_region() const
{
	return ROM_NAME(cocopsg);
}

//-------------------------------------------------
//  INPUT_PORTS( cocopsg )
//-------------------------------------------------

static INPUT_PORTS_START(cocopsg)
	PORT_START("GAMEPORT_A")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("GAMEPORT_B")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor coco_psg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(cocopsg);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void coco_psg_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker").front_center();
	YM2149(config, m_psg, 1_MHz_XTAL);
	m_psg->port_a_read_callback().set_ioport("GAMEPORT_A");
	m_psg->port_b_read_callback().set_ioport("GAMEPORT_B");
	m_psg->add_route(ALL_OUTPUTS, "speaker", 1.0);

	SST_39SF040(config, "flash");
}


//-------------------------------------------------
//  coco_psg_device - constructor
//-------------------------------------------------

coco_psg_device::coco_psg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COCO_PSG, tag, owner, clock)
	, device_cococart_interface(mconfig, *this)
	, m_psg(*this, "psg")
	, m_flash(*this, "flash")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_psg_device::device_start()
{
	m_sram = std::make_unique<uint8_t[]>(0x80000);

	/* registers are set to a default of zero */
	m_bank[0] = 0x00;
	m_bank[1] = 0x00;
	m_control = 0x00;

	set_line_value(line::CART, line_value::Q);

	/* register for save states */
	save_item(NAME(m_bank));
	save_item(NAME(m_control));
	save_pointer(NAME(m_sram), 0x80000);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_psg_device::device_reset()
{
	install_write_handler(0xb555, 0xb555, write8sm_delegate(*this, FUNC(coco_psg_device::flash5555_w)));
	install_write_handler(0xbaaa, 0xbaaa, write8sm_delegate(*this, FUNC(coco_psg_device::flash2aaa_w)));
}

void coco_psg_device::flash2aaa_w(offs_t offset, uint8_t data)
{
	if (BIT(m_control, 5) && BIT(m_control, 3))
	{
		m_flash->write(0x2aaa, data);
	}
}

void coco_psg_device::flash5555_w(offs_t offset, uint8_t data)
{
	if (BIT(m_control, 5) && BIT(m_control, 3))
	{
		m_flash->write(0x5555, data);
	}
}

//-------------------------------------------------
//  scs_read
//-------------------------------------------------

READ8_MEMBER(coco_psg_device::scs_read)
{
	uint8_t data = 0x00;

	switch (offset)
	{
	case 0x1a:
	case 0x1b:
		/* memory bank register */
		data = m_bank[BIT(offset, 0)];
		break;

	case 0x1f:
		/* ym data port */
		data = m_psg->data_r();
		break;
	}

	return data;
}

//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(coco_psg_device::scs_write)
{
	switch (offset)
	{
	case 0x1a:
	case 0x1b:
		/* memory bank register */
		m_bank[BIT(offset, 0)] = data;
		break;

	case 0x1d:
		/* control register
				BIT FUNCTION
				0   YM2149 MASTER CLOCK; 0=2MHz, 1=1MHz
				1   Gameport A SEL signal (pin 7 of controller port A)
				2   Gameport B SEL signal (pin 7 of controller port B)
				3   Write Enable (for FLASH/SRAM); 0=disabled, 1=enabled
				4   Autostart enable; 0=enabled, 1=disabled
				5   FLASH programming enable; 0=disabled, 1=enabled
				6   not used
				7   not used
		*/
		m_control = data;

		/* bit 0 - YM2149 MASTER CLOCK; 0=2MHz, 1=1MHz */
		if (BIT(data, 0))
			m_psg->set_pin26_low_w();
		else
			m_psg->set_pin26_high_w();

		/* bit 4 - Autostart enable; 0=enabled, 1=disabled */
		set_line_value(line::CART, BIT(data, 4) ? line_value::CLEAR : line_value::Q);
		break;

	case 0x1e:
		/* ym register select port*/
		m_psg->address_w(data & 0x0f);
		break;
	case 0x1f:
		/* ym data port */
		m_psg->data_w(data);
		break;
	}
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

READ8_MEMBER(coco_psg_device::cts_read)
{
	uint8_t data = 0x00;

	if (m_bank[BIT(offset, 13)] & 0x80)
	{
		data = m_sram[(offset & 0x1fff) | (m_bank[BIT(offset, 13)] & 0x3f) << 13];
	}
	else
	{
		data = m_flash->read_raw((offset & 0x1fff) | (m_bank[BIT(offset, 13)] & 0x3f) << 13);
	}

	return data;
}

//-------------------------------------------------
//  cts_write
//-------------------------------------------------

WRITE8_MEMBER(coco_psg_device::cts_write)
{
	if (BIT(m_control, 3))
	{
		if (m_bank[BIT(offset, 13)] & 0x80)
		{
			m_sram[(offset & 0x1fff) | (m_bank[BIT(offset, 13)] & 0x3f) << 13] = data;
		}
	}
}
