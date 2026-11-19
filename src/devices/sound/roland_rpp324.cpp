// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Roland RPP-324 RDAC phrase player, as used on System Board Y2

    The register protocol comes from the games' BDSND SDK.  The codec and the
    phrase directory were reconstructed independently and checked against
    PSP/PS2 assets.  The sequencer reads word-swapped, checksummed packets
    stored in the NOR right after the phrase directory.

    TODO: gain/pan law, fade and status pulse time units, FC delay clock,
    Late modes, filters, stereo effect, FIFO depth, command busy latency
*/

#include "emu.h"
#include "roland_rpp324.h"

#define LOG_COMMAND (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_COMMAND)
#include "logmacro.h"

#define LOGCOMMAND(...) LOGMASKED(LOG_COMMAND, __VA_ARGS__)

DEFINE_DEVICE_TYPE(ROLAND_RPP324, roland_rpp324_device, "roland_rpp324", "Roland RPP-324 RDAC")

roland_rpp324_device::roland_rpp324_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ROLAND_RPP324, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_rom(*this, DEVICE_SELF)
	, m_stream(nullptr)
	, m_lowcut{}
	, m_highcut{}
	, m_lowgain{}
	, m_effect{}
	, m_total_effect(0)
	, m_mix_c(false)
	, m_mono_c(false)
	, m_control(0)
	, m_handshake(0)
	, m_packet{}
	, m_packet_pos(0)
	, m_sequence_table(0)
	, m_sequence_count(0)
{
}

void roland_rpp324_device::ramp::set(u8 level, u8 fade)
{
	// provisional linear law, 10 ms per fade step; 0 and the SDK's 127 default are immediate until measured
	target = level;
	remaining = (fade && fade != 127) ? u32(fade) * 441 : 0;
	delta = remaining ? (target - value) / remaining : 0;
	if (!remaining)
	{
		value = target;
	}
}

double roland_rpp324_device::ramp::tick()
{
	if (remaining && !--remaining)
	{
		value = target;
	}
	else if (remaining)
	{
		value += delta;
	}
	return value;
}

void roland_rpp324_device::save_ramp(ramp &value, unsigned index)
{
	save_item(NAME(value.value), index);
	save_item(NAME(value.target), index);
	save_item(NAME(value.delta), index);
	save_item(NAME(value.remaining), index);
}

