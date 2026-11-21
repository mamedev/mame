// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
// The internal UPP DSP owns the E8 PCM resource. The original NOS audio
// driver submits signed mono samples in two SDRAM halves via its MDI ring.

#include "emu.h"
#include "ngage_pcm.h"

#include <bit>

namespace {

u16 frame_word(std::span<u8 const> packet, unsigned offset)
{
	return (u16(packet[offset]) << 8) | packet[offset + 1];
}

bool valid_halves(u32 first, u32 second, u32 bytes)
{
	// The fifteen physical pages mapped by audio.ldd.
	return bytes && !((first | second) & 1)
		&& first >= 0x0a061000 && u64(first) + bytes <= 0x0a070000
		&& second >= 0x0a061000 && u64(second) + bytes <= 0x0a070000
		&& (first + bytes <= second || second + bytes <= first);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE(NGAGE_PCM, ngage_pcm_device, "ngage_pcm", "N-Gage UPP DSP PCM")

ngage_pcm_device::ngage_pcm_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NGAGE_PCM, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_reply_ready(*this)
{
}

void ngage_pcm_device::device_start()
{
	m_stream = stream_alloc(0, 1, STREAM_RATE);
	m_completion = timer_alloc(FUNC(ngage_pcm_device::half_complete), this);
	save_item(NAME(m_reply));
	save_item(NAME(m_media));
	save_item(NAME(m_device));
	save_item(NAME(m_object));
	save_item(NAME(m_dsp_device));
	save_item(NAME(m_dsp_object));
	save_item(NAME(m_reply_pending));
	save_item(NAME(m_open));
	save_item(NAME(m_first));
	save_item(NAME(m_final_done));
	save_item(NAME(m_requested));
	save_item(NAME(m_final));
	save_item(NAME(m_playing));
	save_item(NAME(m_free));
	save_item(NAME(m_capacity));
	save_item(NAME(m_final_frames));
	save_item(NAME(m_address));
	save_item(NAME(m_frames));
	save_item(NAME(m_output_position));
	save_item(NAME(m_repeat));
}

void ngage_pcm_device::device_reset()
{
	disconnect();
}

void ngage_pcm_device::stop()
{
	m_stream->update();
	m_completion->adjust(attotime::never);
	m_frames[0] = m_frames[1] = 0;
	m_playing = m_requested = m_final = NO_HALF;
	m_free = 0;
	m_output_position = 0;
	m_open = m_final_done = false;
	m_first = true;
}

void ngage_pcm_device::disconnect()
{
	if (m_open)
		stop();
	m_reply_pending = false;
}

void ngage_pcm_device::set_route(std::span<u8 const> packet)
{
	m_media = packet[0];
	m_dsp_device = packet[1];
	m_device = packet[2];
	m_dsp_object = packet[6];
	m_object = packet[7];
}

void ngage_pcm_device::make_reply(u8 transaction, u8 opcode, u16 value)
{
	m_reply[0] = (u16(m_media) << 8) | m_device;
	m_reply[1] = (u16(m_dsp_device) << 8) | 0xe8;
	m_reply[2] = 8;
	m_reply[3] = (u16(m_object) << 8) | m_dsp_object;
	m_reply[4] = (u16(transaction) << 8) | opcode;
	m_reply[5] = value;
	m_reply[6] = 0;
	m_reply_pending = true;
}

std::span<u16 const> ngage_pcm_device::reply() const
{
	return m_reply_pending ? std::span<u16 const>(m_reply) : std::span<u16 const>();
}

void ngage_pcm_device::reply_consumed()
{
	m_reply_pending = false;
	service_reply();
}

void ngage_pcm_device::service_reply()
{
	if (m_reply_pending || !m_open)
		return;
	if (m_final_done)
	{
		make_reply(0, 0x2b, m_final_frames * 2);
		stop();
	}
	else if (m_final == NO_HALF && m_requested == NO_HALF && m_free)
	{
		m_requested = std::countr_zero(unsigned(m_free));
		m_free &= ~(1U << m_requested);
		make_reply(0, 0x26, m_requested);
	}
}

void ngage_pcm_device::receive(std::span<u8 const> packet)
{
	if (packet.size() < 10)
		return;

	u8 const transaction = packet[8];
	u8 const opcode = packet[9];
	if (opcode == 0x20 && packet.size() == 24)
	{
		u16 const capacity = frame_word(packet, 12);
		u32 const first = (u32(frame_word(packet, 14)) << 16) | frame_word(packet, 16);
		u32 const second = (u32(frame_word(packet, 18)) << 16) | frame_word(packet, 20);
		if (packet[10] != 0x82 || packet[11] != 2 || packet[22]
			|| (packet[23] != 0x32 && packet[23] != 0x46) || !valid_halves(first, second, u32(capacity) * 2))
			return;

		stop();
		set_route(packet);
		m_capacity = capacity;
		m_address[0] = first;
		m_address[1] = second;
		m_repeat = packet[23] == 0x32 ? 2 : 1;
		m_open = true;
		make_reply(transaction, 0x21);
		return;
	}

	bool const owner = m_open && packet[1] == m_dsp_device && packet[2] == m_device
		&& packet[6] == m_dsp_object && packet[7] == m_object;
	if (opcode == 0x2a && packet.size() == 10 && (owner || !m_open))
	{
		m_stream->update();
		set_route(packet);
		make_reply(transaction, 0x2b, m_output_position / m_repeat * 2);
		stop();
		return;
	}
	if (!owner || packet.size() != 14 || m_final != NO_HALF)
		return;

	u8 const half = opcode == 0x23 && packet[10] < 2 ? packet[10] : opcode == 0x27 ? m_requested : NO_HALF;
	u16 const frames = frame_word(packet, 12);
	if (half == NO_HALF || m_frames[half] || frames > m_capacity || (packet[11] != 1 && packet[11] != 2))
		return;

	if (half == m_requested)
		m_requested = NO_HALF;
	if (m_first)
		m_free |= 1U << (half ^ 1);
	m_first = false;
	m_free &= ~(1U << half);
	m_frames[half] = frames;
	if (packet[11] == 1)
	{
		m_final = half;
		m_final_frames = frames;
		m_final_done = !m_frames[0] && !m_frames[1];
	}
	make_reply(transaction, opcode + 1);
	if (m_playing == NO_HALF)
		start_half(half);
}

void ngage_pcm_device::start_half(u8 preferred)
{
	u8 const half = m_frames[preferred] ? preferred : preferred ^ 1;
	if (!m_frames[half])
		return;
	m_stream->update();
	m_playing = half;
	m_output_position = 0;
	m_completion->adjust(attotime::from_ticks(u32(m_frames[half]) * m_repeat, STREAM_RATE));
}

TIMER_CALLBACK_MEMBER(ngage_pcm_device::half_complete)
{
	m_stream->update();
	u8 const half = m_playing;
	m_frames[half] = 0;
	m_playing = NO_HALF;
	m_output_position = 0;
	m_free |= 1U << half;
	m_final_done = m_final == half || (m_final != NO_HALF && !m_final_frames && !m_frames[half ^ 1]);
	if (!m_final_done)
		start_half(half ^ 1);
	service_reply();
	if (m_reply_pending)
		m_reply_ready(1);
}

void ngage_pcm_device::sound_stream_update(sound_stream &stream)
{
	for (s32 index = 0; index < stream.samples(); ++index)
	{
		s16 sample = 0;
		if (m_memory && m_playing != NO_HALF && m_output_position < u32(m_frames[m_playing]) * m_repeat)
		{
			sample = s16(m_memory->read_word(m_address[m_playing] + 2 * (m_output_position / m_repeat)));
			++m_output_position;
		}
		stream.put(0, index, sample / 32768.0F);
	}
}
