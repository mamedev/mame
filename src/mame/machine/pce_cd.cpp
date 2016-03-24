// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/************************************************************

PC Engine CD HW notes:

TODO:
- Dragon Ball Z: ADPCM dies after the first upload;
- Dragon Slayer - The Legend of Heroes: black screen;
- Mirai Shonen Conan: dies at new game selection;
- Snatcher: black screen after Konami logo, tries set up CD-DA
            while transferring data?
- Steam Heart's: needs transfer ready irq to get past the
                 gameplay hang, don't know exactly when it should fire
- Steam Heart's: bad ADPCM irq, dialogue is cutted due of it;

=============================================================

CD Interface Register 0x00 - CDC status
x--- ---- busy signal
-x-- ---- request signal
---x ---- cd signal
---- x--- i/o signal

CD Interface Register 0x03 - BRAM lock / CD status
-x-- ---- acknowledge signal
--x- ---- done signal
---x ---- bram signal
---- x--- ADPCM 2
---- -x-- ADPCM 1
---- --x- CDDA left/right speaker select

CD Interface Register 0x05 - CD-DA Volume low 8-bit port

CD Interface Register 0x06 - CD-DA Volume high 8-bit port

CD Interface Register 0x07 - BRAM unlock / CD status
x--- ---- Enables BRAM

CD Interface Register 0x0c - ADPCM status
x--- ---- ADPCM is reading data
---- x--- ADPCM playback (0) stopped (1) currently playing
---- -x-- pending ADPCM data write
---- ---x ADPCM playback (1) stopped (0) currently playing

CD Interface Register 0x0d - ADPCM address control
x--- ---- ADPCM reset
-x-- ---- ADPCM play
--x- ---- ADPCM repeat
---x ---- ADPCM set length
---- x--- ADPCM set read address
---- --xx ADPCM set write address
(note: some games reads bit 5 and wants it to be low otherwise they hangs, surely NOT an ADPCM repeat flag read because it doesn't make sense)

CD Interface Register 0x0e - ADPCM playback rate

CD Interface Register 0x0f - ADPCM fade in/out register
---- xxxx command setting:
0x00 ADPCM/CD-DA Fade-in
0x01 CD-DA fade-in
0x08 CD-DA fade-out (short) ADPCM fade-in
0x09 CD-DA fade-out (long)
0x0a ADPCM fade-out (long)
0x0c CD-DA fade-out (short) ADPCM fade-in
0x0d CD-DA fade-out (short)
0x0e ADPCM fade-out (short)

*************************************************************/

#include "emu.h"
#include "coreutil.h"
#include "machine/pce_cd.h"


#define PCE_CD_CLOCK    9216000


const device_type PCE_CD = &device_creator<pce_cd_device>;


pce_cd_device::pce_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, PCE_CD, "PCE CD Add-on", tag, owner, clock, "pcecd", __FILE__),
						m_maincpu(*this, ":maincpu"),
						m_msm(*this, "msm5205"),
						m_cdda(*this, "cdda"),
						m_nvram(*this, "bram"),
						m_cdrom(*this, "cdrom")
{
}

void pce_cd_device::device_start()
{
	/* Initialize BRAM */
	m_bram = std::make_unique<UINT8[]>(PCE_BRAM_SIZE * 2);
	memset(m_bram.get(), 0, PCE_BRAM_SIZE);
	memset(m_bram.get() + PCE_BRAM_SIZE, 0xff, PCE_BRAM_SIZE);
	m_bram_locked = 1;

	m_nvram->set_base(m_bram.get(), PCE_BRAM_SIZE);

	/* set up adpcm related things */
	m_adpcm_ram = make_unique_clear<UINT8[]>(PCE_ADPCM_RAM_SIZE);
	m_adpcm_clock_divider = 1;

	/* Set up cd command buffer */
	m_command_buffer = make_unique_clear<UINT8[]>(PCE_CD_COMMAND_BUFFER_SIZE);
	m_command_buffer_index = 0;

	/* Set up Arcade Card RAM buffer */
	m_acard_ram = make_unique_clear<UINT8[]>(PCE_ACARD_RAM_SIZE);

	m_data_buffer = make_unique_clear<UINT8[]>(8192);
	m_data_buffer_size = 0;
	m_data_buffer_index = 0;

	m_subcode_buffer = std::make_unique<UINT8[]>(96);

	m_data_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::data_timer_callback),this));
	m_data_timer->adjust(attotime::never);
	m_adpcm_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::adpcm_dma_timer_callback),this));
	m_adpcm_dma_timer->adjust(attotime::never);

	m_cdda_fadeout_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::cdda_fadeout_callback),this));
	m_cdda_fadeout_timer->adjust(attotime::never);
	m_cdda_fadein_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::cdda_fadein_callback),this));
	m_cdda_fadein_timer->adjust(attotime::never);

	m_adpcm_fadeout_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::adpcm_fadeout_callback),this));
	m_adpcm_fadeout_timer->adjust(attotime::never);
	m_adpcm_fadein_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pce_cd_device::adpcm_fadein_callback),this));
	m_adpcm_fadein_timer->adjust(attotime::never);

	// m_cd_file pointer is setup at a later stage because it is still empty when this function is called

	// TODO: add proper restore for the cd data...
	save_item(NAME(m_regs));
	save_pointer(NAME(m_bram.get()), PCE_BRAM_SIZE * 2);
	save_pointer(NAME(m_adpcm_ram.get()), PCE_ADPCM_RAM_SIZE);
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
	save_pointer(NAME(m_command_buffer.get()), PCE_CD_COMMAND_BUFFER_SIZE);
	save_item(NAME(m_command_buffer_index));
	save_item(NAME(m_status_sent));
	save_item(NAME(m_message_after_status));
	save_item(NAME(m_message_sent));
	save_pointer(NAME(m_data_buffer.get()), 8192);
	save_item(NAME(m_data_buffer_size));
	save_item(NAME(m_data_buffer_index));
	save_item(NAME(m_data_transferred));
	save_pointer(NAME(m_acard_ram.get()), PCE_ACARD_RAM_SIZE);
	save_item(NAME(m_acard_latch));
	save_item(NAME(m_acard_ctrl));
	save_item(NAME(m_acard_base_addr));
	save_item(NAME(m_acard_addr_offset));
	save_item(NAME(m_acard_addr_inc));
	save_item(NAME(m_acard_shift));
	save_item(NAME(m_acard_shift_reg));
	save_item(NAME(m_current_frame));
	save_item(NAME(m_end_frame));
	save_item(NAME(m_last_frame));
	save_item(NAME(m_cdda_status));
	save_item(NAME(m_cdda_play_mode));
	save_pointer(NAME(m_subcode_buffer.get()), 96);
	save_item(NAME(m_end_mark));
	save_item(NAME(m_cdda_volume));
	save_item(NAME(m_adpcm_volume));
}

