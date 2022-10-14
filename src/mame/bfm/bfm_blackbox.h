// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Bellfruit Black Box

**********************************************************************/

#ifndef MAME_INCLUDES_BFM_BLACKBOX_H
#define MAME_INCLUDES_BFM_BLACKBOX_H

#pragma once

#include "fruitsamples.h"

#include "cpu/m6800/m6800.h"
#include "cpu/tms1000/tms1000.h"
#include "machine/6821pia.h"
#include "machine/em_reel.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#define OUT_READ(name) uint8_t name##_r(offs_t offset) { name##_w(offset, 0); return 0; }
#define OUT_READ_T(name) template <unsigned T> uint8_t name##_r(offs_t offset) { name##_w<T>(offset, 0); return 0; }
#define OUTPUT(name) rw(FUNC(name##_r), FUNC(name##_w))
#define OUTPUT_T(name, T) rw(FUNC(name##_r<T>), FUNC(name##_w<T>))

class blackbox_base_state : public driver_device
{
public:
	blackbox_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_chute_timer(*this, "chute_timer"),
		m_pia(*this, "pia"),
		m_in_1800(*this, "IN1800"),
		m_in_1800_en(*this, "IN1800_%u", 1U),
		m_in_2000(*this, "IN2000"),
		m_in_perc(*this, "PERCENTAGE"),
		m_lamps(*this, "lamp%u", 0U),
		m_digits(*this, "digit%u", 0U),
		m_test_led(*this, "test%u", 1U),
		m_samples(*this, "samples"),
		m_nvram(*this, "nvram", 0x40, ENDIANNESS_BIG)
	{ }

	void blackbox_base(machine_config &config);

	DECLARE_READ_LINE_MEMBER(in_perc_r);
	DECLARE_READ_LINE_MEMBER(chute_r) { return m_50p_chute; }
	DECLARE_INPUT_CHANGED_MEMBER(chute_inserted);

protected:
	virtual void machine_start() override;

	DECLARE_WRITE_LINE_MEMBER(pia_cb2_w) {} // Prevent CB2 write spam

	void pia_porta_w(uint8_t data);
	uint8_t pia_portb_r();
	void pia_portb_w(uint8_t data);
	uint8_t in_1800_r(offs_t offset);
	uint8_t in_2000_r(offs_t offset);
	void out_triacs1_w(offs_t offset, uint8_t data); OUT_READ(out_triacs1);
	void out_meters_w(offs_t offset, uint8_t data); OUT_READ(out_meters);
	template <unsigned Offset> void out_lamps_w(offs_t offset, uint8_t data); OUT_READ_T(out_lamps);
	void out_misc_w(offs_t offset, uint8_t data); OUT_READ(out_misc);
	template <unsigned Digit> void out_disp_w(offs_t offset, uint8_t data); OUT_READ_T(out_disp);
	void payout_w(uint8_t payout, uint8_t enable, bool state);

	void blackbox_base_map(address_map &map);

	uint8_t m_out_data;
	uint8_t m_input_en[6];
	bool m_payouts[2][2];
	bool m_payout_state[2];
	bool m_50p_chute;
	uint8_t m_nvram_data;

	TIMER_DEVICE_CALLBACK_MEMBER(nmi) { m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); }
	TIMER_DEVICE_CALLBACK_MEMBER(irq) { m_maincpu->pulse_input_line(M6800_IRQ_LINE, attotime::from_usec(2500)); }
	TIMER_DEVICE_CALLBACK_MEMBER(toggle_50p_chute);

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_device<timer_device> m_chute_timer;
	required_device<pia6821_device> m_pia;
	required_ioport m_in_1800;
	required_ioport_array<6> m_in_1800_en;
	required_ioport m_in_2000;
	required_ioport m_in_perc;
	output_finder<52> m_lamps;
	output_finder<4> m_digits;
	output_finder<2> m_test_led;
	required_device<fruit_samples_device> m_samples;

	memory_share_creator<uint8_t> m_nvram;
};

class blackbox_em_base_state : public blackbox_base_state
{
public:
	blackbox_em_base_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_base_state(mconfig, type, tag),
			m_reels(*this, "emreel%u", 1U)
	{ }

	void blackbox_em_base_20symbol(machine_config &config);
	void blackbox_em_base_24symbol(machine_config &config);

protected:
	void out_triacs2_w(offs_t offset, uint8_t data); OUT_READ(out_triacs2);

	required_device_array<em_reel_device, 4> m_reels;
};

class blackbox_em_state : public blackbox_em_base_state
{
public:
	blackbox_em_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_base_state(mconfig, type, tag),
			m_in_extra(*this, "EXTRA")
	{ }

	void blackbox_em(machine_config &config);
	void blackbox_em_bellt(machine_config &config);

	DECLARE_READ_LINE_MEMBER(in_extra_r);

protected:
	uint8_t in_2000_r(offs_t offset);
	void out_lamps2_buzzer_w(offs_t offset, uint8_t data); OUT_READ(out_lamps2_buzzer);
	void out_bellt_in_select_w(offs_t offset, uint8_t data); OUT_READ(out_bellt_in_select);

	void blackbox_em_map(address_map &map);
	void blackbox_em_bellt_map(address_map &map);

	bool m_in_extra_select[8];

	optional_ioport m_in_extra;
};

