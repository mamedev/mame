// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "aeffect.h"

#include "compressor.h"
#include "eq.h"
#include "filter.h"
#include "reverb.h"

#include "util/language.h"


const char *const audio_effect::effect_names[COUNT] = {
	N_p("audio-effect", "Filters"),
	N_p("audio-effect", "Compressor"),
	N_p("audio-effect", "Reverb"),
	N_p("audio-effect", "Equalizer")
};

std::unique_ptr<audio_effect> audio_effect::create(int type, speaker_device *speaker, u32 sample_rate, audio_effect *def)
{
	switch(type) {
	case FILTER:     return std::make_unique<audio_effect_filter    >(speaker, sample_rate, def);
	case COMPRESSOR: return std::make_unique<audio_effect_compressor>(speaker, sample_rate, def);
	case REVERB:     return std::make_unique<audio_effect_reverb    >(speaker, sample_rate, def);
	case EQ:         return std::make_unique<audio_effect_eq        >(speaker, sample_rate, def);
	}
	return nullptr;
}


void audio_effect::copy(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) const
{
	u32 samples = src.available_samples();
	dest.prepare_space(samples);
	for(u32 channel = 0; channel != m_channels; channel++) {
		const sample_t *srcd = src.ptrs(channel, 0);
		sample_t *destd = dest.ptrw(channel, 0);
		std::copy(srcd, srcd + samples, destd);
	}
	dest.commit(samples);
}

u32 audio_effect::history_size() const
{
	return 0;
}
