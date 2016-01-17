// license:BSD-3-Clause
// copyright-holders:Acho A. Tang, Alex W. Jackson
/***************************************************************************

    B-Wings

***************************************************************************/

#include "machine/bankdev.h"

#define BW_DEBUG 0

class bwing_state : public driver_device
{
public:
	bwing_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vrambank(*this, "vrambank"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_fgscrollram(*this, "fgscrollram"),
		m_bgscrollram(*this, "bgscrollram"),
		m_gfxram(*this, "gfxram") { }

	/* device */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_vrambank;

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_fgscrollram;
	required_shared_ptr<UINT8> m_bgscrollram;
	required_shared_ptr<UINT8> m_gfxram;

	/* video-related */
	tilemap_t *m_charmap;
	tilemap_t *m_fgmap;
	tilemap_t *m_bgmap;
	unsigned m_sreg[8];
	unsigned m_palatch;
	unsigned m_mapmask;

	/* sound-related */
	int m_bwp3_nmimask;
	int m_bwp3_u8F_d;

	DECLARE_WRITE8_MEMBER(bwp3_u8F_w);
	DECLARE_WRITE8_MEMBER(bwp3_nmimask_w);
	DECLARE_WRITE8_MEMBER(bwp3_nmiack_w);
	DECLARE_WRITE8_MEMBER(bwp1_ctrl_w);
	DECLARE_WRITE8_MEMBER(bwp2_ctrl_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(fgscrollram_w);
	DECLARE_WRITE8_MEMBER(bgscrollram_w);
	DECLARE_WRITE8_MEMBER(gfxram_w);
	DECLARE_WRITE8_MEMBER(scrollreg_w);
	DECLARE_WRITE8_MEMBER(paletteram_w);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(tilt_pressed);

	TILE_GET_INFO_MEMBER(get_fgtileinfo);
	TILE_GET_INFO_MEMBER(get_bgtileinfo);
	TILE_GET_INFO_MEMBER(get_charinfo);
	TILEMAP_MAPPER_MEMBER(scan_cols);

	DECLARE_DRIVER_INIT(bwing);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void bwing_postload();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( bitmap_ind16 &bmp, const rectangle &clip, UINT8 *ram, int pri );

	INTERRUPT_GEN_MEMBER(bwp3_interrupt);
};
