/***************************************************************************

  Galaxian hardware family

***************************************************************************/

#include "driver.h"
#include "includes/galaxold.h"

static const rectangle _spritevisiblearea =
{
	2*8+1, 32*8-1,
	2*8,   30*8-1
};
static const rectangle _spritevisibleareaflipx =
{
	0*8, 30*8-2,
	2*8, 30*8-1
};

static const rectangle* spritevisiblearea;
static const rectangle* spritevisibleareaflipx;


#define STARS_COLOR_BASE 		(memory_region_length(machine, "proms"))
#define BULLETS_COLOR_BASE		(STARS_COLOR_BASE + 64)
#define BACKGROUND_COLOR_BASE	(BULLETS_COLOR_BASE + 2)


UINT8 *galaxold_videoram;
UINT8 *galaxold_spriteram;
UINT8 *galaxold_spriteram2;
UINT8 *galaxold_attributesram;
UINT8 *galaxold_bulletsram;
UINT8 *rockclim_videoram;
size_t galaxold_spriteram_size;
size_t galaxold_spriteram2_size;
size_t galaxold_bulletsram_size;


static TILE_GET_INFO( get_tile_info );
static TILE_GET_INFO( rockclim_get_tile_info );
static tilemap *bg_tilemap;
static tilemap *rockclim_tilemap;
static int mooncrst_gfxextend;
static int spriteram2_present;
static UINT8 gfxbank[5];
static UINT8 flipscreen_x;
static UINT8 flipscreen_y;
static UINT8 color_mask;
static tilemap *dambustr_tilemap2;
static UINT8 *dambustr_videoram2;
static void (*modify_charcode)(running_machine *machine, UINT16 *code,UINT8 x);		/* function to call to do character banking */
static void mooncrst_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x);
static void   pisces_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x);
static void mimonkey_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x);
static void  mariner_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x);
static void dambustr_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x);

static void (*modify_spritecode)(UINT8 *spriteram,int*,int*,int*,int);	/* function to call to do sprite banking */
static void mshuttle_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);
static void mimonkey_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);
static void  batman2_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);
static void dkongjrm_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);
static void   ad2083_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);
static void dambustr_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs);

static void (*modify_color)(UINT8 *color);	/* function to call to do modify how the color codes map to the PROM */
static void drivfrcg_modify_color(UINT8 *color);

static void (*modify_ypos)(UINT8*);	/* function to call to do modify how vertical positioning bits are connected */

static TIMER_CALLBACK( stars_blink_callback );
static TIMER_CALLBACK( stars_scroll_callback );

static void (*tilemap_set_scroll)( tilemap *, int col, int value );

/* star circuit */
#define STAR_COUNT  252
struct star
{
	int x,y,color;
};
static struct star stars[STAR_COUNT];
static int stars_colors_start;
       UINT8 galaxold_stars_on;
static INT32 stars_scrollpos;
static UINT8 stars_blink_state;
static emu_timer *stars_blink_timer;
static emu_timer *stars_scroll_timer;
static UINT8 timer_adjusted;
       void galaxold_init_stars(running_machine *machine, int colors_offset);
static void (*draw_stars)(running_machine *machine, bitmap_t *, const rectangle *);		/* function to call to draw the star layer */
static void     noop_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
       void galaxold_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void scramble_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void   rescue_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void  mariner_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void start_stars_blink_timer(double ra, double rb, double c);
static void start_stars_scroll_timer(running_machine *machine);

/* bullets circuit */
static UINT8 darkplnt_bullet_color;
static void (*draw_bullets)(running_machine *,bitmap_t *,const rectangle *,int,int,int);	/* function to call to draw a bullet */
static void galaxold_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
static void scramble_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
static void darkplnt_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);
static void dambustr_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y);

/* background circuit */
static UINT8 background_enable;
static UINT8 background_red, background_green, background_blue;
static void (*draw_background)(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);	/* function to call to draw the background */
static void galaxold_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void scramble_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void  turtles_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void  mariner_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void stratgyx_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void  minefld_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void   rescue_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);
static void dambustr_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect);

static UINT16 rockclim_v;
static UINT16 rockclim_h;
static int dambustr_bg_split_line;
static int dambustr_bg_color_1;
static int dambustr_bg_color_2;
static int dambustr_bg_priority;
static int dambustr_char_bank;
static bitmap_t *dambustr_tmpbitmap;



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Galaxian has one 32 bytes palette PROM, connected to the RGB output this way:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The output of the background star generator is connected this way:

  bit 5 -- 100 ohm resistor  -- BLUE
        -- 150 ohm resistor  -- BLUE
        -- 100 ohm resistor  -- GREEN
        -- 150 ohm resistor  -- GREEN
        -- 100 ohm resistor  -- RED
  bit 0 -- 150 ohm resistor  -- RED

  The blue background in Scramble and other games goes through a 390 ohm
  resistor.

  The bullet RGB outputs go through 100 ohm resistors.

  The RGB outputs have a 470 ohm pull-down each.

***************************************************************************/
PALETTE_INIT( galaxold )
{
	int i, len;


	/* first, the character/sprite palette */
	len = memory_region_length(machine, "proms");
	for (i = 0;i < len;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}


	galaxold_init_stars(machine, STARS_COLOR_BASE);


	/* bullets - yellow and white */
	palette_set_color(machine,BULLETS_COLOR_BASE+0,MAKE_RGB(0xef,0xef,0x00));
	palette_set_color(machine,BULLETS_COLOR_BASE+1,MAKE_RGB(0xef,0xef,0xef));
}

PALETTE_INIT( scrambold )
{
	PALETTE_INIT_CALL(galaxold);


	/* blue background - 390 ohm resistor */
	palette_set_color(machine,BACKGROUND_COLOR_BASE,MAKE_RGB(0,0,0x56));
}


