// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Motorola MCD251 MPEG-1 full motion video decoder.  See mcd251.h.

    Emulation notes:

    - The chip's MPEG decoding is undocumented, so the elementary stream is
      decoded with mpeg_video.  The picture stores, the display order and the
      status the registers report are the chip's own.
    - The 30 MHz and 45 kHz counters are derived from machine time, with
      emu_timers for the events, rather than counted tick by tick.
    - The chip distinguishes the vsync pulse from the vblank interval.  A
      screen only offers vblank begin and end, so vblank begin stands in for
      "vsync trailing edge" and vblank end for "picture starts display",
      which preserves the ordering and the one frame latency between them.

*******************************************************************************/

#include "emu.h"
#include "mcd251.h"

#include <algorithm>
#include <cmath>

#define LOG_REGS_R    (1U << 1)
#define LOG_REGS_W    (1U << 2)
#define LOG_FMV       (1U << 3)
#define LOG_IRQ       (1U << 4)
#define LOG_VIDEO     (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(MCD251, mcd251_device, "mcd251", "Motorola MCD251 MPEG-1 video decoder")

//**************************************************************************
//  CONSTANTS
//**************************************************************************

namespace {

// The decoder clock ticks at 45 kHz.
constexpr int DCLK_HZ = 45000;

// Frame display is paced in 30 MHz system clock ticks: 1200000 per picture
// at 25 Hz.
constexpr int FRAME_CLOCK_HZ = 30000000;

// the video starts this many decoder clock ticks after the play command
constexpr int PLAY_DELAY_TICKS = 1000;
// and pauses this many ticks after the pause command
constexpr int PAUSE_DELAY_TICKS = 100;

// the display rate the chip reports for PAL and NTSC
constexpr uint16_t DISPLAY_RATE_PAL  = 0x0708;
constexpr uint16_t DISPLAY_RATE_NTSC = 0x05dc;

// The decoded picture queue.  The real chip only has RAM for about three
// pictures; queuing more keeps the decode rate steady.
constexpr size_t MAX_PICTURES = 8;

// the largest picture the decoder accepts, and a YCbCr 4:2:0 store for it
constexpr int MAX_PICTURE_WIDTH = 768;
constexpr int MAX_PICTURE_HEIGHT = 576;
constexpr size_t PICTURE_STORE_BYTES = size_t((MAX_PICTURE_WIDTH + 15) & ~15) * ((MAX_PICTURE_HEIGHT + 15) & ~15) * 3 / 2;

// The chip drives 4 Mbit of DRAM directly, 256K words.
constexpr size_t DRAM_WORDS = 0x80000 / 2;

// Size of the input stream FIFO, which the "request for bits" status
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
// GEN_PICT_RATE reports.
constexpr uint16_t FRAME_PERIOD_90KHZ[16] =
{
	3600, 3754, 3750, 3600, 3003, 3000, 1800, 1502,
	1500, 3600, 3600, 3600, 3600, 3600, 3600, 3600
};

} // anonymous namespace

//**************************************************************************
//  CONSTRUCTION
//**************************************************************************

mcd251_device::mcd251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, MCD251, tag, owner, clock)
	, m_irq_cb(*this)
	, m_drq_cb(*this)
	, m_timer_cb(*this)
{
}

mcd251_device::~mcd251_device()
{
}

void mcd251_device::device_start()
{
	m_dram = std::make_unique<uint16_t []>(DRAM_WORDS);
	std::fill_n(m_dram.get(), DRAM_WORDS, 0);

	m_video = std::make_unique<mpeg_video>(MAX_PICTURE_WIDTH, MAX_PICTURE_HEIGHT);
	for (picture_store &store : m_store)
		store.ycbcr = std::make_unique<uint8_t []>(PICTURE_STORE_BYTES);

	m_tim_timer = timer_alloc(FUNC(mcd251_device::tim_tick), this);
	m_start_video_timer = timer_alloc(FUNC(mcd251_device::start_video_tick), this);
	m_pause_video_timer = timer_alloc(FUNC(mcd251_device::pause_video_tick), this);
	m_frame_timer = timer_alloc(FUNC(mcd251_device::frame_period_tick), this);

	m_picture_fifo.reserve(MAX_PICTURES);

	save_pointer(NAME(m_dram), DRAM_WORDS);
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
	m_demux.register_save_state(*this);
}

