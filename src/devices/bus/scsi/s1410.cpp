// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Xebec S1410 5.25" Winchester Disk Controller emulation

**********************************************************************/

/*

Xebec S1410

PCB Layout
----------

ASSY 104527 REV E04 SN 127623

|-------------------------------------------|
|                                           |
|CN1                                        |
|                                           |
|                                           |
|CN2                                        |
|                   XEBEC2               CN5|
|   PROM                        2114        |
|CN3                XEBEC1      2114        |
|                                           |
|CN4                Z80         ROM         |
|   20MHz                             16MHz |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    Z80     - Zilog Z8400APS Z80A CPU
    ROM     - 2732 pinout ROM "XEBEC 104521G 2155008 M460949"
    PROM    - National Semiconductor DM74S288N "103911" 32x8 TTL PROM
    2114    - National Semiconductor NMC2148HN-3 1Kx4 RAM
    XEBEC1  - Xebec 3198-0009
    XEBEC2  - Xebec 3198-0045 "T20"
    CN1     - 4-pin Molex, drive power
    CN2     - 34-pin PCB edge, ST-506 drive 0/1 control
    CN3     - 2x10 PCB header, ST-506 drive 0 data
    CN4     - 2x10 PCB header, ST-506 drive 1 data
    CN5     - 2x25 PCB header, SASI host interface


ASSY 104766 REV C02 SN 231985P

|-------------------------------------------|
|                                           |
| CN1                           SY2158      |
|                               CN7     CN6 |
|                               ROM         |
| CN2                                       |
|           XEBEC1          Z80             |
|                                       CN5 |
| CN3       XEBEC2   20MHz      XEBEC3      |
|                                           |
| CN4       XEBEC4              XEBEC5      |
|                                           |
|-------------------------------------------|

Notes:
    Relevant IC's shown.

    Z80     - Zilog Z8400APS Z80A CPU
    ROM     - 2732 pinout ROM "104788D"
    SY2158  - Synertek SY2158A-3 1Kx8 RAM
    XEBEC1  - Xebec 3198-0046N8445
    XEBEC2  - Xebec 3198-0009
    XEBEC3  - Xebec 3198-0057
    XEBEC4  - Xebec 3198-0058
    XEBEC5  - Xebec 3198-0056
    CN1     - 4-pin Molex, drive power
    CN2     - 34-pin PCB edge, ST-506 drive 0/1 control
    CN3     - 2x10 PCB header, ST-506 drive 0 data
    CN4     - 2x10 PCB header, ST-506 drive 1 data
    CN5     - 2x25 PCB header, SASI host interface
    CN6     - 2x8 PCB header
    CN7     - 2x10 PCB header (test only)

*/


#include "s1410.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z8400A_TAG          "z80"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type S1410 = &device_creator<s1410_device>;

//-------------------------------------------------
//  ROM( s1410 )
//-------------------------------------------------

ROM_START( s1410 )
	ROM_REGION( 0x1000, Z8400A_TAG, 0 )
	ROM_LOAD( "104521f", 0x0000, 0x1000, CRC(305b8e76) SHA1(9efaa53ae86bc111bd263ad433e083f78a000cab) )
	ROM_LOAD( "104521g", 0x0000, 0x1000, CRC(24385115) SHA1(c389f6108cd5ed798a090acacce940ee43d77042) )
	ROM_LOAD( "104788d", 0x0000, 0x1000, CRC(2e385e2d) SHA1(7e2c349b2b6e95f2134f82cffc38d86b8a68390d) )

	ROM_REGION( 0x20, "103911", 0 )
	ROM_LOAD( "103911", 0x00, 0x20, NO_DUMP ) // DM74S288N
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *s1410_device::device_rom_region() const
{
	return ROM_NAME( s1410 );
}


