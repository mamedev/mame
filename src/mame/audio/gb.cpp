// license:BSD-3-Clause
// copyright-holders:Anthony Kruize
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
*
***************************************************************************************/

#include "emu.h"
#include "gb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NR10 0x00
#define NR11 0x01
#define NR12 0x02
#define NR13 0x03
#define NR14 0x04
#define NR21 0x06
#define NR22 0x07
#define NR23 0x08
#define NR24 0x09
#define NR30 0x0A
#define NR31 0x0B
#define NR32 0x0C
#define NR33 0x0D
#define NR34 0x0E
#define NR41 0x10
#define NR42 0x11
#define NR43 0x12
#define NR44 0x13
#define NR50 0x14
#define NR51 0x15
#define NR52 0x16
#define AUD3W0 0x20
#define AUD3W1 0x21
#define AUD3W2 0x22
#define AUD3W3 0x23
#define AUD3W4 0x24
#define AUD3W5 0x25
#define AUD3W6 0x26
#define AUD3W7 0x27
#define AUD3W8 0x28
#define AUD3W9 0x29
#define AUD3WA 0x2A
#define AUD3WB 0x2B
#define AUD3WC 0x2C
#define AUD3WD 0x2D
#define AUD3WE 0x2E
#define AUD3WF 0x2F

#define LEFT 1
#define RIGHT 2
#define FIXED_POINT 16

/* Represents wave duties of 12.5%, 25%, 50% and 75% */
static const float wave_duty_table[4] = { 8.0f, 4.0f, 2.0f, 1.33f };

// device type definition
const device_type GAMEBOY = &device_creator<gameboy_sound_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  gameboy_sound_device - constructor
//-------------------------------------------------

