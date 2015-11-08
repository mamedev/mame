// license:BSD-3-Clause
// copyright-holders:smf
#include "t10mmc.h"

static int to_msf(int frame)
{
	int m = frame / (75 * 60);
	int s = (frame / 75) % 60;
	int f = frame % 75;

	return (m << 16) | (s << 8) | f;
}

void t10mmc::t10_start(device_t &device)
{
	m_device = &device;
	t10spc::t10_start(device);

	device.save_item(NAME(m_lba));
	device.save_item(NAME(m_blocks));
	device.save_item(NAME(m_last_lba));
	device.save_item(NAME(m_num_subblocks));
	device.save_item(NAME(m_cur_subblock));
	device.save_item(NAME(m_audio_sense));
}

void t10mmc::t10_reset()
{
	t10spc::t10_reset();

	SetDevice( m_image->get_cdrom_file() );
	if( !m_cdrom )
	{
		m_device->logerror( "T10MMC %s: no CD found!\n", m_image->tag() );
	}

	m_lba = 0;
	m_blocks = 0;
	m_last_lba = 0;
	m_sector_bytes = 2048;
	m_num_subblocks = 1;
	m_cur_subblock = 0;
	m_audio_sense = 0;
}

// scsicd_exec_command

void t10mmc::abort_audio()
{
	if (m_cdda->audio_active())
	{
		m_cdda->stop_audio();
		m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_STOPPED_DUE_TO_ERROR;
	}
}

t10mmc::toc_format_t t10mmc::toc_format()
{
	int mmc_format = command[2] & 0xf;
	if (mmc_format != 0)
	{
		return (toc_format_t) mmc_format;
	}

	/// SFF8020 legacy format field (see T10/1836-D Revision 2g page 643)
	return (toc_format_t) ((command[9] >> 6) & 3);
}

int t10mmc::toc_tracks()
{
	int start_track = command[6];
	int end_track = cdrom_get_last_track(m_cdrom);

	if (start_track == 0)
	{
		return end_track + 1;
	}
	else if (start_track <= end_track)
	{
		return ( end_track - start_track ) + 2;
	}
	else if (start_track <= 0xaa)
	{
		return 1;
	}

	return 0;
}

//
// Execute a SCSI command.

