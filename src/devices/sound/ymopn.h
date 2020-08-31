// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh,Aaron Giles

#ifndef MAME_SOUND_YMOPN_H
#define MAME_SOUND_YMOPN_H

#pragma once

#include "ay8910.h"
#include "dirom.h"

#define YMOPN_NEW_SOUND (0)

class ymopn_device_base;

DECLARE_DEVICE_TYPE(YM2203, ym2203_device);
DECLARE_DEVICE_TYPE(YM2608, ym2608_device);
DECLARE_DEVICE_TYPE(YM2610, ym2610_device);
DECLARE_DEVICE_TYPE(YM2610B, ym2610b_device);


// ======================> ymopn_block_fnum

class ymopn_block_fnum
{
public:
	ymopn_block_fnum() : m_raw(0) { }

	void set(u16 raw) { m_raw = raw & 0x3fff; }

	u8 block() const { return (m_raw >> 11) & 7; }
	u16 fnum() const { return m_raw & 0x7ff; }

	u8 keycode() const;
	u32 fc(s32 offset = 0) const;

	u16 m_raw;
};


// ======================> ymopn_3slot_state

class ymopn_3slot_state
{
public:
	ymopn_3slot_state() : m_upper(0) { }
	void set_fnum(u8 chnum, u8 value) { m_blk_fnum[chnum].set((m_upper << 8) | value); }

	ymopn_block_fnum m_blk_fnum[3]; // three 16-bit block/fnum values
	u8 m_upper;            // upper 8-bit latch
};


// ======================> ymopn_slot

class ymopn_slot
{
public:
	// constructor
	ymopn_slot(ymopn_device_base &opn);

	// register for save states
	void save(int index);

	// reset the state
	void reset();

	// return the current volume, applying AM
	u32 volume(u32 am) const { return m_total_level + m_envelope_volume + (am & m_am_mask); }

	// return whether the key is on or off
	bool key() const { return (m_key != 0); }

	// return the current phase
	u32 phase() const { return m_phase; }

	// parameter settings
	void set_det_mul(u8 value);
	void set_ar_ksr(u8 value);
	void set_dr_am(u8 value);
	void set_sr(u8 value);
	void set_sl_rr(int value);
	void set_tl(u8 value);
	void set_ssg(u8 value);

	// process a key on/off signals
	void keyonoff(bool on);

	// updates
	void refresh_fc_eg(ymopn_block_fnum blk_fnum);
	void update_phase_lfo(s32 lfo_pm, s32 pm_shift, ymopn_block_fnum blk_fnum);
	void advance_eg(u32 eg_cnt);

private:
	// set of basic parameters for each envelope generator phase
	struct eg_params
	{
		eg_params() : rate(0), shift(0), inctable(0) { }
		u8 rate;              // raw rate value; kept for recalcs
		u8 shift;             // bits to shift eg_count left to make 4.12
		u8 inctable;          // index into the increment steps table
	};
	void update_eg_params(eg_params &params);
	void update_eg_params_all();

	// internal state
	ymopn_device_base &m_opn; // reference to base device
	u8 m_detune;              // detune table index
	u8 m_ksr_shift;           // key scale rate  :3-KSR
	u8 m_attack_rate;         // attack rate
	u8 m_decay_rate;          // decay rate
	u8 m_sustain_rate;        // sustain rate
	u8 m_release_rate;        // release rate
	u8 m_ksr;                 // key scale rate  :kcode>>(3-KSR)
	u8 m_multiply;            // multiple (multiply?)

	// phase generator
	u32 m_phase;              // phase counter
	s32 m_phase_step;         // phase step

	// envelope generator
	u8 m_eg_state;            // phase type
	eg_params m_eg_params[4]; // parameters for 4 EG phases
	u32 m_total_level;        // total level: TL << 3
	s32 m_volume;             // envelope counter
	u32 m_sustain_level;      // sustain level:s_sl_table[SL]
	u32 m_envelope_volume;    // current output from EG circuit (without AM from LFO)

