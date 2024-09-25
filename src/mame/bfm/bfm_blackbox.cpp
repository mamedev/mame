// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

    Bellfruit Black Box (1979)

    Bellfruit's first CPU-based fruit machine platform. The CPU
    is a MC6802, with a PIA and extension bus for I/O. Games are
    stored on ROM cards up to 8K in size, the CPU board also has a
    socket for a 64 nibble NVRAM board. The base system consists
    of the CPU board and output board, extension boards provide
    more functionality such as reel drive, sound and displays.
    The exact set of boards used differs for each game. There were
    3 distinct types of reel systems used. Early games had plain
    electromechanical reels, which were quickly upgraded to a EM
    reel system using opto sensors for positioning, presumably
    to prevent cheating. The first games with stepper reels were
    introduced in late 1981. Later versions of the CPU board have
    an ACIA used for communicating with the LPE DRS 35 datalogger
    system, which was introduced in 1982.

    TODO:
    - Add stepper games
    - ACIA datalogger, currently no games use it
    - Layouts
    - Sound could be more accurate with more circuit information

**********************************************************************/

#include "emu.h"

#include "fruitsamples.h"

#include "cpu/m6800/m6800.h"
#include "cpu/tms1000/tms1000.h"

#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/em_reel.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#include "speaker.h"

namespace {

#include "bfm_blackbox.lh"
#include "bb_21up.lh"
#include "bb_bellt.lh"
#include "bb_cjack.lh"
#include "bb_dblit.lh"
#include "bb_fiest.lh"
#include "bb_firec.lh"
#include "bb_gspin.lh"
#include "bb_nudcl.lh"
#include "bb_nudgm.lh"
#include "bb_oal.lh"
#include "bb_spinu.lh"
#include "bb_upndn.lh"

class blackbox_base_state : public driver_device
{
public:
	int in_perc_r();
	int chute_r() { return m_50p_chute; }
	DECLARE_INPUT_CHANGED_MEMBER(chute_inserted);

protected:
	blackbox_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nmi_timer(*this, "nmi"),
		m_irq_timer(*this, "irq"),
		m_chute_timer(*this, "chute_timer"),
		m_pia(*this, "pia"),
		m_acia(*this, "acia"),
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

	virtual void machine_start() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(nmi) { m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero); }
	TIMER_DEVICE_CALLBACK_MEMBER(irq) { m_maincpu->pulse_input_line(M6800_IRQ_LINE, attotime::from_usec(2500)); }
	TIMER_DEVICE_CALLBACK_MEMBER(toggle_50p_chute);

	void pia_porta_w(uint8_t data);
	uint8_t pia_portb_r();
	void pia_portb_w(uint8_t data);
	uint8_t in_1800_r(offs_t offset);
	uint8_t in_2000_r(offs_t offset);
	void out_triacs1_w(address_space &space, uint8_t data);
	void out_meters_w(address_space &space, uint8_t data);
	template <unsigned Offset> void out_lamps_w(address_space &space, uint8_t data);
	void out_misc_w(address_space &space, uint8_t data);
	template <unsigned Digit> void out_disp_w(address_space &space, uint8_t data);
	uint8_t out_triacs1_r(address_space &space);
	uint8_t out_meters_r(address_space &space);
	template <unsigned Offset> uint8_t out_lamps_r(address_space &space);
	uint8_t out_misc_r(address_space &space);
	template <unsigned Digit> uint8_t out_disp_r(address_space &space);
	void payout_w(uint8_t payout, uint8_t enable, bool state);

	void blackbox_base_map(address_map &map) ATTR_COLD;

	uint8_t m_out_data;
	uint8_t m_input_en[6];
	bool m_payouts[2][2];
	bool m_payout_state[2];
	bool m_50p_chute;
	uint8_t m_nvram_data;

	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_nmi_timer;
	required_device<timer_device> m_irq_timer;
	required_device<timer_device> m_chute_timer;
	required_device<pia6821_device> m_pia;
	required_device<acia6850_device> m_acia;
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
protected:
	blackbox_em_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		blackbox_base_state(mconfig, type, tag),
		m_reels(*this, "emreel%u", 1U)
	{ }

	enum { STEPS_PER_SYMBOL = 20 };

	void add_em_reels(machine_config &config, int symbols, attotime period);
	template <unsigned Reel> void reel_sample_cb(int state);

	void out_triacs2_w(address_space &space, uint8_t data);
	uint8_t out_triacs2_r(address_space &space);

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

	int in_extra_r();

protected:
	uint8_t in_2000_r(offs_t offset);
	uint8_t reel_pos_r(uint8_t reel);
	void out_lamps2_buzzer_w(address_space &space, uint8_t data);
	void out_bellt_in_select_w(address_space &space, uint8_t data);
	uint8_t out_lamps2_buzzer_r(address_space &space);
	uint8_t out_bellt_in_select_r(address_space &space);

	void blackbox_em_map(address_map &map) ATTR_COLD;
	void blackbox_em_bellt_map(address_map &map) ATTR_COLD;

	bool m_buzzer_on;
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
	void out_lamps1_beeper_w(address_space &space, uint8_t data);
	uint8_t out_lamps1_beeper_r(address_space &space);

	void blackbox_em_21up_map(address_map &map) ATTR_COLD;

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
	void out_triacs2_w(address_space &space, uint8_t data);
	void out_lamps_480_w(address_space &space, uint8_t data);
	void out_sound_l_w(address_space &space, uint8_t data);
	void out_sound_h_w(address_space &space, uint8_t data);
	void out_prot_clock_w(address_space &space, uint8_t data) { m_prot_index++; }
	uint8_t out_triacs2_r(address_space &space);
	uint8_t out_lamps_480_r(address_space &space);
	uint8_t out_sound_l_r(address_space &space);
	uint8_t out_sound_h_r(address_space &space);
	uint8_t out_prot_clock_r(address_space &space);
	uint8_t prot_r();
	void prot_reset_w(uint8_t data) { m_prot_index = 0; }

	void blackbox_em_admc_map(address_map &map) ATTR_COLD;

	uint8_t m_prot_index;
	uint8_t m_sound_value;
};

class blackbox_em_opto_state : public blackbox_em_base_state
{
public:
	blackbox_em_opto_state(const machine_config &mconfig, device_type type, const char *tag) :
		blackbox_em_base_state(mconfig, type, tag)
	{ }

	template <unsigned Reel> int symbol_opto_r();
	template <unsigned Reel> int reel_opto_r();

protected:
	void out_meters_w(address_space &space, uint8_t data);
	uint8_t out_meters_r(address_space &space);

	void blackbox_em_opto_map(address_map &map) ATTR_COLD;
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
	void out_tone_w(address_space &space, uint8_t data);
	void out_mute_w(address_space &space, uint8_t data);
	uint8_t out_tone_r(address_space &space);
	uint8_t out_mute_r(address_space &space);

	void blackbox_em_opto_sndgen_map(address_map &map) ATTR_COLD;

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
	void out_tone_w(address_space &space, uint8_t data);
	uint8_t out_tone_r(address_space &space);

	void blackbox_em_opto_aux_map(address_map &map) ATTR_COLD;

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
	void out_music_480_w(address_space &space, uint8_t data);
	void out_music_500_w(address_space &space, uint8_t data);
	uint8_t out_music_480_r(address_space &space);
	uint8_t out_music_500_r(address_space &space);
	uint8_t tms1000_k_r();
	void tms1000_r_w(uint32_t data);
	void tms1000_o_w(uint16_t data);

