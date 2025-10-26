// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "pc8801_30.h"

#include "coreutil.h"
#include "multibyte.h"

#define LOG_CMD            (1U << 1)
#define LOG_CDDA           (1U << 2)

#define VERBOSE (LOG_GENERAL | LOG_CMD)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCMD(...)         LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCDDA(...)        LOGMASKED(LOG_CDDA, __VA_ARGS__)

DEFINE_DEVICE_TYPE(NSCSI_CDROM_PC8801_30, nscsi_cdrom_pc8801_30_device, "scsi_pc8801_30", "SCSI NEC PC8801-30 CD-ROM")

nscsi_cdrom_pc8801_30_device::nscsi_cdrom_pc8801_30_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	nscsi_cdrom_device(mconfig, NSCSI_CDROM_PC8801_30, tag, owner, clock)
{
}

void nscsi_cdrom_pc8801_30_device::device_reset()
{
	nscsi_cdrom_device::device_reset();
	m_cdda_status = PCE_CD_CDDA_OFF;
	cdda->stop_audio();
}


bool nscsi_cdrom_pc8801_30_device::scsi_command_done(uint8_t command, uint8_t length)
{
	switch (command)
	{
		case 0xd8:
		case 0xd9:
		case 0xda:
		case 0xdd:
		case 0xde:
			return length == 10;

	default:
		return nscsi_cdrom_device::scsi_command_done(command, length);
	}
}

void nscsi_cdrom_pc8801_30_device::nec_set_audio_start_position()
{
	uint32_t frame = 0;
	const uint8_t mode = scsi_cmdbuf[9] & 0xc0;

	LOGCMD("0xd8 SET AUDIO PLAYBACK START POSITION (NEC): mode %02x\n", mode);
	if (!image->exists())
	{
		return_no_cd();
		return;
	}

	const cdrom_file::toc &toc = image->get_toc();

	switch (mode)
	{
		case 0x00:
			popmessage("CD-DA set start mode 0x00");
			frame = (scsi_cmdbuf[3] << 16) | (scsi_cmdbuf[4] << 8) | scsi_cmdbuf[5];
			break;
		case 0x40:
		{
			const u8 m = bcd_2_dec(scsi_cmdbuf[2]);
			const u8 s = bcd_2_dec(scsi_cmdbuf[3]);
			const u8 f = bcd_2_dec(scsi_cmdbuf[4]);
			frame = f + 75 * (s + m * 60);


			const u32 pregap = toc.tracks[image->get_track(frame)].pregap;

			LOGCMD("MSF=%d %02d:%02d:%02d (pregap = %d)\n", frame, m, s, f, pregap);
			// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
			// default to 2 secs if that isn't provided
			// cfr. draculax in-game, fzone2 / fzone2j / ddragon2 intro etc.
			// TODO: is this a global issue and INDEX 01 with no explicit pregap is always 2 seconds in INDEX 00 like in the aforementioned examples?
			frame -= std::max(pregap, (u32)150);
			break;
		}
		case 0x80:
		{
			const u8 track_number = bcd_2_dec(scsi_cmdbuf[2]);
			const u32 pregap = toc.tracks[image->get_track(track_number - 1)].pregap;
			LOGCMD("TRACK=%d (pregap = %d)\n", track_number, pregap);
			frame = toc.tracks[ track_number - 1 ].logframeofs;
			// Not right for emeraldd, breaks intro lip sync
			//frame -= std::max(pregap, (u32)150);
			break;
		}
		default:
			popmessage("CD-DA set start mode 0xc0");
			//assert(nullptr == nec_set_audio_start_position);
			break;
	}

	m_current_frame = frame;

	m_cdda_status = PCE_CD_CDDA_PAUSED;

	const u8 play_mode = scsi_cmdbuf[1] & 0x03;
	LOGCMD("Play mode = %d\n", play_mode);
	if (play_mode)
	{
		// Required by audio CD player
		// (will keep skipping tracks over and over, never sends an audio end command)
		m_cdda_status = PCE_CD_CDDA_PLAYING;
		m_end_frame = m_last_frame;

		LOGCDDA("Audio start (end of CD) current %d end %d\n", m_current_frame, m_end_frame);

		cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
		m_cdda_play_mode = (play_mode & 0x02) ? 2 : 3; // mode 2 sets IRQ at end
		m_end_mark = (play_mode & 0x02) ? 1 : 0;
	}
	else
	{
		//m_cdda_status = PCE_CD_CDDA_PLAYING;
		m_end_frame = toc.tracks[ image->get_track(m_current_frame) ].logframeofs
					+ toc.tracks[ image->get_track(m_current_frame) ].logframes;

		LOGCDDA("Audio start (end of track) current %d end %d\n", m_current_frame, m_end_frame);
		// Several places definitely don't want this to start redbook,
		// it's done later with 0xd9 command.
		// - fzone2 / fzone2j
		// - draculax (stage 2' pre-boss)
		// - manhole (fires this during Sunsoft logo but expects playback on successive
		//            credit sequence instead)
		//if (m_end_frame > m_current_frame)
		//  m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);

		// These ones additionally wants a CDDA pause issued:
		// - audio CD player ("fade out" button trigger, otherwise will playback the
		//                    very next track at the end of the sequence)
		// - ppersia (picking up sword in stage 1, cancels then restarts redbook BGM)
		cdda->pause_audio(1);
		m_cdda_play_mode = 3;
		m_end_mark = 0;

		// snatcher requires that the irq is sent here
		// otherwise it will hang on title screen
		// set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
	}

	scsi_data_in(SBUF_MAIN, 0);
	scsi_status_complete(SS_GOOD);
	// Definitely not here, breaks jleagt94, iganin, macr2036 "press RUN button" prompt
	// (expects to fire an irq at the end of redbook playback thru end_mark = 2 for proper attract mode)
	//set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
}