void roland_rpp324_device::device_start()
{
	m_stream = stream_alloc(0, 4, 44'100);

	// the sequence table follows the phrase directory and shares its halfword addressing; Higurashi has none
	roland_rdac::phrase first;
	m_sequence_table = roland_rdac::phrase_count(m_rom, m_rom.bytes()) * 10 + 4;
	if (roland_rdac::describe(m_rom, m_rom.bytes(), 0, first) && m_sequence_table + 4 <= first.start)
	{
		const u32 first_sequence = roland_rdac::read32(m_rom + m_sequence_table) >> 3;
		if (first_sequence > m_sequence_table && first_sequence < first.start && !((first_sequence - m_sequence_table) & 3))
		{
			m_sequence_count = std::min((first_sequence - m_sequence_table) / 4, 4096U);
		}
	}

	for (unsigned i = 0; i < VOICES; ++i)
	{
		roland_rdac::decoder &d = m_voice[i].decoder;
		save_item(NAME(d.cursor), i);
		save_item(NAME(d.end), i);
		save_item(NAME(d.limit), i);
		save_item(NAME(d.silence), i);
		save_item(NAME(d.previous), i);
		save_item(NAME(d.pcm), i);
		save_item(NAME(d.table), i);
		save_item(NAME(d.channels), i);
		save_item(NAME(d.index), i);
		save_item(NAME(d.failed), i);
	}
	save_item(STRUCT_MEMBER(m_voice, phrase));
	save_item(STRUCT_MEMBER(m_voice, queued));
	save_item(STRUCT_MEMBER(m_voice, queued_loops));
	save_item(STRUCT_MEMBER(m_voice, queued_late));
	save_item(STRUCT_MEMBER(m_voice, queue_count));
	save_item(STRUCT_MEMBER(m_voice, loops));
	save_item(STRUCT_MEMBER(m_voice, channels));
	save_item(STRUCT_MEMBER(m_voice, step));
	save_item(STRUCT_MEMBER(m_voice, phase));
	save_item(STRUCT_MEMBER(m_voice, late));
	save_item(STRUCT_MEMBER(m_voice, owner));
	save_item(STRUCT_MEMBER(m_voice, sequence));
	save_item(STRUCT_MEMBER(m_voice, playing));
	save_item(STRUCT_MEMBER(m_voice, paused));
	save_item(STRUCT_MEMBER(m_voice, release));
	save_item(STRUCT_MEMBER(m_voice, next_valid));
	save_item(STRUCT_MEMBER(m_voice, current));
	save_item(STRUCT_MEMBER(m_voice, next));

	unsigned ramp_index = 0;
	for (unsigned i = 0; i < VOICES; ++i)
	{
		save_ramp(m_gain[i], ramp_index++);
		save_ramp(m_pan[i], ramp_index++);
		for (ramp &r : m_route[i])
		{
			save_ramp(r, ramp_index++);
		}
	}
	for (ramp &r : m_master)
	{
		save_ramp(r, ramp_index++);
	}
	for (ramp &r : m_output)
	{
		save_ramp(r, ramp_index++);
	}

	save_item(STRUCT_MEMBER(m_sequence, start));
	save_item(STRUCT_MEMBER(m_sequence, cursor));
	save_item(STRUCT_MEMBER(m_sequence, end));
	save_item(STRUCT_MEMBER(m_sequence, delay));
	save_item(STRUCT_MEMBER(m_sequence, loops));
	save_item(STRUCT_MEMBER(m_sequence, playing));
	save_item(STRUCT_MEMBER(m_sequence, paused));
	save_item(STRUCT_MEMBER(m_sequence, ended));
	save_item(STRUCT_MEMBER(m_sequence, release));
	save_item(STRUCT_MEMBER(m_port, source));
	save_item(STRUCT_MEMBER(m_port, config));
	save_item(STRUCT_MEMBER(m_port, time));
	save_item(STRUCT_MEMBER(m_port, pulse));

	save_item(NAME(m_lowcut));
	save_item(NAME(m_highcut));
	save_item(NAME(m_lowgain));
	save_item(NAME(m_effect));
	save_item(NAME(m_total_effect));
	save_item(NAME(m_mix_c));
	save_item(NAME(m_mono_c));
	save_item(NAME(m_control));
	save_item(NAME(m_handshake));
	save_item(NAME(m_packet));
	save_item(NAME(m_packet_pos));
}

void roland_rpp324_device::device_reset()
{
	soft_reset();
	m_control = 0;
}

// the player's own reset; the 0x80 command does this without touching the board's control latch
void roland_rpp324_device::soft_reset()
{
	reset_players();
	for (unsigned ch = 0; ch < VOICES; ++ch)
	{
		m_voice[ch] = voice();
		m_gain[ch] = ramp();
		m_pan[ch] = ramp();
		m_pan[ch].set(64, 0);
		for (ramp &r : m_route[ch])
		{
			r = ramp();
		}
	}
	for (ramp &r : m_master)
	{
		r = ramp();
	}
	for (ramp &r : m_output)
	{
		r = ramp();
	}
	for (unsigned i = 0; i < 8; ++i)
	{
		m_port[i] = port();
		m_port[i].source = i;
	}

	std::fill(std::begin(m_lowcut), std::end(m_lowcut), 0);
	std::fill(std::begin(m_highcut), std::end(m_highcut), 4096);
	std::fill(std::begin(m_lowgain), std::end(m_lowgain), 127);
	std::fill(std::begin(m_effect), std::end(m_effect), 0);
	m_total_effect = 0;
	m_mix_c = false;
	m_mono_c = false;
	m_packet_pos = 0;
	m_handshake = 0;
}

void roland_rpp324_device::reset_players()
{
	for (unsigned ch = 0; ch < VOICES; ++ch)
	{
		stop_voice(ch);
	}
	for (unsigned id = 0; id < SEQUENCES; ++id)
	{
		stop_sequence(id);
	}
}

unsigned roland_rpp324_device::command_size(u8 op)
{
	if ((op >= 0xc0 && op <= 0xc5) || (op >= 0xc8 && op <= 0xcd))
	{
		return 5;
	}
	switch (op)
	{
		case 0x80: case 0x81: case 0xfe:
			return 1;

		case 0x82: case 0x8c: case 0x8d: case 0x98: case 0x99: case 0x9c: case 0x9d:
		case 0xd0: case 0xd1: case 0xe0: case 0xe1: case 0xf1:
			return 2;

		case 0x83: case 0x84:
			return 3;

		case 0x85: case 0x87: case 0x89: case 0x90: case 0x91: case 0x94: case 0x95:
		case 0xa0: case 0xb0: case 0xfc:
			return 4;

		case 0x86: case 0x88: case 0xf0:
			return 5;

		default:
			return 0;
	}
}

u16 roland_rpp324_device::host_r(offs_t offset)
{
	return offset ? m_handshake : 0;
}

void roland_rpp324_device::host_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (!(mem_mask & 0xff))
	{
		return;
	}
	if (offset)
	{
		m_handshake = data;
		if (BIT(data, 7))
		{
			m_packet_pos = 0;
		}
	}
	else
	{
		command_byte(data);
	}
}

