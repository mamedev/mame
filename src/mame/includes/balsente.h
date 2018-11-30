// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/
#ifndef MAME_INCLUDES_BALSENTE_H
#define MAME_INCLUDES_BALSENTE_H

#pragma once

#include "machine/6850acia.h"
#include "machine/pit8253.h"
#include "machine/timer.h"
#include "machine/x2212.h"
#include "machine/74259.h"
#include "sound/cem3394.h"
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


#define POLY17_BITS 17
#define POLY17_SIZE ((1 << POLY17_BITS) - 1)
#define POLY17_SHL  7
#define POLY17_SHR  10
#define POLY17_ADD  0x18000


class balsente_state : public driver_device
{
public:
	balsente_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_scanline_timer(*this, "scan_timer")
		, m_pit(*this, "pit")
		, m_counter_0_timer(*this, "8253_0_timer")
		, m_cem_device(*this, "cem%u", 1U)
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
		, m_audiouart(*this, "audiouart")
		, m_generic_paletteram_8(*this, "paletteram")
	{ }

	void shrike(machine_config &config);
	void rescraid(machine_config &config);
	void balsente(machine_config &config);
	void triviamb(machine_config &config);
	DECLARE_CUSTOM_INPUT_MEMBER(nstocker_bits_r);
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
	void init_teamht();
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

private:
	DECLARE_WRITE8_MEMBER(random_reset_w);
	DECLARE_READ8_MEMBER(random_num_r);
	DECLARE_WRITE8_MEMBER(rombank_select_w);
	DECLARE_WRITE8_MEMBER(rombank2_select_w);
	DECLARE_WRITE_LINE_MEMBER(out0_w);
	DECLARE_WRITE_LINE_MEMBER(out1_w);
	DECLARE_WRITE_LINE_MEMBER(out2_w);
	DECLARE_WRITE_LINE_MEMBER(out3_w);
	DECLARE_WRITE_LINE_MEMBER(out4_w);
	DECLARE_WRITE_LINE_MEMBER(out5_w);
	DECLARE_WRITE_LINE_MEMBER(out6_w);
	DECLARE_WRITE_LINE_MEMBER(nvrecall_w);
	DECLARE_READ8_MEMBER(novram_8bit_r);
	DECLARE_WRITE8_MEMBER(novram_8bit_w);
	DECLARE_WRITE8_MEMBER(acia_w);
	DECLARE_WRITE_LINE_MEMBER(uint_propagate_w);
	DECLARE_READ8_MEMBER(adc_data_r);
	DECLARE_WRITE8_MEMBER(adc_select_w);
	DECLARE_READ8_MEMBER(counter_state_r);
	DECLARE_WRITE8_MEMBER(counter_control_w);
	DECLARE_WRITE8_MEMBER(chip_select_w);
	DECLARE_WRITE8_MEMBER(dac_data_w);
	DECLARE_WRITE8_MEMBER(register_addr_w);
	DECLARE_WRITE8_MEMBER(spiker_expand_w);
	DECLARE_READ8_MEMBER(spiker_expand_r);
	DECLARE_READ8_MEMBER(grudge_steering_r);
	DECLARE_READ8_MEMBER(shrike_shared_6809_r);
	DECLARE_WRITE8_MEMBER(shrike_shared_6809_w);
	DECLARE_WRITE16_MEMBER(shrike_io_68k_w);
	DECLARE_READ16_MEMBER(shrike_io_68k_r);
	DECLARE_READ8_MEMBER(teamht_extra_r);
	DECLARE_WRITE8_MEMBER(teamht_multiplex_select_w);
	DECLARE_WRITE_LINE_MEMBER(counter_0_set_out);

	void update_counter_0_timer();
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(palette_select_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);
	DECLARE_WRITE8_MEMBER(shrike_sprite_select_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_balsente(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(update_analog_inputs);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(adc_finished);
	TIMER_DEVICE_CALLBACK_MEMBER(interrupt_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(clock_counter_0_ff);
	void draw_one_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *sprite);
	void poly17_init();
	DECLARE_WRITE_LINE_MEMBER(set_counter_0_ff);
	void update_grudge_steering();
	void expand_roms(uint8_t cd_rom_mask);
	inline void config_shooter_adc(uint8_t shooter, uint8_t adc_shift);
	inline void noise_gen_chip(int chip, int count, short *buffer);
	CEM3394_EXT_INPUT(noise_gen_0);
	CEM3394_EXT_INPUT(noise_gen_1);
	CEM3394_EXT_INPUT(noise_gen_2);
	CEM3394_EXT_INPUT(noise_gen_3);
	CEM3394_EXT_INPUT(noise_gen_4);
	CEM3394_EXT_INPUT(noise_gen_5);

	void cpu1_base_map(address_map &map);
	void cpu1_map(address_map &map);
	void cpu1_smudge_map(address_map &map);
	void cpu2_io_map(address_map &map);
	void cpu2_map(address_map &map);
	void cpu2_triviamb_io_map(address_map &map);
	void cpu2_triviamb_map(address_map &map);
	void shrike68k_map(address_map &map);

	required_device<timer_device> m_scanline_timer;
	required_device<pit8253_device> m_pit;

	/* global data */
	uint8_t m_shooter;
	uint8_t m_shooter_x;
	uint8_t m_shooter_y;
	uint8_t m_adc_shift;

	/* manually clocked counter 0 states */
	uint8_t m_counter_control;
	bool m_counter_0_ff;
	bool m_counter_0_out;
	required_device<timer_device> m_counter_0_timer;
	bool m_counter_0_timer_active;

	/* random number generator states */
	uint8_t m_poly17[POLY17_SIZE + 1];
	uint8_t m_rand17[POLY17_SIZE + 1];

	/* ADC I/O states */
	int8_t m_analog_input_data[4];
	uint8_t m_adc_value;

	/* CEM3394 DAC control states */
	uint16_t m_dac_value;
	uint8_t m_dac_register;
	uint8_t m_chip_select;

	/* sound CPU 6850 states */
	bool m_uint;

	/* noise generator states */
	uint32_t m_noise_position[6];
	required_device_array<cem3394_device, 6> m_cem_device;

	/* game-specific states */
	uint8_t m_nstocker_bits;
	uint8_t m_spiker_expand_color;
	uint8_t m_spiker_expand_bgcolor;
	uint8_t m_spiker_expand_bits;
	uint8_t m_grudge_steering_result;
	uint8_t m_grudge_last_steering[3];
	uint8_t m_teamht_input;

	/* video data */
	uint8_t m_expanded_videoram[256*256];
	uint8_t *m_sprite_data;
	uint32_t m_sprite_mask;
	uint8_t *m_sprite_bank[2];

	uint8_t m_palettebank_vis;

	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint16_t> m_shrike_io;
	optional_shared_ptr<uint16_t> m_shrike_shared;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_68k;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;
	required_device_array<x2212_device, 2> m_novram;
	required_device<acia6850_device> m_acia;
	required_device<acia6850_device> m_audiouart;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;
};

#endif // MAME_INCLUDES_BALSENTE_H
