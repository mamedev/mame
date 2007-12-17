/***************************************************************************

                            -= Seta Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)


Note:   if MAME_DEBUG is defined, pressing Z with:

        Q           shows layer 0
        W           shows layer 1
        A           shows the sprites

        Keys can be used together!


                        [ 0, 1 Or 2 Scrolling Layers ]

    Each layer consists of 2 tilemaps: only one can be displayed at a
    given time (the games usually flip continuously between the two).
    The two tilemaps share the same scrolling registers.

        Layer Size:             1024 x 512
        Tiles:                  16x16x4 (16x16x6 in some games)
        Tile Format:

            Offset + 0x0000:
                            f--- ---- ---- ----     Flip X
                            -e-- ---- ---- ----     Flip Y
                            --dc ba98 7654 3210     Code

            Offset + 0x1000:

                            fedc ba98 765- ----     -
                            ---- ---- ---4 3210     Color

            The other tilemap for this layer (always?) starts at
            Offset + 0x2000.


                            [ 1024 Sprites ]

    Sprites are 16x16x4. They are just like those in "The Newzealand Story",
    "Revenge of DOH" etc (tnzs.c). Obviously they're hooked to a 16 bit
    CPU here, so they're mapped a bit differently in memory. Additionally,
    there are two banks of sprites. The game can flip between the two to
    do double buffering, writing to a bit of a control register(see below)


        Spriteram16_2 + 0x000.w

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc ba-- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0x400.w

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     X

        Spriteram16   + 0x000.w

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y



                            [ Floating Tilemap ]

    There's a floating tilemap made of vertical colums composed of 2x16
    "sprites". Each 32 consecutive "sprites" define a column.

    For column I:

        Spriteram16_2 + 0x800 + 0x40 * I:

                        f--- ---- ---- ----     Flip X
                        -e-- ---- ---- ----     Flip Y
                        --dc b--- ---- ----     -
                        ---- --98 7654 3210     Code (Lower bits)

        Spriteram16_2 + 0xc00 + 0x40 * I:

                        fedc b--- ---- ----     Color
                        ---- -a9- ---- ----     Code (Upper Bits)
                        ---- ---8 7654 3210     -

    Each column has a variable horizontal position and a vertical scrolling
    value (see also the Sprite Control Registers). For column I:


        Spriteram16   + 0x400 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Y

        Spriteram16   + 0x408 + 0x20 * I:

                        fedc ba98 ---- ----     -
                        ---- ---- 7654 3210     Low Bits Of X



                        [ Sprites Control Registers ]


        Spriteram16   + 0x601.b

                        7--- ----       0
                        -6-- ----       Flip Screen
                        --5- ----       0
                        ---4 ----       1 (Sprite Enable?)
                        ---- 3210       ???

        Spriteram16   + 0x603.b

                        7--- ----       0
                        -6-- ----       Sprite Bank
                        --5- ----       0 = Sprite Buffering (blandia,msgundam,qzkklogy)
                        ---4 ----       0
                        ---- 3210       Columns To Draw (1 is the special value for 16)

        Spriteram16   + 0x605.b

                        7654 3210       High Bit Of X For Columns 7-0

        Spriteram16   + 0x607.b

                        7654 3210       High Bit Of X For Columns f-8




***************************************************************************/

#include "driver.h"
#include "sound/x1_010.h"
#include "seta.h"

/* Variables only used here */

static tilemap *tilemap_0, *tilemap_1;	// Layer 0
static tilemap *tilemap_2, *tilemap_3;	// Layer 1
static int tilemaps_flip;

/* Variables used elsewhere */

int seta_tiles_offset;

UINT16 *seta_vram_0, *seta_vctrl_0;
UINT16 *seta_vram_2, *seta_vram_3, *seta_vctrl_2;
UINT16 *seta_vregs;

UINT16 *seta_workram; // Used for zombraid crosshair hack

static int twineagl_tilebank[4];
static int	seta_samples_bank;

struct x_offset
{
	/* 2 values, for normal and flipped */
	const char *gamename;
	int sprite_offs[2];
	int tilemap_offs[2];
};