void roland_rpp324_device::control_w(offs_t offset, u16 data, u16 mem_mask)
{
	// Y2 FPGA board-control latch; only the initialization value 0xbd01 is known
	COMBINE_DATA(&m_control);
}

void roland_rpp324_device::command_byte(u8 data)
{
	if (BIT(data, 7))
	{
		m_packet_pos = 0;
		if (!command_size(data))
		{
			m_handshake |= 0x80;
			return;
		}
	}
	else if (!m_packet_pos)
	{
		return;
	}
	m_packet[m_packet_pos++] = data;
	const unsigned length = command_size(m_packet[0]);
	if (m_packet_pos != length + 1)
	{
		return;
	}

	u8 sum = 0;
	for (unsigned i = 0; i < length; ++i)
	{
		sum += m_packet[i];
	}
	m_packet_pos = 0;
	if ((sum & 0x7f) != data)
	{
		m_handshake |= 0x80;
		logerror("rejected checksum for command %02x\n", m_packet[0]);
		return;
	}
	m_stream->update();
	execute(m_packet);
	m_handshake &= ~0x80;
}

void roland_rpp324_device::event(unsigned source, bool start)
{
	for (port &p : m_port)
	{
		const unsigned mode = (p.config >> 4) & 3;
		if (p.source == source && (mode == 3 || mode == (start ? 1 : 2)))
		{
			p.pulse = std::max(1U, unsigned(p.time)) * 441; // provisional 10 ms units
		}
	}
}

bool roland_rpp324_device::playing(unsigned source) const
{
	if (BIT(source, 6))
	{
		return m_sequence[source & 7].playing;
	}
	const voice &v = m_voice[source & 31];
	return v.owner >= 0 && m_voice[v.owner].playing;
}

u16 roland_rpp324_device::status_r()
{
	if (!machine().side_effects_disabled())
	{
		m_stream->update();
	}
	u16 result = 0;
	for (unsigned i = 0; i < 8; ++i)
	{
		const port &p = m_port[i];
		const bool active = (p.config & 0x30) ? (p.pulse != 0) : playing(p.source);
		if (active == bool(BIT(p.config, 6)))
		{
			result |= 1U << i;
		}
	}
	return result;
}

void roland_rpp324_device::stop_voice(unsigned ch)
{
	if (ch >= VOICES || m_voice[ch].owner < 0)
	{
		return;
	}
	const unsigned owner = m_voice[ch].owner;
	voice &v = m_voice[owner];
	for (unsigned n = 0; n < v.channels; ++n)
	{
		event(owner + n, false);
		m_voice[owner + n].owner = -1;
	}
	v.playing = false;
	v.queue_count = 0;
	v.sequence = -1;
}

