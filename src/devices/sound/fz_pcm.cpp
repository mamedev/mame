// license:BSD-3-Clause
// copyright-holders:Devin Acker

/*
    Casio FZ series PCM

    This hardware actually comprises two main gate arrays (GAA and GAB)
    which each handle about half of the address generation & timing logic
    for 8 PCM voices. An additional gate array (GAX) demultiplexes the
    sample RAM output for each voice.

    TODO:
    - crossfade loop support
    - sampling (writes 16-bit mic/line input to sample RAM)
*/

#include "emu.h"

#include "fz_pcm.h"

#include <climits>

DEFINE_DEVICE_TYPE(FZ_PCM, fz_pcm_device, "fz_pcm", "Casio FZ PCM")

fz_pcm_device::fz_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, FZ_PCM, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_ram_config("ram", ENDIANNESS_LITTLE, 16, 21, -1)
{
}

/**************************************************************************/
void fz_pcm_device::device_start()
{
	m_stream = stream_alloc(1, 8, clock() / CLOCKS_PER_SAMPLE);

	m_irq_timer = timer_alloc(FUNC(fz_pcm_device::timer_tick), this);

	space().specific(m_ram);

	save_item(NAME(m_gaa_param));
	save_item(NAME(m_gab_param));
	save_item(NAME(m_gaa_cmd));
	save_item(NAME(m_gab_cmd));

	save_item(NAME(m_irq_stat));

	save_item(STRUCT_MEMBER(m_voices, m_flags));

	save_item(STRUCT_MEMBER(m_voices, m_addr_start));
	save_item(STRUCT_MEMBER(m_voices, m_addr_end));
	save_item(STRUCT_MEMBER(m_voices, m_loop_start));
	save_item(STRUCT_MEMBER(m_voices, m_loop_end));
	save_item(STRUCT_MEMBER(m_voices, m_loop_start_fine));
	save_item(STRUCT_MEMBER(m_voices, m_loop_len));
	save_item(STRUCT_MEMBER(m_voices, m_loop_trace));
	save_item(STRUCT_MEMBER(m_voices, m_loop_xfade));

	save_item(STRUCT_MEMBER(m_voices, m_pitch));
	save_item(STRUCT_MEMBER(m_voices, m_addr));
	save_item(STRUCT_MEMBER(m_voices, m_addr_frac));

	save_item(STRUCT_MEMBER(m_voices, m_sample));
	save_item(STRUCT_MEMBER(m_voices, m_sample_last));
}

/**************************************************************************/
void fz_pcm_device::device_reset()
{
	m_gaa_param[0] = m_gaa_param[1] = m_gaa_param[2] = 0;
	m_gab_param[0] = m_gab_param[1] = 0;
	m_gaa_cmd = m_gab_cmd = 0;
	m_voice_mask = 0;

	m_irq_stat = 0;
	m_irq_timer->adjust(attotime::never);
	m_irq_cb(0);

	for (voice_t &v : m_voices)
		v = voice_t();
}

/**************************************************************************/
void fz_pcm_device::device_clock_changed()
{
	m_stream->set_sample_rate(clock() / CLOCKS_PER_SAMPLE);
	update_pending_irq();
}

/**************************************************************************/
device_memory_interface::space_config_vector fz_pcm_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_ram_config)
	};
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(fz_pcm_device::timer_tick)
{
	m_irq_cb(1);
}

/**************************************************************************/
void fz_pcm_device::sound_stream_update(sound_stream &stream)
{
	for (int s = 0; s < stream.samples(); s++)
	{
		for (int i = 0; i < 8; i++)
		{
			voice_t &v = m_voices[i];

			if (BIT(v.m_flags, FLAG_PLAY))
			{
				v.m_addr_frac += v.m_pitch;
				if (v.m_addr_frac >= (1 << ADDR_FRAC_SHIFT))
				{
					v.m_sample_last = v.m_sample;
					v.m_sample = (s16)m_ram.read_word(v.m_addr);

					if (v.update())
						m_irq_stat |= (1 << i);
				}
			}

			s16 sample = 0;
			if (BIT(v.m_flags, FLAG_OUTPUT))
			{
				const u8 frac = BIT(v.m_addr_frac, ADDR_FRAC_SHIFT - 3, 3);
				sample = v.m_sample_last + (s32(v.m_sample - v.m_sample_last) * frac / 8);
			}

			stream.put_int_clamp(i, s, sample, 1 << 15);
		}
	}
}

