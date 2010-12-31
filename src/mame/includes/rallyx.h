struct jungler_star
{
	int x, y, color;
};

#define JUNGLER_MAX_STARS 1000

class rallyx_state : public driver_device
{
public:
	rallyx_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT8 *  videoram;
	UINT8 *  spriteram;
	UINT8 *  spriteram2;
	UINT8 *  radarattr;
	UINT8 *  radarx;
	UINT8 *  radary;

	/* video-related */
	tilemap_t  *bg_tilemap;
	tilemap_t  *fg_tilemap;

	/* misc */
	int      last_bang;
	int      spriteram_base, stars_enable, total_stars;
	UINT8    drawmode_table[4];
	struct jungler_star stars[JUNGLER_MAX_STARS];

	/* devices */
	cpu_device *maincpu;
	device_t *samples;
};


/*----------- defined in video/rallyx.c -----------*/

WRITE8_HANDLER( rallyx_videoram_w );
WRITE8_HANDLER( rallyx_scrollx_w );
WRITE8_HANDLER( rallyx_scrolly_w );
WRITE8_HANDLER( tactcian_starson_w );

PALETTE_INIT( rallyx );
PALETTE_INIT( jungler );
VIDEO_START( rallyx );
VIDEO_START( jungler );
VIDEO_START( locomotn );
VIDEO_START( commsega );
VIDEO_UPDATE( rallyx );
VIDEO_UPDATE( jungler );
VIDEO_UPDATE( locomotn );
