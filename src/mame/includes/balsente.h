// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Bally/Sente SAC-1 system

    driver by Aaron Giles

***************************************************************************/

#include "machine/timer.h"
#include "machine/74259.h"
#include "sound/cem3394.h"
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
		: driver_device(mconfig, type, tag),
			m_scanline_timer(*this, "scan_timer"),
			m_counter_0_timer(*this, "8253_0_timer"),
			m_cem1(*this, "cem1"),
			m_cem2(*this, "cem2"),
			m_cem3(*this, "cem3"),
			m_cem4(*this, "cem4"),
			m_cem5(*this, "cem5"),
			m_cem6(*this, "cem6") ,
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_shrike_io(*this, "shrike_io"),
		m_shrike_shared(*this, "shrike_shared"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_68k(*this, "68k"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_outlatch(*this, "outlatch"),
		m_generic_paletteram_8(*this, "paletteram") { }

	required_device<timer_device> m_scanline_timer;

	/* global data */
	uint8_t m_shooter;
	uint8_t m_shooter_x;
	uint8_t m_shooter_y;
	uint8_t m_adc_shift;

	/* 8253 counter state */
	struct
	{
		timer_device *timer;
		uint8_t timer_active;
		int32_t initial;
		int32_t count;
		uint8_t gate;
		uint8_t out;
		uint8_t mode;
		uint8_t readbyte;
		uint8_t writebyte;
	} m_counter[3];


	/* manually clocked counter 0 states */
	uint8_t m_counter_control;
	uint8_t m_counter_0_ff;
	required_device<timer_device> m_counter_0_timer;
	uint8_t m_counter_0_timer_active;

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

	/* main CPU 6850 states */
	uint8_t m_m6850_status;
	uint8_t m_m6850_control;
	uint8_t m_m6850_input;
	uint8_t m_m6850_output;
	uint8_t m_m6850_data_ready;

	/* sound CPU 6850 states */
	uint8_t m_m6850_sound_status;
	uint8_t m_m6850_sound_control;
	uint8_t m_m6850_sound_input;
	uint8_t m_m6850_sound_output;

	/* noise generator states */
	uint32_t m_noise_position[6];
	required_device<cem3394_device> m_cem1;
	required_device<cem3394_device> m_cem2;
	required_device<cem3394_device> m_cem3;
	required_device<cem3394_device> m_cem4;
	required_device<cem3394_device> m_cem5;
	required_device<cem3394_device> m_cem6;
	cem3394_device *m_cem_device[6];

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

	DECLARE_WRITE8_MEMBER(balsente_random_reset_w);
	DECLARE_READ8_MEMBER(balsente_random_num_r);
	DECLARE_WRITE8_MEMBER(balsente_rombank_select_w);
	DECLARE_WRITE8_MEMBER(balsente_rombank2_select_w);
	DECLARE_WRITE_LINE_MEMBER(out0_w);
	DECLARE_WRITE_LINE_MEMBER(out1_w);
	DECLARE_WRITE_LINE_MEMBER(out2_w);
	DECLARE_WRITE_LINE_MEMBER(out3_w);
	DECLARE_WRITE_LINE_MEMBER(out4_w);
	DECLARE_WRITE_LINE_MEMBER(out5_w);
	DECLARE_WRITE_LINE_MEMBER(out6_w);
	DECLARE_WRITE_LINE_MEMBER(nvrecall_w);
	DECLARE_READ8_MEMBER(balsente_m6850_r);
	DECLARE_WRITE8_MEMBER(balsente_m6850_w);
	DECLARE_READ8_MEMBER(balsente_m6850_sound_r);
	DECLARE_WRITE8_MEMBER(balsente_m6850_sound_w);
	DECLARE_READ8_MEMBER(balsente_adc_data_r);
	DECLARE_WRITE8_MEMBER(balsente_adc_select_w);
	DECLARE_READ8_MEMBER(balsente_counter_8253_r);
	DECLARE_WRITE8_MEMBER(balsente_counter_8253_w);
	DECLARE_READ8_MEMBER(balsente_counter_state_r);
	DECLARE_WRITE8_MEMBER(balsente_counter_control_w);
	DECLARE_WRITE8_MEMBER(balsente_chip_select_w);
	DECLARE_WRITE8_MEMBER(balsente_dac_data_w);
	DECLARE_WRITE8_MEMBER(balsente_register_addr_w);
	DECLARE_WRITE8_MEMBER(spiker_expand_w);
	DECLARE_READ8_MEMBER(spiker_expand_r);
	DECLARE_READ8_MEMBER(grudge_steering_r);
	DECLARE_READ8_MEMBER(shrike_shared_6809_r);
	DECLARE_WRITE8_MEMBER(shrike_shared_6809_w);
	DECLARE_WRITE16_MEMBER(shrike_io_68k_w);
	DECLARE_READ16_MEMBER(shrike_io_68k_r);
	DECLARE_READ8_MEMBER(teamht_extra_r);
	DECLARE_WRITE8_MEMBER(teamht_multiplex_select_w);

	void counter_set_out(int which, int out);
	void counter_start(int which);
	void counter_stop( int which);
	void counter_update_count(int which);
	void counter_set_gate(int which, int gate);
	void update_counter_0_timer();
	DECLARE_WRITE8_MEMBER(balsente_videoram_w);
	DECLARE_WRITE8_MEMBER(balsente_palette_select_w);
	DECLARE_WRITE8_MEMBER(balsente_paletteram_w);
	DECLARE_WRITE8_MEMBER(shrike_sprite_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(nstocker_bits_r);
	void init_otwalls();
	void init_triviaes();
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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_balsente(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(balsente_update_analog_inputs);
	TIMER_CALLBACK_MEMBER(irq_off);
	TIMER_CALLBACK_MEMBER(m6850_data_ready_callback);
	TIMER_CALLBACK_MEMBER(m6850_w_callback);
	TIMER_CALLBACK_MEMBER(adc_finished);
	TIMER_DEVICE_CALLBACK_MEMBER(balsente_interrupt_timer);
	TIMER_DEVICE_CALLBACK_MEMBER(balsente_counter_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(balsente_clock_counter_0_ff);
	void draw_one_sprite(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *sprite);
	void poly17_init();
	void m6850_update_io();
	void set_counter_0_ff(timer_device &timer, int newstate);
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
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_68k;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ls259_device> m_outlatch;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;
	void shrike(machine_config &config);
	void balsente(machine_config &config);
	void cpu1_map(address_map &map);
	void cpu2_io_map(address_map &map);
	void cpu2_map(address_map &map);
	void shrike68k_map(address_map &map);
};


/*----------- defined in machine/balsente.c -----------*/
void balsente_noise_gen(device_t *device, int count, short *buffer);
