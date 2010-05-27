/***************************************************************************

  Functions to emulate the video hardware of some Toaplan games,
  which use one or more Toaplan L7A0498 GP9001 graphic controllers.

  The simpler hardware of these games use one GP9001 controller.
  Next we have games that use two GP9001 controllers, whose priority
  schemes between the two controllers is unknown at this time, and
  may be game dependant.
  Finally we have games using one GP9001 controller and an additional
  text tile layer, which has highest priority. This text tile layer
  appears to have line-scroll support. Some of these games copy the
  text tile gfx data to RAM from the main CPU ROM, which easily allows
  for effects to be added to the tiles, by manipulating the text tile
  gfx data. The tiles are then dynamically decoded from RAM before
  displaying them.


 To Do / Unknowns
    -  Hack is needed to reset sound CPU and sound chip when machine
        is 'tilted' in Pipi & Bibis. Otherwise sound CPU interferes
        with the main CPU test of shared RAM. You get a 'Sub CPU RAM Error'
    -  What do Scroll registers 0Eh and 0Fh really do ????
    -  Snow Bros 2 sets bit 6 of the sprite X info word during weather
        world map, and bits 4, 5 and 6 of the sprite X info word during
        the Rabbit boss screen - reasons are unknown.
    -  Fourth set of scroll registers have been used for Sprite scroll
        though it may not be correct. For most parts this looks right
        except for Snow Bros 2 when in the rabbit boss screen (all sprites
        jump when big green nasty (which is the foreground layer) comes
        in from the left)
    -  Teki Paki tests video RAM from address 0 past SpriteRAM to $37ff.
        This seems to be a bug in Teki Paki's vram test routine !
    -  Batsugun, relationship between the two video controllers (priority
        wise) is wrong and unknown.


 GP9001 Video RAM address layout:

    Bank          data size of video layer
    -----------------------------------------
    $0000-07FF    800h words for background layer
    $0800-0FFF    800h words for foreground layer
    $1000-17FF    800h words for top (text) layer
    $1800-1BFF    400h words for sprites (100 possible sprites)



 GP9001 Tile RAM format (each tile takes up 32 bits)

  0         1         2         3
  ---- ---- ---- ---- xxxx xxxx xxxx xxxx = Tile number (0 - FFFFh)
  ---- ---- -xxx xxxx ---- ---- ---- ---- = Color (0 - 7Fh)
  ---- ---- ?--- ---- ---- ---- ---- ---- = unknown / unused
  ---- xxxx ---- ---- ---- ---- ---- ---- = Priority (0 - Fh)
  ???? ---- ---- ---- ---- ---- ---- ---- = unknown / unused / possible flips

Sprites are of varying sizes between 8x8 and 128x128 with any variation
in between, in multiples of 8 either way.

Here we draw the first 8x8 part of the sprite, then by using the sprite
dimensions, we draw the rest of the 8x8 parts to produce the complete
sprite.

There seems to be sprite buffering - double buffering actually.

 GP9001 Sprite RAM format (data for each sprite takes up 4 words)

  0
  ---- ----  ---- --xx = top 2 bits of Sprite number
  ---- ----  xxxx xx-- = Color (0 - 3Fh)
  ---- xxxx  ---- ---- = Priority (0 - Fh)
  ---x ----  ---- ---- = Flip X
  --x- ----  ---- ---- = Flip Y
  -x-- ----  ---- ---- = Multi-sprite
  x--- ----  ---- ---- = Show sprite ?

  1
  xxxx xxxx  xxxx xxxx = Sprite number (top two bits in word 0)

  2
  ---- ----  ---- xxxx = Sprite X size (add 1, then multiply by 8)
  ---- ----  -??? ---- = unknown - used in Snow Bros. 2
  xxxx xxxx  x--- ---- = X position

  3
  ---- ----  ---- xxxx = Sprite Y size (add 1, then multiply by 8)
  ---- ----  -??? ---- = unknown / unused
  xxxx xxxx  x--- ---- = Y position


 Extra-text RAM format

 Truxton 2, Fixeight and Raizing games have an extra-text layer.

  Text RAM format      $0000-1FFF (actually its probably $0000-0FFF)
  ---- --xx xxxx xxxx = Tile number
  xxxx xx-- ---- ---- = Color (0 - 3Fh) + 40h

  Text flip / ???      $0000-01EF (some games go to $01FF (excess?))
  ---x xxxx xxxx xxxx = ??? line something (line to draw ?) ???
  x--- ---- ---- ---- = flip for the Text tile

  Text X line-scroll ? $0000-01EF (some games go to $01FF (excess?))
  ---- ---x xxxx xxxx = X-Scroll for each line



 GP9001 Scroll Registers (hex) :

    00      Background scroll X (X flip off)
    01      Background scroll Y (Y flip off)
    02      Foreground scroll X (X flip off)
    03      Foreground scroll Y (Y flip off)
    04      Top (text) scroll X (X flip off)
    05      Top (text) scroll Y (Y flip off)
    06      Sprites    scroll X (X flip off) ???
    07      Sprites    scroll Y (Y flip off) ???
    0E      ??? Initialise Video controller at startup ???
    0F      Scroll update complete ??? (Not used in Ghox and V-Five)

    80      Background scroll X (X flip on)
    81      Background scroll Y (Y flip on)
    82      Foreground scroll X (X flip on)
    83      Foreground scroll Y (Y flip on)
    84      Top (text) scroll X (X flip on)
    85      Top (text) scroll Y (Y flip on)
    86      Sprites    scroll X (X flip on) ???
    87      Sprites    scroll Y (Y flip on) ???
    8F      Same as 0Fh except flip bit is active


Scroll Register 0E writes (Video controller inits ?) from different games:

Teki-Paki        | Ghox             | Knuckle Bash     | Truxton 2        |
0003, 0002, 4000 | ????, ????, ???? | 0202, 0203, 4200 | 0003, 0002, 4000 |

Dogyuun          | Batsugun         |
0202, 0203, 4200 | 0202, 0203, 4200 |
1202, 1203, 5200 | 1202, 1203, 5200 | <--- Second video controller

Pipi & Bibis     | Fix Eight        | V-Five           | Snow Bros. 2     |
0003, 0002, 4000 | 0202, 0203, 4200 | 0202, 0203, 4200 | 0202, 0203, 4200 |

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/3812intf.h"
#include "includes/toaplan2.h"



#define TOAPLAN2_BG_VRAM_SIZE   0x1000	/* Background RAM size */
#define TOAPLAN2_FG_VRAM_SIZE   0x1000	/* Foreground RAM size */
#define TOAPLAN2_TOP_VRAM_SIZE  0x1000	/* Top Layer  RAM size */
#define TOAPLAN2_SPRITERAM_SIZE 0x800	/* Sprite     RAM size */
#define TOAPLAN2_UNUSEDRAM_SIZE 0x800	/* Unused     RAM size */
#define RAIZING_TX_GFXRAM_SIZE  0x8000	/* GFX data decode RAM size */
size_t toaplan2_tx_vram_size;		 /* 0x2000 Text layer RAM size */
size_t toaplan2_tx_offs_vram_size;	 /* 0x200 Text layer tile flip and positon ? */
size_t toaplan2_tx_scroll_vram_size; /* 0x200 Text layer scroll ? */
size_t batrider_paletteram16_size;



#define TOAPLAN2_SPRITE_FLIPX 0x1000	/* Sprite flip flags */
#define TOAPLAN2_SPRITE_FLIPY 0x2000

#define CPU_2_NONE		0x00
#define CPU_2_Z80		0x5a
#define CPU_2_HD647180	0xa5
#define CPU_2_V25		0xff


