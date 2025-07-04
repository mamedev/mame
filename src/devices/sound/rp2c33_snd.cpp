// license:BSD-3-Clause
// copyright-holders:cam900, Brad Smith, Brezza
/***************************************************************************

    Ricoh RP2C33 Sound emulation

    Based on:
    - NSFplay github code by Brad Smith/Brezza
    - Information from NESDev wiki
      (https://www.nesdev.org/wiki/FDS_audio)

    TODO:
    - verify register behaviors
    - verify unknown read, writes
    - Lowpass filter?

***************************************************************************/

#include "emu.h"
#include "rp2c33_snd.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(RP2C33_SOUND, rp2c33_sound_device, "rp2c33_snd", "Ricoh RP2C33 (sound)")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  rp2c33_sound_device - constructor
//-------------------------------------------------

rp2c33_sound_device::rp2c33_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RP2C33_SOUND, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
{
	std::fill(std::begin(m_wave), std::end(m_wave), 0);
	std::fill(std::begin(m_mod_table), std::end(m_mod_table), 0);
	std::fill(std::begin(m_mvol_table), std::end(m_mvol_table), 0);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void rp2c33_sound_device::device_start()
{
	// normalize output to 16 bit
	for (int i = 0; i < 4; i++)
		m_mvol_table[i] = int((65536.0 / (32.0 * 64.0)) * 2.0 / double(i + 2));

	m_stream = stream_alloc(0, 1, clock());

	// save states
	save_item(NAME(m_regs));
	save_item(NAME(m_wave));
	save_item(NAME(m_vol_env_disable));
	save_item(NAME(m_vol_env_mode));
	save_item(NAME(m_vol_env_spd));
	save_item(NAME(m_vol_env_clk));
	save_item(NAME(m_vol_env_out));
	save_item(NAME(m_wave_halt));
	save_item(NAME(m_env_halt));
	save_item(NAME(m_wave_freq));
	save_item(NAME(m_wave_acc));
	save_item(NAME(m_wave_addr));

	save_item(NAME(m_mod_env_disable));
	save_item(NAME(m_mod_env_mode));
	save_item(NAME(m_mod_env_spd));
	save_item(NAME(m_mod_env_clk));
	save_item(NAME(m_mod_env_out));
	save_item(NAME(m_mod_halt));
	save_item(NAME(m_mod_freq));
	save_item(NAME(m_mod_table));
	save_item(NAME(m_mod_acc));
	save_item(NAME(m_mod_addr));
	save_item(NAME(m_mod_pos));

	save_item(NAME(m_env_spd));
	save_item(NAME(m_wave_write));
	save_item(NAME(m_mvol));
	save_item(NAME(m_output));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void rp2c33_sound_device::device_reset()
{
}


//-------------------------------------------------
//  device_clock_changed - called if the clock
//  changes
//-------------------------------------------------

void rp2c33_sound_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock());
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

u8 rp2c33_sound_device::read(offs_t offset)
{
	m_stream->update();

	offset += 0x4080;
	// TODO: open bus?
	u8 ret = 0;
	switch (offset)
	{
		case 0x4090: // volume envelope output
			ret = (m_vol_env_out & 0x3f) | 0x40;
			break;
		case 0x4091: // bit 12-19 of accumulator
			ret = (m_wave_addr << 4) + (m_wave_acc >> 12);
			break;
		case 0x4092: // modulator envelope output
			ret = (m_mod_env_out & 0x3f) | 0x40;
			break;
		case 0x4093: // bit 5-11 of modtable address
			ret = (m_mod_addr >> 5) & 0x7f;
			break;
		case 0x4094: // mod counter + gain result
			break;
		case 0x4095: // mod counter increment
			ret = mod_inc[m_mod_table[(m_mod_addr >> 1) & 0x1f]] & 0xf;
			// bit 4-7 : unknown counter
			break;
		case 0x4096: // wavetable value
			ret = (m_wave[m_wave_addr & 0x3f] & 0x3f) | 0x40;
			break;
		case 0x4097: // modulator position
			ret = m_mod_pos & 0x7f;
			break;
	}
	return ret;
}

void rp2c33_sound_device::write(offs_t offset, u8 data)
{
	m_regs[offset & 0xf] = data;
	m_stream->update();

	offset += 0x4080;
	switch (offset)
	{
		case 0x4080: // volume envelope
			m_vol_env_disable = BIT(data, 7);
			m_vol_env_mode = BIT(data, 6);
			m_vol_env_spd = data & 0x3f;
			m_vol_env_clk = 0;
			if (m_vol_env_disable)
				m_vol_env_out = m_vol_env_spd;
			break;
		case 0x4082: // wave frequency low
			m_wave_freq = (m_wave_freq & 0xf00) | (data & 0xff);
			break;
		case 0x4083: // wave frequency high
			m_wave_halt = BIT(data, 7);
			m_env_halt = BIT(data, 6);
			m_wave_freq = (m_wave_freq & 0x0ff) | ((data & 0xf) << 8);
			if (m_wave_halt)
				m_wave_acc = 0;
			if (m_env_halt)
				m_vol_env_clk = m_mod_env_clk = 0;
			break;
		case 0x4084: // modulator envelope
			m_mod_env_disable = BIT(data, 7);
			m_mod_env_mode = BIT(data, 6);
			m_mod_env_spd = data & 0x3f;
			m_mod_env_clk = 0;
			if (m_mod_env_disable)
				m_mod_env_out = m_mod_env_spd;
			break;
		case 0x4085: // modulator position
			m_mod_pos = data & 0x7f;
			break;
		case 0x4086: // modulator frequency low
			m_mod_freq = (m_mod_freq & 0xf00) | (data & 0xff);
			break;
		case 0x4087: // modulator frequency high
			m_mod_halt = BIT(data, 7);
			// TODO: bit 6?
			m_mod_freq = (m_mod_freq & 0x0ff) | ((data & 0xf) << 8);
			if (m_mod_halt)
				m_mod_acc = 0;
			break;
		case 0x4088: // modulator table
			if (m_mod_halt)
			{
				m_mod_table[(m_mod_addr >> 1) & 0x1f] = data & 7;
				m_mod_addr += 2;
			}
			break;
		case 0x4089: // wave write, master volume
			m_wave_write = BIT(data, 7);
			m_mvol = data & 3;
			break;
		case 0x408a: // envelope speed
			m_env_spd = data;
			break;
	}
}

//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void rp2c33_sound_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); i++)
	{
		m_output = 0;
		if (!m_env_halt && !m_wave_halt)
		{
			exec_vol_env();
			exec_mod_env();
		}
		exec_mod();
		exec_wave();
		/* Update the buffers */
		stream.put_int(0, i, m_output * m_mvol_table[m_mvol], 32768);
	}
}

