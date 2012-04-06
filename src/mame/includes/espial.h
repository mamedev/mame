/***************************************************************************

 Espial hardware games (drivers: espial.c)

***************************************************************************/

class espial_state : public driver_device
{
public:
	espial_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *   m_videoram;
	UINT8 *   m_colorram;
	UINT8 *   m_attributeram;
	UINT8 *   m_scrollram;
	UINT8 *   m_spriteram_1;
	UINT8 *   m_spriteram_2;
	UINT8 *   m_spriteram_3;

	/* video-related */
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;
	int       m_flipscreen;

	/* sound-related */
	UINT8     m_main_nmi_enabled;
	UINT8     m_sound_nmi_enabled;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(espial_master_interrupt_mask_w);
	DECLARE_WRITE8_MEMBER(espial_master_soundlatch_w);
	DECLARE_WRITE8_MEMBER(espial_sound_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(espial_videoram_w);
	DECLARE_WRITE8_MEMBER(espial_colorram_w);
	DECLARE_WRITE8_MEMBER(espial_attributeram_w);
	DECLARE_WRITE8_MEMBER(espial_scrollram_w);
	DECLARE_WRITE8_MEMBER(espial_flipscreen_w);
};

/*----------- defined in video/espial.c -----------*/

PALETTE_INIT( espial );
VIDEO_START( espial );
VIDEO_START( netwars );
SCREEN_UPDATE_IND16( espial );