	u8 m_ssg;                 // SSG-EG waveform
	u8 m_ssg_state;           // SSG-EG negated output; bit 1=negate, bit 0=have we swapped yet?
	u8 m_key;                 // 0=last key was KEY OFF, 1=KEY ON
	u32 m_am_mask;            // AM enable flag
};


// ======================> ymopn_channel

class ymopn_channel
{
public:
	// constructor
	ymopn_channel(ymopn_device_base &opn);

	// save state handling
	void save(int index);
	void post_load();

	// enable/disable
	bool disabled() const { return m_disabled; }
	void set_disabled(bool disabled) { m_disabled = disabled; }

	// reset our state and all our owned slots
	void reset();

	// return a reference to the given slot; note that the indexing order is
	// different than the expected order
	ymopn_slot &slot(int index)
	{
		switch (index)
		{
			default:
			case 0:	return m_slot1;
			case 1:	return m_slot3;
			case 2:	return m_slot2;
			case 3:	return m_slot4;
		}
	}
	ymopn_slot &slot1() { return m_slot1; }
	ymopn_slot &slot2() { return m_slot2; }
	ymopn_slot &slot3() { return m_slot3; }
	ymopn_slot &slot4() { return m_slot4; }

	// return the computed output value, with panning applied
	s32 output() const { return m_out_fm; }
	s32 output_l() const { return BIT(m_pan, 1) ? output() : 0; }
	s32 output_r() const { return BIT(m_pan, 0) ? output() : 0; }

	// parameter settings
	void set_three_slot_mode(ymopn_3slot_state *state);
	void set_fnum(u8 upper, u8 value);
	void set_lfo_shift_pan(u8 value);
	void set_algorithm_feedback(u8 value);

	// called to force a refresh of the parameters
	void force_refresh() { m_refresh = true; }

	// updates
	void refresh_fc_eg();
	void advance_eg(u32 eg_cnt);
	void update(u32 lfo_am, u32 lfo_pm);
	void csm_key_control();

private:
	// helpers
	s32 op_calc(u32 phase, u32 env, s32 pm);
	void set_connections(s32 *c1, s32 *c2, s32 *c3, s32 *c4, s32 *mem);
	void setup_connection();

	// internal state
	ymopn_device_base &m_opn; // reference to base device
	bool m_refresh;           // true if slots need a refresh
	bool m_disabled;          // true of this channel is disabled
	ymopn_slot m_slot1;       // four SLOTs (operators)
	ymopn_slot m_slot2;       // four SLOTs (operators)
	ymopn_slot m_slot3;       // four SLOTs (operators)
	ymopn_slot m_slot4;       // four SLOTs (operators)

	u8 m_algorithm;           // algorithm
	u8 m_fb_shift;            // feedback shift
	s32 m_op1_out[2];         // op1 output for feedback

	s32 *m_connect1;          // SLOT1 output pointer
	s32 *m_connect3;          // SLOT3 output pointer
	s32 *m_connect2;          // SLOT2 output pointer
	s32 *m_connect4;          // SLOT4 output pointer
	s32 *m_mem_connect;       // where to put the delayed sample (MEM)
	s32 m_mem_value;          // delayed sample (MEM) value

	s32 m_pm_shift;           // PM shift
	u8 m_am_shift;            // AM shift

	ymopn_block_fnum m_blk_fnum; // current blk/fnum values

	s32 m_m2, m_c1, m_c2;     // phase modulation input for slots 2,3,4
	s32 m_mem;                // one sample delay memory
	s32 m_out_fm;             // outputs of working channels
	u8 m_pan;                 // pan values (bit 1 = left, bit 0 = right)

	ymopn_3slot_state *m_3slot; // pointer to 3-slot state if active (or nullptr if not)
};


// ======================> ymopn_adpcm_channel

