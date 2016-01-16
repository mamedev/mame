// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/namcoio.h"
#include "sound/dac.h"
#include "sound/namco.h"

class mappy_state : public driver_device
{
public:
	enum
	{
		TIMER_SUPERPAC_IO_RUN,
		TIMER_PACNPAL_IO_RUN,
		TIMER_GROBDA_IO_RUN,
		TIMER_PHOZON_IO_RUN,
		TIMER_MAPPY_IO_RUN,
		TIMER_DIGDUG2_IO_RUN,
		TIMER_MOTOS_IO_RUN
	};

	enum
	{
		GAME_SUPERPAC = 0,
		GAME_PACNPAL,
		GAME_GROBDA,
		GAME_PHOZON,
		GAME_MAPPY,
		GAME_DRUAGA,
		GAME_DIGDUG2,
		GAME_MOTOS
	};

	mappy_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_namco_15xx(*this, "namco"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	optional_device<cpu_device> m_subcpu2;
	required_device<namco_15xx_device> m_namco_15xx;
	optional_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	namco56xx_device *m_namco56xx_1;
	namco56xx_device *m_namco56xx_2;
	namco58xx_device *m_namco58xx_1;
	namco58xx_device *m_namco58xx_2;
	namco59xx_device *m_namco59xx;

	// per-game variable to distinguish between the various IO chip config
	int m_type;

	tilemap_t *m_bg_tilemap;
	bitmap_ind16 m_sprite_bitmap;

	UINT8 m_scroll;
	int m_mux;

	UINT8 m_main_irq_mask;
	UINT8 m_sub_irq_mask;
	UINT8 m_sub2_irq_mask;

	void common_latch_w(UINT32 offset);
	DECLARE_WRITE8_MEMBER(superpac_latch_w);
	DECLARE_WRITE8_MEMBER(phozon_latch_w);
	DECLARE_WRITE8_MEMBER(mappy_latch_w);
	DECLARE_WRITE8_MEMBER(superpac_videoram_w);
	DECLARE_WRITE8_MEMBER(mappy_videoram_w);
	DECLARE_WRITE8_MEMBER(superpac_flipscreen_w);
	DECLARE_READ8_MEMBER(superpac_flipscreen_r);
	DECLARE_WRITE8_MEMBER(mappy_scroll_w);
	DECLARE_READ8_MEMBER(dipA_l);
	DECLARE_READ8_MEMBER(dipA_h);
	DECLARE_READ8_MEMBER(dipB_mux);
	DECLARE_READ8_MEMBER(dipB_muxi);
	DECLARE_WRITE8_MEMBER(out_mux);
	DECLARE_WRITE8_MEMBER(out_lamps);
	DECLARE_WRITE8_MEMBER(grobda_DAC_w);
	TILEMAP_MAPPER_MEMBER(superpac_tilemap_scan);
	TILEMAP_MAPPER_MEMBER(mappy_tilemap_scan);
	TILE_GET_INFO_MEMBER(superpac_get_tile_info);
	TILE_GET_INFO_MEMBER(phozon_get_tile_info);
	TILE_GET_INFO_MEMBER(mappy_get_tile_info);
	DECLARE_MACHINE_START(mappy);
	DECLARE_MACHINE_RESET(superpac);
	DECLARE_VIDEO_START(superpac);
	DECLARE_PALETTE_INIT(superpac);
	DECLARE_MACHINE_RESET(phozon);
	DECLARE_VIDEO_START(phozon);
	DECLARE_PALETTE_INIT(phozon);
	DECLARE_MACHINE_RESET(mappy);
	DECLARE_VIDEO_START(mappy);
	DECLARE_PALETTE_INIT(mappy);
	UINT32 screen_update_superpac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_phozon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_mappy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(superpac_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(pacnpal_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(grobda_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(phozon_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(mappy_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(digdug2_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(motos_main_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub_vblank_irq);
	INTERRUPT_GEN_MEMBER(sub2_vblank_irq);
	TIMER_CALLBACK_MEMBER(superpac_io_run);
	TIMER_CALLBACK_MEMBER(pacnpal_io_run);
	TIMER_CALLBACK_MEMBER(grobda_io_run);
	TIMER_CALLBACK_MEMBER(phozon_io_run);
	TIMER_CALLBACK_MEMBER(mappy_io_run);
	TIMER_CALLBACK_MEMBER(digdug2_io_run);
	TIMER_CALLBACK_MEMBER(motos_io_run);
	DECLARE_DRIVER_INIT(superpac);
	DECLARE_DRIVER_INIT(pacnpal);
	DECLARE_DRIVER_INIT(grobda);
	DECLARE_DRIVER_INIT(phozon);
	DECLARE_DRIVER_INIT(mappy);
	DECLARE_DRIVER_INIT(druaga);
	DECLARE_DRIVER_INIT(digdug2);
	DECLARE_DRIVER_INIT(motos);
	void mappy_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base);
	void phozon_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 *spriteram_base);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