static UINT16 *bgvideoram16[2];
static UINT16 *fgvideoram16[2];
static UINT16 *topvideoram16[2];
static UINT16 *unusedvideoram16[2];
static UINT16 *spriteram16_now[2];	/* Sprites to draw this frame */
static UINT16 *spriteram16_new[2];	/* Sprites to add to next frame */
static UINT16 *spriteram16_n[2];
UINT16 *toaplan2_txvideoram16;		/* Video ram for extra text layer */
UINT16 *toaplan2_txvideoram16_offs;	/* Text layer tile flip and positon ? */
UINT16 *toaplan2_txscrollram16;		/* Text layer scroll ? */
UINT16 *toaplan2_tx_gfxram16;			/* Text Layer RAM based tiles */
static UINT16 *raizing_tx_gfxram16;			/* Text Layer RAM based tiles (Batrider) */

static UINT16 toaplan2_scroll_reg[2];
static UINT16 toaplan2_voffs[2];
static UINT16 bg_scrollx[2];
static UINT16 bg_scrolly[2];
static UINT16 fg_scrollx[2];
static UINT16 fg_scrolly[2];
static UINT16 top_scrollx[2];
static UINT16 top_scrolly[2];
static UINT16 sprite_scrollx[2];
static UINT16 sprite_scrolly[2];
static int objectbank_dirty = 0;		/* dirty flag of object bank (for Batrider) */
static UINT16 batrider_object_bank[8];		/* Batrider object bank */


static bitmap_t* toaplan2_custom_priority_bitmap;
static UINT16 tile_limit[2]; // prevent bad tile in Batsugun, might be something like the CPS1 tile addressing limits?
static int toaplan2_banked_gfx;


#ifdef MAME_DEBUG
static int display_bg[2];
static int display_fg[2];
static int display_top[2];
static int displog = 0;
static int display_tx;
#endif
static int display_sp[2];

static UINT8 bg_flip[2] = { 0, 0 };
static UINT8 fg_flip[2] = { 0, 0 };
static UINT8 top_flip[2] = { 0, 0 };
static UINT8 sprite_flip[2] = { 0, 0 };
static UINT8 tx_flip = 0;

static UINT8 sprite_priority[2][16];
static UINT8 top_tile_priority[2][16];
static UINT8 fg_tile_priority[2][16];
static UINT8 bg_tile_priority[2][16];


static tilemap_t *top_tilemap[2], *fg_tilemap[2], *bg_tilemap[2];
static tilemap_t *tx_tilemap;	/* Tilemap for extra-text-layer */

static int xoffset[4];
static int yoffset[4];

static void defaultOffsets(void)
{
		xoffset[0]=0;
		xoffset[1]=0;
		xoffset[2]=0;
		xoffset[3]=0;

		yoffset[0]=0;
		yoffset[1]=0;
		yoffset[2]=0;
		yoffset[3]=0;
}

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

