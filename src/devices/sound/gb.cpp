// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Anthony Kruize
// thanks-to:Shay Green
/**************************************************************************************
* Game Boy sound emulation (c) Anthony Kruize (trandor@labyrinth.net.au)
*
* Anyways, sound on the Game Boy consists of 4 separate 'channels'
*   Sound1 = Quadrangular waves with SWEEP and ENVELOPE functions  (NR10,11,12,13,14)
*   Sound2 = Quadrangular waves with ENVELOPE functions (NR21,22,23,24)
*   Sound3 = Wave patterns from WaveRAM (NR30,31,32,33,34)
*   Sound4 = White noise with an envelope (NR41,42,43,44)
*
* Each sound channel has 2 modes, namely ON and OFF...  whoa
*
* These tend to be the two most important equations in
* converting between Hertz and GB frequency registers:
* (Sounds will have a 2.4% higher frequency on Super GB.)
*       gb = 2048 - (131072 / Hz)
*       Hz = 131072 / (2048 - gb)
*
* Changes:
*
*   10/2/2002       AK - Preliminary sound code.
*   13/2/2002       AK - Added a hack for mode 4, other fixes.
*   23/2/2002       AK - Use lookup tables, added sweep to mode 1. Re-wrote the square
*                        wave generation.
*   13/3/2002       AK - Added mode 3, better lookup tables, other adjustments.
*   15/3/2002       AK - Mode 4 can now change frequencies.
*   31/3/2002       AK - Accidently forgot to handle counter/consecutive for mode 1.
*    3/4/2002       AK - Mode 1 sweep can still occur if shift is 0.  Don't let frequency
*                        go past the maximum allowed value. Fixed Mode 3 length table.
*                        Slight adjustment to Mode 4's period table generation.
*    5/4/2002       AK - Mode 4 is done correctly, using a polynomial counter instead
*                        of being a total hack.
*    6/4/2002       AK - Slight tweak to mode 3's frequency calculation.
*   13/4/2002       AK - Reset envelope value when sound is initialized.
*   21/4/2002       AK - Backed out the mode 3 frequency calculation change.
*                        Merged init functions into gameboy_sound_w().
*   14/5/2002       AK - Removed magic numbers in the fixed point math.
*   12/6/2002       AK - Merged SOUNDx structs into one SOUND struct.
*  26/10/2002       AK - Finally fixed channel 3!
* xx/4-5/2016       WP - Rewrote sound core. Most of the code is not optimized yet.

TODO:
- Implement different behavior of CGB-02.
- Implement different behavior of CGB-05.
- Implement different behavior of AGB-*.
- Perform more tests on real hardware to figure out when the frequency counters are
  reloaded.
- Perform more tests on real hardware to understand when changes to the noise divisor
  and shift kick in.
- Optimize the channel update methods.

***************************************************************************************/

#include "emu.h"
#include "gb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/


/* Represents wave duties of 12.5%, 25%, 50% and 75% */
const int gameboy_sound_device::wave_duty_table[4][8] =
{
	{ -1, -1, -1, -1, -1, -1, -1,  1},
	{  1, -1, -1, -1, -1, -1, -1,  1},
	{  1, -1, -1, -1, -1,  1,  1,  1},
	{ -1,  1,  1,  1,  1,  1,  1, -1}
};

// device type definitions
DEFINE_DEVICE_TYPE(DMG_APU, dmg_apu_device, "dmg_apu", "LR35902 APU")
//DEFINE_DEVICE_TYPE(CGB02_APU, cgb02_apu_device, "cgb02_apu", fullname)
DEFINE_DEVICE_TYPE(CGB04_APU, cgb04_apu_device, "cgb04_apu", "CGB04 APU")
//DEFINE_DEVICE_TYPE(CGB05_APU, cgb05_apu_device, "cgb05_apu", fullname)
DEFINE_DEVICE_TYPE(AGB_APU, agb_apu_device, "agb_apu", "AGB APU")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gameboy_sound_device - constructor
//-------------------------------------------------

gameboy_sound_device::gameboy_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
}


dmg_apu_device::dmg_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: gameboy_sound_device(mconfig, DMG_APU, tag, owner, clock)
{
}

cgb04_apu_device::cgb04_apu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: gameboy_sound_device(mconfig, type, tag, owner, clock)
{
}

cgb04_apu_device::cgb04_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cgb04_apu_device(mconfig, CGB04_APU, tag, owner, clock)
{
}

