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
	enum {
		LP3 = 1,
		LP4 = 2,
		LP_MASK = LP3 | LP4
	};

	// constants for volumes
	const s8 VOLUME_ACC_BIT = 20;

	// constants for address
	const s8 ADDRESS_FRAC_BIT = 11;

	// struct describing a single playing voice
	struct es550x_voice
	{
		es550x_voice() { }

		// external state
		u32      control   = 0;          // control register
		u64      freqcount = 0;          // frequency count register
		u64      start     = 0;          // start register
		u32      lvol      = 0;          // left volume register
		u64      end       = 0;          // end register
		u32      lvramp    = 0;          // left volume ramp register
		u64      accum     = 0;          // accumulator register
		u32      rvol      = 0;          // right volume register
		u32      rvramp    = 0;          // right volume ramp register
		u32      ecount    = 0;          // envelope count register
		u32      k2        = 0;          // k2 register
		u32      k2ramp    = 0;          // k2 ramp register
		u32      k1        = 0;          // k1 register
		u32      k1ramp    = 0;          // k1 ramp register
		s32      o4n1      = 0;          // filter storage O4(n-1)
		s32      o3n1      = 0;          // filter storage O3(n-1)
		s32      o3n2      = 0;          // filter storage O3(n-2)
		s32      o2n1      = 0;          // filter storage O2(n-1)
		s32      o2n2      = 0;          // filter storage O2(n-2)
		s32      o1n1      = 0;          // filter storage O1(n-1)
		u64      exbank    = 0;          // external address bank

		// internal state
		u8       index      = 0;         // index of this voice
		u8       filtcount  = 0;         // filter count
	};

	es550x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;
	virtual void device_stop() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void update_irq_state();
	void update_internal_irq_state();
	void compute_tables(u32 total_volume_bit = 16, u32 exponent_bit = 4, u32 mantissa_bit = 8);
	void get_accum_mask(u32 address_integer = 21, u32 address_frac = 11);

	virtual inline u32 get_bank(u32 control) { return 0; }
	virtual inline u32 get_ca(u32 control) { return 0; }
	virtual inline u32 get_lp(u32 control) { return 0; }

	template<typename T, typename U> inline T lshift_signed(T val, U shift) { return (shift >= 0) ? val << shift : val >> (-shift); }
	template<typename T, typename U> inline T rshift_signed(T val, U shift) { return (shift >= 0) ? val >> shift : val << (-shift); }

	inline u64 get_volume(u32 volume) { return m_volume_lookup[rshift_signed<u32, s8>(volume, m_volume_shift)]; }

	inline u64 get_address_acc_shifted_val(u64 val, int bias = 0) { return lshift_signed<u64, s8>(val, m_address_acc_shift - bias); }
	inline u64 get_address_acc_res(u64 val, int bias = 0) { return rshift_signed<u64, s8>(val, m_address_acc_shift - bias); }
	inline u64 get_integer_addr(u64 accum, s32 bias = 0) { return ((accum + (bias << ADDRESS_FRAC_BIT)) & m_address_acc_mask) >> ADDRESS_FRAC_BIT; }

	inline s64 get_sample(s32 sample, u32 volume) { return rshift_signed<s64, s8>(sample * get_volume(volume), m_volume_acc_shift); }

	inline s32 interpolate(s32 sample1, s32 sample2, u64 accum);
	inline void apply_filters(es550x_voice *voice, s32 &sample);
	virtual inline void update_envelopes(es550x_voice *voice) {};
	virtual inline void check_for_end_forward(es550x_voice *voice, u64 &accum) {};
	virtual inline void check_for_end_reverse(es550x_voice *voice, u64 &accum) {};
	void generate_dummy(es550x_voice *voice, u16 *base, s32 *lbuffer, s32 *rbuffer, int samples);
	void generate_ulaw(es550x_voice *voice, u16 *base, s32 *lbuffer, s32 *rbuffer, int samples);
	void generate_pcm(es550x_voice *voice, u16 *base, s32 *lbuffer, s32 *rbuffer, int samples);
	inline void generate_irq(es550x_voice *voice, int v);
	virtual void generate_samples(s32 **outputs, int offset, int samples) {};

	// internal state
	sound_stream *m_stream;               /* which stream are we using */
	int           m_sample_rate;          /* current sample rate */
	u16 *         m_region_base[4];       /* pointer to the base of the region */
	u32           m_master_clock;         /* master clock frequency */
	s8            m_address_acc_shift;
	u64           m_address_acc_mask;

	s8            m_volume_shift;
	s64           m_volume_acc_shift;

	u8            m_current_page;         /* current register page */
	u8            m_active_voices;        /* number of active voices */
	u16           m_mode;                 /* MODE register */
	u8            m_irqv;                 /* IRQV register */

	es550x_voice  m_voice[32];             /* the 32 voices */

	std::unique_ptr<s32[]>    m_scratch;

	std::unique_ptr<s16[]>    m_ulaw_lookup;
	std::unique_ptr<u32[]>    m_volume_lookup;

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
	es5506_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	~es5506_device() {}

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	const s8 VOLUME_BIT_ES5506 = 16;
	const s8 ADDRESS_INTEGER_BIT_ES5506 = 21;
	const s8 ADDRESS_FRAC_BIT_ES5506 = 11;

	virtual inline u32 get_bank(u32 control) override { return (control >> 14) & 3; }
	virtual inline u32 get_ca(u32 control) override { return (control >> 10) & 7; }
	virtual inline u32 get_lp(u32 control) override { return (control >> 8) & LP_MASK; }

	virtual inline void update_envelopes(es550x_voice *voice) override;
	virtual inline void check_for_end_forward(es550x_voice *voice, u64 &accum) override;
	virtual inline void check_for_end_reverse(es550x_voice *voice, u64 &accum) override;
	virtual void generate_samples(s32 **outputs, int offset, int samples) override;

