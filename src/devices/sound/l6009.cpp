// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
	Akai L6009

	This is the sound hardware used in the S1000 and S1100 samplers. It actually comprises two chips
	running in tandem: ITP-L6009 (tone generator) and FLR-L6009 (filters and mixer).

	TODO:
	- verify volume and filter envelope behavior (ENV_SHIFT is basically a complete guess, and volume
	  is probably not entirely linear)
	- sample interpolation: 6-point sinc interpolation using one of four coefficient tables in ROM
	  (more info: https://www.kvraudio.com/forum/viewtopic.php?t=628777)
***************************************************************************/

#include "emu.h"
#include "l6009.h"

DEFINE_DEVICE_TYPE(L6009, l6009_device, "l6009", "Akai L6009")

l6009_device::l6009_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, L6009, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_sample_clock(*this, "sample_clock")
	, m_ram_config("ram", ENDIANNESS_LITTLE, 16, 24, -1)
	, m_rom_config("rom", ENDIANNESS_LITTLE, 16, 16, -1)
{
}

/**************************************************************************/
void l6009_device::device_add_mconfig(machine_config &config)
{
	device_t::device_add_mconfig(config);

	CLOCK(config, m_sample_clock, DERIVED_CLOCK(1, CLOCKS_PER_SAMPLE));
}

/**************************************************************************/
void l6009_device::device_start()
{
	m_stream = stream_alloc(0, OUT_COUNT, clock() / CLOCKS_PER_SAMPLE);

	space(AS_RAM).specific(m_ram);
	space(AS_ROM).cache(m_rom);

	save_item(NAME(m_ram_addr));
	save_item(NAME(m_temp));

	save_item(STRUCT_MEMBER(m_voices, m_addr));
	save_item(STRUCT_MEMBER(m_voices, m_addr_frac));
	save_item(STRUCT_MEMBER(m_voices, m_loop));
	save_item(STRUCT_MEMBER(m_voices, m_loop_frac));
	save_item(STRUCT_MEMBER(m_voices, m_loop_end));
	save_item(STRUCT_MEMBER(m_voices, m_pitch));
	save_item(STRUCT_MEMBER(m_voices, m_pitch_unk));

	save_item(STRUCT_MEMBER(m_voices, m_output_sel));
	save_item(STRUCT_MEMBER(m_voices, m_stereo));
	save_item(STRUCT_MEMBER(m_voices, m_volume_env));
	save_item(STRUCT_MEMBER(m_voices, m_volume_set));
	save_item(STRUCT_MEMBER(m_voices, m_volume));
	save_item(STRUCT_MEMBER(m_voices, m_filter_env));
	save_item(STRUCT_MEMBER(m_voices, m_filter_set));
	save_item(STRUCT_MEMBER(m_voices, m_filter));
}

/**************************************************************************/
void l6009_device::device_reset()
{
	m_ram_addr = 0;
	m_temp = 0;

	for (voice_t &v : m_voices)
		v = voice_t();
}

/**************************************************************************/
void l6009_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
}

/**************************************************************************/
device_memory_interface::space_config_vector l6009_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_ROM, &m_rom_config),
		std::make_pair(AS_RAM, &m_ram_config)
	};
}

/**************************************************************************/
void l6009_device::sound_stream_update(sound_stream &stream)
{
	for (int s = 0; s < stream.samples(); s++)
	{
		s32 mono_out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		s32 stereo_out[2] = {0, 0};
		s32 effect_out = 0;

		for (voice_t &v : m_voices)
		{
			// TODO: interpolation
			s32 sample = (s16)m_ram.read_word(v.m_addr);

			v.m_addr_frac += v.m_pitch;
			v.m_addr += (v.m_addr_frac >> 12);
			v.m_addr_frac &= 0xfff;

			if (v.m_addr > v.m_loop_end)
			{
				v.m_addr -= (v.m_loop_end + 1);
				v.m_addr_frac += v.m_loop_frac;
				v.m_addr += (v.m_loop + (v.m_addr_frac >> 12));
				v.m_addr_frac &= 0xfff;
			}

			update_env(v.m_volume, v.m_volume_set, v.m_volume_env);
			update_env(v.m_filter, v.m_filter_set, v.m_filter_env);

			// TODO: convert lin->log? (or does the FW do this scaling with levels already?)
			sample = s32(sample * (v.m_volume >> ENV_SHIFT)) >> 15;

			// -18dB filter with no resonance and only one settable coefficient
			// (most likely just three -6dB first order filters in series)
			for (auto &filter : v.m_filter_out)
			{
				filter += s32((sample - filter) * (v.m_filter >> ENV_SHIFT)) >> 15;
				sample = filter;
			}

			for (int i = 0; i < 2; i++)
				stereo_out[i] += s32(sample * v.m_stereo[i]) >> 8;

			if (BIT(v.m_output_sel, 12))
				mono_out[v.m_output_sel & 7] += sample;

			effect_out += s32(sample * (v.m_output_sel >> 13)) >> 3;
		}

		for (int i = 0; i < 8; i++)
			stream.put_int_clamp(OUT_MONO + i, s, mono_out[i], 1 << 19);

		stream.put_int_clamp(OUT_STEREO_L, s, stereo_out[0], 1 << 19);
		stream.put_int_clamp(OUT_STEREO_R, s, stereo_out[1], 1 << 19);
		stream.put_int_clamp(OUT_EFFECT, s, effect_out, 1 << 19);
	}
}