void pce_cd_device::device_reset()
{
	m_adpcm_read_buf = 0;
	m_adpcm_write_buf = 0;

	// TODO: add CD-DA stop command here
	//m_cdda_status = PCE_CD_CDDA_OFF;
	//m_cdda->stop_audio();

	memset(m_regs, 0, sizeof(m_regs));

	m_regs[0x0c] |= PCE_CD_ADPCM_STOP_FLAG;
	m_regs[0x0c] &= ~PCE_CD_ADPCM_PLAY_FLAG;
	//m_regs[0x03] = (m_regs[0x03] & ~0x0c) | (PCE_CD_SAMPLE_STOP_PLAY);
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
	m_cd_file = m_cdrom->get_cdrom_file();
	if (m_cd_file)
	{
		m_toc = cdrom_get_toc(m_cd_file);
		m_cdda->set_cdrom(m_cd_file);
		m_last_frame = cdrom_get_track_start(m_cd_file, cdrom_get_last_track(m_cd_file) - 1);
		m_last_frame += m_toc->tracks[cdrom_get_last_track(m_cd_file) - 1].frames;
		m_end_frame = m_last_frame;
	}

	// MSM5205 might be initialized after PCE CD as well...
	m_msm->change_clock_w((PCE_CD_CLOCK / 6) / m_adpcm_clock_divider);
}

void pce_cd_device::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	static const UINT8 init[8] = { 0x48, 0x55, 0x42, 0x4d, 0x00, 0xa0, 0x10, 0x80 };

	memset(data, 0x00, size);
	memcpy(data, init, sizeof(init));
}

// TODO: left and right speaker tags should be passed from the parent config, instead of using the hard-coded ones below!?!
static MACHINE_CONFIG_FRAGMENT( pce_cd )
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("bram", pce_cd_device, nvram_init)

	MCFG_CDROM_ADD("cdrom")
	MCFG_CDROM_INTERFACE("pce_cdrom")

	MCFG_SOUND_ADD( "msm5205", MSM5205, PCE_CD_CLOCK / 6 )
	MCFG_MSM5205_VCLK_CB(WRITELINE(pce_cd_device, msm5205_int)) /* interrupt function */
	MCFG_MSM5205_PRESCALER_SELECTOR(MSM5205_S48_4B)      /* 1/48 prescaler, 4bit data */
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "^:lspeaker", 0.50 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "^:rspeaker", 0.50 )

	MCFG_SOUND_ADD( "cdda", CDDA, 0 )
	MCFG_SOUND_ROUTE( 0, "^:lspeaker", 1.00 )
	MCFG_SOUND_ROUTE( 1, "^:rspeaker", 1.00 )
MACHINE_CONFIG_END


machine_config_constructor pce_cd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pce_cd );
}


void pce_cd_device::adpcm_stop(UINT8 irq_flag)
{
	m_regs[0x0c] |= PCE_CD_ADPCM_STOP_FLAG;
	m_regs[0x0c] &= ~PCE_CD_ADPCM_PLAY_FLAG;
	//m_regs[0x03] = (m_regs[0x03] & ~0x0c) | (PCE_CD_SAMPLE_STOP_PLAY);
	if (irq_flag)
		set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, ASSERT_LINE);
	m_regs[0x0d] &= ~0x60;
	m_msm_idle = 1;
}

void pce_cd_device::adpcm_play()
{
	m_regs[0x0c] &= ~PCE_CD_ADPCM_STOP_FLAG;
	m_regs[0x0c] |= PCE_CD_ADPCM_PLAY_FLAG;
	set_irq_line(PCE_CD_IRQ_SAMPLE_FULL_PLAY, CLEAR_LINE);
	m_regs[0x03] = (m_regs[0x03] & ~0x0c);
	m_msm_idle = 0;
}


/* Callback for new data from the MSM5205.
  The PCE cd unit actually divides the clock signal supplied to
  the MSM5205. Currently we can only use static clocks for the
  MSM5205.
 */