void mcd251_device::device_reset()
{
	// power-on state of the chip
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

	m_demux.reset();
	m_video_startcode_shift = 0;

	clear_video_fifo();
	m_video->clear();

	m_irq_state = false;
	m_irq_cb(0);

	// The TIM interrupt has to land at exactly 100.446428... Hz for the
	// host's SCR bookkeeping to add up: 45000 / ((55 + 1) * 8) = 100.446...
	m_tim_timer->adjust(attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ),
			0, attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ));
}

//**************************************************************************
//  CLOCKS
//**************************************************************************

uint32_t mcd251_device::raw_dclk() const
{
	return uint32_t((machine().time() - m_dclk_origin).as_ticks(DCLK_HZ));
}

uint32_t mcd251_device::fmv_dclk() const
{
	return raw_dclk() + m_fmv_dclk_offset;
}

//**************************************************************************
//  PICTURE DRAM
//**************************************************************************

// The pictures this emulation reconstructs live in the decoder rather than
// in the DRAM, so what the host writes here it reads back unchanged.
uint16_t mcd251_device::dram_r(offs_t offset, uint16_t mem_mask)
{
	return m_dram[offset & (DRAM_WORDS - 1)];
}

void mcd251_device::dram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dram[offset & (DRAM_WORDS - 1)]);
}

//**************************************************************************
//  STREAM INPUT
//**************************************************************************

