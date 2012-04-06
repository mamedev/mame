class clshroad_state : public driver_device
{
public:
	clshroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vram_0;
	UINT8 *m_vram_1;
	UINT8 *m_vregs;
	tilemap_t *m_tilemap_0a;
	tilemap_t *m_tilemap_0b;
	tilemap_t *m_tilemap_1;
	UINT8 *m_spriteram;
	size_t m_spriteram_size;
	DECLARE_READ8_MEMBER(clshroad_input_r);
	DECLARE_WRITE8_MEMBER(clshroad_flipscreen_w);
	DECLARE_WRITE8_MEMBER(clshroad_vram_0_w);
	DECLARE_WRITE8_MEMBER(clshroad_vram_1_w);
};


/*----------- defined in video/clshroad.c -----------*/


PALETTE_INIT( firebatl );
PALETTE_INIT( clshroad );
VIDEO_START( firebatl );
VIDEO_START( clshroad );
SCREEN_UPDATE_IND16( clshroad );