agb_apu_device::agb_apu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cgb04_apu_device(mconfig, AGB_APU, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

#define SAVE_CHANNEL(snd) \
	save_item(NAME(snd.reg)); \
	save_item(NAME(snd.on)); \
	save_item(NAME(snd.channel)); \
	save_item(NAME(snd.length)); \
	save_item(NAME(snd.length_mask)); \
	save_item(NAME(snd.length_counting)); \
	save_item(NAME(snd.length_enabled)); \
	save_item(NAME(snd.cycles_left)); \
	save_item(NAME(snd.duty)); \
	save_item(NAME(snd.envelope_enabled)); \
	save_item(NAME(snd.envelope_value)); \
	save_item(NAME(snd.envelope_direction)); \
	save_item(NAME(snd.envelope_time)); \
	save_item(NAME(snd.envelope_count)); \
	save_item(NAME(snd.signal)); \
	save_item(NAME(snd.frequency)); \
	save_item(NAME(snd.frequency_counter)); \
	save_item(NAME(snd.sweep_enabled)); \
	save_item(NAME(snd.sweep_neg_mode_used)); \
	save_item(NAME(snd.sweep_shift)); \
	save_item(NAME(snd.sweep_direction)); \
	save_item(NAME(snd.sweep_time)); \
	save_item(NAME(snd.sweep_count)); \
	save_item(NAME(snd.size)); \
	save_item(NAME(snd.bank)); \
	save_item(NAME(snd.level)); \
	save_item(NAME(snd.offset)); \
	save_item(NAME(snd.duty_count)); \
	save_item(NAME(snd.current_sample)); \
	save_item(NAME(snd.sample_reading)); \
	save_item(NAME(snd.noise_short)); \
	save_item(NAME(snd.noise_lfsr));


void gameboy_sound_device::device_start()
{
	m_channel = stream_alloc(0, 2, SAMPLE_RATE_OUTPUT_ADAPTIVE);
	m_timer = timer_alloc(FUNC(gameboy_sound_device::timer_callback), this);
	m_timer->adjust(clocks_to_attotime(FRAME_CYCLES/128), 0, clocks_to_attotime(FRAME_CYCLES/128));

	save_item(NAME(m_last_updated));
	save_item(NAME(m_snd_regs));
	save_item(NAME(m_wave_ram));
	// sound control
	save_item(NAME(m_snd_control.on));
	save_item(NAME(m_snd_control.vol_left));
	save_item(NAME(m_snd_control.vol_right));
	save_item(NAME(m_snd_control.mode1_left));
	save_item(NAME(m_snd_control.mode1_right));
	save_item(NAME(m_snd_control.mode2_left));
	save_item(NAME(m_snd_control.mode2_right));
	save_item(NAME(m_snd_control.mode3_left));
	save_item(NAME(m_snd_control.mode3_right));
	save_item(NAME(m_snd_control.mode4_left));
	save_item(NAME(m_snd_control.mode4_right));
	save_item(NAME(m_snd_control.cycles));

	SAVE_CHANNEL(m_snd_1);
	SAVE_CHANNEL(m_snd_2);
	SAVE_CHANNEL(m_snd_3);
	SAVE_CHANNEL(m_snd_4);
}


//-------------------------------------------------
//  device_clock_changed
//-------------------------------------------------

void gameboy_sound_device::device_clock_changed()
{
	m_timer->adjust(clocks_to_attotime(FRAME_CYCLES / 128), 0, clocks_to_attotime(FRAME_CYCLES / 128));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void gameboy_sound_device::device_reset()
{
	memset(&m_snd_1, 0, sizeof(m_snd_1));
	memset(&m_snd_2, 0, sizeof(m_snd_2));
	memset(&m_snd_3, 0, sizeof(m_snd_3));
	memset(&m_snd_4, 0, sizeof(m_snd_4));

	m_snd_1.channel = 1;
	m_snd_1.length_mask = 0x3f;
	m_snd_2.channel = 2;
	m_snd_2.length_mask = 0x3f;
	m_snd_3.channel = 3;
	m_snd_3.length_mask = 0xff;
	m_snd_4.channel = 4;
	m_snd_4.length_mask = 0x3f;

	sound_w_internal(NR52, 0x00);
	m_wave_ram[0][0x0] = 0xac;
	m_wave_ram[0][0x1] = 0xdd;
	m_wave_ram[0][0x2] = 0xda;
	m_wave_ram[0][0x3] = 0x48;
	m_wave_ram[0][0x4] = 0x36;
	m_wave_ram[0][0x5] = 0x02;
	m_wave_ram[0][0x6] = 0xcf;
	m_wave_ram[0][0x7] = 0x16;
	m_wave_ram[0][0x8] = 0x2c;
	m_wave_ram[0][0x9] = 0x04;
	m_wave_ram[0][0xa] = 0xe5;
	m_wave_ram[0][0xb] = 0x2c;
	m_wave_ram[0][0xc] = 0xac;
	m_wave_ram[0][0xd] = 0xdd;
	m_wave_ram[0][0xe] = 0xda;
	m_wave_ram[0][0xf] = 0x48;
}


void cgb04_apu_device::device_reset()
{
	gameboy_sound_device::device_reset();

	m_wave_ram[0][0x0] = 0x00;
	m_wave_ram[0][0x1] = 0xff;
	m_wave_ram[0][0x2] = 0x00;
	m_wave_ram[0][0x3] = 0xff;
	m_wave_ram[0][0x4] = 0x00;
	m_wave_ram[0][0x5] = 0xff;
	m_wave_ram[0][0x6] = 0x00;
	m_wave_ram[0][0x7] = 0xff;
	m_wave_ram[0][0x8] = 0x00;
	m_wave_ram[0][0x9] = 0xff;
	m_wave_ram[0][0xa] = 0x00;
	m_wave_ram[0][0xb] = 0xff;
	m_wave_ram[0][0xc] = 0x00;
	m_wave_ram[0][0xd] = 0xff;
	m_wave_ram[0][0xe] = 0x00;
	m_wave_ram[0][0xf] = 0xff;
}

void agb_apu_device::device_reset()
{
	gameboy_sound_device::device_reset();

	// TODO: needs verification
	m_wave_ram[0][0x0] = 0x00;
	m_wave_ram[0][0x1] = 0xff;
	m_wave_ram[0][0x2] = 0x00;
	m_wave_ram[0][0x3] = 0xff;
	m_wave_ram[0][0x4] = 0x00;
	m_wave_ram[0][0x5] = 0xff;
	m_wave_ram[0][0x6] = 0x00;
	m_wave_ram[0][0x7] = 0xff;
	m_wave_ram[0][0x8] = 0x00;
	m_wave_ram[0][0x9] = 0xff;
	m_wave_ram[0][0xa] = 0x00;
	m_wave_ram[0][0xb] = 0xff;
	m_wave_ram[0][0xc] = 0x00;
	m_wave_ram[0][0xd] = 0xff;
	m_wave_ram[0][0xe] = 0x00;
	m_wave_ram[0][0xf] = 0xff;

	m_wave_ram[1][0x0] = 0x00;
	m_wave_ram[1][0x1] = 0xff;
	m_wave_ram[1][0x2] = 0x00;
	m_wave_ram[1][0x3] = 0xff;
	m_wave_ram[1][0x4] = 0x00;
	m_wave_ram[1][0x5] = 0xff;
	m_wave_ram[1][0x6] = 0x00;
	m_wave_ram[1][0x7] = 0xff;
	m_wave_ram[1][0x8] = 0x00;
	m_wave_ram[1][0x9] = 0xff;
	m_wave_ram[1][0xa] = 0x00;
	m_wave_ram[1][0xb] = 0xff;
	m_wave_ram[1][0xc] = 0x00;
	m_wave_ram[1][0xd] = 0xff;
	m_wave_ram[1][0xe] = 0x00;
	m_wave_ram[1][0xf] = 0xff;
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

TIMER_CALLBACK_MEMBER(gameboy_sound_device::timer_callback)
{
	m_channel->update();
	update_state();
}


void gameboy_sound_device::tick_length(SOUND &snd)
{
	if (snd.length_enabled)
	{
		snd.length = (snd.length + 1) & snd.length_mask;
		if (snd.length == 0)
		{
			snd.on = false;
			snd.length_counting = false;
		}
	}
}


int32_t gameboy_sound_device::calculate_next_sweep(SOUND &snd)
{
	snd.sweep_neg_mode_used = (snd.sweep_direction < 0);
	int32_t new_frequency = snd.frequency + snd.sweep_direction * (snd.frequency >> snd.sweep_shift);

	if (new_frequency > 0x7ff)
	{
		snd.on = false;
	}

	return new_frequency;
}


void gameboy_sound_device::apply_next_sweep(SOUND &snd)
{
	int32_t new_frequency = calculate_next_sweep(snd);

	if (snd.on && snd.sweep_shift > 0)
	{
		snd.frequency = new_frequency;
		snd.reg[3] = snd.frequency & 0xff;
	}
}


void gameboy_sound_device::tick_sweep(SOUND &snd)
{
	snd.sweep_count = (snd.sweep_count - 1) & 0x07;
	if (snd.sweep_count == 0)
	{
		snd.sweep_count = snd.sweep_time;

		if (snd.sweep_enabled && snd.sweep_time > 0)
		{
			apply_next_sweep(snd);
			calculate_next_sweep(snd);
		}
	}
}


void gameboy_sound_device::tick_envelope(SOUND &snd)
{
	if (snd.envelope_enabled)
	{
		snd.envelope_count = (snd.envelope_count - 1) & 0x07;

		if (snd.envelope_count == 0)
		{
			snd.envelope_count = snd.envelope_time;

			if (snd.envelope_count)
			{
				int8_t new_envelope_value = snd.envelope_value + snd.envelope_direction;

				if (new_envelope_value >= 0 && new_envelope_value <= 15)
				{
					snd.envelope_value = new_envelope_value;
				}
				else
				{
					snd.envelope_enabled = false;
				}
			}
		}
	}
}


bool gameboy_sound_device::dac_enabled(SOUND &snd)
{
	return (snd.channel != 3) ? (snd.reg[2] & 0xf8) : (snd.reg[0] & 0x80);
}


void gameboy_sound_device::update_square_channel(SOUND &snd, uint64_t cycles)
{
	if (snd.on)
	{
		// compensate for leftover cycles
		snd.cycles_left += cycles;
		if (snd.cycles_left <= 0)
			return;

		cycles = snd.cycles_left >> 2;
		snd.cycles_left &= 3;
		uint16_t distance = 0x800 - snd.frequency_counter;
		if (cycles >= distance)
		{
			cycles -= distance;
			distance = 0x800 - snd.frequency;
			uint64_t counter = 1 + cycles / distance;

			snd.duty_count = (snd.duty_count + counter) & 0x07;
			snd.signal = wave_duty_table[snd.duty][snd.duty_count];

			snd.frequency_counter = snd.frequency + cycles % distance;
		}
		else
		{
			snd.frequency_counter += cycles;
		}
	}
}


void dmg_apu_device::update_wave_channel(SOUND &snd, uint64_t cycles)
{
	if (snd.on)
	{
		// compensate for leftover cycles
		snd.cycles_left += cycles;

		const uint8_t level = snd.level & 3;
		while (snd.cycles_left >= 2)
		{
			snd.cycles_left -= 2;

			// Calculate next state
			snd.frequency_counter = (snd.frequency_counter + 1) & 0x7ff;
			snd.sample_reading = false;
			if (snd.frequency_counter == 0x7ff)
			{
				snd.offset = (snd.offset + 1) & 0x1f;
			}
			if (snd.frequency_counter == 0)
			{
				// Read next sample
				snd.sample_reading = true;
				snd.current_sample = m_wave_ram[0][(snd.offset / 2)];
				if (!(snd.offset & 0x01))
				{
					snd.current_sample >>= 4;
				}
				snd.current_sample = (snd.current_sample & 0x0f) - 8;

				snd.signal = level ? (snd.current_sample >> (level - 1)) : 0;

				// Reload frequency counter
				snd.frequency_counter = snd.frequency;
			}
		}
	}
}


void cgb04_apu_device::update_wave_channel(SOUND &snd, uint64_t cycles)
{
	if (snd.on)
	{
		// compensate for left over cycles
		snd.cycles_left += cycles;
		if (snd.cycles_left <= 0)
			return;

		cycles = snd.cycles_left >> 1;
		snd.cycles_left &= 1;
		uint16_t distance = 0x800 - snd.frequency_counter;
		const uint8_t level = snd.level & 3;
		if (cycles >= distance)
		{
			cycles -= distance;
			distance = 0x800 - snd.frequency;
			// How many times the condition snd.frequency_counter == 0 is true
			uint64_t counter = 1 + cycles / distance;

			snd.offset = (snd.offset + counter) & 0x1f;
			snd.current_sample = m_wave_ram[0][snd.offset / 2];
			if (!(snd.offset & 1))
			{
				snd.current_sample >>= 4;
			}
			snd.current_sample = (snd.current_sample & 0x0f) - 8;
			snd.signal = level ? (snd.current_sample >> (level - 1)) : 0;

			cycles %= distance;
			snd.sample_reading = !cycles;

			snd.frequency_counter = snd.frequency + cycles;
		}
		else
		{
			snd.frequency_counter += cycles;
		}
	}
}

void agb_apu_device::update_wave_channel(SOUND &snd, uint64_t cycles)
{
	if (snd.on)
	{
		constexpr uint8_t level_table[8] = { 0, 4, 2, 1, 3, 3, 3, 3 };

		// compensate for left over cycles
		snd.cycles_left += cycles;
		if (snd.cycles_left <= 0)
			return;

		cycles = (snd.cycles_left >> 1);
		snd.cycles_left &= 1;
		uint16_t      distance = 0x800 - snd.frequency_counter;
		const uint8_t level = level_table[snd.level];
		if (cycles >= distance)
		{
			cycles -= distance;
			distance = 0x800 - snd.frequency;
			// How many times the condition snd.frequency_counter == 0 is true
			uint64_t    counter = 1 + cycles / distance;

			snd.offset = (snd.offset + counter) & 0x3f;
			const uint8_t bank = snd.size ? BIT(snd.offset, 5) : snd.bank;
			snd.current_sample = m_wave_ram[bank][(snd.offset / 2) & 0xf];
			if (!(snd.offset & 1))
			{
				snd.current_sample >>= 4;
			}
			snd.current_sample = (snd.current_sample & 0x0f) - 8;
			snd.signal = level ? ((snd.current_sample * level) / 4) : 0;

			cycles %= distance;
			snd.sample_reading = !cycles;

			snd.frequency_counter = snd.frequency + cycles;
		}
		else
		{
			snd.frequency_counter += cycles;
		}
	}
}

void gameboy_sound_device::update_noise_channel(SOUND &snd, uint64_t cycles)
{
	snd.cycles_left += cycles;
	uint64_t period = noise_period_cycles();
	while (snd.cycles_left >= period)
	{
		snd.cycles_left -= period;

		// Using a Polynomial Counter (aka Linear Feedback Shift Register)
		// Mode 4 has a 15 bit counter so we need to shift the bits around accordingly.
		uint16_t feedback = ((snd.noise_lfsr >> 1) ^ snd.noise_lfsr) & 1;
		snd.noise_lfsr = (snd.noise_lfsr >> 1) | (feedback << 14);
		if (snd.noise_short)
		{
			snd.noise_lfsr = (snd.noise_lfsr & ~(1 << 6)) | (feedback << 6);
		}
		snd.signal = (snd.noise_lfsr & 1) ? -1 : 1;
	}
}


void gameboy_sound_device::update_state()
{
	attotime now = machine().time();

	// No time travelling
	if (now <= m_last_updated)
	{
		return;
	}

	if (m_snd_control.on)
	{
		uint64_t cycles = attotime_to_clocks(now - m_last_updated);

		uint64_t old_cycles = m_snd_control.cycles;
		m_snd_control.cycles += cycles;

		if ((old_cycles / FRAME_CYCLES) != (m_snd_control.cycles / FRAME_CYCLES))
		{
			// Left over cycles in current frame
			uint64_t cycles_current_frame = FRAME_CYCLES - (old_cycles & (FRAME_CYCLES - 1));

			update_square_channel(m_snd_1, cycles_current_frame);
			update_square_channel(m_snd_2, cycles_current_frame);
			update_wave_channel(m_snd_3, cycles_current_frame);
			update_noise_channel(m_snd_4, cycles_current_frame);

			cycles -= cycles_current_frame;

			// Switch to next frame
			switch ((m_snd_control.cycles / FRAME_CYCLES) & 0x07)
			{
			case 0:
				// length
				tick_length(m_snd_1);
				tick_length(m_snd_2);
				tick_length(m_snd_3);
				tick_length(m_snd_4);
				break;
			case 2:
				// sweep
				tick_sweep(m_snd_1);
				// length
				tick_length(m_snd_1);
				tick_length(m_snd_2);
				tick_length(m_snd_3);
				tick_length(m_snd_4);
				break;
			case 4:
				// length
				tick_length(m_snd_1);
				tick_length(m_snd_2);
				tick_length(m_snd_3);
				tick_length(m_snd_4);
				break;
			case 6:
				// sweep
				tick_sweep(m_snd_1);
				// length
				tick_length(m_snd_1);
				tick_length(m_snd_2);
				tick_length(m_snd_3);
				tick_length(m_snd_4);
				break;
			case 7:
				// update envelope
				tick_envelope(m_snd_1);
				tick_envelope(m_snd_2);
				tick_envelope(m_snd_4);
				break;
			}
		}

		update_square_channel(m_snd_1, cycles);
		update_square_channel(m_snd_2, cycles);
		update_wave_channel(m_snd_3, cycles);
		update_noise_channel(m_snd_4, cycles);
	}

	m_last_updated = now;
}


uint64_t gameboy_sound_device::noise_period_cycles()
{
	static const int divisor[8] = { 8, 16,32, 48, 64, 80, 96, 112 };
	return divisor[m_snd_4.reg[3] & 7] << (m_snd_4.reg[3] >> 4);
}


u8 dmg_apu_device::wave_r(offs_t offset)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		return m_snd_3.sample_reading ? m_wave_ram[0][(m_snd_3.offset / 2)] : 0xff;
	}

	return m_wave_ram[0][offset];
}

u8 cgb04_apu_device::wave_r(offs_t offset)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		return m_wave_ram[0][(m_snd_3.offset / 2)];
	}

	return m_wave_ram[0][offset];
}

