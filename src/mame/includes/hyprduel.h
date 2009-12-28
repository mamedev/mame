#define RASTER_LINES 262
#define FIRST_VISIBLE_LINE 0
#define LAST_VISIBLE_LINE 223

typedef struct _hyprduel_state hyprduel_state;
struct _hyprduel_state
{
	/* memory pointers */
	UINT16 *  videoregs;
	UINT16 *  screenctrl;
	UINT16 *  tiletable_old;
	UINT16 *  tiletable;
	UINT16 *  vram_0;
	UINT16 *  vram_1;
	UINT16 *  vram_2;
	UINT16 *  window;
	UINT16 *  scroll;
	UINT16 *  rombank;
	UINT16 *  blitter_regs;
	UINT16 *  irq_enable;
	UINT16 *  sharedram1;
	UINT16 *  sharedram3;
	UINT16 *  spriteram;
	UINT16 *  paletteram;
	size_t    tiletable_size;
	size_t    spriteram_size;

	/* video-related */
	tilemap_t   *bg_tilemap[3];
	UINT8     *empty_tiles;
	UINT8     *dirtyindex;
	int       sprite_xoffs, sprite_yoffs, sprite_yoffs_sub;

	/* misc */
	emu_timer *magerror_irq_timer;
	int       blitter_bit;
	int       requested_int;
	int       subcpu_resetline;
	int       cpu_trigger;
	int       int_num;

	/* devices */
	const device_config *maincpu;
	const device_config *subcpu;
};



/*----------- defined in video/hyprduel.c -----------*/


WRITE16_HANDLER( hyprduel_paletteram_w );
WRITE16_HANDLER( hyprduel_window_w );
WRITE16_HANDLER( hyprduel_vram_0_w );
WRITE16_HANDLER( hyprduel_vram_1_w );
WRITE16_HANDLER( hyprduel_vram_2_w );
WRITE16_HANDLER( hyprduel_scrollreg_w );
WRITE16_HANDLER( hyprduel_scrollreg_init_w );

VIDEO_START( hyprduel_14220 );
VIDEO_START( magerror_14220 );
VIDEO_UPDATE( hyprduel );