/* note that drgnunit, stg and qzkklogy run on the same board, yet they need different alignment */
static struct x_offset game_offsets[] =
{
	/* only sprites */
	{ "tndrcade", { -1,  0 } },				// correct (wall at beginning of game)
	{ "tndrcadj", { -1,  0 } },				// correct (wall at beginning of game)
	{ "wits",     {  0,  0 } },				// unknown
	{ "thunderl", {  0,  0 } },				// unknown
	{ "wiggie",   {  0,  0 } },				// some problems but they seem y co-ordinate related?
	{ "blockcar", {  0,  0 } },				// unknown
	{ "umanclub", {  0,  0 } },				// unknown
	{ "atehate",  {  0,  0 } },				// correct (test grid)
	{ "kiwame",   {  0,-16 } },				// correct (test grid)
	{ "krzybowl", {  0,  0 } },				// correct (test grid)

	/* 1 layer */
	{ "twineagl", {  0,  0 }, {  0, -3 } },	// unknown
	{ "downtown", {  1,  0 }, { -1,  0 } },	// sprites correct (test grid), tilemap unknown but at least -1 non-flipped to fix glitches later in the game
	{ "usclssic", {  1,  2 }, {  0, -1 } },	// correct (test grid and bg)
	{ "calibr50", { -1,  2 }, { -3, -2 } },	// correct (test grid and roof in animation at beginning of game)
	{ "arbalest", {  0,  1 }, { -2, -1 } },	// correct (test grid and landing pad at beginning of game)
	{ "metafox",  {  0,  0 }, { 16,-19 } },	// sprites unknown, tilemap correct (test grid)
	{ "drgnunit", {  2,  2 }, { -2, -2 } },	// correct (test grid and I/O test)
	{ "stg",      {  0,  0 }, { -2, -2 } },	// sprites correct? (panel), tilemap correct (test grid)
	{ "qzkklogy", {  1,  1 }, { -1, -1 } },	// correct (timer, test grid)
	{ "qzkklgy2", {  0,  0 }, { -1, -3 } },	// sprites unknown, tilemaps correct (test grid)

	/* 2 layers */
	{ "rezon",    {  0,  0 }, { -2, -2 } },	// correct (test grid)
	{ "blandia",  {  0,  8 }, { -2,  6 } },	// correct (test grid, startup bg)
	{ "blandiap", {  0,  8 }, { -2,  6 } },	// correct (test grid, startup bg)
	{ "zingzip",  {  0,  0 }, { -1, -2 } },	// sprites unknown, tilemaps correct (test grid)
	{ "eightfrc", {  3,  4 }, {  0,  0 } },	// unknown
	{ "daioh",    {  1,  1 }, { -1, -1 } },	// correct? (launch window and test grid are right, but planet is wrong)
	{ "msgundam", {  0,  0 }, { -2, -2 } },	// correct (test grid, banpresto logo)
	{ "msgunda1", {  0,  0 }, { -2, -2 } },	// correct (test grid, banpresto logo)
	{ "oisipuzl", {  0,  0 }, { -1, -1 } },	// correct (test mode) flip screen not supported?
	{ "triplfun", {  0,  0 }, { -1, -1 } },	// correct (test mode) flip screen not supported?
	{ "wrofaero", {  0,  0 }, {  0,  0 } },	// unknown
	{ "jjsquawk", {  1,  1 }, { -1, -1 } },	// correct (test mode)
	{ "kamenrid", {  0,  0 }, { -2, -2 } },	// correct (map, banpresto logo)
	{ "extdwnhl", {  0,  0 }, { -2, -2 } },	// correct (test grid, background images)
	{ "sokonuke", {  0,  0 }, { -2, -2 } },	// correct (game selection, test grid)
	{ "gundhara", {  0,  0 }, {  0,  0 } },	// unknown, flip screen not supported?
	{ "zombraid", {  0,  0 }, { -2, -2 } },	// correct for normal, flip screen not working yet
	{ "madshark", {  0,  0 }, {  0,  0 } },	// unknown (wrong when flipped, but along y)
	{ "utoukond", {  0,  0 }, { -2,  0 } }, // unknown (wrong when flipped, but along y)
	{ "crazyfgt", {  0,  0 }, { -2,  0 } }, // wrong (empty background column in title screen, but aligned sprites in screen select)
	{ NULL }
};

