// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * IDE Compact Flash Adapter
 *
 * This implements a Compact flash adapter expansion card for QX-10
 * systems. The boot disk necessary to support this card is listed below.
 *
 * Disk Image: https://github.com/brijohn/qx10/raw/master/diskimages/qx10cf.imd
 * Board Design: https://github.com/brijohn/qx10/tree/master/kicad/cf_adapter
 *
 *******************************************************************/

#include "emu.h"
#include "ide.h"


//**************************************************************************
//  EPSON IDE DEVICE
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_OPTION_IDE, bus::epson_qx::ide_device, "epson_qx_option_ide", "Epson QX-10 Compact Flash Adapter")

namespace bus::epson_qx {

static INPUT_PORTS_START( ide )
	PORT_START("IOBASE")
	PORT_CONFNAME(0xf0, 0xd0, "IO Base Address Selection")
	PORT_CONFSETTING(0x80, "&80")
	PORT_CONFSETTING(0x90, "&90")
	PORT_CONFSETTING(0xa0, "&A0")
	PORT_CONFSETTING(0xb0, "&B0")
	PORT_CONFSETTING(0xc0, "&C0")
	PORT_CONFSETTING(0xd0, "&D0")
	PORT_CONFSETTING(0xe0, "&E0")
	PORT_CONFSETTING(0xf0, "&F0")
INPUT_PORTS_END

//-------------------------------------------------
//  ide_device - constructor
//-------------------------------------------------
ide_device::ide_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EPSON_QX_OPTION_IDE, tag, owner, clock),
	device_option_expansion_interface(mconfig, *this),
	m_hdd(*this, "hdd"),
	m_iobase(*this, "IOBASE"),
	m_installed(false)
{
}

//-------------------------------------------------
//  device_input_ports - device-specific ports
//-------------------------------------------------
ioport_constructor ide_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ide );
}

//-------------------------------------------------
//  device_add_mconfig - device-specific config
//-------------------------------------------------
void ide_device::device_add_mconfig(machine_config &config)
{
	IDE_HARDDISK(config, m_hdd, 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void ide_device::device_start()
{
	m_installed = false;

	save_item(NAME(m_installed));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void ide_device::device_reset()
{
	if (!m_installed) {
		address_space &space = m_bus->iospace();
		offs_t iobase = m_iobase->read() & 0xf0;
		space.install_device(iobase, iobase+0x0f, *this, &ide_device::map);
		m_installed = true;
	}
}

uint8_t ide_device::read(offs_t offset)
{
	if (offset < 8) {
		return m_hdd->read_cs0(offset);
	} else if (offset == 14 || offset == 15) {
		return m_hdd->read_cs1(offset & 7);
	}
	return 0xff;
}

void ide_device::write(offs_t offset, uint8_t data)
{
	if (offset < 8) {
		m_hdd->write_cs0(offset, data);
	} else if (offset == 14) {
		m_hdd->write_cs1(offset & 7, data);
	}
}

void ide_device::map(address_map &map)
{
	map(0x00, 0x0f).rw(FUNC(ide_device::read), FUNC(ide_device::write));
}

} // namespace bus::epson_qx
