// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Ernesto Corvi, Nicola Salmoria
#include "sound/namco.h"
#include "sound/samples.h"
#include "machine/namcoio.h"

#define MAX_STARS           250

struct star {
	float x,y;
	int col,set;
};


class gaplus_state : public driver_device
{
public:
	enum
	{
		TIMER_NAMCOIO_RUN
	};

	enum
	{
		GAME_GAPLUS = 0,
		GAME_GAPLUSD,
		GAME_GALAGA3
	};

	gaplus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_15xx(*this, "namco"),
		m_samples(*this, "samples") ,
		m_customio_3(*this,"customio_3"),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_15xx_device> m_namco_15xx;
	required_device<samples_device> m_samples;
	required_shared_ptr<UINT8> m_customio_3;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	namco58xx_device *m_namco58xx;
	namco56xx_device *m_namco56xx;

	int m_type;

	tilemap_t *m_bg_tilemap;
	UINT8 m_starfield_control[4];
	int m_total_stars;
	struct star m_stars[MAX_STARS];
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;
	DECLARE_READ8_MEMBER(gaplus_spriteram_r);
	DECLARE_WRITE8_MEMBER(gaplus_spriteram_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_2_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_irq_3_ctrl_w);
	DECLARE_WRITE8_MEMBER(gaplus_sreset_w);
	DECLARE_WRITE8_MEMBER(gaplus_freset_w);
	DECLARE_WRITE8_MEMBER(gaplus_customio_3_w);
	DECLARE_READ8_MEMBER(gaplus_customio_3_r);
	DECLARE_READ8_MEMBER(gaplus_videoram_r);
	DECLARE_WRITE8_MEMBER(gaplus_videoram_w);
	DECLARE_WRITE8_MEMBER(gaplus_starfield_control_w);
	DECLARE_WRITE8_MEMBER(out_lamps0);
	DECLARE_WRITE8_MEMBER(out_lamps1);
	DECLARE_MACHINE_START(gaplus);
	DECLARE_DRIVER_INIT(gaplus);
	DECLARE_DRIVER_INIT(gaplusd);
	DECLARE_DRIVER_INIT(galaga3);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(gaplus);
	UINT32 screen_update_gaplus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_gaplus(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(gaplus_vblank_main_irq);
	INTERRUPT_GEN_MEMBER(gapluso_vblank_main_irq);
	INTERRUPT_GEN_MEMBER(gaplus_vblank_sub_irq);
	INTERRUPT_GEN_MEMBER(gaplus_vblank_sub2_irq);
	TIMER_CALLBACK_MEMBER(namcoio_run);
	void starfield_init();
	void starfield_render(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
