// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Very inspired by https://github.com/apmastering/APComp

#include "emu.h"
#include "compressor.h"
#include "xmlfile.h"

audio_effect_compressor::audio_effect_compressor(speaker_device *speaker, u32 sample_rate, audio_effect *def) :
	audio_effect(speaker, sample_rate, def)
{
	m_slewed_signal.resize(m_channels, -200);
	m_gain_reduction.resize(m_channels, 0);
	m_input_samples.resize(m_channels, 0);
	m_output_samples.resize(m_channels, 0);
	m_inertia_velocity.resize(m_channels, 0);

	reset_all();
}

void audio_effect_compressor::config_load(util::xml::data_node const *ef_node)
{
	if(ef_node->has_attribute("mode")) {
		m_mode = ef_node->get_attribute_int("mode", 0);
		m_isset_mode = true;
	} else
		reset_mode();

	if(ef_node->has_attribute("attack")) {
		m_attack = ef_node->get_attribute_float("attack", 0);
		m_isset_attack = true;
	} else
		reset_attack();

	if(ef_node->has_attribute("release")) {
		m_release = ef_node->get_attribute_float("release", 0);
		m_isset_release = true;
	} else
		reset_release();

	if(ef_node->has_attribute("ratio")) {
		m_ratio = ef_node->get_attribute_float("ratio", 0);
		m_isset_ratio = true;
	} else
		reset_ratio();

	if(ef_node->has_attribute("input_gain")) {
		m_input_gain = ef_node->get_attribute_float("input_gain", 0);
		m_isset_input_gain = true;
	} else
		reset_input_gain();

	if(ef_node->has_attribute("output_gain")) {
		m_output_gain = ef_node->get_attribute_float("output_gain", 0);
		m_isset_output_gain = true;
	} else
		reset_output_gain();

	if(ef_node->has_attribute("convexity")) {
		m_convexity = ef_node->get_attribute_float("convexity", 0);
		m_isset_convexity = true;
	} else
		reset_convexity();

	if(ef_node->has_attribute("threshold")) {
		m_threshold = ef_node->get_attribute_float("threshold", 0);
		m_isset_threshold = true;
	} else
		reset_threshold();

	if(ef_node->has_attribute("channel_link")) {
		m_channel_link = ef_node->get_attribute_float("channel_link", 0);
		m_isset_channel_link = true;
	} else
		reset_channel_link();

	if(ef_node->has_attribute("feedback")) {
		m_feedback = ef_node->get_attribute_float("feedback", 0);
		m_isset_feedback = true;
	} else
		reset_feedback();

	if(ef_node->has_attribute("inertia")) {
		m_inertia = ef_node->get_attribute_float("inertia", 0);
		m_isset_inertia = true;
	} else
		reset_inertia();

	if(ef_node->has_attribute("inertia_decay")) {
		m_inertia_decay = ef_node->get_attribute_float("inertia_decay", 0);
		m_isset_inertia_decay = true;
	} else
		reset_inertia_decay();

	if(ef_node->has_attribute("ceiling")) {
		m_ceiling = ef_node->get_attribute_float("ceiling", 0);
		m_isset_ceiling = true;
	} else
		reset_ceiling();
}

void audio_effect_compressor::config_save(util::xml::data_node *ef_node) const
{
	if(m_isset_mode)
		ef_node->set_attribute_int("mode", m_mode);
	if(m_isset_attack)
		ef_node->set_attribute_float("attack", m_attack);
	if(m_isset_release)
		ef_node->set_attribute_float("release", m_release);
	if(m_isset_ratio)
		ef_node->set_attribute_float("ratio", m_ratio);
	if(m_isset_input_gain)
		ef_node->set_attribute_float("input_gain", m_input_gain);
	if(m_isset_output_gain)
		ef_node->set_attribute_float("output_gain", m_output_gain);
	if(m_isset_convexity)
		ef_node->set_attribute_float("convexity", m_convexity);
	if(m_isset_threshold)
		ef_node->set_attribute_float("threshold", m_threshold);
	if(m_isset_channel_link)
		ef_node->set_attribute_float("channel_link", m_channel_link);
	if(m_isset_feedback)
		ef_node->set_attribute_float("feedback", m_feedback);
	if(m_isset_inertia)
		ef_node->set_attribute_float("inertia", m_inertia);
	if(m_isset_inertia_decay)
		ef_node->set_attribute_float("inertia_decay", m_inertia_decay);
	if(m_isset_ceiling)
		ef_node->set_attribute_float("ceiling", m_ceiling);
}

