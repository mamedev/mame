// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "eq.h"
#include "xmlfile.h"

// This effect implements a parametric EQ using peak and shelf filters

// Formulas taken from (with some fixes):

// [Zölzer 2011] "DAFX: Digital Audio Effects", Udo Zölzer, Second Edition, Wiley publishing, 2011 (Tables 2.3 and 2.4)
// [Zölzer 2008] "Digital Audio Signal Processing", Udo Zölzer, Second Edition, Wiley publishing, 2008 (Tables 5.3, 5.4 and 5.5)

audio_effect_eq::audio_effect_eq(u32 sample_rate, audio_effect *def) : audio_effect(sample_rate, def)
{
	// Minimal init to avoid using uninitialized values when reset_*
	// recomputes filters

	for(u32 band = 0; band != BANDS; band++) {
		m_q[band] = 0.7;
		m_f[band] = 1000;
		m_db[band] = 0;
	}

	reset_mode();
	reset_low_shelf();
	reset_high_shelf();
	for(u32 band = 0; band != BANDS; band++) {
		reset_q(band);
		reset_f(band);
		reset_db(band);
	}
}

void audio_effect_eq::reset_mode()
{
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_mode = false;
	m_mode = d ? d->mode() : 1;
}

void audio_effect_eq::reset_q(u32 band)
{
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_q[band] = false;
	m_q[band] = d ? d->q(band) : 0.7;
	build_filter(band);
}

void audio_effect_eq::reset_f(u32 band)
{
	static const u32 defs[BANDS] = { 80, 200, 500, 3200, 8000 };
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_f[band] = false;
	m_f[band] = d ? d->f(band) : defs[band];
	build_filter(band);
}

void audio_effect_eq::reset_db(u32 band)
{
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_db[band] = false;
	m_db[band] = d ? d->db(band) : 0;
	build_filter(band);
}

void audio_effect_eq::reset_low_shelf()
{
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_low_shelf = false;
	m_low_shelf = d ? d->low_shelf() : true;
	build_filter(0);
}

void audio_effect_eq::reset_high_shelf()
{
	audio_effect_eq *d = static_cast<audio_effect_eq *>(m_default);
	m_isset_high_shelf = false;
	m_high_shelf = d ? d->high_shelf() : true;
	build_filter(BANDS-1);
}

void audio_effect_eq::config_load(util::xml::data_node const *ef_node)
{
	if(ef_node->has_attribute("mode")) {
		m_mode = ef_node->get_attribute_int("mode", 0);
		m_isset_mode = true;
	} else
		reset_mode();

	if(ef_node->has_attribute("low_shelf")) {
		m_low_shelf = ef_node->get_attribute_int("low_shelf", 0);
		m_isset_low_shelf = true;
	} else
		reset_low_shelf();

	if(ef_node->has_attribute("high_shelf")) {
		m_high_shelf = ef_node->get_attribute_int("high_shelf", 0);
		m_isset_high_shelf = true;
	} else
		reset_high_shelf();

	for(u32 band = 0; band != BANDS; band++) {
		if(ef_node->has_attribute(util::string_format("q%d", band+1).c_str())) {
			m_q[band] = ef_node->get_attribute_float(util::string_format("q%d", band+1).c_str(), 0);
			m_isset_q[band] = true;
		} else
			reset_q(band);

		if(ef_node->has_attribute(util::string_format("f%d", band+1).c_str())) {
			m_f[band] = ef_node->get_attribute_float(util::string_format("f%d", band+1).c_str(), 0);
			m_isset_f[band] = true;
		} else
			reset_f(band);

		if(ef_node->has_attribute(util::string_format("db%d", band+1).c_str())) {
			m_db[band] = ef_node->get_attribute_float(util::string_format("db%d", band+1).c_str(), 0);
			m_isset_db[band] = true;
		} else
			reset_db(band);
	}
}

void audio_effect_eq::config_save(util::xml::data_node *ef_node) const
{
	if(m_isset_mode)
		ef_node->set_attribute_int("mode", m_mode);
	if(m_isset_low_shelf)
		ef_node->set_attribute_int("low_shelf", m_low_shelf);
	if(m_isset_high_shelf)
		ef_node->set_attribute_int("high_shelf", m_high_shelf);
	for(u32 band = 0; band != BANDS; band++) {
		if(m_isset_q[band])
			ef_node->set_attribute_float(util::string_format("q%d", band+1).c_str(), m_q[band]);
		if(m_isset_f[band])
			ef_node->set_attribute_float(util::string_format("f%d", band+1).c_str(), m_f[band]);
		if(m_isset_db[band])
			ef_node->set_attribute_float(util::string_format("db%d", band+1).c_str(), m_db[band]);
	}
}

void audio_effect_eq::default_changed()
{
	if(!m_default)
		return;
	if(!m_isset_mode)
		reset_mode();
	if(!m_isset_low_shelf)
		reset_low_shelf();
	if(!m_isset_high_shelf)
		reset_high_shelf();
	for(u32 band = 0; band != BANDS; band++) {
		if(!m_isset_q[band])
			reset_q(band);
		if(!m_isset_f[band])
			reset_f(band);
		if(!m_isset_db[band])
			reset_db(band);
	}
}

void audio_effect_eq::set_mode(u32 mode)
{
	m_isset_mode = true;
	m_mode = mode;
}

void audio_effect_eq::set_q(u32 band, float q)
{
	m_isset_q[band] = true;
	m_q[band] = q;
	build_filter(band);
}

