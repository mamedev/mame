/*************************************************************************

    Karate Champ

*************************************************************************/

class kchamp_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, kchamp_state(machine)); }

	kchamp_state(running_machine &machine)
		: driver_data_t(machine) { }

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
	running_device *audiocpu;
};


/*----------- defined in video/kchamp.c -----------*/

WRITE8_HANDLER( kchamp_videoram_w );
WRITE8_HANDLER( kchamp_colorram_w );
WRITE8_HANDLER( kchamp_flipscreen_w );

PALETTE_INIT( kchamp );
VIDEO_START( kchamp );
VIDEO_UPDATE( kchamp );
VIDEO_UPDATE( kchampvs );
