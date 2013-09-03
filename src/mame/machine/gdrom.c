/***************************************************************************

 gdrom.c - Implementation of the Sega GD-ROM device

***************************************************************************/

#include "emu.h"
#include "machine/scsihle.h"
#include "cdrom.h"
#include "imagedev/chd_cd.h"
#include "gdrom.h"
#include "debugger.h"

#define GDROM_BUSY_STATE    0x00
#define GDROM_PAUSE_STATE   0x01
#define GDROM_STANDBY_STATE 0x02
#define GDROM_PLAY_STATE    0x03
#define GDROM_SEEK_STATE    0x04
#define GDROM_SCAN_STATE    0x05
#define GDROM_OPEN_STATE    0x06
#define GDROM_NODISC_STATE  0x07
#define GDROM_RETRY_STATE   0x08
#define GDROM_ERROR_STATE   0x09

static const UINT8 GDROM_Cmd71_Reply[] =
{
	0x0b,0x96,0xf0,0x45,0xff,0x7e,0x06,0x3d,0x7d,0x4d,0xbf,0x10,0x00,0x07,0xcf,0x73,
	0x00,0x9c,0x0c,0xbc,0xaf,0x1c,0x30,0x1c,0xa7,0xe7,0xa8,0x03,0x00,0x98,0x0f,0xbd,
	0x5b,0xbd,0x50,0xaa,0x39,0x23,0x10,0x31,0x69,0x0e,0xe5,0x13,0xd2,0x00,0x66,0x0d,
	0xbf,0x54,0xfd,0x5f,0x74,0x37,0x5b,0xf4,0x00,0x22,0x09,0xc6,0xca,0x0f,0xe8,0x93,
	0xab,0xa4,0x61,0x00,0x2e,0x0e,0x4b,0xe1,0x8b,0x76,0xa5,0x6a,0xe6,0x9c,0xc4,0x23,
	0x4b,0x00,0x1b,0x06,0x01,0x91,0xe2,0x00,0xcf,0x0d,0x38,0xca,0xb9,0x3a,0x91,0xe7,
	0xef,0xe5,0x00,0x4b,0x09,0xd6,0x68,0xd3,0xc4,0x3e,0x2d,0xaf,0x2a,0x00,0xf9,0x0d,
	0x78,0xfc,0xae,0xed,0xb3,0x99,0x5a,0x32,0x00,0xe7,0x0a,0x4c,0x97,0x22,0x82,0x5b,
	0x7a,0x06,0x00,0x4c,0x0e,0x42,0x78,0x57,0xf5,0x46,0xfc,0x20,0xcb,0x6b,0x5b,0x01,
	0x00,0x86,0x0e,0xe4,0x26,0xb2,0x71,0xcd,0xa5,0xe3,0x06,0x33,0x9a,0x8e,0x00,0x50,
	0x07,0x07,0x34,0xf5,0xe6,0xef,0x32,0x00,0x13,0x0f,0x59,0x41,0x0f,0x56,0x38,0x02,
	0x64,0x2a,0x07,0x2a,0x00,0x3e,0x11,0x52,0x1d,0x2a,0x76,0x5f,0xa0,0x66,0x2f,0xb2,
	0xc7,0x97,0x6e,0x5e,0xe2,0x52,0x58,0x00,0xca,0x09,0xa5,0x89,0x0a,0xdf,0x00,0xde,
	0x06,0x50,0xb8,0x49,0x00,0xb4,0x05,0x77,0xe8,0x24,0xbb,0x00,0x91,0x0c,0xa2,0x89,
	0x62,0x8b,0x6a,0xde,0x60,0xc6,0xe7,0x00,0x0f,0x0f,0x96,0x11,0xd2,0x55,0xe6,0xbf,
	0x0b,0x48,0xab,0x5c,0x00,0xdc,0x0a,0xba,0xd7,0x30,0x0e,0x48,0x63,0x78,0x00,0x0c,
	0x0d,0xd2,0x8a,0xfb,0xfe,0xa3,0x3a,0xf8,0x88,0xdd,0x4b,0xa9,0xa2,0x00,0x75,0x0a,
	0x0d,0x5d,0x24,0x37,0x9d,0xc5,0xf7,0x00,0x25,0x0b,0xdb,0xef,0xe0,0x41,0x3e,0x52,
	0x00,0x4e,0x03,0xb7,0xe5,0x00,0xb9,0x11,0x5a,0xde,0xcf,0x57,0x1a,0xb9,0x7f,0xfc,
	0xee,0x26,0xcd,0x7b,0x00,0x2b,0x08,0x4b,0x09,0xb8,0x6a,0x70,0x00,0x9f,0x11,0x4b,
	0x15,0x8c,0xa3,0x87,0x4f,0x05,0x8e,0x37,0xde,0x63,0x39,0xef,0x4b,0xfc,0xab,0x00,
	0x0b,0x10,0xaa,0x91,0xe1,0x0f,0xae,0xe9,0x3a,0x69,0x03,0xf8,0xd2,0x69,0xe2,0x00,
	0xc1,0x07,0x3d,0x5c,0x00,0x82,0x08,0xa9,0xc4,0x68,0x2e,0xad,0x00,0xd1,0x0e,0xf7,
	0x47,0xc6,0xcd,0xc8,0x7c,0x8e,0x5c,0x00,0xb9,0x95,0x00,0xf4,0x04,0xe3,0x00,0x5b,
	0x07,0x74,0xc7,0x65,0x8e,0x84,0xc6,0x00,0x61,0x07,0x44,0x80,0x00,0x3f,0x0e,0xc8,
	0x78,0x72,0xd3,0x47,0x4d,0xc2,0xc0,0xaf,0x13,0x54,0x00,0x31,0x0d,0xf7,0xd8,0x48,
	0x92,0xe2,0x7f,0x9f,0x44,0x2f,0x33,0x68,0x0d,0x00,0xab,0x10,0xea,0xfe,0x19,0x8e,
	0xf8,0x81,0x7c,0x6f,0xe1,0xde,0x06,0xb3,0x4d,0x00,0x66,0x11,0x4c,0xae,0xb7,0xf9,
	0xee,0x2f,0x8e,0xb0,0xe1,0x7e,0x95,0x8d,0x00,0x6f,0x0d,0xf4,0x9d,0x88,0xe3,0xca,
	0xb2,0xc4,0xbb,0x47,0x69,0xa0,0xf3,0x00,0x48,0x0b,0x41,0x17,0xa0,0x64,0x71,0x0e,
	0x00,0x82,0x1e,0x34,0x4d,0x18,0x80,0x85,0xa9,0x4c,0x66,0x0b,0x75,0x9b,0x61,0x13,
	0x27,0x70,0x7a,0x81,0xcd,0x02,0xab,0x57,0x02,0xdf,0x52,0x93,0xdf,0x83,0xa8,0x48,
	0x9e,0xa6,0x6f,0x74,0x03,0x89,0x25,0x28,0x96,0x52,0x67,0xff,0xd8,0x7a,0xb1,0x3c,
	0x46,0x2c,0xef,0x84,0xc1,0xe1,0xc9,0xc6,0x96,0xdc,0xa9,0xaa,0x82,0xc4,0x27,0x58,
	0x75,0x57,0x34,0x67,0x3b,0xfb,0xbf,0x25,0x3b,0xfb,0x13,0xf6,0x96,0xec,0x16,0xe5,
	0xfd,0x26,0xda,0xa8,0xc6,0x1b,0x7f,0x50,0xff,0x47,0x55,0x08,0xed,0x08,0x93,0x00,
	0xc4,0x9b,0x67,0x71,0xa6,0xec,0x16,0xcc,0x87,0x20,0x07,0x47,0x00,0xa6,0x5d,0x79,
	0xab,0x4f,0x6f,0xa1,0x6b,0x7a,0xc4,0x27,0xa3,0xda,0x94,0xc3,0x7f,0x4f,0xe5,0xf3,
	0x6f,0x1b,0xe5,0xcc,0xe5,0xf0,0xc9,0x9d,0xfd,0xae,0xac,0x39,0xe5,0x4c,0x83,0x58,
	0x65,0x25,0x74,0x92,0x81,0x9e,0xb6,0xa0,0x02,0xa9,0x07,0x9b,0xe7,0xb6,0x57,0x79,
	0x4a,0xd9,0xfa,0xce,0x94,0xb4,0xcc,0x05,0x3c,0x86,0x06,0xdd,0xa6,0xcd,0x24,0x24,
	0xc1,0xfa,0x48,0xf9,0x0c,0xc9,0xc4,0x6c,0x82,0x96,0xf6,0x17,0x09,0x31,0xe2,0xc4,
	0xfd,0x77,0x46,0xcf,0xb2,0x18,0x01,0x5f,0xd1,0x6b,0x56,0x7b,0x94,0xb8,0xe5,0x4a,
	0x19,0x6c,0xc0,0xf0,0x70,0xb6,0xf7,0x93,0xd1,0xd3,0x6e,0x2b,0x53,0x7c,0x85,0x6d,
	0x0c,0xd1,0x77,0x8b,0x90,0xee,0x15,0xda,0xe0,0x55,0x09,0x58,0xfc,0x56,0x9f,0x31,
	0x46,0xaf,0xc3,0xcb,0x71,0x8d,0xf2,0x75,0xc3,0x2c,0xa1,0xbb,0xcf,0xc4,0x56,0x27,
	0x9b,0x7c,0xaf,0xfe,0x4e,0x3e,0xcd,0xb4,0xaa,0x6a,0xf3,0xf5,0x22,0xe3,0xe1,0x82,
	0x68,0xa5,0xdb,0xb3,0x9e,0x8f,0x7b,0x5e,0xf0,0x90,0x3f,0x79,0x8c,0x52,0x88,0x61,
	0xae,0x76,0x63,0x14,0x0f,0x19,0xce,0x1d,0x63,0xa1,0xb2,0x10,0xd7,0xe2,0xb1,0x94,
	0xcb,0x33,0x85,0x28,0x9b,0x7d,0xf4,0xf5,0x50,0x25,0xdb,0x9b,0xa5,0x35,0x9c,0xb0,
	0x92,0x09,0x31,0xe3,0xab,0x40,0xf4,0x4d,0xe8,0x35,0x0a,0xb3,0xc3,0x21,0x9c,0x86,
	0x29,0xcb,0x77,0xa4,0xbc,0x57,0xda,0xd8,0x82,0xa5,0xe8,0x80,0x72,0xcf,0xad,0x81,
	0x28,0x2e,0xd8,0xff,0xd1,0xb6,0x97,0x2b,0xff,0x00,0x06,0xe1,0x39,0x44,0x4b,0x1c,
	0x19,0xab,0x4d,0x5b,0x3e,0xd6,0x5c,0x1b,0xbb,0x64,0x68,0x32,0x7c,0xf5,0x9e,0xc9,
	0xb4,0xe8,0x1b,0x29,0x4d,0x7f,0x80,0x80,0x8b,0x7e,0x0a,0x1c,0x9a,0xe6,0x49,0xbf,
	0xc5,0x1e,0x67,0xb6,0x05,0x7d,0x90,0xe4,0x4b,0x40,0x9b,0xaf,0xde,0x52,0x80,0x17,
	0x56,0x81,0x3a,0xea,0x82,0x53,0x62,0x8c,0x96,0xfb,0x6f,0x97,0x16,0xc1,0xd4,0x78,
	0xe7,0x7b,0x5a,0xb9,0xeb,0x2a,0x68,0x87,0xd3,0x33,0x45,0x31,0xfe,0xfa,0x1c,0xf4,
	0x86,0x90,0x77,0x73,0xa9,0xd9,0x4a,0xd1,0xcf,0x4a,0x23,0xae,0xf9,0xdb,0xd8,0x09,
	0xdc,0x18,0x0d,0x6a,0x19,0xe4,0x65,0x8c,0x64,0xc6,0xdc,0xc7,0xe3,0xa9,0xb1,0x91,
	0xc8,0x4c,0x9e,0xc1,0x7f,0x3b,0xa3,0xcb,0xdd,0xcf,0x1d,0xf0,0x6e,0x07,0xce,0xdc,
	0xcd,0x0d,0x1e,0x7e,0x11,0x55,0xdf,0x8b,0xab,0x3a,0x3b,0xb6,0x52,0x6e,0xa7,0x7f,
	0xd1,0x00,0xbe,0x33,0x9b,0xf2,0x4a,0xfc,0x9d,0xcf,0xc6,0x8f,0x7b,0xc4,0xe7,0xda,
	0x1c,0x2a,0x6e,0x26
};