PALETTE_INIT( stratgyx )
{
	int base = BACKGROUND_COLOR_BASE;
	int i;


	PALETTE_INIT_CALL(galaxold);


	/*  The background color generator is connected this way:

        RED   - 270 ohm resistor
        GREEN - 560 ohm resistor
        BLUE  - 470 ohm resistor */

	for (i = 0; i < 8; i++)
	{
		int r = BIT(i,0) * 0x7c;
		int g = BIT(i,1) * 0x3c;
		int b = BIT(i,2) * 0x47;

		palette_set_color_rgb(machine,base+i,r,g,b);
	}
}

PALETTE_INIT( rockclim )
{
	int i, len;


	/* first, the character/sprite palette */
	len = memory_region_length(machine, "proms");
	for (i = 0;i < len;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		b = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}
}
/***************************************************************************

  Convert the color PROMs into a more useable format.

  Dark Planet has one 32 bytes palette PROM, connected to the RGB output this way:

  bit 5 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

  The bullet RGB outputs go through 100 ohm resistors.

  The RGB outputs have a 470 ohm pull-down each.

***************************************************************************/
PALETTE_INIT( darkplnt )
{
	int i;


	/* first, the character/sprite palette */

	for (i = 0;i < 32;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		g = 0x00;
		/* blue component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}


	/* bullets - red and blue */
	palette_set_color(machine,BULLETS_COLOR_BASE+0,MAKE_RGB(0xef,0x00,0x00));
	palette_set_color(machine,BULLETS_COLOR_BASE+1,MAKE_RGB(0x00,0x00,0xef));
}

PALETTE_INIT( minefld )
{
	int base = BACKGROUND_COLOR_BASE;
	int i;


	PALETTE_INIT_CALL(galaxold);


	/* set up background colors */

	/* graduated blue */

	for (i = 0; i < 128; i++)
	{
		int r = 0;
		int g = i;
		int b = i * 2;
		palette_set_color_rgb(machine,base+i,r,g,b);
	}

	/* graduated brown */

	for (i = 0; i < 128; i++)
	{
		int r = i * 1.5;
		int g = i * 0.75;
		int b = i / 2;
		palette_set_color_rgb(machine,base+128+i,r,g,b);
	}
}

PALETTE_INIT( rescue )
{
	int base = BACKGROUND_COLOR_BASE;
	int i;


	PALETTE_INIT_CALL(galaxold);


	/* set up background colors */

	/* graduated blue */

	for (i = 0; i < 128; i++)
	{
		int r = 0;
		int g = i;
		int b = i * 2;
		palette_set_color_rgb(machine,base+i,r,g,b);
	}
}

PALETTE_INIT( mariner )
{
	int base = BACKGROUND_COLOR_BASE;
	int i;


	PALETTE_INIT_CALL(galaxold);


	/* set up background colors */

	/* 16 shades of blue - the 4 bits are connected to the following resistors:

        bit 0 -- 4.7 kohm resistor
              -- 2.2 kohm resistor
              -- 1   kohm resistor
        bit 0 -- .47 kohm resistor */

	for (i = 0; i < 16; i++)
	{
		int r,g,b;

		r = 0;
		g = 0;
		b = 0x0e * BIT(i,0) + 0x1f * BIT(i,1) + 0x43 * BIT(i,2) + 0x8f * BIT(i,3);

		palette_set_color_rgb(machine,base+i,r,g,b);
	}
}

/* swapped r/g/b hook-up */
PALETTE_INIT( dambustr )
{
	int base = BACKGROUND_COLOR_BASE;
	int i, len;

	/* first, the character/sprite palette */
	len = memory_region_length(machine, "proms");

	for (i = 0;i < len;i++)
	{
		int bit0,bit1,bit2,r,g,b;

		/* red component */
		bit0 = BIT(*color_prom,0);
		bit1 = BIT(*color_prom,1);
		bit2 = BIT(*color_prom,2);
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* green component */
		bit0 = BIT(*color_prom,3);
		bit1 = BIT(*color_prom,4);
		bit2 = BIT(*color_prom,5);
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		/* blue component */
		bit0 = BIT(*color_prom,6);
		bit1 = BIT(*color_prom,7);
		g = 0x4f * bit0 + 0xa8 * bit1;

		palette_set_color_rgb(machine,i,r,g,b);
		color_prom++;
	}


	galaxold_init_stars(machine, STARS_COLOR_BASE);


	/* bullets - yellow and white */
	palette_set_color(machine,BULLETS_COLOR_BASE+0,MAKE_RGB(0xef,0xef,0x00));
	palette_set_color(machine,BULLETS_COLOR_BASE+1,MAKE_RGB(0xef,0xef,0xef));

	/*
    Assumption (not clear from the schematics):
    The background color generator is connected this way:

        RED   - 470 ohm resistor
        GREEN - 470 ohm resistor
        BLUE  - 470 ohm resistor */


	for (i = 0; i < 8; i++)
	{
		int r = BIT(i,0) * 0x47;
		int g = BIT(i,1) * 0x47;
		int b = BIT(i,2) * 0x4f;
		palette_set_color_rgb(machine,base+i,r,g,b);
	}
}


PALETTE_INIT( turtles )
{
	int base = BACKGROUND_COLOR_BASE;
	int i;


	PALETTE_INIT_CALL(galaxold);


	/*  The background color generator is connected this way:

        RED   - 390 ohm resistor
        GREEN - 470 ohm resistor
        BLUE  - 390 ohm resistor */

	for (i = 0; i < 8; i++)
	{
		int r = BIT(i,0) * 0x55;
		int g = BIT(i,1) * 0x47;
		int b = BIT(i,2) * 0x55;

		palette_set_color_rgb(machine,base+i,r,g,b);
	}
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static void state_save_register(running_machine *machine)
{
	state_save_register_global_array(machine, gfxbank);
	state_save_register_global(machine, flipscreen_x);
	state_save_register_global(machine, flipscreen_y);

	state_save_register_global(machine, galaxold_stars_on);
	state_save_register_global(machine, stars_scrollpos);
	state_save_register_global(machine, stars_blink_state);

	state_save_register_global(machine, darkplnt_bullet_color);

	state_save_register_global(machine, background_enable);
	state_save_register_global(machine, background_red);
	state_save_register_global(machine, background_green);
	state_save_register_global(machine, background_blue);
}

static void video_start_common(running_machine *machine, tilemap_mapper_func get_memory_offset)
{
	bg_tilemap = tilemap_create(machine, get_tile_info,get_memory_offset,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);


	modify_charcode = 0;
	modify_spritecode = 0;
	modify_color = 0;
	modify_ypos = 0;

	mooncrst_gfxextend = 0;

	draw_bullets = 0;

	draw_background = galaxold_draw_background;
	background_enable = 0;
	background_blue = 0;
	background_red = 0;
	background_green = 0;

	draw_stars = noop_draw_stars;

	flipscreen_x = 0;
	flipscreen_y = 0;

	spriteram2_present = 0;

	spritevisiblearea      = &_spritevisiblearea;
	spritevisibleareaflipx = &_spritevisibleareaflipx;

	color_mask = (machine->gfx[0]->color_granularity == 4) ? 7 : 3;

	state_save_register(machine);
}

VIDEO_START( galaxold_plain )
{
	video_start_common(machine,tilemap_scan_rows);

	tilemap_set_scroll_cols(bg_tilemap, 32);
	tilemap_set_scroll = tilemap_set_scrolly;
}

VIDEO_START( galaxold )
{
	VIDEO_START_CALL(galaxold_plain);

	draw_stars = galaxold_draw_stars;

	draw_bullets = galaxold_draw_bullets;
}

VIDEO_START( scrambold )
{
	VIDEO_START_CALL(galaxold_plain);

	/* FIXME: This most probably needs to be adjusted
     * again when RAW video params are added to scramble
     */
	tilemap_set_scrolldx(bg_tilemap, 0, 0);

	draw_stars = scramble_draw_stars;

	draw_bullets = scramble_draw_bullets;

	draw_background = scramble_draw_background;
}

VIDEO_START( darkplnt )
{
	VIDEO_START_CALL(galaxold_plain);

	tilemap_set_scrolldx(bg_tilemap, 0, 0);
	draw_bullets = darkplnt_draw_bullets;
}

VIDEO_START( rescue )
{
	VIDEO_START_CALL(scrambold);

	draw_stars = rescue_draw_stars;

	draw_background = rescue_draw_background;
}

VIDEO_START( minefld )
{
	VIDEO_START_CALL(scrambold);

	draw_stars = rescue_draw_stars;

	draw_background = minefld_draw_background;
}

VIDEO_START( stratgyx )
{
	VIDEO_START_CALL(galaxold_plain);

	draw_background = stratgyx_draw_background;
}

VIDEO_START( ckongs )
{
	VIDEO_START_CALL(scrambold);

	modify_spritecode = mshuttle_modify_spritecode;
}

VIDEO_START( mariner )
{
	VIDEO_START_CALL(galaxold_plain);

	draw_stars = mariner_draw_stars;

	draw_bullets = scramble_draw_bullets;

	draw_background = mariner_draw_background;

	modify_charcode = mariner_modify_charcode;
}

VIDEO_START( mimonkey )
{
	VIDEO_START_CALL(scrambold);

	modify_charcode   = mimonkey_modify_charcode;
	modify_spritecode = mimonkey_modify_spritecode;
}

VIDEO_START( dkongjrm )
{
	VIDEO_START_CALL(galaxold_plain);

	modify_charcode   = pisces_modify_charcode;
	modify_spritecode = dkongjrm_modify_spritecode;

	spriteram2_present= 1;
}

VIDEO_START( newsin7 )
{
	VIDEO_START_CALL(scrambold);

	spritevisiblearea      = &_spritevisibleareaflipx;
	spritevisibleareaflipx = &_spritevisiblearea;
}

VIDEO_START( scorpion )
{
	VIDEO_START_CALL(scrambold);

	modify_spritecode = batman2_modify_spritecode;
}

static void pisces_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	*code |= (gfxbank[0] << 6);
}

VIDEO_START( pisces )
{
	VIDEO_START_CALL(galaxold);

	modify_charcode   = pisces_modify_charcode;
	modify_spritecode = pisces_modify_spritecode;
}

static void theend_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	int i;


	/* same as Galaxian, but all bullets are yellow */
	for (i = 0; i < 4; i++)
	{
		x--;

		if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
			*BITMAP_ADDR16(bitmap, y, x) = BULLETS_COLOR_BASE;
	}
}

