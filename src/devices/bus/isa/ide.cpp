// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 16 bit IDE controller

***************************************************************************/

#include "emu.h"
#include "ide.h"

#include "imagedev/harddriv.h"
#include "machine/idectrl.h"
#include "sound/cdda.h"
#include "speaker.h"


READ8_MEMBER(isa16_ide_device::ide16_alt_r )
{
	return m_ide->read_cs1(6/2, 0xff);
}

WRITE8_MEMBER(isa16_ide_device::ide16_alt_w )
{
	m_ide->write_cs1(6/2, data, 0xff);
}

void isa16_ide_device::map(address_map &map)
{
	map(0x0, 0x7).rw("ide", FUNC(ide_controller_device::cs0_r), FUNC(ide_controller_device::cs0_w));
}

void isa16_ide_device::alt_map(address_map &map)
{
	map(0x6, 0x6).rw(FUNC(isa16_ide_device::ide16_alt_r), FUNC(isa16_ide_device::ide16_alt_w));
}

WRITE_LINE_MEMBER(isa16_ide_device::ide_interrupt)
{
	if (is_primary())
	{
		m_isa->irq14_w(state);
	}
	else
	{
		m_isa->irq15_w(state);
	}
}

void isa16_ide_device::cdrom_headphones(device_t *device)
{
	cdda_device *cdda = device->subdevice<cdda_device>("cdda");
	cdda->add_route(0, "^^lheadphone", 1.0);
	cdda->add_route(1, "^^rheadphone", 1.0);
}

static INPUT_PORTS_START( ide )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "IDE Configuration")
	PORT_DIPSETTING(    0x00, "Primary" )
	PORT_DIPSETTING(    0x01, "Secondary" )
INPUT_PORTS_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA16_IDE, isa16_ide_device, "isa_ide", "IDE Fixed Drive Adapter")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa16_ide_device::device_add_mconfig(machine_config &config)
{
	IDE_CONTROLLER(config, m_ide).options(ata_devices, "hdd", nullptr, false);
	m_ide->irq_handler().set(FUNC(isa16_ide_device::ide_interrupt));

	SPEAKER(config, "lheadphone").front_left();
	SPEAKER(config, "rheadphone").front_right();

	m_ide->slot(0).set_option_machine_config("cdrom", cdrom_headphones);
	m_ide->slot(1).set_option_machine_config("cdrom", cdrom_headphones);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor isa16_ide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ide );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_ide_device - constructor
//-------------------------------------------------

isa16_ide_device::isa16_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ISA16_IDE, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_is_primary(true)
	, m_ide(*this, "ide")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_ide_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa16_ide_device::device_reset()
{
	m_is_primary = (ioport("DSW")->read() & 1) ? false : true;
	if (m_is_primary) {
		m_isa->install_device(0x01f0, 0x01f7, *this, &isa16_ide_device::map);
		m_isa->install_device(0x03f0, 0x03f7, *this, &isa16_ide_device::alt_map);
	} else {
		m_isa->install_device(0x0170, 0x0177, *this, &isa16_ide_device::map);
		m_isa->install_device(0x0370, 0x0377, *this, &isa16_ide_device::alt_map);
	}
}
