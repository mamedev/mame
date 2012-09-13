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
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;

	UINT8   m_nmi_mask;
	DECLARE_WRITE8_MEMBER(brkthru_1803_w);
	DECLARE_WRITE8_MEMBER(darwin_0803_w);
	DECLARE_WRITE8_MEMBER(brkthru_soundlatch_w);
	DECLARE_WRITE8_MEMBER(brkthru_bgram_w);
	DECLARE_WRITE8_MEMBER(brkthru_fgram_w);
	DECLARE_WRITE8_MEMBER(brkthru_1800_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(brkthru);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/brkthru.c -----------*/



SCREEN_UPDATE_IND16( brkthru );
