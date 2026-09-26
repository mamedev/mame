// license:GPL2+
// copyright-holders:Felipe Sanches

// Shared register file and gating for the KN6000/KN7000 tone generators. The
// synthesis datapath is not modelled here, so the stream is silent.

#include "emu.h"
#include "kn_tonegen.h"

kn_tonegen_base_device::kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int num_voices)
	: device_t(mconfig, type, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_num_voices(num_voices)
{
}

void kn_tonegen_base_device::key_break(uint8_t key)
{
	for (int v = 0; v < m_num_voices; v++)
		if (m_gate[v] && m_mode[v] == 1 && m_srckey[v] == key)
			m_gate[v] = 0;
}

void kn_tonegen_base_device::device_start()
{
	m_stream = stream_alloc(0, 2, 44100);
	std::fill(std::begin(m_gate),  std::end(m_gate),  0);
	std::fill(std::begin(m_level), std::end(m_level), 1.0);
	std::fill(std::begin(m_srckey), std::end(m_srckey), 0xFF);
	save_item(NAME(m_gate));
	save_item(NAME(m_level));
	save_item(NAME(m_envreg));
	save_item(NAME(m_eg012));
	save_item(NAME(m_ton));
	save_item(NAME(m_aux));
	save_item(NAME(m_mode));
	save_item(NAME(m_busreg));
	save_item(NAME(m_srckey));
	save_item(NAME(m_ctx_key));
	save_item(NAME(m_ctx_time));
	save_item(NAME(m_tgwrites));
	save_item(NAME(m_gain_direct));
	save_item(NAME(m_gain_return));
	save_item(NAME(m_gain_send));
	save_item(NAME(m_gain_depth));
	save_item(NAME(m_gain_chorus));
	save_item(NAME(m_gain_dsp));
	save_item(NAME(m_gain_multi));
	save_item(NAME(m_gain_dsp_ret));
	save_item(NAME(m_gain_multi_ret));
}

void kn_tonegen_base_device::sound_stream_update(sound_stream &stream)
{
	stream.fill(0, 0.0);
	stream.fill(1, 0.0);
}
