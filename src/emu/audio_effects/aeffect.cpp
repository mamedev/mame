// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "aeffect.h"
#include "filter.h"
#include "compressor.h"
#include "reverb.h"
#include "eq.h"

const char *const audio_effect::effect_names[COUNT] = {
	"Filters",
	"Compressor",
	"Reverb",
	"Equalizer"
};

audio_effect *audio_effect::create(int type, u32 sample_rate, audio_effect *def)
{
	switch(type) {
	case FILTER:     return new audio_effect_filter    (sample_rate, def);
	case COMPRESSOR: return new audio_effect_compressor(sample_rate, def);
	case REVERB:     return new audio_effect_reverb    (sample_rate, def);
	case EQ:         return new audio_effect_eq        (sample_rate, def);
	}
	return nullptr;
}


void audio_effect::copy(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) const
{
	u32 samples = src.available_samples();
	dest.prepare_space(samples);
	u32 channels = src.channels();
	for(u32 channel = 0; channel != channels; channel++) {
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
