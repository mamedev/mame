// license:GPL2+
// copyright-holders:Felipe Sanches

// KN6000/KN6500 tone generator: register decode.

#include "emu.h"
#include "kn6000_tonegen.h"

DEFINE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device, "kn6000_tonegen", "KN6000 Tone Generator")

void kn6000_tonegen_device::tg_write(int tg, uint16_t addr, uint16_t data)
{
	(void)tg;                                          // one chip, one window -- always 1
	if ((addr & 0xFF00) == 0xFC00) return;             // 0xFC0x idle / status refresh
	const int v = (addr >> 4) & 0x3F;                  // voice slot 0..63
	const uint16_t cls = addr & 0xFC0F;                // register class (slot masked out)

	if ((cls & 0xFC00) == 0x5000)                      // PITCH (plane 0x14): the note-on trigger
	{
		// This register is written near the end of the note-on blit, after the envelope,
		// so a programmed EG is the signal that the voice is really starting.
		bool eg_programmed = false;
		for (int k = 0; k < 3; k++) if (m_eg012[v][k] != 0) { eg_programmed = true; break; }

		if (!m_gate[v] && eg_programmed)
		{
			m_gate[v] = 1;
			m_ton[v]  = machine().time().as_double();

			// Voice life cycle. The KN6000's aux/mode word (the analogue of the
			// KN7000's class 0x1C02 gate-follow marker) is not yet identified.
			m_mode[v]   = 0;
			m_srckey[v] = 0xFF;
		}
		m_tgwrites++;
	}
	else if (cls == 0x0000)                            // idx 0 = GATE
	{
		if ((data >> 8) == 0x80)
		{
			const double now = machine().time().as_double();
			if (m_gate[v] && (now - m_ton[v]) > 0.020)
				m_gate[v] = 0;
		}
	}
	else if (cls == 0x0004)                            // amp EG [ATK rate | PEAK level]
	{
		m_eg012[v][0] = data;
		// KEY-RELEASE path 2 (firmware-managed sounds): after the gate-off the firmware
		// rewrites the first two registers of each envelope bank ({4,5,8,9,C,D}).
		if (data != 0 && (data >> 8) < 0xFF)
		{
			const double now = machine().time().as_double();
			if (m_gate[v] && (now - m_ton[v]) > 0.020)
				m_gate[v] = 0;
		}
	}
	else if (cls == 0x0005)                            // amp EG [DCY1 rate | SUS1 level]
	{
		m_eg012[v][1] = data;
		if (data == 0xC000) m_gate[v] = 0;
	}
	else if (cls == 0x0006)                            // amp EG [DCY2 rate | SUS2 level]
	{
		m_eg012[v][2] = data;
		if (data == 0xC000) m_gate[v] = 0;
	}
	else if (cls >= 0x0008 && cls <= 0x000E)           // pitch/filter envelope banks
	{
		// The positional analogue of the KN7000's r4..rA: further three-register banks
		// holding the pitch and filter envelopes.
		m_envreg[v][cls - 0x0008] = data;
	}
	else if (cls == 0x4000)                            // per-voice LEVEL (plane 0x10)
	{
		// Written once per note-on. Full velocity gives the nominal maximum.
		m_level[v] = std::clamp(double(data) / double(0x3FFF), 0.0, 1.4);
	}
	// The per-channel effect-send matrix, the KN7000's group 0x20, is deliberately not
	// decoded here: the KN6000's use of that range has not been mapped.
}
