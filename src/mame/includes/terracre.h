class terracre_state : public driver_device
{
public:
	terracre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	const UINT16 *m_mpProtData;
	UINT8 m_mAmazonProtCmd;
	UINT8 m_mAmazonProtReg[6];
	UINT16 *m_amazon_videoram;
	UINT16 m_xscroll;
	UINT16 m_yscroll;
	tilemap_t *m_background;
	tilemap_t *m_foreground;
	UINT16 *m_spriteram;
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
};


/*----------- defined in video/terracre.c -----------*/

PALETTE_INIT( amazon );
VIDEO_START( amazon );
SCREEN_UPDATE_IND16( amazon );