VIDEO_START( theend )
{
	VIDEO_START_CALL(galaxold);

	draw_bullets = theend_draw_bullets;
}

static void mooncrst_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	if (gfxbank[2] && ((*code & 0x30) == 0x20))
	{
		*code = (*code & 0x0f) | (gfxbank[0] << 4) | (gfxbank[1] << 5) | 0x40;
	}
}

VIDEO_START( mooncrst )
{
	VIDEO_START_CALL(galaxold);

	modify_charcode   = mooncrst_modify_charcode;
	modify_spritecode = mooncrst_modify_spritecode;
}

static void batman2_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	if (*code & 0x80)
	{
		*code |= (gfxbank[0] << 8);
	}
}

VIDEO_START( batman2 )
{
	VIDEO_START_CALL(galaxold);

	modify_charcode   = batman2_modify_charcode;
	modify_spritecode = batman2_modify_spritecode;

}



static void rockclim_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	tilemap_draw(bitmap,cliprect,rockclim_tilemap, 0,0);
}

static void rockclim_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	if (gfxbank[2])	*code|=0x40;
}

VIDEO_START( rockclim )
{
	VIDEO_START_CALL(galaxold);
	rockclim_tilemap = tilemap_create(machine, rockclim_get_tile_info,tilemap_scan_rows,8,8,64,32);
	draw_background = rockclim_draw_background;
	modify_charcode = mooncrst_modify_charcode;
	modify_spritecode = rockclim_modify_spritecode;
	rockclim_v = rockclim_h = 0;
	state_save_register_global(machine, rockclim_v);
	state_save_register_global(machine, rockclim_h);
}

