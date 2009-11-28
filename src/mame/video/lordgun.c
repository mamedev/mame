/***************************************************************************

                      -= IGS Lord Of Gun =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q / W / E / R   Shows Layer 0 / 1 / 2 / 3
        A               Shows Sprites

        Keys can be used together!


    [ 4 Scrolling Layers ]

                Tiles       Layer size

                8 x 8 x 6   0x800 x 0x200
                8 x 8 x 6   0x200 x 0x100
                16x16 x 6   0x800 x 0x200
                32x32 x 6   0x800 x 0x800

    [ 256 Sprites ]

        Plain 16 x 16 x 6 sprites

    [ 2048 colors ]

    [ Priorities ]

        RAM based priorities, with probably a per tile priority code
        (the same sprite goes below some parts, and above others, of the same layer)

***************************************************************************/

#include "driver.h"
#include "lordgun.h"

// Variables needed by driver:

UINT16 *lordgun_vram_0, *lordgun_scroll_x_0, *lordgun_scroll_y_0;
UINT16 *lordgun_vram_1, *lordgun_scroll_x_1, *lordgun_scroll_y_1;
UINT16 *lordgun_vram_2, *lordgun_scroll_x_2, *lordgun_scroll_y_2;
UINT16 *lordgun_vram_3, *lordgun_scroll_x_3, *lordgun_scroll_y_3;
UINT16 *lordgun_scrollram;
int lordgun_whitescreen;

lordgun_gun_data lordgun_gun[2];


/***************************************************************************

    Tilemaps

***************************************************************************/

static tilemap *tilemap_0, *tilemap_1, *tilemap_2, *tilemap_3;

static TILE_GET_INFO( get_tile_info_0 )
{
	UINT16 attr = lordgun_vram_0[ tile_index * 2 + 0 ];
	UINT16 code = lordgun_vram_0[ tile_index * 2 + 1 ];
	SET_TILE_INFO( 1, code, (attr & 0x0030) >> 4, TILE_FLIPXY(attr >> 14));
}

static TILE_GET_INFO( get_tile_info_1 )
{
	UINT16 attr = lordgun_vram_1[ tile_index * 2 + 0 ];
	UINT16 code = lordgun_vram_1[ tile_index * 2 + 1 ];
	SET_TILE_INFO( 2, code, (attr & 0x0070) >> 4, TILE_FLIPXY(attr >> 14));
}

static TILE_GET_INFO( get_tile_info_2 )
{
	UINT16 attr = lordgun_vram_2[ tile_index * 2 + 0 ];
	UINT16 code = lordgun_vram_2[ tile_index * 2 + 1 ];
	SET_TILE_INFO( 3, code, (attr & 0x0300) >> 8, TILE_FLIPXY(attr >> 14));
}

static TILE_GET_INFO( get_tile_info_3 )
{
	UINT16 attr = lordgun_vram_3[ tile_index * 2 + 0 ];
	UINT16 code = lordgun_vram_3[ tile_index * 2 + 1 ];
	SET_TILE_INFO( 4, code, (attr & 0x00f0) >> 4, TILE_FLIPXY(attr >> 14));
}

WRITE16_HANDLER( lordgun_vram_0_w )
{
	COMBINE_DATA(&lordgun_vram_0[offset]);
	tilemap_mark_tile_dirty(tilemap_0, offset/2);
}

WRITE16_HANDLER( lordgun_vram_1_w )
{
	COMBINE_DATA(&lordgun_vram_1[offset]);
	tilemap_mark_tile_dirty(tilemap_1, offset/2);
}

WRITE16_HANDLER( lordgun_vram_2_w )
{
	COMBINE_DATA(&lordgun_vram_2[offset]);
	tilemap_mark_tile_dirty(tilemap_2, offset/2);
}

WRITE16_HANDLER( lordgun_vram_3_w )
{
	COMBINE_DATA(&lordgun_vram_3[offset]);
	tilemap_mark_tile_dirty(tilemap_3, offset/2);
}

/***************************************************************************

    Video Init

***************************************************************************/

VIDEO_START( lordgun )
{
	tilemap_0 = tilemap_create(	machine, get_tile_info_0, tilemap_scan_rows,
								 8,8, 0x100, 0x40 );

	tilemap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows,
								 16,16, 0x80,0x20 );

	tilemap_2 = tilemap_create(	machine, get_tile_info_2, tilemap_scan_rows,
								 32,32, 0x40,0x40 );

	tilemap_3 = tilemap_create(	machine, get_tile_info_3, tilemap_scan_rows,
								 8,8, 0x40,0x20 );

	tilemap_set_scroll_rows(tilemap_0,1);
	tilemap_set_scroll_cols(tilemap_0,1);
	tilemap_set_transparent_pen(tilemap_0,0x3f);

	tilemap_set_scroll_rows(tilemap_1,0x200);
	tilemap_set_scroll_cols(tilemap_1,1);
	tilemap_set_transparent_pen(tilemap_1,0x3f);

	tilemap_set_scroll_rows(tilemap_2,1);
	tilemap_set_scroll_cols(tilemap_2,1);
	tilemap_set_transparent_pen(tilemap_2,0x3f);

	tilemap_set_scroll_rows(tilemap_3,1);
	tilemap_set_scroll_cols(tilemap_3,1);
	tilemap_set_transparent_pen(tilemap_3,0x3f);
}

