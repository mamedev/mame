class terracre_state : public driver_device
{
public:
	terracre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_amazon_videoram(*this, "amazon_videoram"),
		m_videoram(*this, "videoram"){ }

	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_amazon_videoram;
	required_shared_ptr<UINT16> m_videoram;

	const UINT16 *m_mpProtData;
	UINT8 m_mAmazonProtCmd;
	UINT8 m_mAmazonProtReg[6];
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	DECLARE_READ16_MEMBER(horekid_IN2_r);
	DECLARE_WRITE16_MEMBER(amazon_sound_w);
	DECLARE_READ8_MEMBER(soundlatch_clear_r);
	DECLARE_READ16_MEMBER(amazon_protection_r);
	DECLARE_WRITE16_MEMBER(amazon_protection_w);
	DECLARE_WRITE16_MEMBER(amazon_background_w);
	DECLARE_WRITE16_MEMBER(amazon_foreground_w);
	DECLARE_WRITE16_MEMBER(amazon_flipscreen_w);
	DECLARE_WRITE16_MEMBER(amazon_scrolly_w);
	DECLARE_WRITE16_MEMBER(amazon_scrollx_w);
	DECLARE_DRIVER_INIT(amazon);
	DECLARE_DRIVER_INIT(amatelas);
	DECLARE_DRIVER_INIT(horekid);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
};


/*----------- defined in video/terracre.c -----------*/

PALETTE_INIT( amazon );
VIDEO_START( amazon );
SCREEN_UPDATE_IND16( amazon );
