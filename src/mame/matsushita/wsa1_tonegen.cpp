// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

    Technics SX-WSA1R -- IC4 tone generator PLACEHOLDER SINE backend.

    A 64-voice bank of sine oscillators, each fed a frequency by the firmware's
    PITCH register and gated by its own block-0 lifecycle latch, summed into a
    stereo stream.  No wave ROM, no synthesis, no read of another device's
    memory -- only the values the firmware writes to 0x0010C000.  What this
    stands in for is described in wsa1_tonegen.h.

***************************************************************************/

#include "emu.h"
#include "wsa1_tonegen.h"

#include <cmath>
#include <numbers>

DEFINE_DEVICE_TYPE(WSA1_TONEGEN, wsa1_tonegen_device, "wsa1_tonegen", "Technics SX-WSA1R Tone Generator (placeholder sine)")

namespace {
// Per-voice peak with headroom for many simultaneous voices; a voice at unity
// output level (register field 0x0FF4) reaches this.
constexpr float VOICE_PEAK = 0.18f;
} // anonymous namespace


wsa1_tonegen_device::wsa1_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, WSA1_TONEGEN, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}


void wsa1_tonegen_device::device_start()
{
	for (unsigned i = 0; i < SINE_SIZE; i++)
		m_sine[i] = float(std::sin(2.0 * std::numbers::pi * double(i) / double(SINE_SIZE)));

	// 0 inputs, 2 outputs (stereo), at the pair's real audio rate.
	m_stream = stream_alloc(0, 2, STREAM_RATE);

	for (voice &v : m_voice)
		v = voice{};

	for (int i = 0; i < NUM_VOICES; i++)
	{
		save_item(NAME(m_voice[i].phase), i);
		save_item(NAME(m_voice[i].inc), i);
		save_item(NAME(m_voice[i].pitch), i);
		save_item(NAME(m_voice[i].level), i);
		save_item(NAME(m_voice[i].env0800), i);
		save_item(NAME(m_voice[i].env0840), i);
		save_item(NAME(m_voice[i].gate), i);
		save_item(NAME(m_voice[i].released), i);
		save_item(NAME(m_voice[i].env), i);
	}
}


//-------------------------------------------------
//  inc_for - PITCH register (1/256 semitone) -> phase increment per sample
//-------------------------------------------------

uint32_t wsa1_tonegen_device::inc_for(uint16_t reg0400) const
{
	// chan + 0x0400 is the pitch in units of 1/256 semitone, seeded
	// note * 256 + 0x80 and saturated to 0..0x7FFF, so value / 256 is a MIDI
	// note number.  Whether the +0x80 seed is an offset or a rounding bias is
	// not established; this takes the plain reading, which is well inside the
	// tuning a placeholder sine wants.
	if (reg0400 == 0)
		return 0;                       // no pitch programmed yet -> no oscillator

	double const note = double(reg0400) / 256.0;
	double freq = 440.0 * std::pow(2.0, (note - 69.0) / 12.0);
	if (freq <= 0.0)
		return 0;
	if (freq > double(STREAM_RATE) / 2.0)   // never alias past Nyquist
		freq = double(STREAM_RATE) / 2.0;

	return uint32_t((freq / double(STREAM_RATE)) * 4294967296.0);
}


void wsa1_tonegen_device::set_pitch(unsigned ch, uint16_t reg0400)
{
	if (ch >= NUM_VOICES)
		return;
	m_stream->update();                 // apply at the correct point in the stream
	m_voice[ch].pitch = reg0400;
	m_voice[ch].inc   = inc_for(reg0400);
}


void wsa1_tonegen_device::set_gate(unsigned ch, bool on)
{
	if (ch >= NUM_VOICES)
		return;
	m_stream->update();
	if (on && !m_voice[ch].gate)
	{
		m_voice[ch].phase = 0;          // start each note at zero phase, click-free with the env ramp
		m_voice[ch].released = false;   // a fresh gate cancels any pending note-off decay
	}
	m_voice[ch].gate = on;
	if (!on)
		m_voice[ch].released = false;   // the voice is FREE now; clear release state for reuse
}


//-------------------------------------------------
//  set_release - the firmware's note-off: decay this voice while it stays gated
//-------------------------------------------------

