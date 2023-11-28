// license:BSD-3-Clause
// copyright-holders:Devin Acker

#include "emu.h"
#include "gew7.h"

DEFINE_DEVICE_TYPE(GEW7_PCM, gew7_pcm_device, "gew7_pcm", "Yamaha GEW7 PCM")

gew7_pcm_device::gew7_pcm_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock) :
	gew_pcm_device(mconfig, GEW7_PCM, tag, owner, clock, 12, 72)
{
}

void gew7_pcm_device::device_start()
{
	gew_pcm_device::device_start();

	// fudge the default envelope step values to make them more like how they sound on this chip
	// TODO: get some measurements from real hardware and try to make this more accurate
	static const unsigned steps[] = { 0x99, 0xaa, 0xcc, 0x100 };

	for (int32_t i = 4; i < 0x40; ++i)
	{
		m_attack_step[i] = steps[i % 4] << (i / 4);
		m_decay_release_step[i] = m_attack_step[i] >> 5;
	}
	m_attack_step[0x3f] = 0x400 << EG_SHIFT;

	// stereo channels are reversed compared to GEW8
	std::swap(m_left_pan_table, m_right_pan_table);
}

void gew7_pcm_device::init_sample(sample_t& sample, uint32_t index)
{
	uint32_t address = 0x4000 | index * 12;

	// format & address parts are mostly the same as GEW8
	sample.m_start = (read_byte(address) << 16) | (read_byte(address + 1) << 8) | read_byte(address + 2);
	sample.m_format = (sample.m_start >> 20) & 0xfe;
	sample.m_start &= 0x1fffff;
	sample.m_loop = (read_byte(address + 3) << 8) | read_byte(address + 4);
	sample.m_end = 0x4000 - ((read_byte(address + 5) << 8) | read_byte(address + 6));

	// TODO: bytes 7 & 8 control LFO, reverse playback, etc (verify the details)
	// the LFO bits are laid out a little differently than they are for GEW8
	sample.m_lfo_vibrato_reg = read_byte(address + 7);
	sample.m_lfo_amplitude_reg = read_byte(address + 8);

	// ADSR bits are arranged differently compared to GEW8
	sample.m_attack_reg = read_byte(address + 9) >> 4;
	sample.m_decay1_reg = read_byte(address + 9) & 0xf;
	sample.m_decay2_reg = read_byte(address + 10) >> 4;
	sample.m_release_reg = read_byte(address + 10) & 0xf;
	sample.m_key_rate_scale = read_byte(address + 11) >> 4;
	sample.m_decay_level = read_byte(address + 11) & 0xf;
}

uint8_t gew7_pcm_device::read(offs_t offset)
{
	if (offset >= 0x60) return 0;

	if (!machine().side_effects_disabled())
		m_stream->update();

	slot_t& slot = m_slots[offset >> 3];
	const uint8_t reg = offset & 7;

	switch (reg)
	{
	case 3:
		// at least some GEW7-based keyboards seem to expect the key-on flag to go off by itself
		// for certain "one-shot" sounds (i.e. the beeps when changing volume/tempo)
		if (!slot.m_playing)
			return slot.m_regs[reg] & 0x7f;
		break;

	case 6:
		// uppermost bits of current envelope level (inverted)
		if (!slot.m_playing)
			return 0xff;

		return ~slot.m_envelope_gen.m_volume >> (EG_SHIFT + 2);

	case 7:
		// next 2 lower bits of envelope level (inverted) + 2 bits of envelope state
		if (!slot.m_playing)
			return 0xf0;

		return ((~slot.m_envelope_gen.m_volume >> EG_SHIFT) << 6)
			| ((uint8_t)slot.m_envelope_gen.m_state << 4);
	}

	return slot.m_regs[reg];
}

void gew7_pcm_device::write(offs_t offset, uint8_t data)
{
	if (offset >= 0x60) return;

	m_stream->update();

	const uint8_t voice = offset >> 3;
	const uint8_t reg = offset & 7;
	slot_t& slot = m_slots[voice];
	slot.m_regs[reg] = data;

	switch (reg)
	{
	case 0: // sample
		init_sample(slot.m_sample, slot.m_regs[0] | ((slot.m_regs[1] & 1) << 8));

		slot.m_lfo_frequency = slot.m_sample.m_lfo_amplitude_reg & 7;
		write_hi((voice << 2) | 3, slot.m_sample.m_lfo_vibrato_reg);

		// retrigger if key is on
		if (slot.m_playing)
			retrigger_sample(slot);
		break;

	case 1: // pitch
	case 2:
		slot.m_octave = slot.m_regs[2] >> 4;
		slot.m_pitch = ((slot.m_regs[2] & 0xf) << 6) | (slot.m_regs[1] >> 2);
		update_step(slot);
		// adjust pitch step for oversampling
		slot.m_step >>= 1;
		break;

	case 3: // key on + reverb + panpot
		if (data & 0x80)
		{
			slot.m_playing = true;
			retrigger_sample(slot);
		}
		else if (slot.m_playing)
		{
			slot.m_envelope_gen.m_state = state_t::RELEASE;
		}

		slot.m_envelope_gen.m_reverb = BIT(data, 4);
		slot.m_pan = data & 0xf;
		break;

	case 4: // TL
		slot.m_dest_total_level = data & 0x7f;
		slot.m_total_level = slot.m_dest_total_level << TL_SHIFT;
		break;
	}
}

uint8_t gew7_pcm_device::read_hi(offs_t offset)
{
	if (offset >= 0x30) return 0;

	if (!machine().side_effects_disabled())
		m_stream->update();

	slot_t& slot = m_slots[offset >> 2];
	switch (offset & 3)
	{
	case 0: // TODO: unverified, just a guess based on reg 1
		return (slot.m_sample.m_attack_reg << 4) | slot.m_sample.m_decay1_reg;

	case 1: // TODO: unverified, but gets ORed with 0x0f when killing a voice before waiting for envelope to end
		return (slot.m_sample.m_decay2_reg << 4) | slot.m_sample.m_release_reg;

	case 3:
		return (slot.m_tremolo << 3) | slot.m_vibrato;
	}

	return 0;
}

void gew7_pcm_device::write_hi(offs_t offset, uint8_t data)
{
	if (offset >= 0x30) return;

	m_stream->update();

	slot_t& slot = m_slots[offset >> 2];
	switch (offset & 3)
	{
	case 0:
		slot.m_sample.m_attack_reg = data >> 4;
		slot.m_sample.m_decay1_reg = data & 0xf;
		envelope_generator_calc(slot);
		break;

	case 1:
		slot.m_sample.m_decay2_reg = data >> 4;
		slot.m_sample.m_release_reg = data & 0xf;
		envelope_generator_calc(slot);
		break;

	case 2: // TODO: ??? (probably related to unknown bits of sample table)
		break;

	case 3:
		slot.m_vibrato = data & 7;
		slot.m_tremolo = (data >> 3) & 7;
		slot.m_reverse = data >> 7;
		lfo_compute_step(slot.m_pitch_lfo, slot.m_lfo_frequency, slot.m_vibrato, 0);
		lfo_compute_step(slot.m_amplitude_lfo, slot.m_lfo_frequency, slot.m_tremolo, 1);
		break;
	}
}
