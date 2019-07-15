// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#ifndef MAME_SOUND_ES5506_H
#define MAME_SOUND_ES5506_H

#pragma once

#define ES5506_MAKE_WAVS 0

class es550x_device : public device_t, public device_sound_interface
{
public:
	void set_region0(const char *region0) { m_region0 = region0; }
	void set_region1(const char *region1) { m_region1 = region1; }
	void set_region2(const char *region2) { m_region2 = region2; }
	void set_region3(const char *region3) { m_region3 = region3; }
	void set_channels(int channels) { m_channels = channels; }

	auto irq_cb() { return m_irq_cb.bind(); }
	auto read_port_cb() { return m_read_port_cb.bind(); }
	auto sample_rate_changed() { return m_sample_rate_changed_cb.bind(); }

protected:
	// struct describing a single playing voice
	struct es550x_voice
	{
		es550x_voice() { }

		// external state
		uint32_t      control   = 0;          // control register
		uint32_t      freqcount = 0;          // frequency count register
		uint32_t      start     = 0;          // start register
		uint32_t      lvol      = 0;          // left volume register
		uint32_t      end       = 0;          // end register
		uint32_t      lvramp    = 0;          // left volume ramp register
		uint32_t      accum     = 0;          // accumulator register
		uint32_t      rvol      = 0;          // right volume register
		uint32_t      rvramp    = 0;          // right volume ramp register
		uint32_t      ecount    = 0;          // envelope count register
		uint32_t      k2        = 0;          // k2 register
		uint32_t      k2ramp    = 0;          // k2 ramp register
		uint32_t      k1        = 0;          // k1 register
		uint32_t      k1ramp    = 0;          // k1 ramp register
		int32_t       o4n1      = 0;          // filter storage O4(n-1)
		int32_t       o3n1      = 0;          // filter storage O3(n-1)
		int32_t       o3n2      = 0;          // filter storage O3(n-2)
		int32_t       o2n1      = 0;          // filter storage O2(n-1)
		int32_t       o2n2      = 0;          // filter storage O2(n-2)
		int32_t       o1n1      = 0;          // filter storage O1(n-1)
		uint32_t      exbank    = 0;          // external address bank

		// internal state
		uint8_t       index      = 0;         // index of this voice
		uint8_t       filtcount  = 0;         // filter count
		uint32_t      accum_mask = 0;
	};

	es550x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void update_irq_state();
	void update_internal_irq_state();
	void compute_tables();

	void generate_dummy(es550x_voice *voice, uint16_t *base, int32_t *lbuffer, int32_t *rbuffer, int samples);
	void generate_ulaw(es550x_voice *voice, uint16_t *base, int32_t *lbuffer, int32_t *rbuffer, int samples);
	void generate_pcm(es550x_voice *voice, uint16_t *base, int32_t *lbuffer, int32_t *rbuffer, int samples);

	// internal state
	sound_stream *m_stream;               /* which stream are we using */
	int         m_sample_rate;            /* current sample rate */
	uint16_t *    m_region_base[4];         /* pointer to the base of the region */
	uint32_t      m_write_latch;            /* currently accumulated data for write */
	uint32_t      m_read_latch;             /* currently accumulated data for read */
	uint32_t      m_master_clock;           /* master clock frequency */

	uint8_t       m_current_page;           /* current register page */
	uint8_t       m_active_voices;          /* number of active voices */
	uint8_t       m_mode;                   /* MODE register */
	uint8_t       m_wst;                    /* W_ST register */
	uint8_t       m_wend;                   /* W_END register */
	uint8_t       m_lrend;                  /* LR_END register */
	uint8_t       m_irqv;                   /* IRQV register */

	es550x_voice m_voice[32];             /* the 32 voices */

	std::unique_ptr<int32_t[]>     m_scratch;

	std::unique_ptr<int16_t[]>     m_ulaw_lookup;
	std::unique_ptr<uint16_t[]>    m_volume_lookup;

#if ES5506_MAKE_WAVS
	void *      m_wavraw;                 /* raw waveform */
#endif

	const char * m_region0;                       /* memory region where the sample ROM lives */
	const char * m_region1;                       /* memory region where the sample ROM lives */
	const char * m_region2;                       /* memory region where the sample ROM lives */
	const char * m_region3;                       /* memory region where the sample ROM lives */
	int m_channels;                               /* number of output channels: 1 .. 6 */
	devcb_write_line m_irq_cb;  /* irq callback */
	devcb_read16 m_read_port_cb;          /* input port read */
	devcb_write32 m_sample_rate_changed_cb;          /* callback for when sample rate is changed */
};


class es5506_device : public es550x_device
{
public:
	es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~es5506_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;


	void generate_samples(int32_t **outputs, int offset, int samples);

private:
	inline void reg_write_low(es550x_voice *voice, offs_t offset, uint32_t data);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, uint32_t data);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, uint32_t data);
	inline uint32_t reg_read_low(es550x_voice *voice, offs_t offset);
	inline uint32_t reg_read_high(es550x_voice *voice, offs_t offset);
	inline uint32_t reg_read_test(es550x_voice *voice, offs_t offset);
};

DECLARE_DEVICE_TYPE(ES5506, es5506_device)


class es5505_device : public es550x_device
{
public:
	es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void generate_samples(int32_t **outputs, int offset, int samples);

private:
	// internal state
	inline void reg_write_low(es550x_voice *voice, offs_t offset, uint16_t data, uint16_t mem_mask);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, uint16_t data, uint16_t mem_mask);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, uint16_t data, uint16_t mem_mask);
	inline uint16_t reg_read_low(es550x_voice *voice, offs_t offset);
	inline uint16_t reg_read_high(es550x_voice *voice, offs_t offset);
	inline uint16_t reg_read_test(es550x_voice *voice, offs_t offset);
};

DECLARE_DEVICE_TYPE(ES5505, es5505_device)


#endif // MAME_SOUND_ES5506_H
