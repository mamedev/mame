/***************************************************************************

    Bank Panic

***************************************************************************/

class bankp_state : public driver_device
{
public:
	bankp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_videoram2(*this, "videoram2"),
		m_colorram2(*this, "colorram2"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram2;
	required_shared_ptr<UINT8> m_colorram2;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	int     m_scroll_x;
	int     m_priority;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(bankp_scroll_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram_w);
	DECLARE_WRITE8_MEMBER(bankp_videoram2_w);
	DECLARE_WRITE8_MEMBER(bankp_colorram2_w);
	DECLARE_WRITE8_MEMBER(bankp_out_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/bankp.c -----------*/




SCREEN_UPDATE_IND16( bankp );


