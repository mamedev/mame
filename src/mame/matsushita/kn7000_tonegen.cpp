// license:GPL2+
// copyright-holders:Felipe Sanches

// KN7000 tone generator: register decode.

#include "emu.h"
#include "kn7000_tonegen.h"

DEFINE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device, "kn7000_tonegen", "KN7000 Tone Generator")

void kn7000_tonegen_device::tg_write(int tg, uint16_t addr, uint16_t data)
{
	if ((addr & 0xFF00) == 0xFC00) return;            // 0xFC0x idle / status refresh
	const int v = (tg << 6) | ((addr >> 4) & 0x3F);   // voice 0..127 (0..63 sub, 64..127 master)
	const uint16_t cls = addr & 0xFC0F;               // register class (channel masked out)
	if ((cls & 0xFC0E) == 0x2400)                     // pitch (bit0 = pitch18 bit16)
	{
		// A real note-on always programs the 7-halfword amplitude EG first, so require
		// a programmed EG before keying a voice on.
		bool eg_programmed = false;
		for (int k = 0; k < 7; k++) if (m_envreg[v][k] != 0) { eg_programmed = true; break; }
		if (!m_gate[v] && eg_programmed)
		{
			m_gate[v] = 1;        // held
			m_ton[v]  = machine().time().as_double();   // for release detection (reg0 rule)
			m_mode[v] = (m_aux[v] & 0x8000) ? 1 : 0;
			const double nowt = machine().time().as_double();
			m_srckey[v] = (m_ctx_time >= 0.0 && (nowt - m_ctx_time) < 0.060) ? m_ctx_key : 0xFF;
		}
		m_tgwrites++;
	}
	else if (cls == 0x0000)                           // r0 = [ATK rate | PEAK level]
	{
		m_eg012[v][0] = data;
		// KEY-RELEASE, path 2 (firmware-managed sounds): after the r3=0x8000 gate-off
		// the firmware rewrites regs 0,1,4,5,8,9 of the note's odd companion block.
		if (data != 0 && (data >> 8) < 0xFF)
		{
			const double now = machine().time().as_double();
			for (int y = (v & ~1); y <= (v | 1); y++)
				if (m_gate[y] && (now - m_ton[y]) > 0.020)
					m_gate[y] = 0;
		}
	}
	else if (cls == 0x0001)                           // r1 = [DCY1 rate | SUS1 level];
	{                                                 // 0xC000 = mute (boot init / voice-steal)
		m_eg012[v][1] = data;
		if (data == 0xC000) m_gate[v] = 0;
	}
	else if (cls == 0x0002)                           // r2 = [DCY2 rate | SUS2 level]
	{
		m_eg012[v][2] = data;
	}
	else if (cls == 0x0003)                           // r3 = GATE (sweep result 3):
	{                                                 // 0x87FF at note-on, 0x8000 at key-up
		// Universal key-release trigger, written for every voice class on key-up.
		if ((data >> 8) == 0x80)
		{
			const double now = machine().time().as_double();
			if (m_gate[v] && (now - m_ton[v]) > 0.020)
				m_gate[v] = 0;
		}
	}
	else if (cls == 0x2009)                           // per-voice level (best-effort)
	{
		// The firmware writes this once at note-on.
		m_level[v] = std::clamp(double(data) / double(0x5FFF), 0.0, 1.4);
	}
	else if ((addr & 0xFC00) == 0x8000)               // group 0x20: per-channel OUTPUT BUS /
	{                                                  // EFFECT-SEND record (0x80xx-0x83xx)
		// Decoded from the library's setter family and its live argument traffic:
		const int ch = (addr >> 4) & 0x3F, reg = addr & 0x0F;
		m_busreg[(tg << 6) | ch][reg] = data;
		if (tg == 1 && ch == 0x03 && reg == 0x0A)
		{
			m_gain_direct = float((data >> 8) & 0x7F) / 127.0f;
			m_gain_return = float(data & 0x7F) / 127.0f;
		}
		if (tg == 1 && ch == 0x0B && reg == 0x08)
			m_gain_send = float(data & 0x7F) / 127.0f;
		if (tg == 1 && ch == 0x33 && reg == 0x08)
			m_gain_depth = float(data & 0x7F) / 127.0f;   // TOTAL DEPTH (0x8500|depth)
		if (tg == 1 && ch == 0x19 && reg == 0x08)
			m_gain_chorus = float(data & 0x7F) / 127.0f;  // CHORUS send (0x8198 low7 = per-part depth;
														  // 0x0B00 off -> 0; routes to CHORUS unit 9)
		if (tg == 1 && ch == 0x09 && reg == 0x08)
			m_gain_dsp = float(data & 0x7F) / 127.0f;     // SOUND DSP send (0x8098 low7 = per-part depth;
														  // routes to the per-part insert pool u2..u6,
														  // RIGHT1 being unit 2)
		if (tg == 1 && ch == 0x29 && reg == 0x08)
		{
			const uint8_t lvl = data & 0x7F;
			const bool on = ((data >> 8) & 0x0F) == 0x06;
			m_gain_multi = float(lvl ? lvl : (on ? 0x50 : 0)) / 127.0f;
		}
		// PER-EFFECT RETURN levels (reg 0xA low byte = DSP-return level for THIS effect's own
		// bus), from the per-effect toggle map:
		if (tg == 1 && ch == 0x09 && reg == 0x0A)
			m_gain_dsp_ret = float(data & 0x7F) / 127.0f;   // SOUND DSP return (0x809A low7)
		if (tg == 1 && ch == 0x06 && reg == 0x0A)
			m_gain_multi_ret = float(data & 0x7F) / 127.0f; // MULTI return (0x806A low7)
	}
	else if (cls == 0x1C02)                           // per-voice aux/mode word
	{
		// bit15 marks the gate-follow voice classes (see key_context/key_break above).
		m_aux[v] = data;
	}
	else if (cls >= 0x0004 && cls <= 0x000A)          // amplitude-envelope params r4..rA
	{
		m_envreg[v][cls - 0x0004] = data;
	}
}
