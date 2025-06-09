// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "reverb.h"
#include "xmlfile.h"

audio_effect_reverb::audio_effect_reverb(u32 sample_rate, audio_effect *def) : audio_effect(sample_rate, def)
{
}


void audio_effect_reverb::config_load(util::xml::data_node const *ef_node)
{
}

void audio_effect_reverb::config_save(util::xml::data_node *ef_node) const
{
}

void audio_effect_reverb::default_changed()
{
}

void audio_effect_reverb::apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest)
{
	copy(src, dest);
}
