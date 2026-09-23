// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Motorola GSC38GG307 full motion audio decoder.

    The MPEG-1 audio decoder of the CD-i Digital Video Cartridge, marked
    GSC38GG307CF50 on the GMPEG and VMPEG boards, with a DSP56001 beside it.
    On VMPEG it sits next to an MCD251 video decoder and both are fed the
    same MPEG-1 system stream, each selecting its own elementary stream by
    number.

    No datasheet is available, so the register behaviour was worked out from
    the CDi_MiSTer FPGA core by Andre Zeps: the decoding itself is done with
    mpeg_audio, and the DSP56001, which the driver uses for the attenuation
    ramp rather than for decoding, is not emulated.

*******************************************************************************/

#ifndef MAME_SOUND_GSC38GG307_H
#define MAME_SOUND_GSC38GG307_H

#pragma once

#include "machine/mpeg_demux.h"
#include "mpeg_audio.h"

#include <memory>
#include <vector>


// ======================> gsc38gg307_device

class gsc38gg307_device : public device_t, public device_sound_interface
{
public:
	gsc38gg307_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~gsc38gg307_device();

	// interrupt request, and the transfer request that asks the host to move
	// the next part of the system stream into the chip
	auto irq_callback() { return m_irq_cb.bind(); }
	auto drq_callback() { return m_drq_cb.bind(); }

	// host interface, word registers
	uint16_t regs_r(offs_t offset, uint16_t mem_mask = ~0);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// MPEG-1 system stream in
	void write_data(uint8_t data);

	// the transfer that fed those bytes has finished, dma false when the host
	// wrote them one word at a time
	void end_of_transfer(bool dma);

	// is this chip asking for the interrupt, and with which vector
	bool irq_pending() const { return (m_fma_isr & m_fma_ier) != 0; }
	uint8_t vector() const { return uint8_t(m_fma_ivec); }

	// the decoder clock tick the host is polled at
	void poll_w(int state);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// status/interrupt register bits
	enum : uint16_t
	{
		FMA_EOI  = 1 << 0,  // ISO end detected
		FMA_CSU  = 1 << 1,  // stream changed
		FMA_UPD  = 1 << 2,  // frame header updated
		FMA_UNF  = 1 << 3,  // underflow
		FMA_DEC  = 1 << 4,  // decoding started
		FMA_ERR  = 1 << 5,
		FMA_POLL = 1 << 8
	};

	TIMER_CALLBACK_MEMBER(audio_tick);

	void audio_decode_pending();
	void audio_es_write(const uint8_t *data, size_t length);
	void reset_audio_decoder();
	void update_intreq();

	uint32_t fma_dclk() const;

	devcb_write_line m_irq_cb;
	devcb_write_line m_drq_cb;

	sound_stream *m_stream = nullptr;
	emu_timer *m_audio_timer = nullptr;

	// ---- register file ----
	uint16_t m_fma_cmd = 0;
	uint8_t  m_fma_status = 0;
	uint16_t m_fma_isr = 0;
	uint16_t m_fma_ier = 0;
	uint16_t m_fma_ivec = 0;
	uint8_t  m_fma_stream = 0;
	uint8_t  m_fma_dspa = 0;
	bool     m_fma_dsp_enable = false;
	uint16_t m_fma_dclkl_latch = 0;
	uint32_t m_fma_audio_header = 0;
	bool     m_pending_fma_stream_change = false;

	// ---- timing ----
	attotime m_dclk_origin;

	// playback start time, from the first PTS
	bool m_fma_scr_start_valid = false;
	int64_t m_fma_scr_start_time = 0;

	// ---- stream plumbing ----
	mpeg_demux m_demux;

	// the raw MPEG audio frame header the registers report
	uint32_t m_audio_header_shift = 0;

	// decoded samples.  A read head rather than erase(begin()), which would
	// make the stream callback quadratic in the queue depth.
	std::vector<int16_t> m_audio_samples[2];
	size_t m_audio_head = 0;
	uint32_t m_audio_sample_rate = 44100;
	size_t audio_available() const { return m_audio_samples[0].size() - m_audio_head; }
	void audio_clear() { m_audio_samples[0].clear(); m_audio_samples[1].clear(); m_audio_head = 0; }

	// MPEG audio decoding.  The layer II frames are found here, since the
	// frame length covers padding and ancillary data the decoder does not
	// read.
	std::unique_ptr<uint8_t []> m_audio_es;  // elementary stream not decoded yet
	size_t m_audio_es_head = 0, m_audio_es_tail = 0;
	std::unique_ptr<mpeg_audio> m_audio;
	size_t audio_es_available() const { return m_audio_es_tail - m_audio_es_head; }

	bool m_irq_state = false;
};

DECLARE_DEVICE_TYPE(GSC38GG307, gsc38gg307_device)

#endif // MAME_SOUND_GSC38GG307_H
