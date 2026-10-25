// license:BSD-3-Clause
// copyright-holders:Matt Phelps
/***************************************************************************

    LSI Logic L64111 MPEG audio decoder

    Reference: LSI Logic, "L64111 MPEG Audio Decoder Technical Manual",
    MD72-000101-99 A, September 1993 (preliminary; chip revision B).

    MPEG-1 Layer I and II decoder with an ISO 11172 system stream parser.
    Data written through the controller interface goes into a 128-byte input
    FIFO (FEMPTY/FALE/FALF flags). The pre-parser drains it at 15 Mbit/s at a
    30 MHz SYSCLK, extracts the audio frames of the selected stream (capturing
    presentation time stamps), and stores them in the channel buffer: an
    external 256K x 4 DRAM holding 128 Layer I or 64 Layer II frames. Once
    started, the decoder takes frames from the channel buffer and outputs 16-bit
    PCM at the rate set by the PCM timer (TCR + TORH/TORL against SYSCLK), not
    the rate in the frame headers.

    Registers (A[4:0]):
     0  Data            W   MPEG data, 8 or 16 bits (CR2 SEL_16/8_BIT)
     1  Control 1       RW  7 ANC FIFO RST, 6 SER/PAR, 5 BYPASS, 2 BUF_RST, 1 SOFT_RST, 0 STARTI
     2  Control 2       RW  7-6 MUTE (error response), 5 SOFT_MUTE, 4 SEL_16/8_BIT, 3 AUDIO ONLY,
                            2 I2S, 1-0 PCM MODE
     3  Control 3       RW  7 AUDIO_STREAM_ID_IGNORE, 4-0 AUDIO_STREAM_IDENTIFIER
     4  Int mask 1 (W) / status 1 (R): 7 ANC DATA VALID, 6 ANC FIFO OVRFL, 5 ANC FIFO HFF, 3 ERR BUF OVFL
     5  Int mask 2 (W): 7 SYNTAX ERR, 6 PTS, 5 SYNC AUD, 4 SYNC SYS, 3 FRAME DETECT IN, 2 CRC ERR,
                        1 NEW FRAME OUT, 0 LAST FRAME
        status 2 (R):   6 SYNTAX ERR, 5 PTS, 4 SYNC AUD, 3 SYNC SYS, 2 FRAME DETECT IN, 1 CRC ERR,
                        0 NEW FRAME OUT/LAST FRAME
        Reading either status register deasserts INTR.
     6  TCR             RW  PCM timer terminal count (>= 6)
     7  TORH, 8 TORL    RW  PCM timer fraction
     9-11 Parametric data words: the last frame header after the sync word
    12-16 PTS
    17  Ancillary/user data FIFO
    18  Channel buffer status counter: frames stored
    19  Channel buffer write counter, 20 read counter (frame pointers, for cue and review)

    Not emulated: serial input and bypass, CRC checking (CRC ERR never sets),
    free-format bitstreams, and the A/D pin strapping of stand-alone systems.
    Ancillary data enters its FIFO when the frame is decoded rather than when
    the pre-parser stores it. A frame that fails to decode plays as silence
    unless MUTE = 10 (replay the last good frame).

***************************************************************************/

#include "emu.h"
#include "l64111.h"

#include <algorithm>
#include <memory>

#define LOG_REGS  (1U << 1)
#define LOG_INTR  (1U << 2)
#define LOG_FRAME (1U << 3)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(L64111, l64111_device, "l64111", "LSI Logic L64111 MPEG audio decoder")