void t10mmc::ExecCommand()
{
	int trk;

	// keep updating the sense data while playing audio.
	if (command[0] == T10SPC_CMD_REQUEST_SENSE && m_audio_sense != SCSI_SENSE_ASC_ASCQ_NO_SENSE && m_sense_key == SCSI_SENSE_KEY_NO_SENSE && m_sense_asc == 0 && m_sense_ascq == 0)
	{
		if (m_audio_sense == SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS && !m_cdda->audio_active())
		{
			m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED;
		}

		set_sense(SCSI_SENSE_KEY_NO_SENSE, (sense_asc_ascq_t) m_audio_sense);

		if (m_audio_sense != SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS)
		{
			m_audio_sense = SCSI_SENSE_ASC_ASCQ_NO_SENSE;
		}
	}

	switch ( command[0] )
	{
	case T10SPC_CMD_INQUIRY:
		m_device->logerror("T10MMC: INQUIRY\n");
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		if (m_transfer_length > 36)
			m_transfer_length = 36;
		break;

	case T10SPC_CMD_MODE_SELECT_6:
		m_device->logerror("T10MMC: MODE SELECT(6) length %x control %x\n", command[4], command[5]);
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case T10SPC_CMD_MODE_SENSE_6:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT8( &command[ 4 ] );
		break;

	case T10SPC_CMD_START_STOP_UNIT:
		abort_audio();
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SPC_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SBC_CMD_READ_CAPACITY:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 8;
		break;

	case T10SBC_CMD_READ_10:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = SCSILengthFromUINT16( &command[7] );

		m_device->logerror("T10MMC: READ(10) at LBA %x for %d blocks (%d bytes)\n", m_lba, m_blocks, m_blocks * m_sector_bytes);

		if (m_num_subblocks > 1)
		{
			m_cur_subblock = m_lba % m_num_subblocks;
			m_lba /= m_num_subblocks;
		}
		else
		{
			m_cur_subblock = 0;
		}

		abort_audio();

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10MMC_CMD_READ_SUB_CHANNEL:
		//m_device->logerror("T10MMC: READ SUB-CHANNEL type %d\n", command[3]);
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT16( &command[ 7 ] );
		break;

	case T10MMC_CMD_READ_TOC_PMA_ATIP:
	{
		int length;

		switch (toc_format())
		{
		case TOC_FORMAT_TRACKS:
			length = 4 + (8 * toc_tracks());
			break;

		case TOC_FORMAT_SESSIONS:
			length = 4 + (8 * 1);
			break;

		default:
			m_device->logerror("T10MMC: Unhandled READ TOC format %d\n", toc_format());
			length = 0;
			break;
		}

		int allocation_length = SCSILengthFromUINT16( &command[ 7 ] );

		if( length > allocation_length )
		{
			length = allocation_length;
		}

		abort_audio();

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = length;
		break;
	}
	case T10MMC_CMD_PLAY_AUDIO_10:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = SCSILengthFromUINT16( &command[7] );

		// special cases: lba of 0 means MSF of 00:02:00
		if (m_lba == 0)
		{
			m_lba = 150;
		}
		else if (m_lba == 0xffffffff)
		{
			m_device->logerror("T10MMC: play audio from current not implemented!\n");
		}

		m_device->logerror("T10MMC: PLAY AUDIO(10) at LBA %x for %x blocks\n", m_lba, m_blocks);

		trk = cdrom_get_track(m_cdrom, m_lba);

		if (cdrom_get_track_type(m_cdrom, trk) == CD_TRACK_AUDIO)
		{
			m_cdda->start_audio(m_lba, m_blocks);
			m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS;
		}
		else
		{
			m_device->logerror("T10MMC: track is NOT audio!\n");
			set_sense(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_SENSE_ASC_ASCQ_ILLEGAL_MODE_FOR_THIS_TRACK);
		}

		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10MMC_CMD_PLAY_AUDIO_TRACK_INDEX:
		// be careful: tracks here are zero-based, but the SCSI command
		// uses the real CD track number which is 1-based!
		m_device->logerror("T10MMC: PLAY AUDIO T/I: strk %d idx %d etrk %d idx %d frames %d\n", command[4], command[5], command[7], command[8], m_blocks);
		m_lba = cdrom_get_track_start(m_cdrom, command[4]-1);
		m_blocks = cdrom_get_track_start(m_cdrom, command[7]-1) - m_lba;
		if (command[4] > command[7])
		{
			m_blocks = 0;
		}

		if (command[4] == command[7])
		{
			m_blocks = cdrom_get_track_start(m_cdrom, command[4]) - m_lba;
		}

		trk = cdrom_get_track(m_cdrom, m_lba);

		if (cdrom_get_track_type(m_cdrom, trk) == CD_TRACK_AUDIO)
		{
			m_cdda->start_audio(m_lba, m_blocks);
			m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS;
		}
		else
		{
			m_device->logerror("T10MMC: track is NOT audio!\n");
			set_sense(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_SENSE_ASC_ASCQ_ILLEGAL_MODE_FOR_THIS_TRACK);
		}

		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10MMC_CMD_PAUSE_RESUME:
		if (m_cdrom)
		{
			m_cdda->pause_audio((command[8] & 0x01) ^ 0x01);
		}

		m_device->logerror("T10MMC: PAUSE/RESUME: %s\n", command[8]&1 ? "RESUME" : "PAUSE");
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10MMC_CMD_STOP_PLAY_SCAN:
		abort_audio();

		m_device->logerror("T10MMC: STOP_PLAY_SCAN\n");
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SPC_CMD_MODE_SELECT_10:
		m_device->logerror("T10MMC: MODE SELECT length %x control %x\n", command[7]<<8 | command[8], command[1]);
		m_phase = SCSI_PHASE_DATAOUT;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT16( &command[ 7 ] );
		break;

	case T10SPC_CMD_MODE_SENSE_10:
		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = SCSILengthFromUINT16( &command[ 7 ] );
		break;

	case T10MMC_CMD_PLAY_AUDIO_12:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = command[6]<<24 | command[7]<<16 | command[8]<<8 | command[9];

		// special cases: lba of 0 means MSF of 00:02:00
		if (m_lba == 0)
		{
			m_lba = 150;
		}
		else if (m_lba == 0xffffffff)
		{
			m_device->logerror("T10MMC: play audio from current not implemented!\n");
		}

		m_device->logerror("T10MMC: PLAY AUDIO(12) at LBA %x for %x blocks\n", m_lba, m_blocks);

		trk = cdrom_get_track(m_cdrom, m_lba);

		if (cdrom_get_track_type(m_cdrom, trk) == CD_TRACK_AUDIO)
		{
			m_cdda->start_audio(m_lba, m_blocks);
			m_audio_sense = SCSI_SENSE_ASC_ASCQ_AUDIO_PLAY_OPERATION_IN_PROGRESS;
		}
		else
		{
			m_device->logerror("T10MMC: track is NOT audio!\n");
			set_sense(SCSI_SENSE_KEY_ILLEGAL_REQUEST, SCSI_SENSE_ASC_ASCQ_ILLEGAL_MODE_FOR_THIS_TRACK);
		}

		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	case T10SBC_CMD_READ_12:
		m_lba = command[2]<<24 | command[3]<<16 | command[4]<<8 | command[5];
		m_blocks = command[7]<<16 | command[8]<<8 | command[9];

		m_device->logerror("T10MMC: READ(12) at LBA %x for %x blocks (%x bytes)\n", m_lba, m_blocks, m_blocks * m_sector_bytes);

		if (m_num_subblocks > 1)
		{
			m_cur_subblock = m_lba % m_num_subblocks;
			m_lba /= m_num_subblocks;
		}
		else
		{
			m_cur_subblock = 0;
		}

		abort_audio();

		m_phase = SCSI_PHASE_DATAIN;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = m_blocks * m_sector_bytes;
		break;

	case T10MMC_CMD_SET_CD_SPEED:
		m_device->logerror("T10MMC: SET CD SPEED to %d kbytes/sec.\n", command[2]<<8 | command[3]);
		m_phase = SCSI_PHASE_STATUS;
		m_status_code = SCSI_STATUS_CODE_GOOD;
		m_transfer_length = 0;
		break;

	default:
		t10spc::ExecCommand();
	}
}

