/***************************************************************************

    Popper

***************************************************************************/

class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 * m_videoram;
	UINT8 * m_ol_videoram;
	UINT8 * m_attribram;
	UINT8 * m_ol_attribram;
	UINT8 * m_spriteram;
	size_t  m_spriteram_size;

	/* video-related */
	tilemap_t *m_p123_tilemap;
	tilemap_t *m_p0_tilemap;
	tilemap_t *m_ol_p123_tilemap;
	tilemap_t *m_ol_p0_tilemap;
	INT32 m_flipscreen;
	INT32 m_e002;
	INT32 m_gfx_bank;
	rectangle m_tilemap_clip;

	/* devices */
	device_t *m_audiocpu;

	UINT8 m_nmi_mask;
};


/*----------- defined in video/popper.c -----------*/

WRITE8_HANDLER( popper_videoram_w );
WRITE8_HANDLER( popper_attribram_w );
WRITE8_HANDLER( popper_ol_videoram_w );
WRITE8_HANDLER( popper_ol_attribram_w );
WRITE8_HANDLER( popper_flipscreen_w );
WRITE8_HANDLER( popper_e002_w );
WRITE8_HANDLER( popper_gfx_bank_w );

PALETTE_INIT( popper );
VIDEO_START( popper );
SCREEN_UPDATE( popper );

