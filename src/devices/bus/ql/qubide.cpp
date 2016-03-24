// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Qubbesoft QubIDE emulation

**********************************************************************/

/*
  -------------------------------------------------------------------
 |                    This side goes into the QL                     |
 |                                                                   |
+---------------------------------------------------------------------+
|   o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o   |
|    +-------+               +---------+                              |
|    |o o|o o|               |o|o|o|o|o| +--------------------------+ |
|    +-------+               |o|o|o|o|o| |                          | |
|     J6   J7                +---------+ |                          | |
|+----------+--              J1J2J3J4J5  |)          EPROM          | |
||    |     |                            |                          | |
||    |7805 |--      +---------------+   |                          | |
||    |     |        |               |   +--------------------------+ |
|+----------+--      |)  74HCT688    |                                |
|                    |               |                                |
|                    +---------------+                                |
|+-------------------+  +-------------------+  +--------------------+ |
||                   |  |                   |  |                    | |
||)       GAL 1      |  |)       GAL 2      |  |)     74HCT646      | |
||                   |  |                   |  |                    | |
|+-------------------+  +-------------------+  +--------------------+ |
|                                                                     |
| +-----------------------------------------+  +--------------------+ |
| | o o o o o o o o o o o o o o o o o o o o |  |                    | |
| | o o o o o o o o o o o o o o o o o o o o |  |)     74HCT646      | |
| +-----------------------------------------+  |                    | |
|                                              +--------------------+ |
| +---+         +-----------+                                         |
| |o o| +       |o o o o o o| Exp. Conn.                              |
| +---+         +-----------+                                         |
| LED                                                                 |
|                                                                     |
|   o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o o   |
+---------------------------------------------------------------------+
 |                                                                   |
 |                         Through Connector                         |
  -------------------------------------------------------------------
*/

#include "qubide.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type QUBIDE = &device_creator<qubide_t>;


//-------------------------------------------------
//  ROM( qubide )
//-------------------------------------------------

