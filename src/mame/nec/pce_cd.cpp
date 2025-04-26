// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
/**************************************************************************************************

PC Engine CD HW sub-portion:

TODO:
- Rewrite SCSI to honor actual nscsi_device;
- Remove legacy CD drive implementation and merge with PC-8801 and PC-FX
  \- cfr. src/devices/bus/pc8801/pc8801_31.cpp
- Split into slot options, particularly Arcade Card shouldn't really be tied together
  thru a Machine Configuration option;
- verify CD read timing for edge cases marked in pcecd.xml with [SCSI];
- ADPCM half events aren't honored (dbz, draculax), they causes hangs for no benefit if enabled.
  \- In dbz, they will effectively send an half irq, game enables full irq mask and xfer done,
     neither is sent back, more stuff requiring SCSI rewrite first?
- Implement Game Express slot option;
- BRAM is unsafe on prolonged use of pcecd.xml games, verify
  \- tend to corrupt itself when 8 saves are already in (madoum x4, draculax, gulliver, ...);
- Unsafe on debugger access, recheck once conversion to SCSI bus is done;
- Audio CD player pregap don't work properly
  \- overflows into next track for a bit with standard 2 secs discs;
- Audio CD player rewind/fast forward don't work properly
  \- never go past 1 minute mark, underflows;
- Fader feature is sketchy and unchecked against real HW;
- Implement proper check condition errors (non-SCSI compliant);

**************************************************************************************************/

#include "emu.h"
#include "coreutil.h"
#include "pce_cd.h"

#define LOG_CMD            (1U << 1)
#define LOG_CDDA           (1U << 2)
#define LOG_SCSI           (1U << 3)
#define LOG_FADER          (1U << 4)
#define LOG_IRQ            (1U << 5)
#define LOG_SCSIXFER       (1U << 6) // single byte transfers, verbose

#define VERBOSE (LOG_GENERAL | LOG_CMD | LOG_CDDA | LOG_FADER)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCMD(...)         LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCDDA(...)        LOGMASKED(LOG_CDDA, __VA_ARGS__)
#define LOGSCSI(...)        LOGMASKED(LOG_SCSI, __VA_ARGS__)
#define LOGFADER(...)       LOGMASKED(LOG_FADER, __VA_ARGS__)
#define LOGIRQ(...)         LOGMASKED(LOG_IRQ, __VA_ARGS__)
#define LOGSCSIXFER(...)    LOGMASKED(LOG_SCSIXFER, __VA_ARGS__)

// 0xdd subchannel read is special and very verbose when it happens, treat differently
#define LIVE_SUBQ_VIEW    0
#define LIVE_ADPCM_VIEW   0

static constexpr XTAL PCE_CD_CLOCK = XTAL(9'216'000);


// TODO: correct name, split into incremental HuCard slot devices
DEFINE_DEVICE_TYPE(PCE_CD, pce_cd_device, "pcecd", "PCE CD Add-on")

// registers 9, e and f are known to be write only
void pce_cd_device::regs_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(pce_cd_device::cdc_status_r), FUNC(pce_cd_device::cdc_status_w));
	map(0x01, 0x01).rw(FUNC(pce_cd_device::cdc_data_r), FUNC(pce_cd_device::cdc_data_w));
	map(0x02, 0x02).rw(FUNC(pce_cd_device::irq_mask_r), FUNC(pce_cd_device::irq_mask_w));
	map(0x03, 0x03).r(FUNC(pce_cd_device::irq_status_r));
	map(0x04, 0x04).rw(FUNC(pce_cd_device::cdc_reset_r), FUNC(pce_cd_device::cdc_reset_w));
	map(0x05, 0x06).r(FUNC(pce_cd_device::cdda_data_r));
	map(0x07, 0x07).rw(FUNC(pce_cd_device::bram_status_r), FUNC(pce_cd_device::bram_unlock_w));
	map(0x08, 0x08).rw(FUNC(pce_cd_device::cd_data_r), FUNC(pce_cd_device::adpcm_address_lo_w));
	map(0x09, 0x09).w(FUNC(pce_cd_device::adpcm_address_hi_w));
	map(0x0a, 0x0a).rw(FUNC(pce_cd_device::adpcm_data_r), FUNC(pce_cd_device::adpcm_data_w));
	map(0x0b, 0x0b).rw(FUNC(pce_cd_device::adpcm_dma_control_r), FUNC(pce_cd_device::adpcm_dma_control_w));
	map(0x0c, 0x0c).r(FUNC(pce_cd_device::adpcm_status_r));
	map(0x0d, 0x0d).rw(FUNC(pce_cd_device::adpcm_address_control_r), FUNC(pce_cd_device::adpcm_address_control_w));
	map(0x0e, 0x0e).w(FUNC(pce_cd_device::adpcm_playback_rate_w));
	map(0x0f, 0x0f).w(FUNC(pce_cd_device::fader_control_w));
}

pce_cd_device::pce_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PCE_CD, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this, 2)
	, m_space_config("io", ENDIANNESS_LITTLE, 8, 4, 0, address_map_constructor(FUNC(pce_cd_device::regs_map), this))
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_irq_cb(*this)
	, m_msm(*this, "msm5205")
	, m_cdda(*this, "cdda")
	, m_nvram(*this, "bram")
	, m_cdrom(*this, "cdrom")
{
}

device_memory_interface::space_config_vector pce_cd_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_IO, &m_space_config)
	};
}

