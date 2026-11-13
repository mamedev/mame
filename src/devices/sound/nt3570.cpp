// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Novatek NT3570F, 16-voice sample generator, 8 MHz input on IGS hardware.

    No data sheet is available. The indexed bus and streamed sample path are
    reconstructed from the Paomadi NT3580 programs. The firmware supplies an
    initial 256-byte page, starts the voice, and queues the next page. It then
    services a per-voice event and queues another page. Writing a zero to a
    voice's bit in registers 15/16 acknowledges the event before re-enabling it.

    Instrument size is inferred as 256 << (register 50 bits 1-3): the firmware
    scales note pitch by the same power of two, and the resulting lengths fit
    waveform boundaries in the sample ROM. Bit 0 selects one-shot playback.

    Instrument envelopes are an EXPERIMENTAL model, not a recovered hardware
    specification. Descriptor ranges and instrument families suggest A0/B0/C0
    low five bits are attack/decay/sustain-decay rates, and D0 packs a sustain
    attenuation nibble and release rate. A0 bit 7 bypasses this model for speech.
    The rate curve, linear gain, sustain scale, and key-off release behavior
    remain hypotheses. See envelope_step for the provisional timing law.

    TODO: verify waveform size/loop flags, envelope field meanings/rates,
    gain/routing law, precise busy time, event timing and oscillator dividers.
    The nominal pitch scale follows firmware division by 98, not a measured clock.
    The 8 MHz / 256 output cadence is provisional.
*/

#include "emu.h"
#include "nt3570.h"

#include <algorithm>
#include <iterator>

#define LOG_REG (1U << 1)
#define LOG_KEY (1U << 2)
#define LOG_EVENT (1U << 3)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(NT3570, nt3570_device, "nt3570", "Novatek NT3570F sound generator")

nt3570_device::nt3570_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NT3570, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_irq_cb(*this)
	, m_stream(nullptr)
	, m_service_timer(nullptr)
	, m_irq_state(false)
{
}

void nt3570_device::device_start()
{
	// The first reset flushes the stream before clearing the voice registers.
	std::fill(std::begin(m_active), std::end(m_active), false);
	m_stream = stream_alloc(0, 1, clock() / 256);
	m_service_timer = timer_alloc(FUNC(nt3570_device::service_tick), this);
	save_item(NAME(m_registers));
	save_item(NAME(m_select));
	save_item(NAME(m_enable));
	save_item(NAME(m_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_busy_until));
	save_item(NAME(m_active));
	save_item(NAME(m_streaming));
	save_item(NAME(m_last));
	save_item(NAME(m_next_valid));
	save_item(NAME(m_page));
	save_item(NAME(m_next_page));
	save_item(NAME(m_length));
	save_item(NAME(m_position));
	save_item(NAME(m_envelope_state));
	save_item(NAME(m_envelope_level));
}

void nt3570_device::device_reset()
{
	m_stream->update();
	std::fill(std::begin(m_registers), std::end(m_registers), 0);
	std::fill(std::begin(m_active), std::end(m_active), false);
	std::fill(std::begin(m_streaming), std::end(m_streaming), false);
	std::fill(std::begin(m_next_valid), std::end(m_next_valid), false);
	std::fill(std::begin(m_last), std::end(m_last), false);
	std::fill(std::begin(m_page), std::end(m_page), 0);
	std::fill(std::begin(m_next_page), std::end(m_next_page), 0);
	std::fill(std::begin(m_length), std::end(m_length), 0);
	std::fill(std::begin(m_position), std::end(m_position), 0);
	std::fill(std::begin(m_envelope_state), std::end(m_envelope_state), ENV_OFF);
	std::fill(std::begin(m_envelope_level), std::end(m_envelope_level), 0);
	m_select = 0;
	m_enable = m_pending = 0;
	m_busy_until = attotime::zero;
	update_irq();
	device_clock_changed();
}

void nt3570_device::device_clock_changed()
{
	if (m_stream)
	{
		m_stream->set_sample_rate(clock() / 256);
		const attotime period = attotime::from_ticks(256, clock());
		m_service_timer->adjust(period, 0, period);
	}
}