	void blackbox_em_opto_music_map(address_map &map) ATTR_COLD;

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
	virtual void machine_start() override ATTR_COLD;

	void out_triacs1_w(address_space &space, uint8_t data);
	void out_triacs2_w(address_space &space, uint8_t data);
	void out_meters_w(address_space &space, uint8_t data);
	uint8_t out_triacs1_r(address_space &space);
	uint8_t out_triacs2_r(address_space &space);
	uint8_t out_meters_r(address_space &space);
	void update_payout(uint8_t payout);

	void blackbox_em_opto_club_map(address_map &map) ATTR_COLD;

	bool m_payout_en[2];
};

void blackbox_base_state::blackbox_base_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0800, 0x0801).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1000, 0x1003).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x1800, 0x1807).r(FUNC(blackbox_base_state::in_1800_r)).nopw();
	map(0x2000, 0x2007).r(FUNC(blackbox_base_state::in_2000_r)).nopw();
	map(0x2800, 0x2fff).rw(FUNC(blackbox_base_state::out_triacs1_r), FUNC(blackbox_base_state::out_triacs1_w));
	map(0x3800, 0x3fff).rw(FUNC(blackbox_base_state::out_meters_r), FUNC(blackbox_base_state::out_meters_w));
	map(0x4000, 0x47ff).rw(FUNC(blackbox_base_state::out_lamps_r<0>), FUNC(blackbox_base_state::out_lamps_w<0>));
	map(0x4800, 0x50ff).rw(FUNC(blackbox_base_state::out_lamps_r<8>), FUNC(blackbox_base_state::out_lamps_w<8>));
	map(0x5000, 0x57ff).rw(FUNC(blackbox_base_state::out_misc_r), FUNC(blackbox_base_state::out_misc_w));
	map(0x6000, 0x7fff).rom();
}

void blackbox_em_state::blackbox_em_map(address_map &map)
{
	blackbox_base_map(map);

	map(0x2000, 0x2007).r(FUNC(blackbox_em_state::in_2000_r));
	map(0x3000, 0x37ff).rw(FUNC(blackbox_em_state::out_triacs2_r), FUNC(blackbox_em_state::out_triacs2_w));
	map(0x4800, 0x4fff).rw(FUNC(blackbox_em_state::out_lamps2_buzzer_r), FUNC(blackbox_em_state::out_lamps2_buzzer_w));
}

void blackbox_em_state::blackbox_em_bellt_map(address_map &map)
{
	blackbox_em_map(map);

	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_state::out_bellt_in_select_r), FUNC(blackbox_em_state::out_bellt_in_select_w));
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_state::out_lamps_r<34>), FUNC(blackbox_em_state::out_lamps_w<34>));
	map(0x0580, 0x05ff).rw(FUNC(blackbox_em_state::out_lamps_r<26>), FUNC(blackbox_em_state::out_lamps_w<26>));
	map(0x0600, 0x067f).rw(FUNC(blackbox_em_state::out_lamps_r<18>), FUNC(blackbox_em_state::out_lamps_w<18>));
}

void blackbox_em_21up_state::blackbox_em_21up_map(address_map &map)
{
	blackbox_em_map(map);

	map(0x0580, 0x05ff).rw(FUNC(blackbox_em_21up_state::out_disp_r<0>), FUNC(blackbox_em_21up_state::out_disp_w<0>));
	map(0x0600, 0x067f).rw(FUNC(blackbox_em_21up_state::out_disp_r<1>), FUNC(blackbox_em_21up_state::out_disp_w<1>));
	map(0x4000, 0x47ff).rw(FUNC(blackbox_em_21up_state::out_lamps1_beeper_r), FUNC(blackbox_em_21up_state::out_lamps1_beeper_w));
}

void blackbox_em_admc_state::blackbox_em_admc_map(address_map &map)
{
	blackbox_em_map(map);

	map(0x0200, 0x027f).rw(FUNC(blackbox_em_admc_state::out_prot_clock_r), FUNC(blackbox_em_admc_state::out_prot_clock_w));
	map(0x0380, 0x03ff).rw(FUNC(blackbox_em_admc_state::out_sound_l_r), FUNC(blackbox_em_admc_state::out_sound_l_w));
	map(0x0400, 0x047f).rw(FUNC(blackbox_em_admc_state::out_sound_h_r), FUNC(blackbox_em_admc_state::out_sound_h_w));
	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_admc_state::out_lamps_480_r), FUNC(blackbox_em_admc_state::out_lamps_480_w));
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_admc_state::out_lamps_r<34>), FUNC(blackbox_em_admc_state::out_lamps_w<34>));
	map(0x0580, 0x05ff).rw(FUNC(blackbox_em_admc_state::out_lamps_r<26>), FUNC(blackbox_em_admc_state::out_lamps_w<26>));
	map(0x0600, 0x067f).rw(FUNC(blackbox_em_admc_state::out_lamps_r<18>), FUNC(blackbox_em_admc_state::out_lamps_w<18>));
	map(0x3000, 0x37ff).rw(FUNC(blackbox_em_admc_state::out_triacs2_r), FUNC(blackbox_em_admc_state::out_triacs2_w));
	map(0x6400, 0x6400).r(FUNC(blackbox_em_admc_state::prot_r));
	map(0x6800, 0x6800).w(FUNC(blackbox_em_admc_state::prot_reset_w));
}

void blackbox_em_opto_state::blackbox_em_opto_map(address_map &map)
{
	blackbox_base_map(map);

	map(0x0580, 0x05ff).rw(FUNC(blackbox_em_opto_state::out_disp_r<0>), FUNC(blackbox_em_opto_state::out_disp_w<0>));
	map(0x0600, 0x067f).rw(FUNC(blackbox_em_opto_state::out_disp_r<1>), FUNC(blackbox_em_opto_state::out_disp_w<1>));
	map(0x3000, 0x37ff).rw(FUNC(blackbox_em_opto_state::out_triacs2_r), FUNC(blackbox_em_opto_state::out_triacs2_w));
	map(0x3800, 0x3fff).rw(FUNC(blackbox_em_opto_state::out_meters_r), FUNC(blackbox_em_opto_state::out_meters_w));
}

void blackbox_em_opto_sndgen_state::blackbox_em_opto_sndgen_map(address_map &map)
{
	blackbox_em_opto_map(map);

	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_opto_sndgen_state::out_tone_r), FUNC(blackbox_em_opto_sndgen_state::out_tone_w));
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_opto_sndgen_state::out_mute_r), FUNC(blackbox_em_opto_sndgen_state::out_mute_w));
}

void blackbox_em_opto_aux_state::blackbox_em_opto_aux_map(address_map &map)
{
	blackbox_em_opto_map(map);

	map(0x0400, 0x047f).rw(FUNC(blackbox_em_opto_aux_state::out_lamps_r<26>), FUNC(blackbox_em_opto_aux_state::out_lamps_w<26>)); // MFME lamps 40-47
	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_opto_aux_state::out_lamps_r<18>), FUNC(blackbox_em_opto_aux_state::out_lamps_w<18>)); // MFME lamps 32-39
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_opto_aux_state::out_tone_r), FUNC(blackbox_em_opto_aux_state::out_tone_w));
}

void blackbox_em_opto_music_state::blackbox_em_opto_music_map(address_map &map)
{
	blackbox_em_opto_map(map);

	map(0x0400, 0x047f).rw(FUNC(blackbox_em_opto_music_state::out_lamps_r<18>), FUNC(blackbox_em_opto_music_state::out_lamps_w<18>));
	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_opto_music_state::out_music_480_r), FUNC(blackbox_em_opto_music_state::out_music_480_w));
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_opto_music_state::out_music_500_r), FUNC(blackbox_em_opto_music_state::out_music_500_w));
}

