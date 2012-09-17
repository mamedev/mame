/*************************************************************************

    Mosaic

*************************************************************************/

class mosaic_state : public driver_device
{
public:
	mosaic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgvideoram(*this, "fgvideoram"),
		m_bgvideoram(*this, "bgvideoram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_bgvideoram;
//      UINT8 *        m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;

	/* misc */
	int            m_prot_val;
	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(gfire2_protection_w);
	DECLARE_READ8_MEMBER(gfire2_protection_r);
	DECLARE_WRITE8_MEMBER(mosaic_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(mosaic_bgvideoram_w);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_mosaic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/mosaic.c -----------*/