// A byte of the system stream, from a transfer or from a host write.
void mcd251_device::write_data(uint8_t data)
{
	const bool was_body = m_demux.packet_body;
	m_demux.byte(data, m_fmv_stream);

	if (was_body)
	{
		m_video_es.push_back(data);

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

	if (m_demux.dts_updated)
	{
		m_next_picture_dts = m_demux.dts;
		// GEN_VDI_CMD bit 14 flags the arrival of a DTS
		m_fmv_vdi_cmd |= 1 << 14;
	}

	if (m_demux.program_end)
	{
		// End of the program: nothing follows, so the reference picture
		// held back for reordering is shown too, or the last PIC event
		// never fires and the driver waits for it forever.
		video_decode_pending();
		if (m_store_decoding < 0)
			release_held_reference();

		m_fmv_isr |= FMV_IRQ_EII;
		update_intreq();
	}
}

void mcd251_device::end_of_transfer(bool dma)
{
	video_decode_pending();
}

//**************************************************************************
//  VIDEO DECODE
//**************************************************************************

void mcd251_device::video_decode_pending()
{
	if (!m_fmv_dsp_enable)
		return;

	m_video_needs_data = false;
	size_t taken = 0;
	while (m_picture_fifo.size() < MAX_PICTURES)
	{
		std::size_t consumed = 0;
		int width = 0, height = 0;
		double frame_rate = 0;
		mpeg_video::decode_result result = m_video->decode(
				std::span<const uint8_t>(m_video_es.data() + taken, m_video_es.size() - taken),
				consumed, video_buffers(), width, height, frame_rate);
		taken += consumed;

		// The decoder takes the start code after a picture as its end, but the
		// chip ends a picture on its last macroblock: a stream that stops
		// there, or a lone still, must not wait for more data.
		if (result == mpeg_video::decode_result::NEED_DATA && m_store_decoding >= 0)
			result = m_video->complete_picture(video_buffers(), width, height, frame_rate);

		switch (result)
		{
		case mpeg_video::decode_result::PICTURE_HEADER:
			video_picture_header(width, height);
			break;

		case mpeg_video::decode_result::PICTURE:
			video_picture_decoded();
			if (m_video->picture_ends_sequence())
				video_sequence_end();
			break;

		case mpeg_video::decode_result::SEQUENCE_END:
			video_sequence_end();
			break;

		case mpeg_video::decode_result::INVALID_DATA:
			LOGMASKED(LOG_VIDEO, "FMV invalid video data skipped\n");
			m_store_decoding = -1;
			break;

		case mpeg_video::decode_result::NEED_DATA:
			// see "request for bits"
			m_video_needs_data = true;
			break;
		}

		if (m_video_needs_data)
			break;
	}
	m_video_es.erase(m_video_es.begin(), m_video_es.begin() + taken);
}

void mcd251_device::video_picture_header(int width, int height)
{
	const mpeg_video::picture_type type = m_video->coding_type();

	// A new picture size replaces the stores, so show the held reference
	// picture while its size still applies.
	if (m_store_newest >= 0 && (m_store[m_store_newest].info.width != width
			|| m_store[m_store_newest].info.height != height))
	{
		release_held_reference();
		m_store_newest = m_store_older = -1;
	}

	if (type == mpeg_video::picture_type::B)
	{
		m_store_decoding = 2;
		m_store_backward = m_store_newest;
		m_store_forward = (m_store_older >= 0) ? m_store_older : m_store_newest;
	}
	else
	{
		m_store_decoding = (m_store_newest == 0) ? 1 : 0;
		m_store_forward = m_store_newest;
		m_store_backward = -1;
	}
	// a P or B picture with nothing to predict from is decoded against itself
	if (type != mpeg_video::picture_type::I && m_store_forward < 0)
		m_store_forward = m_store_decoding;
	if (type == mpeg_video::picture_type::B && m_store_backward < 0)
		m_store_backward = m_store_decoding;

	// every sequence header flags the next intra picture, which the driver
	// sees as SEQ once it is displayed
	m_pending_seq |= m_video->picture_follows_sequence_header();
	m_pending_gop |= m_video->picture_follows_sequence_header() || m_video->picture_follows_group();

	picture_info &info = m_store[m_store_decoding].info;
	info.width = uint16_t(width);
	info.height = uint16_t(height);
	// SYS_VSR @ 0xe0405c carries the temporal reference and picture type
	info.video_status = uint8_t(((int(type) & 0x03) << 6) | (m_video->temporal_reference() & 0x3f));
	info.timecode = pack_timecode(m_video->time_code());
	info.frameperiod_rawhdr = m_video->picture_rate_code() & 0x0f;
	info.first_intra_of_gop = false;
	info.first_intra_of_seq = false;
	if (type == mpeg_video::picture_type::I)
	{
		info.first_intra_of_gop = m_pending_gop;
		info.first_intra_of_seq = m_pending_seq;
		m_pending_gop = m_pending_seq = false;
	}
}

void mcd251_device::video_picture_decoded()
{
	const int decoded = m_store_decoding;
	m_store_decoding = -1;

	const picture_info &info = m_store[decoded].info;
	m_decoder_width = info.width;
	m_decoder_height = info.height;
	m_decoder_timecode = info.timecode;
	m_decoder_frameperiod_rawhdr = info.frameperiod_rawhdr;
	m_decoder_frameperiod_90khz = FRAME_PERIOD_90KHZ[info.frameperiod_rawhdr];
	m_frame_period_30mhz = FRAME_PERIOD_30MHZ[info.frameperiod_rawhdr];

	if (m_fmv_playback_active)
	{
		m_image_width = m_decoder_width;
		m_image_height = m_decoder_height;
		m_image_rt = m_decoder_frameperiod_rawhdr;
	}

	LOGMASKED(LOG_VIDEO, "decoded picture %dx%d type %d tref %d\n",
			info.width, info.height, info.video_status >> 6, info.video_status & 0x3f);

	if (decoded == 2)
	{
		queue_picture(m_store[decoded]);
	}
	else
	{
		release_held_reference();
		m_store_older = m_store_newest;
		m_store_newest = decoded;
		m_reference_held = true;
	}
}

void mcd251_device::video_sequence_end()
{
	release_held_reference();
	m_fmv_isr |= FMV_IRQ_ESI;
	LOGMASKED(LOG_VIDEO, "FMV sequence end\n");
	update_intreq();
}

void mcd251_device::release_held_reference()
{
	if (m_reference_held)
	{
		m_reference_held = false;
		queue_picture(m_store[m_store_newest]);
	}
}

void mcd251_device::queue_picture(const picture_store &store)
{
	picture pic;
	pic.info = store.info;
	const int width = pic.info.width, height = pic.info.height;
	pic.rgb = std::make_unique<uint32_t []>(size_t(width) * height);

	const int luma_pitch = (width + 15) & ~15;
	const int chroma_pitch = luma_pitch / 2;
	const uint8_t *const y_plane = store.ycbcr.get();
	const uint8_t *const cb_plane = y_plane + size_t(luma_pitch) * ((height + 15) & ~15);
	const uint8_t *const cr_plane = cb_plane + size_t(chroma_pitch) * (((height + 15) & ~15) / 2);

	// ITU-R BT.601, studio range, in 16.16 fixed point
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const int c = (y >> 1) * chroma_pitch + (x >> 1);
			const int cr = cr_plane[c] - 128;
			const int cb = cb_plane[c] - 128;
			const int r = (cr * 104597) >> 16;
			const int g = (cb * 25674 + cr * 53278) >> 16;
			const int b = (cb * 132201) >> 16;
			const int luma = ((y_plane[y * luma_pitch + x] - 16) * 76309) >> 16;
			pic.rgb[size_t(y) * width + x] = rgb_t(
					std::clamp(luma + r, 0, 255),
					std::clamp(luma - g, 0, 255),
					std::clamp(luma + b, 0, 255));
		}
	}

	m_picture_fifo.push_back(std::move(pic));
	if (m_pictures_in_input_fifo)
		m_pictures_in_input_fifo--;
}