/***************************************************************************

    Gun screen position

***************************************************************************/

static const int lordgun_gun_x_table[] =
{
	-100, 0x001,0x001,0x002,0x002,0x003,0x003,0x004,0x005,0x006,0x007,0x008,0x009,0x00A,0x00B,0x00C,
	0x00D,0x00E,0x00F,0x010,0x011,0x012,0x013,0x014,0x015,0x016,0x017,0x018,0x019,0x01A,0x01B,0x01C,
	0x01D,0x01E,0x01F,0x020,0x021,0x022,0x023,0x024,0x025,0x026,0x027,0x028,0x029,0x02A,0x02B,0x02C,
	0x02D,0x02E,0x02F,0x030,0x031,0x032,0x033,0x034,0x035,0x036,0x037,0x038,0x039,0x03A,0x03B,0x03C,
	0x03D,0x03E,0x03F,0x040,0x041,0x043,0x044,0x045,0x046,0x047,0x048,0x049,0x04A,0x04B,0x04C,0x04E,
	0x04F,0x050,0x051,0x052,0x053,0x054,0x055,0x056,0x057,0x059,0x05A,0x05B,0x05C,0x05D,0x05E,0x05F,
	0x060,0x061,0x05A,0x063,0x065,0x066,0x067,0x068,0x069,0x06A,0x06B,0x06C,0x06D,0x06E,0x06F,0x071,
	0x072,0x074,0x075,0x077,0x078,0x07A,0x07B,0x07D,0x07E,0x080,0x081,0x083,0x085,0x087,0x089,0x08B,
	0x08D,0x08E,0x08F,0x090,0x092,0x093,0x095,0x097,0x098,0x099,0x09A,0x09B,0x09C,0x09D,0x09E,0x0A0,
	0x0A1,0x0A2,0x0A3,0x0A4,0x0A5,0x0A6,0x0A7,0x0A8,0x0A9,0x0AA,0x0AC,0x0AD,0x0AE,0x0AF,0x0B0,0x0B1,
	0x0B2,0x0B3,0x0B4,0x0B5,0x0B6,0x0B8,0x0B9,0x0BA,0x0BB,0x0BC,0x0BD,0x0BE,0x0BF,0x0C0,0x0C1,0x0C2,
	0x0C4,0x0C5,0x0C6,0x0C7,0x0C8,0x0C9,0x0CA,0x0CB,0x0CC,0x0CD,0x0CF,0x0D0,0x0D1,0x0D2,0x0D3,0x0D4,
	0x0D5,0x0D6,0x0D7,0x0D8,0x0D9,0x0DB,0x0DC,0x0DD,0x0DE,0x0DF,0x0E0,0x0E1,0x0E2,0x0E3,0x0E4,0x0E5,
	0x0E7,0x0E8,0x0E9,0x0EA,0x0EB,0x0EC,0x0ED,0x0EE,0x0EF,0x0F0,0x0F1,0x0F3,0x0F4,0x0F5,0x0F6,0x0F7,
	0x0F8,0x0F9,0x0FA,0x0FB,0x0FC,0x0FE,0x0FF,0x100,0x101,0x102,0x103,0x104,0x105,0x106,0x107,0x108,
	0x10A,0x10B,0x10C,0x10D,0x10E,0x10F,0x110,0x111,0x112,0x113,0x114,0x116,0x117,0x118,0x119,0x11A,
	0x11B,0x11C,0x11D,0x11E,0x11F,0x120,0x122,0x123,0x124,0x125,0x126,0x127,0x128,0x129,0x12A,0x12B,
	0x12C,0x12E,0x12F,0x130,0x131,0x132,0x133,0x134,0x135,0x136,0x137,0x139,0x13A,0x13B,0x13C,0x13D,
	0x13E,0x13F,0x140,0x141,0x142,0x143,0x145,0x146,0x147,0x148,0x149,0x14A,0x14B,0x14C,0x14D,0x14E,
	0x14F,0x151,0x152,0x153,0x154,0x155,0x156,0x157,0x158,0x159,0x15A,0x15B,0x15D,0x15E,0x15F,0x160,
	0x161,0x162,0x163,0x164,0x165,0x166,0x167,0x169,0x16A,0x16B,0x16C,0x16D,0x16E,0x16F,0x170,0x171,
	0x172,0x174,0x175,0x176,0x177,0x178,0x179,0x17A,0x17B,0x17C,0x17D,0x17E,0x17F,0x180,0x181,0x182,
	0x183,0x184,0x185,0x186,0x187,0x188,0x189,0x18A,0x18B,0x18C,0x18D,0x18E,0x18F,0x190,0x191,0x192,
	0x193,0x194,0x195,0x196,0x197,0x198,0x199,0x19A,0x19B,0x19C,0x19D,0x19E,0x19F,0x1A0,0x1A1,0x1A2,
	0x1A3,0x1A4,0x1A5,0x1A6,0x1A7,0x1A8,0x1A9,0x1AA,0x1AB,0x1AC,0x1AD,0x1AE,0x1AF,0x1B0,0x1B1,0x1B2,
	0x1B3,0x1B4,0x1B5,0x1B6,0x1B7,0x1B8,0x1B9,0x1BA,0x1BB,0x1BC,0x1BD,0x1BE,0x1BF,0x1BF
};


