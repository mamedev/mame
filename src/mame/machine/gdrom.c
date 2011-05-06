/***************************************************************************

 gdrom.c - Implementation of the Sega GD-ROM device

***************************************************************************/

#include "emu.h"
#include "machine/scsidev.h"
#include "cdrom.h"
#include "sound/cdda.h"
#include "imagedev/chd_cd.h"
#include "gdrom.h"

typedef struct
{
	UINT32 lba;
	UINT32 blocks;
	UINT32 last_lba;
	UINT32 bytes_per_sector;
	UINT32 num_subblocks;
	UINT32 cur_subblock;
	UINT32 play_err_flag;
	UINT32 read_type;	// for command 0x30 only
	UINT32 data_select;	// for command 0x30 only
	cdrom_file *cdrom;
	bool is_file;
} SCSIGd;

static const UINT8 GDROM_Cmd11_Reply[32] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0xB4, 0x19, 0x00, 0x00, 0x08, 0x53, 0x45, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x52, 0x65, 0x76, 0x20, 0x36, 0x2E, 0x34, 0x32, 0x39, 0x39, 0x30, 0x33, 0x31, 0x36
};

static void phys_frame_to_msf(int phys_frame, int *m, int *s, int *f)
{
	*m = phys_frame / (60*75);
	phys_frame -= (*m * 60 * 75);
	*s = phys_frame / 75;
	*f = phys_frame % 75;
}


// scsicd_exec_command
//
// Execute a SCSI command.