void nscsi_cdrom_pc8801_30_device::nec_set_audio_stop_position()
{
	uint32_t frame = 0;
	const uint8_t mode = scsi_cmdbuf[9] & 0xc0;
	LOGCMD("0xd9 SET AUDIO PLAYBACK END POSITION (NEC): mode %02x\n", mode);

	if (!image->exists())
	{
		return_no_cd();
		return;
	}

	const cdrom_file::toc &toc = image->get_toc();

	switch (mode)
	{
		case 0x00:
			// tadaima
			popmessage("CD-DA set end mode 0x00");
			frame = (scsi_cmdbuf[3] << 16) | (scsi_cmdbuf[4] << 8) | scsi_cmdbuf[5];
			break;
		case 0x40:
		{
			const u8 m = bcd_2_dec(scsi_cmdbuf[2]);
			const u8 s = bcd_2_dec(scsi_cmdbuf[3]);
			const u8 f = bcd_2_dec(scsi_cmdbuf[4]);
			const u32 pregap = toc.tracks[image->get_track(frame)].pregap;

			frame = f + 75 * (s + m * 60);
			LOGCMD("MSF=%d %02d:%02d:%02d (pregap = %d)\n", frame, m, s, f, pregap);
			frame -= std::max(pregap, (u32)150);
			break;
		}
		case 0x80:
		{
			const u8 track_number = bcd_2_dec(scsi_cmdbuf[2]);
			const u32 pregap = toc.tracks[image->get_track(track_number - 1)].pregap;
			// NB: crazyhos uses this command with track = 1 on pre-title screen intro.
			// It's not supposed to playback anything according to real HW refs.
			frame = toc.tracks[ track_number - 1 ].logframeofs;

			LOGCMD("TRACK=%d (raw %02x pregap = %d frame = %d)\n"
				, track_number
				, scsi_cmdbuf[2]
				, pregap
				, frame
			);
			//frame -= std::max(pregap, (u32)150);
			break;
		}
		default:
			popmessage("CD-DA set end mode 0xc0");
			//assert(nullptr == nec_set_audio_start_position);
			break;
	}

	m_end_frame = frame;
	m_cdda_play_mode = scsi_cmdbuf[1] & 0x03;
	LOGCMD("Play mode = %d\n", m_cdda_play_mode);

	if (m_cdda_play_mode)
	{
		if (m_cdda_status == PCE_CD_CDDA_PAUSED)
		{
			LOGCDDA("Audio unpause\n");
			cdda->pause_audio(0);
		}

		LOGCDDA("Audio end current %d end %d\n", m_current_frame, m_end_frame);
		//printf("%08x %08x\n",m_current_frame,m_end_frame - m_current_frame);
		cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
		m_end_mark = 1;
		m_cdda_status = PCE_CD_CDDA_PLAYING;
	}
	else
	{
		LOGCDDA("Audio stop\n");
		m_cdda_status = PCE_CD_CDDA_OFF;
		cdda->stop_audio();
		m_end_frame = m_last_frame;
		m_end_mark = 0;
//      assert(nullptr == nec_set_audio_stop_position);
	}

	scsi_data_in(SBUF_MAIN, 0);
	scsi_status_complete(SS_GOOD);
}


