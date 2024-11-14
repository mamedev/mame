// license:BSD-3-Clause
// copyright-holders:Steve Baines, Frank Palazzolo
/***************************************************************************

    Atari Star Wars hardware

***************************************************************************/
#ifndef MAME_ATARI_STARWARS_H
#define MAME_ATARI_STARWARS_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/mos6530.h"
#include "slapstic.h"
#include "machine/x2212.h"
#include "sound/pokey.h"
#include "sound/tms5220.h"


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
		m_slapstic(*this, "slapstic"),
		m_slapstic_bank(*this, "slapstic_bank")
	{ }

	void starwars(machine_config &config);
	void esb(machine_config &config);

	void init_esb();
	void init_starwars();

	int matrix_flag_r();

private:
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_mainlatch;
	required_device<mos6532_device> m_riot;
	required_shared_ptr<uint8_t> m_mathram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device_array<pokey_device, 4> m_pokey;
	required_device<tms5220_device> m_tms;
	required_device<x2212_device> m_novram;
	optional_device<atari_slapstic_device> m_slapstic;
	optional_memory_bank m_slapstic_bank;

	int m_MPA = 0;
	int m_BIC = 0;
	uint16_t m_dvd_shift = 0;
	uint16_t m_quotient_shift = 0;
	uint16_t m_divisor = 0;
	uint16_t m_dividend = 0;
	std::unique_ptr<uint8_t[]> m_PROM_STR;
	std::unique_ptr<uint8_t[]> m_PROM_MAS;
	std::unique_ptr<uint8_t[]> m_PROM_AM;
	int m_math_run = 0;
	emu_timer *m_math_timer = nullptr;
	int16_t m_A = 0;
	int16_t m_B = 0;
	int16_t m_C = 0;
	int32_t m_ACC = 0;
	void irq_ack_w(uint8_t data);
	void starwars_nstore_w(uint8_t data);
	void recall_w(int state);
	void coin1_counter_w(int state);
	void coin2_counter_w(int state);
	uint8_t starwars_prng_r();
	void prng_reset_w(int state);
	uint8_t starwars_div_reh_r();
	uint8_t starwars_div_rel_r();
	void starwars_math_w(offs_t offset, uint8_t data);

	uint8_t starwars_main_ready_flag_r();
	void starwars_soundrst_w(uint8_t data);
	void quad_pokeyn_w(offs_t offset, uint8_t data);
	virtual void machine_reset() override ATTR_COLD;
	TIMER_CALLBACK_MEMBER(math_run_clear);

	void starwars_mproc_init();
	void starwars_mproc_reset();
	void run_mproc();

	void esb_main_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_ATARI_STARWARS_H