void audio_effect_eq::set_f(u32 band, float f)
{
	m_isset_f[band] = true;
	m_f[band] = f;
	build_filter(band);
}

void audio_effect_eq::set_db(u32 band, float db)
{
	m_isset_db[band] = true;
	m_db[band] = db;
	build_filter(band);
}

void audio_effect_eq::set_low_shelf(bool active)
{
	m_isset_low_shelf = true;
	m_low_shelf = active;
	build_filter(0);
}

void audio_effect_eq::set_high_shelf(bool active)
{
	m_isset_high_shelf = true;
	m_high_shelf = active;
	build_filter(BANDS-1);
}

void audio_effect_eq::build_filter(u32 band)
{
	if(band == 0 && m_low_shelf) {
		build_low_shelf(band);
		return;
	}
	if(band == BANDS-1 && m_high_shelf) {
		build_high_shelf(band);
		return;
	}
	build_peak(band);
}

void audio_effect_eq::build_low_shelf(u32 band)
{
	auto &fi = m_filter[band];
    if(m_db[band] == 0) {
		fi.clear();
		return;
	}

	float V = pow(10, abs(m_db[band])/20);
    float K = tan(M_PI*m_f[band]/m_sample_rate);
	float K2 = K*K;

	if(m_db[band] > 0) {
        float d = 1 + sqrt(2)*K + K2;
        fi.m_b0 = (1 + sqrt(2*V)*K + V*K2)/d;
        fi.m_b1 = 2*(V*K2-1)/d;
        fi.m_b2 = (1 - sqrt(2*V)*K + V*K2)/d;
        fi.m_a1 = 2*(K2-1)/d;
        fi.m_a2 = (1 - sqrt(2)*K + K2)/d;
    } else {
        float d = 1 + sqrt(2*V)*K + V*K2;
        fi.m_b0 = (1 + sqrt(2)*K + K2)/d;
        fi.m_b1 = 2*(K2-1)/d;
        fi.m_b2 = (1 - sqrt(2)*K + K2)/d;
        fi.m_a1 = 2*(V*K2-1)/d;
        fi.m_a2 = (1 - sqrt(2*V)*K + V*K2)/d;
	}
}

void audio_effect_eq::build_high_shelf(u32 band)
{
	auto &fi = m_filter[band];
    if(m_db[band] == 0) {
		fi.clear();
		return;
	}

	float V = pow(10, m_db[band]/20);
    float K = tan(M_PI*m_f[band]/m_sample_rate);
	float K2 = K*K;

	if(m_db[band] > 0) {
		float d = 1 + sqrt(2)*K + K2;
		fi.m_b0 = (V + sqrt(2*V)*K + K2)/d;
		fi.m_b1 = 2*(K2-V)/d;
		fi.m_b2 = (V - sqrt(2*V)*K + K2)/d;
		fi.m_a1 = 2*(K2-1)/d;
		fi.m_a2 = (1 - sqrt(2)*K + K2)/d;
    } else {
		float d = 1 + sqrt(2*V)*K + V*K2;
		fi.m_b0 = V*(1 + sqrt(2)*K + K2)/d;
		fi.m_b1 = 2*V*(K2-1)/d;
		fi.m_b2 = V*(1 - sqrt(2)*K + K2)/d;
		fi.m_a1 = 2*(V*K2-1)/d;
		fi.m_a2 = (1 - sqrt(2*V)*K + V*K2)/d;
	}
}

void audio_effect_eq::build_peak(u32 band)
{
	auto &fi = m_filter[band];
    if(m_db[band] == 0) {
		fi.clear();
		return;
	}

	float V = pow(10, m_db[band]/20);
    float K = tan(M_PI*m_f[band]/m_sample_rate);
	float K2 = K*K;
	float Q = m_q[band];

	if(m_db[band] > 0) {
		float d = 1 + K/Q + K2;
		fi.m_b0 = (1 + V*K/Q + K2)/d;
		fi.m_b1 = 2*(K2-1)/d;
		fi.m_b2 = (1 - V*K/Q + K2)/d;
		fi.m_a1 = fi.m_b1;
		fi.m_a2 = (1 - K/Q + K2)/d;
    } else {
		float d = 1 + K/(V*Q) + K2;
		fi.m_b0 = (1 + K/Q + K2)/d;
		fi.m_b1 = 2*(K2-1)/d;
		fi.m_b2 = (1 - K/Q + K2)/d;
		fi.m_a1 = fi.m_b1;
		fi.m_a2 = (1 - K/(V*Q) + K2)/d;
	}
}


void audio_effect_eq::apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest)
{
	if(m_mode == 0) {
		copy(src, dest);
		return;
	}

	u32 samples = src.available_samples();
	dest.prepare_space(samples);
	u32 channels = src.channels();
	if(m_history.empty())
		m_history.resize(channels);

	for(u32 channel = 0; channel != channels; channel++) {
		const sample_t *srcd = src.ptrs(channel, 0);
		sample_t *destd = dest.ptrw(channel, 0);
		for(u32 sample = 0; sample != samples; sample++) {
			m_history[channel][0].push(*srcd++);
			for(u32 band = 0; band != BANDS; band++)
				m_filter[band].apply(m_history[channel][band], m_history[channel][band+1]);
			*destd++ = m_history[channel][BANDS].m_v0;
		}
	}

	dest.commit(samples);
}