class ymopn_adpcm_channel
{
	static constexpr int FRAC_SHIFT = 16;
	static constexpr u32 FRAC_ONE = 1 << FRAC_SHIFT;

public:
	// constructor
	ymopn_adpcm_channel(ymopn_device_base &opn);

	// save state handling
	void save(int index);

	// reset our status
	void reset();

	// return the computed output value, with panning applied
	s32 output() const
	{
		int vol = m_instrument_level + m_total_level;
		if (vol >= 63)
			return 0;

		s8 mul = 15 - (vol & 7);
		u8 shift = 1 + (vol >> 3);
		return ((m_adpcm_acc * mul) >> shift) & ~3;
	}
	s32 output_l() const { return BIT(m_pan, 1) ? output() : 0; }
	s32 output_r() const { return BIT(m_pan, 0) ? output() : 0; }

	// signal key on/off
	void keyonoff(bool on);

	// parameter setting
	void set_volume_pan(u8 value);
	void set_start_byte(u8 value, u8 shift) { m_start = (m_start & ~(0xff << shift)) | (value << shift); }
	void set_end_byte(u8 value, u8 shift) { m_end = (m_end & ~(0xff << shift)) | (value << shift); }

	// direct parameter setting
	void set_tl(u8 value) { m_total_level = value; }
	void set_start(u32 value) { m_start = value; m_addrshift = 1; }
	void set_end(u32 value) { m_end = value; }
	void set_step_divisor(u8 value) { m_step_divisor = value; }

	// overall update
	bool update();

private:
	// internal helpers
	u32 start_address() const { return m_start << m_addrshift; }
	u32 end_address() const { return ((m_end + 1) << m_addrshift) - 2; }

	// internal state
	ymopn_device_base &m_opn; // reference to base device
	u8 m_total_level;         // total level
	u8 m_instrument_level;    // instrument level
	u8 m_flag;                // port state
	u8 m_curbyte;             // current ROM data
	u32 m_curaddress;         // current ROM address
	u32 m_curfrac;            // current fractional address
	u16 m_start;              // sample data start address
	u16 m_end;                // sample data end address
	s32 m_adpcm_acc;          // accumulator
	s32 m_adpcm_step;         // step
	u8 m_addrshift;           // address bits shift-left
	u8 m_pan;                 // pan (bit 1=L, bit 0=R)
	u8 m_step_divisor;        // step divisor
};


// ======================> ymopn_deltat_channel

class ymopn_deltat_channel
{
	static constexpr int FRAC_SHIFT = 16;
	static constexpr u32 FRAC_ONE = 1 << FRAC_SHIFT;
	static constexpr s32 DELTA_MIN = 127;
	static constexpr s32 DELTA_MAX = 24576;
	static constexpr s32 DELTA_DEFAULT = DELTA_MIN;

public:
	static constexpr u8 STATUS_EOS = 0x01;
	static constexpr u8 STATUS_BRDY = 0x02;
	static constexpr u8 STATUS_BUSY = 0x04;

	// constructor
	ymopn_deltat_channel(ymopn_device_base &opn);

	// save state handling
	void save(int index);

	// reset the state
	void reset();

	// status
	u8 status() const { return m_status; }
	void status_set_reset(u8 set, u8 reset = 0);

	// return the computed output value, with panning applied
	s32 output() const { return (m_output * m_volume) >> 8; }
	s32 output_l() const { return BIT(m_control2, 7) ? output() : 0; }
	s32 output_r() const { return BIT(m_control2, 6) ? output() : 0; }