// scsicd_read_data
//
// Read data from the device resulting from the execution of a command

void t10mmc::ReadData( UINT8 *data, int dataLength )
{
	UINT32 temp;
	UINT8 tmp_buffer[2048];

	switch ( command[0] )
	{
	case T10SPC_CMD_INQUIRY:
		data[0] = 0x05; // device is present, device is CD/DVD (MMC-3)
		data[1] = 0x80; // media is removable
		data[2] = 0x05; // device complies with SPC-3 standard
		data[3] = 0x02; // response data format = SPC-3 standard
		data[4] = 0x1f;
		data[5] = 0;
		data[6] = 0;
		data[7] = 0;
		memset(&data[8], ' ', 28);
		memcpy(&data[8], "MAME", 4);
		memcpy(&data[16], "Virtual CDROM", 13);
		memcpy(&data[32], "1.0", 3);
		break;

	case T10SBC_CMD_READ_CAPACITY:
		m_device->logerror("T10MMC: READ CAPACITY\n");

		temp = cdrom_get_track_start(m_cdrom, 0xaa);
		temp--; // return the last used block on the disc

		data[0] = (temp>>24) & 0xff;
		data[1] = (temp>>16) & 0xff;
		data[2] = (temp>>8) & 0xff;
		data[3] = (temp & 0xff);
		data[4] = 0;
		data[5] = 0;
		data[6] = (m_sector_bytes>>8)&0xff;
		data[7] = (m_sector_bytes & 0xff);
		break;

	case T10SBC_CMD_READ_10:
	case T10SBC_CMD_READ_12:
		m_device->logerror("T10MMC: read %x dataLength, \n", dataLength);
		if ((m_cdrom) && (m_blocks))
		{
			while (dataLength > 0)
			{
				if (!cdrom_read_data(m_cdrom, m_lba, tmp_buffer, CD_TRACK_MODE1))
				{
					m_device->logerror("T10MMC: CD read error!\n");
				}

				m_device->logerror("True LBA: %d, buffer half: %d\n", m_lba, m_cur_subblock * m_sector_bytes);

				memcpy(data, &tmp_buffer[m_cur_subblock * m_sector_bytes], m_sector_bytes);

				m_cur_subblock++;
				if (m_cur_subblock >= m_num_subblocks)
				{
					m_cur_subblock = 0;

					m_lba++;
					m_blocks--;
				}

				m_last_lba = m_lba;
				dataLength -= m_sector_bytes;
				data += m_sector_bytes;
			}
		}
		break;

	case T10MMC_CMD_READ_SUB_CHANNEL:
		switch (command[3])
		{
			case 1: // return current position
			{
				if (!m_cdrom)
				{
					return;
				}

				m_device->logerror("T10MMC: READ SUB-CHANNEL Time = %x, SUBQ = %x\n", command[1], command[2]);

				bool msf = (command[1] & 0x2) != 0;

				data[0]= 0x00;

				int audio_active = m_cdda->audio_active();
				if (audio_active)
				{
					// if audio is playing, get the latest LBA from the CDROM layer
					m_last_lba = m_cdda->get_audio_lba();
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
					m_last_lba = 0;
					if (m_cdda->audio_ended())
					{
						data[1] = 0x13; // ended successfully
					}
					else
					{
//                          data[1] = 0x14;    // stopped due to error
						data[1] = 0x15; // No current audio status to return
					}
				}

				if (command[2] & 0x40)
				{
					data[2] = 0;
					data[3] = 12;       // data length
					data[4] = 0x01; // sub-channel format code
					data[5] = 0x10 | (audio_active ? 0 : 4);
					data[6] = cdrom_get_track(m_cdrom, m_last_lba) + 1; // track
					data[7] = 0;    // index

					UINT32 frame = m_last_lba;

					if (msf)
					{
						frame = to_msf(frame);
					}

					data[8] = (frame>>24)&0xff;
					data[9] = (frame>>16)&0xff;
					data[10] = (frame>>8)&0xff;
					data[11] = frame&0xff;

					frame -= cdrom_get_track_start(m_cdrom, data[6] - 1);

					if (msf)
					{
						frame = to_msf(frame);
					}

					data[12] = (frame>>24)&0xff;
					data[13] = (frame>>16)&0xff;
					data[14] = (frame>>8)&0xff;
					data[15] = frame&0xff;
				}
				else
				{
					data[2] = 0;
					data[3] = 0;
				}
				break;
			}
			default:
				m_device->logerror("T10MMC: Unknown subchannel type %d requested\n", command[3]);
		}
		break;

	case T10MMC_CMD_READ_TOC_PMA_ATIP:
		/*
		    Track numbers are problematic here: 0 = lead-in, 0xaa = lead-out.
		    That makes sense in terms of how real-world CDs are referred to, but
		    our internal routines for tracks use "0" as track 1.  That probably
		    should be fixed...
		*/
		{
			bool msf = (command[1] & 0x2) != 0;

			m_device->logerror("T10MMC: READ TOC, format = %d time=%d\n", toc_format(),msf);
			switch (toc_format())
			{
			case TOC_FORMAT_TRACKS:
				{
					int tracks = toc_tracks();
					int len = 2 + (tracks * 8);

					// the returned TOC DATA LENGTH must be the full amount,
					// regardless of how much we're able to pass back due to in_len
					int dptr = 0;
					data[dptr++] = (len>>8) & 0xff;
					data[dptr++] = (len & 0xff);
					data[dptr++] = 1;
					data[dptr++] = cdrom_get_last_track(m_cdrom);

					int first_track = command[6];
					if (first_track == 0)
					{
						first_track = 1;
					}

					for (int i = 0; i < tracks; i++)
					{
						int track = first_track + i;
						int cdrom_track = track - 1;
						if( i == tracks - 1 )
						{
							track = 0xaa;
							cdrom_track = 0xaa;
						}

						if( dptr >= dataLength )
						{
							break;
						}

						data[dptr++] = 0;
						data[dptr++] = cdrom_get_adr_control(m_cdrom, cdrom_track);
						data[dptr++] = track;
						data[dptr++] = 0;

						UINT32 tstart = cdrom_get_track_start(m_cdrom, cdrom_track);

						if (msf)
						{
							tstart = to_msf(tstart+150);
						}

						data[dptr++] = (tstart>>24) & 0xff;
						data[dptr++] = (tstart>>16) & 0xff;
						data[dptr++] = (tstart>>8) & 0xff;
						data[dptr++] = (tstart & 0xff);
					}
				}
				break;

			case TOC_FORMAT_SESSIONS:
				{
					int len = 2 + (8 * 1);

					int dptr = 0;
					data[dptr++] = (len>>8) & 0xff;
					data[dptr++] = (len & 0xff);
					data[dptr++] = 1;
					data[dptr++] = 1;

					data[dptr++] = 0;
					data[dptr++] = cdrom_get_adr_control(m_cdrom, 0);
					data[dptr++] = 1;
					data[dptr++] = 0;

					UINT32 tstart = cdrom_get_track_start(m_cdrom, 0);

					if (msf)
					{
						tstart = to_msf(tstart+150);
					}

					data[dptr++] = (tstart>>24) & 0xff;
					data[dptr++] = (tstart>>16) & 0xff;
					data[dptr++] = (tstart>>8) & 0xff;
					data[dptr++] = (tstart & 0xff);
				}
				break;

			default:
				m_device->logerror("T10MMC: Unhandled READ TOC format %d\n", toc_format());
				break;
			}
		}
		break;

	case T10SPC_CMD_MODE_SENSE_6:
	case T10SPC_CMD_MODE_SENSE_10:
		m_device->logerror("T10MMC: MODE SENSE page code = %x, PC = %x\n", command[2] & 0x3f, (command[2]&0xc0)>>6);

		memset(data, 0, SCSILengthFromUINT16( &command[ 7 ] ));

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
			case 0x2a:  // Page capabilities
				data[0] = 0x2a;
				data[1] = 0x14; // page length
				data[2] = 0x00; data[3] = 0x00; // CD-R only
				data[4] = 0x01; // can play audio
				data[5] = 0;
				data[6] = 0;
				data[7] = 0;
				data[8] = 0x02; data[9] = 0xc0; // 4x speed
				data[10] = 0;
				data[11] = 2; // two volumen levels
				data[12] = 0x00; data[13] = 0x00; // buffer
				data[14] = 0x02; data[15] = 0xc0; // 4x read speed
				data[16] = 0;
				data[17] = 0;
				data[18] = 0;
				data[19] = 0;
				data[20] = 0;
				data[21] = 0;
				break;

			default:
				m_device->logerror("T10MMC: MODE SENSE unknown page %x\n", command[2] & 0x3f);
				break;
		}
		break;

	default:
		t10spc::ReadData( data, dataLength );
		break;
	}
}