static TILE_GET_INFO( drivfrcg_get_tile_info )
{
	int code = galaxold_videoram[tile_index];
	UINT8 x = tile_index & 0x1f;
	UINT8 color = galaxold_attributesram[(x << 1) | 1] & 7;
	UINT8 bank = galaxold_attributesram[(x << 1) | 1] & 0x30;

	code |= (bank << 4);
	color |= ((galaxold_attributesram[(x << 1) | 1] & 0x40) >> 3);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( drivfrcg )
{
	bg_tilemap = tilemap_create(machine, drivfrcg_get_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_scroll_cols(bg_tilemap, 32);
	tilemap_set_scroll = tilemap_set_scrolly;

	modify_charcode = 0;
	modify_spritecode = mshuttle_modify_spritecode;
	modify_color = drivfrcg_modify_color;
	modify_ypos = 0;

	mooncrst_gfxextend = 0;

	draw_bullets = 0;

	draw_background = galaxold_draw_background;
	background_enable = 0;
	background_blue = 0;
	background_red = 0;
	background_green = 0;

	draw_stars = noop_draw_stars;

	flipscreen_x = 0;
	flipscreen_y = 0;

	spriteram2_present = 0;

	spritevisiblearea      = &_spritevisiblearea;
	spritevisibleareaflipx = &_spritevisibleareaflipx;

	color_mask = 0xff;

	state_save_register(machine);
}

VIDEO_START( ad2083 )
{
	bg_tilemap = tilemap_create(machine, drivfrcg_get_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_scroll_cols(bg_tilemap, 32);
	tilemap_set_scroll = tilemap_set_scrolly;

	modify_charcode = 0;
	modify_spritecode = ad2083_modify_spritecode;
	modify_color = 0;
	modify_ypos = 0;

	mooncrst_gfxextend = 0;

	draw_bullets = scramble_draw_bullets;

	draw_background = turtles_draw_background;
	background_enable = 0;
	background_blue = 0;
	background_red = 0;
	background_green = 0;

	draw_stars = noop_draw_stars;

	flipscreen_x = 0;
	flipscreen_y = 0;

	spriteram2_present = 0;

	spritevisiblearea      = &_spritevisiblearea;
	spritevisibleareaflipx = &_spritevisibleareaflipx;

	color_mask = 7;

	state_save_register(machine);
}

UINT8 *racknrol_tiles_bank;

WRITE8_HANDLER( racknrol_tiles_bank_w )
{
	racknrol_tiles_bank[offset] = data;
	tilemap_mark_all_tiles_dirty(bg_tilemap);
}

static TILE_GET_INFO( racknrol_get_tile_info )
{
	int code = galaxold_videoram[tile_index];
	UINT8 x = tile_index & 0x1f;
	UINT8 color = galaxold_attributesram[(x << 1) | 1] & 7;
	UINT8 bank = racknrol_tiles_bank[x] & 7;

	code |= (bank << 8);

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( racknrol )
{
	bg_tilemap = tilemap_create(machine, racknrol_get_tile_info,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(bg_tilemap,0);
	tilemap_set_scroll_cols(bg_tilemap, 32);
	tilemap_set_scroll = tilemap_set_scrolly;

	modify_charcode = 0;
	modify_spritecode = 0;
	modify_color = 0;
	modify_ypos = 0;

	mooncrst_gfxextend = 0;

	draw_bullets = 0;

	draw_background = galaxold_draw_background;
	background_enable = 0;
	background_blue = 0;
	background_red = 0;
	background_green = 0;

	draw_stars = noop_draw_stars;

	flipscreen_x = 0;
	flipscreen_y = 0;

	spriteram2_present = 0;

	spritevisiblearea      = &_spritevisiblearea;
	spritevisibleareaflipx = &_spritevisibleareaflipx;

	color_mask = 0xff;

	state_save_register(machine);
}

VIDEO_START( bongo )
{
	VIDEO_START_CALL(galaxold_plain);

	modify_spritecode = batman2_modify_spritecode;
}

static TILE_GET_INFO( dambustr_get_tile_info2 )
{
	UINT8 x = tile_index & 0x1f;

	UINT16 code = dambustr_videoram2[tile_index];
	UINT8 color = galaxold_attributesram[(x << 1) | 1] & color_mask;

	if (modify_charcode)
	{
		modify_charcode(machine, &code, x);
	}

	if (modify_color)
	{
		modify_color(&color);
	}

	SET_TILE_INFO(0, code, color, 0);
}

VIDEO_START( dambustr )
{
	VIDEO_START_CALL(galaxold);

	dambustr_bg_split_line = 0;
	dambustr_bg_color_1 = 0;
	dambustr_bg_color_2 = 0;
	dambustr_bg_priority = 0;
	dambustr_char_bank = 0;

	draw_background = dambustr_draw_background;

	modify_charcode   = dambustr_modify_charcode;
	modify_spritecode = dambustr_modify_spritecode;

	draw_bullets = dambustr_draw_bullets;

	/* allocate the temporary bitmap for the background priority */
	dambustr_tmpbitmap = video_screen_auto_bitmap_alloc(machine->primary_screen);

	/* make a copy of the tilemap to emulate background priority */
	dambustr_videoram2 = auto_alloc_array(machine, UINT8, 0x0400);
	dambustr_tilemap2 = tilemap_create(machine, dambustr_get_tile_info2,tilemap_scan_rows,8,8,32,32);

	tilemap_set_transparent_pen(dambustr_tilemap2,0);
}


WRITE8_HANDLER( galaxold_videoram_w )
{
	galaxold_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap, offset);
}

READ8_HANDLER( galaxold_videoram_r )
{
	return galaxold_videoram[offset];
}


WRITE8_HANDLER( galaxold_attributesram_w )
{
	if (galaxold_attributesram[offset] != data)
	{
		if (offset & 0x01)
		{
			/* color change */
			int i;

			for (i = offset >> 1; i < 0x0400; i += 32)
				tilemap_mark_tile_dirty(bg_tilemap, i);
		}
		else
		{
			if (modify_ypos)
			{
				modify_ypos(&data);
			}

			tilemap_set_scroll(bg_tilemap, offset >> 1, data);
		}

		galaxold_attributesram[offset] = data;
	}
}


WRITE8_HANDLER( galaxold_flip_screen_x_w )
{
	if (flipscreen_x != (data & 0x01))
	{
		flipscreen_x = data & 0x01;

		tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}

WRITE8_HANDLER( galaxold_flip_screen_y_w )
{
	if (flipscreen_y != (data & 0x01))
	{
		flipscreen_y = data & 0x01;

		tilemap_set_flip(bg_tilemap, (flipscreen_x ? TILEMAP_FLIPX : 0) | (flipscreen_y ? TILEMAP_FLIPY : 0));
	}
}


WRITE8_HANDLER( gteikob2_flip_screen_x_w )
{
	galaxold_flip_screen_x_w(space, offset, ~data);
}

WRITE8_HANDLER( gteikob2_flip_screen_y_w )
{
	galaxold_flip_screen_y_w(space, offset, ~data);
}


WRITE8_HANDLER( hotshock_flip_screen_w )
{
	galaxold_flip_screen_x_w(space, offset, data);
	galaxold_flip_screen_y_w(space, offset, data);
}


WRITE8_HANDLER( scrambold_background_enable_w )
{
	background_enable = data & 0x01;
}

WRITE8_HANDLER( scrambold_background_red_w )
{
	background_red = data & 0x01;
}

WRITE8_HANDLER( scrambold_background_green_w )
{
	background_green = data & 0x01;
}

WRITE8_HANDLER( scrambold_background_blue_w )
{
	background_blue = data & 0x01;
}


WRITE8_HANDLER( galaxold_stars_enable_w )
{
	galaxold_stars_on = data & 0x01;

	if (!galaxold_stars_on)
	{
		stars_scrollpos = 0;
	}
}


WRITE8_HANDLER( darkplnt_bullet_color_w )
{
	darkplnt_bullet_color = data & 0x01;
}



WRITE8_HANDLER( galaxold_gfxbank_w )
{
	if (gfxbank[offset] != data)
	{
		gfxbank[offset] = data;

		tilemap_mark_all_tiles_dirty(bg_tilemap);
	}
}

WRITE8_HANDLER( rockclim_videoram_w )
{
	rockclim_videoram[offset] = data;
	tilemap_mark_tile_dirty(rockclim_tilemap, offset);
}

WRITE8_HANDLER( rockclim_scroll_w )
{
	switch(offset&3)
	{
		case 0: rockclim_h=(rockclim_h&0xff00)|data;tilemap_set_scrollx(rockclim_tilemap , 0, rockclim_h );break;
		case 1:	rockclim_h=(rockclim_h&0xff)|(data<<8);tilemap_set_scrollx(rockclim_tilemap , 0, rockclim_h );break;
		case 2:	rockclim_v=(rockclim_v&0xff00)|data;tilemap_set_scrolly(rockclim_tilemap , 0, rockclim_v );break;
		case 3:	rockclim_v=(rockclim_v&0xff)|(data<<8);tilemap_set_scrolly(rockclim_tilemap , 0, rockclim_v );break;
	}

}


READ8_HANDLER( rockclim_videoram_r )
{
	return rockclim_videoram[offset];
}


WRITE8_HANDLER( dambustr_bg_split_line_w )
{
	dambustr_bg_split_line = data;
}


WRITE8_HANDLER( dambustr_bg_color_w )
{
	dambustr_bg_color_1 = (BIT(data,2)<<2) | (BIT(data,1)<<1) | BIT(data,0);
	dambustr_bg_color_2 = (BIT(data,6)<<2) | (BIT(data,5)<<1) | BIT(data,4);
	dambustr_bg_priority = BIT(data,3);
	dambustr_char_bank = BIT(data,7);
	tilemap_mark_all_tiles_dirty(bg_tilemap);
}



/* character banking functions */

static void mooncrst_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	if (gfxbank[2] && ((*code & 0xc0) == 0x80))
	{
		*code = (*code & 0x3f) | (gfxbank[0] << 6) | (gfxbank[1] << 7) | 0x0100;
	}
}

static void pisces_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	*code |= (gfxbank[0] << 8);
}

static void mimonkey_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	*code |= (gfxbank[0] << 8) | (gfxbank[2] << 9);
}

static void mariner_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	UINT8 *prom;


	/* bit 0 of the PROM controls character banking */

	prom = memory_region(machine, "user2");

	*code |= ((prom[x] & 0x01) << 8);
}

static void dambustr_modify_charcode(running_machine *machine, UINT16 *code,UINT8 x)
{
	if (dambustr_char_bank == 0) {	// text mode
		*code |= 0x0300;
	}
	else {				// graphics mode
		if (x == 28)		// only line #28 stays in text mode
			*code |= 0x0300;
		else
			*code &= 0x00ff;
	};
}



/* sprite banking functions */

static void mshuttle_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	*code |= ((spriteram[offs + 2] & 0x30) << 2);
}


