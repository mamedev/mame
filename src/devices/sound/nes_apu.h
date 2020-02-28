// license:GPL-2.0+
// copyright-holders:Matthew Conte
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES N2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.  This core is freely available for
  use in any freeware project, subject to the following terms:

  Any modifications to this code must be duly noted in the source and
  approved by Matthew Conte and myself prior to public submission.

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

	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	/* GLOBAL CONSTANTS */
	static constexpr unsigned  SYNCS_MAX1     = 0x20;
	static constexpr unsigned  SYNCS_MAX2     = 0x80;

	// internal state
	apu_t   m_APU;                   /* Actual APUs */
	u32     m_samps_per_sync;        /* Number of samples per vsync */
	u32     m_buffer_size;           /* Actual buffer size in bytes */
	u8      m_noise_lut[apu_t::NOISE_LONG]; /* Noise sample lookup table */
	u32     m_vbl_times[0x20];       /* VBL durations in samples */
	u32     m_sync_times1[SYNCS_MAX1]; /* Samples per sync table */
	u32     m_sync_times2[SYNCS_MAX2]; /* Samples per sync table */
	sound_stream *m_stream;
	devcb_write_line m_irq_handler;
	devcb_read8 m_mem_read_cb;

	void calculate_rates();
	void create_syncs(unsigned long sps);
	s8 apu_square(apu_t::square_t *chan);
	s8 apu_triangle(apu_t::triangle_t *chan);
	s8 apu_noise(apu_t::noise_t *chan);
	s8 apu_dpcm(apu_t::dpcm_t *chan);
	inline void apu_regwrite(int address, u8 value);
};

DECLARE_DEVICE_TYPE(NES_APU, nesapu_device)

#endif // MAME_SOUND_NES_APU_H