u8 agb_apu_device::wave_r(offs_t offset)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		return m_wave_ram[m_snd_3.bank ^ 1][(m_snd_3.offset / 2) & 0xf];
	}

	return m_wave_ram[m_snd_3.bank ^ 1][offset & 0xf];
}

u8 gameboy_sound_device::sound_r(offs_t offset)
{
	if ((offset >= AUD3W0) && (offset <= AUD3WF))
		return wave_r(offset - AUD3W0);

	static const uint8_t read_mask[0x40] =
	{
		0x80,0x3f,0x00,0xff,0xbf,0xff,0x3f,0x00,0xff,0xbf,0x7f,0xff,0x9f,0xff,0xbf,0xff,
		0xff,0x00,0x00,0xbf,0x00,0x00,0x70,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
	};

	// Make sure we are up to date.
	m_channel->update();
	update_state();

	if (m_snd_control.on)
	{
		if (offset == NR52)
		{
			return (m_snd_regs[NR52] & 0xf0) | (m_snd_1.on ? 1 : 0) | (m_snd_2.on ? 2 : 0) | (m_snd_3.on ? 4 : 0) | (m_snd_4.on ? 8 : 0) | 0x70;
		}
		return m_snd_regs[offset] | read_mask[offset & 0x3f];
	}
	else
	{
		return read_mask[offset & 0x3f];
	}
}

