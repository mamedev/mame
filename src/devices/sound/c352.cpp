// license:BSD-3-Clause
// copyright-holders:R. Belmont, superctr
/*
    c352.c - Namco C352 custom PCM chip emulation
    v2.0
    By R. Belmont
    Rewritten and improved by superctr
    Additional code by cync and the hoot development team

    Thanks to Cap of VivaNonno for info and The_Author for preliminary reverse-engineering

    Chip specs:
    32 voices
    Supports 8-bit linear and 8-bit muLaw samples
    Output: digital, 16 bit, 4 channels
    Output sample rate is the input clock / (288 * 2).
 */

#include "emu.h"
#include "c352.h"
#include "wavwrite.h"

//#define VERBOSE 1
#include "logmacro.h"

#define C352_LOG_PCM	(0)

#if C352_LOG_PCM
#include <map>
static std::map<uint32_t, bool> s_found_pcm;
#endif

// device type definition
DEFINE_DEVICE_TYPE(C352, c352_device, "c352", "Namco C352")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c352_device - constructor
//-------------------------------------------------

c352_device::c352_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, C352, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this, 24)
	, m_stream(nullptr)
{
}

//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void c352_device::rom_bank_updated()
{
	m_stream->update();
}

void c352_device::fetch_sample(c352_voice_t& v)
{
	v.last_sample = v.sample;

	if (v.flags & C352_FLG_NOISE)
	{
		m_random = (m_random >> 1) ^ ((-(m_random & 1)) & 0xfff6);
		v.sample = m_random;
	}
	else
	{
		int8_t s = (int8_t)read_byte(v.pos);

		if (v.flags & C352_FLG_MULAW)
			v.sample = m_mulawtab[s & 0xff];
		else
			v.sample = s << 8;

		uint16_t pos = v.pos & 0xffff;

		if ((v.flags & C352_FLG_LOOP) && v.flags & C352_FLG_REVERSE)
		{
			// backwards>forwards
			if ((v.flags & C352_FLG_LDIR) && pos == v.wave_loop)
				v.flags &= ~C352_FLG_LDIR;
			// forwards>backwards
			else if (!(v.flags & C352_FLG_LDIR) && pos == v.wave_end)
				v.flags |= C352_FLG_LDIR;

			v.pos += (v.flags & C352_FLG_LDIR) ? -1 : 1;
		}
		else if (pos == v.wave_end)
		{
			if ((v.flags & C352_FLG_LINK) && (v.flags & C352_FLG_LOOP))
			{
				v.pos = (v.wave_start << 16) | v.wave_loop;
				v.flags |= C352_FLG_LOOPHIST;
			}
			else if (v.flags & C352_FLG_LOOP)
			{
				v.pos = (v.pos & 0xff0000) | v.wave_loop;
				v.flags |= C352_FLG_LOOPHIST;
			}
			else
			{
				v.flags |= C352_FLG_KEYOFF;
				v.flags &= ~C352_FLG_BUSY;
				v.sample = 0;
			}
		}
		else
		{
			v.pos += (v.flags & C352_FLG_REVERSE) ? -1 : 1;
		}
	}
}

void c352_device::ramp_volume(c352_voice_t &v, int ch, uint8_t val)
{
	int16_t vol_delta = v.curr_vol[ch] - val;
	if (vol_delta != 0)
		v.curr_vol[ch] += (vol_delta > 0) ? -1 : 1;
}

void c352_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *buffer_fl = outputs[0];
	stream_sample_t *buffer_fr = outputs[1];
	stream_sample_t *buffer_rl = outputs[2];
	stream_sample_t *buffer_rr = outputs[3];

	for (int i = 0; i < samples; i++)
	{
		long out[4] = { 0, 0, 0, 0 };

		for (int j = 0; j < 32; j++)
		{
			c352_voice_t &v = m_c352_v[j];
			int16_t s = 0;

			if (v.flags & C352_FLG_BUSY)
			{
				int32_t next_counter = v.counter + v.freq;

				if (next_counter & 0x10000)
				{
					fetch_sample(v);
				}

				if ((next_counter ^ v.counter) & 0x18000)
				{
					ramp_volume(v, 0, v.vol_f >> 8);
					ramp_volume(v, 1, v.vol_f & 0xff);
					ramp_volume(v, 2, v.vol_r >> 8);
					ramp_volume(v, 3, v.vol_r & 0xff);
				}

				v.counter = next_counter & 0xffff;

				s = v.sample;

				// Interpolate samples
				if ((v.flags & C352_FLG_FILTER) == 0)
					s = v.last_sample + (v.counter * (v.sample - v.last_sample) >> 16);
			}

			// Left
			out[0] += (((v.flags & C352_FLG_PHASEFL) ? -s : s) * v.curr_vol[0]) >> 8;
			out[2] += (((v.flags & C352_FLG_PHASERL) ? -s : s) * v.curr_vol[2]) >> 8;

			// Right
			out[1] += (((v.flags & C352_FLG_PHASEFR) ? -s : s) * v.curr_vol[1]) >> 8;
			out[3] += (((v.flags & C352_FLG_PHASEFR) ? -s : s) * v.curr_vol[3]) >> 8;
		}

		*buffer_fl++ = (int16_t)(out[0] >> 3);
		*buffer_fr++ = (int16_t)(out[1] >> 3);
		*buffer_rl++ = (int16_t)(out[2] >> 3);
		*buffer_rr++ = (int16_t)(out[3] >> 3);
	}
}