void nscsi_cdrom_pc8801_30_device::nec_pause()
{
	if (!image->exists())
	{
		return_no_cd();
		return;
	}

	LOGCMD("0xda PAUSE (NEC)\n");

	/* If there was no cdda playing, throw an error */
	if (m_cdda_status == PCE_CD_CDDA_OFF)
	{
		// TODO: sense key/ASC/ASCQ
		LOG("Issued SCSI_CHECK_CONDITION in 0xda!\n");
		scsi_status_complete(SS_CHECK_CONDITION);
		return;
	}

	m_cdda_status = PCE_CD_CDDA_PAUSED;
	m_current_frame = cdda->get_audio_lba();
	LOGCDDA("Audio pause on %d LBA\n", m_current_frame);
	cdda->pause_audio(1);

	scsi_data_in(SBUF_MAIN, 0);
	scsi_status_complete(SS_GOOD);
}

void nscsi_cdrom_pc8801_30_device::nec_get_subq()
{
	/* WP - I do not have access to chds with subchannel information yet, so I'm faking something here */
	uint32_t msf_abs, msf_rel, track, frame;
	//LOGCMD("0xdd READ SUBCHANNEL Q (NEC) %d\n", m_cdda_status);

	if (!image->exists())
	{
		return_no_cd();
		return;
	}

	frame = m_current_frame;

	switch (m_cdda_status)
	{
		case PCE_CD_CDDA_PAUSED:
			scsi_cmdbuf[0] = 2;
			frame = cdda->get_audio_lba();
			break;
		case PCE_CD_CDDA_PLAYING:
			scsi_cmdbuf[0] = 0;
			frame = cdda->get_audio_lba();
			break;
		default:
			scsi_cmdbuf[0] = 3;
			break;
	}

	msf_abs = cdrom_file::lba_to_msf_alt(frame);
	track = image->get_track(frame);
	msf_rel = cdrom_file::lba_to_msf_alt(frame - image->get_track_start(track));

	scsi_cmdbuf[1] = 0x01 | ((image->get_track_type(image->get_track(track+1)) == cdrom_file::CD_TRACK_AUDIO) ? 0x00 : 0x40);
	// track
	scsi_cmdbuf[2] = dec_2_bcd(track+1);
	// index
	scsi_cmdbuf[3] = 1;
	// MSF (relative)
	scsi_cmdbuf[4] = dec_2_bcd((msf_rel >> 16) & 0xFF);
	scsi_cmdbuf[5] = dec_2_bcd((msf_rel >> 8) & 0xFF);
	scsi_cmdbuf[6] = dec_2_bcd(msf_rel & 0xFF);
	// MSF (absolute)
	scsi_cmdbuf[7] = dec_2_bcd((msf_abs >> 16) & 0xFF);
	scsi_cmdbuf[8] = dec_2_bcd((msf_abs >> 8) & 0xFF);
	scsi_cmdbuf[9] = dec_2_bcd(msf_abs & 0xFF);
	//if(LIVE_SUBQ_VIEW)
	//{
	//	const std::vector<std::string> status_types = {"standby", "play", "pause"};
	//	popmessage("SUBQ - status %s type %02x|track %d index %d| MSF rel %06x MSF abs %06x\n"
	//		, status_types[m_cdda_status]
	//		, m_data_buffer[1]
	//		, track + 1
	//		, 1
	//		, msf_rel
	//		, msf_abs
	//	);
	//}

	scsi_data_in(SBUF_MAIN, 10);
	scsi_status_complete(SS_GOOD);
}


