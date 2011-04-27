class pokechmp_state : public driver_device
{
public:
	pokechmp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	tilemap_t *m_bg_tilemap;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/pokechmp.c -----------*/

WRITE8_HANDLER( pokechmp_videoram_w );
WRITE8_HANDLER( pokechmp_flipscreen_w );

VIDEO_START( pokechmp );
SCREEN_UPDATE( pokechmp );