/**************************************************************************/
bool fz_pcm_device::voice_t::update()
{
	bool looped = false;

	while (m_addr_frac >= (1 << ADDR_FRAC_SHIFT))
	{
		m_addr_frac -= (1 << ADDR_FRAC_SHIFT);

		if (!BIT(m_flags, FLAG_REVERSE))
		{
			if (m_addr < m_addr_end)
			{
				m_addr++;

				if (BIT(m_flags, FLAG_LOOP) && m_addr >= m_loop_end)
				{
					looped = BIT(m_flags, FLAG_INT);

					if (m_loop_trace)
					{
						// next=trace: continue as normal
						m_addr = m_loop_start + (m_addr - m_loop_end);
						m_addr_frac += m_loop_start_fine << (ADDR_FRAC_SHIFT - 8);
					}
					else
					{
						// next=skip: jump to exact loop start, stop further interrupts
						m_addr = m_loop_start;
						m_addr_frac = m_loop_start_fine << (ADDR_FRAC_SHIFT - 8);
						m_flags &= ~(1 << FLAG_INT);
					}
				}
			}
			else
			{
				m_flags &= ~(1 << FLAG_PLAY);
			}
		}
		else
		{
			if (m_addr > m_addr_end)
				m_addr--;
			else
				m_flags &= ~(1 << FLAG_PLAY);
		}
	}

	return looped;
}

/**************************************************************************/
bool fz_pcm_device::voice_t::calc_timeout(u32 &samples) const
{
	// don't update interrupt timer if this voice isn't set to interrupt
	if (!BIT(m_flags, FLAG_PLAY)
		|| !BIT(m_flags, FLAG_LOOP)
		|| !BIT(m_flags, FLAG_INT)
		|| BIT(m_flags, FLAG_REVERSE)
		|| !m_pitch)
	{
		return false;
	}

	if (m_addr >= m_loop_end)
	{
		// loop end is somehow in the past
		samples = 0;
	}
	else
	{
		// calculate number of output samples until loop end is reached
		const u64 to_loop = ((u64(m_loop_end - m_addr) << ADDR_FRAC_SHIFT) - m_addr_frac + m_pitch - 1) / m_pitch;
		if (to_loop < samples)
			samples = to_loop;
	}

	return true;
}

/**************************************************************************/
void fz_pcm_device::update_pending_irq()
{
	bool pending = false;
	u32 new_time = UINT_MAX;

	for (voice_t &v : m_voices)
		pending |= v.calc_timeout(new_time);

	if (pending)
		m_irq_timer->adjust(clocks_to_attotime((u64)new_time * CLOCKS_PER_SAMPLE));
	else
		m_irq_timer->adjust(attotime::never);
}

/**************************************************************************/
u16 fz_pcm_device::gaa_r(offs_t offset)
{
	offset &= 3;

	switch (offset & 3)
	{
	default:
		return m_gaa_param[offset & 3];

	case 3:
		return m_gaa_cmd;
	}
}

/**************************************************************************/
u16 fz_pcm_device::gab_r(offs_t offset)
{
	switch (offset & 3)
	{
	default:
		return m_gab_param[offset & 3];

	case 2:
		if (!machine().side_effects_disabled())
			m_stream->update();
		return m_irq_stat;

	case 3:
		return m_gab_cmd;
	}
}

/**************************************************************************/
void fz_pcm_device::gaa_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();

	switch (offset & 3)
	{
	default:
		COMBINE_DATA(&m_gaa_param[offset & 3]);
		break;

	case 3:
		COMBINE_DATA(&m_gaa_cmd);
		if (ACCESSING_BITS_8_15)
		{
			m_voice_mask = m_gaa_cmd & 0xff;

			switch (m_gaa_cmd >> 8)
			{
			case 0x40: voice_cmd(CMD_ADDR_START); break;
			case 0x41: voice_cmd(CMD_LOOP_START); break;
			case 0x42: voice_cmd(CMD_PITCH);      break;
			case 0x43: voice_cmd(CMD_LOOP_LEN);   break;
			case 0x45: voice_cmd(CMD_LOOP_XFADE); break;
			case 0x80: voice_cmd(CMD_GET_ADDR);   break;

			default:
				logerror("%s: unknown GAA cmd %04x (param %04x %04x %04x)\n", machine().describe_context(),
					m_gaa_cmd, m_gaa_param[0], m_gaa_param[1], m_gaa_param[2]);
				break;
			}

			update_pending_irq();
		}
		break;
	}
}