u8 agb_apu_device::sound_r(offs_t offset)
{
	if ((offset >= AUD3W0) && (offset <= AUD3WF))
		return wave_r(offset - AUD3W0);

	static constexpr uint8_t read_mask[0x40] = {
			0x80, 0x3f, 0x00, 0xff, 0xbf, 0xff, 0x3f, 0x00, 0xff, 0xbf, 0x1f, 0xff, 0x1f, 0xff, 0xbf, 0xff,
			0xff, 0x00, 0x00, 0xbf, 0x00, 0x00, 0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Make sure we are up to date.
	m_channel->update();
	update_state();

	if (m_snd_control.on)
	{
		if (offset == NR52)
		{
			return (m_snd_regs[NR52] & 0xf0) | (m_snd_1.on ? 1 : 0) | (m_snd_2.on ? 2 : 0) | (m_snd_3.on ? 4 : 0) | (m_snd_4.on ? 8 : 0) | 0x70;
		}
		return m_snd_regs[offset] | read_mask[offset & 0x3f];
	}
	else
	{
		return read_mask[offset & 0x3f];
	}
}

void dmg_apu_device::wave_w(offs_t offset, u8 data)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		if (m_snd_3.sample_reading)
		{
			m_wave_ram[0][(m_snd_3.offset / 2)] = data;
		}
	}
	else
	{
		m_wave_ram[0][offset] = data;
	}
}