uint16_t c352_device::read_reg16(unsigned long address)
{
	m_stream->update();

	const int reg_map[8] =
	{
		offsetof(c352_voice_t, vol_f) / sizeof(uint16_t),
		offsetof(c352_voice_t, vol_r) / sizeof(uint16_t),
		offsetof(c352_voice_t, freq) / sizeof(uint16_t),
		offsetof(c352_voice_t, flags) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_bank) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_start) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_end) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_loop) / sizeof(uint16_t),
	};

	if (address < 0x100)
		return *((uint16_t*)&m_c352_v[address / 8] + reg_map[address % 8]);
	else if (address == 0x200)
		return m_control;
	else
		return 0;

	return 0;
}

void c352_device::write_reg16(unsigned long address, unsigned short val)
{
	m_stream->update();

	const int reg_map[8] =
	{
		offsetof(c352_voice_t, vol_f) / sizeof(uint16_t),
		offsetof(c352_voice_t, vol_r) / sizeof(uint16_t),
		offsetof(c352_voice_t, freq) / sizeof(uint16_t),
		offsetof(c352_voice_t, flags) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_bank) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_start) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_end) / sizeof(uint16_t),
		offsetof(c352_voice_t, wave_loop) / sizeof(uint16_t),
	};

	if (address < 0x100)
	{
		*((uint16_t*)&m_c352_v[address / 8] + reg_map[address % 8]) = val;
	}
	else if (address == 0x200)
	{
		m_control = val;
		logerror("C352 control register write: %04x\n",val);
	}
	else if (address == 0x202) // execute keyons/keyoffs
	{
		for (int i = 0; i < 32; i++)
		{
			if (m_c352_v[i].flags & C352_FLG_KEYON)
			{
				m_c352_v[i].pos = (m_c352_v[i].wave_bank << 16) | m_c352_v[i].wave_start;

				m_c352_v[i].sample = 0;
				m_c352_v[i].last_sample = 0;
				m_c352_v[i].counter = 0xffff;

				m_c352_v[i].flags |= C352_FLG_BUSY;
				m_c352_v[i].flags &= ~(C352_FLG_KEYON | C352_FLG_LOOPHIST);

				m_c352_v[i].curr_vol[0] = m_c352_v[i].curr_vol[1] = 0;
				m_c352_v[i].curr_vol[2] = m_c352_v[i].curr_vol[3] = 0;

#if C352_LOG_PCM
				if (!(m_c352_v[i].flags & C352_FLG_NOISE))
				{
					std::map<uint32_t, bool>::iterator iter = s_found_pcm.find(m_c352_v[i].pos);
					if (iter != s_found_pcm.end())
					{
						return;
					}

					s_found_pcm[m_c352_v[i].pos] = true;

					char filebuf[256];
					snprintf(filebuf, 256, "c352_%08x.wav", m_c352_v[i].pos);
					wav_file *file = wav_open(filebuf, m_stream->sample_rate(), 1);
					if (file != nullptr)
					{
						c352_voice_t &v = m_c352_v[i];
						uint32_t pos = v.pos;
						uint32_t flags = v.flags;
						uint32_t counter = v.counter;
						int16_t sample = 0;

						while (pos != v.wave_end && !(flags & C352_FLG_KEYOFF))
						{
							int32_t next_counter = counter + v.freq;

							if (next_counter & 0x10000)
							{
								counter = next_counter & 0xffff;

								int8_t s = (int8_t)read_byte(pos);

								if (v.flags & C352_FLG_MULAW)
									sample = m_mulawtab[s & 0xff];
								else
									sample = s << 8;

								uint16_t subpos = pos & 0xffff;

								if ((flags & C352_FLG_LOOP) && flags & C352_FLG_REVERSE)
								{
									// backwards>forwards
									if ((flags & C352_FLG_LDIR) && subpos == v.wave_loop)
										flags &= ~C352_FLG_LDIR;
									// forwards>backwards
									else if (!(flags & C352_FLG_LDIR) && subpos == v.wave_end)
										flags |= C352_FLG_LDIR;

									pos += (flags & C352_FLG_LDIR) ? -1 : 1;
								}
								else if (subpos == v.wave_end)
								{
									if ((flags & C352_FLG_LINK) && (flags & C352_FLG_LOOP))
									{
										pos = (v.wave_start << 16) | v.wave_loop;
										flags |= C352_FLG_LOOPHIST;
									}
									else if (flags & C352_FLG_LOOP)
									{
										pos = (pos & 0xff0000) | v.wave_loop;
										if (flags & C352_FLG_LOOPHIST)
										{
											flags |= C352_FLG_KEYOFF;
										}
										flags |= C352_FLG_LOOPHIST;
									}
									else
									{
										flags |= C352_FLG_KEYOFF;
										flags &= ~C352_FLG_BUSY;
										sample = 0;
									}
								}
								else
								{
									pos += (flags & C352_FLG_REVERSE) ? -1 : 1;
								}
							}

							counter = next_counter & 0xffff;

							wav_add_data_16(file, &sample, 1);
						}

						wav_close(file);
					}
				}
#endif
			}
			if (m_c352_v[i].flags & C352_FLG_KEYOFF)
			{
				m_c352_v[i].flags &= ~(C352_FLG_BUSY | C352_FLG_KEYOFF);
				m_c352_v[i].counter = 0xffff;
			}
		}
	}
}