static void mimonkey_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	*code |= (gfxbank[0] << 6) | (gfxbank[2] << 7);
}

static void batman2_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	/* only the upper 64 sprites are used */
	*code |= 0x40;
}

static void dkongjrm_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	/* No x flip */
	*code = (spriteram[offs + 1] & 0x7f) | 0x80;
	*flipx = 0;
}

static void ad2083_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	/* No x flip */
	*code = (spriteram[offs + 1] & 0x7f) | ((spriteram[offs + 2] & 0x30) << 2);
	*flipx = 0;
}

static void dambustr_modify_spritecode(UINT8 *spriteram,int *code,int *flipx,int *flipy,int offs)
{
	*code += 0x40;
}


/* color PROM mapping functions */

static void drivfrcg_modify_color(UINT8 *color)
{
	*color = ((*color & 0x40) >> 3) | (*color & 7);
}

/* y position mapping functions */


/* bullet drawing functions */

static void galaxold_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	int i;


	for (i = 0; i < 4; i++)
	{
		x--;

		if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
		{
			int color;


			/* yellow missile, white shells (this is the terminology on the schematics) */
			color = ((offs == 7*4) ? BULLETS_COLOR_BASE : BULLETS_COLOR_BASE + 1);

			*BITMAP_ADDR16(bitmap, y, x) = color;
		}
	}
}