void nt3570_device::rom_bank_pre_change()
{
	m_stream->update();
}

TIMER_CALLBACK_MEMBER(nt3570_device::service_tick)
{
	// Keep page interrupts timely even when the mixer would otherwise update
	// a large batch of audio: the controller must have time to supply each page.
	m_stream->update();
	update_irq();
}

void nt3570_device::update_irq()
{
	const bool state = bool(m_pending & m_enable);
	if (m_irq_state != state)
	{
		m_irq_state = state;
		m_irq_cb(state ? 1 : 0);
	}
}

u8 nt3570_device::read(offs_t offset)
{
	if (!BIT(offset, 0))
		return (machine().time() < m_busy_until) ? 1 : 0;
	if (!machine().side_effects_disabled())
		m_stream->update();
	if (m_select == 0x15)
		return m_pending & m_enable;
	if (m_select == 0x16)
		return (m_pending & m_enable) >> 8;
	return m_registers[m_select];
}

void nt3570_device::write(offs_t offset, u8 data)
{
	m_stream->update();
	if (!BIT(offset, 0))
		m_select = data;
	else
		register_w(m_select, data);
	// Provisional finite bus turnaround; firmware waits before selecting and
	// before accessing data, with six additional NOPs between those accesses.
	m_busy_until = machine().time() + attotime::from_ticks(2, clock());
}

u32 nt3570_device::envelope_step(u8 rate)
{
	// Provisional, clock-relative curve: each two rate codes double the speed.
	// Zero holds the current level. At 8 MHz, a full-scale falling ramp takes
	// 4.096 ms at rate 31, 131.072 ms at 21, or about 2.80 s at 12. Attacks are
	// sixteen times faster; maximum attack is immediate. These timings are
	// deliberately isolated here pending measurements of a physical device.
	rate &= 0x1f;
	return rate ? ((BIT(rate, 0) ? 4U : 3U) << (rate >> 1)) : 0;
}

void nt3570_device::voice_stop(unsigned ch)
{
	m_active[ch] = false;
	m_envelope_state[ch] = ENV_OFF;
	m_envelope_level[ch] = 0;
}

void nt3570_device::envelope_start(unsigned ch)
{
	if (m_streaming[ch])
	{
		m_envelope_state[ch] = ENV_BYPASS;
		m_envelope_level[ch] = ENV_ONE;
	}
	else if ((m_registers[0xa0 | ch] & 0x1f) == 0x1f)
	{
		m_envelope_state[ch] = ENV_DECAY;
		m_envelope_level[ch] = ENV_ONE;
	}
	else
	{
		m_envelope_state[ch] = ENV_ATTACK;
		m_envelope_level[ch] = 0;
	}
}

void nt3570_device::envelope_advance(unsigned ch)
{
	u32 &level = m_envelope_level[ch];
	u8 &state = m_envelope_state[ch];
	if (state == ENV_ATTACK)
	{
		level = std::min(ENV_ONE, level + 16 * envelope_step(m_registers[0xa0 | ch]));
		if (level == ENV_ONE)
			state = ENV_DECAY;
	}
	else if (state == ENV_DECAY)
	{
		const u32 target = (u64(ENV_ONE) * (15 - (m_registers[0xd0 | ch] >> 4))) / 15;
		const u32 step = envelope_step(m_registers[0xb0 | ch]);
		if (level <= target)
			state = ENV_SUSTAIN;
		else if (level - target <= step)
		{
			level = target;
			state = ENV_SUSTAIN;
		}
		else
			level -= step;
	}
	else if (state == ENV_SUSTAIN || state == ENV_RELEASE)
	{
		// The four-bit release field spans the five-bit rate range. Unlike a
		// zero sustain rate (hold), every release rate eventually becomes silent.
		const u8 rate = state == ENV_RELEASE ? (2 * (m_registers[0xd0 | ch] & 15) + 1) : m_registers[0xc0 | ch];
		const u32 step = envelope_step(rate);
		if (level <= step)
			voice_stop(ch);
		else
			level -= step;
	}
}