private:
	inline void reg_write_low(es550x_voice *voice, offs_t offset, u32 data);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, u32 data);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, u32 data);
	inline u32 reg_read_low(es550x_voice *voice, offs_t offset);
	inline u32 reg_read_high(es550x_voice *voice, offs_t offset);
	inline u32 reg_read_test(es550x_voice *voice, offs_t offset);

	// ES5506 specific registers
	u32      m_write_latch;            /* currently accumulated data for write */
	u32      m_read_latch;             /* currently accumulated data for read */
	u8       m_wst;                    /* W_ST register */
	u8       m_wend;                   /* W_END register */
	u8       m_lrend;                  /* LR_END register */
};

DECLARE_DEVICE_TYPE(ES5506, es5506_device)


class es5505_device : public es550x_device
{
public:
	es5505_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);
	void voice_bank_w(int voice, int bank);

protected:
	// device-level overrides
	virtual void device_start() override;

	const s8 VOLUME_BIT_ES5505 = 8;
	const s8 ADDRESS_INTEGER_BIT_ES5505 = 20;
	const s8 ADDRESS_FRAC_BIT_ES5505 = 9;

	virtual inline u32 get_lp(u32 control) override { return (control >> 10) & LP_MASK; }
	virtual inline u32 get_ca(u32 control) override { return (control >> 8) & 3; }
	virtual inline u32 get_bank(u32 control) override { return (control >> 2) & 1; }

	virtual inline void update_envelopes(es550x_voice *voice) override;
	virtual inline void check_for_end_forward(es550x_voice *voice, u64 &accum) override;
	virtual inline void check_for_end_reverse(es550x_voice *voice, u64 &accum) override;
	virtual void generate_samples(s32 **outputs, int offset, int samples) override;

private:
	// internal state
	inline void reg_write_low(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask);
	inline void reg_write_high(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask);
	inline void reg_write_test(es550x_voice *voice, offs_t offset, u16 data, u16 mem_mask);
	inline u16 reg_read_low(es550x_voice *voice, offs_t offset);
	inline u16 reg_read_high(es550x_voice *voice, offs_t offset);
	inline u16 reg_read_test(es550x_voice *voice, offs_t offset);
};

DECLARE_DEVICE_TYPE(ES5505, es5505_device)


#endif // MAME_SOUND_ES5506_H