static struct x_offset *global_offsets;


/*  ---- 3---       Coin #1 Lock Out
    ---- -2--       Coin #0 Lock Out
    ---- --1-       Coin #1 Counter
    ---- ---0       Coin #0 Counter     */

void seta_coin_lockout_w(int data)
{
	static int seta_coin_lockout = 1;
	static const game_driver *seta_driver = NULL;
	static const char *seta_nolockout[8] = { "blandia", "gundhara", "kamenrid", "zingzip", "eightfrc", "extdwnhl", "sokonuke", "zombraid"};

	/* Only compute seta_coin_lockout when confronted with a new gamedrv */
	if (seta_driver != Machine->gamedrv)
	{
		int i;
		seta_driver = Machine->gamedrv;

		seta_coin_lockout = 1;
		for (i=0; i<ARRAY_LENGTH(seta_nolockout); i++)
		{
			if (strcmp(seta_driver->name, seta_nolockout[i]) == 0 ||
				strcmp(seta_driver->parent, seta_nolockout[i]) == 0)
			{
				seta_coin_lockout = 0;
				break;
			}
		}
	}

	coin_counter_w		(0, (( data) >> 0) & 1 );
	coin_counter_w		(1, (( data) >> 1) & 1 );

	/* blandia, gundhara, kamenrid & zingzip haven't the coin lockout device */
	if (	!seta_coin_lockout )
		return;
	coin_lockout_w		(0, ((~data) >> 2) & 1 );
	coin_lockout_w		(1, ((~data) >> 3) & 1 );
}


WRITE16_HANDLER( seta_vregs_w )
{
	COMBINE_DATA(&seta_vregs[offset]);
	switch (offset)
	{
		case 0/2:

/*      fedc ba98 76-- ----
        ---- ---- --5- ----     Sound Enable
        ---- ---- ---4 ----     toggled in IRQ1 by many games, irq acknowledge?
                                [original comment for the above: ?? 1 in oisipuzl, sokonuke (layers related)]
        ---- ---- ---- 3---     Coin #1 Lock Out
        ---- ---- ---- -2--     Coin #0 Lock Out
        ---- ---- ---- --1-     Coin #1 Counter
        ---- ---- ---- ---0     Coin #0 Counter     */
			if (ACCESSING_LSB)
			{
				seta_coin_lockout_w (data & 0x0f);
				if (sndti_exists(SOUND_X1_010, 0))
					seta_sound_enable_w (data & 0x20);
				coin_counter_w(0,data & 0x01);
				coin_counter_w(1,data & 0x02);
			}
			break;

		case 2/2:
			if (ACCESSING_LSB)
			{
				int new_bank;

				/* Partly handled in vh_screenrefresh:

                        fedc ba98 76-- ----
                        ---- ---- --54 3---     Samples Bank (in blandia, eightfrc, zombraid)
                        ---- ---- ---- -2--
                        ---- ---- ---- --1-     Sprites Above Frontmost Layer
                        ---- ---- ---- ---0     Layer 0 Above Layer 1
                */

				new_bank = (data >> 3) & 0x7;

				if (new_bank != seta_samples_bank)
				{
					UINT8 *rom = memory_region(REGION_SOUND1);
					int samples_len = memory_region_length(REGION_SOUND1);
					int addr;

					seta_samples_bank = new_bank;

					if (samples_len == 0x240000)	/* blandia, eightfrc */
					{
						addr = 0x40000 * new_bank;
						if (new_bank >= 3)	addr += 0x40000;

						if ( (samples_len > 0x100000) && ((addr+0x40000) <= samples_len) )
							memcpy(&rom[0xc0000],&rom[addr],0x40000);
						else
							logerror("PC %06X - Invalid samples bank %02X !\n", activecpu_get_pc(), new_bank);
					}
					else if (samples_len == 0x480000)	/* zombraid */
					{
						/* bank 1 is never explicitly selected, 0 is used in its place */
						if (new_bank == 0) new_bank = 1;
						addr = 0x80000 * new_bank;
						if (new_bank > 0) addr += 0x80000;

						memcpy(&rom[0x80000],&rom[addr],0x80000);
					}
				}

			}
			break;


		case 4/2:	// ?
			break;
	}
}