bool roland_rpp324_device::start_voice(unsigned ch, unsigned phrase, u8 loops, u8 late, int seq)
{
	roland_rdac::phrase p;
	if (!roland_rdac::describe(m_rom, m_rom.bytes(), phrase, p) || ch + p.channels > VOICES)
	{
		logerror("invalid phrase %u on channel %u\n", phrase, ch);
		return false;
	}
	stop_voice(ch);
	if (p.channels == 2)
	{
		stop_voice(ch + 1);
	}

	voice &v = m_voice[ch];
	v = voice();
	v.phrase = phrase;
	v.loops = loops;
	v.late = late;
	v.channels = p.channels;
	v.step = p.step;
	v.sequence = seq;
	v.decoder.start(p);
	if (!v.decoder.frame(m_rom, v.current))
	{
		return false;
	}
	v.next_valid = v.decoder.frame(m_rom, v.next);
	v.playing = true;
	for (unsigned n = 0; n < p.channels; ++n)
	{
		m_voice[ch + n].owner = ch;
		event(ch + n, true);
	}
	if (late)
	{
		logerror("unmeasured Late mode %u\n", late);
	}
	LOGCOMMAND("phrase %u on %u, loops %u, sequence %d\n", phrase, ch, loops, seq);
	return true;
}

void roland_rpp324_device::play(unsigned ch, unsigned phrase, u8 loops, u8 late, bool immediate, int seq)
{
	if (ch >= VOICES)
	{
		return;
	}
	roland_rdac::phrase p;
	if (!roland_rdac::describe(m_rom, m_rom.bytes(), phrase, p))
	{
		return;
	}

	// the SDK pairs even/odd channels for stereo, and KOF addresses either member of the pair
	if (p.channels == 2)
	{
		ch &= ~1U;
	}
	if (!immediate && m_voice[ch].owner >= 0)
	{
		voice &v = m_voice[m_voice[ch].owner];
		if (v.queue_count == QUEUE)
		{
			logerror("phrase queue full\n");
			return;
		}
		const unsigned q = v.queue_count++;
		v.queued[q] = phrase;
		v.queued_loops[q] = loops;
		v.queued_late[q] = late;
		return;
	}
	start_voice(ch, phrase, loops, late, seq);
}

void roland_rpp324_device::finish_voice(unsigned ch)
{
	voice &v = m_voice[ch];
	const u8 phase = v.phase;
	if (v.decoder.failed)
	{
		logerror("malformed RDAC phrase %u at %08x\n", v.phrase, v.decoder.cursor);
		stop_voice(ch);
	}
	else if (v.release)
	{
		stop_voice(ch);
	}
	else if (v.queue_count)
	{
		const voice old = v;
		start_voice(ch, old.queued[0], old.queued_loops[0], old.queued_late[0], old.sequence);
		v.queue_count = old.queue_count - 1;
		for (unsigned i = 0; i < v.queue_count; ++i)
		{
			v.queued[i] = old.queued[i + 1];
			v.queued_loops[i] = old.queued_loops[i + 1];
			v.queued_late[i] = old.queued_late[i + 1];
		}
	}
	else if (!v.loops || v.loops > 1)
	{
		start_voice(ch, v.phrase, v.loops ? v.loops - 1 : 0, v.late, v.sequence);
	}
	else
	{
		stop_voice(ch);
	}

	// keep the fractional sample clock across loops and chained phrases
	if (v.playing)
	{
		v.phase = phase;
	}
}

void roland_rpp324_device::stop_sequence(unsigned id)
{
	for (unsigned ch = 0; ch < VOICES; ++ch)
	{
		if (m_voice[ch].owner == ch && m_voice[ch].sequence == id)
		{
			stop_voice(ch);
		}
	}
	if (m_sequence[id].playing)
	{
		event(0x40 | id, false);
	}
	m_sequence[id] = sequence();
}

void roland_rpp324_device::play_sequence(unsigned id, unsigned number, u8 loops)
{
	stop_sequence(id);
	if (number >= m_sequence_count)
	{
		logerror("invalid sequence %u\n", number);
		return;
	}
	roland_rdac::phrase first;
	if (!roland_rdac::describe(m_rom, m_rom.bytes(), 0, first))
	{
		return;
	}
	const u32 start = roland_rdac::read32(m_rom + m_sequence_table + number * 4) >> 3;
	const u32 end = (number + 1 < m_sequence_count) ? (roland_rdac::read32(m_rom + m_sequence_table + (number + 1) * 4) >> 3) : first.start;
	if (start < m_sequence_table + m_sequence_count * 4 || start >= end || end > first.start || (start & 1))
	{
		return;
	}

	sequence &s = m_sequence[id];
	s.start = start;
	s.cursor = start;
	s.end = end;
	s.loops = loops;
	s.playing = true;
	event(0x40 | id, true);
}