void cgb04_apu_device::wave_w(offs_t offset, u8 data)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		m_wave_ram[0][(m_snd_3.offset / 2)] = data;
	}
	else
	{
		m_wave_ram[0][offset] = data;
	}
}

void agb_apu_device::wave_w(offs_t offset, u8 data)
{
	m_channel->update();
	update_state();

	if (m_snd_3.on)
	{
		m_wave_ram[m_snd_3.bank ^ 1][(m_snd_3.offset / 2) & 0xf] = data;
	}
	else
	{
		m_wave_ram[m_snd_3.bank ^ 1][offset & 0xf] = data;
	}
}

void dmg_apu_device::sound_w(offs_t offset, u8 data)
{
	/* change in registers so update first */
	m_channel->update();
	update_state();

	/* Only register NR52 is accessible if the sound controller is disabled */
	if (!m_snd_control.on && offset != NR52 && offset != NR11 && offset != NR21 && offset != NR31 && offset != NR41)
		return;

	sound_w_internal(offset, data);
}


void cgb04_apu_device::sound_w(offs_t offset, u8 data)
{
	/* change in registers so update first */
	m_channel->update();
	update_state();

	/* Only register NR52 is accessible if the sound controller is disabled */
	if (!m_snd_control.on && offset != NR52)
		return;

	sound_w_internal(offset, data);
}


void dmg_apu_device::corrupt_wave_ram()
{
	if (m_snd_3.offset < 8)
	{
		m_wave_ram[0][0x0] = m_wave_ram[0][(m_snd_3.offset / 2)];
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			m_wave_ram[0][i] = m_wave_ram[0][((m_snd_3.offset / 2) & ~0x03) + i];
		}
	}
}