WRITE_LINE_MEMBER( pce_cd_device::msm5205_int )
{
	UINT8 msm_data;

	//  popmessage("%08x %08x %08x %02x %02x",m_msm_start_addr,m_msm_end_addr,m_msm_half_addr,m_regs[0x0c],m_regs[0x0d]);

	if (m_msm_idle)
		return;

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

void pce_cd_device::reply_status_byte(UINT8 status)
{
	logerror("Setting CD in reply_status_byte\n");
	m_scsi_CD = m_scsi_IO = m_scsi_REQ = 1;
	m_scsi_MSG = 0;
	m_message_after_status = 1;
	m_status_sent = m_message_sent = 0;

	if (status == SCSI_STATUS_OK)
	{
		m_regs[0x01] = 0x00;
	}
	else
	{
		m_regs[0x01] = 0x01;
	}
}

/* 0x00 - TEST UNIT READY */
void pce_cd_device::test_unit_ready()
{
	logerror("test unit ready\n");
	if (m_cd_file)
	{
		logerror("Sending STATUS_OK status\n");
		reply_status_byte(SCSI_STATUS_OK);
	}
	else
	{
		logerror("Sending CHECK_CONDITION status\n");
		reply_status_byte(SCSI_CHECK_CONDITION);
	}
}

/* 0x08 - READ (6) */
void pce_cd_device::read_6()
{
	UINT32 frame = ((m_command_buffer[1] & 0x1f) << 16) | (m_command_buffer[2] << 8) | m_command_buffer[3];
	UINT32 frame_count = m_command_buffer[4];
	printf("%08x %08x\n",frame,frame_count);

	/* Check for presence of a CD */
	if (!m_cd_file)
	{
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
		/* Star Breaker uses this */
		popmessage("Read Sector frame count == 0, contact MESSdev");
		reply_status_byte(SCSI_STATUS_OK);
	}
	else
	{
		m_data_timer->adjust(attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND), 0, attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND));
	}

	/* TODO: correct place? */
	set_irq_line(PCE_CD_IRQ_TRANSFER_READY, ASSERT_LINE);
}

/* 0xD8 - SET AUDIO PLAYBACK START POSITION (NEC) */
void pce_cd_device::nec_set_audio_start_position()
{
	UINT32  frame = 0;

	if (!m_cd_file)
	{
		/* Throw some error here */
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	switch (m_command_buffer[9] & 0xC0)
	{
		case 0x00:
			popmessage("CD-DA set start mode 0x00, contact MESSdev");
			frame = (m_command_buffer[3] << 16) | (m_command_buffer[4] << 8) | m_command_buffer[5];
			break;
		case 0x40:
		{
			UINT8 m,s,f;

			m = bcd_2_dec(m_command_buffer[2]);
			s = bcd_2_dec(m_command_buffer[3]);
			f = bcd_2_dec(m_command_buffer[4]);

			frame = f + 75 * (s + m * 60);
			// PCE tries to be clever here and set (start of track + track pregap size) to skip the pregap
			// (I guess it wants the TOC to have the real start sector for data tracks and the start of the pregap for audio?)
			frame -= m_toc->tracks[cdrom_get_track(m_cd_file, frame)].pregap;
			break;
		}
		case 0x80:
			frame = m_toc->tracks[ bcd_2_dec(m_command_buffer[2]) - 1 ].logframeofs;
			break;
		default:
			popmessage("CD-DA set start mode 0xc0, contact MESSdev");
			//assert(NULL == nec_set_audio_start_position);
			break;
	}

	m_current_frame = frame;

	if (m_cdda_status == PCE_CD_CDDA_PAUSED)
	{
		m_cdda_status = PCE_CD_CDDA_OFF;
		m_cdda->stop_audio();
		m_end_frame = m_last_frame;
		m_end_mark = 0;
	}
	else
	{
		if (m_command_buffer[1] & 0x03)
		{
			m_cdda_status = PCE_CD_CDDA_PLAYING;
			m_end_frame = m_last_frame; //get the end of the CD
			m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
			m_cdda_play_mode = (m_command_buffer[1] & 0x02) ? 2 : 3; // mode 2 sets IRQ at end
			m_end_mark =  (m_command_buffer[1] & 0x02) ? 1 : 0;
		}
		else
		{
			m_cdda_status = PCE_CD_CDDA_PLAYING;
			m_end_frame = m_toc->tracks[ cdrom_get_track(m_cd_file, m_current_frame) + 1 ].logframeofs; //get the end of THIS track
			m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
			m_end_mark = 0;
			m_cdda_play_mode = 3;
		}
	}

	reply_status_byte(SCSI_STATUS_OK);
	set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
}

/* 0xD9 - SET AUDIO PLAYBACK END POSITION (NEC) */
void pce_cd_device::nec_set_audio_stop_position()
{
	UINT32  frame = 0;

	if (!m_cd_file)
	{
		/* Throw some error here */
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	switch (m_command_buffer[9] & 0xC0)
	{
		case 0x00:
			popmessage("CD-DA set end mode 0x00, contact MESSdev");
			frame = (m_command_buffer[3] << 16) | (m_command_buffer[4] << 8) | m_command_buffer[5];
			break;
		case 0x40:
		{
			UINT8 m,s,f;

			m = bcd_2_dec(m_command_buffer[2]);
			s = bcd_2_dec(m_command_buffer[3]);
			f = bcd_2_dec(m_command_buffer[4]);

			frame = f + 75 * (s + m * 60);
			//      if (frame >= 525) // TODO: seven seconds gap? O_o
			//          frame -= 525;
			break;
		}
		case 0x80:
			frame = m_toc->tracks[ bcd_2_dec(m_command_buffer[2]) - 1 ].logframeofs;
			break;
		default:
			popmessage("CD-DA set end mode 0xc0, contact MESSdev");
			//assert(NULL == nec_set_audio_start_position);
			break;
	}

	m_end_frame = frame;
	m_cdda_play_mode = m_command_buffer[1] & 0x03;

	if (m_cdda_play_mode)
	{
		if (m_cdda_status == PCE_CD_CDDA_PAUSED)
		{
			m_cdda->pause_audio(0);
		}
		else
		{
			//printf("%08x %08x\n",m_current_frame,m_end_frame - m_current_frame);
			m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame);
			m_end_mark = 1;
		}
		m_cdda_status = PCE_CD_CDDA_PLAYING;
	}
	else
	{
		m_cdda_status = PCE_CD_CDDA_OFF;
		m_cdda->stop_audio();
		m_end_frame = m_last_frame;
		m_end_mark = 0;
//      assert(NULL == nec_set_audio_stop_position);
	}

	reply_status_byte(SCSI_STATUS_OK);
	set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE);
}