gameboy_sound_device::gameboy_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, GAMEBOY, "LR35902 Sound", tag, owner, clock, "gameboy_sound", __FILE__),
						device_sound_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void gameboy_sound_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gameboy_sound_device::device_start()
{
	m_channel = machine().sound().stream_alloc(*this, 0, 2, machine().sample_rate());
	m_rate = machine().sample_rate();

	save_item(NAME(m_snd_regs));
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
	// sound 1
	save_item(NAME(m_snd_1.on));
	save_item(NAME(m_snd_1.channel));
	save_item(NAME(m_snd_1.length));
	save_item(NAME(m_snd_1.pos));
	save_item(NAME(m_snd_1.period));
	save_item(NAME(m_snd_1.count));
	save_item(NAME(m_snd_1.mode));
	save_item(NAME(m_snd_1.duty));
	save_item(NAME(m_snd_1.env_value));
	save_item(NAME(m_snd_1.env_direction));
	save_item(NAME(m_snd_1.env_length));
	save_item(NAME(m_snd_1.env_count));
	save_item(NAME(m_snd_1.signal));
	save_item(NAME(m_snd_1.frequency));
	save_item(NAME(m_snd_1.swp_shift));
	save_item(NAME(m_snd_1.swp_direction));
	save_item(NAME(m_snd_1.swp_time));
	save_item(NAME(m_snd_1.swp_count));
	save_item(NAME(m_snd_1.level));
	save_item(NAME(m_snd_1.offset));
	save_item(NAME(m_snd_1.dutycount));
	save_item(NAME(m_snd_1.ply_step));
	save_item(NAME(m_snd_1.ply_value));
	// sound 2
	save_item(NAME(m_snd_2.on));
	save_item(NAME(m_snd_2.channel));
	save_item(NAME(m_snd_2.length));
	save_item(NAME(m_snd_2.pos));
	save_item(NAME(m_snd_2.period));
	save_item(NAME(m_snd_2.count));
	save_item(NAME(m_snd_2.mode));
	save_item(NAME(m_snd_2.duty));
	save_item(NAME(m_snd_2.env_value));
	save_item(NAME(m_snd_2.env_direction));
	save_item(NAME(m_snd_2.env_length));
	save_item(NAME(m_snd_2.env_count));
	save_item(NAME(m_snd_2.signal));
	save_item(NAME(m_snd_2.frequency));
	save_item(NAME(m_snd_2.swp_shift));
	save_item(NAME(m_snd_2.swp_direction));
	save_item(NAME(m_snd_2.swp_time));
	save_item(NAME(m_snd_2.swp_count));
	save_item(NAME(m_snd_2.level));
	save_item(NAME(m_snd_2.offset));
	save_item(NAME(m_snd_2.dutycount));
	save_item(NAME(m_snd_2.ply_step));
	save_item(NAME(m_snd_2.ply_value));
	// sound 3
	save_item(NAME(m_snd_3.on));
	save_item(NAME(m_snd_3.channel));
	save_item(NAME(m_snd_3.length));
	save_item(NAME(m_snd_3.pos));
	save_item(NAME(m_snd_3.period));
	save_item(NAME(m_snd_3.count));
	save_item(NAME(m_snd_3.mode));
	save_item(NAME(m_snd_3.duty));
	save_item(NAME(m_snd_3.env_value));
	save_item(NAME(m_snd_3.env_direction));
	save_item(NAME(m_snd_3.env_length));
	save_item(NAME(m_snd_3.env_count));
	save_item(NAME(m_snd_3.signal));
	save_item(NAME(m_snd_3.frequency));
	save_item(NAME(m_snd_3.swp_shift));
	save_item(NAME(m_snd_3.swp_direction));
	save_item(NAME(m_snd_3.swp_time));
	save_item(NAME(m_snd_3.swp_count));
	save_item(NAME(m_snd_3.level));
	save_item(NAME(m_snd_3.offset));
	save_item(NAME(m_snd_3.dutycount));
	save_item(NAME(m_snd_3.ply_step));
	save_item(NAME(m_snd_3.ply_value));
	// sound 4
	save_item(NAME(m_snd_4.on));
	save_item(NAME(m_snd_4.channel));
	save_item(NAME(m_snd_4.length));
	save_item(NAME(m_snd_4.pos));
	save_item(NAME(m_snd_4.period));
	save_item(NAME(m_snd_4.count));
	save_item(NAME(m_snd_4.mode));
	save_item(NAME(m_snd_4.duty));
	save_item(NAME(m_snd_4.env_value));
	save_item(NAME(m_snd_4.env_direction));
	save_item(NAME(m_snd_4.env_length));
	save_item(NAME(m_snd_4.env_count));
	save_item(NAME(m_snd_4.signal));
	save_item(NAME(m_snd_4.frequency));
	save_item(NAME(m_snd_4.swp_shift));
	save_item(NAME(m_snd_4.swp_direction));
	save_item(NAME(m_snd_4.swp_time));
	save_item(NAME(m_snd_4.swp_count));
	save_item(NAME(m_snd_4.level));
	save_item(NAME(m_snd_4.offset));
	save_item(NAME(m_snd_4.dutycount));
	save_item(NAME(m_snd_4.ply_step));
	save_item(NAME(m_snd_4.ply_value));
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

	/* Calculate the envelope and sweep tables */
	for (int i = 0; i < 8; i++)
	{
		m_env_length_table[i] = (i * ((1 << FIXED_POINT) / 64) * m_rate) >> FIXED_POINT;
		m_swp_time_table[i] = (((i << FIXED_POINT) / 128) * m_rate) >> (FIXED_POINT - 1);
	}

	/* Calculate the period tables */
	for (int i = 0; i < MAX_FREQUENCIES; i++)
	{
		m_period_table[i] = ((1 << FIXED_POINT) / (131072 / (2048 - i))) * m_rate;
		m_period_mode3_table[i] = ((1 << FIXED_POINT) / (65536 / (2048 - i))) * m_rate;
	}
	/* Calculate the period table for mode 4 */
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			// i is the dividing ratio of frequencies
			// j is the shift clock frequency
			m_period_mode4_table[i][j] = ((1 << FIXED_POINT) / (524288 / ((i == 0) ? 0.5 : i) / (1 << (j + 1)))) * m_rate;
		}
	}

	/* Calculate the length table */
	for (int i = 0; i < 64; i++)
		m_length_table[i] = ((64 - i) * ((1 << FIXED_POINT)/256) * m_rate) >> FIXED_POINT;

	/* Calculate the length table for mode 3 */
	for (int i = 0; i < 256; i++)
		m_length_mode3_table[i] = ((256 - i) * ((1 << FIXED_POINT)/256) * m_rate) >> FIXED_POINT;

	sound_w_internal(NR52, 0x00);
	m_snd_regs[AUD3W0] = 0xac;
	m_snd_regs[AUD3W1] = 0xdd;
	m_snd_regs[AUD3W2] = 0xda;
	m_snd_regs[AUD3W3] = 0x48;
	m_snd_regs[AUD3W4] = 0x36;
	m_snd_regs[AUD3W5] = 0x02;
	m_snd_regs[AUD3W6] = 0xcf;
	m_snd_regs[AUD3W7] = 0x16;
	m_snd_regs[AUD3W8] = 0x2c;
	m_snd_regs[AUD3W9] = 0x04;
	m_snd_regs[AUD3WA] = 0xe5;
	m_snd_regs[AUD3WB] = 0x2c;
	m_snd_regs[AUD3WC] = 0xac;
	m_snd_regs[AUD3WD] = 0xdd;
	m_snd_regs[AUD3WE] = 0xda;
	m_snd_regs[AUD3WF] = 0x48;
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

