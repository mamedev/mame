// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Philips CD-i Digital Video Cartridge (DVC) - VMPEG

    See cdidvc.h for the memory layout.

    The register behaviour, the command handling, the MPEG-1 system stream
    demuxing and the frame display pacing were worked out using the
    CDi_MiSTer FPGA core by Andre Zeps as the hardware reference, in
    particular its notes in doc/dvc.md:
    https://github.com/MiSTer-devel/CDi_MiSTer

    Emulation notes:

    - The cartridge's MPEG decoder chips are undocumented, so decoding is
      done with pl_mpeg (3rdparty/pl_mpeg).
    - The 30 MHz and 45 kHz counters are derived from machine time, with
      emu_timers for the events, rather than counted tick by tick.
    - The cartridge distinguishes the vsync pulse from the vblank interval.
      We only get vblank begin/end from the screen, so vblank begin stands in
      for "vsync trailing edge" and vblank end for "picture starts display".
      The ordering and the one-frame latency between the two are preserved,
      which is what the driver actually observes.

*******************************************************************************/

#include "emu.h"
#include "cdidvc.h"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg/pl_mpeg.h"

#include <algorithm>
#include <cmath>

#define LOG_REGS_R    (1U << 1)
#define LOG_REGS_W    (1U << 2)
#define LOG_FMA       (1U << 3)
#define LOG_FMV       (1U << 4)
#define LOG_DEMUX     (1U << 5)
#define LOG_DMA       (1U << 6)
#define LOG_IRQ       (1U << 7)
#define LOG_VIDEO     (1U << 8)
#define LOG_AUDIO     (1U << 9)

// LOG_FMA | LOG_FMV | LOG_VIDEO | LOG_AUDIO show the commands, decoded pictures
// and interrupts without the per-register noise of LOG_REGS_R | LOG_REGS_W.
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(CDI_DVC, cdi_dvc_device, "cdi_dvc", "CD-i Digital Video Cartridge")
DEFINE_DEVICE_TYPE(CDI_DVC_SLOT, cdi_dvc_slot_device, "cdi_dvc_slot", "CD-i Digital Video Cartridge slot")

//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace {

// The FMA/FMV decoder clock ticks at 45 kHz, the same tick for both.
constexpr int DCLK_HZ = 45000;

// Frame display is paced in 30 MHz system clock ticks: 1200000 per picture
// at 25 Hz.
constexpr int FRAME_CLOCK_HZ = 30000000;

// the video starts this many decoder clock ticks after the play command
constexpr int PLAY_DELAY_TICKS = 1000;
// and pauses this many ticks after the pause command
constexpr int PAUSE_DELAY_TICKS = 100;

// the display rate the cartridge reports for PAL and NTSC
constexpr uint16_t DISPLAY_RATE_PAL  = 0x0708;
constexpr uint16_t DISPLAY_RATE_NTSC = 0x05dc;

// The decoded picture queue.  A real VMPEG only has RAM for about three
// pictures; queuing more keeps the decode rate steady.
constexpr size_t MAX_PICTURES = 8;

// pl_mpeg needs a decent run-up of elementary stream before it can hand back a
// picture, because it has to find the *next* picture start code first.
constexpr size_t VIDEO_BUFFER_SIZE = 512 * 1024;
constexpr size_t AUDIO_BUFFER_SIZE = 64 * 1024;

// A CD-i digital video stream carries MPEG-1 layer II at 224 kbit/s and
// 44.1 kHz, so one 1152 sample frame is about 731 bytes.  Round up: below this
// much undecoded elementary stream the decoder genuinely cannot produce
// another frame.
constexpr size_t MP2_FRAME_BYTES = 1024;

// Size of the FMV input stream FIFO, which the "request for bits" status
// reflects.
constexpr size_t VIDEO_FIFO_FULL = 32 * 1024;

// Picture periods in 30 MHz ticks, indexed by the MPEG-1 picture_rate code.
constexpr uint32_t FRAME_PERIOD_30MHZ[16] =
{
	1200000, // 0: forbidden, treat as 25 Hz
	1251251, // 1: 23.976 Hz
	1250000, // 2: 24 Hz
	1200000, // 3: 25 Hz
	1001001, // 4: 29.97 Hz
	1000000, // 5: 30 Hz
	600000,  // 6: 50 Hz
	500500,  // 7: 59.94 Hz
	500000,  // 8: 60 Hz
	1200000, 1200000, 1200000, 1200000, 1200000, 1200000, 1200000
};

// Same rates expressed as a picture period in 90 kHz ticks, which is what
// GEN_PICT_RATE @ 0xe040a8 reports.
constexpr uint16_t FRAME_PERIOD_90KHZ[16] =
{
	3600, 3754, 3750, 3600, 3003, 3000, 1800, 1502,
	1500, 3600, 3600, 3600, 3600, 3600, 3600, 3600
};

// MPEG system clock values are 33 bits, so the difference between two of them
// wraps and is signed.  Crime Patrol's streams start
// with an SCR just below zero, 2^33 - 33746, against a first PTS of 0: that
// is 0.375 s of pre-roll, not a start time 95443 s in the past.
int64_t mpeg_timestamp_diff(int64_t a, int64_t b)
{
	int64_t d = (a - b) & 0x1ffffffffLL;
	if (d & 0x100000000LL)
		d -= 0x200000000LL;
	return d;
}

} // anonymous namespace

//**************************************************************************
//  pl_mpeg GLUE
//**************************************************************************

struct cdi_dvc_device::decoder_state
{
	plm_buffer_t *video_buffer = nullptr;
	plm_video_t *video = nullptr;
	plm_buffer_t *audio_buffer = nullptr;
	plm_audio_t *audio = nullptr;

	~decoder_state()
	{
		// the decoders were created with destroy_when_done, so they own their
		// buffers and destroying them is enough
		if (video)
			plm_video_destroy(video);
		else if (video_buffer)
			plm_buffer_destroy(video_buffer);

		if (audio)
			plm_audio_destroy(audio);
		else if (audio_buffer)
			plm_buffer_destroy(audio_buffer);
	}
};

//**************************************************************************
//  ROM
//**************************************************************************

// The cartridge ROM holds the OS-9 FMV/FMA driver the base machine loads.
ROM_START( cdi_dvc )
	// Philips CD-i DVC card 22ER9141
	ROM_REGION16_BE(0x20000, "vmpeg", ROMREGION_ERASEFF)
	ROMX_LOAD( "fmv ffd9 p7308 r4.1 vmpeg.bin", 0x00000, 0x10000, CRC(30ba9273) SHA1(d8adca0627b356ced6131b9458ac1175e43e6548), ROM_SKIP(1) )
	ROMX_LOAD( "fmv 4ba9 p7307 r4.1 vmpeg.bin", 0x00001, 0x10000, CRC(623edb1f) SHA1(4c6b11e28ad4c2f5c2e439f7910a783e0a79d1a9), ROM_SKIP(1) )
ROM_END

const tiny_rom_entry *cdi_dvc_device::device_rom_region() const
{
	return ROM_NAME( cdi_dvc );
}

//**************************************************************************
//  CONSTRUCTION
//**************************************************************************

cdi_dvc_device::cdi_dvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_DVC, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_intreq_cb(*this)
	, m_scc(*this, finder_base::DUMMY_TAG)
	, m_rom(*this, "vmpeg")
	, m_screen(*this, finder_base::DUMMY_TAG)
{
}

cdi_dvc_device::~cdi_dvc_device()
{
}