void nscsi_cdrom_pc8801_30_device::nec_get_dir_info()
{
	uint32_t frame, msf, track = 0;
	LOGCMD("0xde GET DIR INFO (NEC)\n");

	if (!image->exists())
	{
		return_no_cd();
		return;
	}

	const cdrom_file::toc &toc = image->get_toc();

	// NOTE: PC8801-30 CD player wants 4 bytes back vs. PC Engine
	switch(scsi_cmdbuf[1])
	{
		case 0x00:
			scsi_cmdbuf[0] = dec_2_bcd(1);
			scsi_cmdbuf[1] = dec_2_bcd(toc.numtrks);
			scsi_cmdbuf[2] = 0;
			scsi_cmdbuf[3] = 0;
			LOGCMD("Get first and last track numbers => 1-%02x\n", scsi_cmdbuf[1]);

			scsi_data_in(SBUF_MAIN, 4);
			scsi_status_complete(SS_GOOD);
			break;
		case 0x01:
		{
			frame = toc.tracks[toc.numtrks-1].logframeofs;
			frame += toc.tracks[toc.numtrks-1].frames;
			msf = to_msf(frame);
			LOGCMD("Get total disk size in MSF format => %06x\n", msf);

			//scsi_cmdbuf[0] = image->get_adr_control(image->get_last_track());
			scsi_cmdbuf[0] = dec_2_bcd(BIT(msf, 16, 8)); // minutes
			scsi_cmdbuf[1] = dec_2_bcd(BIT(msf, 8, 8));  // seconds
			scsi_cmdbuf[2] = dec_2_bcd(BIT(msf, 0, 8));  // frames
			scsi_cmdbuf[3] = 0;

			scsi_data_in(SBUF_MAIN, 4);
			scsi_status_complete(SS_GOOD);
			break;
		}
		case 0x02:
		{
			uint32_t frame;
			uint8_t track_type;
			if (scsi_cmdbuf[2] == 0xaa)
			{
				frame = toc.tracks[toc.numtrks-1].logframeofs;
				frame += toc.tracks[toc.numtrks-1].frames;
				LOGCMD("Get lead-out => %06x\n", frame);
				// TODO: correct?
				track_type = 0x04;
			}
			else
			{
				track = std::max(bcd_2_dec(scsi_cmdbuf[2]), 1U);
				frame = toc.tracks[track-1].logframeofs;

				track_type = toc.tracks[track-1].trktype == cdrom_file::CD_TRACK_AUDIO ? 0x00 : 0x04;
				LOGCMD("Get track info track = %d, frame = %d %s\n", track, frame, track_type ? "data" : "audio");
			}

			msf = to_msf(frame + 150);

			scsi_cmdbuf[0] = dec_2_bcd(BIT(msf, 16, 8)); // minutes
			scsi_cmdbuf[1] = dec_2_bcd(BIT(msf, 8, 8));  // seconds
			scsi_cmdbuf[2] = dec_2_bcd(BIT(msf, 0, 8));  // frames
			scsi_cmdbuf[3] = track_type;

			scsi_data_in(SBUF_MAIN, 4);
			scsi_status_complete(SS_GOOD);

			break;
		}
		default:
			popmessage("nec_get_dir_info: unknown subcommand %02x", scsi_cmdbuf[1]);
			break;
	}
}

void nscsi_cdrom_pc8801_30_device::scsi_command()
{
	switch (scsi_cmdbuf[0])
	{
		case SC_MODE_SELECT_6:
			LOG("command MODE SELECT 6 length %d\n", scsi_cmdbuf[4]);

			// accept mode select parameter data
			// wants 11 bytes compared to regular cd.h
			scsi_cmdbuf[4] += 1;
			nscsi_cdrom_device::scsi_command();
			break;

		case 0xd8: nec_set_audio_start_position(); break;
		case 0xd9: nec_set_audio_stop_position();  break;
		case 0xda: nec_pause();                    break;
		case 0xdd: nec_get_subq();                 break;
		case 0xde: nec_get_dir_info();             break;

		default:
			if (scsi_cmdbuf[0] != 0)
				LOGCMD("Not yet emulated command %02x\n", scsi_cmdbuf[0]);
			// TODO: purge commands that don't exist on this implementation
			nscsi_cdrom_device::scsi_command();
			break;
	}
}

