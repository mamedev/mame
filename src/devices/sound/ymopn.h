// license:GPL-2.0+
// copyright-holders:Jarek Burczynski,Tatsuyuki Satoh
/*
  File: fm.h -- header file for software emulation for FM sound generator

*/

#ifndef MAME_SOUND_YMOPN_H
#define MAME_SOUND_YMOPN_H

#pragma once

#include "ay8910.h"

class ymopn_device_base;


DECLARE_DEVICE_TYPE(YM2203, ym2203_device);
#include "2608intf.h"
#include "2610intf.h"
//DECLARE_DEVICE_TYPE(YM2608, ym2608_device);
//DECLARE_DEVICE_TYPE(YM2610, ym2610_device);
//DECLARE_DEVICE_TYPE(YM2610B, ym2610b_device);


// ======================> opn_3slot_t

struct opn_3slot_t
{
	opn_3slot_t();
	void set_fnum(ymopn_device_base &opn, u8 chnum, u8 value);

	u32  m_fc[3];          // fnum3,blk3: calculated
	u8   m_fn_h;           // freq3 latch
	u8   m_kcode[3];       // key code
	u32  m_block_fnum[3];  // current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode)
};


// ======================> opn_slot_t

class opn_slot_t
{
public:
	// constructor
	opn_slot_t(ymopn_device_base &opn);

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
	bool set_ar_ksr(u8 value);
	void set_dr_am(u8 value);
	void set_sr(u8 value);
	void set_sl_rr(int value);
	void set_tl(u8 value);
	void set_ssg(u8 value);

	// process a key on/off signals
	void keyonoff(bool on);

	// updates
	void refresh_fc_eg(int fc, int kc);
	void update_phase_lfo(s32 lfo_pm, s32 pm_shift, u32 block_fnum);
	void advance_eg(u32 eg_cnt);

private:
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
	u32 m_total_level;        // total level: TL << 3
	s32 m_volume;             // envelope counter
	u32 m_sustain_level;      // sustain level:s_sl_table[SL]
	u32 m_envelope_volume;    // current output from EG circuit (without AM from LFO)

	u8 m_attack_shift;        //  (attack state)
	u8 m_attack_select;       //  (attack state)
	u8 m_decay_shift;         //  (decay state)
	u8 m_decay_select;        //  (decay state)
	u8 m_sustain_shift;       //  (sustain state)
	u8 m_sustain_select;      //  (sustain state)
	u8 m_release_shift;       //  (release state)
	u8 m_release_select;      //  (release state)

	u8 m_ssg;                 // SSG-EG waveform
	u8 m_ssg_state;           // SSG-EG negated output; bit 1=negate, bit 0=have we swapped yet?
	u8 m_key;                 // 0=last key was KEY OFF, 1=KEY ON
	u32 m_am_mask;            // AM enable flag
};


// ======================> opn_channel_t

class opn_channel_t
{
public:
	// constructor
	opn_channel_t(ymopn_device_base &opn);

	// save state handling
	void save(int index);
	void post_load();

	// reset our state and all our owned slots
	void reset();

	// return a reference to the given slot; note that the indexing order is
	// different than the expected order
	opn_slot_t &slot(int index)
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
	opn_slot_t &slot1() { return m_slot1; }
	opn_slot_t &slot2() { return m_slot2; }
	opn_slot_t &slot3() { return m_slot3; }
	opn_slot_t &slot4() { return m_slot4; }

	// return the computed output value, with panning applied
	s32 output() const { return m_out_fm; }
	s32 output_l() const { return BIT(m_pan, 1) ? m_out_fm : 0; }
	s32 output_r() const { return BIT(m_pan, 0) ? m_out_fm : 0; }

	// parameter settings
	void set_three_slot_mode(opn_3slot_t *state);
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
	opn_slot_t m_slot1;       // four SLOTs (operators)
	opn_slot_t m_slot2;       // four SLOTs (operators)
	opn_slot_t m_slot3;       // four SLOTs (operators)
	opn_slot_t m_slot4;       // four SLOTs (operators)

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

	u32 m_fc;                 // fnum, blk
	u8 m_kcode;               // key code:
	u32 m_block_fnum;         // current blk/fnum value

	s32 m_m2, m_c1, m_c2;     // phase modulation input for slots 2,3,4
	s32 m_mem;                // one sample delay memory
	s32 m_out_fm;             // outputs of working channels
	u8 m_pan;                 // pan values (bit 1 = left, bit 0 = right)

	opn_3slot_t *m_3slot;     // pointer to 3-slot state if active (or nullptr if not)
};


// ======================> ymopn_device_base

class ymopn_device_base : public ay8910_device
{
public:
	// calculations
	u32 fn_value(u32 val) const;
	u32 fn_max() const;
	u32 fc_fix(s32 fc) const;

	// lookups
	s32 detune(u8 fd, u8 index) const;
	u32 lfo_step(u8 index) const;

	// configuration helpers
	auto irq_handler() { return m_irq_handler.bind(); }

protected:
	ymopn_device_base(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, device_type type);

	// return a reference to the given channel; if extended is true,
	// return an extended channel, if currently enabled
	opn_channel_t &channel(int index, bool extended = false)
	{
		if (extended && m_channel.size() > 3)
			index += 3;
		return *m_channel[index];
	}