void roland_rpp324_device::step_sequence(unsigned id)
{
	sequence &s = m_sequence[id];
	if (!s.playing || s.paused)
	{
		return;
	}
	if (s.delay)
	{
		--s.delay;
		return;
	}
	if (s.ended)
	{
		for (unsigned ch = 0; ch < VOICES; ++ch)
		{
			if (m_voice[ch].playing && m_voice[ch].owner == ch && m_voice[ch].sequence == id)
			{
				return;
			}
		}
		if (!s.release && (!s.loops || s.loops > 1))
		{
			if (s.loops)
			{
				--s.loops;
			}
			s.cursor = s.start;
			s.ended = false;
		}
		else
		{
			stop_sequence(id);
			return;
		}
	}

	// bound the work per sample, including for malformed programs
	for (unsigned count = 0; count < 32 && !s.ended && !s.delay; ++count)
	{
		if (s.cursor >= s.end)
		{
			stop_sequence(id);
			return;
		}
		u8 packet[6]{};
		packet[0] = m_rom[s.cursor ^ 1];
		const unsigned size = command_size(packet[0]);
		if (!size || s.cursor + size + 1 > s.end)
		{
			logerror("invalid sequence packet\n");
			stop_sequence(id);
			return;
		}
		u8 sum = packet[0];
		for (unsigned i = 1; i <= size; ++i)
		{
			packet[i] = m_rom[(s.cursor + i) ^ 1];
			if (i < size)
			{
				sum += packet[i];
			}
		}
		if ((sum & 0x7f) != packet[size])
		{
			logerror("bad sequence checksum\n");
			stop_sequence(id);
			return;
		}
		s.cursor += (size + 2) & ~1U;
		if (packet[0] == 0xfe)
		{
			s.ended = true;
		}
		else if (packet[0] == 0xfc)
		{
			s.delay = (u32(packet[1]) << 14) | (packet[2] << 7) | packet[3];
		}
		else if (packet[0] == 0xf0 || packet[0] == 0xf1 || packet[0] == 0x80 || packet[0] == 0x81)
		{
			logerror("unsupported sequence control %02x\n", packet[0]);
			stop_sequence(id);
			return;
		}
		else
		{
			execute(packet, id);
		}
	}
}

void roland_rpp324_device::execute(const u8 *p, int seq)
{
	const unsigned op = p[0];
	const unsigned ch = p[1] & 31;
	if ((op >= 0xc0 && op <= 0xc5) || (op >= 0xc8 && op <= 0xcd))
	{
		play(ch, (p[3] << 7) | p[4], p[2], op & 7, BIT(op, 3), seq);
		return;
	}
	switch (op)
	{
		case 0x80: // resets the player, not the board's control latch
			soft_reset();
			break;

		case 0x81:
			reset_players();
			m_master[1].set(127, 0);
			break;

		case 0x82:
			m_mono_c = bool(p[1]);
			break;

		case 0x83: case 0x84:
			m_master[op - 0x83].set(p[2], p[1]);
			break;

		case 0x85:
			if ((p[1] & ~5) == 0)
			{
				m_output[BIT(p[1], 0) | (BIT(p[1], 2) << 1)].set(p[3], p[2]);
			}
			break;

		case 0x86: case 0x87: case 0x88:
			if ((p[1] & ~5) == 0)
			{
				const unsigned output = BIT(p[1], 0) | (BIT(p[1], 2) << 1);
				if (op == 0x86)
				{
					m_lowcut[output] = (p[3] << 7) | p[4];
				}
				else if (op == 0x87)
				{
					m_lowgain[output] = p[3];
				}
				else
				{
					m_highcut[output] = (p[3] << 7) | p[4];
				}
				logerror("filter command %02x stored, filter response unmeasured\n", op);
			}
			break;

		case 0x89:
		{
			port &target = m_port[p[2] & 7];
			if (target.source != p[1] || target.config != p[2])
			{
				target.pulse = 0;
			}
			target.source = p[1];
			target.config = p[2];
			target.time = p[3];
			break;
		}

		case 0x8c:
			m_total_effect = p[1];
			break;

		case 0x8d:
			m_mix_c = bool(p[1]);
			break;

		case 0x90: case 0x91: case 0x94: case 0x95:
			m_route[ch][BIT(op, 0) | (BIT(op, 2) << 1)].set(p[3], p[2]);
			break;

		case 0x98: case 0x99: case 0x9c: case 0x9d:
		{
			const unsigned bit = BIT(op, 2);
			m_effect[ch] = (m_effect[ch] & ~(1U << bit)) | (BIT(op, 0) << bit);
			break;
		}

		case 0xa0:
			m_pan[ch].set(p[3], p[2]);
			break;

		case 0xb0:
			m_gain[ch].set(p[3], p[2]);
			break;

		case 0xd0: case 0xd1:
			if (m_voice[ch].owner >= 0)
			{
				m_voice[m_voice[ch].owner].paused = BIT(op, 0);
			}
			break;

		case 0xe0:
			stop_voice(ch);
			break;

		case 0xe1:
			if (m_voice[ch].owner >= 0)
			{
				m_voice[m_voice[ch].owner].release = true;
			}
			break;

		case 0xf0:
			play_sequence(p[1] & 7, (p[3] << 7) | p[4], p[2]);
			break;

		case 0xf1:
		{
			const unsigned id = p[1] & 7;
			const unsigned mode = (p[1] >> 5) & 3;
			if (!mode)
			{
				stop_sequence(id);
			}
			else if (mode == 1)
			{
				m_sequence[id].release = true;
				m_sequence[id].ended = true;
				for (voice &v : m_voice)
				{
					if (v.sequence == id)
					{
						v.release = true;
					}
				}
			}
			else
			{
				m_sequence[id].paused = (mode == 2);
				for (voice &v : m_voice)
				{
					if (v.sequence == id)
					{
						v.paused = (mode == 2);
					}
				}
			}
			break;
		}

		default:
			logerror("unsupported host command %02x\n", op);
			break;
	}
}

