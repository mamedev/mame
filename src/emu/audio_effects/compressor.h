// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_COMPRESSOR_H
#define MAME_EMU_AUDIO_EFFECTS_COMPRESSOR_H

#include "aeffect.h"

class audio_effect_compressor : public audio_effect
{
public:
	audio_effect_compressor(u32 sample_rate, audio_effect *def);
	virtual ~audio_effect_compressor() = default;

	virtual int type() const override { return COMPRESSOR; }
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;
	virtual void config_load(util::xml::data_node const *ef_node) override;
	virtual void config_save(util::xml::data_node *ef_node) const override;
	virtual void default_changed() override;
};

#endif