	// parameter setting
	void set_portstate(u8 value);
	void set_pan_control2(u8 value, u8 shift_override = 0);
	void set_start_byte(u8 value, u8 shift) { m_start = (m_start & ~(0xff << shift)) | (value << shift); }
	void set_delta_byte(u8 value, u8 shift) { m_delta = (m_delta & ~(0xff << shift)) | (value << shift); }
	void set_end_byte(u8 value, u8 shift) { m_end = (m_end & ~(0xff << shift)) | (value << shift); }
	void set_limit_byte(u8 value, u8 shift) { m_limit = (m_limit & ~(0xff << shift)) | (value << shift); }
	void set_volume(u8 value) { m_volume = value; }

	// direct data access
	void write_data(u8 value);
	u8 read_data();

	// overall update
	bool update();

private:
	// internal helpers
	u32 start_address() const { return m_start << m_addrshift; }
	u32 end_address() const { return ((m_end + 1) << m_addrshift) - 2; }
	u32 limit_address() const { return m_limit << m_addrshift; }

	// internal state
	ymopn_device_base &m_opn; // reference to base device

	u32 m_curaddress;         // current ROM address, as a nibble index
	u32 m_curfrac;            // current fractional address
	u16 m_start;              // sample data start address
	u16 m_limit;              // limit address
	u16 m_end;                // end address
	u16 m_delta;              // delta scale
	s16 m_volume;             // current volume
	s32 m_adpcm_acc;          // current accumulator
	s32 m_prev_acc;           // previous accumulator
	s32 m_adpcm_step;         // next forecast
	s32 m_output;             // current interpolated output value
	u8 m_curbyte;             // current rom data
	u8 m_cpudata;             // current data from reg 08
	u8 m_portstate;           // port status
	u8 m_control2;            // control reg: SAMPLE, DA/AD, RAM TYPE (x8bit / x1bit), ROM/RAM
	u8 m_addrshift;           // address bits shift-left
	u8 m_memread;             // needed for reading/writing external memory
	u8 m_pcm_busy;            // set when playing
	u8 m_status;              // current status
};


// ======================> ymopn_device_base

class ymopn_device_base : public ay8910_device
{
public:
	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

	// overridables
	virtual u8 adpcm_read(offs_t address);
	virtual u8 deltat_read(offs_t address);
	virtual void deltat_write(offs_t address, u8 data);
	virtual void deltat_status_change(u8 status);

protected:
	// status bits
	static constexpr u8 STATUS_BUSY = 0x80;
	static constexpr u8 STATUS_DELTAT_ZERO_2608 = 0x10;
	static constexpr u8 STATUS_DELTAT_BRDY_2608 = 0x08;
	static constexpr u8 STATUS_DELTAT_EOS_2608 = 0x04;
	static constexpr u8 STATUS_TIMERB = 0x02;
	static constexpr u8 STATUS_TIMERA = 0x01;

	// constructor
	ymopn_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

	// return a reference to the given channel; if extended is true,
	// return an extended channel, if currently enabled
	ymopn_channel &channel(int index, bool extended = false)
	{
		if (extended && m_channel.size() > 3)
			index += 3;
		return *m_channel[index];
	}

	// return a reference to the channel with 3-slot support
	ymopn_channel &three_slot_channel() { return channel(2); }

	// return a reference to an ADPCM channel
	ymopn_adpcm_channel &adpcm(int index) { return *m_adpcm_channel[index]; }

	// return a reference to the delta-T channel
	ymopn_deltat_channel &deltat() { return *m_deltat_channel; }

	// status/IRQ handling
	u8 status();
	void status_set_reset(u8 set = 0, u8 reset = 0);
	void set_irqmask(u8 flag);
	void busy_set();

	// OPN Mode Register Write
	void set_mode(u8 value);

	// writes to registers
	void write_mode(u8 reg, u8 value);
	void write_reg(u16 reg, u8 value);
	void write_adpcm(u8 reg, u8 value);
	void write_deltat(u8 reg, u8 value);

	// prescale config
	void prescaler_w(u8 addr);
	void set_prescale(u8 sel);

	// force an update
	void stream_update() { m_stream->update(); }

	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
#if (YMOPN_NEW_SOUND)
	virtual void sound_stream_update_ex(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs) override;
#else
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
#endif

