// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_EQ_H
#define MAME_EMU_AUDIO_EFFECTS_EQ_H

#include "aeffect.h"

class audio_effect_eq : public audio_effect
{
public:
	enum { BANDS = 5 };

	audio_effect_eq(u32 sample_rate, audio_effect *def);
	virtual ~audio_effect_eq() = default;

	virtual int type() const override { return EQ; }
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;
	virtual void config_load(util::xml::data_node const *ef_node) override;
	virtual void config_save(util::xml::data_node *ef_node) const override;
	virtual void default_changed() override;

	void set_mode(u32 mode);
	void set_q(u32 band, float q);
	void set_f(u32 band, float f);
	void set_db(u32 band, float db);
	void set_low_shelf(bool active);
	void set_high_shelf(bool active);

	u32 mode() const         { return m_mode; }
	float q(u32 band) const  { return m_q[band]; }
	float f(u32 band) const  { return m_f[band]; }
	float db(u32 band) const { return m_db[band]; }
	bool low_shelf() const   { return m_low_shelf; }
	bool high_shelf() const  { return m_high_shelf; }

	bool isset_mode() const       { return m_isset_mode; }
	bool isset_q(u32 band) const  { return m_isset_q[band]; }
	bool isset_f(u32 band) const  { return m_isset_f[band]; }
	bool isset_db(u32 band) const { return m_isset_db[band]; }
	bool isset_low_shelf() const  { return m_isset_low_shelf; }
	bool isset_high_shelf() const { return m_isset_high_shelf; }

	void reset_mode();
	void reset_q(u32 band);
	void reset_f(u32 band);
	void reset_db(u32 band);
	void reset_low_shelf();
	void reset_high_shelf();

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

	u32 m_mode;
	float m_q[BANDS], m_f[BANDS], m_db[BANDS];
	bool m_low_shelf, m_high_shelf;
	std::array<filter, BANDS> m_filter;
	std::vector<std::array<history, BANDS+1>> m_history;

	bool m_isset_mode, m_isset_low_shelf, m_isset_high_shelf;
	bool m_isset_q[BANDS], m_isset_f[BANDS], m_isset_db[BANDS];

	void build_filter(u32 band);

	void build_low_shelf(u32 band);
	void build_high_shelf(u32 band);
	void build_peak(u32 band);
};

#endif
