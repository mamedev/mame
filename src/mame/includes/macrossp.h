/*************************************************************************

    Macross Plus

*************************************************************************/

class macrossp_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, macrossp_state(machine)); }

	macrossp_state(running_machine &machine)
		: driver_data_t(machine) { }

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
