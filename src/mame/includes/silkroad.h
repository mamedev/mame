// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont
#include "sound/okim6295.h"

class silkroad_state : public driver_device
{
public:
	silkroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki1(*this, "oki1"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram"),
		m_vidram(*this, "vidram"),
		m_vidram2(*this, "vidram2"),
		m_vidram3(*this, "vidram3"),
		m_sprram(*this, "sprram"),
		m_regs(*this, "regs") { }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki1;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<UINT32> m_generic_paletteram_32;
	required_shared_ptr<UINT32> m_vidram;
	required_shared_ptr<UINT32> m_vidram2;
	required_shared_ptr<UINT32> m_vidram3;
	required_shared_ptr<UINT32> m_sprram;
	required_shared_ptr<UINT32> m_regs;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_fg2_tilemap;
	tilemap_t *m_fg3_tilemap;

	DECLARE_WRITE32_MEMBER(paletteram32_xRRRRRGGGGGBBBBB_dword_w);
	DECLARE_WRITE32_MEMBER(silk_coin_counter_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram2_w);
	DECLARE_WRITE32_MEMBER(silkroad_fgram3_w);
	DECLARE_WRITE32_MEMBER(silk_6295_bank_w);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg2_tile_info);
	TILE_GET_INFO_MEMBER(get_fg3_tile_info);

	virtual void video_start() override;

	UINT32 screen_update_silkroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