static int scsicd_exec_command( SCSIInstance *scsiInstance, UINT8 *statusCode )
{
	UINT8 *command;
	int commandLength;
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );

	cdrom_file *cdrom = our_this->cdrom;
	device_t *cdda;
	int trk;

	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch ( command[0] )
	{


		case 0x03: // REQUEST SENSE
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x11: // REQ_MODE
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			printf("REQ_MODE %02x %02x %02x %02x %02x %02x\n",
				command[0], command[1],
				command[2], command[3],
				command[4], command[5]);
//          if (SCSILengthFromUINT8( &command[ 4 ] ) < 32) return -1;
			return 32; //SCSILengthFromUINT8( &command[ 4 ] );

		case 0x12: // INQUIRY
			logerror("GDROM: REQUEST SENSE\n");
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x15: // MODE SELECT(6)
			logerror("GDROM: MODE SELECT(6) length %x control %x\n", command[4], command[5]);
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x1a: // MODE SENSE(6)
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT8( &command[ 4 ] );

		case 0x1b: // START STOP UNIT
			cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
			if (cdda != NULL)
			{
				cdda_stop_audio(cdda);
			}
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x1e: // PREVENT ALLOW MEDIUM REMOVAL
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x25: // READ CAPACITY
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return 8;

		case 0x28: // READ(10)

			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = SCSILengthFromUINT16( &command[7] );

			logerror("GDROM: READ(10) at LBA %x for %d blocks (%d bytes)\n", our_this->lba, our_this->blocks, our_this->blocks * our_this->bytes_per_sector);

			if (our_this->num_subblocks > 1)
			{
				our_this->cur_subblock = our_this->lba % our_this->num_subblocks;
				our_this->lba /= our_this->num_subblocks;
			}
			else
			{
				our_this->cur_subblock = 0;
			}

			cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
			if (cdda != NULL)
			{
				cdda_stop_audio(cdda);
			}

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return our_this->blocks * our_this->bytes_per_sector;

		case 0x30: // CD_READ
			if (command[1] & 1)
			{
				fatalerror("GDROM: MSF mode used for CD_READ, unsupported");
				return 0;
			}
			else
			{
				our_this->lba = (command[2]<<16 | command[3]<<8 | command[4]) - 150;
				our_this->blocks = command[8]<<16 | command[9]<<8 | command[10];

				our_this->read_type = (command[1] >> 1) & 7;
				our_this->data_select = (command[1]>>4) & 0xf;

				if (our_this->read_type != 2)	// mode 1
				{
					fatalerror("GDROM: Unhandled read_type %d", our_this->read_type);
				}

				if (our_this->data_select != 2)	// just sector data
				{
					fatalerror("GDROM: Unhandled data_select %d", our_this->data_select);
				}

				printf("GDROM: CD_READ at LBA %x for %d blocks (%d bytes, read type %d, data select %d)\n", our_this->lba, our_this->blocks, our_this->blocks * our_this->bytes_per_sector, our_this->read_type, our_this->data_select);

				if (our_this->num_subblocks > 1)
				{
					our_this->cur_subblock = our_this->lba % our_this->num_subblocks;
					our_this->lba /= our_this->num_subblocks;
				}
				else
				{
					our_this->cur_subblock = 0;
				}

				cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
				if (cdda != NULL)
				{
					cdda_stop_audio(cdda);
				}

				SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
				return our_this->blocks * our_this->bytes_per_sector;
			}

		case 0x42: // READ SUB-CHANNEL
//                      logerror("GDROM: READ SUB-CHANNEL type %d\n", command[3]);
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT16( &command[ 7 ] );

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

			cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
			if (cdda != NULL)
			{
				cdda_stop_audio(cdda);
			}

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return length;
		}
		case 0x45: // PLAY AUDIO(10)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = SCSILengthFromUINT16( &command[7] );

			// special cases: lba of 0 means MSF of 00:02:00
			if (our_this->lba == 0)
			{
				our_this->lba = 150;
			}
			else if (our_this->lba == 0xffffffff)
			{
				logerror("GDROM: play audio from current not implemented!\n");
			}

			logerror("GDROM: PLAY AUDIO(10) at LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			trk = cdrom_get_track(cdrom, our_this->lba);

			if (cdrom_get_track_type(cdrom, trk) == CD_TRACK_AUDIO)
			{
				our_this->play_err_flag = 0;
				cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
				if (cdda != NULL)
					cdda_start_audio(cdda, our_this->lba, our_this->blocks);
			}
			else
			{
				logerror("GDROM: track is NOT audio!\n");
				our_this->play_err_flag = 1;
			}

			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x48: // PLAY AUDIO TRACK/INDEX
			// be careful: tracks here are zero-based, but the SCSI command
			// uses the real CD track number which is 1-based!
			our_this->lba = cdrom_get_track_start(cdrom, command[4]-1);
			our_this->blocks = cdrom_get_track_start(cdrom, command[7]-1) - our_this->lba;
			if (command[4] > command[7])
			{
				our_this->blocks = 0;
			}

			if (command[4] == command[7])
			{
				our_this->blocks = cdrom_get_track_start(cdrom, command[4]) - our_this->lba;
			}

			if (our_this->blocks && cdrom)
			{
				cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
				if (cdda != NULL)
					cdda_start_audio(cdda, our_this->lba, our_this->blocks);
			}

			logerror("GDROM: PLAY AUDIO T/I: strk %d idx %d etrk %d idx %d frames %d\n", command[4], command[5], command[7], command[8], our_this->blocks);
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x4b: // PAUSE/RESUME
			if (cdrom)
			{
				cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
				if (cdda != NULL)
					cdda_pause_audio(cdda, (command[8] & 0x01) ^ 0x01);
			}

			logerror("GDROM: PAUSE/RESUME: %s\n", command[8]&1 ? "RESUME" : "PAUSE");
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0x55: // MODE SELECT(10)
			logerror("GDROM: MODE SELECT length %x control %x\n", command[7]<<8 | command[8], command[1]);
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAOUT );
			return SCSILengthFromUINT16( &command[ 7 ] );

		case 0x5a: // MODE SENSE(10)
			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return SCSILengthFromUINT16( &command[ 7 ] );

		case 0xa5: // PLAY AUDIO(12)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

			// special cases: lba of 0 means MSF of 00:02:00
			if (our_this->lba == 0)
			{
				our_this->lba = 150;
			}
			else if (our_this->lba == 0xffffffff)
			{
				logerror("GDROM: play audio from current not implemented!\n");
			}

			logerror("GDROM: PLAY AUDIO(12) at LBA %x for %x blocks\n", our_this->lba, our_this->blocks);

			trk = cdrom_get_track(cdrom, our_this->lba);

			if (cdrom_get_track_type(cdrom, trk) == CD_TRACK_AUDIO)
			{
				our_this->play_err_flag = 0;
				cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
				if (cdda != NULL)
					cdda_start_audio(cdda, our_this->lba, our_this->blocks);
			}
			else
			{
				logerror("GDROM: track is NOT audio!\n");
				our_this->play_err_flag = 1;
			}
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		case 0xa8: // READ(12)
			our_this->lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
			our_this->blocks = command[7]<<16 | command[8]<<8 | command[9];

			logerror("GDROM: READ(12) at LBA %x for %x blocks (%x bytes)\n", our_this->lba, our_this->blocks, our_this->blocks * our_this->bytes_per_sector);

			if (our_this->num_subblocks > 1)
			{
				our_this->cur_subblock = our_this->lba % our_this->num_subblocks;
				our_this->lba /= our_this->num_subblocks;
			}
			else
			{
				our_this->cur_subblock = 0;
			}

			cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
			if (cdda != NULL)
			{
				cdda_stop_audio(cdda);
			}

			SCSISetPhase( scsiInstance, SCSI_PHASE_DATAIN );
			return our_this->blocks * our_this->bytes_per_sector;

		case 0xbb: // SET CD SPEED
			logerror("GDROM: SET CD SPEED to %d kbytes/sec.\n", command[2]<<8 | command[3]);
			SCSISetPhase( scsiInstance, SCSI_PHASE_STATUS );
			return 0;

		default:
			return SCSIBase( &SCSIClassGDROM, SCSIOP_EXEC_COMMAND, scsiInstance, 0, NULL );
	}
}