READ8_MEMBER( gameboy_sound_device::wave_r )
{
	/* TODO: properly emulate scrambling of wave ram area when playback is active */
	return m_snd_regs[AUD3W0 + offset] | m_snd_3.on;
}

READ8_MEMBER( gameboy_sound_device::sound_r )
{
	switch (offset)
	{
		case 0x05:
		case 0x0f:
			return 0xff;
		case NR52:
			return 0x70 | m_snd_regs[offset];
		default:
			return m_snd_regs[offset];
	}
}

WRITE8_MEMBER( gameboy_sound_device::wave_w )
{
	m_snd_regs[AUD3W0 + offset] = data;
}

WRITE8_MEMBER( gameboy_sound_device::sound_w )
{
	/* change in registers so update first */
	m_channel->update();

	/* Only register NR52 is accessible if the sound controller is disabled */
	if (!m_snd_control.on && offset != NR52)
		return;

	sound_w_internal(offset, data);
}


void gameboy_sound_device::sound_w_internal( int offset, UINT8 data )
{
	/* Store the value */
	m_snd_regs[offset] = data;

	switch (offset)
	{
	/*MODE 1 */
	case NR10: /* Sweep (R/W) */
		m_snd_1.swp_shift = data & 0x7;
		m_snd_1.swp_direction = (data & 0x8) >> 3;
		m_snd_1.swp_direction |= m_snd_1.swp_direction - 1;
		m_snd_1.swp_time = m_swp_time_table[ (data & 0x70) >> 4 ];
		break;
	case NR11: /* Sound length/Wave pattern duty (R/W) */
		m_snd_1.duty = (data & 0xc0) >> 6;
		m_snd_1.length = m_length_table[data & 0x3f];
		break;
	case NR12: /* Envelope (R/W) */
		m_snd_1.env_value = data >> 4;
		m_snd_1.env_direction = (data & 0x8) >> 3;
		m_snd_1.env_direction |= m_snd_1.env_direction - 1;
		m_snd_1.env_length = m_env_length_table[data & 0x7];
		break;
	case NR13: /* Frequency lo (R/W) */
		m_snd_1.frequency = ((m_snd_regs[NR14] & 0x7) << 8) | m_snd_regs[NR13];
		m_snd_1.period = m_period_table[m_snd_1.frequency];
		break;
	case NR14: /* Frequency hi / Initialize (R/W) */
		m_snd_1.mode = (data & 0x40) >> 6;
		m_snd_1.frequency = ((m_snd_regs[NR14] & 0x7) << 8) | m_snd_regs[NR13];
		m_snd_1.period = m_period_table[m_snd_1.frequency];
		if (data & 0x80)
		{
			if (!m_snd_1.on)
				m_snd_1.pos = 0;
			m_snd_1.on = 1;
			m_snd_1.count = 0;
			m_snd_1.env_value = m_snd_regs[NR12] >> 4;
			m_snd_1.env_count = 0;
			m_snd_1.swp_count = 0;
			m_snd_1.signal = 0x1;
			m_snd_regs[NR52] |= 0x1;
		}
		break;

	/*MODE 2 */
	case NR21: /* Sound length/Wave pattern duty (R/W) */
		m_snd_2.duty = (data & 0xc0) >> 6;
		m_snd_2.length = m_length_table[data & 0x3f];
		break;
	case NR22: /* Envelope (R/W) */
		m_snd_2.env_value = data >> 4;
		m_snd_2.env_direction = (data & 0x8) >> 3;
		m_snd_2.env_direction |= m_snd_2.env_direction - 1;
		m_snd_2.env_length = m_env_length_table[data & 0x7];
		break;
	case NR23: /* Frequency lo (R/W) */
		m_snd_2.period = m_period_table[((m_snd_regs[NR24] & 0x7) << 8) | m_snd_regs[NR23]];
		break;
	case NR24: /* Frequency hi / Initialize (R/W) */
		m_snd_2.mode = (data & 0x40) >> 6;
		m_snd_2.period = m_period_table[((m_snd_regs[NR24] & 0x7) << 8) | m_snd_regs[NR23]];
		if (data & 0x80)
		{
			if (!m_snd_2.on)
				m_snd_2.pos = 0;
			m_snd_2.on = 1;
			m_snd_2.count = 0;
			m_snd_2.env_value = m_snd_regs[NR22] >> 4;
			m_snd_2.env_count = 0;
			m_snd_2.signal = 0x1;
			m_snd_regs[NR52] |= 0x2;
		}
		break;

	/*MODE 3 */
	case NR30: /* Sound On/Off (R/W) */
		m_snd_3.on = (data & 0x80) >> 7;
		break;
	case NR31: /* Sound Length (R/W) */
		m_snd_3.length = m_length_mode3_table[data];
		break;
	case NR32: /* Select Output Level */
		m_snd_3.level = (data & 0x60) >> 5;
		break;
	case NR33: /* Frequency lo (W) */
		m_snd_3.period = m_period_mode3_table[((m_snd_regs[NR34] & 0x7) << 8) + m_snd_regs[NR33]];
		break;
	case NR34: /* Frequency hi / Initialize (W) */
		m_snd_3.mode = (data & 0x40) >> 6;
		m_snd_3.period = m_period_mode3_table[((m_snd_regs[NR34] & 0x7) << 8) + m_snd_regs[NR33]];
		if (data & 0x80)
		{
			if (!m_snd_3.on)
			{
				m_snd_3.pos = 0;
				m_snd_3.offset = 0;
				m_snd_3.duty = 0;
			}
			m_snd_3.on = 1;
			m_snd_3.count = 0;
			m_snd_3.duty = 1;
			m_snd_3.dutycount = 0;
			m_snd_regs[NR52] |= 0x4;
		}
		break;

	/*MODE 4 */
	case NR41: /* Sound Length (R/W) */
		m_snd_4.length = m_length_table[data & 0x3f];
		break;
	case NR42: /* Envelope (R/W) */
		m_snd_4.env_value = data >> 4;
		m_snd_4.env_direction = (data & 0x8) >> 3;
		m_snd_4.env_direction |= m_snd_4.env_direction - 1;
		m_snd_4.env_length = m_env_length_table[data & 0x7];
		break;
	case NR43: /* Polynomial Counter/Frequency */
		m_snd_4.period = m_period_mode4_table[data & 0x7][(data & 0xF0) >> 4];
		m_snd_4.ply_step = (data & 0x8) >> 3;
		break;
	case NR44: /* Counter/Consecutive / Initialize (R/W)  */
		m_snd_4.mode = (data & 0x40) >> 6;
		if (data & 0x80)
		{
			if (!m_snd_4.on)
				m_snd_4.pos = 0;
			m_snd_4.on = 1;
			m_snd_4.count = 0;
			m_snd_4.env_value = m_snd_regs[NR42] >> 4;
			m_snd_4.env_count = 0;
			m_snd_4.signal = machine().rand();
			m_snd_4.ply_value = 0x7fff;
			m_snd_regs[NR52] |= 0x8;
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
	case NR52: /* Sound On/Off (R/W) */
		/* Only bit 7 is writable, writing to bits 0-3 does NOT enable or
		   disable sound.  They are read-only */
		m_snd_control.on = (data & 0x80) >> 7;
		if (!m_snd_control.on)
		{
			sound_w_internal(NR10, 0x80);
			sound_w_internal(NR11, 0xBF);
			sound_w_internal(NR12, 0xF3);
			sound_w_internal(NR13, 0xFF);
			sound_w_internal(NR14, 0xBF);
			//sound_w_internal(NR20, 0xFF);
			sound_w_internal(NR21, 0x3F);
			sound_w_internal(NR22, 0x00);
			sound_w_internal(NR23, 0xFF);
			sound_w_internal(NR24, 0xBF);
			sound_w_internal(NR30, 0x7F);
			sound_w_internal(NR31, 0xFF);
			sound_w_internal(NR32, 0x9F);
			sound_w_internal(NR33, 0xFF);
			sound_w_internal(NR34, 0xBF);
			//sound_w_internal(NR40, 0xFF);
			sound_w_internal(NR41, 0xFF);
			sound_w_internal(NR42, 0x00);
			sound_w_internal(NR43, 0x00);
			sound_w_internal(NR44, 0xBF);
			sound_w_internal(NR50, 0x77);
			sound_w_internal(NR51, 0xF3);
			m_snd_1.on = 0;
			m_snd_2.on = 0;
			m_snd_3.on = 0;
			m_snd_4.on = 0;
			m_snd_regs[offset] = 0;
		}
		break;
	}
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void gameboy_sound_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t sample, left, right, mode4_mask;

	while (samples-- > 0)
	{
		left = right = 0;

		/* Mode 1 - Wave with Envelope and Sweep */
		if (m_snd_1.on)
		{
			sample = m_snd_1.signal * m_snd_1.env_value;
			m_snd_1.pos++;
			if (m_snd_1.pos == (UINT32)(m_snd_1.period / wave_duty_table[m_snd_1.duty]) >> FIXED_POINT)
			{
				m_snd_1.signal = -m_snd_1.signal;
			}
			else if (m_snd_1.pos > (m_snd_1.period >> FIXED_POINT))
			{
				m_snd_1.pos = 0;
				m_snd_1.signal = -m_snd_1.signal;
			}

			if (m_snd_1.length && m_snd_1.mode)
			{
				m_snd_1.count++;
				if (m_snd_1.count >= m_snd_1.length)
				{
					m_snd_1.on = 0;
					m_snd_regs[NR52] &= 0xFE;
				}
			}

			if (m_snd_1.env_length)
			{
				m_snd_1.env_count++;
				if (m_snd_1.env_count >= m_snd_1.env_length)
				{
					m_snd_1.env_count = 0;
					m_snd_1.env_value += m_snd_1.env_direction;
					if (m_snd_1.env_value < 0)
						m_snd_1.env_value = 0;
					if (m_snd_1.env_value > 15)
						m_snd_1.env_value = 15;
				}
			}

			if (m_snd_1.swp_time)
			{
				m_snd_1.swp_count++;
				if (m_snd_1.swp_count >= m_snd_1.swp_time)
				{
					m_snd_1.swp_count = 0;
					if (m_snd_1.swp_direction > 0)
					{
						m_snd_1.frequency -= m_snd_1.frequency / (1 << m_snd_1.swp_shift);
						if (m_snd_1.frequency <= 0)
						{
							m_snd_1.on = 0;
							m_snd_regs[NR52] &= 0xFE;
						}
					}
					else
					{
						m_snd_1.frequency += m_snd_1.frequency / (1 << m_snd_1.swp_shift);
						if (m_snd_1.frequency >= MAX_FREQUENCIES)
						{
							m_snd_1.frequency = MAX_FREQUENCIES - 1;
						}
					}

					m_snd_1.period = m_period_table[m_snd_1.frequency];
				}
			}

			if (m_snd_control.mode1_left)
				left += sample;
			if (m_snd_control.mode1_right)
				right += sample;
		}

		/* Mode 2 - Wave with Envelope */
		if (m_snd_2.on)
		{
			sample = m_snd_2.signal * m_snd_2.env_value;
			m_snd_2.pos++;
			if( m_snd_2.pos == (UINT32)(m_snd_2.period / wave_duty_table[m_snd_2.duty]) >> FIXED_POINT)
			{
				m_snd_2.signal = -m_snd_2.signal;
			}
			else if (m_snd_2.pos > (m_snd_2.period >> FIXED_POINT))
			{
				m_snd_2.pos = 0;
				m_snd_2.signal = -m_snd_2.signal;
			}

			if (m_snd_2.length && m_snd_2.mode)
			{
				m_snd_2.count++;
				if (m_snd_2.count >= m_snd_2.length)
				{
					m_snd_2.on = 0;
					m_snd_regs[NR52] &= 0xFD;
				}
			}

			if (m_snd_2.env_length)
			{
				m_snd_2.env_count++;
				if (m_snd_2.env_count >= m_snd_2.env_length)
				{
					m_snd_2.env_count = 0;
					m_snd_2.env_value += m_snd_2.env_direction;
					if (m_snd_2.env_value < 0)
						m_snd_2.env_value = 0;
					if (m_snd_2.env_value > 15)
						m_snd_2.env_value = 15;
				}
			}

			if (m_snd_control.mode2_left)
				left += sample;
			if (m_snd_control.mode2_right)
				right += sample;
		}

		/* Mode 3 - Wave patterns from WaveRAM */
		if (m_snd_3.on)
		{
			/* NOTE: This is extremely close, but not quite right.
			 The problem is for GB frequencies above 2000 the frequency gets
			 clipped. This is caused because m_snd_3.pos is never 0 at the test.*/
			sample = m_snd_regs[AUD3W0 + (m_snd_3.offset/2)];
			if (!(m_snd_3.offset % 2))
			{
				sample >>= 4;
			}
			sample = (sample & 0xF) - 8;

			if (m_snd_3.level)
				sample >>= (m_snd_3.level - 1);
			else
				sample = 0;

			m_snd_3.pos++;
			if (m_snd_3.pos >= ((UINT32)(((m_snd_3.period) >> 21)) + m_snd_3.duty))
			{
				m_snd_3.pos = 0;
				if (m_snd_3.dutycount == ((UINT32)(((m_snd_3.period) >> FIXED_POINT)) % 32))
				{
					m_snd_3.duty--;
				}
				m_snd_3.dutycount++;
				m_snd_3.offset++;
				if (m_snd_3.offset > 31)
				{
					m_snd_3.offset = 0;
					m_snd_3.duty = 1;
					m_snd_3.dutycount = 0;
				}
			}

			if (m_snd_3.length && m_snd_3.mode)
			{
				m_snd_3.count++;
				if (m_snd_3.count >= m_snd_3.length)
				{
					m_snd_3.on = 0;
					m_snd_regs[NR52] &= 0xFB;
				}
			}

			if (m_snd_control.mode3_left)
				left += sample;
			if (m_snd_control.mode3_right)
				right += sample;
		}

		/* Mode 4 - Noise with Envelope */
		if (m_snd_4.on)
		{
			/* Similar problem to Mode 3, we seem to miss some notes */
			sample = m_snd_4.signal & m_snd_4.env_value;
			m_snd_4.pos++;
			if (m_snd_4.pos == (m_snd_4.period >> (FIXED_POINT + 1)))
			{
				/* Using a Polynomial Counter (aka Linear Feedback Shift Register)
				 Mode 4 has a 7 bit and 15 bit counter so we need to shift the
				 bits around accordingly */
				mode4_mask = (((m_snd_4.ply_value & 0x2) >> 1) ^ (m_snd_4.ply_value & 0x1)) << (m_snd_4.ply_step ? 6 : 14);
				m_snd_4.ply_value >>= 1;
				m_snd_4.ply_value |= mode4_mask;
				m_snd_4.ply_value &= (m_snd_4.ply_step ? 0x7f : 0x7fff);
				m_snd_4.signal = (INT8)m_snd_4.ply_value;
			}
			else if (m_snd_4.pos > (m_snd_4.period >> FIXED_POINT))
			{
				m_snd_4.pos = 0;
				mode4_mask = (((m_snd_4.ply_value & 0x2) >> 1) ^ (m_snd_4.ply_value & 0x1)) << (m_snd_4.ply_step ? 6 : 14);
				m_snd_4.ply_value >>= 1;
				m_snd_4.ply_value |= mode4_mask;
				m_snd_4.ply_value &= (m_snd_4.ply_step ? 0x7f : 0x7fff);
				m_snd_4.signal = (INT8)m_snd_4.ply_value;
			}

			if (m_snd_4.length && m_snd_4.mode)
			{
				m_snd_4.count++;
				if (m_snd_4.count >= m_snd_4.length)
				{
					m_snd_4.on = 0;
					m_snd_regs[NR52] &= 0xF7;
				}
			}

			if (m_snd_4.env_length)
			{
				m_snd_4.env_count++;
				if (m_snd_4.env_count >= m_snd_4.env_length)
				{
					m_snd_4.env_count = 0;
					m_snd_4.env_value += m_snd_4.env_direction;
					if (m_snd_4.env_value < 0)
						m_snd_4.env_value = 0;
					if (m_snd_4.env_value > 15)
						m_snd_4.env_value = 15;
				}
			}

			if (m_snd_control.mode4_left)
				left += sample;
			if (m_snd_control.mode4_right)
				right += sample;
		}

		/* Adjust for master volume */
		left *= m_snd_control.vol_left;
		right *= m_snd_control.vol_right;

		/* pump up the volume */
		left <<= 6;
		right <<= 6;

		/* Update the buffers */
		*(outputs[0]++) = left;
		*(outputs[1]++) = right;
	}

	m_snd_regs[NR52] = (m_snd_regs[NR52]&0xf0) | m_snd_1.on | (m_snd_2.on << 1) | (m_snd_3.on << 2) | (m_snd_4.on << 3);

}
