
typedef struct _gcpinbal_state gcpinbal_state;
struct _gcpinbal_state
{
	/* memory pointers */
	UINT16 *    tilemapram;
	UINT16 *    ioc_ram;
	UINT16 *    spriteram;
//  UINT16 *    paletteram; // currently this uses generic palette handling
	size_t      spriteram_size;

	/* video-related */
	tilemap_t     *tilemap[3];
	UINT16      scrollx[3], scrolly[3];
	UINT16      bg0_gfxset, bg1_gfxset;
#ifdef MAME_DEBUG
	UINT8       dislayer[4];
#endif

	/* sound-related */
	UINT32      msm_start, msm_end, msm_bank;
	UINT32      adpcm_start, adpcm_end, adpcm_idle;
	UINT8       adpcm_trigger, adpcm_data;

	/* devices */
	running_device *maincpu;
	running_device *oki;
	running_device *msm;
};


/*----------- defined in video/gcpinbal.c -----------*/

VIDEO_START( gcpinbal );
VIDEO_UPDATE( gcpinbal );

READ16_HANDLER ( gcpinbal_tilemaps_word_r );
WRITE16_HANDLER( gcpinbal_tilemaps_word_w );