//-------------------------------------------------
//  exec_vol_env - execute volume envelope
//-------------------------------------------------

inline void rp2c33_sound_device::exec_vol_env()
{
	if (!m_vol_env_disable)
	{
		const u32 period = ((m_vol_env_spd + 1) * (m_env_spd + 1)) << 3; // input clock / ((overall speed + 1) * (volume envelope speed + 1) * 8)
		if (++m_vol_env_clk > period)
		{
			// clock the envelope
			if (m_vol_env_mode)
			{
				if (m_vol_env_out < 32) ++m_vol_env_out;
			}
			else
			{
				if (m_vol_env_out >  0) --m_vol_env_out;
			}
			m_vol_env_clk = 0;
		}
	}
}

//-------------------------------------------------
//  exec_mod_env - execute modulator envelope
//-------------------------------------------------

inline void rp2c33_sound_device::exec_mod_env()
{
	if (!m_mod_env_disable)
	{
		const u32 period = ((m_mod_env_spd + 1) * (m_env_spd + 1)) << 3; // input clock / ((overall speed + 1) * (modulator envelope speed + 1) * 8)
		if (++m_mod_env_clk > period)
		{
			// clock the envelope
			if (m_mod_env_mode)
			{
				if (m_mod_env_out < 32) ++m_mod_env_out;
			}
			else
			{
				if (m_mod_env_out >  0) --m_mod_env_out;
			}
			m_mod_env_clk = 0;
		}
	}
}

//-------------------------------------------------
//  exec_mod - execute modulator table
//-------------------------------------------------

inline void rp2c33_sound_device::exec_mod()
{
	if (!m_mod_halt)
	{
		m_mod_acc += m_mod_freq; // input clock * frequency / 65536
		while (m_mod_acc > 0xffff)
		{
			int val = m_mod_table[((m_mod_addr++) >> 1) & 0x1f] & 7;
			m_mod_acc -= (1 << 16);
			if (val == 4)
				m_mod_pos = 0;
			else
				m_mod_pos = (m_mod_pos + mod_inc[val]) & 0x7f;
		}
	}
}

//-------------------------------------------------
//  exec_wave - execute wavetable output
//-------------------------------------------------

inline void rp2c33_sound_device::exec_wave()
{
	if (!m_wave_halt)
	{
		int mod = 0;
		if (m_mod_env_out != 0) // skip if modulator off
		{
			// convert mod_pos to 7-bit signed
			int pos = (m_mod_pos & 0x3f) - (m_mod_pos & 0x40);

			// multiply pos by gain,
			// shift off 4 bits but with odd "rounding" behaviour
			int temp = pos * m_mod_env_out;
			int rem = temp & 0x0f;
			temp >>= 4;
			if ((rem > 0) && ((temp & 0x80) == 0))
			{
				if (pos < 0) temp -= 1;
				else         temp += 2;
			}

			// wrap if range is exceeded
			while (temp >= 192) temp -= 256;
			while (temp <  -64) temp += 256;

			// multiply result by pitch,
			// shift off 6 bits, round to nearest
			temp = m_wave_freq * temp;
			rem = temp & 0x3f;
			temp >>= 6;
			if (rem >= 32) temp += 1;

			mod = temp;
		}

		// accumulate
		m_wave_acc += std::max<int>(0, m_wave_freq + mod); // input clock * (frequency + modulator output) / 65536
		m_wave_addr += m_wave_acc >> 16;
		m_wave_acc &= 0xffff;
	}

	// calculate output
	if (!m_wave_write)
		m_output = (m_wave[m_wave_addr & 0x3f] - 0x20) * std::min<int>(32, m_vol_env_out);

}