static TILE_GET_INFO( get_top0_tile_info )
{
	int color, tile_number, attrib;

	attrib = topvideoram16[0][2*tile_index];

	tile_number = topvideoram16[0][2*tile_index+1];

	if (toaplan2_banked_gfx)
	{
		tile_number = ( batrider_object_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}
	else
	{
		if (tile_number>tile_limit[0])
		{
			tile_number = 0;
		}
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO( get_fg0_tile_info )
{
	int color, tile_number, attrib;

	attrib = fgvideoram16[0][2*tile_index];

	tile_number = fgvideoram16[0][2*tile_index+1];


	if (toaplan2_banked_gfx)
	{
		tile_number = ( batrider_object_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}
	else
	{
		if (tile_number>tile_limit[0]) tile_number = 0;
	}


	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO( get_bg0_tile_info )
{
	int color, tile_number, attrib;

	attrib = bgvideoram16[0][2*tile_index];

	tile_number = bgvideoram16[0][2*tile_index+1];

	if (toaplan2_banked_gfx)
	{
		tile_number = ( batrider_object_bank[(tile_number >> 13) & 7] << 13 ) | ( tile_number & 0x1fff );
	}
	else
	{
		if (tile_number>tile_limit[0]) tile_number = 0;
	}

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO( get_top1_tile_info )
{
	int color, tile_number, attrib;

	attrib = topvideoram16[1][2*tile_index];
	tile_number = topvideoram16[1][2*tile_index+1];

	if (tile_number>tile_limit[1]) tile_number = 0;

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO( get_fg1_tile_info )
{
	int color, tile_number, attrib;

	attrib = fgvideoram16[1][2*tile_index];
	tile_number = fgvideoram16[1][2*tile_index+1];

	if (tile_number>tile_limit[1]) tile_number = 0;

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}

static TILE_GET_INFO( get_bg1_tile_info )
{
	int color, tile_number, attrib;

	attrib = bgvideoram16[1][2*tile_index];
	tile_number = bgvideoram16[1][2*tile_index+1];

	if (tile_number>tile_limit[1]) tile_number = 0;

	color = attrib & 0x0fff; // 0x0f00 priority, 0x007f colour
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
	//tileinfo->category = (attrib & 0x0f00) >> 8;
}


static TILE_GET_INFO( get_text_tile_info )
{
	int color, tile_number, attrib;

	attrib = toaplan2_txvideoram16[tile_index];
	tile_number = attrib & 0x3ff;
	color = ((attrib >> 10) | 0x40) & 0x7f;
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
}

/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
static void create_tilemaps_0(running_machine *machine)
{
	top_tilemap[0] = tilemap_create(machine, get_top0_tile_info,tilemap_scan_rows,16,16,32,32);
	fg_tilemap[0] = tilemap_create(machine, get_fg0_tile_info,tilemap_scan_rows,16,16,32,32);
	bg_tilemap[0] = tilemap_create(machine, get_bg0_tile_info,tilemap_scan_rows,16,16,32,32);
	tile_limit[0] = 0xffff;

	tilemap_set_transparent_pen(top_tilemap[0],0);
	tilemap_set_transparent_pen(fg_tilemap[0],0);
	tilemap_set_transparent_pen(bg_tilemap[0],0);
}

static void create_tilemaps_1(running_machine *machine)
{
	top_tilemap[1] = tilemap_create(machine, get_top1_tile_info,tilemap_scan_rows,16,16,32,32);
	fg_tilemap[1] = tilemap_create(machine, get_fg1_tile_info,tilemap_scan_rows,16,16,32,32);
	bg_tilemap[1] = tilemap_create(machine, get_bg1_tile_info,tilemap_scan_rows,16,16,32,32);
	tile_limit[1] = 0xffff;

	tilemap_set_transparent_pen(top_tilemap[1],0);
	tilemap_set_transparent_pen(fg_tilemap[1],0);
	tilemap_set_transparent_pen(bg_tilemap[1],0);
}

static void truxton2_create_tilemaps_0(running_machine *machine)
{
	create_tilemaps_0(machine);

	tx_tilemap = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,64,32);
	tilemap_set_scroll_rows(tx_tilemap,8*32);	/* line scrolling */
	tilemap_set_scroll_cols(tx_tilemap,1);
	tilemap_set_transparent_pen(tx_tilemap,0);
}

static void batrider_create_tilemaps_0(running_machine *machine)
{
	create_tilemaps_0(machine);

	tx_tilemap = tilemap_create(machine, get_text_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_scroll_rows(tx_tilemap,8*32);	/* line scrolling */
	tilemap_set_scroll_cols(tx_tilemap,1);
	tilemap_set_transparent_pen(tx_tilemap,0);
}


static void toaplan2_vram_alloc(running_machine *machine, int controller)
{
	spriteram16_new[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_SPRITERAM_SIZE/2);
	spriteram16_now[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_SPRITERAM_SIZE/2);
	topvideoram16[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_TOP_VRAM_SIZE/2);
	fgvideoram16[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_FG_VRAM_SIZE/2);
	bgvideoram16[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_BG_VRAM_SIZE/2);
	unusedvideoram16[controller] = auto_alloc_array_clear(machine, UINT16, TOAPLAN2_UNUSEDRAM_SIZE/2);

	spriteram16_n[controller] = spriteram16_now[controller];
}

static void toaplan2_vh_start(running_machine *machine, int controller)
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	toaplan2_vram_alloc(machine, controller);

	toaplan2_custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);

	if (controller == 0)
	{
		create_tilemaps_0(machine);
	}
	if (controller == 1)
	{
		create_tilemaps_1(machine);
	}

#ifdef MAME_DEBUG
	display_bg[controller] = 1;
	display_fg[controller] = 1;
	display_top[controller] = 1;
	displog = 0;
	display_tx = 1;
#endif

	display_sp[controller] = 1;
}

static void register_state_save(running_machine *machine, int vrams)
{
	state_save_register_global_array(machine, toaplan2_scroll_reg);
	state_save_register_global_array(machine, toaplan2_voffs);
	state_save_register_global_array(machine, bg_scrollx);
	state_save_register_global_array(machine, bg_scrolly);
	state_save_register_global_array(machine, fg_scrollx);
	state_save_register_global_array(machine, fg_scrolly);
	state_save_register_global_array(machine, top_scrollx);
	state_save_register_global_array(machine, top_scrolly);
	state_save_register_global_array(machine, sprite_scrollx);
	state_save_register_global_array(machine, sprite_scrolly);
	state_save_register_global_array(machine, batrider_object_bank);

	state_save_register_global_array(machine, bg_flip);
	state_save_register_global_array(machine, fg_flip);
	state_save_register_global_array(machine, top_flip);
	state_save_register_global_array(machine, sprite_flip);
	state_save_register_global(machine, tx_flip);

	state_save_register_global_2d_array(machine, sprite_priority);
	state_save_register_global_2d_array(machine, top_tile_priority);
	state_save_register_global_2d_array(machine, fg_tile_priority);
	state_save_register_global_2d_array(machine, bg_tile_priority);

	switch (vrams)
	{
	case 2:
		state_save_register_global_pointer(machine, spriteram16_new[1], TOAPLAN2_SPRITERAM_SIZE/2);
		state_save_register_global_pointer(machine, spriteram16_now[1], TOAPLAN2_SPRITERAM_SIZE/2);
		state_save_register_global_pointer(machine, topvideoram16[1], TOAPLAN2_TOP_VRAM_SIZE/2);
		state_save_register_global_pointer(machine, fgvideoram16[1], TOAPLAN2_FG_VRAM_SIZE/2);
		state_save_register_global_pointer(machine, bgvideoram16[1], TOAPLAN2_BG_VRAM_SIZE/2);
		/* fall through */
	case 1:
		state_save_register_global_pointer(machine, spriteram16_new[0], TOAPLAN2_SPRITERAM_SIZE/2);
		state_save_register_global_pointer(machine, spriteram16_now[0], TOAPLAN2_SPRITERAM_SIZE/2);
		state_save_register_global_pointer(machine, topvideoram16[0], TOAPLAN2_TOP_VRAM_SIZE/2);
		state_save_register_global_pointer(machine, fgvideoram16[0], TOAPLAN2_FG_VRAM_SIZE/2);
		state_save_register_global_pointer(machine, bgvideoram16[0], TOAPLAN2_BG_VRAM_SIZE/2);
		/* fall through */
	default:
		break;
	}

}

VIDEO_START( toaplan2_0 )
{
	defaultOffsets();
	toaplan2_vh_start(machine, 0);
	register_state_save(machine,1);
	toaplan2_banked_gfx = 0;
}

VIDEO_START( toaplan2_1 )
{
	toaplan2_vh_start(machine, 0);
	toaplan2_vh_start(machine, 1);
	defaultOffsets();
	register_state_save(machine,2);
	toaplan2_banked_gfx = 0;
}

VIDEO_START( truxton2_0 )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	toaplan2_vram_alloc(machine, 0);
	truxton2_create_tilemaps_0(machine);

	toaplan2_custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);

	if (machine->gfx[2]->srcdata == NULL)
		gfx_element_set_source(machine->gfx[2], (UINT8 *)toaplan2_tx_gfxram16);

	if(!strcmp(machine->gamedrv->name,"fixeightb"))
	{
		xoffset[0]=-26;
		xoffset[1]=-22;
		xoffset[2]=-18;
		xoffset[3]=8;

		yoffset[0]=-15;
		yoffset[1]=-15;
		yoffset[2]=-15;
		yoffset[3]=8;
		tilemap_set_scrolldx(tx_tilemap, 0, 0);
	}
	else
	{
		defaultOffsets();
		tilemap_set_scrolldx(tx_tilemap, 0x1d4 +1, 0x2a);
	}

	display_sp[0] = 1;
	register_state_save(machine,1);
	toaplan2_banked_gfx = 0;
}

VIDEO_START( bgaregga_0 )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	toaplan2_custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);

	toaplan2_vram_alloc(machine, 0);
	truxton2_create_tilemaps_0(machine);
	tilemap_set_scrolldx(tx_tilemap, 0x1d4, 0x2a);
	defaultOffsets();
	display_sp[0] = 1;
	display_sp[1] = 1;
	register_state_save(machine,1);
	toaplan2_banked_gfx = 0;
}

VIDEO_START( batrider_0 )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);

	toaplan2_custom_priority_bitmap = auto_bitmap_alloc(machine, width, height, BITMAP_FORMAT_INDEXED8);


	raizing_tx_gfxram16 = auto_alloc_array_clear(machine, UINT16, RAIZING_TX_GFXRAM_SIZE/2);
	state_save_register_global_pointer(machine, raizing_tx_gfxram16, RAIZING_TX_GFXRAM_SIZE/2);

	gfx_element_set_source(machine->gfx[2], (UINT8 *)raizing_tx_gfxram16);

	toaplan2_vram_alloc(machine, 0);
	spriteram16_n[0] = spriteram16_new[0];

	batrider_create_tilemaps_0(machine);

	tilemap_set_scrolldx(tx_tilemap, 0x1d4, 0x2a);
	defaultOffsets();
	display_sp[0] = 1;
	display_sp[1] = 1;
	register_state_save(machine,1);
	toaplan2_banked_gfx = 1;
}


/***************************************************************************

  Video I/O port hardware.

***************************************************************************/

static void toaplan2_voffs_w(offs_t offset, UINT16 data, UINT16 mem_mask, int controller)
{
	if (data >= 0x1c00)
		logerror("Hmmm, unknown video controller %01x layer being selected (%08x)\n",controller,data);
	COMBINE_DATA(&toaplan2_voffs[controller]);
}

WRITE16_HANDLER( toaplan2_0_voffs_w )
{
	//printf("toaplan2_0_voffs_w %04x %04x\n",offset,data);

	toaplan2_voffs_w(offset, data, mem_mask, 0);
}

WRITE16_HANDLER( toaplan2_1_voffs_w )
{
	//printf("toaplan2_1_voffs_w %04x %04x\n",offset,data);

	toaplan2_voffs_w(offset, data, mem_mask, 1);
}

READ16_HANDLER( toaplan2_txvideoram16_r )
{
	return toaplan2_txvideoram16[offset];
}

WRITE16_HANDLER( toaplan2_txvideoram16_w )
{
	COMBINE_DATA(&toaplan2_txvideoram16[offset]);
	if (offset < (toaplan2_tx_vram_size/4))
		tilemap_mark_tile_dirty(tx_tilemap,offset);
}

