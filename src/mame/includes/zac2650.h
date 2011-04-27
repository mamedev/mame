class zac2650_state : public driver_device
{
public:
	zac2650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_s2636_0_ram;
	bitmap_t *m_spritebitmap;
	int m_CollisionBackground;
	int m_CollisionSprite;
	tilemap_t *m_bg_tilemap;
};


/*----------- defined in video/zac2650.c -----------*/

WRITE8_HANDLER( tinvader_videoram_w );
READ8_HANDLER( zac_s2636_r );
WRITE8_HANDLER( zac_s2636_w );
READ8_HANDLER( tinvader_port_0_r );

VIDEO_START( tinvader );
SCREEN_UPDATE( tinvader );

