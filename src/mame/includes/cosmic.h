// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor
/*************************************************************************

    Cosmic Guerilla & other Universal boards (in cosmic.c)

*************************************************************************/

#include "sound/samples.h"
#include "sound/dac.h"

#define COSMICG_MASTER_CLOCK     XTAL_9_828MHz
#define Z80_MASTER_CLOCK         XTAL_10_816MHz


class cosmic_state : public driver_device
{
public:
	cosmic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_in_ports(*this, "IN%u", 0),
		m_dsw(*this, "DSW"),
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	typedef pen_t (cosmic_state::*color_func)(uint8_t x, uint8_t y);
	color_func     m_map_color;
	int            m_color_registers[3];
	int            m_background_enable;
	int            m_magspot_pen_mask;

	/* sound-related */
	int            m_sound_enabled;
	int            m_march_select;
	int            m_gun_die_select;
	int            m_dive_bomb_b_select;

	/* misc */
	uint32_t         m_pixel_clock;
	int            m_ic_state;   // for 9980
	optional_ioport_array<4> m_in_ports;
	optional_ioport m_dsw;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	optional_device<dac_bit_interface> m_dac;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	void panic_sound_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void panic_sound_output2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmicg_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmica_sound_output_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void dac_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t cosmica_pixel_clock_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t cosmicg_port_0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t magspot_coinage_dip_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nomnlnd_port_0_1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmic_color_register_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cosmic_background_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void panic_coin_inserted(int state);
	void cosmica_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void cosmicg_coin_inserted(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin_inserted_irq0(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void coin_inserted_nmi(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);
	void init_devzone();
	void init_cosmicg();
	void init_nomnlnd();
	void init_cosmica();
	void init_panic();
	void machine_start_cosmic();
	void machine_reset_cosmic();
	void machine_reset_cosmicg();
	void palette_init_cosmicg(palette_device &palette);
	void palette_init_panic(palette_device &palette);
	void palette_init_cosmica(palette_device &palette);
	void palette_init_magspot(palette_device &palette);
	void palette_init_nomnlnd(palette_device &palette);
	uint32_t screen_update_cosmicg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_panic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cosmica(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_magspot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_devzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_nomnlnd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void panic_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int extra_sprites);
	void cosmica_draw_starfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void devzone_draw_grid(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nomnlnd_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	pen_t panic_map_color(uint8_t x, uint8_t y);
	pen_t cosmica_map_color(uint8_t x, uint8_t y);
	pen_t cosmicg_map_color(uint8_t x, uint8_t y);
	pen_t magspot_map_color(uint8_t x, uint8_t y);
};