void wsa1_tonegen_device::set_release(unsigned ch)
{
	if (ch >= NUM_VOICES)
		return;
	m_stream->update();
	m_voice[ch].released = true;        // amp_of() now targets 0; env ramps down at the release rate
}


//-------------------------------------------------
//  amplitude - the voice's current modelled amplitude (0 == silent)
//-------------------------------------------------

float wsa1_tonegen_device::amplitude(unsigned ch)
{
	if (ch >= NUM_VOICES)
		return 0.0f;
	m_stream->update();          // advance env to now, so a poll sees the real decay
	return m_voice[ch].env;
}


void wsa1_tonegen_device::set_level(unsigned ch, uint16_t reg0080)
{
	if (ch >= NUM_VOICES) return;
	m_stream->update();
	m_voice[ch].level = reg0080;
}

void wsa1_tonegen_device::set_env0(unsigned ch, uint16_t reg0800)
{
	if (ch >= NUM_VOICES) return;
	m_stream->update();
	m_voice[ch].env0800 = reg0800;
}

void wsa1_tonegen_device::set_env1(unsigned ch, uint16_t reg0840)
{
	if (ch >= NUM_VOICES) return;
	m_stream->update();
	m_voice[ch].env0840 = reg0840;
}


//-------------------------------------------------
//  amp_of - the voice's target amplitude from its gate, idle marker and level
//-------------------------------------------------

float wsa1_tonegen_device::amp_of(const voice &v) const
{
	// A channel whose two envelope-marker registers hold the reset/voice-clear
	// quiescent pair (0xFF80 / 0xFF00) is idle and makes no sound; this is what
	// silences the boot-init and freed voices instead of a fixed level would.
	if (v.env0800 == 0xff80 && v.env0840 == 0xff00)
		return 0.0f;
	if (!v.gate)
		return 0.0f;
	// Note-off: the firmware re-staged this channel's envelope with a release
	// profile but left the gate up (the chip decays and the poll frees it).  Model
	// that as a decay to silence; the driver watches amplitude() to drop the busy
	// bit once the decay finishes, so the firmware's own retire path can run.
	if (v.released)
		return 0.0f;

	// OUTPUT LEVEL: field = reg0080 & 0x0FFF, a base-2 logarithm with 256 counts
	// per octave, 0x0FF4 = unity and larger = louder.  The 3-bit field
	// (bits 14..12) and the strobe (bit 15) are not amplitude and are ignored.
	unsigned const field = v.level & 0x0fff;
	float const gain = float(std::pow(2.0, (double(field) - double(0x0ff4)) / 256.0));
	return VOICE_PEAK * gain;
}


//-------------------------------------------------
//  sound_stream_update - sum the gated voices' sines
//-------------------------------------------------

void wsa1_tonegen_device::sound_stream_update(sound_stream &stream)
{
	// Gate/level-edge ramps (~4 ms attack, ~40 ms release), against full scale,
	// so an amplitude step (note-on, note-off, or a level change) never clicks.
	constexpr float ATTACK  = VOICE_PEAK / (0.004f * float(STREAM_RATE));
	constexpr float RELEASE = VOICE_PEAK / (0.040f * float(STREAM_RATE));

	for (int s = 0; s < stream.samples(); s++)
	{
		float mix = 0.0f;

		for (voice &v : m_voice)
		{
			// Target amplitude from the real level + idle marker; the enable
			// toggle forces silence.
			float const target = amp_of(v);

			if (v.env < target)
				v.env = std::min(target, v.env + ATTACK);
			else if (v.env > target)
				v.env = std::max(target, v.env - RELEASE);

			if (v.env <= 0.0f || v.inc == 0)
				continue;

			v.phase += v.inc;
			mix += m_sine[v.phase >> (32 - SINE_BITS)] * v.env;
		}

		// A placeholder can legitimately hard-clip on the rare fully-stacked
		// chord; it keeps the mix bounded without a compressor the real part
		// does not have.
		mix = std::clamp(mix, -1.0f, 1.0f);

		stream.put(0, s, sound_stream::sample_t(mix));
		stream.put(1, s, sound_stream::sample_t(mix));
	}
}
