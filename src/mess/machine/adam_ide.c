/**********************************************************************

    Micro Innovations Powermate IDE Hard Disk emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*

    TODO:

    - parallel status port
    - memory bank switching port
    - boot ROM

*/

#include "adam_ide.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define IDE_TAG             "ide"
#define CENTRONICS_TAG      "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_IDE = &device_creator<powermate_ide_device>;


//-------------------------------------------------
//  ROM( adam_ide )
//-------------------------------------------------

ROM_START( adam_ide )
	ROM_REGION( 0x1000, "rom", 0 )
	ROM_LOAD( "exp.rom", 0x0000, 0x1000, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *powermate_ide_device::device_rom_region() const
{
	return ROM_NAME( adam_ide );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( adam_ide )
//-------------------------------------------------
static MACHINE_CONFIG_FRAGMENT( adam_ide )
	MCFG_IDE_CONTROLLER_ADD(IDE_TAG, ide_image_devices, "hdd", NULL, false)
	MCFG_CENTRONICS_PRINTER_ADD(CENTRONICS_TAG, standard_centronics)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor powermate_ide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adam_ide );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  powermate_ide_device - constructor
//-------------------------------------------------

powermate_ide_device::powermate_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADAM_IDE, "Powermate HP IDE", tag, owner, clock),
		device_adam_expansion_slot_card_interface(mconfig, *this),
		m_ide(*this, IDE_TAG),
		m_centronics(*this, CENTRONICS_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void powermate_ide_device::device_start()
{
}


//-------------------------------------------------
//  adam_bd_r - buffered data read
//-------------------------------------------------

UINT8 powermate_ide_device::adam_bd_r(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!biorq)
	{
		switch (offset & 0xff)
		{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			data = ide_bus_r(m_ide, 0, offset & 0x07) & 0xff;
			break;

		case 0x40: // Printer status
			/*

			    bit     description

			    0
			    1
			    2
			    3
			    4
			    5
			    6
			    7

			*/
			break;

		case 0x58:
			m_ide_data = ide_bus_r(m_ide, 0, 0);

			data = m_ide_data & 0xff;
			break;

		case 0x59:
			data = m_ide_data >> 8;
			break;

		case 0x5a:
			data = ide_bus_r(m_ide, 1, 6) & 0xff;
			break;

		case 0x5b: // Digital Input Register
			data = 0xff;
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  adam_bd_w - buffered data write
//-------------------------------------------------

void powermate_ide_device::adam_bd_w(address_space &space, offs_t offset, UINT8 data, int bmreq, int biorq, int aux_rom_cs, int cas1, int cas2)
{
	if (!biorq)
	{
		switch (offset & 0xff)
		{
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			ide_bus_w(m_ide, 0, offset & 0x07, data);
			break;

		case 0x40:
			m_centronics->write(space, 0, data);
			break;

		case 0x42: // Bank Number
			break;

		case 0x58:
			m_ide_data |= data;
			ide_bus_w(m_ide, 0, 0, m_ide_data);
			break;

		case 0x59:
			m_ide_data = data << 8;
			break;

		case 0x5a: // Fixed Disk Control Register
			break;
		}
	}
}
