/*************************************************************************

    Macross Plus

*************************************************************************/

class macrossp_state : public driver_device
{
public:
	macrossp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT32 *         mainram;
	UINT32 *         scra_videoram;
	UINT32 *         scra_videoregs;
	UINT32 *         scrb_videoram;
	UINT32 *         scrb_videoregs;
	UINT32 *         scrc_videoram;
	UINT32 *         scrc_videoregs;
	UINT32 *         text_videoram;
	UINT32 *         text_videoregs;
	UINT32 *         spriteram;
	UINT32 *         spriteram_old;
	UINT32 *         spriteram_old2;
	UINT32 *         paletteram;
	size_t           spriteram_size;

	/* video-related */
	tilemap_t  *scra_tilemap, *scrb_tilemap, *scrc_tilemap, *text_tilemap;

	/* misc */
	int              sndpending;
	int              snd_toggle;
	INT32            fade_effect, old_fade;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
};

/*----------- defined in video/macrossp.c -----------*/

WRITE32_HANDLER( macrossp_scra_videoram_w );
WRITE32_HANDLER( macrossp_scrb_videoram_w );
WRITE32_HANDLER( macrossp_scrc_videoram_w );
WRITE32_HANDLER( macrossp_text_videoram_w );

VIDEO_START(macrossp);
VIDEO_UPDATE(macrossp);
VIDEO_EOF(macrossp);
