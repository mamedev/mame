
class mcatadv_state : public driver_device
{
public:
	mcatadv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *     m_videoram1;
	UINT16 *     m_videoram2;
	UINT16 *     m_scroll1;
	UINT16 *     m_scroll2;
	UINT16 *     m_spriteram;
	UINT16 *     m_spriteram_old;
	UINT16 *     m_vidregs;
	UINT16 *     m_vidregs_old;
//  UINT16 *     m_paletteram;    // this currently uses generic palette handlers
	size_t       m_spriteram_size;

	/* video-related */
	tilemap_t    *m_tilemap1;
	tilemap_t    *m_tilemap2;
	int m_palette_bank1;
	int m_palette_bank2;

	/* devices */
	device_t *m_maincpu;
	device_t *m_soundcpu;
};

/*----------- defined in video/mcatadv.c -----------*/

SCREEN_UPDATE( mcatadv );
VIDEO_START( mcatadv );
SCREEN_EOF( mcatadv );

WRITE16_HANDLER( mcatadv_videoram1_w );
WRITE16_HANDLER( mcatadv_videoram2_w );
