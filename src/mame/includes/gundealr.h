/*************************************************************************

    Gun Dealer

*************************************************************************/

class gundealr_state : public driver_device
{
public:
	gundealr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_rambase(*this, "rambase"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_paletteram;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_rambase;

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	tilemap_t    *m_fg_tilemap;
	int        m_flipscreen;
	UINT8      m_scroll[4];

	/* misc */
	int        m_input_ports_hack;
	DECLARE_WRITE8_MEMBER(yamyam_bankswitch_w);
	DECLARE_WRITE8_MEMBER(gundealr_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(gundealr_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(gundealr_paletteram_w);
	DECLARE_WRITE8_MEMBER(gundealr_fg_scroll_w);
	DECLARE_WRITE8_MEMBER(yamyam_fg_scroll_w);
	DECLARE_WRITE8_MEMBER(gundealr_flipscreen_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(gundealr_scan);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};



/*----------- defined in video/gundealr.c -----------*/


SCREEN_UPDATE_IND16( gundealr );

