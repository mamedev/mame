// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scrollram_0(*this, "scrollram_0"),
		m_scrollram_1(*this, "scrollram_1"),
		m_priorityram(*this, "priorityram"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_scrollram_0;
	required_shared_ptr<UINT16> m_scrollram_1;
	required_shared_ptr<UINT16> m_priorityram;
	required_shared_ptr<UINT16> m_spriteram;

	/* other video-related elements */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	int         m_sprites_scrolldx;
	int         m_sprites_scrolldy;

	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE16_MEMBER(magicbub_sound_command_w);
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);
	DECLARE_DRIVER_INIT(magicbub);
	DECLARE_MACHINE_START(shocking);
	DECLARE_MACHINE_RESET(shocking);
	TILEMAP_MAPPER_MEMBER(tilemap_scan_pages);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_yunsun16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
