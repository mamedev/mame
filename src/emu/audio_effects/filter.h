// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_FILTER_H
#define MAME_EMU_AUDIO_EFFECTS_FILTER_H

#include "aeffect.h"

class audio_effect_filter : public audio_effect
{
public:
	audio_effect_filter(u32 sample_rate, audio_effect *def);
	virtual ~audio_effect_filter() = default;

	virtual int type() const override { return FILTER; }
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;
	virtual void config_load(util::xml::data_node const *ef_node) override;
	virtual void config_save(util::xml::data_node *ef_node) const override;
	virtual void default_changed() override;

	void set_lowpass_active(bool active);
	void set_highpass_active(bool active);
	void set_fl(float f);
	void set_fh(float f);
	void set_ql(float q);
	void set_qh(float q);

	bool lowpass_active() const  { return m_lowpass_active; }
	bool highpass_active() const { return m_highpass_active; }
	float fl() const { return m_fl; }
	float fh() const { return m_fh; }
	float ql() const { return m_ql; }
	float qh() const { return m_qh; }

	bool isset_lowpass_active() const  { return m_isset_lowpass_active; }
	bool isset_highpass_active() const { return m_isset_highpass_active; }
	bool isset_fl() const { return m_isset_fl; }
	bool isset_fh() const { return m_isset_fh; }
	bool isset_ql() const { return m_isset_ql; }
	bool isset_qh() const { return m_isset_qh; }

	void reset_lowpass_active();
	void reset_highpass_active();
	void reset_fl();
	void reset_fh();
	void reset_ql();
	void reset_qh();

private:
	struct history {
		float m_v0, m_v1, m_v2;
		history() { m_v0 = m_v1 = m_v2 = 0; }
		void push(float v) { m_v2 = m_v1; m_v1 = m_v0; m_v0 = v; }
	};

	struct filter {
		float m_a1, m_a2, m_b0, m_b1, m_b2;
		void clear() { m_a1 = 0; m_a2 = 0; m_b0 = 1; m_b1 = 0; m_b2 = 0; }
		void apply(history &x, history &y) const {
			y.push(m_b0 * x.m_v0 + m_b1 * x.m_v1 + m_b2 * x.m_v2 - m_a1 * y.m_v0 - m_a2 * y.m_v1);
		}
	};

	bool m_isset_lowpass_active, m_isset_highpass_active;
	bool m_isset_fl, m_isset_fh, m_isset_ql, m_isset_qh;

	bool m_lowpass_active, m_highpass_active;
	float m_fl, m_fh, m_ql, m_qh;
	std::array<filter, 2> m_filter;
	std::vector<std::array<history, 3>> m_history;

	void build_lowpass();
	void build_highpass();
};

#endif
