// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Blomby Car

***************************************************************************/

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vram_1(*this, "vram_1"),
		m_vram_0(*this, "vram_0"),
		m_scroll_1(*this, "scroll_1"),
		m_scroll_0(*this, "scroll_0"),
		m_spriteram(*this, "spriteram") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_scroll_1;
	required_shared_ptr<UINT16> m_scroll_0;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;

	/* input-related */
	UINT8       m_pot_wheel;    // blmbycar
	int         m_old_val;  // blmbycar
	int         m_retvalue; // waterball

	// common
	DECLARE_WRITE16_MEMBER(okibank_w);
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);

	// blmbycar
	DECLARE_WRITE16_MEMBER(blmbycar_pot_wheel_reset_w);
	DECLARE_WRITE16_MEMBER(blmbycar_pot_wheel_shift_w);
	DECLARE_READ16_MEMBER(blmbycar_pot_wheel_r);
	DECLARE_READ16_MEMBER(blmbycar_opt_wheel_r);

	// waterball
	DECLARE_READ16_MEMBER(waterball_unk_r);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);

	DECLARE_DRIVER_INIT(blmbycar);
	virtual void video_start() override;
	DECLARE_MACHINE_START(blmbycar);
	DECLARE_MACHINE_RESET(blmbycar);
	DECLARE_MACHINE_START(watrball);
	DECLARE_MACHINE_RESET(watrball);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
};
