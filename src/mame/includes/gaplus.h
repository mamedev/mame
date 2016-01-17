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

	gaplus_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_15xx(*this, "namco"),
		m_samples(*this, "samples") ,
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_customio_3(*this,"customio_3"),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_subcpu2;
	required_device<namco_15xx_device> m_namco_15xx;
	required_device<samples_device> m_samples;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	namco58xx_device *m_namco58xx;
	namco56xx_device *m_namco56xx;

	required_shared_ptr<UINT8> m_customio_3;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	int m_type;

	tilemap_t *m_bg_tilemap;
	UINT8 m_starfield_control[4];
	int m_total_stars;
	struct star m_stars[MAX_STARS];
	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;

	DECLARE_WRITE8_MEMBER(irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(irq_2_ctrl_w);
	DECLARE_WRITE8_MEMBER(irq_3_ctrl_w);
	DECLARE_WRITE8_MEMBER(sreset_w);
	DECLARE_WRITE8_MEMBER(freset_w);
	DECLARE_WRITE8_MEMBER(customio_3_w);
	DECLARE_READ8_MEMBER(customio_3_r);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(starfield_control_w);
	DECLARE_WRITE8_MEMBER(out_lamps0);
	DECLARE_WRITE8_MEMBER(out_lamps1);

	DECLARE_DRIVER_INIT(gaplus);
	DECLARE_DRIVER_INIT(gaplusd);
	DECLARE_DRIVER_INIT(galaga3);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(gaplus);

	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);

	INTERRUPT_GEN_MEMBER(vblank_main_irq);
	INTERRUPT_GEN_MEMBER(gapluso_vblank_main_irq);
	INTERRUPT_GEN_MEMBER(vblank_sub_irq);
	INTERRUPT_GEN_MEMBER(vblank_sub2_irq);
	TIMER_CALLBACK_MEMBER(namcoio_run);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);
	void starfield_init();
	void starfield_render(bitmap_ind16 &bitmap);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
