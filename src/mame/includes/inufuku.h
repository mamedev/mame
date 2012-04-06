
class inufuku_state : public driver_device
{
public:
	inufuku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_bg_videoram;
	UINT16 *  m_bg_rasterram;
	UINT16 *  m_tx_videoram;
	UINT16 *  m_spriteram1;
	UINT16 *  m_spriteram2;
//      UINT16 *  m_paletteram;    // currently this uses generic palette handling
	size_t    m_spriteram1_size;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	tilemap_t  *m_tx_tilemap;
	int       m_bg_scrollx;
	int       m_bg_scrolly;
	int       m_tx_scrollx;
	int       m_tx_scrolly;
	int       m_bg_raster;
	int       m_bg_palettebank;
	int       m_tx_palettebank;

	/* misc */
	UINT16    m_pending_command;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(inufuku_soundcommand_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(inufuku_soundrombank_w);
	DECLARE_WRITE16_MEMBER(inufuku_palettereg_w);
	DECLARE_WRITE16_MEMBER(inufuku_scrollreg_w);
	DECLARE_READ16_MEMBER(inufuku_bg_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_bg_videoram_w);
	DECLARE_READ16_MEMBER(inufuku_tx_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_tx_videoram_w);
};


/*----------- defined in video/inufuku.c -----------*/


SCREEN_UPDATE_IND16( inufuku );
VIDEO_START( inufuku );