void gameboy_sound_device::sound_w_internal( int offset, uint8_t data )
{
	/* Store the value */
	uint8_t old_data = m_snd_regs[offset];

	if (m_snd_control.on)
	{
		m_snd_regs[offset] = data;
	}

	switch (offset)
	{
	/*MODE 1 */
	case NR10: /* Sweep (R/W) */
		m_snd_1.reg[0] = data;
		m_snd_1.sweep_shift = data & 0x7;
		m_snd_1.sweep_direction = (data & 0x8) ? -1 : 1;
		m_snd_1.sweep_time = (data & 0x70) >> 4;
		if ((old_data & 0x08) && !(data & 0x08) && m_snd_1.sweep_neg_mode_used)
		{
			m_snd_1.on = false;
		}
		break;
	case NR11: /* Sound length/Wave pattern duty (R/W) */
		m_snd_1.reg[1] = data;
		if (m_snd_control.on)
		{
			m_snd_1.duty = (data & 0xc0) >> 6;
		}
		m_snd_1.length = data & 0x3f;
		m_snd_1.length_counting = true;
		break;
	case NR12: /* Envelope (R/W) */
		m_snd_1.reg[2] = data;
		m_snd_1.envelope_value = data >> 4;
		m_snd_1.envelope_direction = (data & 0x8) ? 1 : -1;
		m_snd_1.envelope_time = data & 0x07;
		if (!dac_enabled(m_snd_1))
		{
			m_snd_1.on = false;
		}
		break;
	case NR13: /* Frequency lo (R/W) */
		m_snd_1.reg[3] = data;
		// Only enabling the frequency line breaks blarggs's sound test #5
		// This condition may not be correct
		if (!m_snd_1.sweep_enabled)
		{
			m_snd_1.frequency = ((m_snd_1.reg[4] & 0x7) << 8) | m_snd_1.reg[3];
		}
		break;
	case NR14: /* Frequency hi / Initialize (R/W) */
		m_snd_1.reg[4] = data;
		{
			bool length_was_enabled = m_snd_1.length_enabled;

			m_snd_1.length_enabled = (data & 0x40) ? true : false;
			m_snd_1.frequency = ((m_snd_regs[NR14] & 0x7) << 8) | m_snd_1.reg[3];

			if (!length_was_enabled && !(m_snd_control.cycles & FRAME_CYCLES) && m_snd_1.length_counting)
			{
				if (m_snd_1.length_enabled)
				{
					tick_length(m_snd_1);
				}
			}

			if (data & 0x80)
			{
				m_snd_1.on = true;
				m_snd_1.envelope_enabled = true;
				m_snd_1.envelope_value = m_snd_1.reg[2] >> 4;
				m_snd_1.envelope_count = m_snd_1.envelope_time;
				m_snd_1.sweep_count = m_snd_1.sweep_time;
				m_snd_1.sweep_neg_mode_used = false;
				m_snd_1.signal = 0;
				m_snd_1.length_counting = true;
				m_snd_1.frequency = ((m_snd_1.reg[4] & 0x7) << 8) | m_snd_1.reg[3];
				m_snd_1.frequency_counter = m_snd_1.frequency;
				m_snd_1.cycles_left = 0;
				m_snd_1.duty_count = 0;
				m_snd_1.sweep_enabled = (m_snd_1.sweep_shift != 0) || (m_snd_1.sweep_time != 0);
				if (!dac_enabled(m_snd_1))
				{
					m_snd_1.on = false;
				}
				if (m_snd_1.sweep_shift > 0)
				{
					calculate_next_sweep(m_snd_1);
				}

				if (m_snd_1.length == 0 && m_snd_1.length_enabled && !(m_snd_control.cycles & FRAME_CYCLES))
				{
					tick_length(m_snd_1);
				}
			}
			else
			{
				// This condition may not be correct
				if (!m_snd_1.sweep_enabled)
				{
					m_snd_1.frequency = ((m_snd_1.reg[4] & 0x7) << 8) | m_snd_1.reg[3];
				}
			}
		}
		break;

	/*MODE 2 */
	case NR21: /* Sound length/Wave pattern duty (R/W) */
		m_snd_2.reg[1] = data;
		if (m_snd_control.on)
		{
			m_snd_2.duty = (data & 0xc0) >> 6;
		}
		m_snd_2.length = data & 0x3f;
		m_snd_2.length_counting = true;
		break;
	case NR22: /* Envelope (R/W) */
		m_snd_2.reg[2] = data;
		m_snd_2.envelope_value = data >> 4;
		m_snd_2.envelope_direction = (data & 0x8) ? 1 : -1;
		m_snd_2.envelope_time = data & 0x07;
		if (!dac_enabled(m_snd_2))
		{
			m_snd_2.on = false;
		}
		break;
	case NR23: /* Frequency lo (R/W) */
		m_snd_2.reg[3] = data;
		m_snd_2.frequency = ((m_snd_2.reg[4] & 0x7) << 8) | m_snd_2.reg[3];
		break;
	case NR24: /* Frequency hi / Initialize (R/W) */
		m_snd_2.reg[4] = data;
		{
			bool length_was_enabled = m_snd_2.length_enabled;

			m_snd_2.length_enabled = (data & 0x40) ? true : false;

			if (!length_was_enabled && !(m_snd_control.cycles & FRAME_CYCLES) && m_snd_2.length_counting)
			{
				if (m_snd_2.length_enabled)
				{
					tick_length(m_snd_2);
				}
			}

			if (data & 0x80)
			{
				m_snd_2.on = true;
				m_snd_2.envelope_enabled = true;
				m_snd_2.envelope_value = m_snd_2.reg[2] >> 4;
				m_snd_2.envelope_count = m_snd_2.envelope_time;
				m_snd_2.frequency = ((m_snd_2.reg[4] & 0x7) << 8) | m_snd_2.reg[3];
				m_snd_2.frequency_counter = m_snd_2.frequency;
				m_snd_2.cycles_left = 0;
				m_snd_2.duty_count = 0;
				m_snd_2.signal = 0;
				m_snd_2.length_counting = true;

				if (!dac_enabled(m_snd_2))
				{
					m_snd_2.on = false;
				}

				if (m_snd_2.length == 0 && m_snd_2.length_enabled && !(m_snd_control.cycles & FRAME_CYCLES))
				{
					tick_length(m_snd_2);
				}
			}
			else
			{
				m_snd_2.frequency = ((m_snd_2.reg[4] & 0x7) << 8) | m_snd_2.reg[3];
			}
		}
		break;

	/*MODE 3 */
	case NR30: /* Sound On/Off (R/W) */
		m_snd_3.reg[0] = data;
		m_snd_3.size = BIT(data, 5);
		m_snd_3.bank = BIT(data, 6);
		if (!dac_enabled(m_snd_3))
		{
			m_snd_3.on = false;
		}
		break;
	case NR31: /* Sound Length (R/W) */
		m_snd_3.reg[1] = data;
		m_snd_3.length = data;
		m_snd_3.length_counting = true;
		break;
	case NR32: /* Select Output Level */
		m_snd_3.reg[2] = data;
		m_snd_3.level = (data & 0xe0) >> 5;
		break;
	case NR33: /* Frequency lo (W) */
		m_snd_3.reg[3] = data;
		m_snd_3.frequency = ((m_snd_3.reg[4] & 0x7) << 8) | m_snd_3.reg[3];
		break;
	case NR34: /* Frequency hi / Initialize (W) */
		m_snd_3.reg[4] = data;
		{
			bool length_was_enabled = m_snd_3.length_enabled;

			m_snd_3.length_enabled = (data & 0x40) ? true : false;

			if (!length_was_enabled && !(m_snd_control.cycles & FRAME_CYCLES) && m_snd_3.length_counting)
			{
				if (m_snd_3.length_enabled)
				{
					tick_length(m_snd_3);
				}
			}

			if (data & 0x80)
			{
				if (m_snd_3.on && m_snd_3.frequency_counter == 0x7ff)
				{
					corrupt_wave_ram();
				}
				m_snd_3.on = true;
				m_snd_3.offset = 0;
				m_snd_3.duty = 1;
				m_snd_3.duty_count = 0;
				m_snd_3.length_counting = true;
				m_snd_3.frequency = ((m_snd_3.reg[4] & 0x7) << 8) | m_snd_3.reg[3];
				m_snd_3.frequency_counter = m_snd_3.frequency;
				// There is a tiny bit of delay in starting up the wave channel
				m_snd_3.cycles_left = -6;
				m_snd_3.sample_reading = false;

				if (!dac_enabled(m_snd_3))
				{
					m_snd_3.on = false;
				}

				if (m_snd_3.length == 0 && m_snd_3.length_enabled && !(m_snd_control.cycles & FRAME_CYCLES))
				{
					tick_length(m_snd_3);
				}
			}
			else
			{
				m_snd_3.frequency = ((m_snd_3.reg[4] & 0x7) << 8) | m_snd_3.reg[3];
			}
		}
		break;

	/*MODE 4 */
	case NR41: /* Sound Length (R/W) */
		m_snd_4.reg[1] = data;
		m_snd_4.length = data & 0x3f;
		m_snd_4.length_counting = true;
		break;
	case NR42: /* Envelope (R/W) */
		m_snd_4.reg[2] = data;
		m_snd_4.envelope_value = data >> 4;
		m_snd_4.envelope_direction = (data & 0x8) ? 1 : -1;
		m_snd_4.envelope_time = data & 0x07;
		if (!dac_enabled(m_snd_4))
		{
			m_snd_4.on = false;
		}
		break;
	case NR43: /* Polynomial Counter/Frequency */
		m_snd_4.reg[3] = data;
		m_snd_4.noise_short = (data & 0x8);
		break;
	case NR44: /* Counter/Consecutive / Initialize (R/W)  */
		m_snd_4.reg[4] = data;
		{
			bool length_was_enabled = m_snd_4.length_enabled;

			m_snd_4.length_enabled = (data & 0x40) ? true : false;

			if (!length_was_enabled && !(m_snd_control.cycles & FRAME_CYCLES) && m_snd_4.length_counting)
			{
				if (m_snd_4.length_enabled)
				{
					tick_length(m_snd_4);
				}
			}

			if (data & 0x80)
			{
				m_snd_4.on = true;
				m_snd_4.envelope_enabled = true;
				m_snd_4.envelope_value = m_snd_4.reg[2] >> 4;
				m_snd_4.envelope_count = m_snd_4.envelope_time;
				m_snd_4.frequency_counter = 0;
				m_snd_4.cycles_left = noise_period_cycles();
				m_snd_4.signal = -1;
				m_snd_4.noise_lfsr = 0x7fff;
				m_snd_4.length_counting = true;

				if (!dac_enabled(m_snd_4))
				{
					m_snd_4.on = false;
				}

				if (m_snd_4.length == 0 && m_snd_4.length_enabled && !(m_snd_control.cycles & FRAME_CYCLES))
				{
					tick_length(m_snd_4);
				}
			}
		}
		break;

	/* CONTROL */
	case NR50: /* Channel Control / On/Off / Volume (R/W)  */
		m_snd_control.vol_left = data & 0x7;
		m_snd_control.vol_right = (data & 0x70) >> 4;
		break;
	case NR51: /* Selection of Sound Output Terminal */
		m_snd_control.mode1_right = data & 0x1;
		m_snd_control.mode1_left = (data & 0x10) >> 4;
		m_snd_control.mode2_right = (data & 0x2) >> 1;
		m_snd_control.mode2_left = (data & 0x20) >> 5;
		m_snd_control.mode3_right = (data & 0x4) >> 2;
		m_snd_control.mode3_left = (data & 0x40) >> 6;
		m_snd_control.mode4_right = (data & 0x8) >> 3;
		m_snd_control.mode4_left = (data & 0x80) >> 7;
		break;
	case NR52: // Sound On/Off (R/W)
		// Only bit 7 is writable, writing to bits 0-3 does NOT enable or disable sound. They are read-only.
		if (!(data & 0x80))
		{
			// On DMG the length counters are not affected and not clocked
			// powering off should actually clear all registers
			apu_power_off();
		}
		else
		{
			if (!m_snd_control.on)
			{
				// When switching on, the next step should be 0.
				m_snd_control.cycles |= 7 * FRAME_CYCLES;
			}
		}
		m_snd_control.on = (data & 0x80) ? true : false;
		m_snd_regs[NR52] = data & 0x80;
		break;
	case AUD3W0: // Wavetable (R/W)
	case AUD3W1:
	case AUD3W2:
	case AUD3W3:
	case AUD3W4:
	case AUD3W5:
	case AUD3W6:
	case AUD3W7:
	case AUD3W8:
	case AUD3W9:
	case AUD3WA:
	case AUD3WB:
	case AUD3WC:
	case AUD3WD:
	case AUD3WE:
	case AUD3WF:
		wave_w(offset - AUD3W0, data);
		break;
	}
}