void pce_cd_device::device_start()
{
	/* Initialize BRAM */
	m_bram = std::make_unique<uint8_t[]>(PCE_BRAM_SIZE * 2);
	memset(m_bram.get(), 0, PCE_BRAM_SIZE);
	memset(m_bram.get() + PCE_BRAM_SIZE, 0xff, PCE_BRAM_SIZE);
	m_bram_locked = 1;

	m_nvram->set_base(m_bram.get(), PCE_BRAM_SIZE);

	/* set up adpcm related things */
	m_adpcm_ram = make_unique_clear<uint8_t[]>(PCE_ADPCM_RAM_SIZE);
	m_adpcm_clock_divider = 1;

	/* Set up cd command buffer */
	m_command_buffer = make_unique_clear<uint8_t[]>(PCE_CD_COMMAND_BUFFER_SIZE);
	m_command_buffer_index = 0;

	m_data_buffer = make_unique_clear<uint8_t[]>(8192);
	m_data_buffer_size = 0;
	m_data_buffer_index = 0;

	m_subcode_buffer = std::make_unique<uint8_t[]>(96);

	m_data_timer = timer_alloc(FUNC(pce_cd_device::data_timer_callback), this);
	m_data_timer->adjust(attotime::never);
	m_adpcm_dma_timer = timer_alloc(FUNC(pce_cd_device::adpcm_dma_timer_callback), this);
	m_adpcm_dma_timer->adjust(attotime::never);

	m_cdda_fadeout_timer = timer_alloc(FUNC(pce_cd_device::cdda_fadeout_callback), this);
	m_cdda_fadeout_timer->adjust(attotime::never);
	m_cdda_fadein_timer = timer_alloc(FUNC(pce_cd_device::cdda_fadein_callback), this);
	m_cdda_fadein_timer->adjust(attotime::never);

	m_adpcm_fadeout_timer = timer_alloc(FUNC(pce_cd_device::adpcm_fadeout_callback), this);
	m_adpcm_fadeout_timer->adjust(attotime::never);
	m_adpcm_fadein_timer = timer_alloc(FUNC(pce_cd_device::adpcm_fadein_callback), this);
	m_adpcm_fadein_timer->adjust(attotime::never);

	m_ack_clear_timer = timer_alloc(FUNC(pce_cd_device::clear_ack), this);
	m_ack_clear_timer->adjust(attotime::never);

	// TODO: add proper restore for the cd data...
	save_pointer(NAME(m_bram), PCE_BRAM_SIZE * 2);
	save_pointer(NAME(m_adpcm_ram), PCE_ADPCM_RAM_SIZE);
	save_item(NAME(m_bram_locked));
	save_item(NAME(m_adpcm_read_ptr));
	save_item(NAME(m_adpcm_read_buf));
	save_item(NAME(m_adpcm_write_ptr));
	save_item(NAME(m_adpcm_write_buf));
	save_item(NAME(m_adpcm_length));
	save_item(NAME(m_adpcm_clock_divider));
	save_item(NAME(m_msm_start_addr));
	save_item(NAME(m_msm_end_addr));
	save_item(NAME(m_msm_half_addr));
	save_item(NAME(m_msm_nibble));
	save_item(NAME(m_msm_idle));
	save_item(NAME(m_msm_repeat));
	save_item(NAME(m_scsi_BSY));
	save_item(NAME(m_scsi_SEL));
	save_item(NAME(m_scsi_CD));
	save_item(NAME(m_scsi_IO));
	save_item(NAME(m_scsi_MSG));
	save_item(NAME(m_scsi_REQ));
	save_item(NAME(m_scsi_ACK));
	save_item(NAME(m_scsi_ATN));
	save_item(NAME(m_scsi_RST));
	save_item(NAME(m_scsi_last_RST));
	save_item(NAME(m_cd_motor_on));
	save_item(NAME(m_selected));
	save_pointer(NAME(m_command_buffer), PCE_CD_COMMAND_BUFFER_SIZE);
	save_item(NAME(m_command_buffer_index));
	save_item(NAME(m_status_sent));
	save_item(NAME(m_message_after_status));
	save_item(NAME(m_message_sent));
	save_pointer(NAME(m_data_buffer), 8192);
	save_item(NAME(m_data_buffer_size));
	save_item(NAME(m_data_buffer_index));
	save_item(NAME(m_data_transferred));
	save_item(NAME(m_current_frame));
	save_item(NAME(m_end_frame));
	save_item(NAME(m_last_frame));
	save_item(NAME(m_cdda_status));
	save_item(NAME(m_cdda_play_mode));
	save_pointer(NAME(m_subcode_buffer), 96);
	save_item(NAME(m_end_mark));
	save_item(NAME(m_cdda_volume));
	save_item(NAME(m_adpcm_volume));

	// internal regs
	save_item(NAME(m_reset_reg));
	save_item(NAME(m_irq_mask));
	save_item(NAME(m_irq_status));
	save_item(NAME(m_cdc_status));
	save_item(NAME(m_cdc_data));
	save_item(NAME(m_bram_status));
	save_item(NAME(m_adpcm_status));
	save_item(NAME(m_adpcm_latch_address));
	save_item(NAME(m_adpcm_control));
	save_item(NAME(m_fader_ctrl));
	save_item(NAME(m_adpcm_dma_reg));
}

void pce_cd_device::device_reset()
{
	m_adpcm_read_buf = 0;
	m_adpcm_write_buf = 0;

	// TODO: add CD-DA stop command here
	//m_cdda_status = PCE_CD_CDDA_OFF;
	//m_cdda->stop_audio();

	m_adpcm_status |= PCE_CD_ADPCM_STOP_FLAG;
	m_adpcm_status &= ~PCE_CD_ADPCM_PLAY_FLAG;
	//m_irq_status = (m_irq_status & ~0x0c) | (PCE_CD_SAMPLE_STOP_PLAY);
	m_msm_idle = 1;

	m_scsi_RST = 0;
	m_scsi_last_RST = 0;
	m_scsi_SEL = 0;
	m_scsi_BSY = 0;
	m_selected = 0;
	m_scsi_ATN = 0;
	m_end_mark = 0;
}


void pce_cd_device::late_setup()
{
	// at device start, the cdrom is not 'ready' yet, so we postpone this part of the initialization at machine_start in the driver
	if (m_cdrom->exists())
	{
		m_toc = &m_cdrom->get_toc();
		m_last_frame = m_cdrom->get_track_start(m_cdrom->get_last_track() - 1);
		m_last_frame += m_toc->tracks[m_cdrom->get_last_track() - 1].frames;
		m_end_frame = m_last_frame;
	}

	// MSM5205 might be initialized after PCE CD as well...
	m_msm->set_unscaled_clock((PCE_CD_CLOCK / 6) / m_adpcm_clock_divider);
}

void pce_cd_device::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	// 0xa0 looks a dev left-over ...
//  static const uint8_t init[8] = { 0x48, 0x55, 0x42, 0x4d, 0x00, 0xa0, 0x10, 0x80 };
	// ... 0x88 is the actual value that cdsys/scdsys init thru format.
	static const uint8_t init[8] = { 'H', 'U', 'B', 'M', 0x00, 0x88, 0x10, 0x80 };

	memset(data, 0x00, size);
	memcpy(data, init, sizeof(init));
}

// TODO: left and right speaker tags should be passed from the parent config, instead of using the hard-coded ones below!?!
void pce_cd_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, m_nvram).set_custom_handler(FUNC(pce_cd_device::nvram_init));

	CDROM(config, m_cdrom).set_interface("cdrom");

	MSM5205(config, m_msm, PCE_CD_CLOCK / 6);
	m_msm->vck_legacy_callback().set(FUNC(pce_cd_device::msm5205_int)); /* interrupt function */
	m_msm->set_prescaler_selector(msm5205_device::S48_4B);  /* 1/48 prescaler, 4bit data */
	m_msm->add_route(ALL_OUTPUTS, *this, 0.50, AUTO_ALLOC_INPUT, 0);
	m_msm->add_route(ALL_OUTPUTS, *this, 0.50, AUTO_ALLOC_INPUT, 1);

	CDDA(config, m_cdda);
	m_cdda->set_cdrom_tag(m_cdrom);
	m_cdda->audio_end_cb().set(FUNC(pce_cd_device::cdda_end_mark_cb));
	m_cdda->add_route(0, *this, 1.00, AUTO_ALLOC_INPUT, 0);
	m_cdda->add_route(1, *this, 1.00, AUTO_ALLOC_INPUT, 1);
}

void pce_cd_device::adpcm_stop(uint8_t irq_flag)
{
	m_adpcm_status |= PCE_CD_ADPCM_STOP_FLAG;
	m_adpcm_status &= ~PCE_CD_ADPCM_PLAY_FLAG;
	//m_irq_status = (m_irq_status & ~0x0c) | (PCE_CD_SAMPLE_STOP_PLAY);
	if (irq_flag)
		set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, ASSERT_LINE);
	m_adpcm_control &= ~0x60;
	m_msm_idle = 1;
}

