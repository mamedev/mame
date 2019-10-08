// license:BSD-3-Clause
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/
#ifndef MAME_INCLUDES_STARWARS_H
#define MAME_INCLUDES_STARWARS_H

#pragma once

#include "machine/6532riot.h"
#include "machine/gen_latch.h"
#include "machine/x2212.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"
#include "includes/slapstic.h"


class starwars_state : public driver_device
{
public:
	starwars_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_soundlatch(*this, "soundlatch"),
		m_mainlatch(*this, "mainlatch"),
		m_riot(*this, "riot"),
		m_mathram(*this, "mathram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_pokey(*this, "pokey%u", 1U),
		m_tms(*this, "tms"),
		m_novram(*this, "x2212"),
		m_slapstic_device(*this, "slapstic")
	{ }

	void starwars(machine_config &config);
	void esb(machine_config &config);

	void init_esb();
	void init_starwars();

	DECLARE_READ_LINE_MEMBER(matrix_flag_r);

private:
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<riot6532_device> m_riot;
	required_shared_ptr<uint8_t> m_mathram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<pokey_device, 4> m_pokey;
	required_device<tms5220_device> m_tms;
	required_device<x2212_device> m_novram;
	optional_device<atari_slapstic_device> m_slapstic_device;

	uint8_t *m_slapstic_source;
	uint8_t *m_slapstic_base;
	uint8_t m_slapstic_current_bank;
	int m_MPA;
	int m_BIC;
	uint16_t m_dvd_shift;
	uint16_t m_quotient_shift;
	uint16_t m_divisor;
	uint16_t m_dividend;
	std::unique_ptr<uint8_t[]> m_PROM_STR;
	std::unique_ptr<uint8_t[]> m_PROM_MAS;
	std::unique_ptr<uint8_t[]> m_PROM_AM;
	int m_math_run;
	emu_timer *m_math_timer;
	int16_t m_A;
	int16_t m_B;
	int16_t m_C;
	int32_t m_ACC;
	DECLARE_WRITE8_MEMBER(irq_ack_w);
	DECLARE_READ8_MEMBER(esb_slapstic_r);
	DECLARE_WRITE8_MEMBER(esb_slapstic_w);
	DECLARE_WRITE8_MEMBER(starwars_nstore_w);
	DECLARE_WRITE_LINE_MEMBER(recall_w);
	DECLARE_WRITE_LINE_MEMBER(coin1_counter_w);
	DECLARE_WRITE_LINE_MEMBER(coin2_counter_w);
	DECLARE_READ8_MEMBER(starwars_prng_r);
	DECLARE_WRITE_LINE_MEMBER(prng_reset_w);
	DECLARE_READ8_MEMBER(starwars_div_reh_r);
	DECLARE_READ8_MEMBER(starwars_div_rel_r);
	DECLARE_WRITE8_MEMBER(starwars_math_w);

	DECLARE_READ8_MEMBER(starwars_main_ready_flag_r);
	DECLARE_WRITE_LINE_MEMBER(boost_interleave_hack);
	DECLARE_WRITE8_MEMBER(starwars_soundrst_w);
	DECLARE_WRITE8_MEMBER(quad_pokeyn_w);
	virtual void machine_reset() override;
	TIMER_CALLBACK_MEMBER(math_run_clear);
	DECLARE_READ8_MEMBER(r6532_porta_r);
	DECLARE_WRITE8_MEMBER(r6532_porta_w);

	void starwars_mproc_init();
	void starwars_mproc_reset();
	void run_mproc();
	void esb_slapstic_tweak(address_space &space, offs_t offset);

	void esb_main_map(address_map &map);
	void main_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_STARWARS_H
