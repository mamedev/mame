/*************************************************************************

    Karate Champ

*************************************************************************/

class kchamp_state : public driver_device
{
public:
	kchamp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap;

	/* misc */
	int        nmi_enable;
	int        sound_nmi_enable;
	int        msm_data;
	int        msm_play_lo_nibble;
	int        counter;

	/* devices */
	device_t *audiocpu;
};


/*----------- defined in video/kchamp.c -----------*/

WRITE8_HANDLER( kchamp_videoram_w );
WRITE8_HANDLER( kchamp_colorram_w );
WRITE8_HANDLER( kchamp_flipscreen_w );

PALETTE_INIT( kchamp );
VIDEO_START( kchamp );
VIDEO_UPDATE( kchamp );
VIDEO_UPDATE( kchampvs );
