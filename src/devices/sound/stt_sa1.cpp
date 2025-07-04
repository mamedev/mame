// license:BSD-3-Clause
// copyright-holders:windyfairy
/*
    ST-Techno STT-SA1 PCM sound chip
    Originally implemented in an FPGA
*/
#include "emu.h"
#include "stt_sa1.h"

// #define VERBOSE (LOG_GENERAL)
// #define LOG_OUTPUT_STREAM std::cout
#include "logmacro.h"


DEFINE_DEVICE_TYPE(STT_SA1, stt_sa1_device, "stt_sa1", "ST-Techno STT-SA1 Sound")

stt_sa1_device::stt_sa1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, STT_SA1, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, device_rom_interface(mconfig, *this)
	, m_stream(nullptr)
{
}

void stt_sa1_device::enable_w(uint16_t data)
{
	m_enabled = data != 0;
}

uint16_t stt_sa1_device::read(offs_t offset, uint16_t mem_mask)
{
	offset &= 0x7f;

	return m_regs[offset]; // TODO: Should this return addr_cur for regs 1 and 2 instead?
}

void stt_sa1_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset &= 0x7f;
	const int v = offset >> 4;
	const int reg = offset & 0xf;

	m_regs[offset] = data;

	switch (reg) {
		case 1:
			m_voice[v].addr_start = m_voice[v].addr_cur = (m_voice[v].addr_cur & 0xffff0000000) | (uint64_t(data) << 12);
			LOG("voice %d: start = %08llx\n", v, m_voice[v].addr_cur >> 12);
			break;
		case 2:
			m_voice[v].addr_start = m_voice[v].addr_cur = (m_voice[v].addr_cur & 0x0000ffff000) | (uint64_t(data & 0x7fff) << 28);
			m_voice[v].is_looped = BIT(data, 15) != 0;
			LOG("voice %d: start = %08llx, is_looped = %d\n", v, m_voice[v].addr_cur >> 12, m_voice[v].is_looped);
			break;

		case 3:
			m_voice[v].freq = data;
			LOG("voice %d: step = %08x\n", v, m_voice[v].freq);
			break;

		case 4:
			m_voice[v].addr_start = (m_voice[v].addr_start & 0xffff0000000) | (uint64_t(data) << 12);
			LOG("voice %d: addr_start = %08llx\n", v, m_voice[v].addr_start >> 12);
			break;
		case 5:
			m_voice[v].addr_start = (m_voice[v].addr_start & 0x0000ffff000) | (uint64_t(data) << 28);
			LOG("voice %d: addr_start = %08llx\n", v, m_voice[v].addr_start >> 12);
			break;

		case 6:
			m_voice[v].addr_end = (m_voice[v].addr_end & 0xffff0000000) | (uint64_t(data) << 12);
			LOG("voice %d: addr_end = %08llx\n", v, m_voice[v].addr_end >> 12);
			break;
		case 7:
			m_voice[v].addr_end = (m_voice[v].addr_end & 0x0000ffff000) | (uint64_t(data) << 28);
			LOG("voice %d: addr_end = %08llx\n", v, m_voice[v].addr_end >> 12);
			break;

		case 0x0b:
			m_voice[v].vol_l = data;
			LOG("voice %d: vol_l = %08x\n", v, m_voice[v].vol_l);
			break;
		case 0x0c:
			m_voice[v].vol_r = data;
			LOG("voice %d: vol_r = %08x\n", v, m_voice[v].vol_r);
			break;

		default:
			LOG("Unknown register usage: voice %d, register %x, data %04x\n", v, reg, data);
			break;
	}
}

uint16_t stt_sa1_device::key_r()
{
	if (!m_enabled)
		return 0;

	if (!machine().side_effects_disabled())
		m_stream->update();

	return m_keyctrl;
}

void stt_sa1_device::key_w(uint16_t data)
{
	if (!m_enabled)
		return;

	const u16 prev = m_keyctrl;

	m_stream->update();

	m_keyctrl = data;

	for (int v = 0; v < 8; v++) {
		if (BIT(m_keyctrl, v) && !BIT(prev, v)) {
			// keyon
			m_voice[v].enabled = true;
			m_voice[v].addr_cur = m_voice[v].addr_start;
		} else if (!BIT(m_keyctrl, v) && BIT(prev, v)) {
			// keyoff
			m_voice[v].enabled = false;
		}
	}
}

void stt_sa1_device::device_start()
{
	m_stream = stream_alloc(0, 2, clock() / 448);

	save_item(STRUCT_MEMBER(m_voice, addr_start));
	save_item(STRUCT_MEMBER(m_voice, addr_cur));
	save_item(STRUCT_MEMBER(m_voice, addr_end));
	save_item(STRUCT_MEMBER(m_voice, vol_l));
	save_item(STRUCT_MEMBER(m_voice, vol_r));
	save_item(STRUCT_MEMBER(m_voice, freq));
	save_item(STRUCT_MEMBER(m_voice, is_looped));
	save_item(STRUCT_MEMBER(m_voice, enabled));
	save_item(NAME(m_keyctrl));
	save_item(NAME(m_regs));
	save_item(NAME(m_enabled));
}

void stt_sa1_device::device_reset()
{
	m_enabled = false;

	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	for (int i = 0; i < 8; i++) {
		m_voice[i].addr_start = 0;
		m_voice[i].addr_cur = 0;
		m_voice[i].addr_end = 0;
		m_voice[i].vol_l = 0;
		m_voice[i].vol_r = 0;
		m_voice[i].freq = 0;
		m_voice[i].is_looped = false;
		m_voice[i].enabled = false;
	}
}

void stt_sa1_device::sound_stream_update(sound_stream &stream)
{
	for (int v = 0; v < 8; v++) {
		voice_t &voice = m_voice[v];

		for (int i = 0; i < stream.samples() && voice.enabled; i++) {
			const offs_t offset = voice.addr_cur >> 12;
			const int sample = s8(read_byte(offset)) << 8;

			voice.addr_cur += voice.freq;

			stream.add_int(0, i, (sample * voice.vol_l) >> 16, 32768 * 8);
			stream.add_int(1, i, (sample * voice.vol_r) >> 16, 32768 * 8);

			if (voice.addr_cur >= voice.addr_end) {
				if (!voice.is_looped) {
					voice.enabled = false;
					m_keyctrl &= ~(1 << v);
				} else {
					voice.addr_cur = voice.addr_start;
				}
			}
		}
	}
}