void blackbox_em_opto_club_state::blackbox_em_opto_club_map(address_map &map)
{
	blackbox_em_opto_map(map);

	map(0x0200, 0x027f).rw(FUNC(blackbox_em_opto_club_state::out_lamps_r<42>), FUNC(blackbox_em_opto_club_state::out_lamps_w<42>));
	map(0x0280, 0x03ff).rw(FUNC(blackbox_em_opto_club_state::out_lamps_r<26>), FUNC(blackbox_em_opto_club_state::out_lamps_w<26>));
	map(0x0300, 0x037f).rw(FUNC(blackbox_em_opto_club_state::out_lamps_r<18>), FUNC(blackbox_em_opto_club_state::out_lamps_w<18>));
	map(0x0380, 0x03ff).rw(FUNC(blackbox_em_opto_club_state::out_tone_r), FUNC(blackbox_em_opto_club_state::out_tone_w));
	map(0x0400, 0x047f).rw(FUNC(blackbox_em_opto_club_state::out_lamps_r<34>), FUNC(blackbox_em_opto_club_state::out_lamps_w<34>));
	map(0x0480, 0x04ff).rw(FUNC(blackbox_em_opto_club_state::out_disp_r<0>), FUNC(blackbox_em_opto_club_state::out_disp_w<0>));
	map(0x0500, 0x057f).rw(FUNC(blackbox_em_opto_club_state::out_disp_r<1>), FUNC(blackbox_em_opto_club_state::out_disp_w<1>));
	map(0x0580, 0x05ff).rw(FUNC(blackbox_em_opto_club_state::out_disp_r<2>), FUNC(blackbox_em_opto_club_state::out_disp_w<2>));
	map(0x0600, 0x067f).rw(FUNC(blackbox_em_opto_club_state::out_disp_r<3>), FUNC(blackbox_em_opto_club_state::out_disp_w<3>));
	map(0x2800, 0x2fff).rw(FUNC(blackbox_em_opto_club_state::out_triacs1_r), FUNC(blackbox_em_opto_club_state::out_triacs1_w));
	map(0x3000, 0x37ff).rw(FUNC(blackbox_em_opto_club_state::out_triacs2_r), FUNC(blackbox_em_opto_club_state::out_triacs2_w));
	map(0x3800, 0x3fff).rw(FUNC(blackbox_em_opto_club_state::out_meters_r), FUNC(blackbox_em_opto_club_state::out_meters_w));
}

void blackbox_base_state::pia_porta_w(uint8_t data)
{
	uint8_t nvram_addr = data & 0x1f;
	if(data & 0x40)
	{
		if(data & 0x80)
			m_nvram_data = m_nvram[nvram_addr];
		else
			m_nvram[nvram_addr] = m_nvram_data;
	}
}

uint8_t blackbox_base_state::pia_portb_r()
{
	return m_nvram_data;
}

void blackbox_base_state::pia_portb_w(uint8_t data)
{
	m_out_data = (data & 0xf0) >> 4;
	m_nvram_data = data & 0xf;
}

uint8_t blackbox_base_state::in_1800_r(offs_t offset)
{
	if(BIT(m_in_1800->read(), offset)) return 0x80;

	for(int i = 0; i < 6; i++)
		if(m_input_en[i] && BIT(m_in_1800_en[i]->read(), offset)) return 0x80;

	return 0;
}

uint8_t blackbox_base_state::in_2000_r(offs_t offset)
{
	return BIT(m_in_2000->read(), offset) ? 0x80 : 0;
}

uint8_t blackbox_em_state::in_2000_r(offs_t offset)
{
	if(offset < 5) // First 5 bits return reel position bits if any of them are enabled
	{
		for(int i = 0; i < 4; i++)
			if(m_input_en[i] && BIT(reel_pos_r(i), offset)) return 0x80;
	}

	return BIT(m_in_2000->read(), offset) ? 0x80 : 0;
}

int blackbox_base_state::in_perc_r()
{
	/* 0x2007 is used for the percentage adjustment switch and it's
	   a bit weird. On older games it's a simple on-off switch,
	   but some later games have 3 possible positions, and use two
	   of the input select bits to read it. */
	uint8_t perc_sw = m_in_perc->read();
	if(perc_sw == 0 && m_input_en[3]) return 1; // Low
	if(perc_sw == 1 && m_input_en[5]) return 1; // High
	// Medium is implied when the others are off

	// Check if switch is set high on 2-position games
	if(perc_sw == 1 && !m_input_en[3] && !m_input_en[5]) return 1;

	return 0;
}

int blackbox_em_state::in_extra_r()
{
	for(int i = 0; i < 8; i++)
		if(m_in_extra_select[i] && BIT(m_in_extra->read(), i)) return 1;

	return 0;
}

uint8_t blackbox_em_state::reel_pos_r(uint8_t reel)
{
	uint16_t pos = m_reels[reel]->get_pos();

	if(pos % STEPS_PER_SYMBOL == 0)
		return (pos / STEPS_PER_SYMBOL) + 1;
	else
		return 0;
}

template <unsigned Reel>
int blackbox_em_opto_state::symbol_opto_r()
{
	uint8_t const sym_pos = m_reels[Reel]->get_pos() % STEPS_PER_SYMBOL;

	return (sym_pos >= 12 && sym_pos <= 15); // Symbol tab is on every symbol
}

template <unsigned Reel>
int blackbox_em_opto_state::reel_opto_r()
{
	uint16_t const pos = m_reels[Reel]->get_pos();

	return (pos >= 10 && pos <= 13); // Reel tab is only on the first symbol
}

INPUT_CHANGED_MEMBER( blackbox_base_state::chute_inserted )
{
	if(newval) m_chute_timer->adjust(attotime::from_msec(100));
}

TIMER_DEVICE_CALLBACK_MEMBER( blackbox_base_state::toggle_50p_chute )
{
	m_50p_chute = !m_50p_chute;
	if(m_50p_chute) m_chute_timer->adjust(attotime::from_msec(50)); // Keep switch high for this long
}

void blackbox_base_state::payout_w(uint8_t payout, uint8_t enable, bool state)
{
	/* Each payout has 2 enable lines, once both are on the payout fires
	   Payouts: 0=10p cash, 1=2x10p token (might be different on some rebuilds) */
	m_payouts[payout][enable] = state;

	if(m_payouts[payout][0] && m_payouts[payout][1] && !m_payout_state[payout])
	{
		m_payout_state[payout] = true;
		m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	}
	else
	{
		m_payout_state[payout] = false;
	}
}

void blackbox_em_opto_club_state::update_payout(uint8_t payout)
{
	/* Payouts: 0=10p cash, 1=£2.50 token */
	if(m_payout_en[payout] && (m_payouts[payout][0] || m_payouts[payout][1]) && !m_payout_state[payout])
	{
		m_payout_state[payout] = true;
		m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	}
	else
	{
		m_payout_state[payout] = false;
	}
}

void blackbox_base_state::out_triacs1_w(address_space &space, uint8_t data)
{
	uint8_t bit = (m_out_data & 0xe) >> 1;
	bool state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
			payout_w(0, 0, state);
			break;
		case 1:
			payout_w(1, 0, state);
			break;
		case 4:
			machine().bookkeeping().coin_lockout_w(0, !state);
			machine().bookkeeping().coin_lockout_w(1, !state);
			machine().bookkeeping().coin_lockout_w(2, !state);
			break;
		case 5:
			machine().bookkeeping().coin_lockout_w(3, !state); // 50p lockout
			break;
		case 6:
			payout_w(0, 1, state);
			break;
		case 7:
			payout_w(1, 1, state);
			break;
	}
}

