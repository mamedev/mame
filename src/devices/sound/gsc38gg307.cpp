// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Motorola GSC38GG307 full motion audio decoder.  See gsc38gg307.h.

    Emulation notes:

    - The chip's MPEG decoding is undocumented, so the elementary stream is
      decoded with mpeg_audio.  The layer II frames are found here: the frame
      length covers the padding byte and the ancillary data the decoder does
      not read, and each frame stands alone, so a stereo mode that changes
      from frame to frame needs no handling.
    - The 45 kHz decoder clock is derived from machine time rather than
      counted tick by tick.
    - Playback starts when the decoder clock reaches the point the first PTS
      asks for, relative to the pack header's SCR.

*******************************************************************************/

#include "emu.h"
#include "gsc38gg307.h"

#include <algorithm>

#define LOG_REGS_R    (1U << 1)
#define LOG_REGS_W    (1U << 2)
#define LOG_FMA       (1U << 3)
#define LOG_IRQ       (1U << 4)
#define LOG_AUDIO     (1U << 5)

#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(GSC38GG307, gsc38gg307_device, "gsc38gg307", "Motorola GSC38GG307 full motion audio decoder")

namespace {

// The decoder clock ticks at 45 kHz.
constexpr int DCLK_HZ = 45000;

// The input FIFO.  The driver keeps it far below this.
constexpr size_t AUDIO_BUFFER_SIZE = 64 * 1024;

// A CD-i digital video stream carries MPEG-1 layer II at 224 kbit/s and
// 44.1 kHz, so one 1152 sample frame is about 731 bytes.  Round up: below
// this much undecoded elementary stream the decoder genuinely cannot produce
// another frame.
constexpr size_t MP2_FRAME_BYTES = 1024;

} // anonymous namespace

//**************************************************************************
//  CONSTRUCTION
//**************************************************************************

gsc38gg307_device::gsc38gg307_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GSC38GG307, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_drq_cb(*this)
{
}

gsc38gg307_device::~gsc38gg307_device()
{
}

void gsc38gg307_device::device_start()
{
	m_audio_es = std::make_unique<uint8_t []>(AUDIO_BUFFER_SIZE);
	m_audio = std::make_unique<mpeg_audio>(m_audio_es.get(), mpeg_audio::L2, false, 0);

	m_stream = stream_alloc(0, 2, 44100);
	m_audio_timer = timer_alloc(FUNC(gsc38gg307_device::audio_tick), this);

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
	save_item(NAME(m_dclk_origin));
	m_demux.register_save_state(*this);
	m_audio->register_save_state(*this);
}

void gsc38gg307_device::device_reset()
{
	// power-on state of the chip
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

	m_dclk_origin = machine().time();

	m_demux.reset();
	m_audio_header_shift = 0;
	m_fma_scr_start_valid = false;
	m_fma_scr_start_time = 0;

	reset_audio_decoder();
	audio_clear();

	m_irq_state = false;
	m_irq_cb(0);

	m_audio_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));
}

//**************************************************************************
//  CLOCK
//**************************************************************************

uint32_t gsc38gg307_device::fma_dclk() const
{
	return uint32_t((machine().time() - m_dclk_origin).as_ticks(DCLK_HZ));
}

//**************************************************************************
//  STREAM INPUT
//**************************************************************************

// A byte of the system stream, from a transfer or from a host write.
void gsc38gg307_device::write_data(uint8_t data)
{
	const bool was_body = m_demux.packet_body;
	m_demux.byte(data, m_fma_stream);

	if (was_body)
	{
		audio_es_write(&data, 1);

		// FMA_MPEG_AUDIO_HEADER @ 0xe03014: keep the most recent frame
		// header the driver can read back
		m_audio_header_shift = (m_audio_header_shift << 8) | data;
		if ((m_audio_header_shift & 0xfff00000) == 0xfff00000)
			m_fma_audio_header = m_audio_header_shift;
	}

	if (m_demux.pts_updated)
	{
		// the first PTS sets when audio playback starts
		if (!m_fma_scr_start_valid)
		{
			m_fma_scr_start_valid = true;
			m_fma_scr_start_time = int64_t(fma_dclk())
					+ (mpeg_timestamp_diff(m_demux.pts, m_demux.scr) >> 1);
		}
	}

	if (m_demux.program_end)
	{
		// ISO end detected
		m_fma_isr |= FMA_EOI;
		m_fma_status |= FMA_EOI;
		update_intreq();
	}
}

