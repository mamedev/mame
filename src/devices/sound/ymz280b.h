// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   Yamaha YMZ280B driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __YMZ280B_H__
#define __YMZ280B_H__

#include "emu.h"

#define MAKE_WAVS           0

#define MCFG_YMZ280B_IRQ_HANDLER(_devcb) \
	devcb = &ymz280b_device::set_irq_handler(*device, DEVCB_##_devcb);

#define MCFG_YMZ280B_EXT_READ_HANDLER(_devcb) \
	devcb = &ymz280b_device::set_ext_read_handler(*device, DEVCB_##_devcb);

#define MCFG_YMZ280B_EXT_WRITE_HANDLER(_devcb) \
	devcb = &ymz280b_device::set_ext_write_handler(*device, DEVCB_##_devcb);

class ymz280b_device : public device_t,
									public device_sound_interface
{
public:
	ymz280b_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ext_read_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_ext_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ext_write_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_ext_write_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:

	/* struct describing a single playing ADPCM voice */
	struct YMZ280BVoice
	{
		UINT8 playing;          /* 1 if we are actively playing */
		bool ended;             /* indicate voice has ended in case samples_left is 0 */

		UINT8 keyon;            /* 1 if the key is on */
		UINT8 looping;          /* 1 if looping is enabled */
		UINT8 mode;             /* current playback mode */
		UINT16 fnum;            /* frequency */
		UINT8 level;            /* output level */
		UINT8 pan;              /* panning */

		UINT32 start;           /* start address, in nibbles */
		UINT32 stop;            /* stop address, in nibbles */
		UINT32 loop_start;      /* loop start address, in nibbles */
		UINT32 loop_end;        /* loop end address, in nibbles */
		UINT32 position;        /* current position, in nibbles */

		INT32 signal;           /* current ADPCM signal */
		INT32 step;             /* current ADPCM step */

		INT32 loop_signal;      /* signal at loop start */
		INT32 loop_step;        /* step at loop start */
		UINT32 loop_count;      /* number of loops so far */

		INT32 output_left;      /* output volume (left) */
		INT32 output_right;     /* output volume (right) */
		INT32 output_step;      /* step value for frequency conversion */
		INT32 output_pos;       /* current fractional position */
		INT16 last_sample;      /* last sample output */
		INT16 curr_sample;      /* current sample target */
		UINT8 irq_schedule;     /* 1 if the IRQ state is updated by timer */

		emu_timer *timer;
	};

	UINT8 ymz280b_read_memory(UINT32 offset);
	void update_irq_state();
	void update_step(struct YMZ280BVoice *voice);
	void update_volumes(struct YMZ280BVoice *voice);
	void update_irq_state_timer_common(int voicenum);
	int generate_adpcm(struct YMZ280BVoice *voice, INT16 *buffer, int samples);
	int generate_pcm8(struct YMZ280BVoice *voice, INT16 *buffer, int samples);
	int generate_pcm16(struct YMZ280BVoice *voice, INT16 *buffer, int samples);
	void write_to_register(int data);
	int compute_status();

	// internal state
	struct YMZ280BVoice m_voice[8];   /* the 8 voices */
	UINT8 m_current_register;         /* currently accessible register */
	UINT8 m_status_register;          /* current status register */
	UINT8 m_irq_state;                /* current IRQ state */
	UINT8 m_irq_mask;                 /* current IRQ mask */
	UINT8 m_irq_enable;               /* current IRQ enable */
	UINT8 m_keyon_enable;             /* key on enable */
	UINT8 m_ext_mem_enable;           /* external memory enable */
	UINT8 m_ext_readlatch;            /* external memory prefetched data */
	UINT32 m_ext_mem_address_hi;
	UINT32 m_ext_mem_address_mid;
	UINT32 m_ext_mem_address;         /* where the CPU can read the ROM */

	devcb_write_line m_irq_handler;  /* IRQ callback */
	devcb_read8 m_ext_read_handler;  /* external RAM read handler */
	devcb_write8 m_ext_write_handler;/* external RAM write handler */

	double m_master_clock;            /* master clock frequency */
	UINT8 *m_mem_base;                /* pointer to the base of external memory */
	UINT32 m_mem_size;
	sound_stream *m_stream;           /* which stream are we using */
	std::unique_ptr<INT16[]> m_scratch;
#if MAKE_WAVS
	void *m_wavresample;              /* resampled waveform */
#endif
};

extern const device_type YMZ280B;


#endif /* __YMZ280B_H__ */