uint8_t blackbox_base_state::out_triacs1_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_triacs1_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_club_state::out_triacs1_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	switch(bit)
	{
		/* Club machines have a different payout arrangement using 10p coins and
		   £2.50 tokens, each of them having two payouts gated by a common enable line */
		case 0: m_payouts[0][0] = state; update_payout(0); break;
		case 1: m_payouts[1][0] = state; update_payout(1); break;
		case 2: m_payouts[0][1] = state; update_payout(0); break;
		case 3: m_payouts[1][1] = state; update_payout(1); break;
		case 4: m_payout_en[0] = state; update_payout(0); break; // 10p
		case 5: m_payout_en[1] = state; update_payout(1); break; // £2.50 token
		case 6: machine().bookkeeping().coin_lockout_w(0, !state); break; // 10p lockout
		case 7: machine().bookkeeping().coin_lockout_w(3, !state); break; // 50p lockout
	}
}

uint8_t blackbox_em_opto_club_state::out_triacs1_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_triacs1_w(space, 0);
	return space.unmap();
}

void blackbox_em_base_state::out_triacs2_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			m_reels[bit]->set_state(state);
			break;
		case 6:
		case 7:
			m_lamps[16 + (bit - 6)] = state; // MFME lamps 30-31
			break;
	}
}

uint8_t blackbox_em_base_state::out_triacs2_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_triacs2_w(space, 0);
	return space.unmap();
}

void blackbox_em_admc_state::out_triacs2_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			m_reels[bit]->set_state(state);
			break;
		case 4:
			if(state)
			{
				for(int i = 0; i < 4; i++)
					m_reels[i]->set_direction(em_reel_device::dir::REVERSE);
			}
			break;
		case 5:
			if(state)
			{
				for(int i = 0; i < 4; i++)
					m_reels[i]->set_direction(em_reel_device::dir::FORWARD);
			}
			break;
		case 6:
		case 7:
			m_lamps[16 + (bit - 6)] = state; // MFME lamps 30-31
			break;
	}
}

uint8_t blackbox_em_admc_state::out_triacs2_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_triacs2_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_club_state::out_triacs2_w(address_space &space, uint8_t data)
{
	uint8_t bit = (m_out_data & 0xe) >> 1;
	bool state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
		case 1:
		case 2:
		case 3:
			m_reels[bit]->set_state(state);
			break;
		case 4:
			machine().bookkeeping().coin_lockout_w(1, !state); // 20p lockout
			break;
		case 6:
		case 7:
			m_lamps[50 + (bit - 6)] = state;
			break;
	}
}

uint8_t blackbox_em_opto_club_state::out_triacs2_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_triacs2_w(space, 0);
	return space.unmap();
}

void blackbox_base_state::out_meters_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	if(bit < 7) machine().bookkeeping().coin_counter_w(bit, state);
	// Black Box doesn't have audible meters
}

uint8_t blackbox_base_state::out_meters_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_meters_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_state::out_meters_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	if(bit < 7)
	{
		machine().bookkeeping().coin_counter_w(bit, state);
	}
	else
	{
		for(int i = 0; i < 4; i++)
			m_reels[i]->set_direction(state ? em_reel_device::dir::FORWARD : em_reel_device::dir::REVERSE);
	}
}

uint8_t blackbox_em_opto_state::out_meters_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_meters_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_club_state::out_meters_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	if(bit < 6)
		machine().bookkeeping().coin_counter_w(bit, state);
	else
		m_lamps[16 + (bit - 6)] = state;
}

uint8_t blackbox_em_opto_club_state::out_meters_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_meters_w(space, 0);
	return space.unmap();
}

template<unsigned Offset>
void blackbox_base_state::out_lamps_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	m_lamps[Offset + bit] = m_out_data & 0x1;
}

template<unsigned Offset>
uint8_t blackbox_base_state::out_lamps_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_lamps_w<Offset>(space, 0);
	return space.unmap();
}

void blackbox_em_21up_state::out_lamps1_beeper_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	/* Bit 6 controls a beeper, which generates a 3500 Hz sine wave amplitude
	   modulated at 50 Hz. The beeper circuit is unknown, so I'm using a sample
	   to generate the beep for now. */
	if(bit == 6)
	{
		if(state && !m_beeper_on)
			m_beep_sample->start_raw(0, m_beep_sample_data, 477, 48000, true);
		else if(!state)
			m_beep_sample->stop(0);

		m_beeper_on = state;
	}
	else
	{
		m_lamps[bit] = state;
	}
}

uint8_t blackbox_em_21up_state::out_lamps1_beeper_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_lamps1_beeper_w(space, 0);
	return space.unmap();
}

void blackbox_em_state::out_lamps2_buzzer_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	if(bit == 5)
	{
		if(state && !m_buzzer_on)
			m_samples->play(fruit_samples_device::SAMPLE_BUZZER);

		m_buzzer_on = state;
	}
	else
	{
		m_lamps[8 + bit] = state;
	}
}

uint8_t blackbox_em_state::out_lamps2_buzzer_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_lamps2_buzzer_w(space, 0);
	return space.unmap();
}

void blackbox_em_admc_state::out_lamps_480_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	bool const state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
		case 1:
			m_in_extra_select[bit] = state;
			break;
		default:
			m_lamps[42 + bit] = state;
			break;
	}
}

uint8_t blackbox_em_admc_state::out_lamps_480_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_lamps_480_w(space, 0);
	return space.unmap();
}

void blackbox_base_state::out_misc_w(address_space &space, uint8_t data)
{
	uint8_t bit = (m_out_data & 0xe) >> 1;
	bool state = m_out_data & 0x1;

	switch(bit)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			m_input_en[bit] = state;
			break;
		case 6:
			m_test_led[1] = state;
			break;
		case 7:
			m_test_led[0] = state;
			break;
	}
}

uint8_t blackbox_base_state::out_misc_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_misc_w(space, 0);
	return space.unmap();
}

template<unsigned Digit>
void blackbox_base_state::out_disp_w(address_space &space, uint8_t data)
{
	static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };
	m_digits[Digit] = patterns[m_out_data];
}

template<unsigned Digit>
uint8_t blackbox_base_state::out_disp_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_disp_w<Digit>(space, 0);
	return space.unmap();
}

void blackbox_em_state::out_bellt_in_select_w(address_space &space, uint8_t data)
{
	uint8_t const bit = (m_out_data & 0xe) >> 1;
	m_in_extra_select[bit] = m_out_data & 0x1;
}

uint8_t blackbox_em_state::out_bellt_in_select_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_bellt_in_select_w(space, 0);
	return space.unmap();
}

/* The Sound Generator board uses a 555 timer-based circuit to generate
   a tone with 8 selectable frequencies. The circuit is unknown, so it is
   currently HLE'd. On later games the circuit was integrated onto the
   Auxiliary Board with a slightly different interface. */
void blackbox_em_opto_sndgen_state::out_tone_w(address_space &space, uint8_t data)
{
	// Ideal frequencies, a real board will be out of tune
	constexpr uint32_t freqs[8] = { 523, 784, 932, 1245, 1568, 1976, 2794, 3729 };

	m_beep->set_clock(freqs[m_out_data & 0x7]);
}

