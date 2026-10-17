// license:BSD-3-Clause
// copyright-holders:Wouter van Nifterick

// Yamaha YSS236-F (XT013A00) VOP3 DSP.
//
// CA0-CA6, CD00-CD15: 128 registers, one 16-bit word at base + reg * 2.
// Write only, as the FS1R firmware uses it (EG status 0x21 aside):
//   0x00       address latch: step, slot or index in bits 8-0. With bit 15
//              set, every data write advances it (bursts).
//   0x01       1 while initialising
//   0x05       mode: 0x1FF4 on the effect chip, 0x1004 on the filter
//   0x06-0x0A  program words of the step, word k in register 10 - k
//   0x0B       step constant (coefficient)
//   0x0C       step tag byte
//   0x0D/0x0E  delay memory offset of the slot, bits 17-9 and 8-0
//   0x16       config entries 1-15; 0x17 is 0x23 while a block is rewritten
//   0x20/0x1C  LFO phase reset mask, then 0 to apply it
//   0x24-0x27  LFO speed, wave, depth, direction of LFO n
//   0x28-0x2A  15 entries each, written once at boot
//
// The microprogram is not decoded. In the FS1R one VOP3 is the per-note
// filter on the YMP706 channel loop, modelled here from the coefficient
// cells the firmware writes. The other runs the effects: a 512-step image at
// boot, then per effect type a program window (variation, insertion and
// delay reverbs) with its constants, delay offsets and LFOs, bracketed by
// 0x17. Those writes are kept per step (m_prog, m_coef, m_tag, m_slot,
// m_cfg, m_lfo_reg) and match the firmware's own copies through type
// loads, parameter changes and performance changes, but nothing runs them.
// The effects are the models in yss236_fx.h, configured from the 112
// performance effect bytes passed to fx_params_w().
//
// Filter: 16 slots, YMP706 channel n is slot n. Per slot the firmware
// writes coefficient cells at step addresses given by set_slot_map():
//   cut        3085 + 169 * cutoff + key scaling, <= 0x6000.
//              Log frequency, 169 words per semitone.
//   reso       0..32634
//   feg_depth  depth * 288, depth -64..64
//   lfo_depth  depth * 124, depth 0..99
//   gain       4096 is unity
//   prog       program reg 6 bits 7-13 select the output tap, which
//              tap[] maps to LPF24, LPF18, LPF12, HPF, BPF, BEF.
//
// One filter EG and one LFO per channel, EG channel set by set_eg_slot()
// and selected with register 0:
//   0x2B  rate. 0 jumps to 0x2C, 255 freezes. Else log time.
//   0x2C  segment target. Crossing it sets the done bit (IRQ).
//   0x2D  aim. The curve is an exponential approach towards the aim
//         that crosses the target.
//   0x21  write: IRQ enable mask. Read: 0x10 | channel when one is done.
//   0x24  LFO speed (bits 0-9)
//   0x25  LFO wave (bits 5-7) and phase (bits 0-4)
//   0x20  LFO phase reset mask (key sync)
//
// Estimates, not measured: the EG time scale, the FEG depth scale, the
// LFO speed and depth scale, and the filter topologies themselves.

#ifndef MAME_SOUND_YSS236_H
#define MAME_SOUND_YSS236_H

#pragma once

#include <memory>

class ymp706_device;
namespace yss236_fx { struct FxSection; }

class yss236_device : public device_t, public device_sound_interface
{
public:
	// Coefficient step addresses and type taps for one filter slot.
	struct slot_map
	{
		u16 cut;
		u16 reso;
		u16 feg_depth;
		u16 lfo_depth;
		u16 gain;
		u16 prog;
		u8 tap[6];
	};

