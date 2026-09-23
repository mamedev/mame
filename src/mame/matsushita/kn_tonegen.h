// license:GPL2+
// copyright-holders:Felipe Sanches

// Shared register file for the KN6000/KN7000 tone generators.

#ifndef MAME_MATSUSHITA_KN_TONEGEN_H
#define MAME_MATSUSHITA_KN_TONEGEN_H

#pragma once

class kn_tonegen_base_device : public device_t, public device_sound_interface
{
public:

	// Voices this model's tone generator(s) provide (KN7000 = 128 across two
	// chips, KN6000/KN6500 = 64 in one). Sizes every per-voice loop below.
	int num_voices() const { return m_num_voices; }

	virtual void tg_write(int tg, uint16_t addr, uint16_t data) = 0;

	uint32_t tg_write_count() const { return m_tgwrites; }
	// Output-bus routing (reverb toggle), decoded from the group-0x20 registers.
	float gain_direct() const { return m_gain_direct; }
	float gain_return() const { return m_gain_return; }
	float gain_send()   const { return m_gain_send; }
	float gain_depth()  const { return m_gain_depth; }
	float gain_chorus() const { return m_gain_chorus; }
	float gain_dsp()    const { return m_gain_dsp; }
	float gain_multi()  const { return m_gain_multi; }
	float gain_dsp_ret()   const { return m_gain_dsp_ret; }    // SOUND DSP own return (ch09.rA)
	float gain_multi_ret() const { return m_gain_multi_ret; }  // MULTI own return (ch06.rA)

	// Keybed coupling for GATE-FOLLOW voices. The SUB TG chip itself hosts the key-bed
	// event FIFO, which the firmware reads at the TG's own +4 register.
	void key_context(uint8_t key) { m_ctx_key = key; m_ctx_time = machine().time().as_double(); }
	void key_break(uint8_t key);

protected:
	kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int num_voices);

	// device_t / device_sound_interface overrides
	virtual void device_start() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	sound_stream *m_stream = nullptr;
	double   m_ton[128]   = { };     // note-on machine time (s) -- release detection
	uint16_t m_aux[128]   = { };     // per-voice aux/mode word (latch class 0x1C02; bit15 = gate-follow)
	uint8_t  m_mode[128]  = { };     // 0=MANAGED (firmware key-up burst) 1=GATE_FOLLOW 2=ONESHOT
	uint16_t m_busreg[128][16] = { };// group-0x20 output-bus/effect-send register file
	float    m_gain_direct = 0.0f;     // DAC crossfade: TG direct (reverb OFF side)
	float    m_gain_return = 1.0f;     // DAC crossfade: DSP return (reverb ON side)
	float    m_gain_send = 0.80f;      // TG -> DSP send level (boot default 0x66/0x7F)
	float    m_gain_depth = float(0x50) / 127.0f;  // REVERB TOTAL DEPTH (0x8338 low7)
	float    m_gain_chorus = 0.0f;     // CHORUS send (0x8198 low7); 0 = chorus off
	float    m_gain_dsp = 0.0f;        // SOUND DSP send (0x8098 low7); 0 = off
	float    m_gain_multi = 0.0f;      // MULTI send (0x8298 low7); 0 = off
	float    m_gain_dsp_ret = 0.0f;    // SOUND DSP own return (0x809A low7); 0 = off
	float    m_gain_multi_ret = 0.0f;  // MULTI own return (0x806A low7); 0 = off
	uint8_t  m_srckey[128];          // keybed key index that caused this voice (0xFF = none)
	uint8_t  m_ctx_key = 0xFF;       // most recent keybed MAKE (key index)
	double   m_ctx_time = -1.0;      // ...and when it was pushed
	uint8_t  m_gate[128]  = { };     // per-voice gate: 1 = firmware note held, 0 = muted/released
	double   m_level[128] = { };     // per-voice level (firmware class 0x2009; 1.0 = default full)
	uint16_t m_eg012[128][3] = { };   // raw r0/r1/r2 per voice (the 7-param amplitude EG)
	uint16_t m_envreg[128][7] = { };  // raw r4..rA per voice (damp/aux bank)
	uint32_t m_tgwrites = 0;         // count of firmware pitch writes seen (0 = engine dormant)
	int      m_num_voices;           // 128 (KN7000) or 64 (KN6000/KN6500)
};

#endif // MAME_MATSUSHITA_KN_TONEGEN_H