void c352_device::device_clock_changed()
{
	m_sample_rate_base = clock() / m_divider;
	if (m_stream != nullptr)
		m_stream->set_sample_rate(m_sample_rate_base);
	else
		m_stream = machine().sound().stream_alloc(*this, 0, 4, m_sample_rate_base);
}

void c352_device::device_start()
{
	m_sample_rate_base = clock() / m_divider;

	m_stream = machine().sound().stream_alloc(*this, 0, 4, m_sample_rate_base);

	// generate mulaw table (Output similar to namco's VC emulator)
	int j = 0;
	for (int i = 0; i < 128; i++)
	{
		m_mulawtab[i] = j << 5;
		if (i < 16)
			j += 1;
		else if (i < 24)
			j += 2;
		else if (i < 48)
			j += 4;
		else if (i < 100)
			j += 8;
		else
			j += 16;
	}
	for (int i = 0; i < 128; i++)
		m_mulawtab[i + 128] = (~m_mulawtab[i]) & 0xffe0;

	// register save state info
	for (int i = 0; i < 32; i++)
	{
		save_item(NAME(m_c352_v[i].pos), i);
		save_item(NAME(m_c352_v[i].counter), i);
		save_item(NAME(m_c352_v[i].sample), i);
		save_item(NAME(m_c352_v[i].last_sample), i);
		save_item(NAME(m_c352_v[i].vol_f), i);
		save_item(NAME(m_c352_v[i].vol_r), i);
		save_item(NAME(m_c352_v[i].curr_vol), i);
		save_item(NAME(m_c352_v[i].freq), i);
		save_item(NAME(m_c352_v[i].flags), i);
		save_item(NAME(m_c352_v[i].wave_bank), i);
		save_item(NAME(m_c352_v[i].wave_start), i);
		save_item(NAME(m_c352_v[i].wave_end), i);
		save_item(NAME(m_c352_v[i].wave_loop), i);
	}
	save_item(NAME(m_random));
	save_item(NAME(m_control));
}

void c352_device::device_reset()
{
	// clear all channels states
	memset(m_c352_v, 0, sizeof(c352_voice_t) * 32);

	// init noise generator
	m_random = 0x1234;
	m_control = 0;
}

READ16_MEMBER( c352_device::read )
{
	return read_reg16(offset);
}

WRITE16_MEMBER( c352_device::write )
{
	if (mem_mask == 0xffff)
	{
		write_reg16(offset, data);
	}
	else
	{
		logerror("C352: byte-wide write unsupported at this time!\n");
	}
}