namespace {

// Layer I and II bit rates in kbit/s by bit_rate_index (manual Parametric Data Word 1)
constexpr u16 BITRATE[2][16] = {
	{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0 },
	{ 0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0 }
};
constexpr u32 SAMPLE_RATE[4] = { 44'100, 48'000, 32'000, 0 };

// ISO 11172-1 start codes (after 00 00 01)
constexpr u8 END_CODE = 0xb9, PACK_START = 0xba, SYSTEM_HEADER = 0xbb, AUDIO_STREAM_FIRST = 0xc0, AUDIO_STREAM_LAST = 0xdf;

} // anonymous namespace


l64111_device::l64111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, L64111, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	m_intr_cb(*this), m_fempty_cb(*this), m_fale_cb(*this), m_falf_cb(*this), m_starto_cb(*this),
	m_stream(nullptr),
	m_frame_timer(nullptr), m_flags_timer(nullptr),
	m_reset_pin(false), m_starti_pin(true), m_hdmute_pin(false),
	m_intr_state(1), m_fempty_state(0), m_fale_state(0), m_falf_state(1), m_starto_state(0),
	m_overflow_logged(false),
	m_sys_code(0), m_sys_hdr{},
	m_frame{},
	m_cb{}, m_cb_len{},
	m_decode{}, m_pcm{}, m_pcm_channels(1),
	m_fifo_save{}, m_anc_save{}, m_fifo_save_count(0), m_anc_save_count(0)
{
}

void l64111_device::device_start()
{
	if (!clock())
		logerror("no SYSCLK configured: the input FIFO will not drain and the PCM rate stays at %u Hz\n", DEFAULT_RATE);
	m_stream = stream_alloc(0, 2, DEFAULT_RATE);
	m_decoder = std::make_unique<mpeg_audio>(m_decode, mpeg_audio::L1 | mpeg_audio::L2, false, 0);
	m_decoder->register_save_state(*this);
	m_frame_timer = timer_alloc(FUNC(l64111_device::frame_tick), this);
	m_flags_timer = timer_alloc(FUNC(l64111_device::flags_tick), this);

	save_item(NAME(m_cr1));
	save_item(NAME(m_cr2));
	save_item(NAME(m_cr3));
	save_item(NAME(m_mask1));
	save_item(NAME(m_mask2));
	save_item(NAME(m_status1));
	save_item(NAME(m_status2));
	save_item(NAME(m_tcr));
	save_item(NAME(m_torh));
	save_item(NAME(m_torl));
	save_item(NAME(m_pd));
	save_item(NAME(m_pts));
	save_item(NAME(m_reset_pin));
	save_item(NAME(m_starti_pin));
	save_item(NAME(m_hdmute_pin));
	save_item(NAME(m_intr_pending));
	save_item(NAME(m_intr_state));
	save_item(NAME(m_fempty_state));
	save_item(NAME(m_fale_state));
	save_item(NAME(m_falf_state));
	save_item(NAME(m_starto_state));
	save_item(NAME(m_fifo_save));
	save_item(NAME(m_fifo_save_count));
	save_item(NAME(m_fifo_time));
	save_item(NAME(m_sys_state));
	save_item(NAME(m_sys_shift));
	save_item(NAME(m_sys_shift_count));
	save_item(NAME(m_sys_code));
	save_item(NAME(m_sys_left));
	save_item(NAME(m_sys_length_bytes));
	save_item(NAME(m_sys_hdr));
	save_item(NAME(m_sys_hdr_len));
	save_item(NAME(m_sys_hdr_need));
	save_item(NAME(m_pack_count));
	save_item(NAME(m_frame));
	save_item(NAME(m_frame_len));
	save_item(NAME(m_frame_need));
	save_item(NAME(m_sync_count));
	save_item(NAME(m_cb));
	save_item(NAME(m_cb_len));
	save_item(NAME(m_cb_write));
	save_item(NAME(m_cb_read));
	save_item(NAME(m_cb_count));
	save_item(NAME(m_cb_layer));
	save_item(NAME(m_anc_save));
	save_item(NAME(m_anc_save_count));
	save_item(NAME(m_anc_acc));
	save_item(NAME(m_anc_bits));
	save_item(NAME(m_anc_overflow));
	save_item(NAME(m_decode));
	save_item(NAME(m_pcm));
	save_item(NAME(m_pcm_pos));
	save_item(NAME(m_pcm_count));
	save_item(NAME(m_pcm_channels));
	save_item(NAME(m_started));
	save_item(NAME(m_underrun));
	save_item(NAME(m_ev1));
	save_item(NAME(m_ev2));

	// a device configured earlier can drive RESET, STARTI or HDMUTE from its own reset, before ours
	reset_state();
}

void l64111_device::device_reset()
{
	chip_reset();
	m_intr_cb(m_intr_state);
	m_fempty_cb(m_fempty_state);
	m_fale_cb(m_fale_state);
	m_falf_cb(m_falf_state);
	m_starto_cb(m_starto_state);
}

// util::fifo has no save state support: copy the FIFOs out and back
void l64111_device::device_pre_save()
{
	m_fifo_save_count = m_fifo.queue_length();
	for (u32 i = 0; i < m_fifo_save_count; i++)
	{
		m_fifo_save[i] = m_fifo.dequeue();
		m_fifo.enqueue(m_fifo_save[i]);
	}
	m_anc_save_count = m_anc.queue_length();
	for (u32 i = 0; i < m_anc_save_count; i++)
	{
		m_anc_save[i] = m_anc.dequeue();
		m_anc.enqueue(m_anc_save[i]);
	}
}

void l64111_device::device_post_load()
{
	m_fifo.clear();
	for (u32 i = 0; i < std::min<u32>(m_fifo_save_count, FIFO_BYTES); i++)
		m_fifo.enqueue(m_fifo_save[i]);
	m_anc.clear();
	for (u32 i = 0; i < std::min<u32>(m_anc_save_count, ANC_BYTES); i++)
		m_anc.enqueue(m_anc_save[i]);
}

void l64111_device::device_clock_changed()
{
	update_rate();
}

// RESET pin or SOFT_RST: registers to their defaults, FIFOs and channel buffer emptied
void l64111_device::chip_reset()
{
	reset_state();
	update_irq_state(0, 0);
	update_outputs();
	update_frame_timer();
}

void l64111_device::reset_state()
{
	m_cr1 = m_starti_pin ? CR1_STARTI : 0x00; // STARTI defaults to the STARTI pin
	m_cr2 = 0; // AUDIO ONLY defaults to address line A4, which a controller-driven design leaves undefined
	m_cr3 = CR3_ID_IGNORE;
	m_mask1 = m_mask2 = 0;
	m_status1 = m_status2 = 0;
	m_tcr = m_torh = m_torl = 0; // latched from A[3:0]/D[15:0] at reset; a controller rewrites them
	std::fill(std::begin(m_pd), std::end(m_pd), 0);
	std::fill(std::begin(m_pts), std::end(m_pts), 0);

	m_fifo.clear();
	m_fifo_time = machine().time();
	m_flags_timer->adjust(attotime::never);
	parser_reset();
	m_cb_write = m_cb_read = m_cb_count = 0;
	m_cb_layer = 2;
	m_anc.clear();
	m_anc_acc = m_anc_bits = 0;
	m_anc_overflow = false;
	m_ev1 = m_ev2 = 0;
	decoder_reset();
	m_intr_pending = false;
}

// STARTI low resets the decoder block; the channel buffer keeps its frames
void l64111_device::decoder_reset()
{
	m_started = false;
	m_underrun = false;
	m_pcm_pos = m_pcm_count = 0;
	m_decoder->clear();
}

void l64111_device::parser_reset()
{
	m_sys_state = SYS_SCAN;
	m_sys_shift = m_sys_shift_count = 0;
	m_sys_left = m_sys_length_bytes = 0;
	m_sys_hdr_len = m_sys_hdr_need = 0;
	m_pack_count = 0;
	m_frame_len = m_frame_need = 0;
	m_sync_count = 0;
}


//**************************************************************************
//  Controller interface
//**************************************************************************

void l64111_device::map(address_map &map)
{
	map(0x00, 0x1f).rw(FUNC(l64111_device::reserved_r), FUNC(l64111_device::reserved_w));
	map(0x00, 0x00).w(FUNC(l64111_device::data8_w));
	map(0x01, 0x01).rw(FUNC(l64111_device::cr1_r), FUNC(l64111_device::cr1_w));
	map(0x02, 0x02).rw(FUNC(l64111_device::cr2_r), FUNC(l64111_device::cr2_w));
	map(0x03, 0x03).rw(FUNC(l64111_device::cr3_r), FUNC(l64111_device::cr3_w));
	map(0x04, 0x04).rw(FUNC(l64111_device::status1_r), FUNC(l64111_device::mask1_w));
	map(0x05, 0x05).rw(FUNC(l64111_device::status2_r), FUNC(l64111_device::mask2_w));
	map(0x06, 0x06).rw(FUNC(l64111_device::tcr_r), FUNC(l64111_device::tcr_w));
	map(0x07, 0x07).rw(FUNC(l64111_device::torh_r), FUNC(l64111_device::torh_w));
	map(0x08, 0x08).rw(FUNC(l64111_device::torl_r), FUNC(l64111_device::torl_w));
	map(0x09, 0x0b).r(FUNC(l64111_device::pd_r));
	map(0x0c, 0x10).r(FUNC(l64111_device::pts_r));
	map(0x11, 0x11).r(FUNC(l64111_device::anc_r));
	map(0x12, 0x12).r(FUNC(l64111_device::cb_status_r));
	map(0x13, 0x13).rw(FUNC(l64111_device::cb_write_r), FUNC(l64111_device::cb_write_w));
	map(0x14, 0x14).rw(FUNC(l64111_device::cb_read_r), FUNC(l64111_device::cb_read_w));
	map(0x09, 0x12).w(FUNC(l64111_device::read_only_w));
}

// registers read as 0 and ignore writes while RESET is held; otherwise bring the chip up to date first
bool l64111_device::reg_read_ready()
{
	if (m_reset_pin)
		return false;
	if (!machine().side_effects_disabled())
		sync();
	return true;
}

bool l64111_device::reg_write_ready(const char *name, u8 data)
{
	if (m_reset_pin)
		return false;
	LOGMASKED(LOG_REGS, "%s: %s = %02x\n", machine().describe_context(), name, data);
	sync();
	return true;
}

u8 l64111_device::reserved_r()
{
	return 0; // also the Data register, which is write-only
}

void l64111_device::reserved_w(offs_t offset, u8 data)
{
	logerror("%s: write %02x to reserved register %02x\n", machine().describe_context(), data, offset);
}

void l64111_device::read_only_w(offs_t offset, u8 data)
{
	logerror("%s: write %02x to read-only register %02x\n", machine().describe_context(), data, offset + 9);
}

void l64111_device::data8_w(u8 data)
{
	data_w(data);
}

u8 l64111_device::cr1_r()
{
	return reg_read_ready() ? m_cr1 : 0;
}

void l64111_device::cr1_w(u8 data)
{
	if (!reg_write_ready("CR1", data))
		return;
	if (data & CR1_SOFT_RST) // self-clearing
	{
		chip_reset();
		return;
	}
	m_cr1 = data; // SOFT_RST is never stored; the chip doesn't clear BUF_RST (manual 4.3)
	if (data & CR1_ANC_RST) // empty the ancillary FIFO, and hold it empty while set
	{
		m_anc.clear();
		m_anc_acc = m_anc_bits = 0;
		m_anc_overflow = false;
	}
	if (data & CR1_BUF_RST) // empty the channel buffer, and hold it empty while set (see store_frame)
		m_cb_write = m_cb_read = m_cb_count = 0;
	if (!decoder_enabled())
		decoder_reset();
	if (data & (CR1_SER_PAR | CR1_BYPASS))
		logerror("serial input / bypass selected (CR1 %02x), not emulated\n", data);
	update_outputs();
	update_frame_timer();
}

u8 l64111_device::cr2_r()
{
	return reg_read_ready() ? (m_cr2 | (m_hdmute_pin ? CR2_SOFT_MUTE : 0x00)) : 0; // SOFT_MUTE reads as SOFT_MUTE | HDMUTE
}

void l64111_device::cr2_w(u8 data)
{
	if (!reg_write_ready("CR2", data))
		return;
	if ((m_cr2 ^ data) & CR2_AUDIO_ONLY) // stream type changed: start parsing afresh
		parser_reset();
	m_cr2 = data;
	update_rate();
}

u8 l64111_device::cr3_r()
{
	return reg_read_ready() ? m_cr3 : 0;
}

void l64111_device::cr3_w(u8 data)
{
	if (!reg_write_ready("CR3", data))
		return;
	if ((m_cr3 ^ data) & (CR3_ID_IGNORE | CR3_STREAM_ID)) // a different audio stream: resynchronize
	{
		m_frame_len = m_frame_need = 0;
		m_sync_count = 0;
	}
	m_cr3 = data & (CR3_ID_IGNORE | CR3_STREAM_ID);
}

// reading either status register clears the latched events and INTR
u8 l64111_device::status1_r()
{
	if (!reg_read_ready())
		return 0;
	const u8 v = (!m_anc.empty() ? S1_ANC_VALID : 0x00) | (m_anc_overflow ? S1_ANC_OVRFL : 0x00)
			| ((m_anc.queue_length() >= ANC_BYTES / 2) ? S1_ANC_HFF : 0x00) | (m_status1 & S1_ERR_BUF_OVFL);
	if (!machine().side_effects_disabled())
	{
		m_status1 = 0;
		m_intr_pending = false;
		update_irq_state(0, 0);
	}
	return v;
}

void l64111_device::mask1_w(u8 data)
{
	// a newly enabled mask bit doesn't raise INTR for an event that is already latched [INFERRED]
	if (reg_write_ready("Interrupt Mask 1", data))
		m_mask1 = data;
}

u8 l64111_device::status2_r()
{
	if (!reg_read_ready())
		return 0;
	const u8 v = m_status2 | ((m_sync_count >= SYNC_FRAMES) ? S2_SYNC_AUD : 0x00) | ((m_pack_count >= SYNC_FRAMES) ? S2_SYNC_SYS : 0x00);
	if (!machine().side_effects_disabled())
	{
		m_status2 = 0;
		m_intr_pending = false;
		update_irq_state(0, 0);
	}
	return v;
}

void l64111_device::mask2_w(u8 data)
{
	if (reg_write_ready("Interrupt Mask 2", data)) // see mask1_w
		m_mask2 = data;
}

u8 l64111_device::tcr_r()
{
	return reg_read_ready() ? m_tcr : 0;
}

void l64111_device::tcr_w(u8 data)
{
	if (!reg_write_ready("TCR", data))
		return;
	m_tcr = data & TCR_MASK;
	update_rate();
}

u8 l64111_device::torh_r()
{
	return reg_read_ready() ? m_torh : 0;
}

void l64111_device::torh_w(u8 data)
{
	if (!reg_write_ready("TORH", data))
		return;
	m_torh = data;
	update_rate();
}

u8 l64111_device::torl_r()
{
	return reg_read_ready() ? m_torl : 0;
}

void l64111_device::torl_w(u8 data)
{
	if (!reg_write_ready("TORL", data))
		return;
	m_torl = data;
	update_rate();
}

u8 l64111_device::pd_r(offs_t offset)
{
	return reg_read_ready() ? m_pd[offset] : 0;
}

u8 l64111_device::pts_r(offs_t offset)
{
	return reg_read_ready() ? m_pts[offset] : 0;
}

u8 l64111_device::anc_r()
{
	if (!reg_read_ready() || m_anc.empty())
		return 0;
	if (machine().side_effects_disabled())
		return m_anc.peek();
	m_anc_overflow = false; // a later overflow interrupts again
	return m_anc.dequeue();
}

// 7-bit counters: a full Layer I buffer (128 frames) reads as 127
u8 l64111_device::cb_status_r()
{
	return reg_read_ready() ? std::min<u32>(m_cb_count, CB_COUNTER_MASK) : 0;
}

u8 l64111_device::cb_write_r()
{
	return reg_read_ready() ? (m_cb_write & CB_COUNTER_MASK) : 0;
}

u8 l64111_device::cb_read_r()
{
	return reg_read_ready() ? (m_cb_read & CB_COUNTER_MASK) : 0;
}

// moving the counters leaves the status counter unreliable (manual 4.9): recompute it
void l64111_device::cb_write_w(u8 data)
{
	if (!reg_write_ready("Channel buffer write counter", data))
		return;
	const u32 n = cb_frames();
	m_cb_write = (data & CB_COUNTER_MASK) % n;
	m_cb_count = (m_cb_write + n - m_cb_read) % n;
}

void l64111_device::cb_read_w(u8 data)
{
	if (!reg_write_ready("Channel buffer read counter", data))
		return;
	const u32 n = cb_frames();
	m_cb_read = (data & CB_COUNTER_MASK) % n;
	m_cb_count = (m_cb_write + n - m_cb_read) % n;
}

void l64111_device::data_w(u16 data)
{
	if (m_reset_pin)
		return;
	sync_fifo();
	auto push = [this] (u8 b)
	{
		if (m_fifo.full())
		{
			if (!m_overflow_logged) // the FIFO keeps acknowledging writes and drops the data
				logerror("%s: input FIFO overflow\n", machine().describe_context());
			m_overflow_logged = true;
			return;
		}
		m_fifo.enqueue(b);
	};
	if (m_cr2 & CR2_SEL_16) // 16-bit data: D[15:8] is the earlier byte of the stream
		push(data >> 8);
	push(data);
	update_outputs();
	schedule_flags();
}

int l64111_device::fempty_r()
{
	if (!machine().side_effects_disabled())
		sync_fifo();
	return m_fifo.empty() ? 0 : 1;
}

int l64111_device::fale_r()
{
	if (!machine().side_effects_disabled())
		sync_fifo();
	return (m_fifo.queue_length() <= FIFO_FLAG_LEVEL) ? 0 : 1;
}

int l64111_device::falf_r()
{
	if (!machine().side_effects_disabled())
		sync_fifo();
	return (FIFO_BYTES - m_fifo.queue_length() <= FIFO_FLAG_LEVEL) ? 0 : 1;
}

int l64111_device::starto_r()
{
	return (decoder_enabled() && m_started && !m_underrun) ? 1 : 0;
}

void l64111_device::reset_w(int state)
{
	const bool asserted = state != 0;
	if (asserted == m_reset_pin)
		return;
	m_stream->update();
	m_reset_pin = asserted;
	if (asserted)
		chip_reset();
	update_frame_timer();
}

void l64111_device::starti_w(int state)
{
	sync(); // deliver the frame events up to now before the decoder can be reset
	m_starti_pin = state != 0;
	if (!decoder_enabled())
		decoder_reset();
	update_outputs();
	update_frame_timer();
}

void l64111_device::hdmute_w(int state)
{
	m_stream->update();
	m_hdmute_pin = state != 0;
}


//**************************************************************************
//  Interrupts, flags and timing
//**************************************************************************

// Interrupt events, given in Interrupt Mask 1 / 2 bit positions: latch the Status bits they set and
// assert INTR if their mask bit is set. Status 1 shares Mask 1's positions; Status 2 sits one bit
// lower than Mask 2, with NEW FRAME OUT and LAST FRAME sharing one Status bit. The ancillary FIFO
// bits and the SYNC bits are levels, read from the state rather than latched. The pre-parser and
// the decoder collect their events in m_ev1/m_ev2 for deliver_events(). Reading a status register
// clears INTR and calls this with no events.
void l64111_device::update_irq_state(u8 events1, u8 events2)
{
	m_status1 |= events1 & S1_ERR_BUF_OVFL;
	m_status2 |= ((events2 >> 1) & ~(S2_SYNC_AUD | S2_SYNC_SYS)) | ((events2 & M2_LAST_FRAME) ? S2_NEW_LAST_FRAME : 0x00);
	if ((events1 & m_mask1) || (events2 & m_mask2))
	{
		LOGMASKED(LOG_INTR, "interrupt, status %02x / %02x\n", m_status1, m_status2);
		m_intr_pending = true;
	}
	const int state = m_intr_pending ? 0 : 1;
	if (state != m_intr_state)
	{
		m_intr_state = state;
		m_intr_cb(state);
	}
}

void l64111_device::update_outputs()
{
	auto out = [] (u8 &old, int state, devcb_write_line &cb)
	{
		if (state != old)
		{
			old = state;
			cb(state);
		}
	};
	const u32 count = m_fifo.queue_length();
	out(m_fempty_state, count ? 1 : 0, m_fempty_cb);
	out(m_fale_state, (count <= FIFO_FLAG_LEVEL) ? 0 : 1, m_fale_cb);
	out(m_falf_state, (FIFO_BYTES - count <= FIFO_FLAG_LEVEL) ? 0 : 1, m_falf_cb);
	out(m_starto_state, starto_r(), m_starto_cb);
}

// wake when the draining FIFO next crosses a flag threshold, so the flag outputs change on time
void l64111_device::schedule_flags()
{
	const u32 count = m_fifo.queue_length();
	if (!count)
	{
		m_flags_timer->adjust(attotime::never);
		return;
	}
	u32 bytes = count; // until FEMPTY
	if (count > FIFO_FLAG_LEVEL)
		bytes = count - FIFO_FLAG_LEVEL; // until FALE
	if (FIFO_BYTES - count <= FIFO_FLAG_LEVEL)
		bytes = count - (FIFO_BYTES - FIFO_FLAG_LEVEL - 1); // until FALF clears
	const attotime when = m_fifo_time + clocks_to_attotime(PREPARSE_CLOCKS) * bytes;
	const attotime now = machine().time();
	m_flags_timer->adjust((when > now) ? (when - now) : attotime::zero);
}

// wake just after the next frame boundary, where the decoder takes a frame from the channel buffer
// (NEW FRAME OUT, LAST FRAME, or an underrun), so those events arrive on time
void l64111_device::update_frame_timer()
{
	if (!decoder_enabled() || m_underrun || (!m_started && !m_cb_count))
	{
		m_frame_timer->adjust(attotime::never);
		return;
	}
	const u32 left = (m_pcm_pos < m_pcm_count) ? (m_pcm_count - m_pcm_pos) : 0;
	const attotime when = m_stream->end_time() + m_stream->sample_period() * (left + 1);
	const attotime now = machine().time();
	m_frame_timer->adjust((when > now) ? (when - now) : attotime::zero);
}

TIMER_CALLBACK_MEMBER(l64111_device::flags_tick)
{
	sync_fifo();
}

TIMER_CALLBACK_MEMBER(l64111_device::frame_tick)
{
	sync();
}

// fs = SYSCLK / (2 * (TCR + TOR / 65536) * SCLKO cycles per sample), manual 6.3
void l64111_device::update_rate()
{
	if (!clock() || m_tcr < TCR_MIN)
		return;
	// PCM MODE 00 = 32, 01 = 64, 10 = 48 SCLKO per sample (register description 4.3;
	// figure 6.3 lists 01 and 10 the other way round)
	static constexpr u32 SCLK_RATIO[4] = { 32, 64, 48, 32 };
	const double divisor = 2.0 * (m_tcr + double((m_torh << 8) | m_torl) / 65536.0) * SCLK_RATIO[m_cr2 & CR2_PCM_MODE];
	const u32 rate = u32(double(clock()) / divisor + 0.5);
	if (rate != m_stream->sample_rate())
		m_stream->set_sample_rate(rate);
}


//**************************************************************************
//  Pre-parser
//**************************************************************************

// bring the device up to now: the pre-parser first, so the decoder sees every frame stored by now
void l64111_device::sync()
{
	sync_fifo();
	m_stream->update();
	deliver_events();
	update_frame_timer();
}

// move the bytes the pre-parser has taken by now out of the input FIFO
void l64111_device::sync_fifo()
{
	const attotime now = machine().time();
	preparse(now);
	if (m_fifo.empty())
	{
		m_fifo_time = now;
		m_overflow_logged = false;
	}
	deliver_events();
	schedule_flags();
	update_frame_timer(); // a newly stored frame can start the decoder
}

// parse the bytes the pre-parser has taken from the input FIFO by the given time
void l64111_device::preparse(const attotime &until)
{
	const attotime per_byte = clocks_to_attotime(PREPARSE_CLOCKS);
	while (!m_fifo.empty() && ((m_fifo_time + per_byte) <= until))
	{
		m_fifo_time += per_byte;
		const u8 b = m_fifo.dequeue();
		if (m_cr2 & CR2_AUDIO_ONLY)
			audio_parse(b);
		else
			system_parse(b);
	}
}

// interrupts for events raised by the pre-parser or the decoder
void l64111_device::deliver_events()
{
	if (m_ev1 || m_ev2)
	{
		const u8 e1 = m_ev1, e2 = m_ev2;
		m_ev1 = m_ev2 = 0;
		update_irq_state(e1, e2);
	}
	update_outputs();
}

void l64111_device::syntax_error()
{
	m_ev2 |= M2_SYNTAX_ERR;
}

// ISO 11172-1 system layer: packs, system headers and packets; the payload of the selected
// audio stream goes on to the audio frame parser
void l64111_device::system_parse(u8 data)
{
	switch (m_sys_state)
	{
	case SYS_SCAN:
		m_sys_shift = (m_sys_shift << 8) | data;
		if (m_sys_shift_count < 4)
			m_sys_shift_count++;
		if ((m_sys_shift_count == 4) && ((m_sys_shift & 0xffff'ff00) == 0x0000'0100))
		{
			m_sys_code = data;
			m_sys_shift = m_sys_shift_count = 0;
			if (data == PACK_START)
			{
				m_sys_state = SYS_PACK;
				m_sys_left = 8;
			}
			else if (data >= SYSTEM_HEADER) // system header or packet: a 16-bit length follows
			{
				m_sys_state = SYS_LENGTH;
				m_sys_left = m_sys_length_bytes = 0;
			}
			else if (data != END_CODE) // anything else is out of place here
			{
				m_pack_count = 0;
				syntax_error();
			}
		}
		break;

	case SYS_PACK:
		if ((m_sys_left == 8) && ((data & 0xf0) != 0x20)) // MPEG-1 pack header: '0010', SCR
		{
			m_pack_count = 0;
			syntax_error();
			m_sys_state = SYS_SCAN;
			break;
		}
		if (!--m_sys_left)
		{
			m_sys_state = SYS_SCAN;
			if (++m_pack_count == SYNC_FRAMES)
				m_ev2 |= M2_SYNC_SYS; // SYNC SYS goes high (a level, read from m_pack_count)
		}
		break;

	case SYS_LENGTH:
		m_sys_left = (m_sys_left << 8) | data;
		if (++m_sys_length_bytes == 2)
		{
			const bool audio = (m_sys_code >= AUDIO_STREAM_FIRST) && (m_sys_code <= AUDIO_STREAM_LAST);
			const bool selected = audio && ((m_cr3 & CR3_ID_IGNORE) || ((m_sys_code & CR3_STREAM_ID) == (m_cr3 & CR3_STREAM_ID)));
			if (!m_sys_left)
			{
				m_sys_state = SYS_SCAN;
			}
			else if (selected)
			{
				m_sys_state = SYS_PACKET_HEADER;
				m_sys_hdr_len = m_sys_hdr_need = 0;
			}
			else
			{
				m_sys_state = SYS_SKIP; // system header, padding, private, video and other audio streams
			}
		}
		break;

	case SYS_SKIP:
		if (!--m_sys_left)
			m_sys_state = SYS_SCAN;
		break;

	case SYS_PACKET_HEADER:
		packet_header(data);
		break;

	case SYS_PACKET_DATA:
		audio_parse(data);
		if (!--m_sys_left)
			m_sys_state = SYS_SCAN;
		break;
	}
}

// audio packet header after the length: stuffing, STD buffer size, then PTS, PTS+DTS or 0x0f
void l64111_device::packet_header(u8 data)
{
	m_sys_left--;
	if (!m_sys_hdr_need)
	{
		if ((data & 0xc0) == 0x40)
		{
			m_sys_hdr_need = 2; // STD_buffer_scale/size
		}
		else if ((data & 0xf0) == 0x20)
		{
			m_sys_hdr_need = 5; // PTS
		}
		else if ((data & 0xf0) == 0x30)
		{
			m_sys_hdr_need = 10; // PTS and DTS
		}
		else if (data == 0x0f)
		{
			m_sys_state = SYS_PACKET_DATA;
		}
		else if (data != 0xff) // 0xff is stuffing
		{
			syntax_error();
			m_sys_state = SYS_SKIP;
		}
		m_sys_hdr_len = 0;
	}
	if (m_sys_hdr_need)
	{
		m_sys_hdr[m_sys_hdr_len++] = data;
		if (m_sys_hdr_len == m_sys_hdr_need)
		{
			if (m_sys_hdr_need != 2)
			{
				const u8 *h = m_sys_hdr;
				const u64 pts = (u64(BIT(h[0], 1, 3)) << 30) | (u64(h[1]) << 22) | (u64(BIT(h[2], 1, 7)) << 15) | (u64(h[3]) << 7) | BIT(h[4], 1, 7);
				for (int i = 0; i < 4; i++)
					m_pts[i] = u8(pts >> (8 * i));
				m_pts[4] = BIT(pts, 32);
				if (!(m_status2 & S2_PTS)) // interrupts only when PTS AVAILABLE goes high
					m_ev2 |= M2_PTS;
				m_sys_state = SYS_PACKET_DATA;
			}
			m_sys_hdr_need = m_sys_hdr_len = 0;
		}
	}
	if (!m_sys_left)
		m_sys_state = SYS_SCAN;
}

void l64111_device::audio_parse(u8 data)
{
	m_frame[m_frame_len++] = data;
	if (m_frame_need)
	{
		if (m_frame_len == m_frame_need)
		{
			store_frame();
			m_frame_len = m_frame_need = 0;
		}
		return;
	}

	// hunting for a byte-aligned sync word, then reading the rest of the header; a frame that
	// doesn't start where the last one ended loses synchronization
	auto drop = [this]
	{
		if (m_sync_count)
		{
			m_sync_count = 0;
			syntax_error();
		}
		std::move(m_frame + 1, m_frame + m_frame_len, m_frame);
		m_frame_len--;
	};
	while (m_frame_len && ((m_frame[0] != 0xff) || ((m_frame_len >= 2) && ((m_frame[1] & 0xf0) != 0xf0))))
		drop();
	if (m_frame_len < 4)
		return;

	const u8 id = BIT(m_frame[1], 3);
	const u8 layer_code = BIT(m_frame[1], 1, 2); // 11 Layer I, 10 Layer II
	const u8 rate_index = BIT(m_frame[2], 4, 4);
	const u8 fs_index = BIT(m_frame[2], 2, 2);
	const u8 padding = BIT(m_frame[2], 1);
	const bool mono = BIT(m_frame[3], 6, 2) == 3;
	const u32 kbps = (layer_code >= 2) ? BITRATE[(layer_code == 3) ? 0 : 1][rate_index] : 0;
	const u32 fs = SAMPLE_RATE[fs_index];
	// Layer II: 32, 48, 56 and 80 kbit/s are single channel only, 224 kbit/s and up two channels only
	const bool bad_mode = (layer_code == 2) && (mono ? (kbps >= 224) : ((kbps < 64) || (kbps == 80)));
	if (!id || !kbps || !fs || bad_mode) // MPEG-2, Layer III, free format, reserved values
	{
		drop();
		return;
	}
	m_frame_need = (layer_code == 3) ? ((12'000 * kbps / fs + padding) * 4) : (144'000 * kbps / fs + padding);
	if (m_frame_need > FRAME_BYTES_MAX)
		m_frame_need = FRAME_BYTES_MAX;
}

void l64111_device::store_frame()
{
	const u8 *h = m_frame;
	m_pd[0] = (BIT(h[1], 3) << 7) | (BIT(h[1], 1, 2) << 5) | (BIT(h[1], 0) << 4) | BIT(h[2], 4, 4);
	m_pd[1] = (BIT(h[2], 2, 2) << 6) | (BIT(h[2], 1) << 5) | (BIT(h[2], 0) << 4) | (BIT(h[3], 6, 2) << 2) | BIT(h[3], 4, 2);
	m_pd[2] = (BIT(h[3], 3) << 7) | (BIT(h[3], 2) << 6) | (BIT(h[3], 0, 2) << 4);
	LOGMASKED(LOG_FRAME, "frame %u bytes, channel buffer %u\n", m_frame_len, m_cb_count);
	// SYNC AUD goes high on the third frame in sequence (a level, read from m_sync_count)
	m_ev2 |= M2_FRAME_DETECT | ((++m_sync_count == SYNC_FRAMES) ? M2_SYNC_AUD : 0x00);

	// the manual doesn't say whether BUF_RST holds the channel buffer empty; ANC DATA FIFO RST
	// in the same register holds its FIFO [INFERRED]
	if (m_cr1 & CR1_BUF_RST)
		return;
	m_cb_layer = (BIT(h[1], 1, 2) == 3) ? 1 : 2;
	if (m_cb_count >= cb_frames())
	{
		m_ev1 |= S1_ERR_BUF_OVFL; // the frame is lost
		return;
	}
	std::copy_n(m_frame, m_frame_len, m_cb[m_cb_write]);
	m_cb_len[m_cb_write] = m_frame_len;
	m_cb_write = (m_cb_write + 1) % cb_frames();
	if (++m_cb_count == 1)
		m_ev2 |= M2_LAST_FRAME; // LAST FRAME: one frame in the channel buffer
}


//**************************************************************************
//  Decoder and PCM output
//**************************************************************************

void l64111_device::silent_frame()
{
	std::fill(std::begin(m_pcm), std::end(m_pcm), 0);
	m_pcm_count = (m_cb_layer == 1) ? LAYER1_SAMPLES : SAMPLES_MAX;
	m_pcm_channels = 1;
}

bool l64111_device::next_frame()
{
	if (m_underrun || !m_cb_count)
		return false;

	// a slot reached through the cue/review counters may never have been written
	const u32 len = std::min<u32>(m_cb_len[m_cb_read], FRAME_BYTES_MAX);
	std::copy_n(m_cb[m_cb_read], len, m_decode);
	m_cb_read = (m_cb_read + 1) % cb_frames();
	if (--m_cb_count == 1)
		m_ev2 |= M2_LAST_FRAME;

	int pos = 0, samples = 0, rate = 0, channels = 0;
	if ((len >= 4) && m_decoder->decode_buffer(pos, len * 8, m_pcm, samples, rate, channels) && (samples > 0))
	{
		m_pcm_count = samples;
		m_pcm_channels = (channels >= 2) ? 2 : 1;
		ancillary(pos, len * 8);
	}
	else if (((m_cr2 & CR2_MUTE) != CR2_MUTE_REPLAY) || !m_pcm_count)
	{
		silent_frame(); // MUTE 10 replays the last good frame instead
	}
	m_pcm_pos = 0;
	m_ev2 |= M2_NEW_FRAME; // NEW FRAME OUT
	m_started = true;
	return true;
}

// the bits after the audio data to the end of the frame, packed MSB first across frames
void l64111_device::ancillary(u32 bit_pos, u32 bit_end)
{
	if (m_cr1 & CR1_ANC_RST) // held empty
		return;
	for (u32 b = bit_pos; b < bit_end; b++)
	{
		m_anc_acc = (m_anc_acc << 1) | BIT(m_decode[b >> 3], ~b & 7);
		if (++m_anc_bits < 8)
			continue;
		if (m_anc.full())
		{
			if (!m_anc_overflow)
			{
				m_anc_overflow = true;
				m_ev1 |= S1_ANC_OVRFL;
			}
		}
		else
		{
			m_anc.enqueue(u8(m_anc_acc));
			if (m_anc.queue_length() == 1)
				m_ev1 |= S1_ANC_VALID;
			else if (m_anc.queue_length() == ANC_BYTES / 2)
				m_ev1 |= S1_ANC_HFF;
		}
		m_anc_acc = m_anc_bits = 0;
	}
}

void l64111_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, 0);
	stream.fill(1, 0);
	if (!decoder_enabled())
		return;

	const bool muted = (m_cr2 & CR2_SOFT_MUTE) || m_hdmute_pin;
	for (int sample = 0; sample < stream.samples(); sample++)
	{
		if (m_pcm_pos >= m_pcm_count)
		{
			// the sound manager also updates the stream, without a sync(): store the frames the
			// pre-parser would have by this sample before taking one
			preparse(stream.start_time() + stream.sample_period() * sample);
			if (!next_frame())
			{
				// an empty channel buffer after playback started is an underrun: the decoder stays
				// silent until it is reset (manual 6.4, input buffering considerations)
				if (m_started)
					m_underrun = true;
				break;
			}
		}
		if (!muted)
		{
			const u32 at = m_pcm_pos * m_pcm_channels;
			const s16 left = m_pcm[at];
			const s16 right = (m_pcm_channels == 2) ? m_pcm[at + 1] : left;
			stream.put_int(0, sample, left, 32768);
			stream.put_int(1, sample, right, 32768);
		}
		m_pcm_pos++;
	}
}