void pce_cd_device::adpcm_play()
{
	m_adpcm_status &= ~PCE_CD_ADPCM_STOP_FLAG;
	m_adpcm_status |= PCE_CD_ADPCM_PLAY_FLAG;
	set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
	m_irq_status = (m_irq_status & ~0x0c);
	m_msm_idle = 0;
}


/* Callback for new data from the MSM5205.
  The PCE cd unit actually divides the clock signal supplied to
  the MSM5205. Currently we can only use static clocks for the
  MSM5205.
 */
void pce_cd_device::msm5205_int(int state)
{
	uint8_t msm_data;

	if (m_msm_idle)
		return;

	if (LIVE_ADPCM_VIEW)
	{
		popmessage("start %08x end %08x half %08x status %02x control %02x"
			, m_msm_start_addr
			, m_msm_end_addr
			, m_msm_half_addr
			, m_adpcm_status
			, m_adpcm_control
		);
	}

	/* Supply new ADPCM data */
	msm_data = (m_msm_nibble) ? (m_adpcm_ram[m_msm_start_addr] & 0x0f) : ((m_adpcm_ram[m_msm_start_addr] & 0xf0) >> 4);

	m_msm->data_w(msm_data);
	m_msm_nibble ^= 1;

	if (m_msm_nibble == 0)
	{
		m_msm_start_addr++;

		if (m_msm_start_addr == m_msm_half_addr)
		{
			//set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
			//set_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, ASSERT_LINE);
		}

		if (m_msm_start_addr > m_msm_end_addr)
		{
			//set_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
			//set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
			adpcm_stop(1);
			m_msm->reset_w(1);
		}
	}
}



#define SCSI_STATUS_OK          0x00
#define SCSI_CHECK_CONDITION    0x02

void pce_cd_device::reply_status_byte(uint8_t status)
{
	LOGSCSI("Setting CD in reply_status_byte\n");
	m_scsi_CD = m_scsi_IO = m_scsi_REQ = 1;
	m_scsi_MSG = 0;
	m_message_after_status = 1;
	m_status_sent = m_message_sent = 0;

	if (status == SCSI_STATUS_OK)
	{
		m_cdc_data = 0x00;
	}
	else
	{
		m_cdc_data = 0x01;
	}
}

/* 0x00 - TEST UNIT READY */
void pce_cd_device::test_unit_ready()
{
	LOGCMD("0x00 TEST UNIT READY: status send ");
	if (m_cdrom->exists())
	{
		LOGCMD("STATUS_OK\n");
		reply_status_byte(SCSI_STATUS_OK);
	}
	else
	{
		// TODO: sense key/ASC/ASCQ
		LOGCMD("CHECK_CONDITION\n");
		reply_status_byte(SCSI_CHECK_CONDITION);
	}
}

/* 0x08 - READ (6) */
void pce_cd_device::read_6()
{
	uint32_t frame = ((m_command_buffer[1] & 0x1f) << 16) | (m_command_buffer[2] << 8) | m_command_buffer[3];
	uint32_t frame_count = m_command_buffer[4];
	LOGCMD("0x08 READ(6): frame: %08x size: %08x\n", frame, frame_count);

	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	if (m_cdda_status != PCE_CD_CDDA_OFF)
	{
		m_cdda_status = PCE_CD_CDDA_OFF;
		m_cdda->stop_audio();
		m_end_mark = 0;
	}

	m_current_frame = frame;
	m_end_frame = frame + frame_count;

	if (frame_count == 0)
	{
		// starbrkr uses this (cannot reproduce)
		// Should supposedly bump to max size (frame_count = 256)
		popmessage("Read Sector frame count == 0");
		reply_status_byte(SCSI_STATUS_OK);
	}
	else
	{
		m_data_timer->adjust(attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND), 0, attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND));
	}

	// TODO: timing likely not right
	set_irq_line(PCE_CD_IRQ_TRANSFER_READY, ASSERT_LINE);
}

