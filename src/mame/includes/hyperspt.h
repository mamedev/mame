// license:BSD-3-Clause
// copyright-holders:Chris Hardy

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class hyperspt_state : public driver_device
{
public:
	hyperspt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_sn(*this, "snsnd"),
		m_maincpu(*this, "maincpu"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	optional_device<sn76496_device> m_sn;
	required_device<cpu_device> m_maincpu;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 *  m_scroll2;
	UINT8 *  m_spriteram2;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_sprites_gfx_banked;

	UINT8    m_irq_mask;
	DECLARE_WRITE8_MEMBER(hyperspt_coin_counter_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(hyperspt_videoram_w);
	DECLARE_WRITE8_MEMBER(hyperspt_colorram_w);
	DECLARE_WRITE8_MEMBER(hyperspt_flipscreen_w);
	DECLARE_DRIVER_INIT(hyperspt);

	UINT8 m_SN76496_latch;
	DECLARE_WRITE8_MEMBER( konami_SN76496_latch_w ) { m_SN76496_latch = data; };
	DECLARE_WRITE8_MEMBER( konami_SN76496_w ) { m_sn->write(space, offset, m_SN76496_latch); };
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(roadf_get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(hyperspt);
	DECLARE_VIDEO_START(roadf);
	UINT32 screen_update_hyperspt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