/* 0xDA - PAUSE (NEC) */
void pce_cd_device::nec_pause()
{
	/* If no cd mounted throw an error */
	if (!m_cd_file)
	{
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	/* If there was no cdda playing, throw an error */
	if (m_cdda_status == PCE_CD_CDDA_OFF)
	{
		reply_status_byte(SCSI_CHECK_CONDITION);
		return;
	}

	m_cdda_status = PCE_CD_CDDA_PAUSED;
	m_current_frame = m_cdda->get_audio_lba();
	m_cdda->pause_audio(1);
	reply_status_byte(SCSI_STATUS_OK);
}

/* 0xDD - READ SUBCHANNEL Q (NEC) */
void pce_cd_device::nec_get_subq()
{
	/* WP - I do not have access to chds with subchannel information yet, so I'm faking something here */
	UINT32 msf_abs, msf_rel, track, frame;

	if (!m_cd_file)
	{
		/* Throw some error here */
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

	msf_abs = lba_to_msf_alt(frame);
	track = cdrom_get_track(m_cd_file, frame);
	msf_rel = lba_to_msf_alt(frame - cdrom_get_track_start(m_cd_file, track));

	m_data_buffer[1] = 0x01 | ((cdrom_get_track_type(m_cd_file, cdrom_get_track(m_cd_file, track+1)) == CD_TRACK_AUDIO) ? 0x00 : 0x40);
	m_data_buffer[2] = dec_2_bcd(track+1);       /* track */
	m_data_buffer[3] = 1;                          /* index */
	m_data_buffer[4] = dec_2_bcd((msf_rel >> 16) & 0xFF);/* M (relative) */
	m_data_buffer[5] = dec_2_bcd((msf_rel >> 8) & 0xFF); /* S (relative) */
	m_data_buffer[6] = dec_2_bcd(msf_rel & 0xFF);          /* F (relative) */
	m_data_buffer[7] = dec_2_bcd((msf_abs >> 16) & 0xFF);/* M (absolute) */
	m_data_buffer[8] = dec_2_bcd((msf_abs >> 8) & 0xFF); /* S (absolute) */
	m_data_buffer[9] = dec_2_bcd(msf_abs & 0xFF);          /* F (absolute) */
	m_data_buffer_size = 10;

	m_data_buffer_index = 0;
	m_data_transferred = 1;
	m_scsi_IO = 1;
	m_scsi_CD = 0;
}

/* 0xDE - GET DIR INFO (NEC) */
void pce_cd_device::nec_get_dir_info()
{
	UINT32 frame, msf, track = 0;
	const cdrom_toc *toc;
	logerror("nec get dir info\n");

	if (!m_cd_file)
	{
		/* Throw some error here */
		reply_status_byte(SCSI_CHECK_CONDITION);
	}

	toc = cdrom_get_toc(m_cd_file);

	switch (m_command_buffer[1])
	{
		case 0x00:      /* Get first and last track numbers */
			m_data_buffer[0] = dec_2_bcd(1);
			m_data_buffer[1] = dec_2_bcd(toc->numtrks);
			m_data_buffer_size = 2;
			break;
		case 0x01:      /* Get total disk size in MSF format */
			frame = toc->tracks[toc->numtrks-1].logframeofs;
			frame += toc->tracks[toc->numtrks-1].frames;
			msf = lba_to_msf(frame + 150);

			m_data_buffer[0] = (msf >> 16) & 0xFF;   /* M */
			m_data_buffer[1] = (msf >> 8) & 0xFF;    /* S */
			m_data_buffer[2] = msf & 0xFF;             /* F */
			m_data_buffer_size = 3;
			break;
		case 0x02:      /* Get track information */
			if (m_command_buffer[2] == 0xAA)
			{
				frame = toc->tracks[toc->numtrks-1].logframeofs;
				frame += toc->tracks[toc->numtrks-1].frames;
				m_data_buffer[3] = 0x04;   /* correct? */
			} else
			{
				track = MAX(bcd_2_dec(m_command_buffer[2]), 1);
				frame = toc->tracks[track-1].logframeofs;
				// PCE wants the start sector for data tracks to *not* include the pregap
				if (toc->tracks[track-1].trktype != CD_TRACK_AUDIO)
				{
					frame += toc->tracks[track-1].pregap;
				}
				m_data_buffer[3] = (toc->tracks[track-1].trktype == CD_TRACK_AUDIO) ? 0x00 : 0x04;
			}
			logerror("track = %d, frame = %d\n", track, frame);
			msf = lba_to_msf(frame + 150);
			m_data_buffer[0] = (msf >> 16) & 0xFF;   /* M */
			m_data_buffer[1] = (msf >> 8) & 0xFF;    /* S */
			m_data_buffer[2] = msf & 0xFF;             /* F */
			m_data_buffer_size = 4;
			break;
		default:
//          assert(pce_cd_nec_get_dir_info == NULL);  // Not implemented yet
			break;
	}

	m_data_buffer_index = 0;
	m_data_transferred = 1;
	m_scsi_IO = 1;
	m_scsi_CD = 0;
}

void pce_cd_device::end_of_list()
{
	reply_status_byte(SCSI_CHECK_CONDITION);
}

typedef void (pce_cd_device::*command_handler_func)();

void pce_cd_device::handle_data_output()
{
	static const struct {
		UINT8   command_byte;
		UINT8   command_size;
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
		logerror("Command byte $%02X received\n", m_regs[0x01]);

		/* Check for buffer overflow */
		assert(m_command_buffer_index < PCE_CD_COMMAND_BUFFER_SIZE);

		m_command_buffer[m_command_buffer_index] = m_regs[0x01];
		m_command_buffer_index++;
		m_scsi_REQ = 0;
	}

	if (! m_scsi_REQ && ! m_scsi_ACK && m_command_buffer_index)
	{
		int i = 0;

		logerror("Check if command done\n");

		for(i = 0; m_command_buffer[0] > pce_cd_commands[i].command_byte; i++);

		/* Check for unknown commands */
		if (m_command_buffer[0] != pce_cd_commands[i].command_byte)
		{
			logerror("Unrecognized command: %02X\n", m_command_buffer[0]);
			if (m_command_buffer[0] == 0x03)
				popmessage("CD command 0x03 issued (Request Sense), contact MESSdev");
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

void pce_cd_device::handle_data_input()
{
	if (m_scsi_CD)
	{
		/* Command / Status byte */
		if (m_scsi_REQ && m_scsi_ACK)
		{
			logerror("status sent\n");
			m_scsi_REQ = 0;
			m_status_sent = 1;
		}

		if (! m_scsi_REQ && ! m_scsi_ACK && m_status_sent)
		{
			m_status_sent = 0;
			if (m_message_after_status)
			{
				logerror("message after status\n");
				m_message_after_status = 0;
				m_scsi_MSG = m_scsi_REQ = 1;
				m_regs[0x01] = 0;
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
				logerror("Transfer byte %02x from offset %d %d\n",m_data_buffer[m_data_buffer_index] , m_data_buffer_index, m_current_frame);
				m_regs[0x01] = m_data_buffer[m_data_buffer_index];
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
			logerror("Performing CD reset\n");
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
		logerror("freeing bus\n");
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
			logerror("Setting CD in device selection\n");
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

	/* FIXME: presumably CD-DA needs an irq interface for this */
	if (m_cdda->audio_ended() && m_end_mark == 1)
	{
		switch (m_cdda_play_mode & 3)
		{
			case 1: m_cdda->start_audio(m_current_frame, m_end_frame - m_current_frame); m_end_mark = 1; break; //play with repeat
			case 2: set_irq_line(PCE_CD_IRQ_TRANSFER_DONE, ASSERT_LINE); m_end_mark = 0; break; //irq when finished
			case 3: m_end_mark = 0; break; //play without repeat
		}
	}
}

void pce_cd_device::set_irq_line(int num, int state)
{
	if (state == ASSERT_LINE)
		m_regs[0x03] |= num;
	else
		m_regs[0x03] &= ~num;

	if (m_regs[0x02] & m_regs[0x03] & 0x7c)
	{
		//printf("IRQ PEND = %02x MASK = %02x IRQ ENABLE %02X\n",m_regs[0x02] & m_regs[0x03] & 0x7c,m_regs[0x02] & 0x7c,m_regs[0x03] & 0x7c);
		m_maincpu->set_input_line(1, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(1, CLEAR_LINE);
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::data_timer_callback)
{
	if (m_data_buffer_index == m_data_buffer_size)
	{
		/* Read next data sector */
		logerror("read sector %d\n", m_current_frame);
		if (! cdrom_read_data(m_cd_file, m_current_frame, m_data_buffer.get(), CD_TRACK_MODE1))
		{
			logerror("Mode1 CD read failed for frame #%d\n", m_current_frame);
		}
		else
		{
			logerror("Successfully read mode1 frame #%d\n", m_current_frame);
		}

		m_data_buffer_index = 0;
		m_data_buffer_size = 2048;
		m_current_frame++;

		m_scsi_IO = 1;
		m_scsi_CD = 0;

		if (m_current_frame == m_end_frame)
		{
			/* We are done, disable the timer */
			logerror("Last frame read from CD\n");
			m_data_transferred = 1;
			m_data_timer->adjust(attotime::never);
		}
		else
		{
			m_data_transferred = 0;
		}
	}
}

WRITE8_MEMBER(pce_cd_device::bram_w)
{
	if (!m_bram_locked)
	{
		m_bram[offset & (PCE_BRAM_SIZE - 1)] = data;
	}
}

READ8_MEMBER(pce_cd_device::bram_r)
{
	return m_bram[(offset & (PCE_BRAM_SIZE - 1)) + m_bram_locked * PCE_BRAM_SIZE];
}

void pce_cd_device::set_adpcm_ram_byte(UINT8 val)
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
		m_cdda->set_volume(0.0);
		m_cdda_fadeout_timer->adjust(attotime::never);
	}
	else
	{
		m_cdda->set_volume(m_cdda_volume);
		m_cdda_fadeout_timer->adjust(attotime::from_usec(param), param);
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::cdda_fadein_callback)
{
	m_cdda_volume += 0.1;

	if (m_cdda_volume >= 100.0)
	{
		m_cdda_volume = 100.0;
		m_cdda->set_volume(100.0);
		m_cdda_fadein_timer->adjust(attotime::never);
	}
	else
	{
		m_cdda->set_volume(m_cdda_volume);
		m_cdda_fadein_timer->adjust(attotime::from_usec(param), param);
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::adpcm_fadeout_callback)
{
	m_adpcm_volume -= 0.1;

	if (m_adpcm_volume <= 0)
	{
		m_adpcm_volume = 0.0;
		m_msm->set_volume(0.0);
		m_adpcm_fadeout_timer->adjust(attotime::never);
	}
	else
	{
		m_msm->set_volume(m_adpcm_volume);
		m_adpcm_fadeout_timer->adjust(attotime::from_usec(param), param);
	}
}

TIMER_CALLBACK_MEMBER(pce_cd_device::adpcm_fadein_callback)
{
	m_adpcm_volume += 0.1;

	if (m_adpcm_volume >= 100.0)
	{
		m_adpcm_volume = 100.0;
		m_msm->set_volume(100.0);
		m_adpcm_fadein_timer->adjust(attotime::never);
	}
	else
	{
		m_msm->set_volume(m_adpcm_volume);
		m_adpcm_fadein_timer->adjust(attotime::from_usec(param), param);
	}
}


WRITE8_MEMBER(pce_cd_device::intf_w)
{
	logerror("%04X: write to CD interface offset %02X, data %02X\n", space.device().safe_pc(), offset, data);

	switch (offset & 0xf)
	{
		case 0x00:  /* CDC status */
			/* select device (which bits??) */
			m_scsi_SEL = 1;
			update();
			m_scsi_SEL = 0;
			m_adpcm_dma_timer->adjust(attotime::never); // stop ADPCM DMA here
			/* any write here clears CD transfer irqs */
			set_irq_line(0x70, CLEAR_LINE);
			break;
		case 0x01:  /* CDC command / status / data */
			break;
		case 0x02:  /* ADPCM / CD control / IRQ enable/disable */
			/* bit 6 - transfer ready irq */
			/* bit 5 - transfer done irq */
			/* bit 4 - BRAM irq? */
			/* bit 3 - ADPCM FULL irq */
			/* bit 2 - ADPCM HALF irq */
			m_scsi_ACK = data & 0x80;
			/* Update mask register now otherwise it won't catch the irq enable/disable change */
			m_regs[0x02] = data;
			/* Don't set or reset any irq lines, but just verify the current state */
			set_irq_line(0, 0);
			break;
		case 0x03:  /* BRAM lock / CD status / IRQ - Read Only register */
			break;
		case 0x04:  /* CD reset */
			m_scsi_RST = data & 0x02;
			break;
		case 0x05:  /* Convert PCM data / PCM data */
		case 0x06:  /* PCM data */
			break;
		case 0x07:  /* BRAM unlock / CD status */
			if (data & 0x80)
				m_bram_locked = 0;
			break;
		case 0x08:  /* ADPCM address (LSB) / CD data */
			break;
		case 0x09:  /* ADPCM address (MSB) */
			break;
		case 0x0A:  /* ADPCM RAM data port */
			set_adpcm_ram_byte(data);
			break;
		case 0x0B:  /* ADPCM DMA control */
			if (data & 0x03)
			{
				/* Start CD to ADPCM transfer */
				m_adpcm_dma_timer->adjust(attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND * 2048), 0, attotime::from_hz(PCE_CD_DATA_FRAMES_PER_SECOND * 2048));
				m_regs[0x0c] |= 4;
			}
			break;
		case 0x0C:  /* ADPCM status */
			break;
		case 0x0D:  /* ADPCM address control */
			if ((m_regs[0x0D] & 0x80) && !(data & 0x80)) // ADPCM reset
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

			if ((data & 0x40) && ((m_regs[0x0D] & 0x40) == 0)) // ADPCM play
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
				/* used by Buster Bros to cancel an in-flight sample */
				adpcm_stop(0);
				m_msm->reset_w(1);
			}

			m_msm_repeat = BIT(data, 5);

			if (data & 0x10) //ADPCM set length
			{
				m_adpcm_length = (m_regs[0x09] << 8) | m_regs[0x08];
			}
			if (data & 0x08) //ADPCM set read address
			{
				m_adpcm_read_ptr = (m_regs[0x09] << 8) | m_regs[0x08];
				m_adpcm_read_buf = 2;
			}
			if ((data & 0x02) == 0x02) //ADPCM set write address
			{
				m_adpcm_write_ptr = (m_regs[0x09] << 8) | m_regs[0x08];
				m_adpcm_write_buf = data & 1;
			}
			break;
		case 0x0E:  /* ADPCM playback rate */
			m_adpcm_clock_divider = 0x10 - (data & 0x0f);
			m_msm->change_clock_w((PCE_CD_CLOCK / 6) / m_adpcm_clock_divider);
			break;
		case 0x0F:  /* ADPCM and CD audio fade timer */
			/* TODO: timers needs HW tests */
			if (m_regs[0xf] != data)
			{
				switch (data & 0xf)
				{
					case 0x00: //CD-DA / ADPCM enable (100 msecs)
						m_cdda_volume = 0.0;
						m_cdda_fadein_timer->adjust(attotime::from_usec(100), 100);
						m_adpcm_volume = 0.0;
						m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
						m_cdda_fadeout_timer->adjust(attotime::never);
						m_adpcm_fadeout_timer->adjust(attotime::never);
						break;
					case 0x01: //CD-DA enable (100 msecs)
						m_cdda_volume = 0.0;
						m_cdda_fadein_timer->adjust(attotime::from_usec(100), 100);
						m_cdda_fadeout_timer->adjust(attotime::never);
						break;
					case 0x08: //CD-DA short (1500 msecs) fade out / ADPCM enable
						m_cdda_volume = 100.0;
						m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
						m_adpcm_volume = 0.0;
						m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
						m_cdda_fadein_timer->adjust(attotime::never);
						m_adpcm_fadeout_timer->adjust(attotime::never);
						break;
					case 0x09: //CD-DA long (5000 msecs) fade out
						m_cdda_volume = 100.0;
						m_cdda_fadeout_timer->adjust(attotime::from_usec(5000), 5000);
						m_cdda_fadein_timer->adjust(attotime::never);
						break;
					case 0x0a: //ADPCM long (5000 msecs) fade out
						m_adpcm_volume = 100.0;
						m_adpcm_fadeout_timer->adjust(attotime::from_usec(5000), 5000);
						m_adpcm_fadein_timer->adjust(attotime::never);
						break;
					case 0x0c: //CD-DA short (1500 msecs) fade out / ADPCM enable
						m_cdda_volume = 100.0;
						m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
						m_adpcm_volume = 0.0;
						m_adpcm_fadein_timer->adjust(attotime::from_usec(100), 100);
						m_cdda_fadein_timer->adjust(attotime::never);
						m_adpcm_fadeout_timer->adjust(attotime::never);
						break;
					case 0x0d: //CD-DA short (1500 msecs) fade out
						m_cdda_volume = 100.0;
						m_cdda_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
						m_cdda_fadein_timer->adjust(attotime::never);
						break;
					case 0x0e: //ADPCM short (1500 msecs) fade out
						m_adpcm_volume = 100.0;
						m_adpcm_fadeout_timer->adjust(attotime::from_usec(1500), 1500);
						m_adpcm_fadein_timer->adjust(attotime::never);
						break;
					default:
						popmessage("CD-DA / ADPCM Fade effect mode %02x, contact MESSdev",data & 0x0f);
						break;
				}
			}
			break;
		default:
			return;
	}
	m_regs[offset & 0xf] = data;
}

TIMER_CALLBACK_MEMBER(pce_cd_device::clear_ack)
{
	update();
	m_scsi_ACK = 0;
	update();
	if (m_scsi_CD)
	{
		m_regs[0x0B] &= 0xFC;
	}
}

UINT8 pce_cd_device::get_cd_data_byte()
{
	UINT8 data = m_regs[0x01];
	if (m_scsi_REQ && !m_scsi_ACK && !m_scsi_CD)
	{
		if (m_scsi_IO)
		{
			m_scsi_ACK = 1;
			machine().scheduler().timer_set(m_maincpu->cycles_to_attotime(15), timer_expired_delegate(FUNC(pce_cd_device::clear_ack),this));
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

		m_regs[0x0c] &= ~4;
	}
}

UINT8 pce_cd_device::get_adpcm_ram_byte()
{
	if (m_adpcm_read_buf > 0)
	{
		m_adpcm_read_buf--;
		return 0;
	}
	else
	{
		UINT8 res;

		res = m_adpcm_ram[m_adpcm_read_ptr];
		m_adpcm_read_ptr = ((m_adpcm_read_ptr + 1) & 0xffff);

		return res;
	}
}

READ8_MEMBER(pce_cd_device::intf_r)
{
	UINT8 data = m_regs[offset & 0x0F];

	logerror("%04X: read from CD interface offset %02X\n", space.device().safe_pc(), offset );

	switch (offset & 0xf)
	{
		case 0x00:  /* CDC status */
			data &= 0x07;
			data |= m_scsi_BSY ? 0x80 : 0;
			data |= m_scsi_REQ ? 0x40 : 0;
			data |= m_scsi_MSG ? 0x20 : 0;
			data |= m_scsi_CD  ? 0x10 : 0;
			data |= m_scsi_IO  ? 0x08 : 0;
			break;
		case 0x01:  /* CDC command / status / data */
			break;
		case 0x02:  /* ADPCM / CD control */
			break;
		case 0x03:  /* BRAM lock / CD status */
			/* bit 4 set when CD motor is on */
			/* bit 2 set when less than half of the ADPCM data is remaining ?? */
			m_bram_locked = 1;
			data = data & 0x6E;
			data |= (m_cd_motor_on ? 0x10 : 0);
			m_regs[0x03] ^= 0x02;          /* TODO: get rid of this hack */
			break;
		case 0x04:  /* CD reset */
			break;
		case 0x05:  /* Convert PCM data / PCM data */
			data = m_cdda->get_channel_volume((m_regs[0x03] & 2) ? 0 : 1) & 0xff;
			break;
		case 0x06:  /* PCM data */
			data = m_cdda->get_channel_volume((m_regs[0x03] & 2) ? 0 : 1) >> 8;
			break;
		case 0x07:  /* BRAM unlock / CD status */
			data = (m_bram_locked ? (data & 0x7f) : (data | 0x80));
			break;
		case 0x08:  /* ADPCM address (LSB) / CD data */
			data = get_cd_data_byte();
			break;
		case 0x0A:  /* ADPCM RAM data port */
			data = get_adpcm_ram_byte();
			break;
		case 0x0B:  /* ADPCM DMA control */
			break;
		case 0x0C:  /* ADPCM status */
			break;
		case 0x0D:  /* ADPCM address control */
			break;
			/* These are read-only registers */
		case 0x09:  /* ADPCM address (MSB) */
		case 0x0E:  /* ADPCM playback rate */
		case 0x0F:  /* ADPCM and CD audio fade timer */
			return 0;
		default:
			data = 0xFF;
			break;
	}

	return data;
}


/*

PC Engine Arcade Card emulation

*/

READ8_MEMBER(pce_cd_device::acard_r)
{
	UINT8 r_num;

	if ((offset & 0x2e0) == 0x2e0)
	{
		switch (offset & 0x2ef)
		{
			case 0x2e0: return (m_acard_shift >> 0)  & 0xff;
			case 0x2e1: return (m_acard_shift >> 8)  & 0xff;
			case 0x2e2: return (m_acard_shift >> 16) & 0xff;
			case 0x2e3: return (m_acard_shift >> 24) & 0xff;
			case 0x2e4: return (m_acard_shift_reg);
			case 0x2e5: return m_acard_latch;
			case 0x2ee: return 0x10;
			case 0x2ef: return 0x51;
		}

		return 0;
	}

	r_num = (offset & 0x30) >> 4;

	switch (offset & 0x0f)
	{
		case 0x00:
		case 0x01:
		{
			UINT8 res;
			if (m_acard_ctrl[r_num] & 2)
				res = m_acard_ram[(m_acard_base_addr[r_num] + m_acard_addr_offset[r_num]) & 0x1fffff];
			else
				res = m_acard_ram[m_acard_base_addr[r_num] & 0x1fffff];

			if (m_acard_ctrl[r_num] & 0x1)
			{
				if (m_acard_ctrl[r_num] & 0x10)
				{
					m_acard_base_addr[r_num] += m_acard_addr_inc[r_num];
					m_acard_base_addr[r_num] &= 0xffffff;
				}
				else
				{
					m_acard_addr_offset[r_num] += m_acard_addr_inc[r_num];
				}
			}

			return res;
		}
		case 0x02: return (m_acard_base_addr[r_num] >> 0) & 0xff;
		case 0x03: return (m_acard_base_addr[r_num] >> 8) & 0xff;
		case 0x04: return (m_acard_base_addr[r_num] >> 16) & 0xff;
		case 0x05: return (m_acard_addr_offset[r_num] >> 0) & 0xff;
		case 0x06: return (m_acard_addr_offset[r_num] >> 8) & 0xff;
		case 0x07: return (m_acard_addr_inc[r_num] >> 0) & 0xff;
		case 0x08: return (m_acard_addr_inc[r_num] >> 8) & 0xff;
		case 0x09: return m_acard_ctrl[r_num];
		default:   return 0;
	}
}

WRITE8_MEMBER(pce_cd_device::acard_w)
{
	UINT8 w_num;

	if ((offset & 0x2e0) == 0x2e0)
	{
		switch (offset & 0x0f)
		{
			case 0: m_acard_shift = (data & 0xff) | (m_acard_shift & 0xffffff00); break;
			case 1: m_acard_shift = (data << 8)   | (m_acard_shift & 0xffff00ff); break;
			case 2: m_acard_shift = (data << 16)  | (m_acard_shift & 0xff00ffff); break;
			case 3: m_acard_shift = (data << 24)  | (m_acard_shift & 0x00ffffff); break;
			case 4:
			{
				m_acard_shift_reg = data & 0x0f;

				if (m_acard_shift_reg != 0)
				{
					m_acard_shift = (m_acard_shift_reg < 8) ?
					(m_acard_shift << m_acard_shift_reg)
					: (m_acard_shift >> (16 - m_acard_shift_reg));
				}
			}
				break;
			case 5: m_acard_latch = data; break;
		}
	}
	else
	{
		w_num = (offset & 0x30) >> 4;

		switch (offset & 0x0f)
		{
			case 0x00:
			case 0x01:
				if (m_acard_ctrl[w_num] & 2)
					m_acard_ram[(m_acard_base_addr[w_num] + m_acard_addr_offset[w_num]) & 0x1fffff] = data;
				else
					m_acard_ram[m_acard_base_addr[w_num] & 0x1FFFFF] = data;

				if (m_acard_ctrl[w_num] & 0x1)
				{
					if (m_acard_ctrl[w_num] & 0x10)
					{
						m_acard_base_addr[w_num] += m_acard_addr_inc[w_num];
						m_acard_base_addr[w_num] &= 0xffffff;
					}
					else
					{
						m_acard_addr_offset[w_num] += m_acard_addr_inc[w_num];
					}
				}

				break;

			case 0x02: m_acard_base_addr[w_num] = (data & 0xff) | (m_acard_base_addr[w_num] & 0xffff00);  break;
			case 0x03: m_acard_base_addr[w_num] = (data << 8) | (m_acard_base_addr[w_num] & 0xff00ff);        break;
			case 0x04: m_acard_base_addr[w_num] = (data << 16) | (m_acard_base_addr[w_num] & 0x00ffff);   break;
			case 0x05: m_acard_addr_offset[w_num] = (data & 0xff) | (m_acard_addr_offset[w_num] & 0xff00);    break;
			case 0x06:
				m_acard_addr_offset[w_num] = (data << 8) | (m_acard_addr_offset[w_num] & 0x00ff);

				if ((m_acard_ctrl[w_num] & 0x60) == 0x40)
				{
					m_acard_base_addr[w_num] += m_acard_addr_offset[w_num] + ((m_acard_ctrl[w_num] & 0x08) ? 0xff0000 : 0);
					m_acard_base_addr[w_num] &= 0xffffff;
				}
				break;
			case 0x07: m_acard_addr_inc[w_num] = (data & 0xff) | (m_acard_addr_inc[w_num] & 0xff00);      break;
			case 0x08: m_acard_addr_inc[w_num] = (data << 8) | (m_acard_addr_inc[w_num] & 0x00ff);            break;
			case 0x09: m_acard_ctrl[w_num] = data & 0x7f;                                              break;
			case 0x0a:
				if ((m_acard_ctrl[w_num] & 0x60) == 0x60)
				{
					m_acard_base_addr[w_num] += m_acard_addr_offset[w_num];
					m_acard_base_addr[w_num] &= 0xffffff;
				}
				break;
		}
	}
}