uint8_t blackbox_em_opto_sndgen_state::out_tone_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_tone_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_sndgen_state::out_mute_w(address_space &space, uint8_t data)
{
	m_beep->set_state(~m_out_data & 0x1);
}

uint8_t blackbox_em_opto_sndgen_state::out_mute_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_mute_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_aux_state::out_tone_w(address_space &space, uint8_t data)
{
	if(m_out_data & 0x8)
	{
		// Ideal frequencies, a real board will be out of tune
		constexpr uint32_t freqs[8] = { 523, 784, 932, 1245, 1568, 1976, 2794, 3729 };
		m_beep->set_clock(freqs[m_out_data & 0x7]);
		m_beep->set_state(1);
	}
	else
	{
		m_beep->set_state(0);
	}
}

uint8_t blackbox_em_opto_aux_state::out_tone_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_tone_w(space, 0);
	return space.unmap();
}

/* Oranges And Lemons music board utiliizing a TMS1000 with the MP0027A
   program, which is a doorbell chip (see cchime in hh_tms1k.cpp) */
void blackbox_em_opto_music_state::out_music_480_w(address_space &space, uint8_t data)
{
	m_k_cols = m_out_data & 0x7;
	m_tempo = BIT(m_out_data, 3);
}

uint8_t blackbox_em_opto_music_state::out_music_480_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_music_480_w(space, 0);
	return space.unmap();
}

void blackbox_em_opto_music_state::out_music_500_w(address_space &space, uint8_t data)
{
	m_r_select = 1 << (m_out_data & 0x7);
}

uint8_t blackbox_em_opto_music_state::out_music_500_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_music_500_w(space, 0);
	return space.unmap();
}

uint8_t blackbox_em_opto_music_state::tms1000_k_r()
{
	uint8_t result = 0;

	if(m_r_bits & m_r_select) result |= m_k_cols;
	return result | (m_tempo_timer->enabled() ? 0x8 : 0);
}

void blackbox_em_opto_music_state::tms1000_r_w(uint32_t data)
{
	m_r_bits = data & 0xff; // R0-7

	if(~data & 0x200 & m_prev_r) // R9
	{
		// Tempo could be adjustable, there is an unknown trim pot on the board
		m_tempo_timer->adjust(m_tempo ? attotime::from_hz(920.32 / 32) : attotime::from_hz(920.32));
	}

	m_prev_r = data;
}

void blackbox_em_opto_music_state::tms1000_o_w(uint16_t data)
{
	m_speaker->level_w(BIT(~data, 6) & BIT(~data, 7));
}

/* ADMC unknown sound board, interface for this is one 8-bit value
   which presumably controls the frequency of a single tone. There are
   no recordings or any other info for this sound board, so emulation
   is currently not possible. */
void blackbox_em_admc_state::out_sound_l_w(address_space &space, uint8_t data)
{
	m_sound_value = (m_sound_value & ~0xf) | m_out_data;
	//if(m_sound_value != 0xff) logerror("ADMC sound write %x\n", m_sound_value);
}

uint8_t blackbox_em_admc_state::out_sound_l_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_sound_l_w(space, 0);
	return space.unmap();
}

void blackbox_em_admc_state::out_sound_h_w(address_space &space, uint8_t data)
{
	m_sound_value = (m_sound_value & ~0xf0) | (m_out_data << 4);
}

uint8_t blackbox_em_admc_state::out_sound_h_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_sound_h_w(space, 0);
	return space.unmap();
}

uint8_t blackbox_em_admc_state::out_prot_clock_r(address_space &space)
{
	if(!machine().side_effects_disabled())
		out_prot_clock_w(space, 0);
	return space.unmap();
}

/* bb_nudgm & presumably other ADMC games have protection: a byte is read from the
   protection device after every spin and compared against a fixed value (0x54 in this case).
   The game ignores any wins if the protection check fails. Before doing the check the game
   writes 0xC6 to 6800, whether it affects the returned value is not known. */
uint8_t blackbox_em_admc_state::prot_r()
{
	uint8_t value = 0;

	constexpr uint8_t prot_values[8] = { 0, 0, 0x80, 0, 0x80, 0, 0x80, 0 }; // 0x54 in reverse
	if(m_prot_index < 8) value = prot_values[m_prot_index];
	return value;
}

template <unsigned Reel>
void blackbox_em_base_state::reel_sample_cb(int state)
{
	if(state == 0)
		m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_STOP + Reel);
	else if(state == 1)
		m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_START + Reel);
}

static INPUT_PORTS_START( blackbox_inputs )
	PORT_START("IN1800")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_5")
	PORT_DIPNAME( 0x01, 0x00, "Test Switch 2" )
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x01, "Test" )
	PORT_DIPNAME( 0x02, 0x00, "Test Switch 1" )
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x02, "Test" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN1800_6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, in_perc_r)

	PORT_START("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x02, "Percentage adjustment" )
	PORT_DIPSETTING(    0x00, "Low" )
	PORT_DIPSETTING(    0x01, "High" )
	PORT_DIPSETTING(    0x02, "Medium" )
INPUT_PORTS_END

static INPUT_PORTS_START( blackbox_inputs_em_opto )
	PORT_INCLUDE( blackbox_inputs )

	PORT_MODIFY("IN1800_3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<0>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<0>)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<1>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<1>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<2>)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<2>)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<3>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<3>)
INPUT_PORTS_END

static INPUT_PORTS_START( blackbox_inputs_em_opto_alt )
	PORT_INCLUDE( blackbox_inputs )

	PORT_MODIFY("IN1800") // Some games expect to see the opto inputs without setting any enable bits
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<0>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<0>)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<1>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<1>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<2>)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<2>)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, reel_opto_r<3>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_opto_state, symbol_opto_r<3>)
INPUT_PORTS_END

