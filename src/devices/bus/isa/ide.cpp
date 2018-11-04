// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  ISA 16 bit IDE controller

***************************************************************************/

#include "emu.h"
#include "ide.h"

#include "imagedev/harddriv.h"
#include "machine/idectrl.h"
#include "speaker.h"


READ8_MEMBER(isa16_ide_device::ide16_alt_r )
{
	return m_ide->read_cs1(6/2, 0xff);
}

WRITE8_MEMBER(isa16_ide_device::ide16_alt_w )
{
	m_ide->write_cs1(6/2, data, 0xff);
}

ADDRESS_MAP_START(isa16_ide_device::map)
	AM_RANGE(0x0, 0x7) AM_DEVREADWRITE("ide", ide_controller_device, read_cs0, write_cs0)
ADDRESS_MAP_END

ADDRESS_MAP_START(isa16_ide_device::alt_map)
	AM_RANGE(0x6, 0x6) AM_READWRITE(ide16_alt_r, ide16_alt_w)
ADDRESS_MAP_END

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
	device = device->subdevice("cdda");
	MCFG_SOUND_ROUTE(0, "^^^^lheadphone", 1.0)
	MCFG_SOUND_ROUTE(1, "^^^^rheadphone", 1.0)
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

MACHINE_CONFIG_START(isa16_ide_device::device_add_mconfig)
	MCFG_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(isa16_ide_device, ide_interrupt))

	MCFG_SPEAKER_STANDARD_STEREO("lheadphone", "rheadphone")

	MCFG_DEVICE_MODIFY("ide:0")
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_headphones)
	MCFG_DEVICE_MODIFY("ide:1")
	MCFG_SLOT_OPTION_MACHINE_CONFIG("cdrom", cdrom_headphones)
MACHINE_CONFIG_END

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
