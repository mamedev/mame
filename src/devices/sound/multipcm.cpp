// license:BSD-3-Clause
// copyright-holders:Miguel Angel Horna
/*
 * Yamaha YMW-258-F 'GEW8' (aka Sega 315-5560) emulation.
 *
 * by Miguel Angel Horna (ElSemi) for Model 2 Emulator and MAME.
 * Information by R. Belmont and the YMF278B (OPL4) manual.
 *
 * voice registers:
 * 0: Pan
 * 1: Index of sample
 * 2: LSB of pitch (low 2 bits seem unused so)
 * 3: MSB of pitch (ooooppppppppppxx) (o=octave (4 bit signed), p=pitch (10 bits), x=unused?
 * 4: voice control: top bit = 1 for key on, 0 for key off
 * 5: bit 0: 0: interpolate volume changes, 1: direct set volume,
 *    bits 1-7 = volume attenuate (0=max, 7f=min)
 * 6: LFO frequency + Phase LFO depth
 * 7: Amplitude LFO size
 *
 * The first sample ROM contains a variable length metadata table with 12
 * bytes per instrument sample. This is very similar to the YMF278B 'OPL4'.
 * This sample format might be derived from the one used by the older YM7138 'GEW6' chip.
 *
 * The first 3 bytes are the offset into the file (big endian). (0, 1, 2).
 * Bit 23 is unknown.
 * Bit 22 is the sample format flag: 0 for 8-bit linear, 1 for 12-bit linear.
 * Bit 21 is used by the MU5 on some samples for as-yet unknown purposes. (YMW-258-F has 22 address pins.)
 * The next 2 bytes are the loop start point, in samples (big endian) (3, 4)
 * The next 2 are the 2's complement negation of of the total number of samples (big endian) (5, 6)
 * The next byte is LFO freq + depth (copied to reg 6 ?) (7, 8)
 * The next 3 are envelope params (Attack, Decay1 and 2, sustain level, release, Key Rate Scaling) (9, 10, 11)
 * The next byte is Amplitude LFO size (copied to reg 7 ?)
 *
 * TODO
 * - http://dtech.lv/techarticles_yamaha_chips.html indicates FM support, which we don't have yet.
 */

#include "emu.h"
#include "multipcm.h"

const int32_t multipcm_device::VALUE_TO_CHANNEL[32] =
{
	0, 1, 2, 3, 4, 5, 6 , -1,
	7, 8, 9, 10,11,12,13, -1,
	14,15,16,17,18,19,20, -1,
	21,22,23,24,25,26,27, -1,
};

void multipcm_device::init_sample(sample_t &sample, uint32_t index)
{
	uint32_t address = index * 12;

	sample.m_start = (read_byte(address) << 16) | (read_byte(address + 1) << 8) | read_byte(address + 2);
	sample.m_format = (sample.m_start>>20) & 0xfe;
	sample.m_start &= 0x3fffff;
	sample.m_loop = (read_byte(address + 3) << 8) | read_byte(address + 4);
	sample.m_end = 0x10000 - ((read_byte(address + 5) << 8) | read_byte(address + 6));
	sample.m_attack_reg = (read_byte(address + 8) >> 4) & 0xf;
	sample.m_decay1_reg = read_byte(address + 8) & 0xf;
	sample.m_decay2_reg = read_byte(address + 9) & 0xf;
	sample.m_decay_level = (read_byte(address + 9) >> 4) & 0xf;
	sample.m_release_reg = read_byte(address + 10) & 0xf;
	sample.m_key_rate_scale = (read_byte(address + 10) >> 4) & 0xf;
	sample.m_lfo_vibrato_reg = read_byte(address + 7);
	sample.m_lfo_amplitude_reg = read_byte(address + 11) & 0xf;
}

void multipcm_device::write_slot(slot_t &slot, int32_t reg, uint8_t data)
{
	m_stream->update();
	slot.m_regs[reg] = data;

	switch(reg)
	{
		case 0: // PANPOT
			slot.m_pan = (data >> 4) & 0xf;
			break;

		case 1: // Sample
		{
			// according to YMF278 sample write causes some base params written to the regs (envelope+lfos)
			init_sample(slot.m_sample, slot.m_regs[1] | ((slot.m_regs[2] & 1) << 8));
			write_slot(slot, 6, slot.m_sample.m_lfo_vibrato_reg);
			write_slot(slot, 7, slot.m_sample.m_lfo_amplitude_reg);

			// retrigger if key is on
			if (slot.m_playing)
				retrigger_sample(slot);

			break;
		}
		case 2: // Pitch
		case 3:
			{
				slot.m_octave = slot.m_regs[3] >> 4;
				slot.m_pitch = ((slot.m_regs[3] & 0xf) << 6) | (slot.m_regs[2] >> 2);
				update_step(slot);
			}
			break;
		case 4: // KeyOn/Off
			if (data & 0x80) // KeyOn
			{
				slot.m_playing = true;
				retrigger_sample(slot);
			}
			else
			{
				if (slot.m_playing)
				{
					if (slot.m_sample.m_release_reg != 0xf)
					{
						slot.m_envelope_gen.m_state = state_t::RELEASE;
					}
					else
					{
						slot.m_playing = false;
					}
				}
			}
			break;
		case 5: // TL + Interpolation
			slot.m_dest_total_level = (data >> 1) & 0x7f;
			if (!(data & 1)) // Interpolate TL
			{
				if ((slot.m_total_level >> TL_SHIFT) > slot.m_dest_total_level)
				{
					slot.m_total_level_step = m_total_level_steps[0]; // decrease
				}
				else
				{
					slot.m_total_level_step = m_total_level_steps[1]; // increase
				}
			}
			else
			{
				slot.m_total_level = slot.m_dest_total_level << TL_SHIFT;
			}
			break;
		case 6: // LFO frequency + Pitch LFO
		case 7: // Amplitude LFO
			slot.m_lfo_frequency = (slot.m_regs[6] >> 3) & 7;
			slot.m_vibrato = slot.m_regs[6] & 7;
			slot.m_tremolo = slot.m_regs[7] & 7;
			if (data)
			{
				lfo_compute_step(slot.m_pitch_lfo, slot.m_lfo_frequency, slot.m_vibrato, 0);
				lfo_compute_step(slot.m_amplitude_lfo, slot.m_lfo_frequency, slot.m_tremolo, 1);
			}
			break;
	}
}

uint8_t multipcm_device::read()
{
	return 0;
}

void multipcm_device::write(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0: // Data write
			write_slot(m_slots[m_cur_slot], m_address, data);
			break;
		case 1:
			m_cur_slot = VALUE_TO_CHANNEL[data & 0x1f];
			break;

		case 2:
			m_address = (data > 7) ? 7 : data;
			break;
	}
}


/* MAME access functions */

DEFINE_DEVICE_TYPE(MULTIPCM, multipcm_device, "ymw258f", "Yamaha YMW-258-F")

multipcm_device::multipcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	gew_pcm_device(mconfig, MULTIPCM, tag, owner, clock, 28, 224),
	m_cur_slot(0),
	m_address(0)
{
}