void cdi_dvc_device::device_start()
{
	m_memory_space = &m_scc->space(AS_PROGRAM);

	m_ram = std::make_unique<uint16_t []>(0x80000 / 2);
	std::fill_n(m_ram.get(), 0x80000 / 2, 0);

	m_dec = std::make_unique<decoder_state>();
	m_dec->video_buffer = plm_buffer_create_with_capacity(VIDEO_BUFFER_SIZE);
	m_dec->video = plm_video_create_with_buffer(m_dec->video_buffer, TRUE);
	m_dec->audio_buffer = plm_buffer_create_with_capacity(AUDIO_BUFFER_SIZE);
	m_dec->audio = plm_audio_create_with_buffer(m_dec->audio_buffer, TRUE);

	m_stream = stream_alloc(0, 2, 44100);

	m_tim_timer = timer_alloc(FUNC(cdi_dvc_device::tim_tick), this);
	m_start_video_timer = timer_alloc(FUNC(cdi_dvc_device::start_video_tick), this);
	m_pause_video_timer = timer_alloc(FUNC(cdi_dvc_device::pause_video_tick), this);
	m_frame_timer = timer_alloc(FUNC(cdi_dvc_device::frame_period_tick), this);
	m_audio_timer = timer_alloc(FUNC(cdi_dvc_device::audio_tick), this);

	m_picture_fifo.reserve(MAX_PICTURES);

	if (m_screen)
		m_screen->register_vblank_callback(vblank_state_delegate(&cdi_dvc_device::vblank_callback, this));

	save_pointer(NAME(m_ram), 0x80000 / 2);
	save_item(NAME(m_fma_cmd));
	save_item(NAME(m_fma_status));
	save_item(NAME(m_fma_isr));
	save_item(NAME(m_fma_ier));
	save_item(NAME(m_fma_ivec));
	save_item(NAME(m_fma_stream));
	save_item(NAME(m_fma_dspa));
	save_item(NAME(m_fma_dsp_enable));
	save_item(NAME(m_fma_dclkl_latch));
	save_item(NAME(m_fma_audio_header));
	save_item(NAME(m_fmv_isr));
	save_item(NAME(m_fmv_ier));
	save_item(NAME(m_fmv_ivec));
	save_item(NAME(m_fmv_syscmd));
	save_item(NAME(m_fmv_vidcmd));
	save_item(NAME(m_fmv_sysscr));
	save_item(NAME(m_fmv_dec_cmd));
	save_item(NAME(m_fmv_vdi_cmd));
	save_item(NAME(m_fmv_frame_rate));
	save_item(NAME(m_fmv_tcnt));
	save_item(NAME(m_fmv_stream));
	save_item(NAME(m_dclk_origin));
	save_item(NAME(m_fmv_dclk_offset));
	save_item(NAME(m_mpeg_ram_enabled));
	save_item(NAME(m_mpeg_ram_enable_cnt));
}

void cdi_dvc_device::device_reset()
{
	// power-on state of the cartridge
	m_dma_active = false;
	m_dma_for_fma = false;
	m_fma_cmd = 0;
	m_fma_dsp_enable = false;
	m_fma_ier = 0;
	m_fma_isr = 0;
	m_fma_ivec = 0;
	m_fma_status = 0;
	m_fma_stream = 0;
	m_fma_dspa = 0;
	m_fma_dclkl_latch = 0;
	m_fma_audio_header = 0;
	m_pending_fma_stream_change = false;

	m_fmv_dec_cmd = 0;
	m_fmv_dsp_enable = false;
	m_fmv_frame_rate = 0;
	m_fmv_ier = 0;
	m_fmv_isr = 0;
	m_fmv_ivec = 0;
	m_fmv_playback_active = false;
	m_fmv_decoder_active = false;
	m_fmv_stream = 0;
	m_fmv_syscmd = 0;
	m_fmv_vidcmd = 0;
	m_fmv_sysscr = 0;
	m_fmv_vdi_cmd = 0;
	m_fmv_slow_motion = 0;
	m_fmv_tcnt = 56 - 1;

	m_image_height = 0;
	m_image_rt = 0;
	m_image_width = 0;

	m_mpeg_ram_enabled = false;
	m_mpeg_ram_enable_cnt = 0;

	m_video_ctrl_decoder_offset_x = 0;
	m_video_ctrl_decoder_offset_y = 0;
	m_video_ctrl_window_height = 0;
	m_video_ctrl_window_width = 0;
	m_video_ctrl_x_active = 0;
	m_video_ctrl_x_display = 0;
	m_video_ctrl_x_offset = 0;
	m_video_ctrl_y_active = 0;
	m_video_ctrl_y_display = 0;
	m_video_ctrl_y_offset = 0;

	m_latched_display_offset_x = 0;
	m_latched_display_offset_y = 0;
	m_latched_window_offset_x = 0;
	m_latched_window_offset_y = 0;
	m_latched_window_width = 0;
	m_latched_window_height = 0;

	m_register_update_latch = false;
	m_register_update_scroll = false;
	m_show_video = false;

	m_dclk_origin = machine().time();
	m_fmv_dclk_offset = 0;
	m_syscr_written = false;

	m_fma_demux.reset();
	m_fmv_demux.reset();
	m_es_batch.clear();
	m_es_batch_for_fma = false;
	m_audio_header_shift = 0;
	m_video_startcode_shift = 0;
	m_audio_head = 0;
	m_fma_scr_start_valid = false;
	m_fma_scr_start_time = 0;

	clear_video_fifo();

	audio_clear();

	m_intreq_state = false;
	m_intreq_cb(0);

	// The TIM interrupt has to land at exactly 100.446428... Hz for the
	// driver's SCR bookkeeping to add up: 45000 / ((55 + 1) * 8) = 100.446...
	m_tim_timer->adjust(attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ),
			0, attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ));

	m_audio_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

//**************************************************************************
//  CLOCKS
//**************************************************************************

uint32_t cdi_dvc_device::fma_dclk() const
{
	return uint32_t((machine().time() - m_dclk_origin).as_ticks(DCLK_HZ));
}

uint32_t cdi_dvc_device::fmv_dclk() const
{
	return fma_dclk() + m_fmv_dclk_offset;
}

//**************************************************************************
//  MPEG-1 SYSTEM STREAM DEMUXER
//**************************************************************************

void cdi_dvc_device::demuxer::reset()
{
	state = DEMUX_IDLE;
	packet_body = false;
	length_decreasing = false;
	length = 0;
	dts_present = false;
	scr = pts = dts = 0;
	scr_temp = pts_temp = dts_temp = 0;
	scr_updated = pts_updated = dts_updated = false;
	program_end = false;
}