static const char *const gunnames[] = { "LIGHT0_X", "LIGHT1_X", "LIGHT0_Y", "LIGHT1_Y" };

static void lorddgun_calc_gun_scr(running_machine *machine, int i)
{
	int x = input_port_read(machine, gunnames[i]) - 0x3c;

	if ( (x < 0) || (x > sizeof(lordgun_gun_x_table)/sizeof(lordgun_gun_x_table[0])) )
		x = 0;

	lordgun_gun[i].scr_x = lordgun_gun_x_table[x];
	lordgun_gun[i].scr_y = input_port_read(machine, gunnames[i+2]);
}

void lordgun_update_gun(running_machine *machine, int i)
{
	const rectangle *visarea = video_screen_get_visible_area(machine->primary_screen);

	lordgun_gun[i].hw_x = input_port_read(machine, gunnames[i]);
	lordgun_gun[i].hw_y = input_port_read(machine, gunnames[i+2]);

	lorddgun_calc_gun_scr(machine, i);

	if (	(lordgun_gun[i].scr_x < visarea->min_x)	||
			(lordgun_gun[i].scr_x > visarea->max_x)	||
			(lordgun_gun[i].scr_y < visarea->min_y)	||
			(lordgun_gun[i].scr_y > visarea->max_y)	)
		lordgun_gun[i].hw_x = lordgun_gun[i].hw_y = 0;
}


/***************************************************************************

    Sprites

***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
	UINT16 *s		=	machine->generic.spriteram.u16;
	UINT16 *end		=	machine->generic.spriteram.u16 + machine->generic.spriteram_size/2;

	for ( ; s < end; s += 8/2 )
	{
		int attr, code, color, sx, sy, flipx, flipy;

		sy		=		s[ 0 ];
		attr	=		s[ 1 ];
		code	=		s[ 2 ];
		sx		=		s[ 3 ];

		// Last sprite
		if (sy & 0x8000) break;

		flipx	=	 attr & 0x8000;
		flipy	=	 attr & 0x4000;
		color	=	(attr & 0x00f0) >> 4;

		// Sign extend the position
		sx	-=	0x18;
		sy	=	(sy & 0x7ff) - (sy & 0x800);

		drawgfx_transpen(	bitmap,	cliprect, machine->gfx[0],
					code,	color,
					flipx,	flipy,	sx,	sy, 0x3f);
	}
}

/***************************************************************************

    Video Update

***************************************************************************/

VIDEO_UPDATE( lordgun )
{
	int layers_ctrl = -1;
	int y;

#ifdef MAME_DEBUG
if (input_code_pressed(screen->machine, KEYCODE_Z))
{
	int msk = 0;

	if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
	if (input_code_pressed(screen->machine, KEYCODE_E))    msk |= 4;
	if (input_code_pressed(screen->machine, KEYCODE_R))	msk |= 8;
	if (input_code_pressed(screen->machine, KEYCODE_A))	msk |= 16;
	if (msk != 0) layers_ctrl &= msk;
}
#endif

	if (lordgun_whitescreen)
	{
		bitmap_fill( bitmap, cliprect , get_white_pen(screen->machine));
		return 0;
	}

	tilemap_set_scrollx( tilemap_0, 0, *lordgun_scroll_x_0 );
	tilemap_set_scrolly( tilemap_0, 0, *lordgun_scroll_y_0 );

	for (y = 0; y < 0x200; y++)
		tilemap_set_scrollx( tilemap_1, y, (*lordgun_scroll_x_1) + lordgun_scrollram[y * 4/2 + 2/2]);
	tilemap_set_scrolly( tilemap_1, 0, *lordgun_scroll_y_1 );

	tilemap_set_scrollx( tilemap_2, 0, *lordgun_scroll_x_2 );
	tilemap_set_scrolly( tilemap_2, 0, *lordgun_scroll_y_2 );

	tilemap_set_scrollx( tilemap_3, 0, *lordgun_scroll_x_3 );
	tilemap_set_scrolly( tilemap_3, 0, *lordgun_scroll_y_3 );

	bitmap_fill( bitmap, cliprect , 0);

	if (layers_ctrl & 4)	tilemap_draw(bitmap, cliprect, tilemap_2, 0, 0);
	if (layers_ctrl & 1)	tilemap_draw(bitmap, cliprect, tilemap_0, 0, 0);
	if (layers_ctrl & 2)	tilemap_draw(bitmap, cliprect, tilemap_1, 0, 0);
	if (layers_ctrl & 16)	draw_sprites(screen->machine, bitmap, cliprect);
	if (layers_ctrl & 8)	tilemap_draw(bitmap, cliprect, tilemap_3, 0, 0);

	return 0;
}