READ16_HANDLER( toaplan2_txvideoram16_offs_r )
{
	return toaplan2_txvideoram16_offs[offset];
}
WRITE16_HANDLER( toaplan2_txvideoram16_offs_w )
{
	/* Besides containing flip, function of this RAM is still unknown */
	/* This is however line related as per line-scroll RAM below */
	/* Maybe specifies which line to draw text info (line number data is */
	/*   opposite when flip bits are on) */

	UINT16 oldword = toaplan2_txvideoram16_offs[offset];

	if (oldword != data)
	{
		if (offset == 0)			/* Wrong ! */
		{
			if (data & 0x8000)		/* Flip off */
			{
				tx_flip = 0;
				tilemap_set_flip(tx_tilemap, tx_flip);
				tilemap_set_scrolly(tx_tilemap, 0, 0);
			}
			else					/* Flip on */
			{
				tx_flip = (TILEMAP_FLIPY | TILEMAP_FLIPX);
				tilemap_set_flip(tx_tilemap, tx_flip);
				tilemap_set_scrolly(tx_tilemap, 0, -16);
			}
		}
		COMBINE_DATA(&toaplan2_txvideoram16_offs[offset]);
	}
//  logerror("Writing %04x to text offs RAM offset %04x\n",data,offset);
}

READ16_HANDLER( toaplan2_txscrollram16_r )
{
	return toaplan2_txscrollram16[offset];
}
WRITE16_HANDLER( toaplan2_txscrollram16_w )
{
	/*** Line-Scroll RAM for Text Layer ***/

	int data_tx = data;

	tilemap_set_scrollx(tx_tilemap, offset, data_tx);

//  logerror("Writing %04x to text scroll RAM offset %04x\n",data,offset);
	COMBINE_DATA(&toaplan2_txscrollram16[offset]);
}

READ16_HANDLER( toaplan2_tx_gfxram16_r )
{
	return toaplan2_tx_gfxram16[offset];
}

WRITE16_HANDLER( toaplan2_tx_gfxram16_w )
{
	/*** Dynamic GFX decoding for Truxton 2 / FixEight ***/

	UINT16 oldword = toaplan2_tx_gfxram16[offset];

	if (oldword != data)
	{
		int code = offset/32;
		COMBINE_DATA(&toaplan2_tx_gfxram16[offset]);
		gfx_element_mark_dirty(space->machine->gfx[2], code);
	}
}

READ16_HANDLER( raizing_tx_gfxram16_r )
{
	offset += 0x3400/2;
	return raizing_tx_gfxram16[offset];
}
WRITE16_HANDLER( raizing_tx_gfxram16_w )
{
	/*** Dynamic Text GFX decoding for Batrider ***/

	UINT16 oldword = raizing_tx_gfxram16[offset + (0x3400 / 2)];

	if (oldword != data)
	{
		offset += 0x3400/2;
		COMBINE_DATA(&raizing_tx_gfxram16[offset]);
	}
}

WRITE16_HANDLER( batrider_textdata_decode )
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/

	int code;
	UINT16 *dest = (UINT16 *)raizing_tx_gfxram16;

	memcpy(dest, toaplan2_txvideoram16, toaplan2_tx_vram_size);
	dest += (toaplan2_tx_vram_size/2);
	memcpy(dest, space->machine->generic.paletteram.u16, batrider_paletteram16_size);
	dest += (batrider_paletteram16_size/2);
	memcpy(dest, toaplan2_txvideoram16_offs, toaplan2_tx_offs_vram_size);
	dest += (toaplan2_tx_offs_vram_size/2);
	memcpy(dest, toaplan2_txscrollram16, toaplan2_tx_scroll_vram_size);

	/* Decode text characters; force them to update immediately */
	for (code = 0; code < 1024; code++)
		gfx_element_decode(space->machine->gfx[2], code);
}

WRITE16_HANDLER( batrider_objectbank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		data &= 0xf;
		if (batrider_object_bank[offset] != data)
		{
			batrider_object_bank[offset] = data;
			objectbank_dirty = 1;
		}
	}
}



static int toaplan2_videoram16_r(offs_t offset, int controller)
{
	int offs = (toaplan2_voffs[controller] &0x1fff);
	toaplan2_voffs[controller]++;

	if (offs<0x0800)
	{
		offs&=0x7ff;
		return bgvideoram16[controller][offs];
	}
	else if (offs<0x1000)
	{
		offs&=0x7ff;
		return fgvideoram16[controller][offs];
	}
	else if (offs<0x1800)
	{
		offs&=0x7ff;
		return topvideoram16[controller][offs];
	}
	else if (offs<0x1c00)
	{
		offs&=0x3ff;
		return spriteram16_new[controller][offs];
	}
	else // wouldn't surprise me if this mirrors spriteram on real hw
	{
		offs&=0x3ff;
		return unusedvideoram16[controller][offs];
	}
}

READ16_HANDLER( toaplan2_0_videoram16_r )
{
	return toaplan2_videoram16_r(offset, 0);
}

READ16_HANDLER( toaplan2_1_videoram16_r )
{
	return toaplan2_videoram16_r(offset, 1);
}

static void toaplan2_videoram16_w(offs_t offset, UINT16 data, UINT16 mem_mask, int controller)
{
	int offs = (toaplan2_voffs[controller] &0x1fff);
	toaplan2_voffs[controller]++;

	if (offs<0x0800)
	{
		offs&=0x7ff;
		COMBINE_DATA(&bgvideoram16[controller][offs]);
		tilemap_mark_tile_dirty(bg_tilemap[controller],offs/2);
	}
	else if (offs<0x1000)
	{
		offs&=0x7ff;
		COMBINE_DATA(&fgvideoram16[controller][offs]);
		tilemap_mark_tile_dirty(fg_tilemap[controller],offs/2);
	}
	else if (offs<0x1800)
	{
		offs&=0x7ff;
		COMBINE_DATA(&topvideoram16[controller][offs]);
		tilemap_mark_tile_dirty(top_tilemap[controller],offs/2);
	}
	else if (offs<0x1c00)
	{
		offs&=0x3ff;
		COMBINE_DATA(&spriteram16_new[controller][offs]);
	}
	else // wouldn't surprise me if this mirrors spriteram on real hw
	{
		offs&=0x3ff;
		COMBINE_DATA(&unusedvideoram16[controller][offs]);
	}
}

WRITE16_HANDLER( toaplan2_0_videoram16_w )
{
	toaplan2_videoram16_w(offset, data, mem_mask, 0);
}

WRITE16_HANDLER( toaplan2_1_videoram16_w )
{
	toaplan2_videoram16_w(offset, data, mem_mask, 1);
}


static void toaplan2_scroll_reg_select_w(offs_t offset, UINT16 data, UINT16 mem_mask, int controller)
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan2_scroll_reg[controller] = data & 0x8f;
		if (data & 0x70)
			logerror("Hmmm, selecting unknown LSB video control register (%04x)  Video controller %01x  \n",toaplan2_scroll_reg[controller],controller);
	}
	else
	{
		logerror("Hmmm, selecting unknown MSB video control register (%04x)  Video controller %01x  \n",toaplan2_scroll_reg[controller],controller);
	}
}

WRITE16_HANDLER( toaplan2_0_scroll_reg_select_w )
{
//  printf("toaplan2_scroll_reg_select_w %04x %04x\n",offset,data);

	toaplan2_scroll_reg_select_w(offset, data, mem_mask, 0);
}

WRITE16_HANDLER( toaplan2_1_scroll_reg_select_w )
{
//  printf("toaplan2_scroll_1_reg_select_w %04x %04x\n",offset,data);

	toaplan2_scroll_reg_select_w(offset, data, mem_mask, 1);
}