// One byte through the demuxer state machine.  `packet_body` says whether the
// *next* byte belongs to an elementary stream payload, exactly as the Verilog
// registered output does.
void cdi_dvc_device::demux_byte(demuxer &dmx, uint8_t data, uint8_t stream_filter)
{
	dmx.scr_updated = false;
	dmx.pts_updated = false;
	dmx.dts_updated = false;
	dmx.program_end = false;

	if (dmx.length_decreasing)
	{
		if (dmx.length == 1)
		{
			dmx.length_decreasing = false;
			dmx.packet_body = false;
		}
		dmx.length--;
	}

	switch (dmx.state)
	{
	case DEMUX_PACK5:
		dmx.state = DEMUX_IDLE;
		dmx.scr = dmx.scr_temp;
		dmx.scr_updated = true;
		break;

	case DEMUX_PACK4:
		dmx.state = DEMUX_PACK5;
		dmx.scr_temp = (dmx.scr_temp & ~int64_t(0x7f)) | ((data >> 1) & 0x7f);
		break;

	case DEMUX_PACK3:
		dmx.state = DEMUX_PACK4;
		dmx.scr_temp = (dmx.scr_temp & ~(int64_t(0xff) << 7)) | (int64_t(data) << 7);
		break;

	case DEMUX_PACK2:
		dmx.state = DEMUX_PACK3;
		dmx.scr_temp = (dmx.scr_temp & ~(int64_t(0x7f) << 15)) | (int64_t((data >> 1) & 0x7f) << 15);
		break;

	case DEMUX_PACK1:
		dmx.state = DEMUX_PACK2;
		dmx.scr_temp = (dmx.scr_temp & ~(int64_t(0xff) << 22)) | (int64_t(data) << 22);
		break;

	case DEMUX_PACK0:
		dmx.state = DEMUX_PACK1;
		dmx.scr_temp = (dmx.scr_temp & ~(int64_t(0x7) << 30)) | (int64_t((data >> 1) & 0x7) << 30);
		break;

	case DEMUX_PES8:
		// only reached when a PTS was present; a DTS cannot occur without one
		dmx.state = DEMUX_IDLE;
		dmx.pts = dmx.pts_temp;
		dmx.pts_updated = true;
		if (dmx.dts_present)
		{
			dmx.dts = dmx.dts_temp;
		}
		else
		{
			// no DTS.  VMPEG uses the PTS instead, and so does ffprobe.
			dmx.dts = dmx.pts_temp;
		}
		dmx.dts_updated = true;
		LOGMASKED(LOG_DEMUX, "PES pts=%d dts=%d\n", int(dmx.pts), int(dmx.dts));
		break;

	case DEMUX_PES_DTS4:
		if (BIT(data, 0))
		{
			dmx.dts_temp = (dmx.dts_temp & ~int64_t(0x7f)) | ((data >> 1) & 0x7f);
			dmx.packet_body = true;
			dmx.state = DEMUX_PES8;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES_DTS3:
		dmx.state = DEMUX_PES_DTS4;
		dmx.dts_temp = (dmx.dts_temp & ~(int64_t(0xff) << 7)) | (int64_t(data) << 7);
		break;

	case DEMUX_PES_DTS2:
		if (BIT(data, 0))
		{
			dmx.dts_temp = (dmx.dts_temp & ~(int64_t(0x7f) << 15)) | (int64_t((data >> 1) & 0x7f) << 15);
			dmx.state = DEMUX_PES_DTS3;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES_DTS1:
		dmx.state = DEMUX_PES_DTS2;
		dmx.dts_temp = (dmx.dts_temp & ~(int64_t(0xff) << 22)) | (int64_t(data) << 22);
		break;

	case DEMUX_PES_DTS0:
		if ((data & 0xf1) == 0x11)
		{
			dmx.dts_temp = (dmx.dts_temp & ~(int64_t(0x7) << 30)) | (int64_t((data >> 1) & 0x7) << 30);
			dmx.state = DEMUX_PES_DTS1;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES7:
		if (BIT(data, 0))
		{
			dmx.pts_temp = (dmx.pts_temp & ~int64_t(0x7f)) | ((data >> 1) & 0x7f);
			if (dmx.dts_present)
			{
				dmx.state = DEMUX_PES_DTS0;
			}
			else
			{
				dmx.packet_body = true;
				dmx.state = DEMUX_PES8;
			}
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES6:
		dmx.state = DEMUX_PES7;
		dmx.pts_temp = (dmx.pts_temp & ~(int64_t(0xff) << 7)) | (int64_t(data) << 7);
		break;

	case DEMUX_PES5:
		if (BIT(data, 0))
		{
			dmx.pts_temp = (dmx.pts_temp & ~(int64_t(0x7f) << 15)) | (int64_t((data >> 1) & 0x7f) << 15);
			dmx.state = DEMUX_PES6;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES4:
		dmx.state = DEMUX_PES5;
		dmx.pts_temp = (dmx.pts_temp & ~(int64_t(0xff) << 22)) | (int64_t(data) << 22);
		break;

	case DEMUX_PES3:
		// second byte of the STD buffer size, ignored
		dmx.state = DEMUX_PES2;
		break;

	case DEMUX_PES2:
		if ((data & 0xf1) == 0x21)
		{
			// PTS only
			dmx.pts_temp = (dmx.pts_temp & ~(int64_t(0x7) << 30)) | (int64_t((data >> 1) & 0x7) << 30);
			dmx.dts_present = false;
			dmx.state = DEMUX_PES4;
		}
		else if ((data & 0xf1) == 0x31)
		{
			// PTS and DTS
			dmx.pts_temp = (dmx.pts_temp & ~(int64_t(0x7) << 30)) | (int64_t((data >> 1) & 0x7) << 30);
			dmx.dts_present = true;
			dmx.state = DEMUX_PES4;
		}
		else if (data == 0x0f)
		{
			// neither
			dmx.state = DEMUX_IDLE;
			dmx.packet_body = true;
		}
		else if ((data & 0xc0) == 0x40)
		{
			// STD buffer size
			dmx.state = DEMUX_PES3;
		}
		else if (data == 0xff)
		{
			// stuffing byte
			dmx.state = DEMUX_PES2;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_PES1:
		dmx.state = DEMUX_PES2;
		dmx.length = (dmx.length & 0xff00) | data;
		dmx.length_decreasing = true;
		break;

	case DEMUX_PES0:
		dmx.state = DEMUX_PES1;
		dmx.length = (dmx.length & 0x00ff) | (uint16_t(data) << 8);
		break;

	case DEMUX_MAGIC_MATCH:
		if (data == 0xba)
		{
			dmx.state = DEMUX_PACK0;
		}
		else if ((data & 0xf0) == 0xc0 || (data & 0xf0) == 0xe0)
		{
			// audio (0xc0) or video (0xe0) elementary stream
			dmx.state = ((data & 0x0f) == stream_filter) ? DEMUX_PES0 : DEMUX_IDLE;
		}
		else if (data == 0xb9)
		{
			dmx.program_end = true;
			dmx.state = DEMUX_IDLE;
		}
		else
		{
			dmx.state = DEMUX_IDLE;
		}
		break;

	case DEMUX_MAGIC2:
		if (data == 0x01)
			dmx.state = DEMUX_MAGIC_MATCH;
		else if (data == 0x00)
			dmx.state = DEMUX_MAGIC2;
		else
			dmx.state = DEMUX_IDLE;
		break;

	case DEMUX_MAGIC0:
		dmx.state = (data == 0x00) ? DEMUX_MAGIC2 : DEMUX_IDLE;
		break;

	case DEMUX_IDLE:
	default:
		if (data == 0x00 && !dmx.packet_body)
			dmx.state = DEMUX_MAGIC0;
		else
			dmx.state = DEMUX_IDLE;
		break;
	}
}

// A byte arriving from DMA or from a write to the XFER register.  It goes to
// whichever unit, FMA or FMV, the current transfer belongs to.
void cdi_dvc_device::feed_byte(uint8_t data)
{
	if (m_dma_for_fma)
	{
		const bool was_body = m_fma_demux.packet_body;
		demux_byte(m_fma_demux, data, m_fma_stream);

		if (was_body)
		{
			if (!m_es_batch_for_fma)
				flush_es_batch();
			m_es_batch_for_fma = true;
			m_es_batch.push_back(data);

			// FMA_MPEG_AUDIO_HEADER @ 0xe03014: keep the most recent frame
			// header the driver can read back
			m_audio_header_shift = (m_audio_header_shift << 8) | data;
			if ((m_audio_header_shift & 0xfff00000) == 0xfff00000)
				m_fma_audio_header = m_audio_header_shift;
		}

		if (m_fma_demux.pts_updated)
		{
			// the first PTS sets when audio playback starts
			if (!m_fma_scr_start_valid)
			{
				m_fma_scr_start_valid = true;
				m_fma_scr_start_time = int64_t(fma_dclk())
						+ (mpeg_timestamp_diff(m_fma_demux.pts, m_fma_demux.scr) >> 1);
			}
		}

		if (m_fma_demux.program_end)
		{
			// ISO end detected
			m_fma_isr |= FMA_EOI;
			m_fma_status |= FMA_EOI;
			update_intreq();
		}
	}
	else
	{
		const bool was_body = m_fmv_demux.packet_body;
		demux_byte(m_fmv_demux, data, m_fmv_stream);

		if (was_body)
		{
			if (m_es_batch_for_fma)
				flush_es_batch();
			m_es_batch_for_fma = false;
			m_es_batch.push_back(data);

			// pictures are counted as they enter the input FIFO;
			// PICS_IN_FIFO reports them before they decode
			m_video_startcode_shift = (m_video_startcode_shift << 8) | data;
			if (m_video_startcode_shift == 0x00000100)
			{
				m_pictures_in_input_fifo++;
				m_dts_fifo.push_back(m_next_picture_dts);
				m_next_picture_dts = 0;
			}
		}

		if (m_fmv_demux.dts_updated)
		{
			m_next_picture_dts = m_fmv_demux.dts;
			// GEN_VDI_CMD bit 14 flags the arrival of a DTS
			m_fmv_vdi_cmd |= 1 << 14;
		}

		if (m_fmv_demux.program_end)
		{
			// End of the program.  pl_mpeg holds its last reference frames
			// back until it knows the stream has ended, because it needs the
			// following picture start code to know the current picture is
			// complete.  Without telling it, those final pictures are never
			// emitted, the last PIC events never fire, and the driver waits
			// for them forever.  Observed as a hang at the end of the Philips
			// logo in 7th Guest and Brain Dead 13, both leaving exactly two
			// pictures stuck in the input count; Atlantis, which drains to
			// zero, plays on through six stream switches.
			flush_es_batch();
			plm_buffer_signal_end(m_dec->video_buffer);
			video_decode_pending();

			m_fmv_isr |= FMV_IRQ_EII;
			update_intreq();
		}
	}
}

//**************************************************************************
//  DMA
//**************************************************************************

void cdi_dvc_device::flush_es_batch()
{
	if (m_es_batch.empty())
		return;

	plm_buffer_write(m_es_batch_for_fma ? m_dec->audio_buffer : m_dec->video_buffer,
			m_es_batch.data(), m_es_batch.size());
	if (!m_es_batch_for_fma)
		m_video_es_written += m_es_batch.size();
	m_es_batch.clear();
}

// The cartridge takes words from the 68070's DMA channel 1 until done.
// MAME's SCC68070 has no peripheral DMA handshake, so we do what cdicdic.cpp
// does and drive channel 1 directly.
void cdi_dvc_device::run_dma(bool for_fma)
{
	auto &ch = m_scc->dma().channel[1];

	const uint32_t start = ch.memory_address_counter;
	const uint32_t count = ch.transfer_counter;

	LOGMASKED(LOG_DMA, "%s: DVC DMA %s: %08x, %04x words\n", machine().describe_context(),
			for_fma ? "FMA" : "FMV", start, count);

	m_dma_for_fma = for_fma;

	for (uint32_t i = 0; i < count; i++)
	{
		const uint16_t word = m_memory_space->read_word(start + i * 2);
		// each bus word carries the high byte first
		feed_byte(uint8_t(word >> 8));
		feed_byte(uint8_t(word));
	}

	ch.memory_address_counter += count * 2;

	flush_es_batch();

	// transfer finished: the DMA bit of the command register clears
	if (for_fma)
		m_fma_cmd &= ~0x8000;
	m_dma_active = false;

	if (for_fma)
		audio_decode_pending();
	else
		video_decode_pending();
}

//**************************************************************************
//  VIDEO DECODE
//**************************************************************************

void cdi_dvc_device::video_decode_pending()
{
	if (!m_fmv_dsp_enable)
		return;

	m_video_needs_data = false;
	while (m_picture_fifo.size() < MAX_PICTURES)
	{
		plm_frame_t *frame = plm_video_decode(m_dec->video);
		check_sequence_end();
		if (!frame)
		{
			// pl_mpeg needs more of the stream, see "request for bits"
			m_video_needs_data = true;
			break;
		}

		queue_picture(make_picture(frame));
	}
}

// ESI, raised when the decoder reaches the sequence end code.  The FMV driver
// appends 00 00 01 B7 to mark a stream end and keys off it; we were only ever
// raising EII from the program end code, so the driver was told the ISO had
// finished but never that the sequence had, and it stopped feeding without
// completing its end-of-stream handling.
void cdi_dvc_device::check_sequence_end()
{
	if (m_dec->video->cdi_saw_sequence_end)
	{
		m_dec->video->cdi_saw_sequence_end = 0;
		m_fmv_isr |= FMV_IRQ_ESI;
		LOGMASKED(LOG_VIDEO, "FMV sequence end\n");
		update_intreq();
	}
}

// Convert a frame pl_mpeg handed back, latching the decoder status registers
// as each picture is decoded.  The frame's planes are reused by
// the next decode, so this has to happen before decoding again.
template <typename Frame>
cdi_dvc_device::picture cdi_dvc_device::make_picture(const Frame *frame)
{
	m_decoder_width = uint16_t(frame->width);
	m_decoder_height = uint16_t(frame->height);
	m_decoder_timecode = pack_timecode(frame->cdi_timecode);

	const int rate_code = m_dec->video->cdi_picture_rate_code & 0x0f;
	m_decoder_frameperiod_rawhdr = uint8_t(rate_code);
	m_decoder_frameperiod_90khz = FRAME_PERIOD_90KHZ[rate_code];
	m_frame_period_30mhz = FRAME_PERIOD_30MHZ[rate_code];

	if (m_fmv_playback_active)
	{
		m_image_width = m_decoder_width;
		m_image_height = m_decoder_height;
		m_image_rt = m_decoder_frameperiod_rawhdr;
	}

	picture pic;
	pic.width = uint16_t(frame->width);
	pic.height = uint16_t(frame->height);
	// SYS_VSR @ 0xe0405c carries the temporal reference and picture type
	pic.video_status = uint8_t(((frame->cdi_picture_type & 0x03) << 6)
			| (frame->cdi_temporal_reference & 0x3f));
	pic.timecode = pack_timecode(frame->cdi_timecode);
	pic.frameperiod_rawhdr = uint8_t(rate_code);
	pic.first_intra_of_gop = frame->cdi_first_intra_of_gop != 0;
	pic.first_intra_of_seq = frame->cdi_first_intra_of_seq != 0;

	pic.rgb = std::make_unique<uint32_t []>(size_t(pic.width) * pic.height);

	// pl_mpeg hands back planar YCbCr; convert once here so the screen
	// update only has to index a buffer.
	std::vector<uint8_t> rgb(size_t(pic.width) * pic.height * 3);
	plm_frame_to_rgb(const_cast<plm_frame_t *>(frame), rgb.data(), int(pic.width) * 3);
	for (size_t i = 0; i < size_t(pic.width) * pic.height; i++)
	{
		pic.rgb[i] = 0xff000000
				| (uint32_t(rgb[i * 3 + 0]) << 16)
				| (uint32_t(rgb[i * 3 + 1]) << 8)
				| uint32_t(rgb[i * 3 + 2]);
	}

	LOGMASKED(LOG_VIDEO, "decoded picture %dx%d type %d tref %d\n",
			frame->width, frame->height, frame->cdi_picture_type,
			frame->cdi_temporal_reference);
	return pic;
}

void cdi_dvc_device::queue_picture(picture &&pic)
{
	m_picture_fifo.push_back(std::move(pic));
	if (m_pictures_in_input_fifo)
		m_pictures_in_input_fifo--;
}

// pl_mpeg only knows a picture is complete once the next picture start code
// has arrived, and it holds the last reference frame back until the next
// reference picture for reordering.  So the last one or two pictures before
// the stream stops never come out: a lone still never shows, and a stream
// that ends without a sequence end code loses its tail.  The real decoder
// finishes a picture on its last macroblock instead, and shows what it holds
// once playback is running with nothing left to display.
//
// Do the same when the display has run dry with pictures still inside the
// decoder: decode as if the stream had ended.  If that runs out of bits, or a
// picture stops short of its last macroblock, the data has not all arrived,
// so put the decoder back exactly as it was and wait for more.  Only retried
// once more bytes have been written.
void cdi_dvc_device::video_flush_stalled()
{
	if (!m_fmv_dsp_enable || m_video_es_written == m_flush_tried_at)
		return;
	m_flush_tried_at = m_video_es_written;

	plm_video_t *const video = m_dec->video;
	plm_buffer_t *const buffer = m_dec->video_buffer;

	// pl_mpeg compacts the buffer as it decodes, so keep the bytes too
	const plm_video_t saved_video = *video;
	const plm_buffer_t saved_buffer = *buffer;
	const std::vector<uint8_t> saved_bytes(buffer->bytes, buffer->bytes + buffer->length);

	buffer->cdi_read_underrun = FALSE;
	buffer->has_ended = TRUE;
	video->cdi_picture_complete = TRUE;
	video->cdi_trial = TRUE;  // stop at a picture size change rather than free the frames

	std::vector<picture> flushed;
	bool complete = true;
	while (m_picture_fifo.size() + flushed.size() < MAX_PICTURES)
	{
		plm_frame_t *const frame = plm_video_decode(video);
		if (buffer->cdi_read_underrun || !video->cdi_picture_complete)
		{
			complete = false;
			break;
		}
		if (!frame)
			break;
		flushed.push_back(make_picture(frame));
	}

	if (!complete)
	{
		*video = saved_video;
		*buffer = saved_buffer;
		std::copy(saved_bytes.begin(), saved_bytes.end(), buffer->bytes);
		LOGMASKED(LOG_VIDEO, "FMV flush: picture incomplete, waiting for data\n");
		return;
	}

	buffer->has_ended = saved_buffer.has_ended;
	video->cdi_trial = FALSE;
	check_sequence_end();
	if (!flushed.empty())
		LOGMASKED(LOG_VIDEO, "FMV flush: %d picture(s) released\n", int(flushed.size()));
	for (picture &pic : flushed)
		queue_picture(std::move(pic));
}

// The GOP time_code is 25 bits: drop_frame(1) hours(5) minutes(6) marker(1)
// seconds(6) frames(6).  TIMECD @ 0xe04058 packs the same fields differently:
//   [10:6] hours, [5:0] minutes, [27:22] seconds, [21:16] frames
uint32_t cdi_dvc_device::pack_timecode(uint32_t gop_timecode)
{
	const uint32_t frames  = gop_timecode & 0x3f;
	const uint32_t seconds = (gop_timecode >> 6) & 0x3f;
	const uint32_t minutes = (gop_timecode >> 13) & 0x3f;
	const uint32_t hours   = (gop_timecode >> 19) & 0x1f;

	return (hours << 6) | minutes | (seconds << 22) | (frames << 16);
}

// SYSCMD "clear FIFO" precedes a new stream.  pl_mpeg still holds the previous
// stream's sequence header, reference frames and any half-read picture, so the
// decoder has to start over or it decodes across the seam.
void cdi_dvc_device::reset_video_decoder()
{
	if (!m_dec)
		return;

	plm_buffer_t *const buffer = plm_buffer_create_with_capacity(VIDEO_BUFFER_SIZE);
	plm_video_t *const video = plm_video_create_with_buffer(buffer, TRUE);

	// keep the sequence header, so a stream resumed mid sequence decodes at
	// once instead of waiting for the next one
	if (m_dec->video)
	{
		plm_video_cdi_copy_sequence_header(video, m_dec->video);
		plm_video_destroy(m_dec->video);
	}
	else if (m_dec->video_buffer)
	{
		plm_buffer_destroy(m_dec->video_buffer);
	}

	m_dec->video_buffer = buffer;
	m_dec->video = video;
}

void cdi_dvc_device::reset_audio_decoder()
{
	if (!m_dec)
		return;

	if (m_dec->audio)
		plm_audio_destroy(m_dec->audio);
	else if (m_dec->audio_buffer)
		plm_buffer_destroy(m_dec->audio_buffer);

	m_dec->audio_buffer = plm_buffer_create_with_capacity(AUDIO_BUFFER_SIZE);
	m_dec->audio = plm_audio_create_with_buffer(m_dec->audio_buffer, TRUE);
}

void cdi_dvc_device::clear_video_fifo()
{
	reset_video_decoder();
	m_video_needs_data = false;
	if (m_es_batch_for_fma == false)
		m_es_batch.clear();

	m_picture_fifo.clear();
	m_dts_fifo.clear();
	m_next_picture_dts = 0;
	m_pictures_in_input_fifo = 0;
	m_latch_until_vsync = false;
	m_latch_until_vblank = false;
	m_single_step_latch = false;
	m_desync_satisfied = false;
	m_display_dts_desync = 0;
}

//**************************************************************************
//  AUDIO DECODE
//**************************************************************************

void cdi_dvc_device::audio_decode_pending()
{
	// keep roughly a quarter second of samples queued
	while (audio_available() < 44100 / 4)
	{
		plm_samples_t *samples = plm_audio_decode(m_dec->audio);
		if (!samples)
			break;

		m_audio_sample_rate = uint32_t(plm_audio_get_samplerate(m_dec->audio));

		for (unsigned i = 0; i < samples->count; i++)
		{
			const float l = samples->interleaved[i * 2 + 0];
			const float r = samples->interleaved[i * 2 + 1];
			m_audio_samples[0].push_back(int16_t(std::clamp(l, -1.0f, 1.0f) * 32767.0f));
			m_audio_samples[1].push_back(int16_t(std::clamp(r, -1.0f, 1.0f) * 32767.0f));
		}

		if (!(m_fma_status & FMA_DEC))
		{
			// first frame decoded: decoding started
			m_fma_status |= FMA_DEC;
			m_fma_isr |= FMA_DEC;
		}

		// every decoded frame updates the frame header
		m_fma_status |= FMA_UPD;
		m_fma_isr |= FMA_UPD;
		if (m_pending_fma_stream_change)
		{
			m_fma_isr |= FMA_CSU;
			m_pending_fma_stream_change = false;
		}
		update_intreq();
	}
}

TIMER_CALLBACK_MEMBER(cdi_dvc_device::audio_tick)
{
	// Playback is held off until the decoder clock reaches the point
	// the first PTS asks for, relative to the pack header's SCR.  That is what
	// starts the FMA decoder, so it has to be checked before the enable test.
	if (m_fma_scr_start_valid && int64_t(fma_dclk()) >= m_fma_scr_start_time)
	{
		m_fma_scr_start_valid = false;
		// the start time is re-armed by each following PTS and merely
		// re-enables the decoder, so this is reached repeatedly during
		// normal playback.  Only the transition is worth logging.
		if (!m_fma_dsp_enable)
			LOGMASKED(LOG_AUDIO, "FMA playback started at dclk %u\n", fma_dclk());
		m_fma_dsp_enable = true;
	}

	audio_decode_pending();

	if (!m_fma_dsp_enable)
		return;

	// An underflow is when the decoder needs data from the input FIFO and
	// there is none.  Looking only at the decoded sample queue trips
	// on ordinary delivery jitter: one MP2 frame is 1152 samples, 26ms at
	// 44.1kHz, while sectors arrive in bursts, so the queue legitimately
	// empties between them.  Require the elementary stream to be dry as well.
	if (audio_available() == 0
			&& plm_buffer_get_remaining(m_dec->audio_buffer) < MP2_FRAME_BYTES)
	{
		// an underflow stops the decoder and flags the status bits
		m_fma_status |= FMA_UNF;
		m_fma_isr |= FMA_UNF;
		m_fma_status &= ~FMA_DEC;
		m_fma_dsp_enable = false;
		m_fma_scr_start_valid = false;
		update_intreq();
		LOGMASKED(LOG_AUDIO, "FMA underflow\n");
	}
}

void cdi_dvc_device::sound_stream_update(sound_stream &stream)
{
	const int samples = stream.samples();

	for (int i = 0; i < samples; i++)
	{
		int16_t l = 0, r = 0;
		if (m_fma_dsp_enable && audio_available() != 0)
		{
			l = m_audio_samples[0][m_audio_head];
			r = m_audio_samples[1][m_audio_head];
			m_audio_head++;
		}
		stream.put_int(0, i, l, 32768);
		stream.put_int(1, i, r, 32768);
	}

	// drop the consumed prefix now and then so the vectors do not grow forever
	if (m_audio_head >= 44100)
	{
		for (auto &chan : m_audio_samples)
			chan.erase(chan.begin(), chan.begin() + m_audio_head);
		m_audio_head = 0;
	}
}

//**************************************************************************
//  FRAME DISPLAY PACING
//**************************************************************************

void cdi_dvc_device::restart_frame_timer()
{
	if (!m_fmv_playback_active)
	{
		m_frame_timer->adjust(attotime::never);
		return;
	}

	// The display period is the picture period less 2048 ticks per unit of
	// DTS desync.  The width matters.  It is a 24 bit value, so the correction wraps rather
	// than running away; computing this in int64 let a large negative desync
	// stretch one frame period past a second, which stalls the display
	// completely.
	int64_t period = (int64_t(m_frame_period_30mhz) - 1
			- int64_t(m_display_dts_desync) * 2048) & 0xffffff;

	// slow motion is a 1..8 divisor on the display rate
	if (m_fmv_slow_motion > 1)
		period *= m_fmv_slow_motion;

	// never run the display at an implausible rate
	if (period < 1200)
		period = m_frame_period_30mhz;

	m_frame_timer->adjust(attotime::from_ticks(period, FRAME_CLOCK_HZ));
}

TIMER_CALLBACK_MEMBER(cdi_dvc_device::frame_period_tick)
{
	if (!m_fmv_playback_active)
		return;

	// Only start counting frames once we are near the DTS the stream asks for,
	// and only in synchronous mode.
	const bool synchronous = BIT(m_fmv_sysscr, 2);

	if (!m_dts_fifo.empty())
	{
		// display_dts_desync = dclk[20:6] - dts[21:7], both at 703.125 Hz.
		// It is a signed 15 bit value, so the subtraction wraps
		// into +/-16384 rather than spanning the full 15 bit range.
		int32_t desync = int32_t((fmv_dclk() >> 6) & 0x7fff)
				- int32_t((m_dts_fifo.front() >> 7) & 0x7fff);
		if (desync >= 0x4000)
			desync -= 0x8000;
		else if (desync < -0x4000)
			desync += 0x8000;

		if (!synchronous || desync > -30)
			m_desync_satisfied = true;

		// When the demuxer's SCR has run far ahead of the timestamp we are
		// about to display there is no point correcting the frame period, so
		// the correction is dropped.
		const bool valid_dts = m_dts_fifo.front() != 0;
		const bool way_out = valid_dts
				&& mpeg_timestamp_diff(m_fmv_demux.scr, m_dts_fifo.front()) > 15000;

		m_display_dts_desync =
				(m_syscr_written && synchronous && valid_dts && !way_out) ? desync : 0;

		// Skipping a frame here to claw back time does not work: it depends on
		// a display pipeline we do not model,
		// and dropping pictures from this queue empties it faster than the
		// disc refills it.  The queue running dry raises NDAT, and the driver
		// answers NDAT by pausing, which ends playback.  Observed killing the
		// second stream of 7th Guest every time.  Correct the frame period and
		// leave the queue alone.
	}
	else
	{
		m_desync_satisfied = true;
		m_display_dts_desync = 0;
	}

	if (!synchronous || m_desync_satisfied)
		m_latch_until_vsync = true;

	restart_frame_timer();
}

void cdi_dvc_device::latch_display_frame()
{
	if (m_picture_fifo.empty())
		return;

	m_display = std::move(m_picture_fifo.front());
	m_picture_fifo.erase(m_picture_fifo.begin());
	m_display_valid = true;

	if (!m_dts_fifo.empty())
		m_dts_fifo.erase(m_dts_fifo.begin());

	m_display_width = m_display.width;
	m_display_height = m_display.height;
	m_display_video_status = m_display.video_status;
	m_display_timecode = m_display.timecode;
	m_display_frameperiod_rawhdr = m_display.frameperiod_rawhdr;
	m_display_first_intra_of_gop = m_display.first_intra_of_gop;
	m_display_first_intra_of_seq = m_display.first_intra_of_seq;
}

// The driver sees the DVC through two vertical events per frame.  The
// cartridge uses the vsync trailing edge to pick the next picture and the vblank trailing
// edge to announce that it is now on screen.  MAME only gives us the vblank
// interval, so vblank begin stands in for the first and vblank end for the
// second, which keeps both the ordering and the one-frame gap between them.
void cdi_dvc_device::screen_vblank(int state)
{
	if (state)
	{
		// --- "vsync trailing edge" ---
		if (m_latch_until_vsync)
		{
			m_latch_until_vsync = false;

			// a picture that may start display applies the deferred
			// register update when scroll is clear
			if (m_register_update_latch && !m_register_update_scroll)
			{
				m_fmv_isr |= FMV_IRQ_VCUP | FMV_IRQ_DCL;
				m_register_update_latch = false;

				m_latched_display_offset_x = m_video_ctrl_x_display;
				m_latched_display_offset_y = m_video_ctrl_y_display;
				m_latched_window_offset_x = m_video_ctrl_decoder_offset_x;
				m_latched_window_offset_y = m_video_ctrl_decoder_offset_y;
				m_latched_window_width = m_video_ctrl_window_width;
				m_latched_window_height = m_video_ctrl_window_height;
			}

			// the display ran dry with pictures still inside the decoder
			if (m_fmv_playback_active && m_picture_fifo.empty() && m_pictures_in_input_fifo)
				video_flush_stalled();

			if (!m_picture_fifo.empty())
			{
				// The underflow is reported as the *last* picture is taken
				// with nothing behind it, not once the queue has already
				// run dry.
				// Here the reordering backlog is the set of picture start
				// codes that have entered the elementary stream but which
				// pl_mpeg has not handed back yet.
				if (m_fmv_playback_active && m_picture_fifo.size() == 1
						&& m_pictures_in_input_fifo == 0)
				{
					m_fmv_isr |= FMV_IRQ_NDAT;
					LOGMASKED(LOG_VIDEO, "FMV underflow\n");
				}

				latch_display_frame();
				m_latch_until_vblank = true;
			}
		}

		if (m_single_step_latch && !m_picture_fifo.empty())
		{
			m_single_step_latch = false;
			latch_display_frame();
			m_latch_until_vblank = true;
		}

		// the VSYNC flag is set on the rising edge of vsync
		m_fmv_isr |= FMV_IRQ_VSYNC;

		// scroll set means the register update happens at vertical retrace
		if (m_register_update_latch && m_register_update_scroll)
		{
			m_fmv_isr |= FMV_IRQ_VCUP | FMV_IRQ_DCL;
			m_register_update_latch = false;

			m_latched_display_offset_x = m_video_ctrl_x_display;
			m_latched_display_offset_y = m_video_ctrl_y_display;
			m_latched_window_offset_x = m_video_ctrl_decoder_offset_x;
			m_latched_window_offset_y = m_video_ctrl_decoder_offset_y;
			m_latched_window_width = m_video_ctrl_window_width;
			m_latched_window_height = m_video_ctrl_window_height;
		}

		update_intreq();
	}
	else
	{
		// --- "picture starts display" ---
		if (m_latch_until_vblank)
		{
			m_latch_until_vblank = false;

			m_fmv_isr |= FMV_IRQ_PIC;
			if (m_display_first_intra_of_gop)
				m_fmv_isr |= FMV_IRQ_GOP;
			if (m_display_first_intra_of_seq)
				m_fmv_isr |= FMV_IRQ_SEQ;
			if (m_picture_fifo.empty() && m_pictures_in_input_fifo == 0)
				m_fmv_isr |= FMV_IRQ_EOD;

			update_intreq();
		}

		// keep the decoder fed
		video_decode_pending();
	}
}

//**************************************************************************
//  VIDEO OUTPUT
//**************************************************************************

// x is a column of the 768 pixel MCD212 line buffer, y is a display line
// counted from the start of active video.  The DVC positioning registers work
// in CD-i pixels, of which there are 384 across the active line.
bool cdi_dvc_device::ext_video_pixel(int x, int y, uint32_t &argb) const
{
	if (!m_show_video || !m_display_valid || !m_display.rgb)
	{
		return false;
	}

	const int cdi_x = x / 2;

	const int sx = cdi_x - int(m_latched_display_offset_x);
	const int sy = y - int(m_latched_display_offset_y);

	if (sx < 0 || sy < 0)
		return false;
	if (m_latched_window_width && sx >= int(m_latched_window_width))
		return false;
	if (m_latched_window_height && sy >= int(m_latched_window_height))
		return false;

	const int px = sx + int(m_latched_window_offset_x);
	const int py = sy + int(m_latched_window_offset_y);

	if (px < 0 || py < 0 || px >= int(m_display.width) || py >= int(m_display.height))
		return false;

	argb = m_display.rgb[size_t(py) * m_display.width + px];
	return true;
}

//**************************************************************************
//  INTERRUPTS
//**************************************************************************

// name the FMV status bits as they are raised
void cdi_dvc_device::log_fmv_isr(const char *where)
{
	const uint16_t fresh = m_fmv_isr & ~m_logged_fmv_isr;
	m_logged_fmv_isr = m_fmv_isr;
	if (!fresh)
		return;

	static const char *const names[16] = {
		"SEQ", "GOP", "PIC", "EOD", "RFB", "NDAT", "OVF", "DCL",
		"TIM", "ESI", "EII", "VSYNC", "PAI", "VCUP", "ERDD", "ERDV"
	};
	for (int i = 0; i < 16; i++)
		if (BIT(fresh, i) && i != 2 && i != 8 && i != 11) // skip PIC/TIM/VSYNC noise
			LOGMASKED(LOG_FMV, "FMV ISR set %s (%s)\n", names[i], where);
}

void cdi_dvc_device::update_intreq()
{
	log_fmv_isr("update");

	const bool fma = (m_fma_isr & m_fma_ier) != 0;
	const bool fmv = (m_fmv_isr & m_fmv_ier) != 0;
	const bool req = fma || fmv;

	if (req != m_intreq_state)
	{
		m_intreq_state = req;
		LOGMASKED(LOG_IRQ, "DVC intreq %d (fma isr %04x ier %04x, fmv isr %04x ier %04x)\n",
				req, m_fma_isr, m_fma_ier, m_fmv_isr, m_fmv_ier);
		m_intreq_cb(req ? 1 : 0);
	}
}

uint8_t cdi_dvc_device::intack_r()
{
	// the vector is the FMA vector's low byte, or bits 10:3 of the FMV one
	if ((m_fma_isr & m_fma_ier) != 0)
		return uint8_t(m_fma_ivec);

	return uint8_t(m_fmv_ivec >> 3);
}

TIMER_CALLBACK_MEMBER(cdi_dvc_device::tim_tick)
{
	m_fmv_isr |= FMV_IRQ_TIM;
	m_fma_isr |= FMA_POLL;
	update_intreq();
}

TIMER_CALLBACK_MEMBER(cdi_dvc_device::start_video_tick)
{
	m_fmv_playback_active = true;
	restart_frame_timer();
}

TIMER_CALLBACK_MEMBER(cdi_dvc_device::pause_video_tick)
{
	m_fmv_isr |= FMV_IRQ_PAI;
	update_intreq();
}

//**************************************************************************
//  ROM AND RAM
//**************************************************************************

uint16_t cdi_dvc_device::rom_r(offs_t offset)
{
	if (!m_rom)
		return 0xffff;

	// 128KB of ROM mirrored across the 256KB window
	return m_rom[offset & 0xffff];
}

uint16_t cdi_dvc_device::ram_r(offs_t offset, uint16_t mem_mask)
{
	return m_ram[offset & 0x3ffff];
}

void cdi_dvc_device::ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ram[offset & 0x3ffff]);
}

//**************************************************************************
//  REGISTER READS
//**************************************************************************

uint16_t cdi_dvc_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	// only A[15:1] is decoded, so the register file mirrors every 64KB
	const uint16_t reg = offset & 0x7fff;
	uint16_t data = 0;

	switch (reg)
	{
	// ---- FMA @ 0xe03000 ----
	case 0x1800: data = m_fma_cmd; break;
	case 0x1801: data = 0x0200 | m_fma_status; break;
	case 0x1802: data = 0x0007; break;
	case 0x1803: data = 0x0900; break;
	case 0x1804: data = m_fma_stream; break;
	case 0x1805: data = m_fma_stream; break;
	case 0x1806: data = m_fma_ivec; break;
	case 0x1807: data = 0x0042; break;

	case 0x1808:
		data = uint16_t(fma_dclk() >> 16);
		if (!machine().side_effects_disabled())
			m_fma_dclkl_latch = uint16_t(fma_dclk());
		break;

	case 0x1809: data = m_fma_dclkl_latch; break;
	case 0x180a: data = uint16_t(m_fma_audio_header >> 16); break;
	case 0x180b: data = uint16_t(m_fma_audio_header); break;
	case 0x180c: data = m_fma_dsp_enable ? 1 : 0; break;

	case 0x180d:
		data = m_fma_isr;
		if (!machine().side_effects_disabled())
		{
			// reading the interrupt status register clears it
			m_fma_isr = 0;
			update_intreq();
		}
		break;

	case 0x180e: data = m_fma_ier; break;
	case 0x1812: data = 0x0004; break; // HF2 flag of the DSP56001

	// ---- FMV @ 0xe04000 ----
	case 0x2001: data = m_image_width; break;
	case 0x2002: data = m_image_height; break;
	case 0x2003: data = m_image_rt; break;
	// 0x2004/0x2005 report the *decoder* timecode, 0x202c/0x202d the
	// *displayed* one: the FMV driver's MVS_TimeCd reads SYS_TCL at 0x0e04058
	// as the timecode of the current picture.
	case 0x2004: data = uint16_t(m_decoder_timecode >> 16); break;
	case 0x2005: data = uint16_t(m_decoder_timecode); break;

	case 0x2029: data = m_display_width; break;
	case 0x202a: data = m_display_height; break;
	case 0x202b: data = m_display_frameperiod_rawhdr; break;
	case 0x202c: data = uint16_t(m_display_timecode >> 16); break;
	case 0x202d: data = uint16_t(m_display_timecode); break;
	case 0x202e: data = m_display_video_status; break;

	case 0x202f:
		// SYS_STS bit 13 is "request for bits", set while the *input stream*
		// FIFO has room.  The decoded picture queue is a different thing and
		// must not gate this, or the driver stops feeding whenever decoding
		// runs ahead of display.
		//
		// pl_mpeg only decodes a picture once the next start code is in, so
		// one large picture can fill the FIFO while the decoder still waits
		// for data: Atlantis has a 27 KB I picture, the driver stopped
		// feeding, the CD play buffers were never released and the read
		// aborted with "your disc may be dirty".  The real decoder drains
		// the FIFO macroblock by macroblock, so keep asking while ours is
		// starved; that can only overshoot by about one picture.
		data = (plm_buffer_get_remaining(m_dec->video_buffer) < VIDEO_FIFO_FULL || m_video_needs_data)
				? 0x2000 : 0;
		break;

	case 0x2030: data = m_fmv_ier; break;

	case 0x2031:
		data = m_fmv_isr;
		if (!machine().side_effects_disabled())
		{
			// reading the interrupt status register clears it
			m_fmv_isr = 0;
			update_intreq();
		}
		break;

	case 0x2032: data = m_fmv_tcnt; break;

	case 0x2036: data = m_video_ctrl_y_offset; break;
	case 0x2037: data = m_video_ctrl_x_offset; break;
	case 0x2038: data = m_video_ctrl_y_active; break;
	case 0x2039: data = m_video_ctrl_x_active; break;
	case 0x203a: data = m_video_ctrl_y_display; break;
	case 0x203b: data = m_video_ctrl_x_display; break;
	case 0x203c: data = m_video_ctrl_window_height; break;
	case 0x203d: data = m_video_ctrl_window_width; break;
	case 0x203e: data = m_video_ctrl_decoder_offset_y; break;
	case 0x203f: data = m_video_ctrl_decoder_offset_x; break;

	case 0x2044: data = m_fmv_dec_cmd; break;
	case 0x2046: data = m_fmv_vdi_cmd; break;
	case 0x2049: data = 0; break; // GEN_VID_BUF
	case 0x204c:
		// GEN_SYSCR: bits 20:6 of the decoder clock.  The driver needs bit 21
		// kept clear: returning it puts
		// it in bit 15 of the result, which the driver reads as a negative
		// SCR; the decoder clock crosses 2^21 ticks 46 seconds after reset, so
		// playback dies at a fixed machine uptime regardless of the stream.
		data = uint16_t((fmv_dclk() >> 6) & 0x7fff);
		break;
	case 0x204e: data = 0; break; // GEN_SYNC_DIFF, always 0 on real hardware
	case 0x204f: data = 1; break; // GEN_DEC_DELAY

	case 0x2050:
		// GEN_DEC_TIM1: the demuxer DTS as the CPU wants it, at 703.125 Hz
		data = uint16_t((m_fmv_demux.dts >> 7) & 0x7fff);
		break;

	case 0x2052:
		// pictures queued.  Before playback starts only the DTS fifo depth is
		// reported, as for a decoder that has not started yet.
		data = m_fmv_playback_active
				? uint16_t(m_picture_fifo.size() + m_pictures_in_input_fifo)
				: uint16_t(m_dts_fifo.size());
		break;

	case 0x2054: data = m_decoder_frameperiod_90khz; break;
	case 0x2055: data = m_pal ? DISPLAY_RATE_PAL : DISPLAY_RATE_NTSC; break;
	case 0x2056: data = m_fmv_frame_rate; break;
	case 0x2060: data = m_fmv_syscmd; break;
	case 0x2061: data = m_fmv_vidcmd; break;
	case 0x2062: data = m_fmv_stream; break;
	case 0x206e: data = m_fmv_ivec; break;
	case 0x2073: data = 0; break; // MMU base pointer, read once and ignored

	default:
		break;
	}

	LOGMASKED(LOG_REGS_R, "%s: dvc_r: %08x = %04x & %04x\n", machine().describe_context(),
			0xe00000 + (offset << 1), data, mem_mask);

	return data;
}

//**************************************************************************
//  REGISTER WRITES
//**************************************************************************

void cdi_dvc_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const uint16_t reg = offset & 0x7fff;

	LOGMASKED(LOG_REGS_W, "%s: dvc_w: %08x = %04x & %04x\n", machine().describe_context(),
			0xe00000 + (offset << 1), data, mem_mask);

	// The 512KB of decoder RAM only appears once the register file has been
	// written 64 times, so the OS RAM crawler never sees it.
	if (!m_mpeg_ram_enabled)
	{
		if (++m_mpeg_ram_enable_cnt >= 64)
		{
			m_mpeg_ram_enabled = true;
			LOGMASKED(LOG_REGS_W, "DVC: MPEG RAM enabled\n");
		}
	}

	// VMPEG pixel clock select @ 0xe01xxx, lower bits are don't care
	if ((reg >> 8) == 0x08)
		return;

	switch (reg)
	{
	// ---- FMA ----
	case 0x1800: // CMD
		LOGMASKED(LOG_FMA, "FMA CMD %04x\n", data);
		m_fma_cmd = data;
		if (BIT(data, 15))
		{
			// green book 8.2.4.3.3: a transfer clears the underflow flag
			m_fma_status &= ~FMA_UNF;
			m_dma_active = true;
			run_dma(true);
		}
		if (BIT(data, 0))
		{
			// stop
			m_fma_dsp_enable = false;
			m_fma_status = 0;
			m_fma_demux.reset();
			m_fma_scr_start_valid = false;
			reset_audio_decoder();
			audio_clear();
		}
		break;

	case 0x1802:
		// immediate 7 written by the VMPEG ROM right after the stream number
		break;

	case 0x1804: // stream number, 0-15 per mv_selstrm()
		LOGMASKED(LOG_FMA, "FMA stream %04x\n", data);
		m_fma_stream = data & 0x0f;
		m_pending_fma_stream_change = true;
		break;

	case 0x1806:
		LOGMASKED(LOG_FMA, "FMA IVEC %04x\n", data);
		m_fma_ivec = data;
		break;

	case 0x180c:
		LOGMASKED(LOG_FMA, "FMA RUN %04x\n", data);
		break;

	case 0x180e:
		LOGMASKED(LOG_FMA, "FMA IER %04x\n", data);
		m_fma_ier = data;
		update_intreq();
		break;

	case 0x1811:
		m_fma_dspa = uint8_t(data);
		break;

	case 0x1812:
		// DSP56001 data port, used for the attenuation ramp
		break;

	// ---- FMV ----
	case 0x2030:
		LOGMASKED(LOG_FMV, "FMV IER %04x\n", data);
		m_fmv_ier = data;
		update_intreq();
		break;

	case 0x2031:
		break;

	case 0x2032: // TCNT
		LOGMASKED(LOG_FMV, "FMV TCNT %04x\n", data);
		m_fmv_tcnt = data;
		{
			const attotime period = attotime::from_ticks(((data & 0xffff) + 1) * 8, DCLK_HZ);
			m_tim_timer->adjust(period, 0, period);
		}
		break;

	case 0x2057: // TRLD
		LOGMASKED(LOG_FMV, "FMV TRLD %04x\n", data);
		{
			const attotime period = attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ);
			m_tim_timer->adjust(period, 0, period);
		}
		break;

	case 0x2060: // SYSCMD
		LOGMASKED(LOG_FMV, "FMV SYSCMD %04x\n", data);
		m_fmv_syscmd = data;

		if (data & SYSCMD_PLAY)
		{
			m_start_video_timer->adjust(attotime::from_ticks(PLAY_DELAY_TICKS, DCLK_HZ));

			m_image_width = m_decoder_width;
			m_image_height = m_decoder_height;
			m_image_rt = m_decoder_frameperiod_rawhdr;
			// the play command also sets 0x42 in the decoder command
			m_fmv_dec_cmd |= (1 << 6) | (1 << 1);
			m_fmv_slow_motion = data & 0x07;
			m_fmv_decoder_active = true;
		}

		if (data & SYSCMD_PAUSE)
		{
			m_fmv_playback_active = false;
			m_start_video_timer->adjust(attotime::never);
			m_frame_timer->adjust(attotime::never);
			m_pause_video_timer->adjust(attotime::from_ticks(PAUSE_DELAY_TICKS, DCLK_HZ));
		}

		if (data & SYSCMD_CONTINUE)
		{
			m_start_video_timer->adjust(attotime::from_ticks(PLAY_DELAY_TICKS, DCLK_HZ));
			m_fmv_slow_motion = data & 0x07;
		}

		if (data & SYSCMD_STEP)
		{
			m_single_step_latch = true;
			m_start_video_timer->adjust(attotime::never);
		}

		if (data & SYSCMD_STOP)
		{
			m_fmv_playback_active = false;
			m_fmv_decoder_active = false;
			m_start_video_timer->adjust(attotime::never);
			m_frame_timer->adjust(attotime::never);
		}

		if (data & SYSCMD_CLEARFIFO)
		{
			m_fmv_playback_active = false;
			m_start_video_timer->adjust(attotime::never);
			m_frame_timer->adjust(attotime::never);
			clear_video_fifo();
			m_fmv_demux.reset();
			// the decoder is briefly disabled, then enabled again
			m_fmv_dsp_enable = true;
		}

		if (data & SYSCMD_GOPSEARCH)
		{
			// handled implicitly: pl_mpeg resumes at the next intra picture
		}

		if (data & SYSCMD_DEC_ON)
		{
			m_fmv_dsp_enable = true;
			LOGMASKED(LOG_FMV, "FMV decoder on\n");
		}

		if (data & SYSCMD_DEC_OFF)
		{
			m_fmv_dsp_enable = false;
			m_fmv_playback_active = false;
			m_fmv_decoder_active = false;
			m_start_video_timer->adjust(attotime::never);
			m_frame_timer->adjust(attotime::never);
			m_image_width = 0;
			m_image_height = 0;
			m_image_rt = 0;
			m_display_video_status = 0;
			m_display_timecode = 0;
			m_decoder_timecode = 0;
			m_decoder_frameperiod_90khz = 0;
			m_decoder_frameperiod_rawhdr = 0;
			m_display_valid = false;
			clear_video_fifo();
			LOGMASKED(LOG_FMV, "FMV decoder off\n");
		}

		if (data & SYSCMD_DMA)
		{
			m_dma_active = true;
			run_dma(false);
		}
		break;

	case 0x2061: // VIDCMD
		LOGMASKED(LOG_FMV, "FMV VIDCMD %04x\n", data);
		m_fmv_vidcmd = data;

		if (data & VIDCMD_REGSUPD)
		{
			m_register_update_latch = true;
			m_register_update_scroll = (data & VIDCMD_SCROLL) != 0;
		}

		if (data & VIDCMD_HIDE)
			m_show_video = false;

		if (data & VIDCMD_SHOW)
			m_show_video = true;

		if (data & VIDCMD_SHOWNEXT)
			m_show_video = true;
		break;

	case 0x2063: // SYS_SCR
		LOGMASKED(LOG_FMV, "FMV SYSSCR %04x\n", data);
		m_fmv_sysscr = data;
		break;

	case 0x206f: // XFER: the CPU pushing stream data by hand (host play)
		feed_byte(uint8_t(data >> 8));
		feed_byte(uint8_t(data));
		flush_es_batch();
		if (m_dma_for_fma)
			audio_decode_pending();
		else
			video_decode_pending();
		break;

	case 0x206e:
		LOGMASKED(LOG_FMV, "FMV IVEC %04x\n", data);
		m_fmv_ivec = data;
		break;

	case 0x2036: m_video_ctrl_y_offset = data; break;             // always 0x001a
	case 0x2037: m_video_ctrl_x_offset = data; break;             // always 0x004a
	case 0x2038: m_video_ctrl_y_active = data; break;
	case 0x2039: m_video_ctrl_x_active = data; break;
	case 0x203a: m_video_ctrl_y_display = data; break;            // FMV_SCRPOS Y
	case 0x203b: m_video_ctrl_x_display = data; break;            // FMV_SCRPOS X
	case 0x203c: m_video_ctrl_window_height = data; break;        // FMV_DECWIN H
	case 0x203d: m_video_ctrl_window_width = data; break;         // FMV_DECWIN W
	case 0x203e: m_video_ctrl_decoder_offset_y = data; break;     // FMV_DECOFF Y
	case 0x203f: m_video_ctrl_decoder_offset_x = data; break;     // FMV_DECOFF X

	case 0x2044: // GEN_DEC_CMD
		m_fmv_dec_cmd = data;
		if ((data >> 8) == 0x22 && !(m_fmv_syscmd & SYSCMD_PAUSE))
			m_single_step_latch = true;
		break;

	case 0x2046:
		m_fmv_vdi_cmd = data;
		break;

	case 0x2049: // GEN_VID_BUF
		break;

	case 0x204c: // GEN_SYSCR, bits 20:6 (bit 21 must stay clear, see the read)
		LOGMASKED(LOG_FMV, "FMV GEN_SYSCR %04x\n", data);
		m_fmv_dclk_offset = int32_t(((uint32_t(data) & 0x7fff) << 6)
				- (fma_dclk() & 0x001fffc0));
		m_syscr_written = true;
		break;

	case 0x2056:
		m_fmv_frame_rate = data;
		break;

	case 0x2001: m_image_width = data; break;
	case 0x2002: m_image_height = data; break;
	case 0x2003: m_image_rt = data; break;

	case 0x2062: // stream number, 0-15 per mv_selstrm()
		LOGMASKED(LOG_FMV, "FMV stream %04x\n", data);
		m_fmv_stream = data & 0x0f;
		break;

	default:
		break;
	}
}


//**************************************************************************
//  CARTRIDGE SLOT
//**************************************************************************

cdi_dvc_slot_device::cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CDI_DVC_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<cdi_dvc_device>(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
{
}

void cdi_dvc_slot_device::device_start()
{
}

void cdi_dvc_cards(device_slot_interface &device)
{
	device.option_add("vmpeg", CDI_DVC);
}
