// license:BSD-3-Clause
// copyright-holders:m1macrophage

// The Xpander vioce board and voice devices.

#ifndef MAME_OBERHEIM_XPANDER_VB_H
#define MAME_OBERHEIM_XPANDER_VB_H

#pragma once

#include "cpu/m6809/m6809.h"
#include "machine/pit8253.h"
#include "sound/cem3372.h"
#include "sound/flt_rc.h"
#include "sound/mixer.h"
#include "sound/va_eg.h"

DECLARE_DEVICE_TYPE(XPANDER_VOICE, xpander_voice_device)
DECLARE_DEVICE_TYPE(XPANDER_VOICEBOARD, xpandervb_device)


// A single voice on the Xpander voice board, revision D.
class xpander_voice_device : public device_t, public device_sound_interface
{
public:
	xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, device_sound_interface *noise) ATTR_COLD;
	xpander_voice_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	double cem3374_tempco_v() const;

	void latch0_w(u8 data);
	void latch1_w(u8 data);
	void latch2_w(u8 data);

	void set_cv(u8 cv_index, double cv, bool fast);
	void set_res_cv(double cv);
	void sh_hold();

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void sound_stream_update(sound_stream &stream) override;

private:
	static double vcf_freq_cv(double vcff);

	void update_osc2_mix();
	void update_filter_mode();
	void update_pan();

	// This mapping applies to revision D of the voice board. Earlier revisions
	// had CV_VCA and CV_PW1 swapped.
	enum cv_index
	{
		CV_PW1 = 0,  // osc 1 pulse width
		CV_VCA,      // amp gain
		CV_VOLA,     // osc 1 volume
		CV_VCOF1,    // osc 1 frequency
		CV_VOLB,     // osc 2 + noise volume
		CV_VCFF,     // filter cutoff frequency
		CV_PW2,      // osc 2 pulse width
		CV_VCOF2,    // osc 2 frequency
		CV_RES,      // filter resonance
		NUM_CVS
	};

	static const char *const CV_NAMES[NUM_CVS];
	static const double RX40;
	static const double CX33;

	device_sound_interface *const m_noise;

	required_device_array<va_smooth_sh_device, 9> m_sh;
	required_device<filter_rc_device> m_osc2_mixer;
	required_device<cem3372_vcf_device> m_vcf;
	required_device<filter_rc_device> m_vcf_out4;
	required_device<mixer_device> m_pole_mixer;
	required_device<cem3372_vca_device> m_vca;
	required_device<filter_rc_device> m_out;

	output_finder<> m_fm_mdac;  // Latch connected to AD7523/MP7523.
	output_finder<> m_saw1;
	output_finder<> m_saw2;
	output_finder<> m_tri1;
	output_finder<> m_tri2;
	output_finder<> m_vcofm;
	output_finder<> m_sync;

	bool m_noi;  // UX14 pin11 (CD4053)
	u8 m_filter_mode;
	u8 m_pan;

	std::array<double, NUM_CVS> m_cv;
	std::array<bool, NUM_CVS - 1> m_fast;  // `-1` because RES does not support fast updates.
};


// The 6-voice Xpander voice board revision D.
class xpandervb_device : public device_t, public device_sound_interface
{
public:
	enum output
	{
		OUTPUT_LEFT = 0,
		OUTPUT_RIGHT,
	};

	xpandervb_device(const machine_config &mconfig, const char *tag, device_t *owner, device_sound_interface *noise) ATTR_COLD;
	xpandervb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) ATTR_COLD;

	// Interface exposed to the main CPU.
	void reset_w(int state);
	void haltreq_w(int state);
	int haltack_r() const { return m_haltack; }
	auto haltack_cb() { return m_haltack_cb.bind(); }

protected:
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;

	void sound_stream_update(sound_stream &stream) override;

private:
	void voicecpu_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(deferred_reset_w);
	TIMER_CALLBACK_MEMBER(deferred_haltreq_w);

	u8 datain_r();
	void dataout_w(u8 data);
	void latch0_w(offs_t offset, u8 data);
	void latch1_w(offs_t offset, u8 data);
	void latch2_w(offs_t offset, u8 data);

	double get_dac_v() const;
	void dac_enable_w(offs_t offset, u8 data);
	void dac_clear_w(u8 data);
	void dac_w(offs_t offset, u8 data);

	void pit_out0_changed(int state);
	void pit_out2_changed(int state);

	void update_line_halt();

	static double DAC_VREF_SCALER;
	static double DEFAULT_DAC_VREF;

	device_sound_interface *const m_noise;

	required_device<mc6809_device> m_voicecpu;
	required_device<pit8253_device> m_pit;
	required_device_array<xpander_voice_device, 6> m_voices;
	devcb_write_line m_haltack_cb;

	bool m_haltdis;  // Halt disable (HALTDS).
	bool m_haltreq;  // Halt request (HALTREQ).
	bool m_haltack;  // Halt acknowledge (HALTAKN).
	bool m_autodone;  // Autotune done (AUTODNE).
	u16 m_dac_data;
	double m_dac_fine_v;
	double m_dac_vref;  // Sampled in C806 and buffered and scaled by U815.
	bool m_fast;  // Fast CV sampling enabled (FAST).
	s8 m_dac_voice;  // DAC destination voice. -1 if none.
};

#endif  // MAME_OBERHEIM_XPANDER_VB_H
