// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class sbasketb_state : public driver_device
{
public:
	sbasketb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_palettebank(*this, "palettebank"),
		m_spriteram_select(*this, "spriteramsel"),
		m_scroll(*this, "scroll"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_palettebank;
	required_shared_ptr<UINT8> m_spriteram_select;
	required_shared_ptr<UINT8> m_scroll;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<sn76489_device> m_sn;
	required_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;

	UINT8    m_irq_mask;
	DECLARE_WRITE8_MEMBER(sbasketb_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(sbasketb_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(sbasketb_videoram_w);
	DECLARE_WRITE8_MEMBER(sbasketb_colorram_w);
	DECLARE_WRITE8_MEMBER(sbasketb_flipscreen_w);
	DECLARE_DRIVER_INIT(sbasketb);

	UINT8 m_SN76496_latch;
	DECLARE_WRITE8_MEMBER( konami_SN76496_latch_w ) { m_SN76496_latch = data; };
	DECLARE_WRITE8_MEMBER( konami_SN76496_w ) { m_sn->write(space, offset, m_SN76496_latch); };
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(sbasketb);
	UINT32 screen_update_sbasketb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
