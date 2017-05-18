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
	ymz280b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// static configuration helpers
	template<class _Object> static devcb_base &set_irq_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_irq_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ext_read_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_ext_read_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_ext_write_handler(device_t &device, _Object object) { return downcast<ymz280b_device &>(device).m_ext_write_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
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
		uint8_t playing;          /* 1 if we are actively playing */
		bool ended;             /* indicate voice has ended in case samples_left is 0 */

		uint8_t keyon;            /* 1 if the key is on */
		uint8_t looping;          /* 1 if looping is enabled */
		uint8_t mode;             /* current playback mode */
		uint16_t fnum;            /* frequency */
		uint8_t level;            /* output level */
		uint8_t pan;              /* panning */

		uint32_t start;           /* start address, in nibbles */
		uint32_t stop;            /* stop address, in nibbles */
		uint32_t loop_start;      /* loop start address, in nibbles */
		uint32_t loop_end;        /* loop end address, in nibbles */
		uint32_t position;        /* current position, in nibbles */

		int32_t signal;           /* current ADPCM signal */
		int32_t step;             /* current ADPCM step */

		int32_t loop_signal;      /* signal at loop start */
		int32_t loop_step;        /* step at loop start */
		uint32_t loop_count;      /* number of loops so far */

		int32_t output_left;      /* output volume (left) */
		int32_t output_right;     /* output volume (right) */
		int32_t output_step;      /* step value for frequency conversion */
		int32_t output_pos;       /* current fractional position */
		int16_t last_sample;      /* last sample output */
		int16_t curr_sample;      /* current sample target */
		uint8_t irq_schedule;     /* 1 if the IRQ state is updated by timer */

		emu_timer *timer;
	};

	uint8_t ymz280b_read_memory(uint32_t offset);
	void update_irq_state();
	void update_step(struct YMZ280BVoice *voice);
	void update_volumes(struct YMZ280BVoice *voice);
	void update_irq_state_timer_common(int voicenum);
	int generate_adpcm(struct YMZ280BVoice *voice, int16_t *buffer, int samples);
	int generate_pcm8(struct YMZ280BVoice *voice, int16_t *buffer, int samples);
	int generate_pcm16(struct YMZ280BVoice *voice, int16_t *buffer, int samples);
	void write_to_register(int data);
	int compute_status();

	// internal state
	struct YMZ280BVoice m_voice[8];   /* the 8 voices */
	uint8_t m_current_register;         /* currently accessible register */
	uint8_t m_status_register;          /* current status register */
	uint8_t m_irq_state;                /* current IRQ state */
	uint8_t m_irq_mask;                 /* current IRQ mask */
	uint8_t m_irq_enable;               /* current IRQ enable */
	uint8_t m_keyon_enable;             /* key on enable */
	uint8_t m_ext_mem_enable;           /* external memory enable */
	uint8_t m_ext_readlatch;            /* external memory prefetched data */
	uint32_t m_ext_mem_address_hi;
	uint32_t m_ext_mem_address_mid;
	uint32_t m_ext_mem_address;         /* where the CPU can read the ROM */

	devcb_write_line m_irq_handler;  /* IRQ callback */
	devcb_read8 m_ext_read_handler;  /* external RAM read handler */
	devcb_write8 m_ext_write_handler;/* external RAM write handler */

	double m_master_clock;            /* master clock frequency */
	uint8_t *m_mem_base;                /* pointer to the base of external memory */
	uint32_t m_mem_size;
	sound_stream *m_stream;           /* which stream are we using */
	std::unique_ptr<int16_t[]> m_scratch;
#if MAKE_WAVS
	void *m_wavresample;              /* resampled waveform */
#endif
};

extern const device_type YMZ280B;


#endif /* __YMZ280B_H__ */