static INPUT_PORTS_START( coin_5p_10p_10pt_50p )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token") PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, chute_r)
	PORT_CONFNAME( 0x20, 0x20, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( coin_10p_10pt_50p )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p Token") PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, chute_r)
	PORT_CONFNAME( 0x20, 0x20, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( coin_5p_10p_50p )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, chute_r)
	PORT_CONFNAME( 0x20, 0x20, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
INPUT_PORTS_END

static INPUT_PORTS_START( coin_10p_2p_50p )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("2p") PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, chute_r)
	PORT_CONFNAME( 0x20, 0x20, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
INPUT_PORTS_END

static INPUT_PORTS_START( coin_20p_10p_10pt_50p )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token") PORT_IMPULSE(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("20p") PORT_IMPULSE(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_base_state, chute_r)
	PORT_CONFNAME( 0x20, 0x20, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( coin_10p_50p_20p_club )
	PORT_MODIFY("IN2000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(1)
												PORT_CHANGED_MEMBER(DEVICE_SELF, blackbox_base_state, chute_inserted, 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("20p") PORT_IMPULSE(1)
INPUT_PORTS_END

static INPUT_PORTS_START( bb_nudcl )
	PORT_INCLUDE( blackbox_inputs )
	PORT_INCLUDE( coin_5p_10p_50p )

	PORT_MODIFY("IN1800")
	PORT_DIPNAME( 0x01, 0x00, "Test Switch 2" ) // Alternate test switches
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x01, "Test" )
	PORT_DIPNAME( 0x02, 0x00, "Test Switch 1" )
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x02, "Test" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Respin")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )

	PORT_MODIFY("IN1800_5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold chance" )
	PORT_DIPSETTING(    0x00, "Low (21.9%)" )
	PORT_DIPSETTING(    0x01, "High (25%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_21up )
	PORT_INCLUDE( blackbox_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN1800")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Stick")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Twist")

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold chance" )
	PORT_DIPSETTING(    0x00, "Low (25%)" )
	PORT_DIPSETTING(    0x01, "High (31.25%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_21up2 )
	PORT_INCLUDE( bb_21up )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold chance" )
	PORT_DIPSETTING(    0x00, "Low (31.25%)" )
	PORT_DIPSETTING(    0x01, "High (34.37%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_bellt )
	PORT_INCLUDE( blackbox_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN1800")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )

	PORT_MODIFY("IN1800_6")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_state, in_extra_r)

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold chance" )
	PORT_DIPSETTING(    0x00, "Low (21.9%)" )
	PORT_DIPSETTING(    0x01, "High (28.1%)" )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Nudge Up 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Nudge Down 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Nudge Down 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Nudge Down 3")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_NAME("Nudge Down 4")
INPUT_PORTS_END

static INPUT_PORTS_START( bb_nudgm )
	PORT_INCLUDE( blackbox_inputs )
	PORT_INCLUDE( coin_20p_10p_10pt_50p )

	PORT_MODIFY("IN1800")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Start Up/Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start Down/Collect")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(blackbox_em_state, in_extra_r)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Jackpot amount" )
	PORT_DIPSETTING(    0x00, u8"£1" )
	PORT_DIPSETTING(    0x01, u8"£2" )

	PORT_START("EXTRA")
	PORT_DIPNAME( 0x03, 0x00, "Percentage adjustment" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_upndn )
	PORT_INCLUDE( blackbox_inputs_em_opto_alt )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN1800_1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Respin")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )

	PORT_MODIFY("IN1800_2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Nudge Up 3")

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Nudge chance" )
	PORT_DIPSETTING(    0x00, "Low (6.25%)" )
	PORT_DIPSETTING(    0x01, "High (7.81%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_reelg )
	PORT_INCLUDE( bb_upndn )
	PORT_INCLUDE( coin_10p_10pt_50p )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_upndna )
	PORT_INCLUDE( bb_upndn )
	PORT_INCLUDE( coin_5p_10p_50p )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_dblit )
	PORT_INCLUDE( blackbox_inputs_em_opto_alt )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN1800_1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold/wild chance" )
	PORT_DIPSETTING(    0x00, "Low (34.37%/12.5%)" )
	PORT_DIPSETTING(    0x01, "High (37.5%/14.1%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_spinu )
	PORT_INCLUDE( bb_dblit )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold/wild chance" )
	PORT_DIPSETTING(    0x00, "Low (31.25%/12.5%)" )
	PORT_DIPSETTING(    0x01, "High (37.5%/14.1%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_firec )
	PORT_INCLUDE( blackbox_inputs_em_opto )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN1800_1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN1800_2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold/nudge chance" )
	PORT_DIPSETTING(    0x00, "Low (26.6%/6.25%)" )
	PORT_DIPSETTING(    0x01, "High (39.1%/10.9%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_cjack )
	PORT_INCLUDE( bb_firec )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold/nudge chance" )
	PORT_DIPSETTING(    0x00, "Low (21.9%/6.25%)" )
	PORT_DIPSETTING(    0x01, "High (26.6%/6.25%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_fiest )
	PORT_INCLUDE( bb_firec )
	PORT_INCLUDE( coin_10p_2p_50p )

	PORT_MODIFY("IN2000")
	PORT_DIPNAME( 0x08, 0x08, "Jackpot amount" )
	PORT_DIPSETTING(    0x00, u8"£2" )
	PORT_DIPSETTING(    0x08, u8"£1" )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Hold/nudge chance" )
	PORT_DIPSETTING(    0x00, "Low (21.9%/6.25%)" )
	PORT_DIPSETTING(    0x01, "High (26.6%/6.25%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_oal )
	PORT_INCLUDE( blackbox_inputs_em_opto )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN1800_1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN1800_2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up 1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Nudge Up 3")

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x02, "Hold/nudge chance" )
	PORT_DIPSETTING(    0x00, "Low (31.2%/6.25%)" )
	PORT_DIPSETTING(    0x01, "High (31.2%/7.8%)" )
	PORT_DIPSETTING(    0x02, "Medium (15.6%/7.8%)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bb_gspin )
	PORT_INCLUDE( blackbox_inputs_em_opto )
	PORT_INCLUDE( coin_10p_50p_20p_club )

	PORT_MODIFY("IN1800_2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_HALF ) PORT_NAME("Gamble Half")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_MODIFY("IN2000")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_INTERLOCK ) PORT_NAME("Back Door") PORT_TOGGLE

	PORT_MODIFY("IN1800_4")
	PORT_DIPNAME( 0x01, 0x00, "Test Switch 2" ) // Alternate test switches
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x01, "Test" )
	PORT_DIPNAME( 0x02, 0x00, "Test Switch 1" )
	PORT_DIPSETTING(    0x00, "Run" )
	PORT_DIPSETTING(    0x02, "Test" )
	PORT_MODIFY("IN1800_5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("PERCENTAGE")
	PORT_DIPNAME( 0x03, 0x01, "Percentage" )
	PORT_DIPSETTING(    0x00, "75%" )
	PORT_DIPSETTING(    0x01, "80%" )
INPUT_PORTS_END

void blackbox_base_state::machine_start()
{
	m_lamps.resolve();
	m_digits.resolve();
	m_test_led.resolve();

	save_item(NAME(m_input_en));

	std::fill(std::begin(m_input_en), std::end(m_input_en), false);
	m_50p_chute = false;

	// bb_upndn & bb_reelg won't properly read nudge down buttons on the first go if RAM starts out as 0's??
	for(int i = 0; i < 0x80; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i, 0xff);
}

void blackbox_em_opto_club_state::machine_start()
{
	blackbox_base_state::machine_start();

	m_payout_en[0] = false;
	m_payout_en[1] = false;
}

void blackbox_base_state::blackbox_base(machine_config &config)
{
	M6802(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_base_state::blackbox_base_map);

	TIMER(config, m_nmi_timer).configure_periodic(FUNC(blackbox_base_state::nmi), attotime::from_hz(100)); // From AC zero crossing detector
	TIMER(config, m_irq_timer).configure_periodic(FUNC(blackbox_base_state::irq), attotime::from_msec(80)); // 555 timer circuit
	m_nmi_timer->set_start_delay(attotime::from_msec(1)); // Don't take interrupts at reset time
	m_irq_timer->set_start_delay(attotime::from_msec(1));
	TIMER(config, m_chute_timer).configure_generic(FUNC(blackbox_base_state::toggle_50p_chute));

	PIA6821(config, m_pia);
	m_pia->writepa_handler().set(FUNC(blackbox_base_state::pia_porta_w));
	m_pia->readpb_handler().set(FUNC(blackbox_base_state::pia_portb_r));
	m_pia->writepb_handler().set(FUNC(blackbox_base_state::pia_portb_w));
	m_pia->cb2_handler().set_nop(); // Not connected

	ACIA6850(config, m_acia, 0);

	FRUIT_SAMPLES(config, m_samples);
}

void blackbox_em_base_state::add_em_reels(machine_config &config, int symbols, attotime period)
{
	for(int i = 0; i < 4; i++)
	{
		std::set<uint16_t> detents;
		for(int i = 0; i < symbols; i++)
			detents.insert(i * STEPS_PER_SYMBOL);

		EM_REEL(config, m_reels[i], symbols * STEPS_PER_SYMBOL, detents, period);
	}

	m_reels[0]->state_changed_callback().set(FUNC(blackbox_em_base_state::reel_sample_cb<0>));
	m_reels[1]->state_changed_callback().set(FUNC(blackbox_em_base_state::reel_sample_cb<1>));
	m_reels[2]->state_changed_callback().set(FUNC(blackbox_em_base_state::reel_sample_cb<2>));
	m_reels[3]->state_changed_callback().set(FUNC(blackbox_em_base_state::reel_sample_cb<3>));
}

void blackbox_em_state::blackbox_em(machine_config &config)
{
	blackbox_base(config);
	add_em_reels(config, 20, attotime::from_double(1.1));

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_state::blackbox_em_map);
}

void blackbox_em_state::blackbox_em_bellt(machine_config &config)
{
	blackbox_em(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_state::blackbox_em_bellt_map);
}

void blackbox_em_admc_state::blackbox_em_admc(machine_config &config)
{
	blackbox_em(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_admc_state::blackbox_em_admc_map);
}

void blackbox_em_21up_state::blackbox_em_21up(machine_config &config)
{
	blackbox_em(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_21up_state::blackbox_em_21up_map);

	SPEAKER(config, "mono").front_center();

	SAMPLES(config, m_beep_sample);
	m_beep_sample->set_channels(1);
	m_beep_sample->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void blackbox_em_opto_sndgen_state::blackbox_em_opto_sndgen(machine_config &config)
{
	blackbox_base(config);
	add_em_reels(config, 20, attotime::from_double(1.1));

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_opto_sndgen_state::blackbox_em_opto_sndgen_map);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.2);
}

void blackbox_em_opto_aux_state::blackbox_em_opto_aux_base(machine_config &config)
{
	blackbox_base(config);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beep, 1000).add_route(ALL_OUTPUTS, "mono", 0.2);
}

void blackbox_em_opto_aux_state::blackbox_em_opto_aux(machine_config &config)
{
	blackbox_em_opto_aux_base(config);
	add_em_reels(config, 20, attotime::from_double(1.1));

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_opto_aux_state::blackbox_em_opto_aux_map);
}

void blackbox_em_opto_music_state::blackbox_em_opto_music(machine_config &config)
{
	blackbox_base(config);
	add_em_reels(config, 20, attotime::from_double(1.1));

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_opto_music_state::blackbox_em_opto_music_map);
	TMS1000(config, m_tms1000, 452000); // R and C unknown, pitch matches a real machine

	m_tms1000->read_k().set(FUNC(blackbox_em_opto_music_state::tms1000_k_r));
	m_tms1000->write_r().set(FUNC(blackbox_em_opto_music_state::tms1000_r_w));
	m_tms1000->write_o().set(FUNC(blackbox_em_opto_music_state::tms1000_o_w));

	TIMER(config, "tempo").configure_generic(nullptr);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.6);
}

void blackbox_em_opto_club_state::blackbox_em_opto_club(machine_config &config)
{
	blackbox_em_opto_aux_base(config);
	add_em_reels(config, 24, attotime::from_double(1.44));

	m_maincpu->set_addrmap(AS_PROGRAM, &blackbox_em_opto_club_state::blackbox_em_opto_club_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

ROM_START( bb_nudcl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudcl.ic1", 0x7000, 0x800, CRC(84a86dd1) SHA1(64f407f7354c53d847a75a31c6f8285ab7bba9ca) )
	ROM_LOAD( "nudcl.ic2", 0x7800, 0x800, CRC(5ec1d534) SHA1(6f27ef90e2cecf4f71968ca007b719e6af2443fa) )
ROM_END

ROM_START( bb_21up )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712020.ic2", 0x7800, 0x800, CRC(477d3eed) SHA1(ee381f1054520537c19d7d26ae592df258d4520d) )
ROM_END

ROM_START( bb_21upa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "21up.ic2", 0x7800, 0x800, CRC(37a5b2f5) SHA1(a248e97071385261d2938f60f8ca6d36b582d107) )
ROM_END

ROM_START( bb_bellt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712167.ic1", 0x7000, 0x800, CRC(5b859f98) SHA1(c7af2c15e3d7e027ab9a15011e6c1a2b5958dfc8) )
	ROM_LOAD( "95712168.ic2", 0x7800, 0x800, CRC(6b3a16c5) SHA1(679008c2f0af89cf2c146332107c1708efc2ec82) )
ROM_END

ROM_START( bb_nudgm )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nudgm.ic1", 0x7000, 0x800, CRC(3c01c508) SHA1(2bcd12ff0eb87a0615e012c4c1d4bb258bbdf8c7) )
	ROM_LOAD( "nudgm.ic2", 0x7800, 0x800, CRC(8b158f80) SHA1(c6f864665fec3c05bdfb2669875ac7cdaa768390) )
ROM_END

ROM_START( bb_upndn )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Original IC1 dump had bad bits
	ROM_LOAD( "95712215.ic1", 0x7000, 0x800, CRC(6dca1ae2) BAD_DUMP SHA1(e02406070f46573da0f0bb9bd56b2e8100a9ad39) )
	ROM_LOAD( "95712216.ic2", 0x7800, 0x800, CRC(df17278f) SHA1(9e671645b5206ab8dc5b2c69675d28907fc18517) )
ROM_END

ROM_START( bb_reelg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712410.ic1", 0x7000, 0x800, CRC(733a116b) SHA1(bc41e93ee77df0052e2f0350f166be8e0989773d) )
	ROM_LOAD( "95712411.ic2", 0x7800, 0x800, CRC(435870d1) SHA1(b3e24ca1e5d026be02a71a79c2a2df68ce2dd7e9) )
ROM_END

ROM_START( bb_upndna )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Original IC2 dump had a stuck line
	ROM_LOAD( "95712467.ic1", 0x7000, 0x800, CRC(798c6e56) SHA1(910139fe917215a147d205eff063d7d17a7d4a94) )
	ROM_LOAD( "95712468.ic2", 0x7800, 0x800, CRC(f3900b77) BAD_DUMP SHA1(2fed5c4c1bcb60b5d9ce36e4bf777b9a2eec2010) )
ROM_END

ROM_START( bb_dblit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dblit.ic1", 0x7000, 0x800, CRC(f898705e) SHA1(2451eab4da1ab370dc8c43d864ae731699dc67f3) )
	ROM_LOAD( "dblit.ic2", 0x7800, 0x800, CRC(a7bcbdbd) SHA1(d68453fab561ebcabc473e2461513200af7614c9) )
ROM_END

ROM_START( bb_spinu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spinu.ic1", 0x7000, 0x800, CRC(2a425a5f) SHA1(862d4d197ee8d179c82b0ac1b4503c60c43d86ff) )
	ROM_LOAD( "spinu.ic2", 0x7800, 0x800, CRC(1b195136) SHA1(c6d411599aa3ce661e67fecad411c676cdc60a05) )
ROM_END

ROM_START( bb_firec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712394.ic1", 0x7000, 0x800, CRC(cd6bc605) SHA1(1ad6ccb6ca901c18b32d5f983e3d5b42fcb92000) )
	ROM_LOAD( "95712395.ic2", 0x7800, 0x800, CRC(d5df2317) SHA1(66f25a2a11657653f5eed3e8196d9b52119236c3) )
ROM_END

ROM_START( bb_cjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712484.ic1", 0x7000, 0x800, CRC(70b34b60) SHA1(fd636c991b971c1c86c52fcffe736ccf0e2117f0) )
	ROM_LOAD( "95712485.ic2", 0x7800, 0x800, CRC(13e9db40) SHA1(aab3d6b852edbd37b68d666673c0720c5b14ad20) )
ROM_END

ROM_START( bb_fiest )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fiest.ic1", 0x7000, 0x800, CRC(3ff1981c) SHA1(f9dd4b4a90f5388867219a87ca7f1d44229524c1) )
	ROM_LOAD( "fiest.ic2", 0x7800, 0x800, CRC(5c879d62) SHA1(77a908a1bc923b42a09c385cb0c3ec2d80712e7c) )
ROM_END

ROM_START( bb_oal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "95712463.ic1", 0x7000, 0x800, CRC(a89c4d4e) SHA1(e94c22532a46d0c0cc1832d9f57ca58193e66205) )
	ROM_LOAD( "95712464.ic2", 0x7800, 0x800, CRC(b4f1d005) SHA1(4b42fdea05e154d1a8686718bd95f499dd2794c0) )

	// TMS1000 on music board
	ROM_REGION( 0x400, "tms1000", 0 )
	ROM_LOAD( "mp0027a", 0x000, 0x400, CRC(8b5e2c4d) SHA1(5364d2836e9daefee2529a20c022e811bb3c7d89) )
	ROM_REGION( 867, "tms1000:mpla", 0 )
	ROM_LOAD( "tms1000_common2_micro.pla", 0, 867, CRC(d33da3cf) SHA1(13c4ebbca227818db75e6db0d45b66ba5e207776) )
	ROM_REGION( 365, "tms1000:opla", 0 )
	ROM_LOAD( "tms1000_cchime_output.pla", 0, 365, CRC(75d68c56) SHA1(85abde0ca0bcc605720551bea360498db350a7af) )
ROM_END

// 'GOLDEN SPIN' MK1.5 (C) Copyright - David John Powell (Author) 5/4/84.
ROM_START( bb_gspin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gspin.ic1", 0x6800, 0x800, CRC(7506ddd0) SHA1(3a0e9d9fd5179e106f7b8c5bb4762883ea833677) )
	ROM_LOAD( "gspin.ic2", 0x7000, 0x800, CRC(47f6ec74) SHA1(27fb0cd64b454145bb04b8411a628d11f5e6cc71) )
	ROM_LOAD( "gspin.ic3", 0x7800, 0x800, CRC(503aa869) SHA1(9e4d9be27e014603d2d2206e17e2ec0adafb91c1) )
ROM_END

void blackbox_em_21up_state::init_21up()
{
	for(int s = 0; s < 477; s++)
	{
		double wave = sin((2 * M_PI * 3500.0 * (double)s) / 48000.0);
		double mod = sin((2 * M_PI * 50.0 * (double)s) / 48000.0);
		m_beep_sample_data[s] = 32767 * wave * mod;
	}
}

} // anonymous namespace

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_SUPPORTS_SAVE
#define GAME_FLAGS_NOSOUND MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_SUPPORTS_SAVE|MACHINE_NO_SOUND

// AWP
GAMEL(1981,  bb_nudcl,  0,         blackbox_em,             bb_nudcl,  blackbox_em_state, empty_init, ROT0, "BFM", u8"Nudge Climber (Bellfruit) (Black Box) (5p Stake, £1 Jackpot, all cash)", GAME_FLAGS, layout_bb_nudcl ) // Not the original release
GAMEL(1979,  bb_21up,   0,         blackbox_em_21up,        bb_21up,   blackbox_em_21up_state, init_21up, ROT0, "BFM", u8"21 Up (Bellfruit) (Black Box) (5p Stake, £1 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_21up )
GAMEL(1981,  bb_21upa,  bb_21up,   blackbox_em_21up,        bb_21up2,  blackbox_em_21up_state, init_21up, ROT0, "BFM", u8"21 Up (Bellfruit) (Black Box) (10p Stake, £2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_21up )
GAMEL(1979,  bb_bellt,  0,         blackbox_em_bellt,       bb_bellt,  blackbox_em_state, empty_init, ROT0, "BFM", u8"Bell Trail (Bellfruit) (Black Box) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_bb_bellt )
GAMEL(1982,  bb_nudgm,  0,         blackbox_em_admc,        bb_nudgm,  blackbox_em_admc_state, empty_init, ROT0, "ADM ", u8"The Nudge Machine (ADMC) (Black Box) (5p Stake, £1/£2 Jackpot)", GAME_FLAGS_NOSOUND, layout_bb_nudgm )
GAMEL(1980,  bb_upndn,  0,         blackbox_em_opto_sndgen, bb_upndn,  blackbox_em_opto_sndgen_state, empty_init, ROT0, "BFM", u8"Upstairs 'N' Downstairs (Bellfruit) (Black Box) (5p Stake, £1 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_upndn )
GAMEL(1981,  bb_reelg,  bb_upndn,  blackbox_em_opto_sndgen, bb_reelg,  blackbox_em_opto_sndgen_state, empty_init, ROT0, "BFM", u8"Reel Gambler (Bellfruit) (Black Box) (10p Stake, £2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_upndn )
GAMEL(1981,  bb_upndna, bb_upndn,  blackbox_em_opto_sndgen, bb_upndna, blackbox_em_opto_sndgen_state, empty_init, ROT0, "BFM", u8"Upstairs 'N' Downstairs (Bellfruit) (Black Box) (5p Stake, £1 Jackpot, all cash)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_upndn ) // Could have another name
GAMEL(1981,  bb_dblit,  0,         blackbox_em_opto_sndgen, bb_dblit,  blackbox_em_opto_sndgen_state, empty_init, ROT0, "BFM", u8"Double It (Bellfruit) (Black Box) (10p Stake, £2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_dblit )
GAMEL(1981,  bb_spinu,  bb_dblit,  blackbox_em_opto_sndgen, bb_spinu,  blackbox_em_opto_sndgen_state, empty_init, ROT0, "CTL", u8"Spin Up (CTL) (Black Box) (10p Stake, £3 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_spinu ) // £3 jackpot rebuild of Double it
GAMEL(1981,  bb_firec,  0,         blackbox_em_opto_aux,    bb_firec,  blackbox_em_opto_aux_state, empty_init, ROT0, "BFM", u8"Fire Cracker (Bellfruit) (Black Box) (10p Stake, £2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_firec )
GAMEL(1981,  bb_cjack,  bb_firec,  blackbox_em_opto_aux,    bb_cjack,  blackbox_em_opto_aux_state, empty_init, ROT0, "BFM", u8"Crackerjack (Bellfruit) (Black Box) (5p Stake, £2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_cjack )
GAMEL(1981?, bb_fiest,  bb_firec,  blackbox_em_opto_aux,    bb_fiest,  blackbox_em_opto_aux_state, empty_init, ROT0, "Associated Leisure", u8"Fiesta (Associated Leisure) (Black Box) (2p Stake, £1/£2 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_fiest ) // £1/2p rebuild of Fire Cracker
GAMEL(1981,  bb_oal,    0,         blackbox_em_opto_music,  bb_oal,    blackbox_em_opto_music_state, empty_init, ROT0, "BFM", u8"Oranges And Lemons (Bellfruit) (Black Box) (10p Stake, £2 Jackpot)", GAME_FLAGS, layout_bb_oal )
// Club
GAMEL(1984,  bb_gspin,  0,         blackbox_em_opto_club,   bb_gspin,  blackbox_em_opto_club_state, empty_init, ROT0, "BWB", u8"Golden Spin (BWB) (Black Box) (MK1.5, 5p Stake, £50 Jackpot)", GAME_FLAGS|MACHINE_IMPERFECT_SOUND, layout_bb_gspin ) // Conversion of Valentine
