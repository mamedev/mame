// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "cr589.h"


static constexpr int identity_offset = 0x3ab;
static constexpr char download_identity[] = "MATSHITA CD98Q4 DOWNLOADGS0N";

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void matsushita_cr589_device::nvram_default()
{
	memset( buffer, 0, sizeof(buffer));
	memcpy( &buffer[ identity_offset ], "MATSHITACD-ROM CR-589   GS0N", 28 );
}



//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool matsushita_cr589_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(buffer, sizeof(buffer), actual) && actual == sizeof(buffer);
}



//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool matsushita_cr589_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(buffer, sizeof(buffer), actual) && actual == sizeof(buffer);
}



void matsushita_cr589_device::ExecCommand()
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_INQUIRY:
		logerror("T10MMC: INQUIRY\n");
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case 0x3b: // WRITE BUFFER
		bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
		break;

	case 0x3c: // READ BUFFER
		bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
		break;

	case 0xcc: // FIRMWARE DOWNLOAD ENABLE
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT16( &command[7] );
		break;

	default:
		t10mmc::ExecCommand();
		break;
	}
}

void matsushita_cr589_device::ReadData( uint8_t *data, int dataLength )
{
	switch( command[ 0 ] )
	{
	case T10SPC_CMD_INQUIRY:
		memset(data, 0, dataLength);

		t10mmc::ReadData( data, dataLength );

		if( download )
		{
			memcpy( &data[ 8 ], download_identity, 28 );
		}
		else
		{
			memcpy( &data[ 8 ], &buffer[ identity_offset ], 28 );
		}
		break;

	case 0x3c: // READ BUFFER
		memcpy( data, &buffer[ bufferOffset ], dataLength );
		bufferOffset += dataLength;
		break;

	default:
		t10mmc::ReadData( data, dataLength );
		break;
	}
}

void matsushita_cr589_device::WriteData( uint8_t *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			memcpy( &buffer[ bufferOffset ], data + 32, dataLength - 32 );
			bufferOffset += dataLength;
			break;

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			if( memcmp( data, &buffer[ identity_offset ], 28 ) == 0 )
			{
				download = 1;
			}
			else if( memcmp( data, download_identity, 28 ) == 0 )
			{
				download = 0;
			}
			break;

		default:
			t10mmc::WriteData( data, dataLength );
			break;
	}
}

// device type definition
DEFINE_DEVICE_TYPE(CR589, matsushita_cr589_device, "cr589", "Matsushita CR589 CD-ROM Drive")

matsushita_cr589_device::matsushita_cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	atapi_cdrom_device(mconfig, CR589, tag, owner, clock),
	device_nvram_interface(mconfig, *this)
{
}

void matsushita_cr589_device::device_start()
{
	save_item(NAME(download));
	save_item(NAME(buffer));
	save_item(NAME(bufferOffset));

	atapi_cdrom_device::device_start();

	/// TODO: split identify buffer into another method as device_start() should be called after it's filled in, but the atapi_cdrom_device has it's own.
	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[ 0 ] = 0x8500; // ATAPI device, cmd set 5 compliant, DRQ within 3 ms of PACKET command

	m_identify_buffer[ 23 ] = ('1' << 8) | '.';
	m_identify_buffer[ 24 ] = ('0' << 8) | ' ';
	m_identify_buffer[ 25 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 26 ] = (' ' << 8) | ' ';

	m_identify_buffer[ 27 ] = ('M' << 8) | 'A';
	m_identify_buffer[ 28 ] = ('T' << 8) | 'S';
	m_identify_buffer[ 29 ] = ('H' << 8) | 'I';
	m_identify_buffer[ 30 ] = ('T' << 8) | 'A';
	m_identify_buffer[ 31 ] = (' ' << 8) | 'C';
	m_identify_buffer[ 32 ] = ('R' << 8) | '-';
	m_identify_buffer[ 33 ] = ('5' << 8) | '8';
	m_identify_buffer[ 34 ] = ('9' << 8) | ' ';
	m_identify_buffer[ 35 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 36 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 37 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 38 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 39 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 40 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 41 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 42 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 43 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 44 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 45 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 46 ] = (' ' << 8) | ' ';

	m_identify_buffer[ 49 ] = 0x0400; // IORDY may be disabled
}

void matsushita_cr589_device::device_reset()
{
	atapi_cdrom_device::device_reset();

	download = 0;
	bufferOffset = 0;
}
