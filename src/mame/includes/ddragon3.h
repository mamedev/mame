/*************************************************************************

    Double Dragon 3 & The Combatribes

*************************************************************************/


class ddragon3_state : public driver_device
{
public:
	ddragon3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *        m_bg_videoram;
	UINT16 *        m_fg_videoram;
	UINT16 *        m_spriteram;
//  UINT16 *        m_paletteram; // currently this uses generic palette handling

	/* video-related */
	tilemap_t         *m_fg_tilemap;
	tilemap_t         *m_bg_tilemap;
	UINT16          m_vreg;
	UINT16          m_bg_scrollx;
	UINT16          m_bg_scrolly;
	UINT16          m_fg_scrollx;
	UINT16          m_fg_scrolly;
	UINT16          m_bg_tilebase;

	/* misc */
	UINT16          m_io_reg[8];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(ddragon3_io_w);
	DECLARE_WRITE16_MEMBER(ddragon3_scroll_w);
	DECLARE_READ16_MEMBER(ddragon3_scroll_r);
	DECLARE_WRITE16_MEMBER(ddragon3_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(ddragon3_fg_videoram_w);
};


/*----------- defined in video/ddragon3.c -----------*/


extern VIDEO_START( ddragon3 );
extern SCREEN_UPDATE_IND16( ddragon3 );
extern SCREEN_UPDATE_IND16( ctribe );
