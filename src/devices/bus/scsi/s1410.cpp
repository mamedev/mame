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


#include "emu.h"
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

DEFINE_DEVICE_TYPE(S1410, s1410_device, "s1410", "Xebec S1410")

//-------------------------------------------------
//  ROM( s1410 )
//-------------------------------------------------

ROM_START( s1410 )
	ROM_REGION( 0x1000, Z8400A_TAG, 0 )
	ROM_LOAD( "104521f", 0x0000, 0x1000, CRC(305b8e76) SHA1(9efaa53ae86bc111bd263ad433e083f78a000cab) )
	ROM_LOAD( "104521g", 0x0000, 0x1000, CRC(24385115) SHA1(c389f6108cd5ed798a090acacce940ee43d77042) )
	ROM_LOAD( "104788d", 0x0000, 0x1000, CRC(2e385e2d) SHA1(7e2c349b2b6e95f2134f82cffc38d86b8a68390d) ) // BASF 6188
	ROM_LOAD( "104788e", 0x0000, 0x1000, CRC(40a7195d) SHA1(bf68e99dad182bfcbb5d488aeb440cba572a6ef8) ) // Seagate ST-251

	ROM_REGION( 0x20, "103911", 0 )
	ROM_LOAD( "103911", 0x00, 0x20, NO_DUMP ) // DM74S288N
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *s1410_device::device_rom_region() const
{
	return ROM_NAME( s1410 );
}


//-------------------------------------------------
//  ADDRESS_MAP( s1410_mem )
//-------------------------------------------------

void s1410_device::s1410_mem(address_map &map)
{
	map(0x0000, 0x0fff).rom().region(Z8400A_TAG, 0);
	map(0x1000, 0x13ff).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( s1410_io )
//-------------------------------------------------

void s1410_device::s1410_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x66, 0x66).nopw();
	map(0x67, 0x67).nopw();
	map(0x68, 0x68).nopr();
	map(0x69, 0x69).nopw();
	map(0x6a, 0x6a).nopw();
	map(0x6b, 0x6b).nopw();
	map(0x6c, 0x6c).nopw();
	map(0xa0, 0xa0).noprw();
	map(0xc1, 0xc1).nopw();
	map(0xc2, 0xc2).nopw();
	map(0xc3, 0xc3).nopw();
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void s1410_device::device_add_mconfig(machine_config &config)
{
	z80_device &z8400a(Z80(config, Z8400A_TAG, XTAL(16'000'000)/4));
	z8400a.set_addrmap(AS_PROGRAM, &s1410_device::s1410_mem);
	z8400a.set_addrmap(AS_IO, &s1410_device::s1410_io);
	z8400a.set_disable();

	HARDDISK(config, "image");
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  s1410_device - constructor
//-------------------------------------------------

s1410_device::s1410_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: scsihd_device(mconfig, S1410, tag, owner, clock)
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
#define S1410_CMD_CONTROLLER_DIAGS ( 0xe4 )

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
			std::vector<uint8_t> data(m_sector_bytes);
			memset(&data[0], 0xc6, m_sector_bytes);

			while (m_blocks > 0)
			{
				if (!m_disk->write(m_lba, &data[0]))
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
	case S1410_CMD_CONTROLLER_DIAGS:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	default:
		scsihd_device::ExecCommand();
		break;
	}
}

void s1410_device::WriteData( uint8_t *data, int dataLength )
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

			uint16_t tracks = ((data[0]<<8)+data[1]);
			uint8_t heads = data[2];
			uint32_t capacity = tracks * heads * sectorsPerTrack * m_sector_bytes;

			logerror("S1410_CMD_INIT_DRIVE_PARAMS Tracks=%d, Heads=%d, Capacity=%d\n",tracks,heads,capacity);
		}
		break;

	default:
		scsihd_device::WriteData( data, dataLength );
		break;
	}
}

void s1410_device::ReadData( uint8_t *data, int dataLength )
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
