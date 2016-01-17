// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

#include "includes/galaxold.h"

class fastfred_state : public galaxold_state
{
public:
	fastfred_state(const machine_config &mconfig, device_type type, std::string tag)
		: galaxold_state(mconfig, type, tag),
			m_gfxdecode(*this, "gfxdecode"),
			m_palette(*this, "palette"),
			m_videoram(*this, "videoram"),
			m_spriteram(*this, "spriteram"),
			m_attributesram(*this, "attributesram"),
			m_background_color(*this, "bgcolor"),
			m_imago_fg_videoram(*this, "imago_fg_vram") { }

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_attributesram;
	optional_shared_ptr<UINT8> m_background_color;
	optional_shared_ptr<UINT8> m_imago_fg_videoram;

	int m_hardware_type;
	UINT16 m_charbank;
	UINT8 m_colorbank;
	UINT8 m_nmi_mask;
	UINT8 m_sound_nmi_mask;
	UINT8 m_imago_sprites[0x800*3];
	UINT16 m_imago_sprites_address;
	UINT8 m_imago_sprites_bank;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_web_tilemap;

	DECLARE_READ8_MEMBER(fastfred_custom_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom1_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom2_io_r);
	DECLARE_READ8_MEMBER(jumpcoas_custom_io_r);
	DECLARE_READ8_MEMBER(boggy84_custom_io_r);
	DECLARE_WRITE8_MEMBER(imago_dma_irq_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_bank_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_dma_w);
	DECLARE_READ8_MEMBER(imago_sprites_offset_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(fastfred_videoram_w);
	DECLARE_WRITE8_MEMBER(fastfred_attributes_w);
	DECLARE_WRITE8_MEMBER(fastfred_charbank1_w);
	DECLARE_WRITE8_MEMBER(fastfred_charbank2_w);
	DECLARE_WRITE8_MEMBER(fastfred_colorbank1_w);
	DECLARE_WRITE8_MEMBER(fastfred_colorbank2_w);
	DECLARE_WRITE8_MEMBER(fastfred_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(fastfred_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(imago_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(imago_charbank_w);

	DECLARE_DRIVER_INIT(fastfred);
	DECLARE_DRIVER_INIT(flyboy);
	DECLARE_DRIVER_INIT(flyboyb);
	DECLARE_DRIVER_INIT(imago);
	DECLARE_DRIVER_INIT(boggy84);
	DECLARE_DRIVER_INIT(jumpcoas);
	DECLARE_DRIVER_INIT(boggy84b);

	TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_bg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_fg);
	TILE_GET_INFO_MEMBER(imago_get_tile_info_web);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_timer_irq);

	virtual void machine_start() override;
	DECLARE_PALETTE_INIT(fastfred);
	DECLARE_MACHINE_START(imago);
	DECLARE_VIDEO_START(fastfred);
	DECLARE_VIDEO_START(imago);

	UINT32 screen_update_fastfred(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_imago(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
