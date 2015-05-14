// license:BSD-3-Clause
// copyright-holders:smf, Mariusz Wojcieszek
/***************************************************************************

    Matsushita/Panasonic CDR521/522 type CDROM drive emulation

Notes:
This version implements the drive found in Amiga CDTV. There are
different drives supporting this command-set (look in the sbpcd driver
in Linux for examples). Most drives support the exact same command
structure, but the command numbers differ. Eventually this driver
can be expanded with support for the other drives as needed.

***************************************************************************/


#include "emu.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"
#include "machine/matsucd.h"


#define MATSU_STATUS_READY      ( 1 << 0 )  /* driver ready */
#define MATSU_STATUS_DOORLOCKED ( 1 << 1 )  /* door locked */
#define MATSU_STATUS_PLAYING    ( 1 << 2 )  /* drive playing */
#define MATSU_STATUS_SUCCESS    ( 1 << 3 )  /* last command was successful */
#define MATSU_STATUS_ERROR      ( 1 << 4 )  /* last command failed */
#define MATSU_STATUS_MOTOR      ( 1 << 5 )  /* spinning */
#define MATSU_STATUS_MEDIA      ( 1 << 6 )  /* media present (in caddy or tray) */
#define MATSU_STATUS_DOORCLOSED ( 1 << 7 )  /* tray status */

struct matsucd
{
	UINT8   enabled;        /* /ENABLE - Unit enabled */
	UINT8   cmd_signal;     /* /CMD - Command mode   */
	UINT8   stch_signal;    /* /STCH - Status Changed */
	UINT8   sten_signal;    /* /STEN - Status Enabled */
	UINT8   scor_signal;    /* /STEN - Subcode Ready */
	UINT8   input[16];
	UINT8   input_pos;
	UINT8   output[16];
	UINT8   output_pos;
	UINT8   output_len;
	UINT8   status;
	UINT8   motor;
	UINT16  sector_size;
	UINT32  lba;
	UINT16  num_blocks;
	UINT16  xfer_offset;
	UINT8   sector_buffer[CD_MAX_SECTOR_DATA];
	UINT8   cdda_set;
	void (*sten_cb)( running_machine &machine, int level ); /* Status enabled callback */
	void (*stch_cb)( running_machine &machine, int level ); /* Status changed callback */
	void (*scor_cb)( running_machine &machine, int level ); /* Subcode ready callback */
	cdrom_file *cdrom;
	cdda_device *cdda;
	emu_timer *frame_timer;
};

static matsucd cd;

#define MSF2LBA(msf) (((msf >> 16) & 0xff) * 60 * 75 + ((msf >> 8) & 0xff) * 75 + ((msf >> 0) & 0xff))
#define LBA2MSF(lba) (((lba / (75 * 60)) << 16) | (((lba / 75) % 60) << 8) | (lba % 75))

static TIMER_CALLBACK(matsu_subcode_proc);

void matsucd_init( cdrom_image_device *cdrom_device, const char *cdda_tag )
{
	memset(&cd, 0, sizeof( matsucd ) );

	cd.cdrom = cdrom_device->get_cdrom_file();
	cd.cdda = cdrom_device->machine().device<cdda_device>(cdda_tag);

	cd.frame_timer = cdrom_device->machine().scheduler().timer_alloc(FUNC(matsu_subcode_proc));

	cd.stch_signal = 1;
}

void matsucd_set_status_enabled_callback( void (*sten_cb)( running_machine &machine, int level ) )
{
	/* add the callback for status enabled signal */
	cd.sten_cb = sten_cb;
}

void matsucd_set_status_changed_callback( void (*stch_cb)( running_machine &machine, int level ) )
{
	/* add the callback for status changed signal */
	cd.stch_cb = stch_cb;
}

void matsucd_set_subcode_ready_callback( void (*scor_cb)( running_machine &machine, int level ) )
{
	/* add the callback for subcode ready signal */
	cd.scor_cb = scor_cb;
}

