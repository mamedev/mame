/*************************************************************************

    Dragonball Z

*************************************************************************/

typedef struct _dbz_state dbz_state;
struct _dbz_state
{
	/* memory pointers */
	UINT16 *      bg1_videoram;
	UINT16 *      bg2_videoram;
//  UINT16 *      paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *bg1_tilemap, *bg2_tilemap;
	int          layer_colorbase[6], layerpri[5], sprite_colorbase;

	/* misc */
	int           control;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k053246;
	const device_config *k053251;
	const device_config *k056832;
	const device_config *k053936_1;
	const device_config *k053936_2;
};


/*----------- defined in video/dbz.c -----------*/

extern void dbz_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
extern void dbz_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);

WRITE16_HANDLER(dbz_bg1_videoram_w);
WRITE16_HANDLER(dbz_bg2_videoram_w);

VIDEO_START(dbz);
VIDEO_UPDATE(dbz);
