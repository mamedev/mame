/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/

typedef struct _rungun_state rungun_state;
struct _rungun_state
{
	/* memory pointers */
	UINT16 *    _936_videoram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *ttl_tilemap, *_936_tilemap;
	UINT16      ttl_vram[0x1000];
	int         ttl_gfx_index, sprite_colorbase;

	/* misc */
	UINT16      sysreg[0x20];
	int         z80_control;
	int         sound_status;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k054539_1;
	const device_config *k054539_2;
	const device_config *k053936;
	const device_config *k055673;
	const device_config *k053252;
};




/*----------- defined in video/rungun.c -----------*/

extern void rng_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);

READ16_HANDLER( rng_ttl_ram_r );
WRITE16_HANDLER( rng_ttl_ram_w );
WRITE16_HANDLER( rng_936_videoram_w );

VIDEO_START( rng );
VIDEO_UPDATE( rng );