// scsicd_read_data
//
// Read data from the device resulting from the execution of a command

static void scsicd_read_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );

	int i;
	UINT32 last_phys_frame;
	cdrom_file *cdrom = our_this->cdrom;
	UINT32 temp;
	UINT8 tmp_buffer[2048];
	device_t *cdda;

	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch ( command[0] )
	{
		case 0x03: // REQUEST SENSE
			logerror("GDROM: Reading REQUEST SENSE data\n");

			memset( data, 0, dataLength );

			data[0] = 0x71;	// deferred error

			cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
			if (cdda != NULL && cdda_audio_active(cdda))
			{
				data[12] = 0x00;
				data[13] = 0x11;	// AUDIO PLAY OPERATION IN PROGRESS
			}
			else if (our_this->play_err_flag)
			{
				our_this->play_err_flag = 0;
				data[12] = 0x64;	// ILLEGAL MODE FOR THIS TRACK
				data[13] = 0x00;
			}
			// (else 00/00 means no error to report)
			break;

		case 0x11: // REQ_MODE
			printf("REQ_MODE: dataLength %d\n", dataLength);
			memcpy(data, &GDROM_Cmd11_Reply[0], (dataLength >= 32) ? 32 : dataLength);
			break;

		case 0x12: // INQUIRY
			memset( data, 0, dataLength );
			data[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
			data[1] = 0x80; // media is removable
			data[2] = 0x05; // device complies with SPC-3 standard
			data[3] = 0x02; // response data format = SPC-3 standard
			// some Konami games freak out if this isn't "Sony", so we'll lie
			// this is the actual drive on my Nagano '98 board
			strcpy((char *)&data[8], "Sony");
			strcpy((char *)&data[16], "CDU-76S");
			strcpy((char *)&data[32], "1.0");
			break;

		case 0x25: // READ CAPACITY
			logerror("GDROM: READ CAPACITY\n");

			temp = cdrom_get_track_start(cdrom, 0xaa);
			temp--;	// return the last used block on the disc

			data[0] = (temp>>24) & 0xff;
			data[1] = (temp>>16) & 0xff;
			data[2] = (temp>>8) & 0xff;
			data[3] = (temp & 0xff);
			data[4] = 0;
			data[5] = 0;
			data[6] = (our_this->bytes_per_sector>>8)&0xff;
			data[7] = (our_this->bytes_per_sector & 0xff);
			break;

		case 0x28: // READ(10)
		case 0xa8: // READ(12)
			logerror("GDROM: read %x dataLength, \n", dataLength);
			if ((our_this->cdrom) && (our_this->blocks))
			{
				while (dataLength > 0)
				{
					if (!cdrom_read_data(our_this->cdrom, our_this->lba, tmp_buffer, CD_TRACK_MODE1))
					{
						logerror("GDROM: CD read error!\n");
					}

					logerror("True LBA: %d, buffer half: %d\n", our_this->lba, our_this->cur_subblock * our_this->bytes_per_sector);

					memcpy(data, &tmp_buffer[our_this->cur_subblock * our_this->bytes_per_sector], our_this->bytes_per_sector);

					our_this->cur_subblock++;
					if (our_this->cur_subblock >= our_this->num_subblocks)
					{
						our_this->cur_subblock = 0;

						our_this->lba++;
						our_this->blocks--;
					}

					our_this->last_lba = our_this->lba;
					dataLength -= our_this->bytes_per_sector;
					data += our_this->bytes_per_sector;
				}
			}
			break;

		case 0x30: // CD_READ
			logerror("GDROM: read %x dataLength, \n", dataLength);
			if ((our_this->cdrom) && (our_this->blocks))
			{
				while (dataLength > 0)
				{
					if (!cdrom_read_data(our_this->cdrom, our_this->lba, tmp_buffer, CD_TRACK_MODE1))
					{
						logerror("GDROM: CD read error!\n");
					}

					logerror("True LBA: %d, buffer half: %d\n", our_this->lba, our_this->cur_subblock * our_this->bytes_per_sector);

					memcpy(data, &tmp_buffer[our_this->cur_subblock * our_this->bytes_per_sector], our_this->bytes_per_sector);

					our_this->cur_subblock++;
					if (our_this->cur_subblock >= our_this->num_subblocks)
					{
						our_this->cur_subblock = 0;

						our_this->lba++;
						our_this->blocks--;
					}

					our_this->last_lba = our_this->lba;
					dataLength -= our_this->bytes_per_sector;
					data += our_this->bytes_per_sector;
				}
			}

		case 0x42: // READ SUB-CHANNEL
			switch (command[3])
			{
				case 1:	// return current position
				{
					int audio_active;
					int msf;

					if (!cdrom)
					{
						return;
					}

					logerror("GDROM: READ SUB-CHANNEL Time = %x, SUBQ = %x\n", command[1], command[2]);

					msf = command[1] & 0x2;

					cdda = cdda_from_cdrom(scsiInstance->machine(), cdrom);
					audio_active = cdda != NULL && cdda_audio_active(cdda);
					if (audio_active)
					{
						if (cdda_audio_paused(cdda))
						{
							data[1] = 0x12;		// audio is paused
						}
						else
						{
							data[1] = 0x11;		// audio in progress
						}
					}
					else
					{
						if (cdda != NULL && cdda_audio_ended(cdda))
						{
							data[1] = 0x13;	// ended successfully
						}
						else
						{
//                          data[1] = 0x14;    // stopped due to error
							data[1] = 0x15;	// No current audio status to return
						}
					}

					// if audio is playing, get the latest LBA from the CDROM layer
					if (audio_active)
					{
						our_this->last_lba = cdda_get_audio_lba(cdda);
					}
					else
					{
						our_this->last_lba = 0;
					}

					data[2] = 0;
					data[3] = 12;		// data length
					data[4] = 0x01;	// sub-channel format code
					data[5] = 0x10 | (audio_active ? 0 : 4);
					data[6] = cdrom_get_track(cdrom, our_this->last_lba) + 1;	// track
					data[7] = 0;	// index

					last_phys_frame = our_this->last_lba;

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
				case 0:		// normal
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

		case 0x1a: // MODE SENSE(6)
		case 0x5a: // MODE SENSE(10)
			logerror("GDROM: MODE SENSE page code = %x, PC = %x\n", command[2] & 0x3f, (command[2]&0xc0)>>6);

			switch (command[2] & 0x3f)
			{
				case 0xe:	// CD Audio control page
					data[0] = 0x8e;	// page E, parameter is savable
					data[1] = 0x0e;	// page length
					data[2] = 0x04;	// IMMED = 1, SOTC = 0
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

		default:
			SCSIBase( &SCSIClassGDROM, SCSIOP_READ_DATA, scsiInstance, dataLength, data );
			break;
	}
}

// scsicd_write_data
//
// Write data to the CD-ROM device as part of the execution of a command

static void scsicd_write_data( SCSIInstance *scsiInstance, UINT8 *data, int dataLength )
{
	UINT8 *command;
	int commandLength;
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );
	SCSIGetCommand( scsiInstance, &command, &commandLength );

	switch (command[ 0 ])
	{
		case 0x15: // MODE SELECT(6)
		case 0x55: // MODE SELECT(10)
			logerror("GDROM: MODE SELECT page %x\n", data[0] & 0x3f);

			switch (data[0] & 0x3f)
			{
				case 0x0:	// vendor-specific
					// check for SGI extension to force 512-byte blocks
					if ((data[3] == 8) && (data[10] == 2))
					{
						logerror("GDROM: Experimental SGI 512-byte block extension enabled\n");

						our_this->bytes_per_sector = 512;
						our_this->num_subblocks = 4;
					}
					else
					{
						logerror("GDROM: Unknown vendor-specific page!\n");
					}
					break;

				case 0xe:	// audio page
					logerror("Ch 0 route: %x vol: %x\n", data[8], data[9]);
					logerror("Ch 1 route: %x vol: %x\n", data[10], data[11]);
					logerror("Ch 2 route: %x vol: %x\n", data[12], data[13]);
					logerror("Ch 3 route: %x vol: %x\n", data[14], data[15]);
					break;
			}
			break;

		default:
			SCSIBase( &SCSIClassGDROM, SCSIOP_WRITE_DATA, scsiInstance, dataLength, data );
			break;
	}
}

static void scsicd_alloc_instance( SCSIInstance *scsiInstance, const char *diskregion )
{
	running_machine &machine = scsiInstance->machine();
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );

	our_this->lba = 0;
	our_this->blocks = 0;
	our_this->last_lba = 0;
	our_this->bytes_per_sector = 2048;
	our_this->num_subblocks = 1;
	our_this->cur_subblock = 0;
	our_this->play_err_flag = 0;

	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->lba );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->blocks );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->last_lba );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->bytes_per_sector );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->num_subblocks );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->cur_subblock );
	state_save_register_item( machine, "scsicd", diskregion, 0, our_this->play_err_flag );

	if (machine.device( diskregion )) {
		our_this->is_file = TRUE;
		our_this->cdrom = cd_get_cdrom_file( machine.device( diskregion ) );
	} else {
		our_this->is_file = FALSE;
		our_this->cdrom = cdrom_open(get_disk_handle( machine, diskregion ));
	}

	if (!our_this->cdrom)
	{
		logerror("GDROM: no CD found!\n");
	}
}

