/***************************************************************************

    Double Dribble

***************************************************************************/

class ddribble_state : public driver_device
{
public:
	ddribble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *     m_sharedram;
	UINT8 *     m_snd_sharedram;
	UINT8 *     m_spriteram_1;
	UINT8 *     m_spriteram_2;
	UINT8 *     m_bg_videoram;
	UINT8 *     m_fg_videoram;
	UINT8 *     m_paletteram;

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
};

/*----------- defined in video/ddribble.c -----------*/


PALETTE_INIT( ddribble );
VIDEO_START( ddribble );
SCREEN_UPDATE_IND16( ddribble );