void gsc38gg307_device::end_of_transfer(bool dma)
{
	// the command register's transfer bit clears when the transfer finishes
	if (dma)
		m_fma_cmd &= ~0x8000;

	audio_decode_pending();
}

//**************************************************************************
//  AUDIO DECODE
//**************************************************************************

void gsc38gg307_device::audio_es_write(const uint8_t *data, size_t length)
{
	if (m_audio_es_tail + length > AUDIO_BUFFER_SIZE)
	{
		std::copy(&m_audio_es[m_audio_es_head], &m_audio_es[m_audio_es_tail], &m_audio_es[0]);
		m_audio_es_tail -= m_audio_es_head;
		m_audio_es_head = 0;
	}
	// an overflowing FIFO drops the newest data
	length = std::min(length, AUDIO_BUFFER_SIZE - m_audio_es_tail);
	std::copy_n(data, length, &m_audio_es[m_audio_es_tail]);
	m_audio_es_tail += length;
}

void gsc38gg307_device::audio_decode_pending()
{
	// layer II bit rates in kbit/s and sample rates in Hz
	static constexpr int BITRATES[16] = { 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0 };
	static constexpr int SAMPLE_RATES[4] = { 44100, 48000, 32000, 0 };

	// keep roughly a quarter second of samples queued
	while (audio_available() < 44100 / 4 && audio_es_available() >= 4)
	{
		// Find an MPEG-1 layer II frame header on a byte boundary.  The frame
		// length is 144 * bitrate / sample rate plus the padding byte, and
		// the next frame starts there whatever the decoder read of this one.
		const uint8_t *const header = &m_audio_es[m_audio_es_head];
		const int bitrate = BITRATES[header[2] >> 4];
		const int sample_rate = SAMPLE_RATES[(header[2] >> 2) & 3];
		const bool mono = (header[3] >> 6) == 3;
		// the decoder has no parameters for these combinations
		const bool allowed = mono ? (bitrate <= 192) : (bitrate != 32 && bitrate != 48 && bitrate != 56 && bitrate != 80);
		if (header[0] != 0xff || (header[1] & 0xfe) != 0xfc || !bitrate || !sample_rate || !allowed)
		{
			m_audio_es_head++;
			continue;
		}

		const size_t frame_bytes = size_t(144000) * bitrate / sample_rate + BIT(header[2], 1);
		if (audio_es_available() < frame_bytes)
			break;

		int16_t samples[1152 * 2];
		int pos = int(m_audio_es_head * 8);
		int count = 0, rate = 0, channels = 0;
		const bool decoded = m_audio->decode_buffer(pos, int((m_audio_es_head + frame_bytes) * 8), samples, count, rate, channels);
		m_audio_es_head += frame_bytes;
		if (!decoded)
		{
			LOGMASKED(LOG_AUDIO, "FMA undecodable frame skipped\n");
			continue;
		}

		m_audio_sample_rate = uint32_t(rate);
		for (int i = 0; i < count; i++)
		{
			m_audio_samples[0].push_back(samples[i * channels]);
			m_audio_samples[1].push_back(samples[i * channels + channels - 1]);
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

void gsc38gg307_device::reset_audio_decoder()
{
	m_audio_es_head = m_audio_es_tail = 0;
	m_audio->clear();
}

TIMER_CALLBACK_MEMBER(gsc38gg307_device::audio_tick)
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

	// Underflow means the decoder wants data and the input FIFO has none.
	// The sample queue alone is not enough: sectors arrive in bursts, so it
	// empties between them in normal play.
	if (audio_available() == 0
			&& audio_es_available() < MP2_FRAME_BYTES)
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

void gsc38gg307_device::sound_stream_update(sound_stream &stream)
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
//  INTERRUPTS
//**************************************************************************

void gsc38gg307_device::update_intreq()
{
	const bool req = irq_pending();
	if (req != m_irq_state)
	{
		m_irq_state = req;
		LOGMASKED(LOG_IRQ, "FMA irq %d (isr %04x ier %04x)\n", req, m_fma_isr, m_fma_ier);
		m_irq_cb(req ? 1 : 0);
	}
}

// the video decoder's programmable timer polls the host through this chip too
void gsc38gg307_device::poll_w(int state)
{
	if (!state)
		return;

	m_fma_isr |= FMA_POLL;
	update_intreq();
}

//**************************************************************************
//  REGISTERS
//**************************************************************************

uint16_t gsc38gg307_device::regs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;

	switch (offset)
	{
	case 0x00: data = m_fma_cmd; break;
	case 0x01: data = 0x0200 | m_fma_status; break;
	case 0x02: data = 0x0007; break;
	case 0x03: data = 0x0900; break;
	case 0x04: data = m_fma_stream; break;
	case 0x05: data = m_fma_stream; break;
	case 0x06: data = m_fma_ivec; break;
	case 0x07: data = 0x0042; break;

	case 0x08:
		data = uint16_t(fma_dclk() >> 16);
		if (!machine().side_effects_disabled())
			m_fma_dclkl_latch = uint16_t(fma_dclk());
		break;

	case 0x09: data = m_fma_dclkl_latch; break;
	case 0x0a: data = uint16_t(m_fma_audio_header >> 16); break;
	case 0x0b: data = uint16_t(m_fma_audio_header); break;
	case 0x0c: data = m_fma_dsp_enable ? 1 : 0; break;

	case 0x0d:
		data = m_fma_isr;
		if (!machine().side_effects_disabled())
		{
			// reading the interrupt status register clears it
			m_fma_isr = 0;
			update_intreq();
		}
		break;

	case 0x0e: data = m_fma_ier; break;
	case 0x12: data = 0x0004; break; // HF2 flag of the DSP56001
	default:
		break;
	}

	LOGMASKED(LOG_REGS_R, "%s: fma_r: %02x = %04x & %04x\n", machine().describe_context(),
			offset, data, mem_mask);

	return data;
}

void gsc38gg307_device::regs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_REGS_W, "%s: fma_w: %02x = %04x & %04x\n", machine().describe_context(),
			offset, data, mem_mask);

	switch (offset)
	{
	case 0x00: // CMD
		LOGMASKED(LOG_FMA, "FMA CMD %04x\n", data);
		m_fma_cmd = data;
		if (BIT(data, 15))
		{
			// green book 8.2.4.3.3: a transfer clears the underflow flag
			m_fma_status &= ~FMA_UNF;
			m_drq_cb(1);
		}
		if (BIT(data, 0))
		{
			// stop
			m_fma_dsp_enable = false;
			m_fma_status = 0;
			m_demux.reset();
			m_fma_scr_start_valid = false;
			reset_audio_decoder();
			audio_clear();
		}
		break;

	case 0x02:
		// immediate 7 written by the VMPEG ROM right after the stream number
		break;

	case 0x04: // stream number, 0-15 per mv_selstrm()
		LOGMASKED(LOG_FMA, "FMA stream %04x\n", data);
		m_fma_stream = data & 0x0f;
		m_pending_fma_stream_change = true;
		break;

	case 0x06:
		LOGMASKED(LOG_FMA, "FMA IVEC %04x\n", data);
		m_fma_ivec = data;
		break;

	case 0x0c:
		LOGMASKED(LOG_FMA, "FMA RUN %04x\n", data);
		break;

	case 0x0e:
		LOGMASKED(LOG_FMA, "FMA IER %04x\n", data);
		m_fma_ier = data;
		update_intreq();
		break;

	case 0x11:
		m_fma_dspa = uint8_t(data);
		break;

	case 0x12:
		// DSP56001 data port, used for the attenuation ramp
		break;
	default:
		break;
	}
}
