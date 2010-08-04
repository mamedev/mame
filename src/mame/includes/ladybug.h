/*************************************************************************

    Universal 8106-A2 + 8106-B PCB set

    and Zero Hour / Red Clash

*************************************************************************/

class ladybug_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, ladybug_state(machine)); }

	ladybug_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    colorram;
	UINT8 *    spriteram;
	UINT8 *    grid_data;
	size_t     spriteram_size;

	/* video-related */
	tilemap_t    *bg_tilemap, *grid_tilemap;	// ladybug
	tilemap_t    *fg_tilemap;	// redclash
	UINT8      grid_color;
	int        star_speed;
	int        gfxbank;	// redclash only
	UINT8      stars_enable;
	UINT8      stars_speed;
	UINT32     stars_state;
	UINT16     stars_offset;
	UINT8      stars_count;

	/* misc */
	UINT8      sound_low;
	UINT8      sound_high;
	UINT8      weird_value[8];
	UINT8      sraider_0x30, sraider_0x38;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/ladybug.c -----------*/

WRITE8_HANDLER( ladybug_videoram_w );
WRITE8_HANDLER( ladybug_colorram_w );
WRITE8_HANDLER( ladybug_flipscreen_w );
WRITE8_HANDLER( sraider_io_w );

PALETTE_INIT( ladybug );
VIDEO_START( ladybug );
VIDEO_UPDATE( ladybug );

PALETTE_INIT( sraider );
VIDEO_START( sraider );
VIDEO_UPDATE( sraider );
VIDEO_EOF( sraider );

/*----------- defined in video/redclash.c -----------*/

WRITE8_HANDLER( redclash_videoram_w );
WRITE8_HANDLER( redclash_gfxbank_w );
WRITE8_HANDLER( redclash_flipscreen_w );

WRITE8_HANDLER( redclash_star0_w );
WRITE8_HANDLER( redclash_star1_w );
WRITE8_HANDLER( redclash_star2_w );
WRITE8_HANDLER( redclash_star_reset_w );

PALETTE_INIT( redclash );
VIDEO_START( redclash );
VIDEO_UPDATE( redclash );
VIDEO_EOF( redclash );

/* sraider uses the zerohour star generator board */
void redclash_set_stars_enable(running_machine *machine, UINT8 on);
void redclash_update_stars_state(running_machine *machine);
void redclash_set_stars_speed(running_machine *machine, UINT8 speed);
void redclash_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, UINT8 palette_offset, UINT8 sraider, UINT8 firstx, UINT8 lastx);