/***************************************************************************

                        Callbacks for the TileMap code

                              [ Tiles Format ]

Offset + 0x0000:
                    f--- ---- ---- ----     Flip X
                    -e-- ---- ---- ----     Flip Y
                    --dc ba98 7654 3210     Code

Offset + 0x1000:

                    fedc ba98 765- ----     -
                    ---- ---- ---4 3210     Color


                      [ TileMaps Control Registers]

Offset + 0x0:                               Scroll X
Offset + 0x2:                               Scroll Y
Offset + 0x4:
                    fedc ba98 7654 3210     -
                    ---- ---- ---- 3---     Tilemap Select (There Are 2 Tilemaps Per Layer)
                    ---- ---- ---- -21-     0 (1 only in eightfrc, when flip is on!)
                    ---- ---- ---- ---0     ?

***************************************************************************/

INLINE void twineagl_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, UINT16 *vram )
{
	UINT16 code =	vram[ tile_index ];
	UINT16 attr =	vram[ tile_index + 0x800 ];
	if ((code & 0x3e00) == 0x3e00)
		code = (code & 0xc07f) | ((twineagl_tilebank[(code & 0x0180) >> 7] >> 1) << 7);
	SET_TILE_INFO( 1, (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

static TILE_GET_INFO( twineagl_get_tile_info_0 ) { twineagl_tile_info( machine, tileinfo, tile_index, seta_vram_0 + 0x0000 ); }
static TILE_GET_INFO( twineagl_get_tile_info_1 ) { twineagl_tile_info( machine, tileinfo, tile_index, seta_vram_0 + 0x1000 ); }


INLINE void get_tile_info( running_machine *machine, tile_data *tileinfo, int tile_index, int layer, UINT16 *vram )
{
	UINT16 code =	vram[ tile_index ];
	UINT16 attr =	vram[ tile_index + 0x800 ];
	SET_TILE_INFO( 1 + layer, seta_tiles_offset + (code & 0x3fff), attr & 0x1f, TILE_FLIPXY((code & 0xc000) >> 14) );
}

static TILE_GET_INFO( get_tile_info_0 ) { get_tile_info( machine, tileinfo, tile_index, 0, seta_vram_0 + 0x0000 ); }
static TILE_GET_INFO( get_tile_info_1 ) { get_tile_info( machine, tileinfo, tile_index, 0, seta_vram_0 + 0x1000 ); }
static TILE_GET_INFO( get_tile_info_2 ) { get_tile_info( machine, tileinfo, tile_index, 1, seta_vram_2 + 0x0000 ); }
static TILE_GET_INFO( get_tile_info_3 ) { get_tile_info( machine, tileinfo, tile_index, 1, seta_vram_2 + 0x1000 ); }


WRITE16_HANDLER( seta_vram_0_w )
{
	COMBINE_DATA(&seta_vram_0[offset]);
	if (offset & 0x1000)
		tilemap_mark_tile_dirty(tilemap_1, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(tilemap_0, offset & 0x7ff);
}

WRITE16_HANDLER( seta_vram_2_w )
{
	COMBINE_DATA(&seta_vram_2[offset]);
	if (offset & 0x1000)
		tilemap_mark_tile_dirty(tilemap_3, offset & 0x7ff);
	else
		tilemap_mark_tile_dirty(tilemap_2, offset & 0x7ff);
}

WRITE16_HANDLER( twineagl_tilebank_w )
{
	if (ACCESSING_LSB)
	{
		data &= 0xff;
		if (twineagl_tilebank[offset] != data)
		{
			twineagl_tilebank[offset] = data;
			tilemap_mark_all_tiles_dirty(ALL_TILEMAPS);
		}
	}
}



static void find_offsets(running_machine *machine)
{
	global_offsets = game_offsets;
	while (global_offsets->gamename && strcmp(machine->gamedrv->name,global_offsets->gamename))
		global_offsets++;
}

/* 2 layers */
VIDEO_START( seta_2_layers )
{
	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	tilemap_0 = tilemap_create(	get_tile_info_0, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );


	/* layer 1 */
	tilemap_2 = tilemap_create(	get_tile_info_2, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );

	tilemap_3 = tilemap_create(	get_tile_info_3, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );

		tilemaps_flip = 0;

		tilemap_set_transparent_pen(tilemap_0,0);
		tilemap_set_transparent_pen(tilemap_1,0);
		tilemap_set_transparent_pen(tilemap_2,0);
		tilemap_set_transparent_pen(tilemap_3,0);

		find_offsets(machine);
		seta_samples_bank = -1;	// set the samples bank to an out of range value at start-up
}


/* 1 layer */
VIDEO_START( seta_1_layer )
{
	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	tilemap_0 = tilemap_create(	get_tile_info_0, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );

	tilemap_1 = tilemap_create(	get_tile_info_1, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );


	/* NO layer 1 */
	tilemap_2 = 0;
	tilemap_3 = 0;

		tilemaps_flip = 0;

		tilemap_set_transparent_pen(tilemap_0,0);
		tilemap_set_transparent_pen(tilemap_1,0);

		find_offsets(machine);
		seta_samples_bank = -1;	// set the samples bank to an out of range value at start-up
}

VIDEO_START( twineagl_1_layer )
{
	/* Each layer consists of 2 tilemaps: only one can be displayed
       at any given time */

	/* layer 0 */
	tilemap_0 = tilemap_create(	twineagl_get_tile_info_0, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );

	tilemap_1 = tilemap_create(	twineagl_get_tile_info_1, tilemap_scan_rows,
								TILEMAP_TYPE_PEN, 16,16, 64,32 );


	/* NO layer 1 */
	tilemap_2 = 0;
	tilemap_3 = 0;

		tilemaps_flip = 0;

		tilemap_set_transparent_pen(tilemap_0,0);
		tilemap_set_transparent_pen(tilemap_1,0);

		find_offsets(machine);
		seta_samples_bank = -1;	// set the samples bank to an out of range value at start-up
}


/* NO layers, only sprites */
VIDEO_START( seta_no_layers )
{
	tilemap_0 = 0;
	tilemap_1 = 0;
	tilemap_2 = 0;
	tilemap_3 = 0;
	find_offsets(machine);
	seta_samples_bank = -1;	// set the samples bank to an out of range value at start-up
}

VIDEO_START( oisipuzl_2_layers )
{
	video_start_seta_2_layers(machine);
	tilemaps_flip = 1;
}


/***************************************************************************


                            Palette Init Functions


***************************************************************************/


/* 2 layers, 6 bit deep. The color codes have a 16 color granularity.

   One layer repeats every 16 colors to fill the 64 colors for the 6bpp gfx

   The other uses the first 64 colors of the palette regardless of
   the color code!
*/
PALETTE_INIT( blandia )
{
	int color, pen;
	for( color = 0; color < 32; color++ )
		for( pen = 0; pen < 64; pen++ )
		{
			colortable[color * 64 + pen + 16*32]       = (pen % 16) + color * 0x10 + 16*32*1;
			colortable[color * 64 + pen + 16*32+64*32] = pen        + 16*32*2;
		}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity,
   even if the low 2 bits are ignored (so there are only 4 different palettes) */
PALETTE_INIT( gundhara )
{
	int color, pen;
	for( color = 0; color < 32; color++ )
		for( pen = 0; pen < 64; pen++ )
		{
			colortable[color * 64 + pen + 32*16 + 32*64*0] = (((color&~3) * 16 + pen)%(32*16)) + 32*16*2;
			colortable[color * 64 + pen + 32*16 + 32*64*1] = (((color&~3) * 16 + pen)%(32*16)) + 32*16*1;
		}
}



/* layers have 6 bits per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT( jjsquawk )
{
	int color, pen;
	for( color = 0; color < 32; color++ )
		for( pen = 0; pen < 64; pen++ )
		{
			colortable[color * 64 + pen + 32*16 + 32*64*0] = ((color * 16 + pen)%(32*16)) + 32*16*2;
			colortable[color * 64 + pen + 32*16 + 32*64*1] = ((color * 16 + pen)%(32*16)) + 32*16*1;
		}
}


/* layer 0 is 6 bit per pixel, but the color code has a 16 colors granularity */
PALETTE_INIT( zingzip )
{
	int color, pen;
	for( color = 0; color < 32; color++ )
		for( pen = 0; pen < 64; pen++ )
			colortable[color * 64 + pen + 32*16*2] = ((color * 16 + pen)%(32*16)) + 32*16*2;
}




PALETTE_INIT( usclssic )
{
	int color, pen;
	int x;

	/* DECODE PROM */
	for (x = 0x000; x < 0x200 ; x++)
	{
		int data;

		data = (color_prom[x*2] <<8) | color_prom[x*2+1];

		if (x>=0x100) palette_set_color_rgb(machine,x,pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
		else palette_set_color_rgb(machine,x+0x300,pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
	}

	for( color = 0; color < 32; color++ )
		for( pen = 0; pen < 64; pen++ )
			colortable[color * 64 + pen + 512] =  ((((color & 0x1f) * 16 + pen)%512)+512);

}





/***************************************************************************


                                Sprites Drawing


***************************************************************************/


static void draw_sprites_map(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs, col;
	int xoffs, yoffs;

	int total_color_codes	=	machine->drv->gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	spriteram16[ 0x600/2 ];
	int ctrl2	=	spriteram16[ 0x602/2 ];

	int flip	=	ctrl & 0x40;
	int numcol	=	ctrl2 & 0x000f;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 *src = spriteram16_2 + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	int upper	=	( spriteram16[ 0x604/2 ] & 0xFF ) +
					( spriteram16[ 0x606/2 ] & 0xFF ) * 256;

	int max_y	=	0xf0;

	int col0;		/* Kludge, needed for krzybowl and kiwame */
	switch (ctrl & 0x0f)
	{
		case 0x01:	col0	=	0x4;	break;	// krzybowl
		case 0x06:	col0	=	0x8;	break;	// kiwame

		default:	col0	=	0x0;
	}

	xoffs = 0;
	yoffs = flip ? 1 : -1;

	/* Number of columns to draw - the value 1 seems special, meaning:
       draw every column */
	if (numcol == 1)
		numcol = 16;


	/* The first column is the frontmost, see twineagl test mode
        BM 071204 - first column frontmost breaks superman.
    */
//  for ( col = numcol - 1 ; col >= 0; col -- )
	for ( col = 0 ; col < numcol; col ++ )
	{
		int x	=	spriteram16[(col * 0x20 + 0x08 + 0x400)/2] & 0xff;
		int y	=	spriteram16[(col * 0x20 + 0x00 + 0x400)/2] & 0xff;

		/* draw this column */
		for ( offs = 0 ; offs < 0x40/2; offs += 2/2 )
		{
			int	code	=	src[((col+col0)&0xf) * 0x40/2 + offs + 0x800/2];
			int	color	=	src[((col+col0)&0xf) * 0x40/2 + offs + 0xc00/2];

			int	flipx	=	code & 0x8000;
			int	flipy	=	code & 0x4000;

			int bank	=	(color & 0x0600) >> 9;

/*
twineagl:   010 02d 0f 10   (ship)
tndrcade:   058 02d 07 18   (start of game - yes, flip on!)
arbalest:   018 02d 0f 10   (logo)
metafox :   018 021 0f f0   (bomb)
zingzip :   010 02c 00 0f   (bomb)
wrofaero:   010 021 00 ff   (test mode)
thunderl:   010 06c 00 ff   (always?)
krzybowl:   011 028 c0 ff   (game)
kiwame  :   016 021 7f 00   (logo)
oisipuzl:   059 020 00 00   (game - yes, flip on!)

superman:   010 021 07 38   (game)
twineagl:   000 027 00 0f   (test mode)
*/

			int sx		=	  x + xoffs  + (offs & 1) * 16;
			int sy		=	-(y + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col))	sx += 256;

			if (flip)
			{
				sy = max_y - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	=	( color >> (16-5) ) % total_color_codes;
			code	=	(code & 0x3fff) + (bank * 0x4000);

			drawgfx(bitmap,machine->gfx[0],
					code,
					color,
					flipx, flipy,
					((sx + 0x10) & 0x1ff) - 0x10,((sy + 8) & 0x0ff) - 8,
					cliprect,TRANSPARENCY_PEN,0);
		}
	/* next column */
	}

}



static void draw_sprites(running_machine *machine, mame_bitmap *bitmap,const rectangle *cliprect)
{
	int offs;
	int xoffs, yoffs;

	int total_color_codes	=	machine->drv->gfxdecodeinfo[0].total_color_codes;

	int ctrl	=	spriteram16[ 0x600/2 ];
	int ctrl2	=	spriteram16[ 0x602/2 ];

	int flip	=	ctrl & 0x40;

	/* Sprites Banking and/or Sprites Buffering */
	UINT16 *src = spriteram16_2 + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	int max_y	=	0xf0;


	draw_sprites_map(machine,bitmap,cliprect);


	xoffs = global_offsets->sprite_offs[flip ? 1 : 0];
	yoffs = -2;

	for ( offs = (0x400-2)/2 ; offs >= 0/2; offs -= 2/2 )
	{
		int	code	=	src[offs + 0x000/2];
		int	x		=	src[offs + 0x400/2];

		int	y		=	spriteram16[offs + 0x000/2] & 0xff;

		int	flipx	=	code & 0x8000;
		int	flipy	=	code & 0x4000;

		int bank	=	(x & 0x0600) >> 9;
		int color	=	( x >> (16-5) ) % total_color_codes;

		if (flip)
		{
			y = (0x100 - machine->screen[0].height) + max_y - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		code = (code & 0x3fff) + (bank * 0x4000);

		y = max_y - y;

		drawgfx(bitmap,machine->gfx[0],
				code,
				color,
				flipx, flipy,
				((x + xoffs + 0x10) & 0x1ff) - 0x10,((y - yoffs + 8) & 0x0ff) - 8,
				cliprect,TRANSPARENCY_PEN,0);
	}

}





/***************************************************************************


                                Screen Drawing


***************************************************************************/

/* For games without tilemaps */
VIDEO_UPDATE( seta_no_layers )
{
	fillbitmap(bitmap,machine->pens[0x1f0],cliprect);
	draw_sprites(machine,bitmap,cliprect);
	return 0;
}


/* For games with 1 or 2 tilemaps */
VIDEO_UPDATE( seta )
{
	int layers_ctrl = -1;
	int enab_0, enab_1, x_0, x_1, y_0, y_1;

	int order	= 	0;
	int flip	=	(spriteram16[ 0x600/2 ] & 0x40) >> 6;

	int vis_dimy = machine->screen[0].visarea.max_y - machine->screen[0].visarea.min_y + 1;

	flip ^= tilemaps_flip;

	tilemap_set_flip(ALL_TILEMAPS, flip ? (TILEMAP_FLIPX|TILEMAP_FLIPY) : 0 );

	x_0		=	seta_vctrl_0[ 0/2 ];
	y_0		=	seta_vctrl_0[ 2/2 ];
	enab_0	=	seta_vctrl_0[ 4/2 ];

	/* Only one tilemap per layer is enabled! */
	tilemap_set_enable(tilemap_0, (!(enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );
	tilemap_set_enable(tilemap_1, ( (enab_0 & 0x0008)) /*&& (enab_0 & 0x0001)*/ );

	/* the hardware wants different scroll values when flipped */

	/*  bg x scroll      flip
        metafox     0000 025d = 0, $400-$1a3 = $400 - $190 - $13
        eightfrc    ffe8 0272
                    fff0 0260 = -$10, $400-$190 -$10
                    ffe8 0272 = -$18, $400-$190 -$18 + $1a      */

	x_0 += 0x10 - global_offsets->tilemap_offs[flip ? 1 : 0];
	y_0 -= (256 - vis_dimy)/2;
	if (flip)
	{
		x_0 = -x_0 - 512;
		y_0 = y_0 - vis_dimy;
	}

	tilemap_set_scrollx (tilemap_0, 0, x_0);
	tilemap_set_scrollx (tilemap_1, 0, x_0);
	tilemap_set_scrolly (tilemap_0, 0, y_0);
	tilemap_set_scrolly (tilemap_1, 0, y_0);

	if (tilemap_2)
	{
		x_1		=	seta_vctrl_2[ 0/2 ];
		y_1		=	seta_vctrl_2[ 2/2 ];
		enab_1	=	seta_vctrl_2[ 4/2 ];

		tilemap_set_enable(tilemap_2, (!(enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );
		tilemap_set_enable(tilemap_3, ( (enab_1 & 0x0008)) /*&& (enab_1 & 0x0001)*/ );

		x_1 += 0x10 - global_offsets->tilemap_offs[flip ? 1 : 0];
		y_1 -= (256 - vis_dimy)/2;
		if (flip)
		{
			x_1 = -x_1 - 512;
			y_1 = y_1 - vis_dimy;
		}

		tilemap_set_scrollx (tilemap_2, 0, x_1);
		tilemap_set_scrollx (tilemap_3, 0, x_1);
		tilemap_set_scrolly (tilemap_2, 0, y_1);
		tilemap_set_scrolly (tilemap_3, 0, y_1);

		order	=	seta_vregs[ 2/2 ];
	}


#ifdef MAME_DEBUG
if (input_code_pressed(KEYCODE_Z))
{	int msk = 0;
	if (input_code_pressed(KEYCODE_Q))	msk |= 1;
	if (input_code_pressed(KEYCODE_W))	msk |= 2;
	if (input_code_pressed(KEYCODE_A))	msk |= 8;
	if (msk != 0) layers_ctrl &= msk;

	if (tilemap_2)		popmessage("VR:%04X-%04X-%04X L0:%04X L1:%04X",seta_vregs[0],seta_vregs[1],seta_vregs[2],seta_vctrl_0[4/2],seta_vctrl_2[4/2]);
	else if (tilemap_0)	popmessage("L0:%04X",seta_vctrl_0[4/2]);
}
#endif

	fillbitmap(bitmap,machine->pens[0],cliprect);

	if (order & 1)	// swap the layers?
	{
		if (tilemap_2)
		{
			if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_2, TILEMAP_DRAW_OPAQUE, 0);
			if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_3, TILEMAP_DRAW_OPAQUE, 0);
		}

		if (order & 2)	// layer-sprite priority?
		{
			if (layers_ctrl & 8)	draw_sprites(machine,bitmap,cliprect);
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_0,  0, 0);
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_1,  0, 0);
		}
		else
		{
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_0,  0, 0);
			if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_1,  0, 0);
			if (layers_ctrl & 8)	draw_sprites(machine, bitmap,cliprect);
		}
	}
	else
	{
		if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_0,  TILEMAP_DRAW_OPAQUE, 0);
		if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tilemap_1,  TILEMAP_DRAW_OPAQUE, 0);

		if (order & 2)	// layer-sprite priority?
		{
			if (layers_ctrl & 8)	draw_sprites(machine, bitmap,cliprect);

			if (tilemap_2)
			{
				if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_2,  0, 0);
				if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_3,  0, 0);
			}
		}
		else
		{
			if (tilemap_2)
			{
				if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_2,  0, 0);
				if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tilemap_3,  0, 0);
			}

			if (layers_ctrl & 8)	draw_sprites(machine, bitmap,cliprect);
		}
	}
	return 0;
}