void roland_rpp324_device::sound_stream_update(sound_stream &stream)
{
	for (int i = 0; i < stream.samples(); ++i)
	{
		for (unsigned id = 0; id < SEQUENCES; ++id)
		{
			step_sequence(id);
		}

		double gain[VOICES];
		double pan[VOICES];
		double route[VOICES][4];
		for (unsigned ch = 0; ch < VOICES; ++ch)
		{
			gain[ch] = m_gain[ch].tick() / 127.0;
			pan[ch] = m_pan[ch].tick();
			for (unsigned n = 0; n < 4; ++n)
			{
				route[ch][n] = m_route[ch][n].tick() / 127.0;
			}
		}

		double mix[4]{};
		for (unsigned ch = 0; ch < VOICES; ++ch)
		{
			voice &v = m_voice[ch];
			if (!v.playing || v.owner != ch || v.paused)
			{
				continue;
			}
			for (unsigned n = 0; n < v.channels; ++n)
			{
				const unsigned physical = ch + n;
				double sample = v.current[n];
				if (v.next_valid)
				{
					sample += (v.next[n] - sample) * (v.phase / 8.0);
				}
				sample *= gain[physical];
				const double left = std::min(1.0, (127.0 - pan[physical]) / 63.0);
				const double right = std::min(1.0, pan[physical] / 64.0);
				for (unsigned out = 0; out < 4; ++out)
				{
					mix[out] += sample * route[physical][out] * ((out & 1) ? right : left);
				}
			}
			v.phase += v.step;
			if (v.phase >= 8)
			{
				v.phase -= 8;
				if (v.next_valid)
				{
					std::copy(std::begin(v.next), std::end(v.next), std::begin(v.current));
					v.next_valid = v.decoder.frame(m_rom, v.next);
				}
				else
				{
					finish_voice(ch);
				}
			}
		}

		const double master = m_master[0].tick() * m_master[1].tick() / (127.0 * 127.0);
		for (unsigned out = 0; out < 4; ++out)
		{
			mix[out] *= m_output[out].tick() * master / 127.0;
		}
		if (m_mono_c)
		{
			mix[2] = mix[3] = (mix[2] + mix[3]) * 0.5;
		}
		if (m_mix_c)
		{
			mix[0] += mix[2];
			mix[1] += mix[3];
		}
		for (unsigned out = 0; out < 4; ++out)
		{
			stream.put_int(out, i, s32(std::clamp(mix[out], -32768.0, 32767.0)), 32768);
		}
		for (port &p : m_port)
		{
			if (p.pulse)
			{
				--p.pulse;
			}
		}
	}
}