static void toaplan2_scroll_reg_data_w(running_machine *machine, offs_t offset, UINT16 data, UINT16 mem_mask, int controller)
{
	/************************************************************************/
	/***** layer X and Y flips can be set independantly, so emulate it ******/
	/************************************************************************/

	//printf("toaplan2_scroll_reg_data_w %04x %04x\n", offset, data);
#ifdef MAME_DEBUG
	int vid_controllers = 1;
#endif

	switch(toaplan2_scroll_reg[controller])
	{
		case 0x00:	data -= 0x1d6;			/* 1D6h */
					COMBINE_DATA(&bg_scrollx[controller]);
					bg_flip[controller] &= (~TILEMAP_FLIPX);
					tilemap_set_flip(bg_tilemap[controller],bg_flip[controller]);
					tilemap_set_scrollx(bg_tilemap[controller],0,bg_scrollx[controller]+xoffset[0]);
					break;
		case 0x01:	data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&bg_scrolly[controller]);
					bg_flip[controller] &= (~TILEMAP_FLIPY);
					tilemap_set_flip(bg_tilemap[controller],bg_flip[controller]);
					tilemap_set_scrolly(bg_tilemap[controller],0,bg_scrolly[controller]+yoffset[0]);
					break;
		case 0x02:	data -= 0x1d8;			/* 1D0h */
					COMBINE_DATA(&fg_scrollx[controller]);
					fg_flip[controller] &= (~TILEMAP_FLIPX);
					tilemap_set_flip(fg_tilemap[controller],fg_flip[controller]);
					tilemap_set_scrollx(fg_tilemap[controller],0,fg_scrollx[controller]+xoffset[1]);
					break;
		case 0x03:  data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&fg_scrolly[controller]);
					fg_flip[controller] &= (~TILEMAP_FLIPY);
					tilemap_set_flip(fg_tilemap[controller],fg_flip[controller]);
					tilemap_set_scrolly(fg_tilemap[controller],0,fg_scrolly[controller]+yoffset[1]);
					break;
		case 0x04:	data -= 0x1da;			/* 1DAh */
					COMBINE_DATA(&top_scrollx[controller]);
					top_flip[controller] &= (~TILEMAP_FLIPX);
					tilemap_set_flip(top_tilemap[controller],top_flip[controller]);
					tilemap_set_scrollx(top_tilemap[controller],0,top_scrollx[controller]+xoffset[2]);
					break;
		case 0x05:	data -= 0x1ef;			/* 1EFh */
					COMBINE_DATA(&top_scrolly[controller]);
					top_flip[controller] &= (~TILEMAP_FLIPY);
					tilemap_set_flip(top_tilemap[controller],top_flip[controller]);
					tilemap_set_scrolly(top_tilemap[controller],0,top_scrolly[controller]+yoffset[2]);
					break;
		case 0x06:  data -= 0x1cc;			/* 1D4h */
					COMBINE_DATA(&sprite_scrollx[controller]);
					if (sprite_scrollx[controller] & 0x8000) sprite_scrollx[controller] |= 0xfffffe00;
					else sprite_scrollx[controller] &= 0x1ff;
					sprite_flip[controller] &= (~TOAPLAN2_SPRITE_FLIPX);
					break;
		case 0x07:	data -= 0x1ef;      /* 1F7h */
					COMBINE_DATA(&sprite_scrolly[controller]);
					if (sprite_scrolly[controller] & 0x8000) sprite_scrolly[controller] |= 0xfffffe00;
					else sprite_scrolly[controller] &= 0x1ff;
					sprite_flip[controller] &= (~TOAPLAN2_SPRITE_FLIPY);
					break;
		case 0x0f:	break;
		case 0x80:  data -= 0x229;			/* 169h */
					COMBINE_DATA(&bg_scrollx[controller]);
					bg_flip[controller] |= TILEMAP_FLIPX;
					tilemap_set_flip(bg_tilemap[controller],bg_flip[controller]);
					tilemap_set_scrollx(bg_tilemap[controller],0,bg_scrollx[controller]+xoffset[0]);
					break;
		case 0x81:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&bg_scrolly[controller]);
					bg_flip[controller] |= TILEMAP_FLIPY;
					tilemap_set_flip(bg_tilemap[controller],bg_flip[controller]);
					tilemap_set_scrolly(bg_tilemap[controller],0,bg_scrolly[controller]+yoffset[0]);
					break;
		case 0x82:	data -= 0x227;			/* 15Fh */
					COMBINE_DATA(&fg_scrollx[controller]);
					fg_flip[controller] |= TILEMAP_FLIPX;
					tilemap_set_flip(fg_tilemap[controller],fg_flip[controller]);
					tilemap_set_scrollx(fg_tilemap[controller],0,fg_scrollx[controller]+xoffset[1]);
					break;
		case 0x83:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&fg_scrolly[controller]);
					fg_flip[controller] |= TILEMAP_FLIPY;
					tilemap_set_flip(fg_tilemap[controller],fg_flip[controller]);
					tilemap_set_scrolly(fg_tilemap[controller],0,fg_scrolly[controller]+yoffset[1]);
					break;
		case 0x84:	data -= 0x225;			/* 165h */
					COMBINE_DATA(&top_scrollx[controller]);
					top_flip[controller] |= TILEMAP_FLIPX;
					tilemap_set_flip(top_tilemap[controller],top_flip[controller]);
					tilemap_set_scrollx(top_tilemap[controller],0,top_scrollx[controller]+xoffset[2]);
					break;
		case 0x85:	data -= 0x210;			/* 100h */
					COMBINE_DATA(&top_scrolly[controller]);
					top_flip[controller] |= TILEMAP_FLIPY;
					tilemap_set_flip(top_tilemap[controller],top_flip[controller]);
					tilemap_set_scrolly(top_tilemap[controller],0,top_scrolly[controller]+yoffset[2]);
					break;
		case 0x86:	data -= 0x17b;			/* 17Bh */
					COMBINE_DATA(&sprite_scrollx[controller]);
					if (sprite_scrollx[controller] & 0x8000) sprite_scrollx[controller] |= 0xfffffe00;
					else sprite_scrollx[controller] &= 0x1ff;
					sprite_flip[controller] |= TOAPLAN2_SPRITE_FLIPX;
					break;
		case 0x87:	data -= 0x108;			/* 108h */
					COMBINE_DATA(&sprite_scrolly[controller]);
					if (sprite_scrolly[controller] & 0x8000) sprite_scrolly[controller] |= 0xfffffe00;
					else sprite_scrolly[controller] &= 0x1ff;
					sprite_flip[controller] |= TOAPLAN2_SPRITE_FLIPY;
					break;
		case 0x8f:	break;

		case 0x0e:	/******* Initialise video controller register ? *******/
					if ((toaplan2_sub_cpu == CPU_2_Z80) && (data == 3))
					{
						/* HACK! When tilted, sound CPU needs to be reset. */
						running_device *ym = devtag_get_device(machine, "ymsnd");

						if (ym && (sound_get_type(ym) == SOUND_YM3812))
						{
							cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, PULSE_LINE);
							devtag_reset(machine, "ymsnd");
						}
					}

		default:	logerror("Hmmm, writing %08x to unknown video control register (%08x)  Video controller %01x  !!!\n",data ,toaplan2_scroll_reg[controller],controller);
					break;
	}

#ifdef MAME_DEBUG

	if (spriteram16_now[1] && spriteram16_new[1]
		&& top_tilemap[1] && fg_tilemap[1] && bg_tilemap[1])
	{
		vid_controllers = 2;
	}

	if ( input_code_pressed_once(machine, KEYCODE_W) )
	{
		display_tx += 1;
		display_tx &= 1;
		if (toaplan2_txvideoram16 != 0)
			tilemap_set_enable(tx_tilemap, display_tx);
	}
	if ( input_code_pressed_once(machine, KEYCODE_L) )
	{
		display_sp[0] += 1;
		display_sp[0] &= 1;
	}
	if ( input_code_pressed_once(machine, KEYCODE_K) )
	{
		display_top[0] += 1;
		display_top[0] &= 1;
		tilemap_set_enable(top_tilemap[0], display_top[0]);
	}
	if ( input_code_pressed_once(machine, KEYCODE_J) )
	{
		display_fg[0] += 1;
		display_fg[0] &= 1;
		tilemap_set_enable(fg_tilemap[0], display_fg[0]);
	}
	if ( input_code_pressed_once(machine, KEYCODE_H) )
	{
		display_bg[0] += 1;
		display_bg[0] &= 1;
		tilemap_set_enable(bg_tilemap[0], display_bg[0]);
	}
	if (vid_controllers == 2)
	{
		if ( input_code_pressed_once(machine, KEYCODE_O) )
		{
			display_sp[1] += 1;
			display_sp[1] &= 1;
		}
		if ( input_code_pressed_once(machine, KEYCODE_I) )
		{
			display_top[1] += 1;
			display_top[1] &= 1;
			tilemap_set_enable(top_tilemap[1], display_top[1]);
		}
		if ( input_code_pressed_once(machine, KEYCODE_U) )
		{
			display_fg[1] += 1;
			display_fg[1] &= 1;
			tilemap_set_enable(fg_tilemap[1], display_fg[1]);
		}
		if ( input_code_pressed_once(machine, KEYCODE_Y) )
		{
			display_bg[1] += 1;
			display_bg[1] &= 1;
			tilemap_set_enable(bg_tilemap[1], display_bg[1]);
		}
	}