void nt3570_device::key_w(u8 data)
{
	const unsigned ch = data & 15;
	m_pending &= ~(1U << ch);
	m_next_valid[ch] = false;
	if (BIT(data, 4))
	{
		m_active[ch] = true;
		m_streaming[ch] = BIT(m_registers[0xa0 | ch], 7);
		m_page[ch] = (u16(m_registers[0x60 | ch]) << 8) | m_registers[0x70 | ch];
		m_last[ch] = BIT(m_registers[0x50 | ch], 0);
		m_length[ch] = m_streaming[ch] ? 256 : (256U << BIT(m_registers[0x50 | ch], 1, 3));
		m_position[ch] = 0;
		envelope_start(ch);
	}
	else if (m_active[ch] && !m_streaming[ch] && m_envelope_level[ch])
		m_envelope_state[ch] = ENV_RELEASE;
	else
		voice_stop(ch);
	LOGMASKED(LOG_KEY, "%s: voice %u key %s page %04x streaming %u\n", machine().describe_context(), ch, BIT(data, 4) ? "on" : "off", m_page[ch], m_streaming[ch]);
	update_irq();
}

void nt3570_device::register_w(u8 reg, u8 data)
{
	LOGMASKED(LOG_REG, "%s: reg %02x = %02x\n", machine().describe_context(), reg, data);
	m_registers[reg] = data;
	if (reg == 0x15 || reg == 0x16)
	{
		const unsigned shift = (reg - 0x15) * 8;
		m_enable = (m_enable & ~(0xffU << shift)) | (u16(data) << shift);
		m_pending &= m_enable;
		update_irq();
	}
	else if (reg == 0x17)
		key_w(data);
	else if ((reg & 0xf0) == 0x50)
	{
		// Unlike the buffered page address, the end flag applies to the page
		// currently playing. The firmware sets it when its remaining-page
		// counter reaches zero, then writes an unused address past the sample.
		const unsigned ch = reg & 15;
		if (m_active[ch] && m_streaming[ch])
			m_last[ch] = BIT(data, 0);
	}
	else if ((reg & 0xf0) == 0x70)
	{
		const unsigned ch = reg & 15;
		if (m_active[ch] && m_streaming[ch])
		{
			m_next_page[ch] = (u16(m_registers[0x60 | ch]) << 8) | data;
			m_next_valid[ch] = true;
		}
	}
}

void nt3570_device::sound_stream_update(sound_stream &stream)
{
	for (unsigned ch = 0; ch < std::size(m_active); ch++)
	{
		if (!m_active[ch])
			continue;
		const u16 pitch = m_registers[0x20 | ch] | (u16(m_registers[0x30 | ch]) << 8);
		const u32 step = (u64(pitch) * 98 * 65'536 * 256) / 8'000'000;
		// Both nibbles participate in firmware volume calculations; routing and
		// gain law are unknown. Average them for the board's mono output for now.
		const unsigned level = (m_registers[0xe0 | ch] >> 4) + (m_registers[0xe0 | ch] & 15);
		for (int i = 0; i < stream.samples() && m_active[ch]; i++)
		{
			const u32 address = (u32(m_page[ch]) << 8) + (m_position[ch] >> 16);
			const s32 sample = s32(read_byte(address)) - 0x80;
			const s32 scaled = sample * s32(level);
			if (m_streaming[ch])
				// Streamed speech bypasses the instrument envelope.
				stream.add_int(0, i, scaled, 128 * 30 * 4);
			else
				stream.add(0, i, sound_stream::sample_t(scaled) * m_envelope_level[ch] / (ENV_ONE * (128.0 * 30 * 4)));
			envelope_advance(ch);
			m_position[ch] += step;
			while (m_position[ch] >= (m_length[ch] << 16) && m_active[ch])
			{
				m_position[ch] -= m_length[ch] << 16;
				if (!m_streaming[ch])
				{
					if (m_last[ch])
						voice_stop(ch);
					continue;
				}
				if (m_last[ch] || !m_next_valid[ch])
					voice_stop(ch);
				else
				{
					m_page[ch] = m_next_page[ch];
					m_next_valid[ch] = false;
				}
				m_pending |= 1U << ch;
				LOGMASKED(LOG_EVENT, "voice %u page boundary, pending %04x\n", ch, m_pending);
			}
		}
	}
}