static void scramble_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	if (flipscreen_x)  x++;

	x = x - 6;

	if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
		/* yellow bullets */
		*BITMAP_ADDR16(bitmap, y, x) = BULLETS_COLOR_BASE;
}

static void darkplnt_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	if (flipscreen_x)  x++;

	x = x - 6;

	if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
		*BITMAP_ADDR16(bitmap, y, x) = 32 + darkplnt_bullet_color;
}

static void dambustr_draw_bullets(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int offs, int x, int y)
{
	int i, color;

	if (flip_screen_x_get(machine))  x++;

	x = x - 6;

	/* bullets are 2 pixels wide */
	for (i = 0; i < 2; i++)
	{
		if (offs < 4*4)
		{
			color = BULLETS_COLOR_BASE;
			y--;
		}
		else {
			color = BULLETS_COLOR_BASE + 1;
			x--;
		}

		if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
			*BITMAP_ADDR16(bitmap, y, x) = color;
	}
}



/* background drawing functions */

static void galaxold_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	/* plain black background */
	bitmap_fill(bitmap,cliprect,0);
}

static void scramble_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	if (background_enable)
		bitmap_fill(bitmap,cliprect,BACKGROUND_COLOR_BASE);
	else
		bitmap_fill(bitmap,cliprect,0);
}

static void turtles_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int color = (background_blue << 2) | (background_green << 1) | background_red;

	bitmap_fill(bitmap,cliprect,BACKGROUND_COLOR_BASE + color);
}

static void stratgyx_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT8 x;
	UINT8 *prom;
	int base = BACKGROUND_COLOR_BASE;


	/* the background PROM is connected the following way:

       bit 0 = 0 enables the blue gun if BCB is asserted
       bit 1 = 0 enables the red gun if BCR is asserted and
                 the green gun if BCG is asserted
       bits 2-7 are unconnected */

	prom = memory_region(machine, "user1");

	for (x = 0; x < 32; x++)
	{
		int sx,color;


		color = 0;

		if ((~prom[x] & 0x02) && background_red)   color |= 0x01;
		if ((~prom[x] & 0x02) && background_green) color |= 0x02;
		if ((~prom[x] & 0x01) && background_blue)  color |= 0x04;

		if (flipscreen_x)
			sx = 8 * (31 - x);
		else
			sx = 8 * x;

		plot_box(bitmap, sx, 0, 8, 256, base + color);
	}
}

static void minefld_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	if (background_enable)
	{
		int base = BACKGROUND_COLOR_BASE;
		int x;


		for (x = 0; x < 128; x++)
			plot_box(bitmap, x,       0, 1, 256, base + x);

		for (x = 0; x < 120; x++)
			plot_box(bitmap, x + 128, 0, 1, 256, base + x + 128);

		plot_box(bitmap, 248, 0, 16, 256, base);
	}
	else
		bitmap_fill(bitmap,cliprect,0);
}

static void rescue_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	if (background_enable)
	{
		int base = BACKGROUND_COLOR_BASE;
		int x;

		for (x = 0; x < 128; x++)
			plot_box(bitmap, x,       0, 1, 256, base + x);

		for (x = 0; x < 120; x++)
			plot_box(bitmap, x + 128, 0, 1, 256, base + x + 8);

		plot_box(bitmap, 248, 0, 16, 256, base);
	}
	else
		bitmap_fill(bitmap,cliprect,0);
}

static void mariner_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int base = BACKGROUND_COLOR_BASE;
	UINT8 x;
	UINT8 *prom;


	/* the background PROM contains the color codes for each 8 pixel
       line (column) of the screen.  The first 0x20 bytes for unflipped,
       and the 2nd 0x20 bytes for flipped screen. */

	prom = memory_region(machine, "user1");

	if (flipscreen_x)
	{
		for (x = 0; x < 32; x++)
		{
			int color;

			if (x == 0)
				color = 0;
			else
				color = prom[0x20 + x - 1];

			plot_box(bitmap, 8 * (31 - x), 0, 8, 256, base + color);
		}
	}
	else
	{
		for (x = 0; x < 32; x++)
		{
			int color;

			if (x == 31)
				color = 0;
			else
				color = prom[x + 1];

			plot_box(bitmap, 8 * x, 0, 8, 256, base + color);
		}
	}
}

static void dambustr_draw_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int base = BACKGROUND_COLOR_BASE;
	int col1 = base + dambustr_bg_color_1;
	int col2 = base + dambustr_bg_color_2;

	if (flip_screen_x_get(machine))
	{
		plot_box(bitmap,   0, 0, 256-dambustr_bg_split_line, 256, col2);
		plot_box(bitmap, 256-dambustr_bg_split_line, 0, dambustr_bg_split_line, 256, col1);
	}
	else
	{
		plot_box(bitmap,   0, 0, 256-dambustr_bg_split_line, 256, col1);
		plot_box(bitmap, 256-dambustr_bg_split_line, 0, dambustr_bg_split_line, 256, col2);
	}

}

