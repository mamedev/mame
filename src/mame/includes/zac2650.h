class zac2650_state : public driver_device
{
public:
	zac2650_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 *m_s2636_0_ram;
	bitmap_ind16 m_bitmap;
	bitmap_ind16 m_spritebitmap;
	int m_CollisionBackground;
	int m_CollisionSprite;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(tinvader_sound_w);
	DECLARE_WRITE8_MEMBER(tinvader_videoram_w);
	DECLARE_READ8_MEMBER(zac_s2636_r);
	DECLARE_WRITE8_MEMBER(zac_s2636_w);
	DECLARE_READ8_MEMBER(tinvader_port_0_r);
};


/*----------- defined in video/zac2650.c -----------*/


VIDEO_START( tinvader );
SCREEN_UPDATE_IND16( tinvader );