class blackbox_em_21up_state : public blackbox_em_state
{
public:
	blackbox_em_21up_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_state(mconfig, type, tag),
			m_beep_sample(*this, "beep_sample")
	{ }

	void blackbox_em_21up(machine_config &config);

	void init_21up();

private:
	void out_lamps1_beeper_w(offs_t offset, uint8_t data); OUT_READ(out_lamps1_beeper);

	void blackbox_em_21up_map(address_map &map);

	bool m_beeper_on;
	int16_t m_beep_sample_data[477];

	required_device<samples_device> m_beep_sample;
};

class blackbox_em_admc_state : public blackbox_em_state
{
public:
	blackbox_em_admc_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_state(mconfig, type, tag)
	{ }

	void blackbox_em_admc(machine_config &config);

private:
	void out_triacs2_w(offs_t offset, uint8_t data); OUT_READ(out_triacs2);
	void out_lamps_480_w(offs_t offset, uint8_t data); OUT_READ(out_lamps_480);
	void out_sound_l_w(offs_t offset, uint8_t data); OUT_READ(out_sound_l);
	void out_sound_h_w(offs_t offset, uint8_t data); OUT_READ(out_sound_h);
	uint8_t prot_r();
	void prot_reset_w(uint8_t data) { m_prot_index = 0; }
	void out_prot_clock_w(offs_t offset, uint8_t data) { m_prot_index++; } OUT_READ(out_prot_clock);

	void blackbox_em_admc_map(address_map &map);

	uint8_t m_prot_index;
	uint8_t m_sound_value;
};

class blackbox_em_opto_state : public blackbox_em_base_state
{
public:
	blackbox_em_opto_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_base_state(mconfig, type, tag)
	{ }

	template <unsigned Reel> DECLARE_READ_LINE_MEMBER(symbol_opto_r) { return m_reels[Reel]->read_bfm_symbol_opto(); }
	template <unsigned Reel> DECLARE_READ_LINE_MEMBER(reel_opto_r) { return m_reels[Reel]->read_bfm_reel_opto(); }

protected:
	void out_meters_w(offs_t offset, uint8_t data); OUT_READ(out_meters);

	void blackbox_em_opto_map(address_map &map);
};

class blackbox_em_opto_sndgen_state : public blackbox_em_opto_state
{
public:
	blackbox_em_opto_sndgen_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_opto_state(mconfig, type, tag),
			m_beep(*this, "beep")
	{ }

	void blackbox_em_opto_sndgen(machine_config &config);

private:
	void out_tone_w(offs_t offset, uint8_t data); OUT_READ(out_tone);
	void out_mute_w(offs_t offset, uint8_t data); OUT_READ(out_mute);
	
	void blackbox_em_opto_sndgen_map(address_map &map);

	required_device<beep_device> m_beep;
};

class blackbox_em_opto_aux_state : public blackbox_em_opto_state
{
public:
	blackbox_em_opto_aux_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_opto_state(mconfig, type, tag),
			m_beep(*this, "beep")
	{ }

	void blackbox_em_opto_aux_base(machine_config &config);
	void blackbox_em_opto_aux(machine_config &config);

protected:
	void out_tone_w(offs_t offset, uint8_t data); OUT_READ(out_tone);

	void blackbox_em_opto_aux_map(address_map &map);

	required_device<beep_device> m_beep;
};

class blackbox_em_opto_music_state : public blackbox_em_opto_state
{
public:
	blackbox_em_opto_music_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_opto_state(mconfig, type, tag),
			m_tms1000(*this, "tms1000"),
			m_tempo_timer(*this, "tempo"),
			m_speaker(*this, "speaker")
	{ }

	void blackbox_em_opto_music(machine_config &config);

private:
	void out_music_480_w(offs_t offset, uint8_t data); OUT_READ(out_music_480);
	void out_music_500_w(offs_t offset, uint8_t data); OUT_READ(out_music_500);
	uint8_t tms1000_k_r();
	void tms1000_r_w(uint32_t data);
	void tms1000_o_w(uint16_t data);

	void blackbox_em_opto_music_map(address_map &map);

	uint8_t m_k_cols;
	uint8_t m_r_bits;
	uint8_t m_r_select;
	uint8_t m_tempo;
	uint32_t m_prev_r;

	required_device<tms1k_base_device> m_tms1000;
	required_device<timer_device> m_tempo_timer;
	required_device<speaker_sound_device> m_speaker;
};

class blackbox_em_opto_club_state : public blackbox_em_opto_aux_state
{
public:
	blackbox_em_opto_club_state(const machine_config &mconfig, device_type type, const char *tag) :
			blackbox_em_opto_aux_state(mconfig, type, tag)
	{ }

	void blackbox_em_opto_club(machine_config &config);

private:
	virtual void machine_start() override;

	void out_triacs1_w(offs_t offset, uint8_t data); OUT_READ(out_triacs1);
	void out_triacs2_w(offs_t offset, uint8_t data); OUT_READ(out_triacs2);
	void out_meters_w(offs_t offset, uint8_t data); OUT_READ(out_meters);
	void update_payout(uint8_t payout);

	void blackbox_em_opto_club_map(address_map &map);

	bool m_payout_en[2];
};

#endif // MAME_INCLUDES_BFM_BLACKBOX_H
