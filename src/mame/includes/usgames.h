class usgames_state : public driver_device
{
public:
	usgames_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_charram;
	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(usgames_rombank_w);
	DECLARE_WRITE8_MEMBER(lamps1_w);
	DECLARE_WRITE8_MEMBER(lamps2_w);
	DECLARE_WRITE8_MEMBER(usgames_videoram_w);
	DECLARE_WRITE8_MEMBER(usgames_charram_w);
};


/*----------- defined in video/usgames.c -----------*/

VIDEO_START( usgames );
PALETTE_INIT( usgames );
SCREEN_UPDATE_IND16( usgames );