	// return a reference to the channel with 3-slot support
	opn_channel_t &three_slot_channel() { return channel((type() == YM2610) ? 1 : 2); }

	// status/IRQ handling
	u8 status();
	void set_reset_status(u8 set = 0, u8 reset = 0);
	void set_irqmask(u8 flag);
	void busy_set();

	// OPN Mode Register Write
	void set_mode(u8 value);

	// advance the LFO
	void advance_lfo();

	// write a OPN mode register 0x20-0x2f
	void write_mode(u8 reg, u8 value);
	void write_reg(u16 reg, u8 value);

	// prescale config
	void prescaler_w(u8 addr);
	void set_prescale(u8 sel);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;
	virtual void device_stop() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void sound_stream_update_ex(sound_stream &stream, std::vector<read_stream_view> &inputs, std::vector<write_stream_view> &outputs) override;

	enum
	{
		TIMER_A,
		TIMER_B,
		TIMER_IRQ_SYNC,
		TIMER_WRITE_REG,
		TIMER_WRITE_MODE
	};

private:
	void init_tables();
	attotime timer_a_period() const;
	attotime timer_b_period() const;

	// internal state
	opn_3slot_t m_3slot_state; // 3 slot mode state
	std::vector<std::unique_ptr<opn_channel_t>> m_channel;

	u32 m_eg_count;            // global envelope generator counter
	u32 m_eg_timer;            // global envelope generator counter works at frequency = chipclock/64/3

	// LFO
	u32 m_lfo_am;              // runtime LFO calculations helper
	s32 m_lfo_pm;              // runtime LFO calculations helper
	u32 m_lfo_count;
	u32 m_lfo_step;

	u8 m_prescaler_sel;        // prescaler selector
	u8 m_freqbase;             // frequency base
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
	u8 m_regs[256];            // registers
};


#if 0

/* -------------------- YM2203(OPN) Interface -------------------- */

/*
** Initialize YM2203 emulator(s).
**
** 'num'           is the number of virtual YM2203's to allocate
** 'baseclock'
** 'rate'          is sampling rate
** 'TimerHandler'  timer callback handler when timer start and clear
** 'IRQHandler'    IRQ callback handler when changed IRQ level
** return      0 = success
*/
void * ym2203_init(device_t *device, int baseclock, int rate,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);

/*
** YM2203 clock changed notifier
*/
void ym2203_clock_changed(void *chip, int clock, int rate);

/*
** shutdown the YM2203 emulators
*/
void ym2203_shutdown(void *chip);

/*
** reset all chip registers for YM2203 number 'num'
*/
void ym2203_reset_chip(void *chip);

/*
** update one of chip
*/
void ym2203_update_one(void *chip, write_stream_view &buffer, int length);

/*
** Write
** return : InterruptLevel
*/
int ym2203_write(void *chip,int a,unsigned char v);

/*
** Read
** return : InterruptLevel
*/
unsigned char ym2203_read(void *chip,int a);

/*
**  Timer OverFlow
*/
int ym2203_timer_over(void *chip, int c);

/*
**  State Save
*/
void ym2203_postload(void *chip);

/* -------------------- YM2608(OPNA) Interface -------------------- */
void * ym2608_init(device_t *device, int baseclock, int rate,
	FM_READBYTE InternalReadByte,
	FM_READBYTE ExternalReadByte, FM_WRITEBYTE ExternalWriteByte,
	FM_TIMERHANDLER TimerHandler, FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void ym2608_clock_changed(void *chip, int clock, int rate);
void ym2608_shutdown(void *chip);
void ym2608_reset_chip(void *chip);
void ym2608_update_one(void *chip, std::vector<write_stream_view> &buffer);

int ym2608_write(void *chip, int a,unsigned char v);
unsigned char ym2608_read(void *chip,int a);
int ym2608_timer_over(void *chip, int c );
void ym2608_postload(void *chip);

/* -------------------- YM2610(OPNB) Interface -------------------- */
void * ym2610_init(device_t *device, int baseclock, int rate,
	FM_READBYTE adpcm_a_read_byte, FM_READBYTE adpcm_b_read_byte,
	FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const ssg_callbacks *ssg);
void ym2610_clock_changed(void *chip, int clock, int rate);
void ym2610_shutdown(void *chip);
void ym2610_reset_chip(void *chip);
void ym2610_update_one(void *chip, std::vector<write_stream_view> &buffer);
void ym2610b_update_one(void *chip, std::vector<write_stream_view> &buffer);
int ym2610_write(void *chip, int a,unsigned char v);
unsigned char ym2610_read(void *chip,int a);
int ym2610_timer_over(void *chip, int c );
void ym2610_postload(void *chip);

void * ym2612_init(device_t *device, int baseclock, int rate,
				FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler);
void ym2612_clock_changed(void *chip, int clock, int rate);
void ym2612_shutdown(void *chip);
void ym2612_reset_chip(void *chip);
void ym2612_update_one(void *chip, std::vector<write_stream_view> &buffer, u8 output_bits);
int ym2612_write(void *chip, int a,unsigned char v);
unsigned char ym2612_read(void *chip,int a);
int ym2612_timer_over(void *chip, int c );
void ym2612_postload(void *chip);
#endif

#endif // MAME_SOUND_YMOPN_H