//-------------------------------------------------
//  ADDRESS_MAP( s1410_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( s1410_mem, AS_PROGRAM, 8, s1410_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(Z8400A_TAG, 0)
	AM_RANGE(0x1000, 0x13ff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( s1410_io )
//-------------------------------------------------

static ADDRESS_MAP_START( s1410_io, AS_IO, 8, s1410_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x66, 0x66) AM_WRITENOP
	AM_RANGE(0x67, 0x67) AM_WRITENOP
	AM_RANGE(0x68, 0x68) AM_READNOP
	AM_RANGE(0x69, 0x69) AM_WRITENOP
	AM_RANGE(0x6a, 0x6a) AM_WRITENOP
	AM_RANGE(0x6b, 0x6b) AM_WRITENOP
	AM_RANGE(0x6c, 0x6c) AM_WRITENOP
	AM_RANGE(0xa0, 0xa0) AM_NOP
	AM_RANGE(0xc1, 0xc1) AM_WRITENOP
	AM_RANGE(0xc2, 0xc2) AM_WRITENOP
	AM_RANGE(0xc3, 0xc3) AM_WRITENOP
ADDRESS_MAP_END


//-------------------------------------------------
//  MACHINE_DRIVER( s1410 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( s1410 )
	MCFG_CPU_ADD(Z8400A_TAG, Z80, XTAL_16MHz/4)
	MCFG_CPU_PROGRAM_MAP(s1410_mem)
	MCFG_CPU_IO_MAP(s1410_io)
	MCFG_DEVICE_DISABLE()

	MCFG_HARDDISK_ADD("image")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor s1410_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( s1410 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s1410_device - constructor
//-------------------------------------------------

s1410_device::s1410_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: scsihd_device(mconfig, S1410, "Xebec S1410", tag, owner, clock, "s1410", __FILE__)
{
}

#define S1410_CMD_CHECK_TRACK_FORMAT ( 0x05 )
#define S1410_CMD_FORMAT_TRACK ( 0x06 )
#define S1410_CMD_INIT_DRIVE_PARAMS ( 0x0c )
#define S1410_CMD_FORMAT_ALT_TRACK ( 0x0E )
#define S1410_CMD_WRITE_SEC_BUFFER ( 0x0F )
#define S1410_CMD_READ_SEC_BUFFER ( 0x10 )
#define S1410_CMD_RAM_DIAGS ( 0xe0 )
#define S1410_CMD_DRIVE_DIAGS ( 0xe3 )
#define S1410_CMD_CONTROLER_DIAGS ( 0xe4 )

#define S1410_STATUS_NOT_READY ( 0x04 )

#define TRANSFERLENGTH_INIT_DRIVE_PARAMS ( 0x08 )
#define TRANSFERLENGTH_FORMAT_ALT_TRACK ( 0x03 )
#define TRANSFERLENGTH_SECTOR_BUFFER ( 0x0200 )

void s1410_device::ExecCommand()
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_RECALIBRATE:
		if (command[1] >> 5)
		{
			m_phase = SCSI_PHASE_STATUS;
			m_status_code = SCSI_STATUS_CODE_CHECK_CONDITION;
			m_sense_asc = S1410_STATUS_NOT_READY;
			m_transfer_length = 0;
		}
		else
		{
			scsihd_device::ExecCommand();
		}
		break;

	case T10SPC_CMD_REQUEST_SENSE:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 4;
		break;

	case S1410_CMD_FORMAT_TRACK:
		{
		m_lba = (command[1]&0x1f)<<16 | command[2]<<8 | command[3];

		switch( m_sector_bytes )
		{
		case 256:
			m_blocks = 32;
			break;

		case 512:
			m_blocks = 17;
			break;
		}

		logerror("S1410: FORMAT TRACK at LBA %x for %x blocks\n", m_lba, m_blocks);

		if ((m_disk) && (m_blocks))
		{
			dynamic_buffer data(m_sector_bytes);
			memset(&data[0], 0xc6, m_sector_bytes);

			while (m_blocks > 0)
			{
				if (!hard_disk_write(m_disk, m_lba, &data[0]))
				{
					logerror("S1410: HD write error!\n");
				}
				m_lba++;
				m_blocks--;
			}
		}

		m_phase = SCSI_PHASE_STATUS;
		m_transfer_length = 0;
		}
		break;

	case S1410_CMD_INIT_DRIVE_PARAMS:
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = TRANSFERLENGTH_INIT_DRIVE_PARAMS;
		break;

	case S1410_CMD_FORMAT_ALT_TRACK:
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = TRANSFERLENGTH_FORMAT_ALT_TRACK;
		break;

	case S1410_CMD_WRITE_SEC_BUFFER:
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = TRANSFERLENGTH_SECTOR_BUFFER;
		break;

	case S1410_CMD_READ_SEC_BUFFER:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = TRANSFERLENGTH_SECTOR_BUFFER;
		break;

	case S1410_CMD_CHECK_TRACK_FORMAT:
	case S1410_CMD_RAM_DIAGS:
	case S1410_CMD_DRIVE_DIAGS:
	case S1410_CMD_CONTROLER_DIAGS:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	default:
		scsihd_device::ExecCommand();
		break;
	}
}

void s1410_device::WriteData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case S1410_CMD_INIT_DRIVE_PARAMS:
		{
			int sectorsPerTrack = 0;

			switch( m_sector_bytes )
			{
			case 256:
				sectorsPerTrack = 32;
				break;

			case 512:
				sectorsPerTrack = 17;
				break;
			}

			UINT16 tracks = ((data[0]<<8)+data[1]);
			UINT8 heads = data[2];
			UINT32 capacity = tracks * heads * sectorsPerTrack * m_sector_bytes;

			logerror("S1410_CMD_INIT_DRIVE_PARAMS Tracks=%d, Heads=%d, Capacity=%d\n",tracks,heads,capacity);
		}
		break;

	default:
		scsihd_device::WriteData( data, dataLength );
		break;
	}
}

void s1410_device::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_REQUEST_SENSE:
		data[0] = m_sense_asc & 0x7f;
		data[1] = (m_sense_information >> 16) & 0x1f;
		data[2] = (m_sense_information >> 8) & 0xff;
		data[3] = (m_sense_information >> 0) & 0xff;
		break;

	default:
		scsihd_device::ReadData( data, dataLength );
		break;
	}
}