static void phys_frame_to_msf(int phys_frame, int *m, int *s, int *f)
{
	*m = phys_frame / (60*75);
	phys_frame -= (*m * 60 * 75);
	*s = phys_frame / 75;
	*f = phys_frame % 75;
}

// device type definition
const device_type SCSI_GDROM = &device_creator<scsi_gdrom_device>;

scsi_gdrom_device::scsi_gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: scsihle_device(mconfig, SCSI_GDROM, "SCSI GDROM", tag, owner, clock, "scsi_gdrom", __FILE__),
		m_cdda(*this, "cdda")
{
}

void scsi_gdrom_device::device_start()
{
	save_item( NAME( lba ) );
	save_item( NAME( blocks ) );
	save_item( NAME( last_lba ) );
	save_item( NAME( bytes_per_sector ) );
	save_item( NAME( num_subblocks ) );
	save_item( NAME( cur_subblock ) );
	save_item( NAME( play_err_flag ) );
}

void scsi_gdrom_device::device_reset()
{
	scsihle_device::device_reset();

	static const UINT8 GDROM_Def_Cmd11_Reply[32] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x19, 0x00, 0x00, 0x08, 0x53, 0x45, 0x20, 0x20, 0x20, 0x20,
		0x20, 0x20, 0x52, 0x65, 0x76, 0x20, 0x36, 0x2E, 0x34, 0x32, 0x39, 0x39, 0x30, 0x33, 0x31, 0x36
	};

	for(int i = 0;i<32;i++)
		GDROM_Cmd11_Reply[i] = GDROM_Def_Cmd11_Reply[i];

	is_file = TRUE;
	cdrom = subdevice<cdrom_image_device>("image")->get_cdrom_file();
	if( !cdrom )
	{
		// try to locate the CHD from a DISK_REGION
		chd_file *chd = get_disk_handle( machine(), tag() );
		if( chd != NULL )
		{
			is_file = FALSE;
			cdrom = cdrom_open( chd );
		}
	}

	if (!cdrom)
	{
		logerror("GDROM: no CD found!\n");
	}

	lba = 0;
	blocks = 0;
	last_lba = 0;
	bytes_per_sector = 2048;
	num_subblocks = 1;
	cur_subblock = 0;
	play_err_flag = 0;
}

