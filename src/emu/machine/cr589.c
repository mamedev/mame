#include "cr589.h"
#include "scsicd.h"
#include "machine/cr589.h"

// device type definition
const device_type SCSI_CR589 = &device_creator<scsi_cr589_device>;

scsi_cr589_device::scsi_cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsicd_device(mconfig, SCSI_CR589, "SCSI CR589", tag, owner, clock, "scsi cr589", __FILE__)
{
}

static const int identity_offset = 0x3ab;
static const char download_identity[] = "MATSHITA CD98Q4 DOWNLOADGS0N";

void scsi_cr589_device::device_start()
{
	scsicd_device::device_start();

	download = 0;
	memcpy( &buffer[ identity_offset ], "MATSHITACD-ROM CR-589   GS0N", 28 );

	save_item(NAME(download));
	save_item(NAME(buffer));
	save_item(NAME(bufferOffset));
}

void scsi_cr589_device::ExecCommand( int *transferLength )
{
	switch( command[ 0 ] )
	{
		case 0x3b: // WRITE BUFFER
			bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
			break;

		case 0x3c: // READ BUFFER
			bufferOffset = ( command[ 3 ] << 16 ) | ( command[ 4 ] << 8 ) | command[ 5 ];
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = ( command[ 6 ] << 16 ) | ( command[ 7 ] << 8 ) | command[ 8 ];
			break;

		case 0xcc: // FIRMWARE DOWNLOAD ENABLE
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = SCSILengthFromUINT16( &command[7] );
			break;

		default:
			scsicd_device::ExecCommand( transferLength );
	}
}

void scsi_cr589_device::ReadData( UINT8 *data, int dataLength )
{
	switch( command[ 0 ] )
	{
		case 0x12: // INQUIRY
			scsicd_device::ReadData( data, dataLength );

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
			scsicd_device::ReadData( data, dataLength );
			break;
	}
}

void scsi_cr589_device::WriteData( UINT8 *data, int dataLength )
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
			scsicd_device::WriteData( data, dataLength );
			break;
	}
}

// device type definition
const device_type CR589 = &device_creator<matsushita_cr589_device>;

matsushita_cr589_device::matsushita_cr589_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atapi_hle_device(mconfig, CR589, "Matsushita CR589", tag, owner, clock, "cr589", __FILE__)
{
}

static MACHINE_CONFIG_FRAGMENT( cr589 )
	MCFG_DEVICE_ADD("device", SCSI_CR589, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor matsushita_cr589_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cr589 );
}

void matsushita_cr589_device::device_start()
{
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

	atapi_hle_device::device_start();
}

void matsushita_cr589_device::perform_diagnostic()
{
	m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
}

void matsushita_cr589_device::identify_packet_device()
{
}