void mcd251_device::video_flush_stalled()
{
	if (!m_fmv_dsp_enable || m_store_decoding >= 0 || !m_reference_held)
		return;

	LOGMASKED(LOG_VIDEO, "FMV flush: held picture released\n");
	release_held_reference();
}

void mcd251_device::reset_video_decoder()
{
	m_video->reset_input();
	m_video_es.clear();
	m_store_newest = m_store_older = -1;
	m_store_decoding = m_store_forward = m_store_backward = -1;
	m_reference_held = false;
	m_pending_seq = m_pending_gop = false;
}

void mcd251_device::clear_video_fifo()
{
	reset_video_decoder();
	m_video_needs_data = false;
	m_video_es.clear();

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

mpeg_video::picture_buffers mcd251_device::video_buffers() const
{
	const auto buffer = [this] (int index)
	{
		return (index >= 0)
				? mpeg_video::picture_buffer{ m_store[index].ycbcr.get(), unsigned(PICTURE_STORE_BYTES) }
				: mpeg_video::picture_buffer{ nullptr, 0 };
	};
	return mpeg_video::picture_buffers{ buffer(m_store_decoding), buffer(m_store_forward), buffer(m_store_backward) };
}

uint32_t mcd251_device::pack_timecode(uint32_t gop_timecode)
{
	const uint32_t frames  = gop_timecode & 0x3f;
	const uint32_t seconds = (gop_timecode >> 6) & 0x3f;
	const uint32_t minutes = (gop_timecode >> 13) & 0x3f;
	const uint32_t hours   = (gop_timecode >> 19) & 0x1f;

	return (hours << 6) | minutes | (seconds << 22) | (frames << 16);
}

//**************************************************************************
//  FRAME DISPLAY PACING
//**************************************************************************

void mcd251_device::restart_frame_timer()
{
	if (!m_fmv_playback_active)
	{
		m_frame_timer->adjust(attotime::never);
		return;
	}

	// The picture period less 2048 ticks per unit of DTS desync, in 24 bits:
	// the correction has to wrap, since computing it wider let a large
	// negative desync stretch a frame past a second and stall the display.
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

TIMER_CALLBACK_MEMBER(mcd251_device::frame_period_tick)
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
				&& mpeg_timestamp_diff(m_demux.scr, m_dts_fifo.front()) > 15000;

		m_display_dts_desync =
				(m_syscr_written && synchronous && valid_dts && !way_out) ? desync : 0;

		// Correct the frame period rather than skipping a picture: the queue
		// empties faster than the disc fills it, and the NDAT that follows
		// makes the driver pause, which ends playback.
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

void mcd251_device::latch_display_frame()
{
	if (m_picture_fifo.empty())
		return;

	m_display = std::move(m_picture_fifo.front());
	m_picture_fifo.erase(m_picture_fifo.begin());
	m_display_valid = true;

	if (!m_dts_fifo.empty())
		m_dts_fifo.erase(m_dts_fifo.begin());

	m_display_info = m_display.info;
}

void mcd251_device::vblank_w(int state)
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
				// Underflow is reported as the last picture is taken with
				// nothing behind it, counting the pictures whose start code
				// has arrived but which the decoder has not handed back.
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
			if (m_display_info.first_intra_of_gop)
				m_fmv_isr |= FMV_IRQ_GOP;
			if (m_display_info.first_intra_of_seq)
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

// x and y are the chip's own pixel coordinates, counted from the start of
// active video.
bool mcd251_device::video_pixel(int x, int y, uint32_t &argb) const
{
	if (!m_show_video || !m_display_valid || !m_display.rgb)
	{
		return false;
	}

	const int sx = x - int(m_latched_display_offset_x);
	const int sy = y - int(m_latched_display_offset_y);

	if (sx < 0 || sy < 0)
		return false;
	if (m_latched_window_width && sx >= int(m_latched_window_width))
		return false;
	if (m_latched_window_height && sy >= int(m_latched_window_height))
		return false;

	const int px = sx + int(m_latched_window_offset_x);
	const int py = sy + int(m_latched_window_offset_y);

	if (px < 0 || py < 0 || px >= int(m_display.info.width) || py >= int(m_display.info.height))
		return false;

	argb = m_display.rgb[size_t(py) * m_display.info.width + px];
	return true;
}

//**************************************************************************
//  INTERRUPTS
//**************************************************************************

void mcd251_device::log_fmv_isr(const char *where)
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

void mcd251_device::update_intreq()
{
	log_fmv_isr("update");

	const bool req = irq_pending();
	if (req != m_irq_state)
	{
		m_irq_state = req;
		LOGMASKED(LOG_IRQ, "MCD251 irq %d (isr %04x ier %04x)\n", req, m_fmv_isr, m_fmv_ier);
		m_irq_cb(req ? 1 : 0);
	}
}

TIMER_CALLBACK_MEMBER(mcd251_device::tim_tick)
{
	m_fmv_isr |= FMV_IRQ_TIM;
	m_timer_cb(1);
	update_intreq();
}

TIMER_CALLBACK_MEMBER(mcd251_device::start_video_tick)
{
	m_fmv_playback_active = true;
	restart_frame_timer();
}

TIMER_CALLBACK_MEMBER(mcd251_device::pause_video_tick)
{
	m_fmv_isr |= FMV_IRQ_PAI;
	update_intreq();
}

//**************************************************************************
//  REGISTERS
//**************************************************************************

uint16_t mcd251_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0x001: data = m_image_width; break;   // T_PWI
	case 0x002: data = m_image_height; break;  // T_PHE
	case 0x003: data = m_image_rt; break;      // T_PRPA
	// 0x2004/0x2005 report the *decoder* timecode, 0x202c/0x202d the
	// *displayed* one: the FMV driver's MVS_TimeCd reads SYS_TCL at 0x0e04058
	// as the timecode of the current picture.
	case 0x004: data = uint16_t(m_decoder_timecode >> 16); break;
	case 0x005: data = uint16_t(m_decoder_timecode); break;

	case 0x029: data = m_display_info.width; break;
	case 0x02a: data = m_display_info.height; break;
	case 0x02b: data = m_display_info.frameperiod_rawhdr; break;
	case 0x02c: data = uint16_t(m_display_info.timecode >> 16); break;
	case 0x02d: data = uint16_t(m_display_info.timecode); break;
	case 0x02e: data = m_display_info.video_status; break;

	case 0x02f:
		// SYS_STS bit 13, "request for bits", follows the input FIFO alone:
		// gating it on the picture queue, or dropping it while the decoder is
		// starved mid picture, stops the driver feeding, and the CD play
		// buffers are then never released.
		data = (m_video_es.size() < VIDEO_FIFO_FULL || m_video_needs_data)
				? 0x2000 : 0;
		break;

	case 0x030: data = m_fmv_ier; break;

	case 0x031:
		data = m_fmv_isr;
		if (!machine().side_effects_disabled())
		{
			// reading the interrupt status register clears it
			m_fmv_isr = 0;
			update_intreq();
		}
		break;

	case 0x032: data = m_fmv_tcnt; break;

	case 0x036: data = m_video_ctrl_y_offset; break;  // Yo
	case 0x037: data = m_video_ctrl_x_offset; break;  // Xo
	case 0x038: data = m_video_ctrl_y_active; break;  // Ya
	case 0x039: data = m_video_ctrl_x_active; break;  // Xa
	case 0x03a: data = m_video_ctrl_y_display; break;  // Yd
	case 0x03b: data = m_video_ctrl_x_display; break;  // Xd
	case 0x03c: data = m_video_ctrl_window_height; break;  // Wh
	case 0x03d: data = m_video_ctrl_window_width; break;  // Ww
	case 0x03e: data = m_video_ctrl_decoder_offset_y; break;  // Yw
	case 0x03f: data = m_video_ctrl_decoder_offset_x; break;  // Xw

	case 0x044: data = m_fmv_dec_cmd; break;
	case 0x046: data = m_fmv_vdi_cmd; break;
	case 0x049: data = 0; break; // GEN_VID_BUF
	case 0x04c:
		// GEN_SYSCR: bits 20:6 of the decoder clock, with bit 21 masked off.
		// Returning it lands in bit 15, which the driver reads as a negative
		// SCR, so playback died 46 s after reset whatever the stream.
		data = uint16_t((fmv_dclk() >> 6) & 0x7fff);
		break;
	case 0x04e: data = 0; break; // GEN_SYNC_DIFF, always 0 on real hardware
	case 0x04f: data = 1; break; // GEN_DEC_DELAY

	case 0x050:
		// GEN_DEC_TIM1: the demuxer DTS as the CPU wants it, at 703.125 Hz
		data = uint16_t((m_demux.dts >> 7) & 0x7fff);
		break;

	case 0x052:
		// pictures queued.  Before playback starts only the DTS fifo depth is
		// reported, as for a decoder that has not started yet.
		data = m_fmv_playback_active
				? uint16_t(m_picture_fifo.size() + m_pictures_in_input_fifo)
				: uint16_t(m_dts_fifo.size());
		break;

	case 0x054: data = m_decoder_frameperiod_90khz; break;
	case 0x055: data = m_pal ? DISPLAY_RATE_PAL : DISPLAY_RATE_NTSC; break;
	case 0x056: data = m_fmv_frame_rate; break;
	case 0x060: data = m_fmv_syscmd; break;
	case 0x061: data = m_fmv_vidcmd; break;
	case 0x062: data = m_fmv_stream; break;
	case 0x06e: data = m_fmv_ivec; break;
	case 0x073: data = 0; break; // MMU base pointer, read once and ignored
	default:
		break;
	}

	LOGMASKED(LOG_REGS_R, "%s: mcd251_r: %03x = %04x & %04x\n", machine().describe_context(),
			offset, data, mem_mask);

	return data;
}