/**************************************************************************/
void fz_pcm_device::gab_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_stream->update();

	switch (offset & 3)
	{
	case 0:
	case 1:
		COMBINE_DATA(&m_gab_param[offset & 3]);
		break;

	case 3:
		COMBINE_DATA(&m_gab_cmd);
		if (ACCESSING_BITS_8_15)
		{
			m_voice_mask = m_gab_cmd & 0xff;

			switch (m_gab_cmd >> 8)
			{
			case 0x20: /* TODO: recording rate  */ break;
			case 0x28: /* TODO: recording cue   */ break;
			case 0x30: /* TODO: recording start */ break;
			case 0x38: /* TODO: recording stop  */ break;
			case 0x40: voice_cmd(CMD_ADDR_END);   break;
			case 0x41: voice_cmd(CMD_LOOP_END);   break;
			case 0x42: voice_cmd(CMD_FLAG_SET);   break;
			case 0x43: voice_cmd(CMD_FLAG_CLR);   break;
			case 0x44: voice_cmd(CMD_LOOP_TRACE); break;

			case 0x46:
				m_irq_stat &= m_gab_param[0];
				if (!m_irq_stat)
					m_irq_cb(0);
				break;

			case 0x82:
				m_gab_param[0] = 0;
				voice_cmd(CMD_GET_STATUS);
				break;

			default:
				logerror("%s: unknown GAB cmd %04x (param %04x %04x)\n", machine().describe_context(),
					m_gab_cmd, m_gab_param[0], m_gab_param[1]);
				break;
			}

			update_pending_irq();
		}
		break;
	}
}

/**************************************************************************/
void fz_pcm_device::voice_cmd(unsigned cmd)
{
	for (int i = 0; i < 8; i++)
	{
		if (!BIT(m_voice_mask, i))
			continue;

		voice_t &v = m_voices[i];

		switch (cmd)
		{
		case CMD_ADDR_START:
			v.m_addr_start = m_gaa_param[1] | (m_gaa_param[2] << 16);
			v.m_addr = v.m_addr_start;
			v.m_addr_frac = 0;
			v.m_sample = v.m_sample_last = 0;
			break;

		case CMD_ADDR_END:
			v.m_addr_end = m_gab_param[0] | (m_gab_param[1] << 16);
			break;

		case CMD_LOOP_START:
			v.m_loop_start = m_gaa_param[0] | (m_gaa_param[1] << 16);
			v.m_loop_start_fine = m_gaa_param[2] & 0xff;
			break;

		case CMD_LOOP_END:
			v.m_loop_end = m_gab_param[0] | (m_gab_param[1] << 16);
			break;

		case CMD_LOOP_LEN:
			// TODO: what is this used for? it only has half the resolution of the actual loop start/end points
			v.m_loop_len = (m_gaa_param[0] << 1) | (m_gaa_param[1] << 17);
			break;

		case CMD_LOOP_TRACE:
			v.m_loop_trace = m_gab_param[0] & 1;
			break;

		case CMD_LOOP_XFADE:
			v.m_loop_xfade = m_gaa_param[0];
			break;

		case CMD_PITCH:
			v.m_pitch = m_gaa_param[0];
			break;

		case CMD_FLAG_SET:
			v.m_flags |= m_gab_param[0];
			break;

		case CMD_FLAG_CLR:
			v.m_flags &= ~m_gab_param[0];
			break;

		case CMD_GET_ADDR:
			m_gaa_param[1] = v.m_addr;
			m_gaa_param[2] = v.m_addr >> 16;
			break;

		case CMD_GET_STATUS:
			// TODO: verify
			m_gab_param[0] |= v.m_flags;
			break;
		}
	}
}
