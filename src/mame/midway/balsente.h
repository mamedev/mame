// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/
#ifndef MAME_MIDWAY_BALSENTE_H
#define MAME_MIDWAY_BALSENTE_H

#pragma once

#include "machine/6850acia.h"
#include "machine/timer.h"
#include "machine/x2212.h"
#include "machine/74259.h"
#include "emupal.h"
#include "screen.h"

#define BALSENTE_MASTER_CLOCK   (20000000)
#define BALSENTE_CPU_CLOCK      (BALSENTE_MASTER_CLOCK / 16)
#define BALSENTE_PIXEL_CLOCK    (BALSENTE_MASTER_CLOCK / 4)
#define BALSENTE_HTOTAL         (0x140)
#define BALSENTE_HBEND          (0x000)
#define BALSENTE_HBSTART        (0x100)
#define BALSENTE_VTOTAL         (0x108)
#define BALSENTE_VBEND          (0x010)
#define BALSENTE_VBSTART        (0x100)


class balsente_state : public driver_device
{
	static constexpr unsigned POLY17_BITS = 17;
	static constexpr size_t POLY17_SIZE = (1 << POLY17_BITS) - 1;
	static constexpr unsigned POLY17_SHL = 7;
	static constexpr unsigned POLY17_SHR = 10;
	static constexpr uint32_t POLY17_ADD = 0x18000;

public:
	balsente_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_scanline_timer(*this, "scan_timer")
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_shrike_io(*this, "shrike_io")
		, m_shrike_shared(*this, "shrike_shared")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_68k(*this, "68k")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_outlatch(*this, "outlatch")
		, m_novram(*this, "nov%u", 0U)
		, m_acia(*this, "acia")
		, m_mainrom(*this, "maincpu")
		, m_bankab(*this, "bankab")
		, m_bankcd(*this, "bankcd")
		, m_bankef(*this, "bankef")
	{ }

	void shrike(machine_config &config);
	void rescraid(machine_config &config);
	void balsente(machine_config &config);
	void teamht(machine_config &config);
	void grudge(machine_config &config);
	void st1002(machine_config &config);
	void spiker(machine_config &config);
	void triviamb(machine_config &config);
	ioport_value nstocker_bits_r();
	void init_otwalls();
	void init_triviaes();
	void init_triviaes2();
	void init_nstocker();
	void init_sentetst();
	void init_rescraid();
	void init_minigolf();
	void init_stompin();
	void init_snakepit();
	void init_spiker();
	void init_hattrick();
	void init_toggle();
	void init_snakjack();
	void init_grudge();
	void init_sfootbal();
	void init_triviag2();
	void init_cshift();
	void init_gimeabrk();
	void init_stocker();
	void init_triviag1();
	void init_shrike();
	void init_minigolf2();
	void init_nametune();
	void init_gghost();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void random_reset_w(uint8_t data);
	uint8_t random_num_r();
	void rombank_select_w(uint8_t data);
	void rombank2_select_w(uint8_t data);
	void out0_w(int state);
	void out1_w(int state);
	void out2_w(int state);
	void out3_w(int state);
	void out4_w(int state);
	void out5_w(int state);
	void out6_w(int state);
	void nvrecall_w(int state);
	uint8_t novram_8bit_r(address_space &space, offs_t offset);
	void novram_8bit_w(offs_t offset, uint8_t data);
	uint8_t adc_data_r();
	void adc_select_w(offs_t offset, uint8_t data);
	void spiker_expand_w(offs_t offset, uint8_t data);
	uint8_t spiker_expand_r();
	uint8_t grudge_steering_r();
	uint8_t shrike_shared_6809_r(offs_t offset);
	void shrike_shared_6809_w(offs_t offset, uint8_t data);
	void shrike_io_68k_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t shrike_io_68k_r(offs_t offset, uint16_t mem_mask = ~0);
	uint8_t teamht_extra_r();
	void teamht_multiplex_select_w(offs_t offset, uint8_t data);

	void videoram_w(offs_t offset, uint8_t data);
	void palette_select_w(uint8_t data);
	void shrike_sprite_select_w(uint8_t data);

	uint32_t screen_update_balsente(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(update_analog_inputs);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(adc_finished);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_timer);
	void draw_one_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *sprite);
	void poly17_init();
	void update_grudge_steering();
	void expand_roms(uint8_t cd_rom_mask);
	inline void config_shooter_adc(uint8_t shooter, uint8_t adc_shift);

	void cpu1_base_map(address_map &map) ATTR_COLD;
	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu1_teamht_map(address_map &map) ATTR_COLD;
	void cpu1_grudge_map(address_map &map) ATTR_COLD;
	void cpu1_st1002_map(address_map &map) ATTR_COLD;
	void cpu1_spiker_map(address_map &map) ATTR_COLD;
	void cpu1_shrike_map(address_map &map) ATTR_COLD;
	void cpu1_smudge_map(address_map &map) ATTR_COLD;
	void cpu1_triviamb_map(address_map &map) ATTR_COLD;
	void cpu2_triviamb_io_map(address_map &map) ATTR_COLD;
	void cpu2_triviamb_map(address_map &map) ATTR_COLD;
	void shrike68k_map(address_map &map) ATTR_COLD;

	required_device<timer_device> m_scanline_timer;


	/* global data */
	uint8_t m_shooter = 0;
	uint8_t m_shooter_x = 0;
	uint8_t m_shooter_y = 0;
	uint8_t m_adc_shift = 0;
	emu_timer *m_irq_off_timer = nullptr;

	/* random number generator states */
	uint8_t m_rand17[POLY17_SIZE + 1]{};

	/* ADC I/O states */
	int8_t m_analog_input_data[4]{};
	uint8_t m_adc_value = 0;
	emu_timer *m_adc_timer = nullptr;

	/* game-specific states */
	uint8_t m_nstocker_bits = 0;
	uint8_t m_spiker_expand_color = 0;
	uint8_t m_spiker_expand_bgcolor = 0;
	uint8_t m_spiker_expand_bits = 0;
	uint8_t m_grudge_steering_result = 0;
	uint8_t m_grudge_last_steering[3]{};
	uint8_t m_teamht_input = 0;

	/* video data */
	uint8_t m_expanded_videoram[256*256]{};
	uint8_t *m_sprite_data = nullptr;
	uint32_t m_sprite_mask = 0;
	uint8_t *m_sprite_bank[2]{};

	uint8_t m_palettebank_vis = 0;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint16_t> m_shrike_io;
	optional_shared_ptr<uint16_t> m_shrike_shared;
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_68k;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<ls259_device> m_outlatch;
	optional_device_array<x2212_device, 2> m_novram;
	optional_device<acia6850_device> m_acia;

	required_memory_region m_mainrom;

	required_memory_bank m_bankab;
	required_memory_bank m_bankcd;
	required_memory_bank m_bankef;
};

#endif // MAME_MIDWAY_BALSENTE_H