// scsicd_write_data
//
// Write data to the CD-ROM device as part of the execution of a command

void t10mmc::WriteData( UINT8 *data, int dataLength )
{
	switch (command[ 0 ])
	{
	case T10SPC_CMD_MODE_SELECT_6:
	case T10SPC_CMD_MODE_SELECT_10:
		m_device->logerror("T10MMC: MODE SELECT page %x\n", data[0] & 0x3f);

		switch (data[0] & 0x3f)
		{
			case 0x0:   // vendor-specific
				// check for SGI extension to force 512-byte blocks
				if ((data[3] == 8) && (data[10] == 2))
				{
					m_device->logerror("T10MMC: Experimental SGI 512-byte block extension enabled\n");

					m_sector_bytes = 512;
					m_num_subblocks = 4;
				}
				else
				{
					m_device->logerror("T10MMC: Unknown vendor-specific page!\n");
				}
				break;

			case 0xe:   // audio page
				m_device->logerror("Ch 0 route: %x vol: %x\n", data[8], data[9]);
				m_device->logerror("Ch 1 route: %x vol: %x\n", data[10], data[11]);
				m_device->logerror("Ch 2 route: %x vol: %x\n", data[12], data[13]);
				m_device->logerror("Ch 3 route: %x vol: %x\n", data[14], data[15]);
				break;
		}
		break;

	default:
		t10spc::WriteData( data, dataLength );
		break;
}
}

void t10mmc::GetDevice( void **_cdrom )
{
	*(cdrom_file **)_cdrom = m_cdrom;
}

void t10mmc::SetDevice( void *_cdrom )
{
	m_cdrom = (cdrom_file *)_cdrom;
	m_cdda->set_cdrom(m_cdrom);
}
