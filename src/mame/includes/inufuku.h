
class inufuku_state : public driver_device
{
public:
	inufuku_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_rasterram(*this, "bg_rasterram"),
		m_tx_videoram(*this, "tx_videoram"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram2(*this, "spriteram2"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg_videoram;
	required_shared_ptr<UINT16> m_bg_rasterram;
	required_shared_ptr<UINT16> m_tx_videoram;
	required_shared_ptr<UINT16> m_spriteram1;
	required_shared_ptr<UINT16> m_spriteram2;
//      UINT16 *  m_paletteram;    // currently this uses generic palette handling

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
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(inufuku_soundcommand_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(inufuku_soundrombank_w);
	DECLARE_WRITE16_MEMBER(inufuku_palettereg_w);
	DECLARE_WRITE16_MEMBER(inufuku_scrollreg_w);
	DECLARE_READ16_MEMBER(inufuku_bg_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_bg_videoram_w);
	DECLARE_READ16_MEMBER(inufuku_tx_videoram_r);
	DECLARE_WRITE16_MEMBER(inufuku_tx_videoram_w);
	DECLARE_CUSTOM_INPUT_MEMBER(soundflag_r);
	TILE_GET_INFO_MEMBER(get_inufuku_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_inufuku_tx_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/inufuku.c -----------*/


SCREEN_UPDATE_IND16( inufuku );

