// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Tone generators of the KN2400, KN6000 and KN7000 families. The register
// interface is decoded; the synthesis datapath is not emulated, so the stream is
// silent.

#include "emu.h"
#include "kn_tonegen.h"

DEFINE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device, "kn6000_tonegen", "KN6000 tone generator")
DEFINE_DEVICE_TYPE(KN2400_TONEGEN, kn2400_tonegen_device, "kn2400_tonegen", "KN2400 tone generator")
DEFINE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device, "kn7000_tonegen", "KN7000 tone generator")

kn_tonegen_base_device::kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int chips)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_chips(chips)
	, m_stream(nullptr)
{
}

void kn_tonegen_base_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);
	m_regs = make_unique_clear<uint16_t []>(m_chips * 0x10000);
	save_pointer(NAME(m_regs), m_chips * 0x10000);
}

void kn_tonegen_base_device::tg_write(int chip, uint16_t addr, uint16_t data)
{
	assert(chip < m_chips);
	m_stream->update();
	m_regs[chip * 0x10000 + addr] = data;
}

void kn_tonegen_base_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, 0.0);
	stream.fill(1, 0.0);
}

kn6000_tonegen_device::kn6000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN6000_TONEGEN, tag, owner, clock, 1)
{
}

kn2400_tonegen_device::kn2400_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN2400_TONEGEN, tag, owner, clock, 1)
{
}

kn7000_tonegen_device::kn7000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: kn_tonegen_base_device(mconfig, KN7000_TONEGEN, tag, owner, clock, 2)
{
}