static int matsucd_getsector_type( void )
{
	switch( cd.sector_size )
	{
		case 2048: return CD_TRACK_MODE1;
		case 2324: return CD_TRACK_MODE2_FORM2;
		case 2336: return CD_TRACK_MODE2;
		case 2352: return CD_TRACK_MODE2_RAW;

		default: logerror( "MATSUCD: Sector size %d unsupported!\n", cd.sector_size ); break;
	}

	return CD_TRACK_RAW_DONTCARE;
}

void matsucd_read_next_block( void )
{
	cd.xfer_offset = 0;

	if ( cd.num_blocks )
	{
		cd.lba++;
		cd.num_blocks--;

		if (!cdrom_read_data(cd.cdrom, cd.lba, cd.sector_buffer, matsucd_getsector_type()))
		{
			logerror( "MATSUCD - Warning: Read error on CD!\n" );
		}
	}
}

int matsucd_get_next_byte( UINT8 *data )
{
	/* no more data to read on this sector */
	if ( cd.xfer_offset >= cd.sector_size )
		return -1;

	if ( data )
		*data = cd.sector_buffer[cd.xfer_offset++];

	return 0;
}

static void matsucd_cdda_stop( running_machine &machine )
{
	if (cd.cdda != NULL)
	{
		cd.cdda->stop_audio();
		cd.frame_timer->reset(  );
	}
}

static void matsucd_cdda_play( running_machine &machine, UINT32 lba, UINT32 num_blocks )
{
	if (cd.cdda != NULL)
	{
		cd.cdda->start_audio(lba, num_blocks);
		cd.frame_timer->adjust(attotime::from_hz( 75 ));
	}
}

static void matsucd_cdda_pause( running_machine &machine, int pause )
{
	if (cd.cdda != NULL)
	{
		cd.cdda->pause_audio(pause);

		if ( pause )
		{
			cd.frame_timer->reset(  );
		}
		else
		{
			cd.frame_timer->adjust(attotime::from_hz( 75 ));
		}
	}
}

static UINT8 matsucd_cdda_getstatus( running_machine &machine, UINT32 *lba )
{
	if ( lba ) *lba = 0;

	if (cd.cdda != NULL)
	{
		if (cd.cdda->audio_active())
		{
			if ( lba ) *lba = cd.cdda->get_audio_lba();

			if (cd.cdda->audio_paused())
			{
				return 0x12;    /* audio paused */
			}
			else
			{
				return 0x11;    /* audio in progress */
			}
		}
		else if (cd.cdda->audio_ended())
		{
			return 0x13;    /* audio ended */
		}
	}

	return 0x15;    /* no audio status */
}

void matsucd_enable_w( int level )
{
	cd.enabled = ( level ) ? 0 : 1;
}

void matsucd_cmd_w( int level )
{
	cd.cmd_signal = ( level ) ? 0 : 1;
}

int matsucd_stch_r( void )
{
	return cd.stch_signal ? 0 : 1;
}

int matsucd_sten_r( void )
{
	return cd.sten_signal ? 0 : 1;
}

int matsucd_scor_r( void )
{
	return cd.scor_signal ? 0 : 1;
}

static void update_status_enable( running_machine &machine, int level )
{
	cd.sten_signal = level;

	if ( cd.sten_cb )
	{
		(*cd.sten_cb)(machine, cd.sten_signal);
	}
}

static void update_status_changed( running_machine &machine, int level )
{
	cd.stch_signal = level;

	if ( cd.stch_cb )
	{
		(*cd.stch_cb)(machine, cd.stch_signal);
	}
}

static void update_subcode_ready( running_machine &machine, int level )
{
	cd.scor_signal = level;

	if ( cd.scor_cb )
	{
		(*cd.scor_cb)(machine, cd.scor_signal);
	}
}

static TIMER_CALLBACK(matsucd_set_status_end)
{
	update_status_changed( machine, 1 );
}

static void matsucd_set_status( running_machine &machine, UINT8 status )
{
	if ( status != cd.status )
	{
		cd.status = status;

		if ( cd.stch_signal != 0 )
		{
			update_status_changed( machine, 0 );
			machine.scheduler().timer_set(attotime::from_msec(1), FUNC(matsucd_set_status_end));
		}
	}
}

