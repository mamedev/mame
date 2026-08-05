// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 16 bit IDE controller

***************************************************************************/

#include "emu.h"
#include "ide.h"

#include "bus/ata/atadev.h"
#include "imagedev/harddriv.h"
#include "sound/cdda.h"
#include "speaker.h"


uint8_t isa16_ide_device::ide16_alt_r()
{
	return m_ide->read_cs1(6/2, 0xff);
}

void isa16_ide_device::ide16_alt_w(uint8_t data)
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

void isa16_ide_device::ide_interrupt(int state)
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
	cdda->add_route(0, "^^headphone", 1.0, 0);
	cdda->add_route(1, "^^headphone", 1.0, 1);
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

	SPEAKER(config, "headphone", 2).front();

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
	remap(AS_IO, 0, 0xffff);
}

void isa16_ide_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		const u16 base_reg = m_is_primary ? 0x1f0 : 0x170;

		// $1f0 / $3f0 or $170 / $370
		m_isa->install_device(base_reg | 0x000, base_reg | 0x007, *this, &isa16_ide_device::map);
		m_isa->install_device(base_reg | 0x200, base_reg | 0x207, *this, &isa16_ide_device::alt_map);
	}
}
