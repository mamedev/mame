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
		m_maincpu(*this, "maincpu"),
		m_samples(*this, "samples"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	optional_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	typedef pen_t (cosmic_state::*color_func)(UINT8 x, UINT8 y);
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
	UINT32         m_pixel_clock;
	int            m_ic_state;   // for 9980

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<samples_device> m_samples;
	required_device<dac_device> m_dac;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	DECLARE_WRITE8_MEMBER(panic_sound_output_w);
	DECLARE_WRITE8_MEMBER(panic_sound_output2_w);
	DECLARE_WRITE8_MEMBER(cosmicg_output_w);
	DECLARE_WRITE8_MEMBER(cosmica_sound_output_w);
	DECLARE_READ8_MEMBER(cosmica_pixel_clock_r);
	DECLARE_READ8_MEMBER(cosmicg_port_0_r);
	DECLARE_READ8_MEMBER(magspot_coinage_dip_r);
	DECLARE_READ8_MEMBER(nomnlnd_port_0_1_r);
	DECLARE_WRITE8_MEMBER(flip_screen_w);
	DECLARE_WRITE8_MEMBER(cosmic_color_register_w);
	DECLARE_WRITE8_MEMBER(cosmic_background_enable_w);
	DECLARE_INPUT_CHANGED_MEMBER(panic_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmica_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(cosmicg_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_irq0);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted_nmi);
	DECLARE_DRIVER_INIT(devzone);
	DECLARE_DRIVER_INIT(cosmicg);
	DECLARE_DRIVER_INIT(nomnlnd);
	DECLARE_DRIVER_INIT(cosmica);
	DECLARE_DRIVER_INIT(panic);
	DECLARE_MACHINE_START(cosmic);
	DECLARE_MACHINE_RESET(cosmic);
	DECLARE_MACHINE_RESET(cosmicg);
	DECLARE_PALETTE_INIT(cosmicg);
	DECLARE_PALETTE_INIT(panic);
	DECLARE_PALETTE_INIT(cosmica);
	DECLARE_PALETTE_INIT(magspot);
	DECLARE_PALETTE_INIT(nomnlnd);
	UINT32 screen_update_cosmicg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_panic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cosmica(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_magspot(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_devzone(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_nomnlnd(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(panic_scanline);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int color_mask, int extra_sprites);
	void cosmica_draw_starfield(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void devzone_draw_grid(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void nomnlnd_draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	pen_t panic_map_color(UINT8 x, UINT8 y);
	pen_t cosmica_map_color(UINT8 x, UINT8 y);
	pen_t cosmicg_map_color(UINT8 x, UINT8 y);
	pen_t magspot_map_color(UINT8 x, UINT8 y);
};
