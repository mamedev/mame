/******************************************************************************

    Data East Side Pocket hardware

******************************************************************************/

class sidepckt_state : public driver_device
{
public:
	sidepckt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	tilemap_t *m_bg_tilemap;
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	const UINT8* m_prot_table[3];
	UINT8 m_i8751_return;
	UINT8 m_current_ptr;
	UINT8 m_current_table;
	UINT8 m_in_math;
	UINT8 m_math_param;

	DECLARE_WRITE8_MEMBER(sound_cpu_command_w);
	DECLARE_READ8_MEMBER(sidepckt_i8751_r);
	DECLARE_WRITE8_MEMBER(sidepckt_i8751_w);
	DECLARE_WRITE8_MEMBER(sidepctj_i8751_w);
	DECLARE_WRITE8_MEMBER(sidepckt_videoram_w);
	DECLARE_WRITE8_MEMBER(sidepckt_colorram_w);
	DECLARE_WRITE8_MEMBER(sidepckt_flipscreen_w);
	DECLARE_DRIVER_INIT(sidepckt);
	DECLARE_DRIVER_INIT(sidepcktj);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_PALETTE_INIT(sidepckt);
	UINT32 screen_update_sidepckt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
