class darkmist_state : public driver_device
{
public:
	darkmist_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_workram;
	int m_hw;
	UINT8 *m_scroll;
	UINT8 *m_spritebank;
	tilemap_t *m_bgtilemap;
	tilemap_t *m_fgtilemap;
	tilemap_t *m_txtilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_WRITE8_MEMBER(darkmist_hw_w);
};


/*----------- defined in video/darkmist.c -----------*/

VIDEO_START( darkmist );
SCREEN_UPDATE_IND16( darkmist );
PALETTE_INIT( darkmist );