#endif
}

WRITE16_HANDLER( toaplan2_0_scroll_reg_data_w )
{
	toaplan2_scroll_reg_data_w(space->machine, offset, data, mem_mask, 0);
}

WRITE16_HANDLER( toaplan2_1_scroll_reg_data_w )
{
	toaplan2_scroll_reg_data_w(space->machine, offset, data, mem_mask, 1);
}


/***************************************************************************/
/**************** PIPIBIBI interface into this video driver ****************/

WRITE16_HANDLER( pipibibi_scroll_w )
{
	if (ACCESSING_BITS_8_15 && ACCESSING_BITS_0_7)
	{
		switch(offset)
		{
			case 0x00:	data -= 0x01f; break;
			case 0x01:	data += 0x1ef; break;
			case 0x02:	data -= 0x01d; break;
			case 0x03:	data += 0x1ef; break;
			case 0x04:	data -= 0x01b; break;
			case 0x05:	data += 0x1ef; break;
			case 0x06:	data += 0x1d4; break;
			case 0x07:	data += 0x1f7; break;
			default:	logerror("PIPIBIBI writing %04x to unknown scroll register %04x",data, offset);
		}

		toaplan2_scroll_reg[0] = offset;
		toaplan2_scroll_reg_data_w(space->machine, offset, data, mem_mask, 0);
	}
}

READ16_HANDLER( pipibibi_videoram16_r )
{
	toaplan2_voffs_w(0, offset, 0xffff, 0);
	return toaplan2_videoram16_r(0, 0);
}

WRITE16_HANDLER( pipibibi_videoram16_w)
{
	toaplan2_voffs_w(0, offset, 0xffff, 0);
	toaplan2_videoram16_w(0, data, mem_mask, 0);
}

READ16_HANDLER( pipibibi_spriteram16_r )
{
	toaplan2_voffs_w(0, (0x1800 + offset), 0, 0);
	return toaplan2_videoram16_r(0, 0);
}

WRITE16_HANDLER( pipibibi_spriteram16_w )
{
	toaplan2_voffs_w(0, (0x1800 + offset), mem_mask, 0);
	toaplan2_videoram16_w(0, data, mem_mask, 0);
}



static void toaplan2_log_vram(running_machine *machine)
{
#ifdef MAME_DEBUG
	offs_t sprite_voffs, tile_voffs;
	int vid_controllers = 1;

	if (spriteram16_now[1] && spriteram16_new[1]
		&& top_tilemap[1] && fg_tilemap[1] && bg_tilemap[1])
	{
		vid_controllers = 2;
	}

	if ( input_code_pressed_once(machine, KEYCODE_M) )
	{
		UINT16 *source_now0  = (UINT16 *)(spriteram16_now[0]);
		UINT16 *source_new0  = (UINT16 *)(spriteram16_new[0]);
		UINT16 *source_now1  = (UINT16 *)(spriteram16_now[0]);
		UINT16 *source_new1  = (UINT16 *)(spriteram16_new[0]);

		int schar[2],sattr[2],sxpos[2],sypos[2];

		if (vid_controllers == 2)
		{
			source_now1  = (UINT16 *)(spriteram16_now[1]);
			source_new1  = (UINT16 *)(spriteram16_new[1]);
		}

		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
		logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[0],bg_scrolly[0],fg_scrollx[0],fg_scrolly[0],top_scrollx[0],top_scrolly[0],sprite_scrollx[0], sprite_scrolly[0]);
		if (vid_controllers == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[1],bg_scrolly[1],fg_scrollx[1],fg_scrolly[1],top_scrollx[1],top_scrolly[1],sprite_scrollx[1], sprite_scrolly[1]);
		}
		for ( sprite_voffs = 0; sprite_voffs < (TOAPLAN2_SPRITERAM_SIZE/2); sprite_voffs += 4 )
		{
			sattr[0] = source_now0[sprite_voffs];
			schar[0] = source_now0[sprite_voffs + 1];
			sxpos[0] = source_now0[sprite_voffs + 2];
			sypos[0] = source_now0[sprite_voffs + 3];
			sattr[1] = source_new0[sprite_voffs];
			schar[1] = source_new0[sprite_voffs + 1];
			sxpos[1] = source_new0[sprite_voffs + 2];
			sypos[1] = source_new0[sprite_voffs + 3];
			logerror("SPoffs    Sprt Attr Xpos Ypos     Sprt Attr Xpos Ypos\n");
			logerror("0:%03x now:%04x %04x %04x %04x new:%04x %04x %04x %04x\n",sprite_voffs,
												schar[0], sattr[0],sxpos[0], sypos[0],
												schar[1], sattr[1],sxpos[1], sypos[1]);
			if (vid_controllers == 2)
			{
				sattr[0] = source_now1[sprite_voffs];
				schar[0] = source_now1[sprite_voffs + 1];
				sxpos[0] = source_now1[sprite_voffs + 2];
				sypos[0] = source_now1[sprite_voffs + 3];
				sattr[1] = source_new1[sprite_voffs];
				schar[1] = source_new1[sprite_voffs + 1];
				sxpos[1] = source_new1[sprite_voffs + 2];
				sypos[1] = source_new1[sprite_voffs + 3];
				logerror("1:%03x now:%04x %04x %04x %04x new:%04x %04x %04x %04x\n",sprite_voffs,
												schar[0], sattr[0],sxpos[0], sypos[0],
												schar[1], sattr[1],sxpos[1], sypos[1]);
			}
		}
	}
	if ( input_code_pressed_once(machine, KEYCODE_N) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
		logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[0],bg_scrolly[0],fg_scrollx[0],fg_scrolly[0],top_scrollx[0],top_scrolly[0],sprite_scrollx[0], sprite_scrolly[0]);
		if (vid_controllers == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[1],bg_scrolly[1],fg_scrollx[1],fg_scrolly[1],top_scrollx[1],top_scrolly[1],sprite_scrollx[1], sprite_scrolly[1]);
		}
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN2_TOP_VRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[0] = topvideoram16[0][tile_voffs + 1];
			tattr[0] = topvideoram16[0][tile_voffs];
			if (vid_controllers == 2)
			{
				tchar[1] = topvideoram16[1][tile_voffs + 1];
				tattr[1] = topvideoram16[1][tile_voffs];
				logerror("TOPoffs:%04x   Tile0:%04x  Attr0:%04x    Tile1:%04x  Attr1:%04x\n", tile_voffs, tchar[0], tattr[0], tchar[1], tattr[1]);
			}
			else
			{
				logerror("TOPoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}
		}
	}
	if ( input_code_pressed_once(machine, KEYCODE_B) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
		logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[0],bg_scrolly[0],fg_scrollx[0],fg_scrolly[0],top_scrollx[0],top_scrolly[0],sprite_scrollx[0], sprite_scrolly[0]);
		if (vid_controllers == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[1],bg_scrolly[1],fg_scrollx[1],fg_scrolly[1],top_scrollx[1],top_scrolly[1],sprite_scrollx[1], sprite_scrolly[1]);
		}
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN2_FG_VRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[0] = fgvideoram16[0][tile_voffs + 1];
			tattr[0] = fgvideoram16[0][tile_voffs];
		if (vid_controllers == 2)
			{
				tchar[1] = fgvideoram16[1][tile_voffs + 1];
				tattr[1] = fgvideoram16[1][tile_voffs];
				logerror("FGoffs:%04x   Tile0:%04x  Attr0:%04x    Tile1:%04x  Attr1:%04x\n", tile_voffs, tchar[0], tattr[0], tchar[1], tattr[1]);
			}
			else
			{
				logerror("FGoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}
		}
	}
	if ( input_code_pressed_once(machine, KEYCODE_V) )
	{
		int tchar[2], tattr[2];
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
		logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[0],bg_scrolly[0],fg_scrollx[0],fg_scrolly[0],top_scrollx[0],top_scrolly[0],sprite_scrollx[0], sprite_scrolly[0]);
		if (vid_controllers == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[1],bg_scrolly[1],fg_scrollx[1],fg_scrolly[1],top_scrollx[1],top_scrolly[1],sprite_scrollx[1], sprite_scrolly[1]);
		}
		for ( tile_voffs = 0; tile_voffs < (TOAPLAN2_BG_VRAM_SIZE/2); tile_voffs += 2 )
		{
			tchar[0] = bgvideoram16[0][tile_voffs + 1];
			tattr[0] = bgvideoram16[0][tile_voffs];
			if (vid_controllers == 2)
			{
				tchar[1] = bgvideoram16[1][tile_voffs + 1];
				tattr[1] = bgvideoram16[1][tile_voffs];
				logerror("BGoffs:%04x   Tile0:%04x  Attr0:%04x    Tile1:%04x  Attr1:%04x\n", tile_voffs, tchar[0], tattr[0], tchar[1], tattr[1]);
			}
			else
			{
				logerror("BGoffs:%04x   Tile0:%04x  Attr0:%04x\n", tile_voffs, tchar[0], tattr[0]);
			}
		}
	}

	if ( input_code_pressed_once(machine, KEYCODE_C) )
		logerror("Mark here\n");

	if ( input_code_pressed_once(machine, KEYCODE_E) )
	{
		displog += 1;
		displog &= 1;
	}
	if (displog)
	{
		logerror("Scrolls   BG-X  BG-Y   FG-X  FG-Y   TOP-X  TOP-Y   Sprite-X  Sprite-Y\n");
		logerror("---0-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[0],bg_scrolly[0],fg_scrollx[0],fg_scrolly[0],top_scrollx[0],top_scrolly[0],sprite_scrollx[0], sprite_scrolly[0]);
		if (vid_controllers == 2)
		{
			logerror("---1-->   %04x  %04x   %04x  %04x    %04x  %04x       %04x    %04x\n", bg_scrollx[1],bg_scrolly[1],fg_scrollx[1],fg_scrolly[1],top_scrollx[1],top_scrolly[1],sprite_scrollx[1], sprite_scrolly[1]);
		}
	}
#endif
}