static TIMER_CALLBACK(matsu_subcode_proc)
{
	(void)param;

	if (cd.cdda != NULL)
	{
		UINT8   s = matsucd_cdda_getstatus(machine, NULL);
		UINT8   newstatus = cd.status;

		if ( s == 0x11 || s == 0x12 )
		{
			if ( s == 0x11 )
			{
				update_subcode_ready( machine, 1 );
				update_subcode_ready( machine, 0 );
			}

			newstatus |= MATSU_STATUS_PLAYING;

			cd.frame_timer->adjust(attotime::from_hz( 75 ));
		}
		else
		{
			newstatus &= ~MATSU_STATUS_PLAYING;
		}

		matsucd_set_status( machine, newstatus );
	}
}

static void matsucd_command_error( running_machine &machine )
{
	UINT8   newstatus = cd.status;

	newstatus &= ~MATSU_STATUS_SUCCESS;
	newstatus |= MATSU_STATUS_ERROR;

	matsucd_set_status( machine, newstatus );
}

static void matsucd_complete_cmd( running_machine &machine, UINT8 len )
{
	UINT8   newstatus = cd.status;

	cd.input_pos = 0;
	cd.output_pos = 0;
	cd.output_len = len;

	newstatus &= ~MATSU_STATUS_ERROR;
	newstatus |= MATSU_STATUS_SUCCESS;

	matsucd_set_status( machine, newstatus );

	update_status_enable( machine, 1 );
	update_status_enable( machine, 0 );
}

UINT8 matsucd_response_r( running_machine &machine )
{
	UINT8   v = cd.output[cd.output_pos++];

	if ( cd.output_pos < cd.output_len )
	{
		update_status_enable( machine, 1 );
		update_status_enable( machine, 0 );
	}

	return v;
}

