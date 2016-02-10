// license:BSD-3-Clause
// copyright-holders:Dirk Best
/***************************************************************************

    Acculogic sIDE-1/16

    IDE Disk Controller for IBM PC, XT and compatibles

***************************************************************************/

#include "side116.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ISA8_SIDE116 = &device_creator<side116_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( side116 )
	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(side116_device, ide_interrupt))
MACHINE_CONFIG_END

machine_config_constructor side116_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( side116 );
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( side116 )
	PORT_START("configuration")
	PORT_DIPNAME(0x01, 0x00, "sIDE-1/16 ROM")
	PORT_DIPLOCATION("JP:1")
	PORT_DIPSETTING(0x00, "Enabled")
	PORT_DIPSETTING(0x01, "Disabled")
	PORT_DIPNAME(0x06, 0x00, "sIDE-1/16 ROM Address")
	PORT_DIPLOCATION("JP:2,3")
	PORT_DIPSETTING(0x00, "Range C800h")
	PORT_DIPSETTING(0x04, "Range CC00h")
	PORT_DIPSETTING(0x02, "Range D800h")
	PORT_DIPSETTING(0x06, "Range DC00h")
	PORT_DIPNAME(0x18, 0x10, "sIDE-1/16 IDE IRQ")
	PORT_DIPLOCATION("JP:4,5")
	PORT_DIPSETTING(0x10, "Level 5")
	PORT_DIPSETTING(0x08, "Level 2")
	PORT_DIPNAME(0x20, 0x20, "sIDE-1/16 IDE")
	PORT_DIPLOCATION("JP:6")
	PORT_DIPSETTING(0x00, "Disabled")
	PORT_DIPSETTING(0x20, "Enabled")
INPUT_PORTS_END

ioport_constructor side116_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( side116 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( side116 )
	ROM_REGION(0x2000, "option", 0)
	ROM_LOAD("bios12.u2", 0x0000, 0x2000, CRC(c202a0e6) SHA1(a5b130a6d17c972d6c378cb2cd8113a4039631fe))
ROM_END

const rom_entry *side116_device::device_rom_region() const
{
	return ROM_NAME( side116 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  side116_device - constructor
//-------------------------------------------------

side116_device::side116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ISA8_SIDE116, "Acculogic sIDE-1/16 IDE Disk Controller", tag, owner, clock, "side116", __FILE__),
	device_isa8_card_interface( mconfig, *this ),
	m_ata(*this, "ata"),
	m_config(*this, "configuration"),
	m_latch(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void side116_device::device_start()
{
	set_isa_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void side116_device::device_reset()
{
	// install option rom
	if ((m_config->read() & 0x01) == 0x00)
	{
		switch ((m_config->read() >> 1) & 0x03)
		{
		case 0: m_isa->install_rom(this, 0xc8000, 0xc9fff, 0, 0, "side116", "option"); break;
		case 1: m_isa->install_rom(this, 0xd8000, 0xd9fff, 0, 0, "side116", "option"); break;
		case 2: m_isa->install_rom(this, 0xcc000, 0xcdfff, 0, 0, "side116", "option"); break;
		case 3: m_isa->install_rom(this, 0xdc000, 0xddfff, 0, 0, "side116", "option"); break;
		}
	}

	// install io access
	if ((m_config->read() & 0x20) == 0x20)
		m_isa->install_device(0x360, 0x36f, 0, 0, read8_delegate(FUNC(side116_device::read), this), write8_delegate(FUNC(side116_device::write), this));
}


//**************************************************************************
//  IDE INTERFACE
//**************************************************************************

READ8_MEMBER( side116_device::read )
{
	UINT8 data;

	if (offset == 0)
	{
		UINT16 ide_data = m_ata->read_cs0(space, 0, 0xffff);
		data = ide_data & 0xff;
		m_latch = ide_data >> 8;
	}
	else if (offset < 8)
	{
		data = m_ata->read_cs0(space, offset & 7, 0xff);
	}
	else if (offset == 8)
	{
		data = m_latch;
	}
	else
	{
		data = m_ata->read_cs1(space, offset & 7, 0xff);
	}

	return data;
}

WRITE8_MEMBER( side116_device::write )
{
	if (offset == 0)
	{
		UINT16 ide_data = (m_latch << 8) | data;
		m_ata->write_cs0(space, 0, ide_data, 0xffff);
	}
	else if (offset < 8)
	{
		m_ata->write_cs0(space, offset & 7, data, 0xff);
	}
	else if (offset == 8)
	{
		m_latch = data;
	}
	else
	{
		m_ata->write_cs1(space, offset & 7, data, 0xff);
	}
}

WRITE_LINE_MEMBER( side116_device::ide_interrupt )
{
	UINT8 level = m_config->read() & 0x18;

	if (level == 0x08)
		m_isa->irq2_w(state);
	else if (level == 0x10)
		m_isa->irq5_w(state);
}