void scsi_gdrom_device::device_stop()
{
	if (!is_file)
	{
		if( cdrom )
		{
			cdrom_close( cdrom );
		}
	}
}

cdrom_interface scsi_gdrom_device::cd_intf = { 0, 0 };

static MACHINE_CONFIG_FRAGMENT(scsi_cdrom)
	MCFG_CDROM_ADD("image", scsi_gdrom_device::cd_intf)
	MCFG_SOUND_ADD("cdda", CDDA, 0)
MACHINE_CONFIG_END

machine_config_constructor scsi_gdrom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(scsi_cdrom);
}

// scsicd_exec_command
//
// Execute a SCSI command.

void scsi_gdrom_device::ExecCommand( int *transferLength )
{
	int trk;

	switch ( command[0] )
	{
		case 0x03: // REQUEST SENSE
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x11: // REQ_MODE
			SetPhase( SCSI_PHASE_DATAIN );
			printf("REQ_MODE %02x %02x %02x %02x %02x %02x\n",
				command[0], command[1],
				command[2], command[3],
				command[4], command[5]);
//          if (SCSILengthFromUINT8( &command[ 4 ] ) < 32) return -1;
			transferOffset = command[2];
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x12: // INQUIRY
			logerror("GDROM: REQUEST SENSE\n");
			SetPhase( SCSI_PHASE_DATAOUT );
			//transferOffset = command[2];
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			printf("SET_MODE %02x %02x\n",command[2],command[4]);
			break;

		case 0x15: // MODE SELECT(6)
			logerror("GDROM: MODE SELECT(6) length %x control %x\n", command[4], command[5]);
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x1a: // MODE SENSE(6)
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT8( &command[ 4 ] );
			break;

		case 0x1b: // START STOP UNIT
			if (m_cdda != NULL)
			{
				m_cdda->stop_audio();
			}
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x1e: // PREVENT ALLOW MEDIUM REMOVAL
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x25: // READ CAPACITY
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = 8;
			break;

		case 0x28: // READ(10)

			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			logerror("GDROM: READ(10) at LBA %x for %d blocks (%d bytes)\n", lba, blocks, blocks * bytes_per_sector);

			if (num_subblocks > 1)
			{
				cur_subblock = lba % num_subblocks;
				lba /= num_subblocks;
			}
			else
			{
				cur_subblock = 0;
			}

			if (m_cdda != NULL)
			{
				m_cdda->stop_audio();
			}

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = blocks * bytes_per_sector;
			break;

		case 0x30: // CD_READ
			if (command[1] & 1)
			{
				fatalerror("GDROM: MSF mode used for CD_READ, unsupported\n");
				*transferLength = 0;
			}
			else
			{
				lba = (command[2]<<16 | command[3]<<8 | command[4]) - 150;
				blocks = command[8]<<16 | command[9]<<8 | command[10];

				read_type = (command[1] >> 1) & 7;
				data_select = (command[1]>>4) & 0xf;

				if (read_type != 2) // mode 1
				{
					fatalerror("GDROM: Unhandled read_type %d\n", read_type);
				}

				if (data_select != 2)   // just sector data
				{
					fatalerror("GDROM: Unhandled data_select %d\n", data_select);
				}

				printf("GDROM: CD_READ at LBA %x for %d blocks (%d bytes, read type %d, data select %d)\n", lba, blocks, blocks * bytes_per_sector, read_type, data_select);

				if (num_subblocks > 1)
				{
					cur_subblock = lba % num_subblocks;
					lba /= num_subblocks;
				}
				else
				{
					cur_subblock = 0;
				}

				if (m_cdda != NULL)
				{
					m_cdda->stop_audio();
				}

				SetPhase( SCSI_PHASE_DATAIN );
				*transferLength = blocks * bytes_per_sector;
			}
			break;

		case 0x42: // READ SUB-CHANNEL
//                      logerror("GDROM: READ SUB-CHANNEL type %d\n", command[3]);
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT16( &command[ 7 ] );
			break;

		case 0x43: // READ TOC
		{
			int start_trk = command[6];
			int end_trk = cdrom_get_last_track(cdrom);
			int length;
			int allocation_length = SCSILengthFromUINT16( &command[ 7 ] );

			if( start_trk == 0 )
			{
				start_trk = 1;
			}
			if( start_trk == 0xaa )
			{
				end_trk = start_trk;
			}

			length = 4 + ( 8 * ( ( end_trk - start_trk ) + 1 ) );
			if( length > allocation_length )
			{
				length = allocation_length;
			}
			else if( length < 4 )
			{
				length = 4;
			}

			if (m_cdda != NULL)
			{
				m_cdda->stop_audio();
			}

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = length;
			//debugger_break(machine());
			break;
		}

		// READ TOC (GD-ROM ver.)
		case 0x14:
		{
			int start_trk = command[2];// ok?
			int end_trk = cdrom_get_last_track(cdrom);
			int length;
			int allocation_length = SCSILengthFromUINT16( &command[ 3 ] );

			if( start_trk == 0 )
			{
				start_trk = 1;
			}
			if( start_trk == 0xaa )
			{
				end_trk = start_trk;
			}

			length = 4 + ( 8 * ( ( end_trk - start_trk ) + 1 ) );
			if( length > allocation_length )
			{
				length = allocation_length;
			}
			else if( length < 4 )
			{
				length = 4;
			}

			if (m_cdda != NULL)
			{
				m_cdda->stop_audio();
			}

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = length;
			break;
		}

		case 0x45: // PLAY AUDIO(10)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = SCSILengthFromUINT16( &command[7] );

			// special cases: lba of 0 means MSF of 00:02:00
			if (lba == 0)
			{
				lba = 150;
			}
			else if (lba == 0xffffffff)
			{
				logerror("GDROM: play audio from current not implemented!\n");
			}

			logerror("GDROM: PLAY AUDIO(10) at LBA %x for %x blocks\n", lba, blocks);

			trk = cdrom_get_track(cdrom, lba);

			if (cdrom_get_track_type(cdrom, trk) == CD_TRACK_AUDIO)
			{
				play_err_flag = 0;
				if (m_cdda != NULL)
					m_cdda->start_audio(lba, blocks);
			}
			else
			{
				logerror("GDROM: track is NOT audio!\n");
				play_err_flag = 1;
			}

			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x48: // PLAY AUDIO TRACK/INDEX
			// be careful: tracks here are zero-based, but the SCSI command
			// uses the real CD track number which is 1-based!
			lba = cdrom_get_track_start(cdrom, command[4]-1);
			blocks = cdrom_get_track_start(cdrom, command[7]-1) - lba;
			if (command[4] > command[7])
			{
				blocks = 0;
			}

			if (command[4] == command[7])
			{
				blocks = cdrom_get_track_start(cdrom, command[4]) - lba;
			}

			if (blocks && cdrom)
			{
				if (m_cdda != NULL)
					m_cdda->start_audio(lba, blocks);
			}

			logerror("GDROM: PLAY AUDIO T/I: strk %d idx %d etrk %d idx %d frames %d\n", command[4], command[5], command[7], command[8], blocks);
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x4b: // PAUSE/RESUME
			if (cdrom)
			{
				if (m_cdda != NULL)
					m_cdda->pause_audio((command[8] & 0x01) ^ 0x01);
			}

			logerror("GDROM: PAUSE/RESUME: %s\n", command[8]&1 ? "RESUME" : "PAUSE");
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x55: // MODE SELECT(10)
			logerror("GDROM: MODE SELECT length %x control %x\n", command[7]<<8 | command[8], command[1]);
			SetPhase( SCSI_PHASE_DATAOUT );
			*transferLength = SCSILengthFromUINT16( &command[ 7 ] );
			break;

		case 0x5a: // MODE SENSE(10)
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = SCSILengthFromUINT16( &command[ 7 ] );
			break;

		case 0xa5: // PLAY AUDIO(12)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

			// special cases: lba of 0 means MSF of 00:02:00
			if (lba == 0)
			{
				lba = 150;
			}
			else if (lba == 0xffffffff)
			{
				logerror("GDROM: play audio from current not implemented!\n");
			}

			logerror("GDROM: PLAY AUDIO(12) at LBA %x for %x blocks\n", lba, blocks);

			trk = cdrom_get_track(cdrom, lba);

			if (cdrom_get_track_type(cdrom, trk) == CD_TRACK_AUDIO)
			{
				play_err_flag = 0;
				if (m_cdda != NULL)
					m_cdda->start_audio(lba, blocks);
			}
			else
			{
				logerror("GDROM: track is NOT audio!\n");
				play_err_flag = 1;
			}
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0xa8: // READ(12)
			lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			blocks = command[7]<<16 | command[8]<<8 | command[9];

			logerror("GDROM: READ(12) at LBA %x for %x blocks (%x bytes)\n", lba, blocks, blocks * bytes_per_sector);

			if (num_subblocks > 1)
			{
				cur_subblock = lba % num_subblocks;
				lba /= num_subblocks;
			}
			else
			{
				cur_subblock = 0;
			}

			if (m_cdda != NULL)
			{
				m_cdda->stop_audio();
			}

			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = blocks * bytes_per_sector;
			break;

		case 0xbb: // SET CD SPEED
			logerror("GDROM: SET CD SPEED to %d kbytes/sec.\n", command[2]<<8 | command[3]);
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x70:
			SetPhase( SCSI_PHASE_STATUS );
			*transferLength = 0;
			break;

		case 0x71:
			SetPhase( SCSI_PHASE_DATAIN );
			*transferLength = sizeof(GDROM_Cmd71_Reply);
			break;

		case 0x40:
			if(command[1] & 0xf)
			{
				SetPhase( SCSI_PHASE_DATAIN );
				*transferLength = 0xe;
			}
			else
			{
				printf("command 0x40: unhandled subchannel request\n");
			}
			break;

		default:
			scsihle_device::ExecCommand( transferLength );
			break;
	}
}