void mcd251_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS_W, "%s: mcd251_w: %03x = %04x & %04x\n", machine().describe_context(),
			offset, data, mem_mask);

	switch (offset)
	{
	case 0x030:
		LOGMASKED(LOG_FMV, "FMV IER %04x\n", data);
		m_fmv_ier = data;
		update_intreq();
		break;

	case 0x031:
		break;

	case 0x032: // TCNT
		LOGMASKED(LOG_FMV, "FMV TCNT %04x\n", data);
		m_fmv_tcnt = data;
		{
			const attotime period = attotime::from_ticks(((data & 0xffff) + 1) * 8, DCLK_HZ);
			m_tim_timer->adjust(period, 0, period);
		}
		break;

	case 0x057: // TRLD
		LOGMASKED(LOG_FMV, "FMV TRLD %04x\n", data);
		{
			const attotime period = attotime::from_ticks((m_fmv_tcnt + 1) * 8, DCLK_HZ);
			m_tim_timer->adjust(period, 0, period);
		}
		break;

	case 0x060: // SYSCMD
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
			m_demux.reset();
			// the decoder is briefly disabled, then enabled again
			m_fmv_dsp_enable = true;
		}

		if (data & SYSCMD_GOPSEARCH)
		{
			// handled implicitly: the decoder resumes at the next picture
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
			m_display_info.video_status = 0;
			m_display_info.timecode = 0;
			m_decoder_timecode = 0;
			m_decoder_frameperiod_90khz = 0;
			m_decoder_frameperiod_rawhdr = 0;
			m_display_valid = false;
			clear_video_fifo();
			LOGMASKED(LOG_FMV, "FMV decoder off\n");
		}

		if (data & SYSCMD_DMA)
		{
			m_drq_cb(1);
		}
		break;

	case 0x061: // VIDCMD
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

	case 0x063: // SYS_SCR
		LOGMASKED(LOG_FMV, "FMV SYSSCR %04x\n", data);
		m_fmv_sysscr = data;
		break;

	case 0x06e:
		LOGMASKED(LOG_FMV, "FMV IVEC %04x\n", data);
		m_fmv_ivec = data;
		break;

	case 0x036: m_video_ctrl_y_offset = data; break;             // always 0x001a (Yo)
	case 0x037: m_video_ctrl_x_offset = data; break;             // always 0x004a (Xo)
	case 0x038: m_video_ctrl_y_active = data; break;  // Ya
	case 0x039: m_video_ctrl_x_active = data; break;  // Xa
	case 0x03a: m_video_ctrl_y_display = data; break;            // FMV_SCRPOS Y (Yd)
	case 0x03b: m_video_ctrl_x_display = data; break;            // FMV_SCRPOS X (Xd)
	case 0x03c: m_video_ctrl_window_height = data; break;        // FMV_DECWIN H (Wh)
	case 0x03d: m_video_ctrl_window_width = data; break;         // FMV_DECWIN W (Ww)
	case 0x03e: m_video_ctrl_decoder_offset_y = data; break;     // FMV_DECOFF Y (Yw)
	case 0x03f: m_video_ctrl_decoder_offset_x = data; break;     // FMV_DECOFF X (Xw)

	case 0x044: // GEN_DEC_CMD
		m_fmv_dec_cmd = data;
		if ((data >> 8) == 0x22 && !(m_fmv_syscmd & SYSCMD_PAUSE))
			m_single_step_latch = true;
		break;

	case 0x046:
		m_fmv_vdi_cmd = data;
		break;

	case 0x049: // GEN_VID_BUF
		break;

	case 0x04c: // GEN_SYSCR, bits 20:6 (bit 21 must stay clear, see the read)
		LOGMASKED(LOG_FMV, "FMV GEN_SYSCR %04x\n", data);
		m_fmv_dclk_offset = int32_t(((uint32_t(data) & 0x7fff) << 6)
				- (raw_dclk() & 0x001fffc0));
		m_syscr_written = true;
		break;

	case 0x056:
		m_fmv_frame_rate = data;
		break;

	case 0x001: m_image_width = data; break;
	case 0x002: m_image_height = data; break;
	case 0x003: m_image_rt = data; break;

	case 0x062: // stream number, 0-15 per mv_selstrm()
		LOGMASKED(LOG_FMV, "FMV stream %04x\n", data);
		m_fmv_stream = data & 0x0f;
		break;
	default:
		break;
	}
}
