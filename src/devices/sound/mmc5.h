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

   mmc5.h

   MMC5 sound emulation core.

 *****************************************************************************/

#ifndef MAME_SOUND_MMC5_H
#define MAME_SOUND_MMC5_H

#pragma once


#include "nes_defs.h"

class mmc5_sound_device : public device_t,
						public device_sound_interface
{
public:
	mmc5_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	void pcm_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	/* GLOBAL CONSTANTS */
	static constexpr unsigned  SYNCS_MAX1     = 0x20;

	// internal state
	mmc5_sound_t m_core;                   /* Sound core */
	u32          m_samps_per_sync;        /* Number of samples per vsync */
	u32          m_vbl_times[SYNCS_MAX1];   /* VBL durations in samples */
	u32          m_sync_times1[SYNCS_MAX1]; /* Samples per sync table */
	sound_stream::sample_t m_square_lut[31];       // Non-linear Square wave output LUT

	sound_stream *m_stream;
	devcb_write_line m_irq_handler;

	u16 m_frame_clocks;

	void calculate_rates();
	void tick_square(mmc5_sound_t::square_t &chan);
};


DECLARE_DEVICE_TYPE(MMC5_SOUND,  mmc5_sound_device)

#endif // MAME_SOUND_MMC5_H