static void scsicd_delete_instance( SCSIInstance *scsiInstance )
{
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );
	if (!our_this->is_file) {
		if( our_this->cdrom )
		{
			cdrom_close( our_this->cdrom );
		}
	}
}

static void scsicd_get_device( SCSIInstance *scsiInstance, cdrom_file **cdrom )
{
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );
	*cdrom = our_this->cdrom;
}

static void scsicd_set_device( SCSIInstance *scsiInstance, cdrom_file *cdrom )
{
	SCSIGd *our_this = (SCSIGd *)SCSIThis( &SCSIClassGDROM, scsiInstance );
	our_this->cdrom = cdrom;
}

static int scsigd_dispatch(int operation, void *file, INT64 intparm, void *ptrparm)
{
	SCSIAllocInstanceParams *params;

	switch (operation)
	{
		case SCSIOP_EXEC_COMMAND:
			return scsicd_exec_command( (SCSIInstance *)file, (UINT8 *)ptrparm );

		case SCSIOP_READ_DATA:
			scsicd_read_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_WRITE_DATA:
			scsicd_write_data( (SCSIInstance *)file, (UINT8 *)ptrparm, intparm );
			return 0;

		case SCSIOP_ALLOC_INSTANCE:
			params = (SCSIAllocInstanceParams *)ptrparm;
			SCSIBase( &SCSIClassGDROM, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
			scsicd_alloc_instance( params->instance, params->diskregion );
			return 0;

		case SCSIOP_DELETE_INSTANCE:
			scsicd_delete_instance( (SCSIInstance *)file );
			break;

		case SCSIOP_GET_DEVICE:
			scsicd_get_device( (SCSIInstance *)file, (cdrom_file **)ptrparm );
			return 0;

		case SCSIOP_SET_DEVICE:
			scsicd_set_device( (SCSIInstance *)file, (cdrom_file *)ptrparm );
			return 0;
	}

	return SCSIBase( &SCSIClassGDROM, operation, (SCSIInstance *)file, intparm, (UINT8 *)ptrparm );
}

const SCSIClass SCSIClassGDROM =
{
	&SCSIClassDevice,
	scsigd_dispatch,
	sizeof( SCSIGd )
};
