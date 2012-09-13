/*************************************************************************

    City Connection

*************************************************************************/

class citycon_state : public driver_device
{
public:
	citycon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_linecolor(*this, "linecolor"),
		m_spriteram(*this, "spriteram"),
		m_scroll(*this, "scroll"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_linecolor;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_scroll;
//  UINT8 *        m_paletteram;  // currently this uses generic palette handling

	/* video-related */
	tilemap_t        *m_bg_tilemap;
	tilemap_t        *m_fg_tilemap;
	int            m_bg_image;

	/* devices */
	cpu_device *m_maincpu;
	DECLARE_READ8_MEMBER(citycon_in_r);
	DECLARE_READ8_MEMBER(citycon_irq_ack_r);
	DECLARE_WRITE8_MEMBER(citycon_videoram_w);
	DECLARE_WRITE8_MEMBER(citycon_linecolor_w);
	DECLARE_WRITE8_MEMBER(citycon_background_w);
	DECLARE_DRIVER_INIT(citycon);
	TILEMAP_MAPPER_MEMBER(citycon_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/citycon.c -----------*/


SCREEN_UPDATE_IND16( citycon );