static void dambustr_draw_upper_background(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	static rectangle clip = { 0, 0, 0, 0 };

	if (flip_screen_x_get(machine))
	{
		clip.min_x = 254 - dambustr_bg_split_line;
		clip.max_x = dambustr_bg_split_line;
		clip.min_y = 0;
		clip.max_y = 255;
		copybitmap(bitmap, dambustr_tmpbitmap, 0, 0, 0, 0, &clip);
	}
	else
	{
		clip.min_x = 0;
		clip.max_x = 254 - dambustr_bg_split_line;
		clip.min_y = 0;
		clip.max_y = 255;
		copybitmap(bitmap, dambustr_tmpbitmap, 0, 0, 0, 0, &clip);
	}
}



/* star drawing functions */

void galaxold_init_stars(running_machine *machine, int colors_offset)
{
	int i;
	int total_stars;
	UINT32 generator;
	int x,y;


	galaxold_stars_on = 0;
	stars_blink_state = 0;
	stars_blink_timer = timer_alloc(machine, stars_blink_callback, NULL);
	stars_scroll_timer = timer_alloc(machine, stars_scroll_callback, NULL);
	timer_adjusted = 0;
	stars_colors_start = colors_offset;

	for (i = 0;i < 64;i++)
	{
		int bits,r,g,b;
		static const int map[4] = { 0x00, 0x88, 0xcc, 0xff };


		bits = (i >> 0) & 0x03;
		r = map[bits];
		bits = (i >> 2) & 0x03;
		g = map[bits];
		bits = (i >> 4) & 0x03;
		b = map[bits];
		palette_set_color_rgb(machine,colors_offset+i,r,g,b);
	}


	/* precalculate the star background */

	total_stars = 0;
	generator = 0;

	for (y = 0;y < 256;y++)
	{
		for (x = 0;x < 512;x++)
		{
			UINT32 bit0;


			bit0 = ((~generator >> 16) & 0x01) ^ ((generator >> 4) & 0x01);

			generator = (generator << 1) | bit0;

			if (((~generator >> 16) & 0x01) && (generator & 0xff) == 0xff)
			{
				int color;


				color = (~(generator >> 8)) & 0x3f;
				if (color)
				{
					stars[total_stars].x = x;
					stars[total_stars].y = y;
					stars[total_stars].color = color;

					total_stars++;
				}
			}
		}
	}

	if (total_stars != STAR_COUNT)
	{
		fatalerror("total_stars = %d, STAR_COUNT = %d",total_stars,STAR_COUNT);
	}
}

static void plot_star(bitmap_t *bitmap, int x, int y, int color, const rectangle *cliprect)
{
	if (flipscreen_x)
		x = 255 - x;

	if (flipscreen_y)
		y = 255 - y;

	if ((x >= cliprect->min_x) && (x <= cliprect->max_x) && (y >= cliprect->min_y) && (y <= cliprect->max_y))
		*BITMAP_ADDR16(bitmap, y, x) = stars_colors_start + color;
}

static void noop_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
}

void galaxold_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;


	if (!timer_adjusted)
	{
		start_stars_scroll_timer(machine);
		timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = ((stars[offs].x +   stars_scrollpos) & 0x01ff) >> 1;
		y = ( stars[offs].y + ((stars_scrollpos + stars[offs].x) >> 9)) & 0xff;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

static void scramble_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;


	if (!timer_adjusted)
	{
		start_stars_blink_timer(100000, 10000, 0.00001);
		timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = stars[offs].x >> 1;
		y = stars[offs].y;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			/* determine when to skip plotting */
			switch (stars_blink_state & 0x03)
			{
			case 0:
				if (!(stars[offs].color & 0x01))  continue;
				break;
			case 1:
				if (!(stars[offs].color & 0x04))  continue;
				break;
			case 2:
				if (!(stars[offs].y & 0x02))  continue;
				break;
			case 3:
				/* always plot */
				break;
			}

			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

static void rescue_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;


	/* same as Scramble, but only top (left) half of screen */

	if (!timer_adjusted)
	{
		start_stars_blink_timer(100000, 10000, 0.00001);
		timer_adjusted = 1;
	}


	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = stars[offs].x >> 1;
		y = stars[offs].y;

		if ((x < 128) && ((y & 0x01) ^ ((x >> 3) & 0x01)))
		{
			/* determine when to skip plotting */
			switch (stars_blink_state & 0x03)
			{
			case 0:
				if (!(stars[offs].color & 0x01))  continue;
				break;
			case 1:
				if (!(stars[offs].color & 0x04))  continue;
				break;
			case 2:
				if (!(stars[offs].y & 0x02))  continue;
				break;
			case 3:
				/* always plot */
				break;
			}

			plot_star(bitmap, x, y, stars[offs].color, cliprect);
		}
	}
}

static void mariner_draw_stars(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;
	UINT8 *prom;


	if (!timer_adjusted)
	{
		start_stars_scroll_timer(machine);
		timer_adjusted = 1;
	}


	/* bit 2 of the PROM controls star visibility */

	prom = memory_region(machine, "user2");

	for (offs = 0;offs < STAR_COUNT;offs++)
	{
		int x,y;


		x = ((stars[offs].x +   -stars_scrollpos) & 0x01ff) >> 1;
		y = ( stars[offs].y + ((-stars_scrollpos + stars[offs].x) >> 9)) & 0xff;

		if ((y & 0x01) ^ ((x >> 3) & 0x01))
		{
			if (prom[(x/8 + 1) & 0x1f] & 0x04)
			{
				plot_star(bitmap, x, y, stars[offs].color, cliprect);
			}
		}
	}
}

