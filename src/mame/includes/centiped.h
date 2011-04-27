/*************************************************************************

    Atari Centipede hardware

*************************************************************************/

class centiped_state : public driver_device
{
public:
	centiped_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_oldpos[4];
	UINT8 m_sign[4];
	UINT8 m_dsw_select;
	UINT8 m_control_select;
	UINT8 *m_rambase;
	UINT8 m_flipscreen;
	UINT8 *m_bullsdrt_tiles_bankram;
	tilemap_t *m_bg_tilemap;
	UINT8 m_bullsdrt_sprites_bank;
	UINT8 m_penmask[64];
	UINT8 *m_spriteram;
};


/*----------- defined in video/centiped.c -----------*/

PALETTE_INIT( warlords );

VIDEO_START( centiped );
VIDEO_START( milliped );
VIDEO_START( warlords );
VIDEO_START( bullsdrt );

SCREEN_UPDATE( centiped );
SCREEN_UPDATE( milliped );
SCREEN_UPDATE( warlords );
SCREEN_UPDATE( bullsdrt );

WRITE8_HANDLER( centiped_paletteram_w );
WRITE8_HANDLER( milliped_paletteram_w );

WRITE8_HANDLER( centiped_videoram_w );
WRITE8_HANDLER( centiped_flip_screen_w );
WRITE8_HANDLER( bullsdrt_tilesbank_w );
WRITE8_HANDLER( bullsdrt_sprites_bank_w );

WRITE8_HANDLER( mazeinv_paletteram_w );
