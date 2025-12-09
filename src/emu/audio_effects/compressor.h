// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#pragma once

#ifndef MAME_EMU_AUDIO_EFFECTS_COMPRESSOR_H
#define MAME_EMU_AUDIO_EFFECTS_COMPRESSOR_H

#include "aeffect.h"

class audio_effect_compressor : public audio_effect
{
public:
	audio_effect_compressor(speaker_device *speaker, u32 sample_rate, audio_effect *def);
	virtual ~audio_effect_compressor() = default;

	virtual int type() const override { return COMPRESSOR; }
	virtual void apply(const emu::detail::output_buffer_flat<sample_t> &src, emu::detail::output_buffer_flat<sample_t> &dest) override;
	virtual void config_load(util::xml::data_node const *ef_node) override;
	virtual void config_save(util::xml::data_node *ef_node) const override;
	virtual void default_changed() override;

	void set_mode(u32 mode);
	void set_attack(float val);
	void set_release(float val);
	void set_ratio(float val);
	void set_input_gain(float val);
	void set_output_gain(float val);
	void set_convexity(float val);
	void set_threshold(float val);
	void set_channel_link(float val);
	void set_feedback(float val);
	void set_inertia(float val);
	void set_inertia_decay(float val);
	void set_ceiling(float val);

	u32 mode() const      { return m_mode; }
	float attack()        { return m_attack; }
	float release()       { return m_release; }
	float ratio()         { return m_ratio; }
	float input_gain()    { return m_input_gain; }
	float output_gain()   { return m_output_gain; }
	float convexity()     { return m_convexity; }
	float threshold()     { return m_threshold; }
	float channel_link()  { return m_channel_link; }
	float feedback()      { return m_feedback; }
	float inertia()       { return m_inertia; }
	float inertia_decay() { return m_inertia_decay; }
	float ceiling()       { return m_ceiling; }

	bool isset_mode() const          { return m_isset_mode; }
	bool isset_attack() const        { return m_isset_attack; }
	bool isset_release() const       { return m_isset_release; }
	bool isset_ratio() const         { return m_isset_ratio; }
	bool isset_input_gain() const    { return m_isset_input_gain; }
	bool isset_output_gain() const   { return m_isset_output_gain; }
	bool isset_convexity() const     { return m_isset_convexity; }
	bool isset_threshold() const     { return m_isset_threshold; }
	bool isset_channel_link() const  { return m_isset_channel_link; }
	bool isset_feedback() const      { return m_isset_feedback; }
	bool isset_inertia() const       { return m_isset_inertia; }
	bool isset_inertia_decay() const { return m_isset_inertia_decay; }
	bool isset_ceiling() const       { return m_isset_ceiling; }

	void reset_mode();
	void reset_attack();
	void reset_release();
	void reset_ratio();
	void reset_input_gain();
	void reset_output_gain();
	void reset_convexity();
	void reset_threshold();
	void reset_channel_link();
	void reset_feedback();
	void reset_inertia();
	void reset_inertia_decay();
	void reset_ceiling();
	void reset_all();

private:
	u32 m_mode;
	float m_attack;
	float m_release;
	float m_ratio;
	float m_input_gain;
	float m_output_gain;
	float m_convexity;
	float m_threshold;
	float m_channel_link;
	float m_feedback;
	float m_inertia;
	float m_inertia_decay;
	float m_ceiling;

	bool m_isset_mode;
	bool m_isset_attack;
	bool m_isset_release;
	bool m_isset_ratio;
	bool m_isset_input_gain;
	bool m_isset_output_gain;
	bool m_isset_convexity;
	bool m_isset_threshold;
	bool m_isset_channel_link;
	bool m_isset_feedback;
	bool m_isset_inertia;
	bool m_isset_inertia_decay;
	bool m_isset_ceiling;

	std::vector<float> m_slewed_signal;
	std::vector<float> m_gain_reduction;
	std::vector<float> m_input_samples;
	std::vector<float> m_output_samples;
	std::vector<float> m_inertia_velocity;

	static double db_to_value(double db);
	static double value_to_db(double value);
};

#endif
