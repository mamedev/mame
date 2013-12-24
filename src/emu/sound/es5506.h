// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**********************************************************************************************
 *
 *   Ensoniq ES5505/6 driver
 *   by Aaron Giles
 *
 **********************************************************************************************/

#pragma once

#ifndef __ES5506_H__
#define __ES5506_H__

#define MAKE_WAVS               0


struct es5505_interface
{
	const char * m_es5505_region0;                       /* memory region where the sample ROM lives */
	const char * m_es5505_region1;                       /* memory region where the sample ROM lives */
	int m_es5505_channels;                               /* number of output channels: 1 .. 4 */
	devcb_write_line m_es5505_irq_callback;  /* irq callback */
	devcb_read16 m_es5505_read_port;          /* input port read */
};

struct es5506_interface
{
	const char * m_region0;                       /* memory region where the sample ROM lives */
	const char * m_region1;                       /* memory region where the sample ROM lives */
	const char * m_region2;                       /* memory region where the sample ROM lives */
	const char * m_region3;                       /* memory region where the sample ROM lives */
	int m_channels;                               /* number of output channels: 1 .. 6 */
	devcb_write_line m_irq_callback;  /* irq callback */
	devcb_read16 m_read_port;          /* input port read */
};

/* struct describing a single playing voice */

struct es550x_voice
{
		es550x_voice():
		control(0),
		freqcount(0),
		start(0),
		lvol(0),
		end(0),
		lvramp(0),
		accum(0),
		rvol(0),
		rvramp(0),
		ecount(0),
		k2(0),
		k2ramp(0),
		k1(0),
		k1ramp(0),
		o4n1(0),
		o3n1(0),
		o3n2(0),
		o2n1(0),
		o2n2(0),
		o1n1(0),
		exbank(0),
		index(0),
		filtcount(0),
		accum_mask(0) {}

	/* external state */
	UINT32      control;                /* control register */
	UINT32      freqcount;              /* frequency count register */
	UINT32      start;                  /* start register */
	UINT32      lvol;                   /* left volume register */
	UINT32      end;                    /* end register */
	UINT32      lvramp;                 /* left volume ramp register */
	UINT32      accum;                  /* accumulator register */
	UINT32      rvol;                   /* right volume register */
	UINT32      rvramp;                 /* right volume ramp register */
	UINT32      ecount;                 /* envelope count register */
	UINT32      k2;                     /* k2 register */
	UINT32      k2ramp;                 /* k2 ramp register */
	UINT32      k1;                     /* k1 register */
	UINT32      k1ramp;                 /* k1 ramp register */
	INT32       o4n1;                   /* filter storage O4(n-1) */
	INT32       o3n1;                   /* filter storage O3(n-1) */
	INT32       o3n2;                   /* filter storage O3(n-2) */
	INT32       o2n1;                   /* filter storage O2(n-1) */
	INT32       o2n2;                   /* filter storage O2(n-2) */
	INT32       o1n1;                   /* filter storage O1(n-1) */
	UINT32      exbank;                 /* external address bank */

	/* internal state */
	UINT8       index;                  /* index of this voice */
	UINT8       filtcount;              /* filter count */
	UINT32      accum_mask;
};

class es550x_device : public device_t,
									public device_sound_interface
{
public:
	es550x_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~es550x_device() {}

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	// internal state
	sound_stream *m_stream;               /* which stream are we using */
	int         m_sample_rate;            /* current sample rate */
	UINT16 *    m_region_base[4];         /* pointer to the base of the region */
	UINT32      m_write_latch;            /* currently accumulated data for write */
	UINT32      m_read_latch;             /* currently accumulated data for read */
	UINT32      m_master_clock;           /* master clock frequency */
	devcb_resolved_write_line m_irq_callback_func;   /* IRQ callback */
	devcb_resolved_read16     m_port_read_func;       /* input port read */

	UINT8       m_current_page;           /* current register page */
	UINT8       m_active_voices;          /* number of active voices */
	UINT8       m_mode;                   /* MODE register */
	UINT8       m_wst;                    /* W_ST register */
	UINT8       m_wend;                   /* W_END register */
	UINT8       m_lrend;                  /* LR_END register */
	UINT8       m_irqv;                   /* IRQV register */

	es550x_voice m_voice[32];             /* the 32 voices */

	INT32 *     m_scratch;

	INT16 *     m_ulaw_lookup;
	UINT16 *    m_volume_lookup;

	#if MAKE_WAVS
	void *      m_wavraw;                 /* raw waveform */
	#endif

	FILE *m_eslog;

	void update_irq_state();
	void update_internal_irq_state();
	void compute_tables();

	void generate_dummy(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples);
	void generate_ulaw(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples);
	void generate_pcm(es550x_voice *voice, UINT16 *base, INT32 *lbuffer, INT32 *rbuffer, int samples);
};


class es5506_device : public es550x_device,
									public es5506_interface
{
public:
	es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~es5506_device() {}

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);


	void generate_samples(INT32 **outputs, int offset, int samples);

private:
	inline void reg_write_low(es550x_voice *voice, offs_t offset, UINT32 data);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, UINT32 data);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, UINT32 data);
	inline UINT32 reg_read_low(es550x_voice *voice, offs_t offset);
	inline UINT32 reg_read_high(es550x_voice *voice, offs_t offset);
	inline UINT32 reg_read_test(es550x_voice *voice, offs_t offset);
};

extern const device_type ES5506;


class es5505_device : public es550x_device,
									public es5505_interface
{
public:
	es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( write );
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	void generate_samples(INT32 **outputs, int offset, int samples);

private:
	// internal state
	inline void reg_write_low(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, UINT16 data, UINT16 mem_mask);
	inline UINT16 reg_read_low(es550x_voice *voice, offs_t offset);
	inline UINT16 reg_read_high(es550x_voice *voice, offs_t offset);
	inline UINT16 reg_read_test(es550x_voice *voice, offs_t offset);
};

extern const device_type ES5505;


#endif /* __ES5506_H__ */