ROM_START( qubide )
	ROM_REGION( 0x4000, "rom", 0 )
	ROM_DEFAULT_BIOS("v156")
	ROM_SYSTEM_BIOS( 0, "v141", "v1.41" )
	ROMX_LOAD( "qide141.bin", 0x0000, 0x4000, CRC(28955132) SHA1(37e47043260977c1fa5bae4a50b65d5575cd8e5f), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v156", "v1.56" )
	ROMX_LOAD( "qub156a.rom", 0x0000, 0x4000, CRC(95e8dd34) SHA1(74ea670ece5f579e61ddf4dbbc32645c21a80c03), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v201", "v2.01" )
	ROMX_LOAD( "qb201_16k.rom", 0x0000, 0x4000, CRC(6f1d62a6) SHA1(1708d85397422e2024daa1a3406cac685f46730d), ROM_BIOS(3) )

	ROM_REGION( 0x22e, "plds", 0 )
	ROM_LOAD( "gal 1a", 0x000, 0x117, CRC(cfb889ba) SHA1(657a2c61e4d372b84eaff78055ddeac6d2ee4d68) ) // old GAL (< v2.0)
	ROM_LOAD( "gal 2a", 0x117, 0x117, CRC(53d01e17) SHA1(4cf0da7ff5c7a950e8e13f8ed7125fff10ddda0d) ) // old GAL (< v2.0)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *qubide_t::device_rom_region() const
{
	return ROM_NAME( qubide );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( qubide )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( qubide )
	MCFG_ATA_INTERFACE_ADD("ata", ata_devices, "hdd", nullptr, false)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor qubide_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( qubide );
}


//-------------------------------------------------
//  INPUT_PORTS( qubide )
//-------------------------------------------------

INPUT_PORTS_START( qubide )
	PORT_START("J1-J5")
	PORT_DIPNAME( 0x1f, 0x03, "Base Address" )
	PORT_DIPSETTING(    0x00, "00000h" )
	PORT_DIPSETTING(    0x01, "04000h" )
	PORT_DIPSETTING(    0x02, "08000h" )
	PORT_DIPSETTING(    0x03, "0c000h" )
	PORT_DIPSETTING(    0x04, "10000h" )
	PORT_DIPSETTING(    0x05, "14000h" )
	PORT_DIPSETTING(    0x06, "18000h" )
	PORT_DIPSETTING(    0x07, "1c000h" )
	PORT_DIPSETTING(    0x08, "20000h" )
	PORT_DIPSETTING(    0x09, "24000h" )
	PORT_DIPSETTING(    0x0a, "28000h" )
	PORT_DIPSETTING(    0x0b, "2c000h" )
	PORT_DIPSETTING(    0x0c, "30000h" )
	PORT_DIPSETTING(    0x0d, "34000h" )
	PORT_DIPSETTING(    0x0e, "38000h" )
	PORT_DIPSETTING(    0x0f, "3c000h" )
	PORT_DIPSETTING(    0x10, "c0000h" )
	PORT_DIPSETTING(    0x11, "c4000h" )
	PORT_DIPSETTING(    0x12, "c8000h" )
	PORT_DIPSETTING(    0x13, "cc000h" )
	PORT_DIPSETTING(    0x14, "d0000h" )
	PORT_DIPSETTING(    0x15, "d4000h" )
	PORT_DIPSETTING(    0x16, "d8000h" )
	PORT_DIPSETTING(    0x17, "dc000h" )
	PORT_DIPSETTING(    0x18, "e0000h" )
	PORT_DIPSETTING(    0x19, "e4000h" )
	PORT_DIPSETTING(    0x1a, "e8000h" )
	PORT_DIPSETTING(    0x1b, "ec000h" )
	PORT_DIPSETTING(    0x1c, "f0000h" )
	PORT_DIPSETTING(    0x1d, "f4000h" )
	PORT_DIPSETTING(    0x1e, "f8000h" )
	PORT_DIPSETTING(    0x1f, "fc000h" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor qubide_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( qubide );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  qubide_t - constructor
//-------------------------------------------------

qubide_t::qubide_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, QUBIDE, "QubIDE", tag, owner, clock, "ql_qubide", __FILE__),
	device_ql_expansion_card_interface(mconfig, *this),
	m_ata(*this, "ata"),
	m_rom(*this, "rom"),
	m_j1_j5(*this, "J1-J5"),
	m_base(0xc000),
	m_ata_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void qubide_t::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void qubide_t::device_reset()
{
	int j1_j5 = m_j1_j5->read();

	m_base = (j1_j5 & 0x0f) << 14;

	if (BIT(j1_j5, 4))
	{
		m_base |= 0xc0000;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

UINT8 qubide_t::read(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xfc000) == m_base)
	{
		if ((offset & 0x3f00) == 0x3f00)
		{
			switch (offset & 0x0f)
			{
			case 0:
				data = m_ata->read_cs1(space, 0x07, 0xff);
				break;

			default:
				data = m_ata->read_cs0(space, offset & 0x07, 0xff);
				break;

			case 0x08: case 0x0a: case 0x0c:
				m_ata_data = m_ata->read_cs0(space, 0x00, 0xffff);

				data = m_ata_data >> 8;
				break;

			case 0x09: case 0x0b: case 0x0d:
				data = m_ata_data & 0xff;
				break;

			case 0x0e: case 0x0f:
				data = m_ata->read_cs1(space, 0x05, 0xff);
				break;
			}
		}
		else
		{
			data = m_rom->base()[offset & 0x3fff];
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void qubide_t::write(address_space &space, offs_t offset, UINT8 data)
{
	if ((offset & 0xfc000) == m_base)
	{
		if ((offset & 0x3f00) == 0x3f00)
		{
			switch (offset & 0x0f)
			{
			case 0: case 0x0e: case 0x0f:
				m_ata->write_cs1(space, 0x05, data, 0xff);
				break;

			case 0x08: case 0x0a: case 0x0c:
				m_ata_data = (data << 8) | (m_ata_data & 0xff);
				break;

			case 0x09: case 0x0b: case 0x0d:
				m_ata_data = (m_ata_data & 0xff00) | data;

				m_ata->write_cs0(space, 0x00, m_ata_data, 0xffff);
				break;

			default:
				m_ata->write_cs0(space, offset & 0x07, data, 0xff);
				break;
			}
		}
	}
}
