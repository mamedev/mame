// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "filter.h"
#include "xmlfile.h"

// This effect implements a couple of very standard biquad filters,
// one lowpass and one highpass.

// Formulas taken from:

// [Zölzer 2011] "DAFX: Digital Audio Effects", Udo Zölzer, Second Edition, Wiley publishing, 2011 (Table 2.2)


audio_effect_filter::audio_effect_filter(u32 sample_rate, audio_effect *def) : audio_effect(sample_rate, def)
{
	// Minimal init to avoid using uninitialized values when reset_*
	// recomputes filters
	m_fl = m_fh = 1000;
	m_ql = m_qh = 0.7;

	reset_lowpass_active();
	reset_highpass_active();
	reset_fl();
	reset_fh();
	reset_ql();
	reset_qh();
}

void audio_effect_filter::reset_lowpass_active()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_lowpass_active = false;
	m_lowpass_active = d ? d->lowpass_active() : false;
	build_lowpass();
}

void audio_effect_filter::reset_highpass_active()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_highpass_active = false;
	m_highpass_active = d ? d->highpass_active() : true;
	build_highpass();
}

void audio_effect_filter::reset_fl()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_fl = false;
	m_fl = d ? d->fl() : 8000;
	build_lowpass();
}

void audio_effect_filter::reset_ql()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_ql = false;
	m_ql = d ? d->ql() : 0.7;
	build_lowpass();
}

void audio_effect_filter::reset_fh()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_fh = false;
	m_fh = d ? d->fh() : 40;
	build_highpass();
}

void audio_effect_filter::reset_qh()
{
	audio_effect_filter *d = static_cast<audio_effect_filter *>(m_default);
	m_isset_qh = false;
	m_qh = d ? d->qh() : 0.7;
	build_highpass();
}

void audio_effect_filter::config_load(util::xml::data_node const *ef_node)
{
	if(ef_node->has_attribute("lowpass_active")) {
		m_lowpass_active = ef_node->get_attribute_int("lowpass_active", 0);
		m_isset_lowpass_active = true;
	} else
		reset_lowpass_active();

	if(ef_node->has_attribute("fl")) {
		m_fl = ef_node->get_attribute_float("fl", 0);
		m_isset_fl = true;
	} else
		reset_fl();

	if(ef_node->has_attribute("ql")) {
		m_ql = ef_node->get_attribute_float("ql", 0);
		m_isset_ql = true;
	} else
		reset_ql();

	if(ef_node->has_attribute("highpass_active")) {
		m_highpass_active = ef_node->get_attribute_int("highpass_active", 0);
		m_isset_highpass_active = true;
	} else
		reset_highpass_active();

	if(ef_node->has_attribute("fh")) {
		m_fh = ef_node->get_attribute_float("fh", 0);
		m_isset_fh = true;
	} else
		reset_fh();

	if(ef_node->has_attribute("qh")) {
		m_qh = ef_node->get_attribute_float("qh", 0);
		m_isset_qh = true;
	} else
		reset_qh();
}

void audio_effect_filter::config_save(util::xml::data_node *ef_node) const
{
	if(m_isset_lowpass_active)
		ef_node->set_attribute_int("lowpass_active", m_lowpass_active);
	if(m_isset_fl)
		ef_node->set_attribute_float("fl", m_fl);
	if(m_isset_ql)
		ef_node->set_attribute_float("ql", m_ql);
	if(m_isset_highpass_active)
		ef_node->set_attribute_int("highpass_active", m_highpass_active);
	if(m_isset_fh)
		ef_node->set_attribute_float("fh", m_fh);
	if(m_isset_qh)
		ef_node->set_attribute_float("qh", m_qh);
}

void audio_effect_filter::default_changed()
{
	if(!m_isset_lowpass_active)
		reset_lowpass_active();
	if(!m_isset_highpass_active)
		reset_highpass_active();
	if(!m_isset_fl)
		reset_fl();
	if(!m_isset_fh)
		reset_fh();
	if(!m_isset_ql)
		reset_ql();
	if(!m_isset_qh)
		reset_qh();
}

void audio_effect_filter::apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest)
{
	if(!m_lowpass_active && !m_highpass_active) {
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
			m_filter[0].apply(m_history[channel][0], m_history[channel][1]);
			m_filter[1].apply(m_history[channel][1], m_history[channel][2]);
			*destd++ = m_history[channel][2].m_v0;
		}
	}

	dest.commit(samples);

}

void audio_effect_filter::set_lowpass_active(bool active)
{
	m_isset_lowpass_active = true;
	m_lowpass_active = active;
	build_lowpass();
}

void audio_effect_filter::set_highpass_active(bool active)
{
	m_isset_highpass_active = true;
	m_highpass_active = active;
	build_highpass();
}

void audio_effect_filter::set_fl(float f)
{
	m_isset_fl = true;
	m_fl = f;
	build_lowpass();
}

void audio_effect_filter::set_fh(float f)
{
	m_isset_fh = true;
	m_fh = f;
	build_highpass();
}

void audio_effect_filter::set_ql(float q)
{
	m_isset_ql = true;
	m_ql = q;
	build_lowpass();
}

void audio_effect_filter::set_qh(float q)
{
	m_isset_qh = true;
	m_qh = q;
	build_highpass();
}

void audio_effect_filter::build_highpass()
{
	auto &fi = m_filter[0];
    if(!m_highpass_active) {
		fi.clear();
		return;
	}

    float K = tan(M_PI*m_fh/m_sample_rate);
	float K2 = K*K;
	float Q = m_qh;

	float d = K2*Q + K + Q;
	fi.m_b0 = Q/d;
	fi.m_b1 = -2*Q/d;
	fi.m_b2 = fi.m_b0;
	fi.m_a1 = 2*Q*(K2-1)/d;
	fi.m_a2 = (K2*Q - K + Q)/d;
}

void audio_effect_filter::build_lowpass()
{
	auto &fi = m_filter[1];
    if(!m_lowpass_active) {
		fi.clear();
		return;
	}

    float K = tan(M_PI*m_fl/m_sample_rate);
	float K2 = K*K;
	float Q = m_ql;

	float d = K2*Q + K + Q;
	fi.m_b0 = K2*Q/d;
	fi.m_b1 = 2*K2*Q /d;
	fi.m_b2 = fi.m_b0;
	fi.m_a1 = 2*Q*(K2-1)/d;
	fi.m_a2 = (K2*Q - K + Q)/d;
}