// scsicd_read_data
//
// Read data from the device resulting from the execution of a command

void scsi_gdrom_device::ReadData( UINT8 *data, int dataLength )
{
	int i;
	UINT32 last_phys_frame;
	UINT32 temp;
	UINT8 tmp_buffer[2048];

	switch ( command[0] )
	{
		case 0x03: // REQUEST SENSE
			logerror("GDROM: Reading REQUEST SENSE data\n");

			memset( data, 0, dataLength );

			data[0] = 0x71; // deferred error

			if (m_cdda != NULL && m_cdda->audio_active())
			{
				data[12] = 0x00;
				data[13] = 0x11;    // AUDIO PLAY OPERATION IN PROGRESS
			}
			else if (play_err_flag)
			{
				play_err_flag = 0;
				data[12] = 0x64;    // ILLEGAL MODE FOR THIS TRACK
				data[13] = 0x00;
			}
			// (else 00/00 means no error to report)
			break;

		case 0x11: // REQ_MODE
			printf("REQ_MODE: dataLength %d\n", dataLength);
			memcpy(data, &GDROM_Cmd11_Reply[transferOffset], (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			break;

		case 0x25: // READ CAPACITY
			logerror("GDROM: READ CAPACITY\n");

			temp = cdrom_get_track_start(cdrom, 0xaa);
			temp--; // return the last used block on the disc

			data[0] = (temp>>24) & 0xff;
			data[1] = (temp>>16) & 0xff;
			data[2] = (temp>>8) & 0xff;
			data[3] = (temp & 0xff);
			data[4] = 0;
			data[5] = 0;
			data[6] = (bytes_per_sector>>8)&0xff;
			data[7] = (bytes_per_sector & 0xff);
			break;

		case 0x28: // READ(10)
		case 0xa8: // READ(12)
			logerror("GDROM: read %x dataLength, \n", dataLength);
			if ((cdrom) && (blocks))
			{
				while (dataLength > 0)
				{
					if (!cdrom_read_data(cdrom, lba, tmp_buffer, CD_TRACK_MODE1))
					{
						logerror("GDROM: CD read error!\n");
					}

					logerror("True LBA: %d, buffer half: %d\n", lba, cur_subblock * bytes_per_sector);

					memcpy(data, &tmp_buffer[cur_subblock * bytes_per_sector], bytes_per_sector);

					cur_subblock++;
					if (cur_subblock >= num_subblocks)
					{
						cur_subblock = 0;

						lba++;
						blocks--;
					}

					last_lba = lba;
					dataLength -= bytes_per_sector;
					data += bytes_per_sector;
				}
			}
			break;

		case 0x30: // CD_READ
			logerror("GDROM: read %x dataLength, \n", dataLength);
			if ((cdrom) && (blocks))
			{
				while (dataLength > 0)
				{
					if (!cdrom_read_data(cdrom, lba, tmp_buffer, CD_TRACK_MODE1))
					{
						logerror("GDROM: CD read error!\n");
					}

					logerror("True LBA: %d, buffer half: %d\n", lba, cur_subblock * bytes_per_sector);

					memcpy(data, &tmp_buffer[cur_subblock * bytes_per_sector], bytes_per_sector);

					cur_subblock++;
					if (cur_subblock >= num_subblocks)
					{
						cur_subblock = 0;

						lba++;
						blocks--;
					}

					last_lba = lba;
					dataLength -= bytes_per_sector;
					data += bytes_per_sector;
				}
			}

		case 0x42: // READ SUB-CHANNEL
			switch (command[3])
			{
				case 1: // return current position
				{
					int audio_active;
					int msf;

					if (!cdrom)
					{
						return;
					}

					logerror("GDROM: READ SUB-CHANNEL Time = %x, SUBQ = %x\n", command[1], command[2]);

					msf = command[1] & 0x2;

					audio_active = m_cdda != NULL && m_cdda->audio_active();
					if (audio_active)
					{
						if (m_cdda->audio_paused())
						{
							data[1] = 0x12;     // audio is paused
						}
						else
						{
							data[1] = 0x11;     // audio in progress
						}
					}
					else
					{
						if (m_cdda != NULL && m_cdda->audio_ended())
						{
							data[1] = 0x13; // ended successfully
						}
						else
						{
//                          data[1] = 0x14;    // stopped due to error
							data[1] = 0x15; // No current audio status to return
						}
					}

					// if audio is playing, get the latest LBA from the CDROM layer
					if (audio_active)
					{
						last_lba = m_cdda->get_audio_lba();
					}
					else
					{
						last_lba = 0;
					}

					data[2] = 0;
					data[3] = 12;       // data length
					data[4] = 0x01; // sub-channel format code
					data[5] = 0x10 | (audio_active ? 0 : 4);
					data[6] = cdrom_get_track(cdrom, last_lba) + 1; // track
					data[7] = 0;    // index

					last_phys_frame = last_lba;

					if (msf)
					{
						int m,s,f;
						phys_frame_to_msf(last_phys_frame, &m, &s, &f);
						data[8] = 0;
						data[9] = m;
						data[10] = s;
						data[11] = f;
					}
					else
					{
						data[8] = last_phys_frame>>24;
						data[9] = (last_phys_frame>>16)&0xff;
						data[10] = (last_phys_frame>>8)&0xff;
						data[11] = last_phys_frame&0xff;
					}

					last_phys_frame -= cdrom_get_track_start(cdrom, data[6] - 1);

					if (msf)
					{
						int m,s,f;
						phys_frame_to_msf(last_phys_frame, &m, &s, &f);
						data[12] = 0;
						data[13] = m;
						data[14] = s;
						data[15] = f;
					}
					else
					{
						data[12] = last_phys_frame>>24;
						data[13] = (last_phys_frame>>16)&0xff;
						data[14] = (last_phys_frame>>8)&0xff;
						data[15] = last_phys_frame&0xff;
					}
					break;
				}
				default:
					logerror("GDROM: Unknown subchannel type %d requested\n", command[3]);
			}
			break;

		case 0x43: // READ TOC
			/*
			    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
			    That makes sense in terms of how real-world CDs are referred to, but
			    our internal routines for tracks use "0" as track 1.  That probably
			    should be fixed...
			*/
			logerror("GDROM: READ TOC, format = %d time=%d\n", command[2]&0xf,(command[1]>>1)&1);
			switch (command[2] & 0x0f)
			{
				case 0:     // normal
					{
						int start_trk;
						int end_trk;
						int len;
						int in_len;
						int dptr;
						UINT32 tstart;

						start_trk = command[6];
						if( start_trk == 0 )
						{
							start_trk = 1;
						}

						end_trk = cdrom_get_last_track(cdrom);
						len = (end_trk * 8) + 2;

						// the returned TOC DATA LENGTH must be the full amount,
						// regardless of how much we're able to pass back due to in_len
						dptr = 0;
						data[dptr++] = (len>>8) & 0xff;
						data[dptr++] = (len & 0xff);
						data[dptr++] = 1;
						data[dptr++] = end_trk;

						if( start_trk == 0xaa )
						{
							end_trk = 0xaa;
						}

						in_len = command[7]<<8 | command[8];

						for (i = start_trk; i <= end_trk; i++)
						{
							int cdrom_track = i;
							if( cdrom_track != 0xaa )
							{
								cdrom_track--;
							}

							if( dptr >= in_len )
							{
								break;
							}

							data[dptr++] = 0;
							data[dptr++] = cdrom_get_adr_control(cdrom, cdrom_track);
							data[dptr++] = i;
							data[dptr++] = 0;

							tstart = cdrom_get_track_start(cdrom, cdrom_track);
							if ((command[1]&2)>>1)
								tstart = lba_to_msf(tstart);
							data[dptr++] = (tstart>>24) & 0xff;
							data[dptr++] = (tstart>>16) & 0xff;
							data[dptr++] = (tstart>>8) & 0xff;
							data[dptr++] = (tstart & 0xff);
						}
					}
					break;
				default:
					logerror("GDROM: Unhandled READ TOC format %d\n", command[2]&0xf);
					break;
			}
			break;

		case 0x14: // READ TOC (GD-ROM ver.)
			/*
			    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
			    That makes sense in terms of how real-world CDs are referred to, but
			    our internal routines for tracks use "0" as track 1.  That probably
			    should be fixed...
			*/
			printf("GDROM: READ TOC, format = %d time=%d\n", command[2]&0xf,(command[1]>>1)&1);
			switch (command[1] & 0x0f)
			{
				case 0:     // normal
					{
						int start_trk;
						int end_trk;
						int len;
						int in_len;
						int dptr;
						UINT32 tstart;

						start_trk = command[2];
						if( start_trk == 0 )
						{
							start_trk = 1;
						}

						end_trk = cdrom_get_last_track(cdrom);
						len = (end_trk * 8) + 2;

						// the returned TOC DATA LENGTH must be the full amount,
						// regardless of how much we're able to pass back due to in_len
						dptr = 0;
						data[dptr++] = (len>>8) & 0xff;
						data[dptr++] = (len & 0xff);
						data[dptr++] = 1;
						data[dptr++] = end_trk;

						if( start_trk == 0xaa )
						{
							end_trk = 0xaa;
						}

						in_len = command[3]<<8 | command[4];

						for (i = start_trk; i <= end_trk; i++)
						{
							int cdrom_track = i;
							if( cdrom_track != 0xaa )
							{
								cdrom_track--;
							}

							if( dptr >= in_len )
							{
								break;
							}

							data[dptr++] = 0;
							data[dptr++] = cdrom_get_adr_control(cdrom, cdrom_track);
							data[dptr++] = i;
							data[dptr++] = 0;

							tstart = cdrom_get_track_start(cdrom, cdrom_track);
							if ((command[1]&2)>>1)
								tstart = lba_to_msf(tstart);
							data[dptr++] = (tstart>>24) & 0xff;
							data[dptr++] = (tstart>>16) & 0xff;
							data[dptr++] = (tstart>>8) & 0xff;
							data[dptr++] = (tstart & 0xff);
						}
					}
					break;
				default:
					logerror("GDROM: Unhandled READ TOC format %d\n", command[2]&0xf);
					break;
			}
			break;

		case 0x1a: // MODE SENSE(6)
		case 0x5a: // MODE SENSE(10)
			logerror("GDROM: MODE SENSE page code = %x, PC = %x\n", command[2] & 0x3f, (command[2]&0xc0)>>6);

			switch (command[2] & 0x3f)
			{
				case 0xe:   // CD Audio control page
					data[0] = 0x8e; // page E, parameter is savable
					data[1] = 0x0e; // page length
					data[2] = 0x04; // IMMED = 1, SOTC = 0
					data[3] = data[4] = data[5] = data[6] = data[7] = 0; // reserved

					// connect each audio channel to 1 output port
					data[8] = 1;
					data[10] = 2;
					data[12] = 4;
					data[14] = 8;

					// indicate max volume
					data[9] = data[11] = data[13] = data[15] = 0xff;
					break;

				default:
					logerror("GDROM: MODE SENSE unknown page %x\n", command[2] & 0x3f);
					break;
			}
			break;

		case 0x71:
			memcpy(data, &GDROM_Cmd71_Reply[0], sizeof(GDROM_Cmd71_Reply));
			break;

		case 0x40: // Get Subchannel status
			data[0] = 0; // Reserved
			data[1] = 0x15; // Audio Playback status (todo)
			data[2] = 0;
			data[3] = 0x0e; // header size
			data[4] = 0; // ?
			data[5] = 1; // Track Number
			data[6] = 1; // gap #1
			data[7] = 0; // ?
			data[8] = 0; // ?
			data[9] = 0; // ?
			data[0xa] = 0; // ?
			data[0xb] = 0; // FAD >> 16
			data[0xc] = 0; // FAD >> 8
			data[0xd] = 0x96; // FAD >> 0
			break;

		default:
			scsihle_device::ReadData( data, dataLength );
			break;
	}
}

// scsicd_write_data
//
// Write data to the CD-ROM device as part of the execution of a command

void scsi_gdrom_device::WriteData( UINT8 *data, int dataLength )
{
	switch (command[ 0 ])
	{
		case 0x15: // MODE SELECT(6)
		case 0x55: // MODE SELECT(10)
			logerror("GDROM: MODE SELECT page %x\n", data[0] & 0x3f);

			switch (data[0] & 0x3f)
			{
				case 0x0:   // vendor-specific
					// check for SGI extension to force 512-byte blocks
					if ((data[3] == 8) && (data[10] == 2))
					{
						logerror("GDROM: Experimental SGI 512-byte block extension enabled\n");

						bytes_per_sector = 512;
						num_subblocks = 4;
					}
					else
					{
						logerror("GDROM: Unknown vendor-specific page!\n");
					}
					break;

				case 0xe:   // audio page
					logerror("Ch 0 route: %x vol: %x\n", data[8], data[9]);
					logerror("Ch 1 route: %x vol: %x\n", data[10], data[11]);
					logerror("Ch 2 route: %x vol: %x\n", data[12], data[13]);
					logerror("Ch 3 route: %x vol: %x\n", data[14], data[15]);
					break;
			}
			break;

		case 0x12:
			memcpy(&GDROM_Cmd11_Reply[transferOffset], data, (dataLength >= 32-transferOffset) ? 32-transferOffset : dataLength);
			break;

		default:
			scsihle_device::WriteData( data, dataLength );
			break;
	}
}

void scsi_gdrom_device::GetDevice( void **_cdrom )
{
	*(cdrom_file **)_cdrom = cdrom;
}

void scsi_gdrom_device::SetDevice( void *_cdrom )
{
	cdrom = (cdrom_file *) _cdrom;
}

int scsi_gdrom_device::GetSectorBytes()
{
	return bytes_per_sector;
}

// device type definition
const device_type GDROM = &device_creator<gdrom_device>;

gdrom_device::gdrom_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: atapi_hle_device(mconfig, GDROM, "GDROM", tag, owner, clock, "gdrom", __FILE__)
{
}

static MACHINE_CONFIG_FRAGMENT( gdrom )
	MCFG_DEVICE_ADD("device", SCSI_GDROM, 0)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor gdrom_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( gdrom );
}

void gdrom_device::device_start()
{
	memset(m_identify_buffer, 0, sizeof(m_identify_buffer));

	m_identify_buffer[ 0 ] = 0x8600; // ATAPI device, cmd set 6 compliant, DRQ within 3 ms of PACKET command

	m_identify_buffer[ 23 ] = ('S' << 8) | 'E';
	m_identify_buffer[ 24 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 25 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 26 ] = (' ' << 8) | ' ';

	m_identify_buffer[ 27 ] = ('C' << 8) | 'D';
	m_identify_buffer[ 28 ] = ('-' << 8) | 'R';
	m_identify_buffer[ 29 ] = ('O' << 8) | 'M';
	m_identify_buffer[ 30 ] = (' ' << 8) | 'D';
	m_identify_buffer[ 31 ] = ('R' << 8) | 'I';
	m_identify_buffer[ 32 ] = ('V' << 8) | 'E';
	m_identify_buffer[ 33 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 34 ] = (' ' << 8) | ' ';
	m_identify_buffer[ 35 ] = ('6' << 8) | '.';
	m_identify_buffer[ 36 ] = ('4' << 8) | '2';
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

void gdrom_device::perform_diagnostic()
{
	m_error = IDE_ERROR_DIAGNOSTIC_PASSED;
}

void gdrom_device::process_buffer()
{
	atapi_hle_device::process_buffer();
	m_sector_number = 0x80 | GDROM_PAUSE_STATE; /// HACK: find out when this should be updated
}

void gdrom_device::identify_packet_device()
{
}
