// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/upd7759.h"

class prehisle_state : public driver_device
{
public:
	prehisle_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_tx_vram(*this, "tx_vram"),
		m_spriteram(*this, "spriteram"),
		m_fg_vram(*this, "fg_vram"),
		m_tilemap_rom(*this, "bgtilemap"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }


	required_shared_ptr<UINT16> m_tx_vram;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_fg_vram;
	required_region_ptr<UINT8> m_tilemap_rom;
	UINT16 m_invert_controls;

	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_tx_tilemap;

	DECLARE_WRITE16_MEMBER(soundcmd_w);
	DECLARE_WRITE16_MEMBER(fg_vram_w);
	DECLARE_WRITE16_MEMBER(tx_vram_w);
	DECLARE_READ16_MEMBER(control_r);
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(D7759_write_port_0_w);
	DECLARE_WRITE8_MEMBER(D7759_upd_reset_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
	virtual void video_start() override;
	UINT32 screen_update_prehisle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
