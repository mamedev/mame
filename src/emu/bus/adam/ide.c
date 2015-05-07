// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Micro Innovations Powermate IDE Hard Disk emulation

**********************************************************************/

/*

    TODO:

    - parallel status port
    - memory bank switching port
    - boot ROM

*/

#include "ide.h"
#include "bus/centronics/ctronics.h"



//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define ATA_TAG             "ata"
#define CENTRONICS_TAG      "centronics"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ADAM_IDE = &device_creator<powermate_ide_device>;


//-------------------------------------------------
//  ROM( adam_ata )
//-------------------------------------------------

ROM_START( adam_ata )
	ROM_REGION( 0x1000, "rom", 0 )
	ROM_LOAD( "exp.rom", 0x0000, 0x1000, NO_DUMP )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *powermate_ide_device::device_rom_region() const
{
	return ROM_NAME( adam_ata );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( adam_ata )
//-------------------------------------------------
static MACHINE_CONFIG_FRAGMENT( adam_ata )
	MCFG_ATA_INTERFACE_ADD(ATA_TAG, ata_devices, "hdd", NULL, false)
	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, centronics_devices, "printer")

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", CENTRONICS_TAG)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor powermate_ide_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( adam_ata );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  powermate_ide_device - constructor
//-------------------------------------------------

powermate_ide_device::powermate_ide_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ADAM_IDE, "Powermate HP IDE", tag, owner, clock, "adam_ide", __FILE__),
		device_adam_expansion_slot_card_interface(mconfig, *this),
		m_ata(*this, ATA_TAG),
		m_cent_data_out(*this, "cent_data_out")
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
			data = m_ata->read_cs0(space, offset & 0x07, 0xff);
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
			m_ata_data = m_ata->read_cs0(space, 0, 0xffff);

			data = m_ata_data & 0xff;
			break;

		case 0x59:
			data = m_ata_data >> 8;
			break;

		case 0x5a:
			data = m_ata->read_cs1(space, 6, 0xff);
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
			m_ata->write_cs0(space, offset & 0x07, data, 0xff);
			break;

		case 0x40:
			m_cent_data_out->write(space, 0, data);
			break;

		case 0x42: // Bank Number
			break;

		case 0x58:
			m_ata_data |= data;
			m_ata->write_cs0(space, 0, m_ata_data, 0xffff);
			break;

		case 0x59:
			m_ata_data = data << 8;
			break;

		case 0x5a: // Fixed Disk Control Register
			break;
		}
	}
}
