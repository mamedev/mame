/***************************************************************************

    Break Thru

***************************************************************************/

class brkthru_state : public driver_device
{
public:
	brkthru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fg_videoram(*this, "fg_videoram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	int     m_bgscroll;
	int     m_bgbasecolor;
	int     m_flipscreen;
	//UINT8 *m_brkthru_nmi_enable; /* needs to be tracked down */

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;

	UINT8   m_nmi_mask;
	DECLARE_WRITE8_MEMBER(brkthru_1803_w);
	DECLARE_WRITE8_MEMBER(darwin_0803_w);
	DECLARE_WRITE8_MEMBER(brkthru_soundlatch_w);
	DECLARE_WRITE8_MEMBER(brkthru_bgram_w);
	DECLARE_WRITE8_MEMBER(brkthru_fgram_w);
	DECLARE_WRITE8_MEMBER(brkthru_1800_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(brkthru);
};


/*----------- defined in video/brkthru.c -----------*/

VIDEO_START( brkthru );
PALETTE_INIT( brkthru );
SCREEN_UPDATE_IND16( brkthru );