void matsucd_command_w( running_machine &machine, UINT8 data )
{
	UINT8   cmd;

	/* make sure we're enabled */
	if ( cd.enabled == 0 )
		return;

	/* make sure /CMD is asserted */
	if ( cd.cmd_signal == 0 )
		return;

	if ( cd.cdda_set == 0 )
	{
		if ( cd.cdrom )
			cd.cdda->set_cdrom(cd.cdrom);

		cd.cdda_set = 1;
	}

	cd.input[cd.input_pos++] = data;

	cmd = cd.input[0];

	switch( cmd )
	{
		case 0x01:  /* seek */
		{
			if ( cd.input_pos < 7 )
				return;

			/* stop CDDA audio if necessary */
			matsucd_cdda_stop(machine);

			cd.motor = 1;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x02:  /* read sectors */
		{
			if ( cd.input_pos < 7 )
				return;

			/* stop CDDA audio if necessary */
			matsucd_cdda_stop(machine);

			/* LBA */
			cd.lba = cd.input[1];
			cd.lba <<= 8;
			cd.lba |= cd.input[2];
			cd.lba <<= 8;
			cd.lba |= cd.input[3];

			/* Number of blocks */
			cd.num_blocks = cd.input[4];
			cd.num_blocks <<= 8;
			cd.num_blocks |= cd.input[5];

			/* Reset transfer count */
			cd.xfer_offset = 0;

			/* go ahead and cache the first block */
			if (!cdrom_read_data(cd.cdrom, cd.lba, cd.sector_buffer, matsucd_getsector_type()))
			{
				logerror( "MATSUCD - Warning: Read error on CD!\n" );
				matsucd_command_error( machine );
				return;
			}

			cd.motor = 1;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x04:  /* motor on */
		{
			if ( cd.input_pos < 7 )
				return;

			cd.motor = 1;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x05:  /* motor off */
		{
			if ( cd.input_pos < 7 )
				return;

			/* stop CDDA audio if necessary */
			matsucd_cdda_stop(machine);

			cd.motor = 0;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x09:  /* play audio cd, LBA mode */
		{
			UINT32  lba, numblocks;

			if ( cd.input_pos < 7 )
				return;

			lba = cd.input[1];
			lba <<= 8;
			lba |= cd.input[2];
			lba <<= 8;
			lba |= cd.input[3];

			numblocks = cd.input[4];
			numblocks <<= 8;
			numblocks |= cd.input[5];
			numblocks <<= 8;
			numblocks |= cd.input[6];

			matsucd_cdda_play( machine, lba, numblocks );

			cd.motor = 1;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x0a:  /* play audio cd, MSF mode */
		{
			UINT32  start, end, lba_start, lba_end;

			if ( cd.input_pos < 7 )
				return;

			start = cd.input[1];
			start <<= 8;
			start |= cd.input[2];
			start <<= 8;
			start |= cd.input[3];

			end = cd.input[4];
			end <<= 8;
			end |= cd.input[5];
			end <<= 8;
			end |= cd.input[6];

			lba_start = MSF2LBA( start );
			lba_end = MSF2LBA( end );

			if ( end == 0xffffff )
			{
				lba_end = cdrom_get_track_start(cd.cdrom,cdrom_get_last_track(cd.cdrom)-1);
				lba_end += cdrom_get_toc(cd.cdrom)->tracks[cdrom_get_last_track(cd.cdrom)-1].frames;
			}

			if ( lba_end <= lba_start )
			{
				matsucd_cdda_stop(machine);
			}
			else
			{
				matsucd_cdda_play( machine, lba_start, lba_end - lba_start );
				cd.motor = 1;
			}

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x0b:  /* play audio track and index */
		{
			UINT8   track_start = cd.input[1];
			UINT8   index_start = cd.input[2];
			UINT8   track_end = cd.input[3];
			UINT8   index_end = cd.input[4];
			UINT32  lba_start, lba_end;

			/* TODO: Add index support once the CDDA engine supports it */
			(void)index_start;
			(void)index_end;

			/* sanitize values */
			if ( track_start == 0 ) track_start++;
			if ( track_end == 0 ) track_end++;
			if ( track_end > cdrom_get_last_track(cd.cdrom) )
				track_end = cdrom_get_last_track(cd.cdrom);

			/* find the start and stop positions */
			lba_start = cdrom_get_track_start(cd.cdrom,track_start-1);
			lba_end = cdrom_get_track_start(cd.cdrom,track_end-1);

			lba_end += cdrom_get_toc(cd.cdrom)->tracks[track_end-1].frames;

			if ( lba_end <= lba_start )
			{
				matsucd_cdda_stop(machine);
			}
			else
			{
				matsucd_cdda_play( machine, lba_start, lba_end - lba_start );
				cd.motor = 1;
			}

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x81:  /* status read */
		{
			UINT8   newstatus = cd.status;

			newstatus &= MATSU_STATUS_SUCCESS | MATSU_STATUS_ERROR | MATSU_STATUS_PLAYING;
			newstatus |= MATSU_STATUS_READY;

			if (cd.cdrom)
			{
				newstatus |= MATSU_STATUS_MEDIA;
			}

			if (cd.motor)
				newstatus |= MATSU_STATUS_MOTOR;

			cd.output[0] = newstatus;

			matsucd_set_status( machine, newstatus );

			matsucd_complete_cmd( machine, 1 );
		}
		break;

		case 0x82:  /* error read */
		{
			if ( cd.input_pos < 7 )
				return;

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 6 );
		}
		break;

		case 0x84:  /* set mode */
		{
			if ( cd.input_pos < 7 )
				return;

			cd.sector_size = cd.input[2];
			cd.sector_size <<= 8;
			cd.sector_size |= cd.input[3];

			memset( cd.output, 0, 6 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0x87:  /* read SUBQ */
		{
			int     msfmode;
			UINT32  lba;
			UINT8   track;

			if ( cd.input_pos < 7 )
				return;

			msfmode = (cd.input[1] & 0x02) ? 1 : 0;

			memset( cd.output, 0, 13 );

			cd.output[0] = matsucd_cdda_getstatus( machine, &lba );

			if ( lba > 0 )
			{
				UINT32  disk_pos;
				UINT32  track_pos;

				track = cdrom_get_track(cd.cdrom, lba);

				cd.output[1] = cdrom_get_adr_control(cd.cdrom, track);
				cd.output[2] = track+1;
				cd.output[3] = 0; /* index */

				disk_pos = lba;
				if ( msfmode ) disk_pos = LBA2MSF(disk_pos);

				cd.output[4] = (disk_pos >> 24) & 0xff;
				cd.output[5] = (disk_pos >> 16) & 0xff;
				cd.output[6] = (disk_pos >> 8) & 0xff;
				cd.output[7] = (disk_pos) & 0xff;

				track_pos = lba - cdrom_get_track_start(cd.cdrom, track);
				if ( msfmode ) track_pos = LBA2MSF(track_pos);

				cd.output[8] = (track_pos >> 24) & 0xff;
				cd.output[9] = (track_pos >> 16) & 0xff;
				cd.output[10] = (track_pos >> 8) & 0xff;
				cd.output[11] = (track_pos) & 0xff;

				/* TODO: UPC flag at offset 12 */
				cd.output[12] = 0;
			}

			matsucd_complete_cmd( machine, 13 );
		}
		break;

		case 0x89:  /* read disk info */
		{
			UINT32  end;

			if ( cd.input_pos < 7 )
				return;

			memset( cd.output, 0, 5 );

			cd.output[0] = cdrom_get_last_track(cd.cdrom) ? 1 : 0;
			cd.output[1] = cdrom_get_last_track(cd.cdrom);
			end = cdrom_get_track_start(cd.cdrom,cd.output[1]-1);
			end += cdrom_get_toc(cd.cdrom)->tracks[cd.output[1]-1].frames;
			end = LBA2MSF(end);
			cd.output[2] = (end >> 16) & 0xff;
			cd.output[3] = (end >> 8) & 0xff;
			cd.output[4] = (end) & 0xff;

			matsucd_complete_cmd( machine, 5 );
		}
		break;

		case 0x8a:  /* read toc */
		{
			UINT8   track;
			int     msfmode;
			UINT32  track_start;

			if ( cd.input_pos < 7 )
				return;

			/* stop CDDA audio if necessary */
			matsucd_cdda_stop(machine);

			track = cd.input[2];
			msfmode = (cd.input[1] & 0x02) ? 1 : 0;

			if ( cd.cdrom == NULL )
			{
				logerror( "MATSUCD - Warning: Reading TOC without a CD!\n" );
				matsucd_command_error( machine );
				return;
			}

			if ( track > cdrom_get_last_track(cd.cdrom) )
			{
				logerror( "MATSUCD - Warning: Reading invalid track entry from TOC!\n" );
				matsucd_command_error( machine );
				return;
			}

			memset( cd.output, 0, 7 );

			track_start = cdrom_get_track_start(cd.cdrom, track > 0 ? (track-1) : track );
			if ( msfmode ) track_start = LBA2MSF( track_start );

			cd.output[1] = cdrom_get_adr_control(cd.cdrom, track > 0 ? (track-1) : track);
			cd.output[2] = track;
			cd.output[3] = (track == 0 ) ? cdrom_get_last_track(cd.cdrom) : 0;
			cd.output[4] = (track_start >> 24) & 0xff;
			cd.output[5] = (track_start >> 16) & 0xff;
			cd.output[6] = (track_start >> 8) & 0xff;
			cd.output[7] = (track_start) & 0xff;

			cd.motor = 1;

			matsucd_complete_cmd( machine, 8 );
		}
		break;

		case 0x8b:  /* pause audio */
		{
			if ( cd.input_pos < 7 )
				return;

			matsucd_cdda_pause( machine, (cd.input[1] == 0) ? 1 : 0 );
			memset( cd.output, 0, 7 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		case 0xa3:  /* front panel */
		{
			if ( cd.input_pos < 7 )
				return;

			/* TODO: ??? */

			memset( cd.output, 0, 7 );
			matsucd_complete_cmd( machine, 0 );
		}
		break;

		default:
			logerror( "MATSUCD: Unknown/inimplemented command %08x\n", cmd );
		break;
	}
}
