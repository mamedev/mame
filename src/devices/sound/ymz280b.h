// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef MAME_SOUND_YMZ280B_H
#define MAME_SOUND_YMZ280B_H

#pragma once

#include "dirom.h"

#define YMZ280B_MAKE_WAVS 0

class ymz280b_device : public device_t, public device_sound_interface, public device_rom_interface<24>
{
public:
	ymz280b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
	/* struct describing a single playing ADPCM voice */
	struct YMZ280BVoice
	{
		u8 playing;          /* 1 if we are actively playing */
		bool ended;          /* indicate voice has ended in case samples_left is 0 */

		u8 keyon;            /* 1 if the key is on */
		u8 looping;          /* 1 if looping is enabled */
		u8 mode;             /* current playback mode */
		u16 fnum;            /* frequency */
		u8 level;            /* output level */
		u8 pan;              /* panning */

		u32 start;           /* start address, in nibbles */
		u32 stop;            /* stop address, in nibbles */
		u32 loop_start;      /* loop start address, in nibbles */
		u32 loop_end;        /* loop end address, in nibbles */
		u32 position;        /* current position, in nibbles */

		s32 signal;          /* current ADPCM signal */
		s32 step;            /* current ADPCM step */

		s32 loop_signal;     /* signal at loop start */
		s32 loop_step;       /* step at loop start */
		u32 loop_count;      /* number of loops so far */

		s32 output_left;     /* output volume (left) */
		s32 output_right;    /* output volume (right) */
		s32 output_step;     /* step value for frequency conversion */
		s32 output_pos;      /* current fractional position */
		s16 last_sample;     /* last sample output */
		s16 curr_sample;     /* current sample target */
		u8 irq_schedule;     /* 1 if the IRQ state is updated by timer */

		emu_timer *timer;
	};

	void update_irq_state();
	void update_step(YMZ280BVoice *voice);
	void update_volumes(YMZ280BVoice *voice);
	TIMER_CALLBACK_MEMBER(update_irq_state_timer_common);
	int generate_adpcm(YMZ280BVoice *voice, s16 *buffer, int samples);
	int generate_pcm8(YMZ280BVoice *voice, s16 *buffer, int samples);
	int generate_pcm16(YMZ280BVoice *voice, s16 *buffer, int samples);
	void write_to_register(int data);
	int compute_status();

	// internal state
	YMZ280BVoice m_voice[8];          /* the 8 voices */
	u8 m_current_register;            /* currently accessible register */
	u8 m_status_register;             /* current status register */
	u8 m_irq_state;                   /* current IRQ state */
	u8 m_irq_mask;                    /* current IRQ mask */
	u8 m_irq_enable;                  /* current IRQ enable */
	u8 m_keyon_enable;                /* key on enable */
	u8 m_ext_mem_enable;              /* external memory enable */
	u8 m_ext_readlatch;               /* external memory prefetched data */
	u32 m_ext_mem_address_hi;
	u32 m_ext_mem_address_mid;
	u32 m_ext_mem_address;            /* where the CPU can read the ROM */

	devcb_write_line m_irq_handler;   /* IRQ callback */

	double m_master_clock;            /* master clock frequency */
	sound_stream *m_stream;           /* which stream are we using */
	std::unique_ptr<s16[]> m_scratch;
#if YMZ280B_MAKE_WAVS
	void *m_wavresample;              /* resampled waveform */
#endif
};

DECLARE_DEVICE_TYPE(YMZ280B, ymz280b_device)

#endif // MAME_SOUND_YMZ280B_H