/* 0xD8 - SET AUDIO PLAYBACK START POSITION (NEC) */
void pce_cd_device::nec_set_audio_start_position()
{
	uint32_t frame = 0;
	const uint8_t mode = m_command_buffer[9] & 0xc0;
	LOGCMD("0xd8 SET AUDIO PLAYBACK START POSITION (NEC): mode %02x\n", mode);

	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	switch (mode)
	{
		case 0x00:
			popmessage("CD-DA set start mode 0x00");
			frame = (m_command_buffer[3] << 16) | (m_command_buffer[4] << 8) | m_command_buffer[5];
			break;
		case 0x40:
		{
			const u8 m = bcd_2_dec(m_command_buffer[2]);
			const u8 s = bcd_2_dec(m_command_buffer[3]);
			const u8 f = bcd_2_dec(m_command_buffer[4]);
			frame = f + 75 * (s + m * 60);

			const u32 pregap = m_toc->tracks[m_cdrom->get_track(frame)].pregap;

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
			const u8 track_number = bcd_2_dec(m_command_buffer[2]);
			const u32 pregap = m_toc->tracks[m_cdrom->get_track(track_number - 1)].pregap;
			LOGCMD("TRACK=%d (pregap = %d)\n", track_number, pregap);
			frame = m_toc->tracks[ track_number - 1 ].logframeofs;
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

	// old code for reference, seems unlikely that this puts status in standby (and breaks Snatcher at the title screen)
//  if (m_cdda_status == PCE_CD_CDDA_PAUSED)
//  {
//      m_cdda_status = PCE_CD_CDDA_OFF;
//      m_cdda->stop_audio();
//      m_end_frame = m_last_frame;
//      m_end_mark = 0;
//  }
//  else
	{
		const u8 play_mode = m_command_buffer[1] & 0x03;
		LOGCMD("Play mode = %d\n", play_mode);
		if (play_mode)
		{
			// Required by audio CD player
			// (will keep skipping tracks over and over, never sends an audio end command)
			m_cdda_status = PCE_CD_CDDA_PLAYING;
			m_end_frame = m_last_frame;

			LOGCDDA("Audio start (end of CD) current %d end %d\n", m_current_frame, m_end_frame);

			m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
			m_cdda_play_mode = (play_mode & 0x02) ? 2 : 3; // mode 2 sets IRQ at end
			m_end_mark = (play_mode & 0x02) ? 1 : 0;
		}
		else
		{
			//m_cdda_status = PCE_CD_CDDA_PLAYING;
			m_end_frame = m_toc->tracks[ m_cdrom->get_track(m_current_frame) ].logframeofs
						+ m_toc->tracks[ m_cdrom->get_track(m_current_frame) ].logframes;

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
			m_cdda->pause_audio(1);
			m_cdda_play_mode = 3;
			m_end_mark = 0;

			// snatcher requires that the irq is sent here
			// otherwise it will hang on title screen
			set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
		}
	}

	reply_status_byte(SCSI_STATUS_OK);
	// Definitely not here, breaks jleagt94, iganin, macr2036 "press RUN button" prompt
	// (expects to fire an irq at the end of redbook playback thru end_mark = 2 for proper attract mode)
	//set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
}

/* 0xD9 - SET AUDIO PLAYBACK END POSITION (NEC) */
void pce_cd_device::nec_set_audio_stop_position()
{
	uint32_t frame = 0;
	const uint8_t mode = m_command_buffer[9] & 0xc0;
	LOGCMD("0xd9 SET AUDIO PLAYBACK END POSITION (NEC): mode %02x\n", mode);

	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	switch (mode)
	{
		case 0x00:
			// tadaima
			popmessage("CD-DA set end mode 0x00");
			frame = (m_command_buffer[3] << 16) | (m_command_buffer[4] << 8) | m_command_buffer[5];
			break;
		case 0x40:
		{
			const u8 m = bcd_2_dec(m_command_buffer[2]);
			const u8 s = bcd_2_dec(m_command_buffer[3]);
			const u8 f = bcd_2_dec(m_command_buffer[4]);
			const u32 pregap = m_toc->tracks[m_cdrom->get_track(frame)].pregap;

			frame = f + 75 * (s + m * 60);
			LOGCMD("MSF=%d %02d:%02d:%02d (pregap = %d)\n", frame, m, s, f, pregap);
			frame -= std::max(pregap, (u32)150);
			break;
		}
		case 0x80:
		{
			const u8 track_number = bcd_2_dec(m_command_buffer[2]);
			const u32 pregap = m_toc->tracks[m_cdrom->get_track(track_number - 1)].pregap;
			// NB: crazyhos uses this command with track = 1 on pre-title screen intro.
			// It's not supposed to playback anything according to real HW refs.
			frame = m_toc->tracks[ track_number - 1 ].logframeofs;

			LOGCMD("TRACK=%d (raw %02x pregap = %d frame = %d)\n"
				, track_number
				, m_command_buffer[2]
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
	m_cdda_play_mode = m_command_buffer[1] & 0x03;
	LOGCMD("Play mode = %d\n", m_cdda_play_mode);

	if (m_cdda_play_mode)
	{
		if (m_cdda_status == PCE_CD_CDDA_PAUSED)
		{
			LOGCDDA("Audio unpause\n");
			m_cdda->pause_audio(0);
		}

		LOGCDDA("Audio end current %d end %d\n", m_current_frame, m_end_frame);
		//printf("%08x %08x\n",m_current_frame,m_end_frame - m_current_frame);
		m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
		m_end_mark = 1;
		m_cdda_status = PCE_CD_CDDA_PLAYING;
	}
	else
	{
		LOGCDDA("Audio stop\n");
		m_cdda_status = PCE_CD_CDDA_OFF;
		m_cdda->stop_audio();
		m_end_frame = m_last_frame;
		m_end_mark = 0;
//      assert(nullptr == nec_set_audio_stop_position);
	}

	reply_status_byte(SCSI_STATUS_OK);
	// as above
	//set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
}

/* 0xDA - PAUSE (NEC) */
void pce_cd_device::nec_pause()
{
	/* If no cd mounted throw an error */
	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	LOGCMD("0xda PAUSE (NEC)\n");

	/* If there was no cdda playing, throw an error */
	if (m_cdda_status == PCE_CD_CDDA_OFF)
	{
		// TODO: sense key/ASC/ASCQ
		LOG("Issued SCSI_CHECK_CONDITION in 0xda!\n");
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	m_cdda_status = PCE_CD_CDDA_PAUSED;
	m_current_frame = m_cdda->get_audio_lba();
	LOGCDDA("Audio pause on %d LBA\n", m_current_frame);
	m_cdda->pause_audio(1);
	reply_status_byte(SCSI_STATUS_OK);
}

/* 0xDD - READ SUBCHANNEL Q (NEC) */
void pce_cd_device::nec_get_subq()
{
	/* WP - I do not have access to chds with subchannel information yet, so I'm faking something here */
	uint32_t msf_abs, msf_rel, track, frame;
	//LOGCMD("0xdd READ SUBCHANNEL Q (NEC) %d\n", m_cdda_status);

	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	frame = m_current_frame;

	switch (m_cdda_status)
	{
		case PCE_CD_CDDA_PAUSED:
			m_data_buffer[0] = 2;
			frame = m_cdda->get_audio_lba();
			break;
		case PCE_CD_CDDA_PLAYING:
			m_data_buffer[0] = 0;
			frame = m_cdda->get_audio_lba();
			break;
		default:
			m_data_buffer[0] = 3;
			break;
	}

	msf_abs = cdrom_file::lba_to_msf_alt(frame);
	track = m_cdrom->get_track(frame);
	msf_rel = cdrom_file::lba_to_msf_alt(frame - m_cdrom->get_track_start(track));

	m_data_buffer[1] = 0x01 | ((m_cdrom->get_track_type(m_cdrom->get_track(track+1)) == cdrom_file::CD_TRACK_AUDIO) ? 0x00 : 0x40);
	// track
	m_data_buffer[2] = dec_2_bcd(track+1);
	// index
	m_data_buffer[3] = 1;
	// MSF (relative)
	m_data_buffer[4] = dec_2_bcd((msf_rel >> 16) & 0xFF);
	m_data_buffer[5] = dec_2_bcd((msf_rel >> 8) & 0xFF);
	m_data_buffer[6] = dec_2_bcd(msf_rel & 0xFF);
	// MSF (absolute)
	m_data_buffer[7] = dec_2_bcd((msf_abs >> 16) & 0xFF);
	m_data_buffer[8] = dec_2_bcd((msf_abs >> 8) & 0xFF);
	m_data_buffer[9] = dec_2_bcd(msf_abs & 0xFF);
	if(LIVE_SUBQ_VIEW)
	{
		const std::vector<std::string> status_types = {"standby", "play", "pause"};
		popmessage("SUBQ - status %s type %02x|track %d index %d| MSF rel %06x MSF abs %06x\n"
			, status_types[m_cdda_status]
			, m_data_buffer[1]
			, track + 1
			, 1
			, msf_rel
			, msf_abs
		);
	}
	m_data_buffer_size = 10;

	m_data_buffer_index = 0;
	m_data_transferred = 1;
	m_scsi_IO = 1;
	m_scsi_CD = 0;
}

/* 0xDE - GET DIR INFO (NEC) */
void pce_cd_device::nec_get_dir_info()
{
	uint32_t frame, msf, track = 0;
	LOGCMD("0xde GET DIR INFO (NEC)\n");

	if (!m_cdrom->exists())
	{
		// TODO: sense key/ASC/ASCQ
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	const cdrom_file::toc &toc = m_cdrom->get_toc();

	switch (m_command_buffer[1])
	{
		case 0x00:
			m_data_buffer[0] = dec_2_bcd(1);
			m_data_buffer[1] = dec_2_bcd(toc.numtrks);
			LOGCMD("Get first and last track numbers => 1-%2d\n", m_data_buffer[1]);
			m_data_buffer_size = 2;
			break;
		case 0x01:
			frame = toc.tracks[toc.numtrks-1].logframeofs;
			frame += toc.tracks[toc.numtrks-1].frames;
			msf = cdrom_file::lba_to_msf(frame + 150);
			LOGCMD("Get total disk size in MSF format => %06x\n", msf);

			// M
			m_data_buffer[0] = (msf >> 16) & 0xFF;
			// S
			m_data_buffer[1] = (msf >> 8) & 0xFF;
			// F
			m_data_buffer[2] = msf & 0xFF;
			m_data_buffer_size = 3;
			break;
		case 0x02:
			if (m_command_buffer[2] == 0xAA)
			{
				frame = toc.tracks[toc.numtrks-1].logframeofs;
				frame += toc.tracks[toc.numtrks-1].frames;
				LOGCMD("Get lead-out => %06x\n", frame);
				m_data_buffer[3] = 0x04;   /* correct? */
			}
			else
			{
				track = std::max(bcd_2_dec(m_command_buffer[2]), 1U);
				frame = toc.tracks[track-1].logframeofs;
				LOGCMD("Get track info track = %d, frame = %d\n", track, frame);
				m_data_buffer[3] = (toc.tracks[track-1].trktype == cdrom_file::CD_TRACK_AUDIO) ? 0x00 : 0x04;
			}
			msf = cdrom_file::lba_to_msf(frame + 150);
			// M
			m_data_buffer[0] = (msf >> 16) & 0xFF;
			// S
			m_data_buffer[1] = (msf >> 8) & 0xFF;
			// F
			m_data_buffer[2] = msf & 0xFF;
			m_data_buffer_size = 4;
			break;
		default:
			popmessage("DIR INFO unemulated %02x", m_command_buffer[1]);
//          assert(pce_cd_nec_get_dir_info == nullptr);  // Not implemented yet
			break;
	}

	m_data_buffer_index = 0;
	m_data_transferred = 1;
	m_scsi_IO = 1;
	m_scsi_CD = 0;
}

void pce_cd_device::end_of_list()
{
	// TODO: sense key/ASC/ASCQ
	reply_status_byte(SCSI_CHECK_CONDITION);
}

typedef void (pce_cd_device::*command_handler_func)();

void pce_cd_device::handle_data_output()
{
	static const struct {
		uint8_t   command_byte;
		uint8_t   command_size;
		command_handler_func command_handler;
	} pce_cd_commands[] = {
		{ 0x00, 6, &pce_cd_device::test_unit_ready },                /* TEST UNIT READY */
		{ 0x08, 6, &pce_cd_device::read_6 },                         /* READ (6) */
		{ 0xD8,10, &pce_cd_device::nec_set_audio_start_position },   /* NEC SET AUDIO PLAYBACK START POSITION */
		{ 0xD9,10, &pce_cd_device::nec_set_audio_stop_position },    /* NEC SET AUDIO PLAYBACK END POSITION */
		{ 0xDA,10, &pce_cd_device::nec_pause },                      /* NEC PAUSE */
		{ 0xDD,10, &pce_cd_device::nec_get_subq },                   /* NEC GET SUBCHANNEL Q */
		{ 0xDE,10, &pce_cd_device::nec_get_dir_info },               /* NEC GET DIR INFO */
		{ 0xFF, 1, &pce_cd_device::end_of_list }                     /* end of list marker */
	};

	if (m_scsi_REQ && m_scsi_ACK)
	{
		/* Command byte received */
		LOGSCSI("Command byte $%02X received\n", m_cdc_data);

		/* Check for buffer overflow */
		assert(m_command_buffer_index < PCE_CD_COMMAND_BUFFER_SIZE);

		m_command_buffer[m_command_buffer_index] = m_cdc_data;
		m_command_buffer_index++;
		m_scsi_REQ = 0;
	}

	if (! m_scsi_REQ && ! m_scsi_ACK && m_command_buffer_index)
	{
		int i = 0;

		LOGSCSI("Check if command done\n");

		for(i = 0; m_command_buffer[0] > pce_cd_commands[i].command_byte; i++);

		/* Check for unknown commands */
		if (m_command_buffer[0] != pce_cd_commands[i].command_byte)
		{
			LOGSCSI("Unrecognized command: %02X\n", m_command_buffer[0]);
			if (m_command_buffer[0] == 0x03)
				popmessage("CD command 0x03 issued (Request Sense)");
		}
		assert(m_command_buffer[0] == pce_cd_commands[i].command_byte);

		if (m_command_buffer_index == pce_cd_commands[i].command_size)
		{
			//printf("%02x command issued\n",m_command_buffer[0]);
			(this->*pce_cd_commands[i].command_handler)();
			m_command_buffer_index = 0;
		}
		else
		{
			m_scsi_REQ = 1;
		}
	}
}

void pce_cd_device::cdda_end_mark_cb(int state)
{
	if (state != ASSERT_LINE)
		return;

	LOGCDDA("CDDA end mark %d\n", m_cdda_play_mode & 3);

	// handle end playback event
	if (m_end_mark == 1)
	{
		switch (m_cdda_play_mode & 3)
		{
			case 1:
			{
				// TODO: should seek rather than be instant
				LOGCDDA(" - Play with repeat %d %d\n", m_current_frame, m_end_frame);
				m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
				m_end_mark = 1;
				break;
			}
			case 2:
				LOGCDDA(" - IRQ when finished\n");
				set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
				m_end_mark = 0;
				break;
			case 3:
				LOGCDDA(" - Play without repeat\n");
				// fzone2 / fzone2j wants a STOP thru SUBQ command during intro
				m_cdda_status = PCE_CD_CDDA_OFF;
				m_cdda->stop_audio();
				m_end_mark = 0;
				break;
		}
	}
	else
		LOGCDDA(" - No end mark encountered, check me\n");
}

void pce_cd_device::handle_data_input()
{
	if (m_scsi_CD)
	{
		/* Command / Status byte */
		if (m_scsi_REQ && m_scsi_ACK)
		{
			LOGSCSI("status sent\n");
			m_scsi_REQ = 0;
			m_status_sent = 1;
		}

		if (! m_scsi_REQ && ! m_scsi_ACK && m_status_sent)
		{
			m_status_sent = 0;
			if (m_message_after_status)
			{
				LOGSCSI("message after status\n");
				m_message_after_status = 0;
				m_scsi_MSG = m_scsi_REQ = 1;
				m_cdc_data = 0;
			}
		}
	}
	else
	{
		/* Data */
		if (m_scsi_REQ && m_scsi_ACK)
		{
			m_scsi_REQ = 0;
		}

		if (! m_scsi_REQ && ! m_scsi_ACK)
		{
			if (m_data_buffer_index == m_data_buffer_size)
			{
				set_irq_line(PCE_CD_IRQ_TRANSFER_READY, CLEAR_LINE);
				if (m_data_transferred)
				{
					m_data_transferred = 0;
					reply_status_byte(SCSI_STATUS_OK);
					set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
				}
			}
			else
			{
				LOGSCSIXFER("Transfer byte %02x from offset %d %d\n",m_data_buffer[m_data_buffer_index] , m_data_buffer_index, m_current_frame);
				m_cdc_data = m_data_buffer[m_data_buffer_index];
				m_data_buffer_index++;
				m_scsi_REQ = 1;
			}
		}
	}
}

void pce_cd_device::handle_message_output()
{
	if (m_scsi_REQ && m_scsi_ACK)
		m_scsi_REQ = 0;
}

void pce_cd_device::handle_message_input()
{
	if (m_scsi_REQ && m_scsi_ACK)
	{
		m_scsi_REQ = 0;
		m_message_sent = 1;
	}

	if (! m_scsi_REQ && ! m_scsi_ACK && m_message_sent)
	{
		m_message_sent = 0;
		m_scsi_BSY = 0;
	}
}

/* Update internal CD statuses */
void pce_cd_device::update()
{
	/* Check for reset of CD unit */
	if (m_scsi_RST != m_scsi_last_RST)
	{
		if (m_scsi_RST)
		{
			LOGSCSI("Performing CD reset\n");
			/* Reset internal data */
			m_scsi_BSY = m_scsi_SEL = m_scsi_CD = m_scsi_IO = 0;
			m_scsi_MSG = m_scsi_REQ = m_scsi_ATN = 0;
			m_cd_motor_on = 0;
			m_selected = 0;
			m_cdda_status = PCE_CD_CDDA_OFF;
			m_cdda->stop_audio();
			m_adpcm_dma_timer->adjust(attotime::never); // stop ADPCM DMA here
		}
		m_scsi_last_RST = m_scsi_RST;
	}

	/* Check if bus can be freed */
	if (! m_scsi_SEL && ! m_scsi_BSY && m_selected)
	{
		LOGSCSI("freeing bus\n");
		m_selected = 0;
		m_scsi_CD = m_scsi_MSG = m_scsi_IO = m_scsi_REQ = 0;
		set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, CLEAR_LINE);
	}

	/* Select the CD device */
	if (m_scsi_SEL)
	{
		if (! m_selected)
		{
			m_selected = 1;
			LOGSCSI("Setting CD in device selection\n");
			m_scsi_BSY = m_scsi_REQ = m_scsi_CD = 1;
			m_scsi_MSG = m_scsi_IO = 0;
		}
	}

	if (m_scsi_ATN)
	{
	}
	else
	{
		/* Check for data and message phases */
		if (m_scsi_BSY)
		{
			if (m_scsi_MSG)
			{
				/* message phase */
				if (m_scsi_IO)
				{
					handle_message_input();
				}
				else
				{
					handle_message_output();
				}
			}
			else
			{
				/* data phase */
				if (m_scsi_IO)
				{
					/* Reading data from target */
					handle_data_input();
				}
				else
				{
					/* Sending data to target */
					handle_data_output();
				}
			}
		}
	}
}

void pce_cd_device::set_irq_line(int num, int state)
{
	if (state == ASSERT_LINE)
		m_irq_status |= num;
	else
		m_irq_status &= ~num;

	if (m_irq_mask & m_irq_status & 0x7c)
	{
		LOGIRQ("IRQ: PEND = %02x MASK = %02x STATUS %02x\n"
			, m_irq_mask & m_irq_status & 0x7c
			, m_irq_mask & 0x7c
			, m_irq_status & 0x7c
		);
		m_irq_cb(ASSERT_LINE);
	}
	else
	{
		m_irq_cb(CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::data_timer_callback)
{
	if (m_data_buffer_index == m_data_buffer_size)
	{
		/* Read next data sector */
		LOGSCSI("read sector %d\n", m_current_frame);
		if (! m_cdrom->read_data(m_current_frame, m_data_buffer.get(), cdrom_file::CD_TRACK_MODE1))
		{
			LOGSCSI("Mode1 CD read failed for frame #%d\n", m_current_frame);
		}
		else
		{
			LOGSCSI("Successfully read mode1 frame #%d\n", m_current_frame);
		}

		m_data_buffer_index = 0;
		m_data_buffer_size = 2048;
		m_current_frame++;

		m_scsi_IO = 1;
		m_scsi_CD = 0;

		if (m_current_frame == m_end_frame)
		{
			/* We are done, disable the timer */
			LOGSCSI("Last frame read from CD\n");
			m_data_transferred = 1;
			// data transfer is done, issue a pause
			m_cdda_status = PCE_CD_CDDA_PAUSED;
			m_data_timer->adjust(attotime::never);
		}
		else
		{
			m_data_transferred = 0;
		}
	}
}

void pce_cd_device::bram_w(offs_t offset, uint8_t data)
{
	if (!m_bram_locked)
	{
		m_bram[offset & (PCE_BRAM_SIZE - 1)] = data;
	}
}

uint8_t pce_cd_device::bram_r(offs_t offset)
{
	return m_bram[(offset & (PCE_BRAM_SIZE - 1)) + m_bram_locked * PCE_BRAM_SIZE];
}

void pce_cd_device::set_adpcm_ram_byte(uint8_t val)
{
	if (m_adpcm_write_buf > 0)
	{
		m_adpcm_write_buf--;
	}
	else
	{
		m_adpcm_ram[m_adpcm_write_ptr] = val;
		m_adpcm_write_ptr = ((m_adpcm_write_ptr + 1) & 0xffff);
		//TODO: length + 1
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::cdda_fadeout_callback)
{
	m_cdda_volume -= 0.1;

	if (m_cdda_volume <= 0)
	{
		m_cdda_volume = 0.0;
		m_cdda_fadeout_timer->adjust(attotime::never);
	}
	else
	{
		m_cdda_fadeout_timer->adjust(attotime::from_usec(param), param);
	}

	m_cdda->set_output_gain(ALL_OUTPUTS, m_cdda_volume / 100.0);
}

TIMER_CALLBACK_MEMBER(pce_cd_device::cdda_fadein_callback)
{
	m_cdda_volume += 0.1;

	if (m_cdda_volume >= 100.0)
	{
		m_cdda_volume = 100.0;
		m_cdda_fadein_timer->adjust(attotime::never);
	}
	else
	{
		m_cdda_fadein_timer->adjust(attotime::from_usec(param), param);
	}

	m_cdda->set_output_gain(ALL_OUTPUTS, m_cdda_volume / 100.0);
}

TIMER_CALLBACK_MEMBER(pce_cd_device::adpcm_fadeout_callback)
{
	m_adpcm_volume -= 0.1;

	if (m_adpcm_volume <= 0)
	{
		m_adpcm_volume = 0.0;
		m_adpcm_fadeout_timer->adjust(attotime::never);
	}
	else
	{
		m_adpcm_fadeout_timer->adjust(attotime::from_usec(param), param);
	}

	m_msm->set_output_gain(ALL_OUTPUTS, m_adpcm_volume / 100.0);
}

TIMER_CALLBACK_MEMBER(pce_cd_device::adpcm_fadein_callback)
{
	m_adpcm_volume += 0.1;

	if (m_adpcm_volume >= 100.0)
	{
		m_adpcm_volume = 100.0;
		m_adpcm_fadein_timer->adjust(attotime::never);
	}
	else
	{
		m_adpcm_fadein_timer->adjust(attotime::from_usec(param), param);
	}

	m_msm->set_output_gain(ALL_OUTPUTS, m_adpcm_volume / 100.0);
}

/*
 *
 * Register section
 *
 */

/*
 * CD Interface Register 0x00 - CDC status
 *
 * x--- ---- busy signal
 * -x-- ---- request signal
 * ---x ---- cd signal
 * ---- x--- i/o signal
 *
 */
uint8_t pce_cd_device::cdc_status_r()
{
	uint8_t res = (m_cdc_status & 7);
	res |= m_scsi_BSY ? 0x80 : 0;
	res |= m_scsi_REQ ? 0x40 : 0;
	res |= m_scsi_MSG ? 0x20 : 0;
	res |= m_scsi_CD  ? 0x10 : 0;
	res |= m_scsi_IO  ? 0x08 : 0;
	return res;
}

void pce_cd_device::cdc_status_w(uint8_t data)
{
	/* select device (which bits??) */
	m_scsi_SEL = 1;
	update();
	m_scsi_SEL = 0;
	m_adpcm_dma_timer->adjust(attotime::never); // stop ADPCM DMA here
	/* any write here clears CD transfer irqs */
	LOGIRQ("IRQ: CD clear & ~0x70\n");
	set_irq_line(0x70, CLEAR_LINE);
	m_cdc_status = data;
}

/*
 * CD Interface Register 0x01 - CDC command / status / data
 */
uint8_t pce_cd_device::cdc_data_r()
{
	return m_cdc_data;
}

void pce_cd_device::cdc_data_w(uint8_t data)
{
	m_cdc_data = data;
}


/*
 * CD Interface Register 0x02 - IRQ Mask and CD control
 *
 * x--- ---- to SCSI ACK
 * -x-- ---- transfer ready irq
 * --x- ---- transfer done irq
 * ---x ---- BRAM irq?
 * ---- x--- ADPCM FULL irq
 * ---- -x-- ADPCM HALF irq
 */
uint8_t pce_cd_device::irq_mask_r()
{
	return m_irq_mask;
}

void pce_cd_device::irq_mask_w(uint8_t data)
{
	m_scsi_ACK = data & 0x80;
	if (data & 0x7c)
		LOGIRQ("IRQ: mask %02x (%02x)\n", m_irq_mask & 0x7c, m_irq_status);
	m_irq_mask = data;
	set_irq_line(0, 0);
}

/*
 * CD Interface Register 0x03 - BRAM lock / CD status (read only)
 *
 * -x-- ---- CD acknowledge signal
 * --x- ---- CD done signal
 * ---x ---- bram signal (?)
 * ---- x--- ADPCM 2
 * ---- -x-- ADPCM 1
 * ---- --x- CDDA left/right speaker select
 */
uint8_t pce_cd_device::irq_status_r()
{
	uint8_t res = m_irq_status & 0x6e;
	// a read here locks the BRAM
	if (!machine().side_effects_disabled())
		m_bram_locked = 1;
	res |= (m_cd_motor_on ? 0x10 : 0);
	// TODO: gross hack, needs actual behaviour of CDDA data select
	if (!machine().side_effects_disabled())
		m_irq_status ^= 0x02;
	return res;
}

/*
 * CD Interface Register 0x04 - CD reset
 *
 * ---- --x- to SCSI RST
 */
uint8_t pce_cd_device::cdc_reset_r()
{
	return m_reset_reg;
}

void pce_cd_device::cdc_reset_w(uint8_t data)
{
	m_scsi_RST = data & 0x02;
	m_reset_reg = data;
}

/*
 * CD Interface Register 0x05 - CD-DA Volume low 8-bit port
 * CD Interface Register 0x06 - CD-DA Volume high 8-bit port
 */
uint8_t pce_cd_device::cdda_data_r(offs_t offset)
{
	// TODO: port 5 also converts?
	uint8_t port_shift = offset ? 8 : 0;

	// TODO: clamp over channel output_gain (audio CD player "fade out")
	return (m_cdda->get_channel_sample((m_irq_status & 2) ? 0 : 1) >> port_shift) & 0xff;
}

/*
 * CD Interface Register 0x07 - BRAM unlock / CD status
 *
 * x--- ---- Enables BRAM
 */
uint8_t pce_cd_device::bram_status_r()
{
	uint8_t res = (m_bram_locked ? (m_bram_status & 0x7f) : (m_bram_status | 0x80));
	return res;
}

void pce_cd_device::bram_unlock_w(uint8_t data)
{
	if (data & 0x80)
		m_bram_locked = 0;
	m_bram_status = data;
}

/*
 * CD Interface Register 0x08 - CD data (R) / ADPCM address low (W)
 */
uint8_t pce_cd_device::cd_data_r()
{
	return get_cd_data_byte();
}

void pce_cd_device::adpcm_address_lo_w(uint8_t data)
{
	m_adpcm_latch_address = (data & 0xff) | (m_adpcm_latch_address & 0xff00);
}

/*
 * CD Interface Register 0x09 - ADPCM address high (W)
 */
void pce_cd_device::adpcm_address_hi_w(uint8_t data)
{
	m_adpcm_latch_address = (data << 8) | (m_adpcm_latch_address & 0xff);
}

/*
 * CD interface Register 0x0a - ADPCM RAM data port
 */
uint8_t pce_cd_device::adpcm_data_r()
{
	return get_adpcm_ram_byte();
}

void pce_cd_device::adpcm_data_w(uint8_t data)
{
	set_adpcm_ram_byte(data);
}

/*
 * CD interface Register 0x0b - ADPCM DMA control
 */
uint8_t pce_cd_device::adpcm_dma_control_r()
{
	return m_adpcm_dma_reg;
}

void pce_cd_device::adpcm_dma_control_w(uint8_t data)
{
	if (data & 3)
	{
		m_adpcm_dma_timer->adjust(attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND * 2048), 0, attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND * 2048));
		m_adpcm_status |= 4;
	}
	m_adpcm_dma_reg = data;
}

/*
 * CD Interface Register 0x0c - ADPCM status
 *
 * x--- ---- ADPCM is reading data
 * ---- x--- ADPCM playback (0) stopped (1) currently playing
 * ---- -x-- pending ADPCM data write
 * ---- ---x ADPCM playback (1) stopped (0) currently playing
 */
uint8_t pce_cd_device::adpcm_status_r()
{
	return m_adpcm_status;
}

/*
 * CD Interface Register 0x0d - ADPCM address control
 *
 * x--- ---- ADPCM reset
 * -x-- ---- ADPCM play   - may be reversed
 * --x- ---- ADPCM repeat /
 * ---x ---- ADPCM set length
 * ---- x--- ADPCM set read address
 * ---- --xx ADPCM set write address
 */
uint8_t pce_cd_device::adpcm_address_control_r()
{
	// TODO: some games read bit 5 and want it to be low otherwise they hang
	// how that can cope with "repeat"?
	return m_adpcm_control;
}

void pce_cd_device::adpcm_address_control_w(uint8_t data)
{
	if ((m_adpcm_control & 0x80) && !(data & 0x80)) // ADPCM reset
	{
		/* Reset ADPCM hardware */
		m_adpcm_read_ptr = 0;
		m_adpcm_write_ptr = 0;
		m_msm_start_addr = 0;
		m_msm_end_addr = 0;
		m_msm_half_addr = 0;
		m_msm_nibble = 0;
		adpcm_stop(0);
		m_msm->reset_w(1);
	}

	// TODO: gulliver really starts an ADPCM play with bit 5 rather than 6
	// Is it a doc mistake and is actually reversed?
	m_msm_repeat = BIT(data, 5);

	if ((data & 0x40) && ((m_adpcm_control & 0x40) == 0)) // ADPCM play
	{
		m_msm_start_addr = (m_adpcm_read_ptr);
		m_msm_end_addr = (m_adpcm_read_ptr + m_adpcm_length) & 0xffff;
		m_msm_half_addr = (m_adpcm_read_ptr + (m_adpcm_length / 2)) & 0xffff;
		m_msm_nibble = 0;
		adpcm_play();
		m_msm->reset_w(0);

		//popmessage("%08x %08x",m_adpcm_read_ptr,m_adpcm_length);
	}
	else if ((data & 0x40) == 0)
	{
		// used by bbros to cancel an in-flight sample
		adpcm_stop(0);
		m_msm->reset_w(1);

		// addfam wants to irq ack here
		// https://mametesters.org/view.php?id=7261
		if(!(m_msm_repeat))
		{
			set_irq_line(PCE_CD_IRQ_SAMPLE_HALF_PLAY, CLEAR_LINE);
			set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
		}
	}


	if (data & 0x10) // ADPCM set length
	{
		m_adpcm_length = m_adpcm_latch_address;
	}
	if (data & 0x08) // ADPCM set read address
	{
		m_adpcm_read_ptr = m_adpcm_latch_address;
		m_adpcm_read_buf = 2;
	}
	if ((data & 0x02) == 0x02) // ADPCM set write address
	{
		m_adpcm_write_ptr = m_adpcm_latch_address;
		m_adpcm_write_buf = data & 1;
	}

	m_adpcm_control = data;
}

/*
 * CD Interface Register 0x0e - ADPCM playback rate
 */
void pce_cd_device::adpcm_playback_rate_w(uint8_t data)
{
	m_adpcm_clock_divider = 0x10 - (data & 0x0f);
	m_msm->set_unscaled_clock((PCE_CD_CLOCK / 6) / m_adpcm_clock_divider);
}

/*
 * CD Interface Register 0x0f - CD-DA/ADPCM fader in/out register
 *
 * ---- xxxx command setting:
 * 0x00 ADPCM/CD-DA fade-in
 * 0x01 CD-DA fade-in
 * 0x08 CD-DA fade-out (short) ADPCM fade-in
 * 0x09 CD-DA fade-out (long)
 * 0x0a ADPCM fade-out (long)
 * 0x0c CD-DA fade-out (short) ADPCM fade-in
 * 0x0d CD-DA fade-out (short)
 * 0x0e ADPCM fade-out (short)
 */
void pce_cd_device::fader_control_w(uint8_t data)
{
	if (data & 0xf0)
		LOG("fader_control_w with upper bits set! %02x\n", data);

	// TODO: timers needs HW tests
	if (m_fader_ctrl != data)
	{
		LOGFADER("Fader %01x ", data & 0xf);
		switch (data & 0xf)
		{
			case 0x00:
				LOGFADER("ADPCM/CD-DA enable (100 msecs)\n");
				m_cdda_volume = 0.0;
				m_cdda_fadein_timer->adjust(attotime::from_usec(100), 100);
				m_adpcm_volume = 0.0;
				m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
				m_cdda_fadeout_timer->adjust(attotime::never);
				m_adpcm_fadeout_timer->adjust(attotime::never);
				break;

			case 0x01:
				LOGFADER("CD-DA enable (100 msecs)\n");
				m_cdda_volume = 0.0;
				m_cdda_fadein_timer->adjust(attotime::from_usec(100), 100);
				m_cdda_fadeout_timer->adjust(attotime::never);
				break;

			case 0x08:
				LOGFADER("CD-DA short (1500 msecs) fade out / ADPCM enable\n");
				m_cdda_volume = 100.0;
				m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
				m_adpcm_volume = 0.0;
				m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
				m_cdda_fadein_timer->adjust(attotime::never);
				m_adpcm_fadeout_timer->adjust(attotime::never);
				break;

			case 0x09:
				LOGFADER("CD-DA long (5000 msecs) fade out\n");
				m_cdda_volume = 100.0;
				m_cdda_fadeout_timer->adjust(attotime::from_usec(5000), 5000);
				m_cdda_fadein_timer->adjust(attotime::never);
				break;

			case 0x0a:
				LOGFADER("ADPCM long (5000 msecs) fade out\n");
				m_adpcm_volume = 100.0;
				m_adpcm_fadeout_timer->adjust(attotime::from_usec(5000), 5000);
				m_adpcm_fadein_timer->adjust(attotime::never);
				break;

			case 0x0c:
				LOGFADER("CD-DA short (1500 msecs) fade out / ADPCM enable\n");
				m_cdda_volume = 100.0;
				m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
				m_adpcm_volume = 0.0;
				m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
				m_cdda_fadein_timer->adjust(attotime::never);
				m_adpcm_fadeout_timer->adjust(attotime::never);
				break;

			case 0x0d:
				LOGFADER("CD-DA short (1500 msecs) fade out\n");
				m_cdda_volume = 100.0;
				m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
				m_cdda_fadein_timer->adjust(attotime::never);
				break;

			case 0x0e:
				LOGFADER("ADPCM short (1500 msecs) fade out\n");
				m_adpcm_volume = 100.0;
				m_adpcm_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
				m_adpcm_fadein_timer->adjust(attotime::never);
				break;

			default:
				popmessage("CD-DA / ADPCM Fade effect mode %02x",data & 0x0f);
				break;
		}
	}
	m_fader_ctrl = data;
}

TIMER_CALLBACK_MEMBER(pce_cd_device::clear_ack)
{
	update();
	m_scsi_ACK = 0;
	update();
	if (m_scsi_CD)
	{
		m_adpcm_dma_reg &= 0xFC;
	}
}

uint8_t pce_cd_device::get_cd_data_byte()
{
	uint8_t data = m_cdc_data;
	if (m_scsi_REQ && !m_scsi_ACK && !m_scsi_CD)
	{
		if (m_scsi_IO)
		{
			m_scsi_ACK = 1;
			m_ack_clear_timer->adjust(m_maincpu->cycles_to_attotime(15));
		}
	}
	return data;
}


TIMER_CALLBACK_MEMBER(pce_cd_device::adpcm_dma_timer_callback)
{
	if (m_scsi_REQ && !m_scsi_ACK && !m_scsi_CD && m_scsi_IO)
	{
		m_adpcm_ram[m_adpcm_write_ptr] = get_cd_data_byte();
		m_adpcm_write_ptr = (m_adpcm_write_ptr + 1) & 0xFFFF;

		m_adpcm_status &= ~4;
	}
}

uint8_t pce_cd_device::get_adpcm_ram_byte()
{
	if (m_adpcm_read_buf > 0)
	{
		if (!machine().side_effects_disabled())
			m_adpcm_read_buf--;
		return 0;
	}
	else
	{
		uint8_t res;

		res = m_adpcm_ram[m_adpcm_read_ptr];
		if (!machine().side_effects_disabled())
			m_adpcm_read_ptr = ((m_adpcm_read_ptr + 1) & 0xffff);

		return res;
	}
}

/*
 *
 * I/O accessors
 *
 */
// TODO: more stuff actually belongs to the whole CD interface,
//       cfr. pce_cd_intf_r/w in drivers/pce.cpp
uint8_t pce_cd_device::intf_r(offs_t offset)
{
	//logerror("%s: read from CD interface offset %02X\n", machine().describe_context(), offset );

	address_space &io_space = this->space(AS_IO);
	return io_space.read_byte(offset & 0xf);
}

void pce_cd_device::intf_w(offs_t offset, uint8_t data)
{
	//logerror("%s write to CD interface offset %02X, data %02X\n", machine().describe_context(), offset, data);

	address_space &io_space = this->space(AS_IO);
	io_space.write_byte(offset & 0xf, data);
}
