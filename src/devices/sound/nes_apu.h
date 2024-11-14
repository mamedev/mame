// license:GPL-2.0+
// copyright-holders:Matthew Conte
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES RP2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.

 *****************************************************************************

   NES_APU.H

   NES APU external interface.

 *****************************************************************************/

#ifndef MAME_SOUND_NES_APU_H
#define MAME_SOUND_NES_APU_H

#pragma once


#include "nes_defs.h"

/* AN EXPLANATION
 *
 * The NES APU is actually integrated into the Nintendo processor.
 * You must supply the same number of APUs as you do processors.
 * Also make sure to correspond the memory regions to those used in the
 * processor, as each is shared.
 */

class nesapu_device : public device_t,
						public device_sound_interface
{
public:
	nesapu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }
	auto mem_read() { return m_mem_read_cb.bind(); }

	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	u8 status_r();
	void write(offs_t offset, u8 data);

protected:
	nesapu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual void update_lfsr(apu_t::noise_t &chan);

private:
	/* GLOBAL CONSTANTS */
	static constexpr unsigned  SYNCS_MAX1     = 0x20;
	static constexpr unsigned  SYNCS_MAX2     = 0x80;
	static constexpr u32       NTSC_APU_CLOCK = 21477272 / 12;
	static constexpr u32       PAL_APU_CLOCK  = 26601712 / 16;

	// internal state
	apu_t   m_APU;                   /* Actual APUs */
	u8      m_is_pal;
	u32     m_samps_per_sync;        /* Number of samples per vsync */
	u32     m_vbl_times[SYNCS_MAX1];   /* VBL durations in samples */
	u32     m_sync_times1[SYNCS_MAX1]; /* Samples per sync table */
	u32     m_sync_times2[SYNCS_MAX2]; /* Samples per sync table */
	stream_buffer::sample_t m_square_lut[31];       // Non-linear Square wave output LUT
	stream_buffer::sample_t m_tnd_lut[16][16][128]; // Non-linear Triangle, Noise, DMC output LUT

	sound_stream *m_stream;
	devcb_write_line m_irq_handler;
	devcb_read8 m_mem_read_cb;

	emu_timer *m_frame_timer;
	attotime m_frame_period;
	u16 m_frame_clocks;

	TIMER_CALLBACK_MEMBER(frame_timer_cb);
	void calculate_rates();
	void apu_square(apu_t::square_t *chan);
	void apu_triangle(apu_t::triangle_t *chan);
	void apu_noise(apu_t::noise_t *chan);
	void apu_dpcm(apu_t::dpcm_t *chan);
};

class apu2a03_device : public nesapu_device
{
public:
	apu2a03_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void update_lfsr(apu_t::noise_t &chan) override;
};


DECLARE_DEVICE_TYPE(NES_APU,  nesapu_device)
DECLARE_DEVICE_TYPE(APU_2A03, apu2a03_device)

#endif // MAME_SOUND_NES_APU_H