	yss236_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 24'576'000);
	~yss236_device();

	auto irq() { return m_irq.bind(); }

	// Enables the filter model and the EG interrupt.
	void set_filter_chip(bool filter) { m_filter_chip = filter; }

	// Enables the effect model. Inputs are the stereo dry, insertion,
	// variation send and reverb send buses; the output is the mix.
	void set_effect_chip(bool effect) { m_effect_chip = effect; }

	// The 112 effect bytes of the current performance (bulk 0x50-0xbf).
	void fx_params_w(u8 const *fx);

	void map(address_map &map) ATTR_COLD;

	void set_slot_map(int slot, slot_map const &m) { m_map[slot] = m; }
	void set_eg_slot(int eg, int slot) { m_eg_slot[eg] = slot; m_slot_eg[slot] = eg; }

	// TGs that call filter_frame. Their streams are brought up to date
	// before any filter state changes.
	void add_client(ymp706_device &tg);

	// One CHOUT -> VOP3 -> CHIN frame for the 16 notes of one TG.
	// base is the first voice index (0 or 16). Samples are filtered in place.
	void filter_frame(int base, float *notes);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	static constexpr int REGS = 0x80;
	static constexpr int STEPS = 0x400;
	static constexpr int CHANS = 16;
	static constexpr int VOICES = 32;
	static constexpr int TICK = 16;
	static constexpr int PORT_LFO_SYNC = 0x20;
	static constexpr int PORT_EGQ = 0x21;
	static constexpr int PORT_LFO_SPEED = 0x24;
	static constexpr int PORT_LFO_WAVE = 0x25;
	static constexpr int PORT_RATE = 0x2b;
	static constexpr int PORT_TARGET = 0x2c;
	static constexpr int PORT_AIM = 0x2d;

	// Coefficient cells and the filter derived from them, per slot.
	struct chan
	{
		u16 cut;
		u16 reso;
		u16 gain;
		s16 feg_depth;
		s16 lfo_depth;
		u8 type;
		bool bypass;
		float in_gain;
		float g;
		float k;
		float r;
	};

	struct eg
	{
		float level;
		s16 target;
		s16 aim;
		u8 rate;
		s8 side;
	};

	struct lfo
	{
		u32 phase;
		u16 speed;
		u8 wave;
		u8 phase0;
		float sh;
	};

	struct voice
	{
		float z[4];
		float s1;
		float s2;
	};

	devcb_write_line m_irq;
	emu_timer *m_tick_timer;
	bool m_filter_chip;
	bool m_effect_chip;
	sound_stream *m_stream;
	std::unique_ptr<yss236_fx::FxSection> m_fx;
	u8 m_fx_params[112];
	ymp706_device *m_client[2];
	slot_map m_map[CHANS];
	u8 m_eg_slot[CHANS];
	u8 m_slot_eg[CHANS];
	float m_eg_k[256];
	u16 m_reg[REGS];
	u16 m_prog[STEPS][5];   // program words of each step, word k from register 10 - k
	u16 m_coef[STEPS];      // register 0x0B per step
	u8 m_tag[STEPS];        // register 0x0C per step
	u16 m_slot[128][2];     // registers 0x0D/0x0E per delay slot
	u16 m_cfg[4][16];       // registers 0x16, 0x28, 0x29, 0x2A per index
	u16 m_lfo_reg[16][4];   // registers 0x24-0x27 per LFO
	u16 m_select;
	chan m_chan[CHANS];
	eg m_eg[CHANS];
	lfo m_lfo[CHANS];
	voice m_voice[VOICES];
	u16 m_seg_done;
	u16 m_seg_enable;

	u32 sample_rate() const { return clock() / 512; }

	u16 reg_r(offs_t offset);
	void reg_w(offs_t offset, u16 data);
	u16 seg_pending() const { return m_seg_done & m_seg_enable; }
	void update_irq();
	void sync_clients();
	TIMER_CALLBACK_MEMBER(tick);
	void note_coef(u16 addr, u16 data);
	void note_prog(u16 step);
	static void arm_eg(eg &g);
	static float lfo_value(lfo const &l);
	void update_slot(int slot);
	float filter_voice(int voice, float x);
};

DECLARE_DEVICE_TYPE(YSS236, yss236_device)

#endif // MAME_SOUND_YSS236_H