void audio_effect_compressor::default_changed()
{
	if(!m_default)
		return;
	if(!m_isset_mode)
		reset_mode();
	if(!m_isset_attack)
		reset_attack();
	if(!m_isset_release)
		reset_release();
	if(!m_isset_ratio)
		reset_ratio();
	if(!m_isset_input_gain)
		reset_input_gain();
	if(!m_isset_output_gain)
		reset_output_gain();
	if(!m_isset_convexity)
		reset_convexity();
	if(!m_isset_threshold)
		reset_threshold();
	if(!m_isset_channel_link)
		reset_channel_link();
	if(!m_isset_feedback)
		reset_feedback();
	if(!m_isset_inertia)
		reset_inertia();
	if(!m_isset_inertia_decay)
		reset_inertia_decay();
	if(!m_isset_ceiling)
		reset_ceiling();
}

void audio_effect_compressor::set_mode(u32 mode)
{
	m_isset_mode = true;
	m_mode = mode;
}

void audio_effect_compressor::set_attack(float val)
{
	m_isset_attack = true;
	m_attack = val;
}

void audio_effect_compressor::set_release(float val)
{
	m_isset_release = true;
	m_release = val;
}

void audio_effect_compressor::set_ratio(float val)
{
	m_isset_ratio = true;
	m_ratio = val;
}

void audio_effect_compressor::set_input_gain(float val)
{
	m_isset_input_gain = true;
	m_input_gain = val;
}

void audio_effect_compressor::set_output_gain(float val)
{
	m_isset_output_gain = true;
	m_output_gain = val;
}

void audio_effect_compressor::set_convexity(float val)
{
	m_isset_convexity = true;
	m_convexity = val;
}

void audio_effect_compressor::set_threshold(float val)
{
	m_isset_threshold = true;
	m_threshold = val;
}

void audio_effect_compressor::set_channel_link(float val)
{
	m_isset_channel_link = true;
	m_channel_link = val;
}

void audio_effect_compressor::set_feedback(float val)
{
	m_isset_feedback = true;
	m_feedback = val;
}

void audio_effect_compressor::set_inertia(float val)
{
	m_isset_inertia = true;
	m_inertia = val;
}

void audio_effect_compressor::set_inertia_decay(float val)
{
	m_isset_inertia_decay = true;
	m_inertia_decay = val;
}

void audio_effect_compressor::set_ceiling(float val)
{
	m_isset_ceiling = true;
	m_ceiling = val;
}

void audio_effect_compressor::reset_mode()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_mode = false;
	m_mode = d ? d->mode() : 0;
}

void audio_effect_compressor::reset_attack()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_attack = false;
	m_attack = d ? d->attack() : 90;
}

void audio_effect_compressor::reset_release()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_release = false;
	m_release = d ? d->release() : 400;
}

void audio_effect_compressor::reset_ratio()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_ratio = false;
	m_ratio = d ? d->ratio() : 4;
}

void audio_effect_compressor::reset_input_gain()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_input_gain = false;
	m_input_gain = d ? d->input_gain() : 0;
}

void audio_effect_compressor::reset_output_gain()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_output_gain = false;
	m_output_gain = d ? d->output_gain() : 0;
}

void audio_effect_compressor::reset_convexity()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_convexity = false;
	m_convexity = d ? d->convexity() : 1;
}

void audio_effect_compressor::reset_threshold()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_threshold = false;
	m_threshold = d ? d->threshold() : -15;
}

void audio_effect_compressor::reset_channel_link()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_channel_link = false;
	m_channel_link = d ? d->channel_link() : 1;
}

void audio_effect_compressor::reset_feedback()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_feedback = false;
	m_feedback = d ? d->feedback() : 0;
}

void audio_effect_compressor::reset_inertia()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_inertia = false;
	m_inertia = d ? d->inertia() : 0;
}

void audio_effect_compressor::reset_inertia_decay()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_inertia_decay = false;
	m_inertia_decay = d ? d->inertia_decay() : 0.94;
}

