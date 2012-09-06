/***************************************************************************

    Double Dribble

***************************************************************************/

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_spriteram_1(*this, "spriteram_1"),
		m_sharedram(*this, "sharedram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_snd_sharedram(*this, "snd_sharedram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_spriteram_1;
	required_shared_ptr<UINT8> m_sharedram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_spriteram_2;
	required_shared_ptr<UINT8> m_snd_sharedram;

	/* video-related */
	tilemap_t     *m_fg_tilemap;
	tilemap_t     *m_bg_tilemap;
	int         m_vregs[2][5];
	int         m_charbank[2];

	/* misc */
	int         m_int_enable_0;
	int         m_int_enable_1;

	/* devices */
	device_t *m_filter1;
	device_t *m_filter2;
	device_t *m_filter3;
	DECLARE_WRITE8_MEMBER(ddribble_bankswitch_w);
	DECLARE_READ8_MEMBER(ddribble_sharedram_r);
	DECLARE_WRITE8_MEMBER(ddribble_sharedram_w);
	DECLARE_READ8_MEMBER(ddribble_snd_sharedram_r);
	DECLARE_WRITE8_MEMBER(ddribble_snd_sharedram_w);
	DECLARE_WRITE8_MEMBER(ddribble_coin_counter_w);
	DECLARE_WRITE8_MEMBER(K005885_0_w);
	DECLARE_WRITE8_MEMBER(K005885_1_w);
	DECLARE_WRITE8_MEMBER(ddribble_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(ddribble_bg_videoram_w);
	DECLARE_READ8_MEMBER(ddribble_vlm5030_busy_r);
	DECLARE_WRITE8_MEMBER(ddribble_vlm5030_ctrl_w);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
};

/*----------- defined in video/ddribble.c -----------*/


PALETTE_INIT( ddribble );
VIDEO_START( ddribble );
SCREEN_UPDATE_IND16( ddribble );
