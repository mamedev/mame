// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "sound/namco.h"

class pacland_state : public driver_device
{
public:
	pacland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	UINT8 m_palette_bank;
	const UINT8 *m_color_prom;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_fg_bitmap;
	UINT32 *m_transmask[3];
	UINT16 m_scroll0;
	UINT16 m_scroll1;
	UINT8 m_main_irq_mask;
	UINT8 m_mcu_irq_mask;
	DECLARE_WRITE8_MEMBER(pacland_subreset_w);
	DECLARE_WRITE8_MEMBER(pacland_flipscreen_w);
	DECLARE_READ8_MEMBER(pacland_input_r);
	DECLARE_WRITE8_MEMBER(pacland_coin_w);
	DECLARE_WRITE8_MEMBER(pacland_led_w);
	DECLARE_WRITE8_MEMBER(pacland_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(pacland_irq_2_ctrl_w);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_WRITE8_MEMBER(pacland_videoram_w);
	DECLARE_WRITE8_MEMBER(pacland_videoram2_w);
	DECLARE_WRITE8_MEMBER(pacland_scroll0_w);
	DECLARE_WRITE8_MEMBER(pacland_scroll1_w);
	DECLARE_WRITE8_MEMBER(pacland_bankswitch_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(pacland);
	UINT32 screen_update_pacland(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(mcu_vblank_irq);
	void switch_palette();
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int flip, int whichmask);
	void draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority );
};