/**************************************************************************/
void l6009_device::update_env(u32 &val, u32 dest, u16 rate)
{
	if (val < dest)
	{
		val += rate;
		if (val > dest)
			val = dest;
	}
	else if (val > dest)
	{
		if (dest + rate > val)
			val = dest;
		else
			val -= rate;
	}
}

/**************************************************************************/
u16 l6009_device::itp_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_stream->update();

	u32 val = 0;
	const voice_t &voice = m_voices[offset & 0xf];

	// address regs are expected to be readable, assume the others are too
	switch (BIT(offset, 4, 3))
	{
	case 0: // address low
		val = voice.m_addr_frac | (voice.m_addr << 12);
		break;

	case 1: // address high
		val = voice.m_addr >> 6;
		break;

	case 2: // pitch
		val = voice.m_pitch | (voice.m_pitch_unk << 16);
		break;

	case 3: // loop endpoint (high bits only)
		val = (~voice.m_loop_end) >> 6;
		break;

	case 4: // loop point low
		val = voice.m_loop_frac | (voice.m_loop << 12);
		break;

	case 5: // loop point high
		val = voice.m_loop >> 6;
		break;

	case 7: // ram access
		switch (BIT(offset, 2, 2))
		{
		case 0:
			return m_temp;

		case 1:
			return m_temp >> 16;

		case 2:
			val = m_ram_addr;
			if (!machine().side_effects_disabled())
				m_ram_addr++;
			return m_ram.read_word(val);
		}
		break;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: ITP reg %03x read\n", machine().describe_context(), offset << 1);
		break;
	}

	// upper bits of 18-bit address regs are read from the same address used to set upper bits of RAM address
	if (!machine().side_effects_disabled())
		m_temp = val & 0x3ffff;

	return val;
}

/**************************************************************************/
u16 l6009_device::flr_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_stream->update();
	voice_t &voice = m_voices[offset & 0xf];

	switch (BIT(offset, 4, 4))
	{
	case 0x3:
		return voice.m_output_sel;
	case 0x4:
		return voice.m_volume_env;
	case 0x6:
		return voice.m_filter_env;
	case 0xb:
		return voice.m_stereo[1] | (voice.m_stereo[0] << 8);
	case 0xc:
		return voice.m_volume_set >> ENV_SHIFT;
	case 0xd:
		return voice.m_filter >> ENV_SHIFT;
	case 0xe:
		return voice.m_filter_set >> ENV_SHIFT;
	case 0xf:
		return voice.m_volume >> ENV_SHIFT;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: FLR reg %03x read\n", machine().describe_context(), offset << 1);
		break;
	}

	return 0;
}

/**************************************************************************/
void l6009_device::itp_w(offs_t offset, u16 data)
{
	m_stream->update();

	const u32 val = data | (BIT(offset, 7, 2) << 16);
	voice_t &voice = m_voices[offset & 0xf];

	switch (BIT(offset, 4, 3))
	{
	case 0: // address low
		voice.m_addr &= 0xffffc0;
		voice.m_addr |= (val >> 12);
		voice.m_addr_frac = val & 0xfff;
		break;

	case 1: // address high
		voice.m_addr &= 0x00003f;
		voice.m_addr |= (val << 6);
		break;

	case 2: // pitch
		voice.m_pitch = val;
		voice.m_pitch_unk = val >> 16; // probably interpolation table selection
		break;

	case 3: // loop endpoint (upper bits only and inverted for some reason)
		voice.m_loop_end = (val << 6) ^ 0xffffff;
		break;

	case 4: // loop point low
		voice.m_loop &= 0xffffc0;
		voice.m_loop |= (val >> 12);
		voice.m_loop_frac = val & 0xfff;
		break;

	case 5: // loop point high
		voice.m_loop &= 0x00003f;
		voice.m_loop |= (val << 6);
		break;

	case 7: // ram access
		switch (BIT(offset, 2, 2))
		{
		case 0:
			m_ram_addr = (m_ram_addr & 0xffff0000) | data;
			break;

		case 1:
			m_ram_addr = (m_ram_addr & 0x0000ffff) | (data << 16);
			break;

		case 2:
			m_ram.write_word(m_ram_addr++, data);
			break;
		}
		break;

	default:
		logerror("%s: ITP reg %03x write %04x\n", machine().describe_context(), offset << 1, data);
		break;
	}
}

/**************************************************************************/
void l6009_device::flr_w(offs_t offset, u16 data)
{
	m_stream->update();

	voice_t &voice = m_voices[offset & 0xf];

	switch (BIT(offset, 4, 4))
	{
	case 0x3:
		voice.m_output_sel = data;
		break;

	case 0x4:
		voice.m_volume_env = data;
		break;

	case 0x6:
		voice.m_filter_env = data;
		break;

	case 0xb:
		voice.m_stereo[0] = data >> 8;
		voice.m_stereo[1] = data;
		break;

	case 0xc:
		voice.m_volume_set = data << ENV_SHIFT;
		break;

	case 0xd:
		voice.m_filter = data << ENV_SHIFT;
		break;

	case 0xe:
		voice.m_filter_set = data << ENV_SHIFT;
		break;

	case 0xf:
		voice.m_volume = data << ENV_SHIFT;
		break;

	case 0x8:
	case 0x9:
	case 0xa:
		// normally only set to zero, possibly to clear the filter outputs
		if (!data)
			break;
		[[fallthrough]];
	default:
		logerror("%s: FLR reg %03x write %04x\n", machine().describe_context(), offset << 1, data);
		break;
	}
}
