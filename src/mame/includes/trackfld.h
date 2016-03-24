// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/***************************************************************************

    Track'n'Field

***************************************************************************/

#include "sound/sn76496.h"
#include "sound/vlm5030.h"

class trackfld_state : public driver_device
{
public:
	trackfld_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram2(*this, "spriteram2"),
		m_scroll(*this, "scroll"),
		m_spriteram(*this, "spriteram"),
		m_scroll2(*this, "scroll2"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_sn(*this, "snsnd"),
		m_vlm(*this, "vlm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram2;
	required_shared_ptr<UINT8> m_scroll;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll2;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<sn76496_device> m_sn;
	optional_device<vlm5030_device> m_vlm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int      m_bg_bank;
	int      m_sprite_bank1;
	int      m_sprite_bank2;
	int      m_old_gfx_bank;                    // needed by atlantol
	int      m_sprites_gfx_banked;

	UINT8    m_irq_mask;
	UINT8    m_yieartf_nmi_mask;
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(questions_bank_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(yieartf_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(trackfld_videoram_w);
	DECLARE_WRITE8_MEMBER(trackfld_colorram_w);
	DECLARE_WRITE8_MEMBER(trackfld_flipscreen_w);
	DECLARE_WRITE8_MEMBER(atlantol_gfxbank_w);
	DECLARE_READ8_MEMBER(trackfld_SN76496_r);
	DECLARE_READ8_MEMBER(trackfld_speech_r);
	DECLARE_WRITE8_MEMBER(trackfld_VLM5030_control_w);

	DECLARE_DRIVER_INIT(trackfld);
	DECLARE_DRIVER_INIT(atlantol);
	DECLARE_DRIVER_INIT(wizzquiz);
	DECLARE_DRIVER_INIT(mastkin);
	DECLARE_DRIVER_INIT(trackfldnz);

	UINT8 m_SN76496_latch;
	DECLARE_WRITE8_MEMBER( konami_SN76496_latch_w ) { m_SN76496_latch = data; };
	DECLARE_WRITE8_MEMBER( konami_SN76496_w ) { m_sn->write(space, offset, m_SN76496_latch); };
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_START(trackfld);
	DECLARE_MACHINE_RESET(trackfld);
	DECLARE_VIDEO_START(trackfld);
	DECLARE_PALETTE_INIT(trackfld);
	DECLARE_VIDEO_START(atlantol);
	UINT32 screen_update_trackfld(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(vblank_nmi);
	INTERRUPT_GEN_MEMBER(yieartf_timer_irq);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
};