void audio_effect_compressor::reset_ceiling()
{
	audio_effect_compressor *d = static_cast<audio_effect_compressor *>(m_default);
	m_isset_ceiling = false;
	m_ceiling = d ? d->ceiling() : 1;
}

void audio_effect_compressor::reset_all()
{
	reset_mode();
	reset_attack();
	reset_release();
	reset_ratio();
	reset_input_gain();
	reset_output_gain();
	reset_convexity();
	reset_threshold();
	reset_channel_link();
	reset_feedback();
	reset_inertia();
	reset_inertia_decay();
	reset_ceiling();
}

double audio_effect_compressor::db_to_value(double db)
{
	if(db <= -1000)
		return 0;
	if(db > 1000)
		db = 1000;
	return pow(10, db/20);
}

double audio_effect_compressor::value_to_db(double value)
{
	if(value <= 0)
		return -1000;
	if(value > 1000)
		value = 1000;
	return 20 * log10(value);
}

void audio_effect_compressor::apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest)
{
	if(m_mode == 0) {
		copy(src, dest);
		return;
	}

	u32 samples = src.available_samples();
	dest.prepare_space(samples);

	double attack_coefficient = exp(-1/(m_sample_rate * m_attack / 1000));
	double release_coefficient = exp(-1/(m_sample_rate * m_release / 1000));
	double m_inertia_decay_coefficient = 0.99 + m_inertia_decay * 0.01;

	for(u32 sample = 0; sample != samples; sample ++) {
		for(u32 channel = 0; channel != m_channels; channel ++) {
			double input_db = value_to_db(std::abs(*src.ptrs(channel, sample))) + m_input_gain + std::abs(m_output_samples[channel]) * m_feedback;
			m_output_samples[channel] = 0;

			if(std::isnan(input_db) || input_db < -200)
				input_db = -200;
			else if(input_db > 4)
				input_db = 4;

			float slewed_signal = m_slewed_signal[channel];
			if(input_db > slewed_signal)
				slewed_signal = attack_coefficient * (slewed_signal - input_db) + input_db;
			else
				slewed_signal = release_coefficient * (slewed_signal - input_db) + input_db;

			m_input_samples[channel] = input_db;

			double gain_reduction;
			if(slewed_signal > m_threshold) {
				double target = m_threshold + (slewed_signal - m_threshold) / m_ratio;
				gain_reduction = pow(slewed_signal - target, m_convexity);
			} else {
				slewed_signal = m_threshold;
				gain_reduction = 0;
			}

			if(slewed_signal < -200)
				slewed_signal = -200;
			else if(slewed_signal > 1000)
				slewed_signal = 1000;

			m_slewed_signal[channel] = slewed_signal;

			double inertia_velocity = m_inertia_velocity[channel];
			double gain_reduction_value = db_to_value(gain_reduction);
			if(m_inertia <= 0 || gain_reduction > m_gain_reduction[channel])
				inertia_velocity -= m_inertia * gain_reduction_value * 0.001;

			inertia_velocity *= m_inertia_decay_coefficient;
			if(inertia_velocity < -100)
				inertia_velocity = -100;
			else if(inertia_velocity > 100)
				inertia_velocity = 100;

			gain_reduction_value += inertia_velocity;
			if(gain_reduction_value < -1000)
				gain_reduction_value = -1000;
			else if(gain_reduction_value > 1000)
				gain_reduction_value = 1000;

			m_gain_reduction[channel] = value_to_db(gain_reduction_value);
		}

		double max_gain = m_gain_reduction[0];
		for(u32 channel = 1; channel != m_channels; channel++)
			if(m_gain_reduction[channel] > max_gain)
				max_gain = m_gain_reduction[channel];

		for(u32 channel = 0; channel != m_channels; channel ++) {
			m_gain_reduction[channel] = max_gain * m_channel_link + m_gain_reduction[channel] * (1 - m_channel_link);
			double output_sample = db_to_value(m_input_samples[channel] - m_gain_reduction[channel]);
			if(*src.ptrs(channel, sample) < 0)
				output_sample = -output_sample;

			output_sample = tanh(output_sample / m_ceiling) * m_ceiling * db_to_value(m_output_gain);
			*dest.ptrw(channel, sample) = output_sample;
			m_output_samples[channel] = output_sample;
		}
	}

	dest.commit(samples);
}
