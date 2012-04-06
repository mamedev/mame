/***************************************************************************

    1942

***************************************************************************/

class _1942_state : public driver_device
{
public:
	_1942_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_fg_videoram;
	UINT8 * m_bg_videoram;
	UINT8 * m_spriteram;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int m_palette_bank;
	UINT8 m_scroll[2];

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(c1942_bankswitch_w);
	DECLARE_WRITE8_MEMBER(c1942_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(c1942_bgvideoram_w);
	DECLARE_WRITE8_MEMBER(c1942_palette_bank_w);
	DECLARE_WRITE8_MEMBER(c1942_scroll_w);
	DECLARE_WRITE8_MEMBER(c1942_c804_w);
};



/*----------- defined in video/1942.c -----------*/


extern PALETTE_INIT( 1942 );
extern VIDEO_START( 1942 );
extern SCREEN_UPDATE_IND16( 1942 );
