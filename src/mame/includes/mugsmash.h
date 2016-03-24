// license:BSD-3-Clause
// copyright-holders:David Haywood

class mugsmash_state : public driver_device
{
public:
	mugsmash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram1(*this, "videoram1"),
		m_videoram2(*this, "videoram2"),
		m_regs1(*this, "regs1"),
		m_regs2(*this, "regs2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"){ }

	required_shared_ptr<UINT16> m_videoram1;
	required_shared_ptr<UINT16> m_videoram2;
	required_shared_ptr<UINT16> m_regs1;
	required_shared_ptr<UINT16> m_regs2;
	required_shared_ptr<UINT16> m_spriteram;

	tilemap_t *m_tilemap1;
	tilemap_t *m_tilemap2;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_WRITE16_MEMBER(mugsmash_reg2_w);
	DECLARE_WRITE16_MEMBER(mugsmash_videoram1_w);
	DECLARE_WRITE16_MEMBER(mugsmash_videoram2_w);
	DECLARE_WRITE16_MEMBER(mugsmash_reg_w);
	TILE_GET_INFO_MEMBER(get_mugsmash_tile_info1);
	TILE_GET_INFO_MEMBER(get_mugsmash_tile_info2);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_mugsmash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect );
};