/***************************************************************************
    Sprite Handlers
***************************************************************************/

static void draw_sprites( running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int controller, UINT8* primap )
{
	const gfx_element *gfx = machine->gfx[ ((controller*2)+1) ];

	int offs, old_x, old_y;

	UINT16 *source = (UINT16 *)(spriteram16_n[controller]);

	old_x = (-(sprite_scrollx[controller]+xoffset[3])) & 0x1ff;
	old_y = (-(sprite_scrolly[controller]+yoffset[3])) & 0x1ff;

	for (offs = 0; offs < (TOAPLAN2_SPRITERAM_SIZE/2); offs += 4)
	{
		int attrib, sprite, color, priority, flipx, flipy, sx, sy;
		int sprite_sizex, sprite_sizey, dim_x, dim_y, sx_base, sy_base;
		int bank, sprite_num;

		attrib = source[offs];
		priority = primap[((attrib & 0x0f00)>>8)]+1+controller*0x40;

		if ((attrib & 0x8000))
		{
			if (!toaplan2_banked_gfx)	/* No Sprite select bank switching needed */
			{
				sprite = ((attrib & 3) << 16) | source[offs + 1];	/* 18 bit */
			}
			else		/* Batrider Sprite select bank switching required */
			{
				sprite_num = source[offs + 1] & 0x7fff;
				bank = ((attrib & 3) << 1) | (source[offs + 1] >> 15);
				sprite = (batrider_object_bank[bank] << 15 ) | sprite_num;
			}
			color = (attrib >> 2) & 0x3f;

			/***** find out sprite size *****/
			sprite_sizex = ((source[offs + 2] & 0x0f) + 1) * 8;
			sprite_sizey = ((source[offs + 3] & 0x0f) + 1) * 8;

			/***** find position to display sprite *****/
			if (!(attrib & 0x4000))
			{
				sx_base = ((source[offs + 2] >> 7) - (sprite_scrollx[controller]+xoffset[3])) & 0x1ff;
				sy_base = ((source[offs + 3] >> 7) - (sprite_scrolly[controller]+yoffset[3])) & 0x1ff;
			} else {
				sx_base = (old_x + (source[offs + 2] >> 7)) & 0x1ff;
				sy_base = (old_y + (source[offs + 3] >> 7)) & 0x1ff;
			}

			old_x = sx_base;
			old_y = sy_base;

			flipx = attrib & TOAPLAN2_SPRITE_FLIPX;
			flipy = attrib & TOAPLAN2_SPRITE_FLIPY;

			if (flipx)
			{
				/***** Wrap sprite position around *****/
				sx_base -= 7;
				if (sx_base >= 0x1c0) sx_base -= 0x200;
			}
			else
			{
				if (sx_base >= 0x180) sx_base -= 0x200;
			}

			if (flipy)
			{
				sy_base -= 7;
				if (sy_base >= 0x1c0) sy_base -= 0x200;
			}
			else
			{
				if (sy_base >= 0x180) sy_base -= 0x200;
			}

			/***** Flip the sprite layer in any active X or Y flip *****/
			if (sprite_flip[controller])
			{
				if (sprite_flip[controller] & TOAPLAN2_SPRITE_FLIPX)
					sx_base = 320 - sx_base;
				if (sprite_flip[controller] & TOAPLAN2_SPRITE_FLIPY)
					sy_base = 240 - sy_base;
			}

			/***** Cancel flip, if it, and sprite layer flip are active *****/
			flipx = (flipx ^ (sprite_flip[controller] & TOAPLAN2_SPRITE_FLIPX));
			flipy = (flipy ^ (sprite_flip[controller] & TOAPLAN2_SPRITE_FLIPY));

			/***** Draw the complete sprites using the dimension info *****/
			for (dim_y = 0; dim_y < sprite_sizey; dim_y += 8)
			{
				if (flipy) sy = sy_base - dim_y;
				else       sy = sy_base + dim_y;
				for (dim_x = 0; dim_x < sprite_sizex; dim_x += 8)
				{
					if (flipx) sx = sx_base - dim_x;
					else       sx = sx_base + dim_x;

					/*
                    drawgfx_transpen(bitmap,cliprect,gfx,sprite,
                        color,
                        flipx,flipy,
                        sx,sy,0);
                    */
					sprite %= gfx->total_elements;
					color %= gfx->total_colors;

					{
						int yy, xx;
						const pen_t *paldata = &gfx->machine->pens[gfx->color_base + gfx->color_granularity * color];
						const UINT8* srcdata = gfx_element_get_data(gfx, sprite);
						int count = 0;
						int ystart, yend, yinc;
						int xstart, xend, xinc;

						if (flipy)
						{
							ystart = 7;
							yend = -1;
							yinc = -1;
						}
						else
						{
							ystart = 0;
							yend = 8;
							yinc = 1;
						}

						if (flipx)
						{
							xstart = 7;
							xend = -1;
							xinc = -1;
						}
						else
						{
							xstart = 0;
							xend = 8;
							xinc = 1;
						}

						for (yy=ystart;yy!=yend;yy+=yinc)
						{
							int drawyy = yy+sy;


							for (xx=xstart;xx!=xend;xx+=xinc)
							{
								int drawxx = xx+sx;

								if (drawxx>=cliprect->min_x && drawxx<cliprect->max_x && drawyy>=cliprect->min_y && drawyy<cliprect->max_y)
								{
									UINT8 pix = srcdata[count];
									UINT16* dstptr = BITMAP_ADDR16(bitmap,drawyy,drawxx);
									UINT8* dstpri = BITMAP_ADDR8(toaplan2_custom_priority_bitmap, drawyy, drawxx);

									if (priority >= dstpri[0])
									{
										if (pix&0xf)
										{
											dstptr[0] = paldata[pix];
											dstpri[0] = priority;

										}
									}
								}


								count++;
							}
						}


					}



					sprite++ ;
				}
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_t.
***************************************************************************/

void toaplan2_draw_custom_tilemap(running_machine* machine, bitmap_t* bitmap, tilemap_t* tilemap, UINT8* priremap, UINT8* pri_enable )
{
	int width = video_screen_get_width(machine->primary_screen);
	int height = video_screen_get_height(machine->primary_screen);
	int y,x;
	bitmap_t *tmb = tilemap_get_pixmap(tilemap);
	UINT16* srcptr;
	UINT16* dstptr;
	UINT8* dstpriptr;

	int scrollx = tilemap_get_scrollx(tilemap, 0);
	int scrolly = tilemap_get_scrolly(tilemap, 0);

	for (y=0;y<height;y++)
	{
		int realy = (y+scrolly)&0x1ff;

		srcptr = BITMAP_ADDR16(tmb, realy, 0);
		dstptr = BITMAP_ADDR16(bitmap, y, 0);
		dstpriptr = BITMAP_ADDR8(toaplan2_custom_priority_bitmap, y, 0);

		for (x=0;x<width;x++)
		{
			int realx = (x+scrollx)&0x1ff;

			UINT16 pixdat = srcptr[realx];
			UINT8 pixpri = ((pixdat & 0xf000)>>12);

			if (pri_enable[pixpri])
			{
				pixpri = priremap[pixpri]+1; // priority of 0 isn't desireable
				pixdat &=0x07ff;

				if (pixdat&0xf)
				{
					if (pixpri >= dstpriptr[x])
					{
						dstptr[x] = pixdat;
						dstpriptr[x] = pixpri;
					}
				}
			}
		}
	}
}


UINT8 toaplan2_primap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };
//UINT8 toaplan2_sprprimap1[16] =  { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
UINT8 toaplan2_sprprimap1[16] =  { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c };

UINT8 batsugun_prienable0[16]={ 1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1 };


VIDEO_UPDATE( toaplan2_0 )
{
	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(toaplan2_custom_priority_bitmap, cliprect, 0);

	toaplan2_draw_custom_tilemap( screen->machine, bitmap, bg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, fg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, top_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	draw_sprites(screen->machine,bitmap,cliprect,0, toaplan2_sprprimap1);

	return 0;
}


/* How do the dual VDP games mix? The internal mixing of each VDP chip is independent, if you view only a single
   VDP then the priorities for that VDP are correct, however, it is completely unclear how the priorities of the
   two VDPs should actually mix together, as a result these games are broken for now. */
VIDEO_UPDATE( dogyuun_1 )
{
	toaplan2_log_vram(screen->machine);

	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(toaplan2_custom_priority_bitmap, cliprect, 0);

	toaplan2_draw_custom_tilemap( screen->machine, bitmap, bg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, fg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, top_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	draw_sprites(screen->machine,bitmap,cliprect,0, toaplan2_sprprimap1);

	toaplan2_draw_custom_tilemap( screen->machine, bitmap, top_tilemap[1], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, fg_tilemap[1], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, bg_tilemap[1], toaplan2_primap1, batsugun_prienable0); // priority 0 of this MUST be below priority 0 of bg_tilemap[0] for lev 3, but above for lev 2?!
	draw_sprites(screen->machine,bitmap,cliprect,1, toaplan2_sprprimap1);


	return 0;
}


VIDEO_UPDATE( batsugun_1 )
{

	toaplan2_log_vram(screen->machine);

	bitmap_fill(bitmap,cliprect,0);
	bitmap_fill(toaplan2_custom_priority_bitmap, cliprect, 0);

	tile_limit[1] = 0x1fff; // 0x2000-0x3fff seem to be for sprites only? (corruption on level 1 otherwise)

	toaplan2_draw_custom_tilemap( screen->machine, bitmap, bg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, fg_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, top_tilemap[0], toaplan2_primap1, batsugun_prienable0);
	draw_sprites(screen->machine,bitmap,cliprect,0, toaplan2_sprprimap1);

	toaplan2_draw_custom_tilemap( screen->machine, bitmap, top_tilemap[1], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, fg_tilemap[1], toaplan2_primap1, batsugun_prienable0);
	toaplan2_draw_custom_tilemap( screen->machine, bitmap, bg_tilemap[1], toaplan2_primap1, batsugun_prienable0); // priority 0 of this MUST be below priority 0 of bg_tilemap[0] for lev 3, but above for lev 2?!
	draw_sprites(screen->machine,bitmap,cliprect,1, toaplan2_sprprimap1);

	return 0;
}

VIDEO_UPDATE( truxton2_0 )
{
	VIDEO_UPDATE_CALL(toaplan2_0);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}

VIDEO_UPDATE( batrider_0 )
{
	int line;
	rectangle clip;
	const rectangle *visarea = video_screen_get_visible_area(screen);

	toaplan2_log_vram(screen->machine);

	/* If object bank is changed, all tile must be redrawn to blow off glitches. */
	/* This causes serious slow down. Is there better algorithm ?                */
	if (objectbank_dirty)
	{
		tilemap_mark_all_tiles_dirty(bg_tilemap[0]);
		tilemap_mark_all_tiles_dirty(fg_tilemap[0]);
		objectbank_dirty = 0;
	}

	VIDEO_UPDATE_CALL( toaplan2_0 );

	clip.min_x = visarea->min_x;
	clip.max_x = visarea->max_x;
	clip.min_y = visarea->min_y;
	clip.max_y = visarea->max_y;

	/* used for 'for use in' and '8ing' screen on bbakraid, raizing on batrider */
	for (line = 0; line < 256;line++)
	{
		clip.min_y = clip.max_y = line;
		tilemap_set_scrolly(tx_tilemap,0,toaplan2_txvideoram16_offs[line&0xff]-line);
		tilemap_draw(bitmap,&clip,tx_tilemap,0,0);
	}
	return 0;
}


VIDEO_UPDATE( mahoudai_0 )
{
	toaplan2_log_vram(screen->machine);

	VIDEO_UPDATE_CALL( toaplan2_0 );

	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	return 0;
}


VIDEO_EOF( toaplan2_0 )
{
	/** Shift sprite RAM buffers  ***  Used to fix sprite lag **/
	memcpy(spriteram16_now[0],spriteram16_new[0],TOAPLAN2_SPRITERAM_SIZE);
}

VIDEO_EOF( toaplan2_1 )
{
	/** Shift sprite RAM buffers  ***  Used to fix sprite lag **/
	memcpy(spriteram16_now[0],spriteram16_new[0],TOAPLAN2_SPRITERAM_SIZE);
	memcpy(spriteram16_now[1],spriteram16_new[1],TOAPLAN2_SPRITERAM_SIZE);
}
