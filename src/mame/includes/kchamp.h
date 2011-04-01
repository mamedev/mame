/*************************************************************************

    Karate Champ

*************************************************************************/

class kchamp_state : public driver_device
{
public:
	kchamp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t    *m_bg_tilemap;

	/* misc */
	int        m_nmi_enable;
	int        m_sound_nmi_enable;
	int        m_msm_data;
	int        m_msm_play_lo_nibble;
	int        m_counter;

	/* devices */
	device_t *m_audiocpu;
};


/*----------- defined in video/kchamp.c -----------*/

WRITE8_HANDLER( kchamp_videoram_w );
WRITE8_HANDLER( kchamp_colorram_w );
WRITE8_HANDLER( kchamp_flipscreen_w );

PALETTE_INIT( kchamp );
VIDEO_START( kchamp );
SCREEN_UPDATE( kchamp );
SCREEN_UPDATE( kchampvs );