void dmg_apu_device::apu_power_off()
{
	sound_w_internal(NR10, 0x00);
	m_snd_1.duty = 0;
	m_snd_regs[NR11] = 0;
	sound_w_internal(NR12, 0x00);
	sound_w_internal(NR13, 0x00);
	sound_w_internal(NR14, 0x00);
	m_snd_1.length_counting = false;
	m_snd_1.sweep_neg_mode_used = false;

	m_snd_regs[NR21] = 0;
	sound_w_internal(NR22, 0x00);
	sound_w_internal(NR23, 0x00);
	sound_w_internal(NR24, 0x00);
	m_snd_2.length_counting = false;

	sound_w_internal(NR30, 0x00);
	sound_w_internal(NR32, 0x00);
	sound_w_internal(NR33, 0x00);
	sound_w_internal(NR34, 0x00);
	m_snd_3.length_counting = false;
	m_snd_3.current_sample = 0;

	m_snd_regs[NR41] = 0;
	sound_w_internal(NR42, 0x00);
	sound_w_internal(NR43, 0x00);
	sound_w_internal(NR44, 0x00);
	m_snd_4.length_counting = false;
	m_snd_4.cycles_left = noise_period_cycles();

	m_snd_1.on = false;
	m_snd_2.on = false;
	m_snd_3.on = false;
	m_snd_4.on = false;

	m_snd_control.wave_ram_locked = false;

	for (int i = NR44 + 1; i < NR52; i++)
	{
		sound_w_internal(i, 0x00);
	}
}