	enum
	{
		TIMER_A,
		TIMER_B,
		TIMER_IRQ_SYNC,
		TIMER_WRITE_REG,
		TIMER_WRITE_MODE,
		TIMER_WRITE_ADPCM,
		TIMER_WRITE_DELTAT
	};

	u8 m_adpcm_status;         // ADPCM status flag

private:
	// internal helpers
	void init_tables();
	attotime timer_a_period() const;
	attotime timer_b_period() const;

	// internal state
	ymopn_3slot_state m_3slot_state; // 3 slot mode state
	std::vector<std::unique_ptr<ymopn_channel>> m_channel;
	std::vector<std::unique_ptr<ymopn_adpcm_channel>> m_adpcm_channel;
	std::unique_ptr<ymopn_deltat_channel> m_deltat_channel;

	// envelope generator
	u16 m_eg_count;            // EG counter
	u8 m_eg_subcount;          // EG sub-counter

	// LFO
	u8 m_lfo_count;            // LFO counter
	u8 m_lfo_subcount;         // LFO sub-counter
	u8 m_lfo_submax;           // LFO sub-counter maximum value

	u8 m_prescaler_sel;        // prescaler selector
	u32 m_timer_prescaler;     // timer prescaler

	attotime m_busy_expiry_time; // expiry time of the busy status
	u8 m_irq;                  // interrupt level
	u8 m_irqmask;              // irq mask
	u8 m_status;               // status flag
	u8 m_mode;                 // mode  CSM / 3SLOT
	u8 m_fn_h;                 // freq latch

	emu_timer *m_timer_a;      // timer a
	u16 m_timer_a_value;       // timer a reset value
	emu_timer *m_timer_b;      // timer b
	u8 m_timer_b_value;        // timer b reset value

	sound_stream *m_stream;    // sound stream
	devcb_write_line m_irq_handler; // IRQ callback
};


// ======================> ym2203_device

class ym2203_device : public ymopn_device_base
{
public:
	ym2203_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	u8 status_port_r() { return read(0); }
	u8 read_port_r() { return read(1); }
	void control_port_w(u8 data) { write(0, data); }
	void write_port_w(u8 data) { write(1, data); }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	u8 m_address;              // address register
};


// ======================> ym2608_device

class ym2608_device : public ymopn_device_base, public device_rom_interface<21>
{
public:
	ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void rom_bank_updated() override;

	// ymopn_base_device overrides
	virtual u8 adpcm_read(offs_t address) override;
	virtual u8 deltat_read(offs_t address) override;
	virtual void deltat_write(offs_t address, u8 data) override;
	virtual void deltat_status_change(u8 status) override;

private:
	// internal helpers
	void irq_flag_write(u8 value);
	void irq_mask_write(u8 value);

	// internal state
	u16 m_address;             // address register
	u8 m_2608_flagmask;        // flag mask
	u8 m_2608_irqmask;         // IRQ mask
	required_memory_region m_internal; // internal memory region
};


// ======================> ym2610_device

class ym2610_device : public ymopn_device_base, public device_memory_interface
{
public:
	ym2610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type = YM2610);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	virtual space_config_vector memory_space_config() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// ymopn_base_device overrides
	virtual u8 adpcm_read(offs_t address) override;
	virtual u8 deltat_read(offs_t address) override;

private:
	// internal state
	u16 m_address;             // address register
	u8 m_2610_flagmask;        // flag mask

	const address_space_config m_adpcm_a_config;
	const address_space_config m_adpcm_b_config;
	const std::string m_adpcm_b_region_name;
	optional_memory_region m_adpcm_a_region;
	optional_memory_region m_adpcm_b_region;
};


// ======================> ym2610b_device

class ym2610b_device : public ym2610_device
{
public:
	ym2610b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

#endif // MAME_SOUND_YMOPN_H
