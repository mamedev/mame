/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/

class rungun_state : public driver_device
{
public:
	rungun_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

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
	device_t *maincpu;
	device_t *audiocpu;
	device_t *k054539_1;
	device_t *k054539_2;
	device_t *k053936;
	device_t *k055673;
	device_t *k053252;
};




/*----------- defined in video/rungun.c -----------*/

extern void rng_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);

READ16_HANDLER( rng_ttl_ram_r );
WRITE16_HANDLER( rng_ttl_ram_w );
WRITE16_HANDLER( rng_936_videoram_w );

VIDEO_START( rng );
VIDEO_UPDATE( rng );