void cgb04_apu_device::apu_power_off()
{
	sound_w_internal(NR10, 0x00);
	m_snd_1.duty = 0;
	sound_w_internal(NR11, 0x00);
	sound_w_internal(NR12, 0x00);
	sound_w_internal(NR13, 0x00);
	sound_w_internal(NR14, 0x00);
	m_snd_1.length_counting = false;
	m_snd_1.sweep_neg_mode_used = false;

	sound_w_internal(NR21, 0x00);
	sound_w_internal(NR22, 0x00);
	sound_w_internal(NR23, 0x00);
	sound_w_internal(NR24, 0x00);
	m_snd_2.length_counting = false;

	sound_w_internal(NR30, 0x00);
	sound_w_internal(NR31, 0x00);
	sound_w_internal(NR32, 0x00);
	sound_w_internal(NR33, 0x00);
	sound_w_internal(NR34, 0x00);
	m_snd_3.length_counting = false;
	m_snd_3.current_sample = 0;

	sound_w_internal(NR41, 0x00);
	sound_w_internal(NR42, 0x00);
	sound_w_internal(NR43, 0x00);
	sound_w_internal(NR44, 0x00);
	m_snd_4.length_counting = false;
	m_snd_4.cycles_left = noise_period_cycles();

	m_snd_1.on = false;
	m_snd_2.on = false;
	m_snd_3.on = false;
	m_snd_4.on = false;

	m_snd_control.wave_ram_locked = false;

	for (int i = NR44 + 1; i < NR52; i++)
	{
		sound_w_internal(i, 0x00);
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gameboy_sound_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &outputl = outputs[0];
	auto &outputr = outputs[1];
	for (int sampindex = 0; sampindex < outputl.samples(); sampindex++)
	{
		s32 sample;
		s32 left = 0;
		s32 right = 0;

		/* Mode 1 - Wave with Envelope and Sweep */
		if (m_snd_1.on)
		{
			sample = m_snd_1.signal * m_snd_1.envelope_value;

			if (m_snd_control.mode1_left)
				left += sample;
			if (m_snd_control.mode1_right)
				right += sample;
		}

		/* Mode 2 - Wave with Envelope */
		if (m_snd_2.on)
		{
			sample = m_snd_2.signal * m_snd_2.envelope_value;
			if (m_snd_control.mode2_left)
				left += sample;
			if (m_snd_control.mode2_right)
				right += sample;
		}

		/* Mode 3 - Wave patterns from WaveRAM */
		if (m_snd_3.on)
		{
			sample = m_snd_3.signal;
			if (m_snd_control.mode3_left)
				left += sample;
			if (m_snd_control.mode3_right)
				right += sample;
		}

		/* Mode 4 - Noise with Envelope */
		if (m_snd_4.on)
		{
			sample = m_snd_4.signal * m_snd_4.envelope_value;
			if (m_snd_control.mode4_left)
				left += sample;
			if (m_snd_control.mode4_right)
				right += sample;
		}

		/* Adjust for master volume */
		left *= m_snd_control.vol_left;
		right *= m_snd_control.vol_right;

		/* Update the buffers */
		outputl.put_int(sampindex, left, 32768 / 64);
		outputr.put_int(sampindex, right, 32768 / 64);
	}
}