static TIMER_CALLBACK( stars_blink_callback )
{
	stars_blink_state++;
}

static void start_stars_blink_timer(double ra, double rb, double c)
{
	/* calculate the period using the formula given in the 555 datasheet */

	int period_in_ms = 693 * (ra + 2.0 * rb) * c;

	timer_adjust_periodic(stars_blink_timer, ATTOTIME_IN_MSEC(period_in_ms), 0, ATTOTIME_IN_MSEC(period_in_ms));
}


static TIMER_CALLBACK( stars_scroll_callback )
{
	if (galaxold_stars_on)
	{
		stars_scrollpos++;
	}
}

static void start_stars_scroll_timer(running_machine *machine)
{
	timer_adjust_periodic(stars_scroll_timer, video_screen_get_frame_period(machine->primary_screen), 0, video_screen_get_frame_period(machine->primary_screen));
}



static TILE_GET_INFO( get_tile_info )
{
	UINT8 x = tile_index & 0x1f;

	UINT16 code = galaxold_videoram[tile_index];
	UINT8 color = galaxold_attributesram[(x << 1) | 1] & color_mask;

	if (modify_charcode)
	{
		modify_charcode(machine, &code, x);
	}

	if (modify_color)
	{
		modify_color(&color);
	}

	SET_TILE_INFO(0, code, color, 0);
}

static TILE_GET_INFO( rockclim_get_tile_info )
{
	UINT16 code = rockclim_videoram[tile_index];
	SET_TILE_INFO(2, code, 0, 0);
}

static void draw_bullets_common(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	int offs;


	for (offs = 0;offs < galaxold_bulletsram_size;offs += 4)
	{
		UINT8 sx,sy;

		sy = 255 - galaxold_bulletsram[offs + 1];
		sx = 255 - galaxold_bulletsram[offs + 3];

		if (flipscreen_y)  sy = 255 - sy;

		draw_bullets(machine, bitmap, cliprect, offs, sx, sy);
	}
}


static void draw_sprites(running_machine *machine, bitmap_t *bitmap, UINT8 *spriteram, size_t spriteram_size)
{
	int offs;


	for (offs = spriteram_size - 4;offs >= 0;offs -= 4)
	{
		UINT8 sx,sy,color;
		int flipx,flipy,code;


		sx = spriteram[offs + 3] + 1;	/* the existence of +1 is supported by a LOT of games */
		sy = spriteram[offs];			/* Anteater, Mariner, for example */
		flipx = spriteram[offs + 1] & 0x40;
		flipy = spriteram[offs + 1] & 0x80;
		code = spriteram[offs + 1] & 0x3f;
		color = spriteram[offs + 2] & color_mask;

		if (modify_spritecode)
		{
			modify_spritecode(spriteram, &code, &flipx, &flipy, offs);
		}

		if (modify_color)
		{
			modify_color(&color);
		}

		if (modify_ypos)
		{
			modify_ypos(&sy);
		}

		if (flipscreen_x)
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flipscreen_y)
		{
			flipy = !flipy;
		}
		else
		{
			sy = 240 - sy;
		}


		/* In at least Amidar Turtles, sprites #0, #1 and #2 need to be moved */
		/* down (left) one pixel to be positioned correctly. */
		/* Note that the adjustment must be done AFTER handling flipscreen, thus */
		/* proving that this is a hardware related "feature" */

		if (offs < 3*4)  sy++;


		drawgfx(bitmap,machine->gfx[1],
				code,color,
				flipx,flipy,
				sx,sy,
				flipscreen_x ? spritevisibleareaflipx : spritevisiblearea,TRANSPARENCY_PEN,0);
	}
}


VIDEO_UPDATE( galaxold )
{
	draw_background(screen->machine, bitmap, cliprect);


	if (galaxold_stars_on)
	{
		draw_stars(screen->machine, bitmap, cliprect);
	}


	tilemap_draw(bitmap, 0, bg_tilemap, 0, 0);


	if (draw_bullets)
	{
		draw_bullets_common(screen->machine, bitmap, cliprect);
	}


	draw_sprites(screen->machine, bitmap, galaxold_spriteram, galaxold_spriteram_size);

	if (spriteram2_present)
	{
		draw_sprites(screen->machine, bitmap, galaxold_spriteram2, galaxold_spriteram2_size);
	}
	return 0;
}


VIDEO_UPDATE( dambustr )
{
	int i, j;
	UINT8 color;

	draw_background(screen->machine, bitmap, cliprect);

	if (galaxold_stars_on)
	{
		draw_stars(screen->machine, bitmap, cliprect);
	}

	/* save the background for drawing it again later, if background has priority over characters */
	copybitmap(dambustr_tmpbitmap, bitmap, 0, 0, 0, 0, NULL);

	tilemap_draw(bitmap, 0, bg_tilemap, 0, 0);

	if (draw_bullets)
	{
		draw_bullets_common(screen->machine, bitmap, cliprect);
	}

	draw_sprites(screen->machine, bitmap, galaxold_spriteram, galaxold_spriteram_size);

	if (dambustr_bg_priority)
	{
		/* draw the upper part of the background, as it has priority */
		dambustr_draw_upper_background(screen->machine, bitmap, cliprect);

		/* only rows with color code > 3 are stronger than the background */
		memset(dambustr_videoram2, 0x20, 0x0400);
		for (i=0; i<32; i++) {
			color = galaxold_attributesram[(i << 1) | 1] & color_mask;
			if (color > 3) {
				for (j=0; j<32; j++)
					dambustr_videoram2[32*j+i] = galaxold_videoram[32*j+i];
			};
		};
		tilemap_mark_all_tiles_dirty(dambustr_tilemap2);
		tilemap_draw(bitmap, 0, dambustr_tilemap2, 0, 0);
	};

	return 0;
}

