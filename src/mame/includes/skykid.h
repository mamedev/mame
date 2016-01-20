// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "sound/namco.h"

class skykid_state : public driver_device
{
public:
	skykid_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_textram(*this, "textram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_cus30(*this, "namco"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	UINT8 m_inputport_selected;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_textram;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
	required_device<namco_cus30_device> m_cus30;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_tx_tilemap;
	UINT8 m_priority;
	UINT16 m_scroll_x;
	UINT16 m_scroll_y;
	UINT8 m_main_irq_mask;
	UINT8 m_mcu_irq_mask;
	DECLARE_WRITE8_MEMBER(inputport_select_w);
	DECLARE_READ8_MEMBER(inputport_r);
	DECLARE_WRITE8_MEMBER(skykid_led_w);
	DECLARE_WRITE8_MEMBER(skykid_subreset_w);
	DECLARE_WRITE8_MEMBER(skykid_bankswitch_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_1_ctrl_w);
	DECLARE_WRITE8_MEMBER(skykid_irq_2_ctrl_w);
	DECLARE_READ8_MEMBER(readFF);
	DECLARE_READ8_MEMBER(skykid_videoram_r);
	DECLARE_WRITE8_MEMBER(skykid_videoram_w);
	DECLARE_READ8_MEMBER(skykid_textram_r);
	DECLARE_WRITE8_MEMBER(skykid_textram_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_x_w);
	DECLARE_WRITE8_MEMBER(skykid_scroll_y_w);
	DECLARE_WRITE8_MEMBER(skykid_flipscreen_priority_w);
	DECLARE_DRIVER_INIT(skykid);
	TILEMAP_MAPPER_MEMBER(tx_tilemap_scan);
	TILE_GET_INFO_MEMBER(tx_get_tile_info);
	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(skykid);
	UINT32 screen_update_skykid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(main_vblank_irq);
	INTERRUPT_GEN_MEMBER(mcu_vblank_irq);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
