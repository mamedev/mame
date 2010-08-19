/***************************************************************************
  video.c

  Functions to emulate the video hardware of these machines.
  Video is 40x30 tiles. (320x240 for Twin Cobra/Flying shark)
  Video is 40x30 tiles. (320x240 for Wardner)

  Video has 3 scrolling tile layers (Background, Foreground and Text) and
  Sprites that have 4 priorities. Lowest priority is "Off".
  Wardner has an unusual sprite priority in the shop scenes, whereby a
  middle level priority Sprite appears over a high priority Sprite ?

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "includes/twincobr.h"


static STATE_POSTLOAD( twincobr_restore_screen );

INT32 twincobr_fg_rom_bank;
INT32 twincobr_bg_ram_bank;
INT32 wardner_sprite_hack;	/* Required for weird sprite priority in wardner */
							/* when hero is in shop. Hero should cover shop owner */

static UINT16 *twincobr_bgvideoram16;
static UINT16 *twincobr_fgvideoram16;
static UINT16 *twincobr_txvideoram16;

static size_t twincobr_bgvideoram_size;
static size_t twincobr_fgvideoram_size;
static size_t twincobr_txvideoram_size;

static INT32 txscrollx;
static INT32 txscrolly;
static INT32 fgscrollx;
static INT32 fgscrolly;
static INT32 bgscrollx;
static INT32 bgscrolly;
static INT32 txoffs;
static INT32 fgoffs;
static INT32 bgoffs;
static INT32 scroll_x;
static INT32 scroll_y;
static INT32 twincobr_display_on;
static INT32 twincobr_flip_screen;

static tilemap_t *bg_tilemap, *fg_tilemap, *tx_tilemap;


/* 6845 used for video sync signals only */
const mc6845_interface twincobr_mc6845_intf =
{
	"screen",	/* screen we are acting on */
	2,			/* number of pixels per video memory address */ /* Horizontal Display programmed to 160 characters */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};



/***************************************************************************
    Callbacks for the TileMap code
***************************************************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	int code, tile_number, color;

	code = twincobr_bgvideoram16[tile_index+twincobr_bg_ram_bank];
	tile_number = code & 0x0fff;
	color = (code & 0xf000) >> 12;
	SET_TILE_INFO(
			2,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_fg_tile_info )
{
	int code, tile_number, color;

	code = twincobr_fgvideoram16[tile_index];
	tile_number = (code & 0x0fff) | twincobr_fg_rom_bank;
	color = (code & 0xf000) >> 12;
	SET_TILE_INFO(
			1,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	int code, tile_number, color;

	code = twincobr_txvideoram16[tile_index];
	tile_number = code & 0x07ff;
	color = (code & 0xf800) >> 11;
	SET_TILE_INFO(
			0,
			tile_number,
			color,
			0);
}

/***************************************************************************
    Start the video hardware emulation.
***************************************************************************/

static void twincobr_create_tilemaps(running_machine *machine)
{
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,64,64);
	tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0);
	tilemap_set_transparent_pen(tx_tilemap,0);
}

VIDEO_START( toaplan0 )
{
	/* the video RAM is accessed via ports, it's not memory mapped */
	twincobr_txvideoram_size = 0x0800;
	twincobr_bgvideoram_size = 0x2000;	/* banked two times 0x1000 */
	twincobr_fgvideoram_size = 0x1000;

	twincobr_create_tilemaps(machine);

	twincobr_txvideoram16 = auto_alloc_array_clear(machine, UINT16, twincobr_txvideoram_size);
	twincobr_fgvideoram16 = auto_alloc_array_clear(machine, UINT16, twincobr_fgvideoram_size);
	twincobr_bgvideoram16 = auto_alloc_array_clear(machine, UINT16, twincobr_bgvideoram_size);

	twincobr_display_on = 0;
	twincobr_display(twincobr_display_on);

	state_save_register_global_pointer(machine, twincobr_txvideoram16, twincobr_txvideoram_size);
	state_save_register_global_pointer(machine, twincobr_fgvideoram16, twincobr_fgvideoram_size);
	state_save_register_global_pointer(machine, twincobr_bgvideoram16, twincobr_bgvideoram_size);
	state_save_register_global(machine, txoffs);
	state_save_register_global(machine, fgoffs);
	state_save_register_global(machine, bgoffs);
	state_save_register_global(machine, scroll_x);
	state_save_register_global(machine, scroll_y);
	state_save_register_global(machine, txscrollx);
	state_save_register_global(machine, fgscrollx);
	state_save_register_global(machine, bgscrollx);
	state_save_register_global(machine, txscrolly);
	state_save_register_global(machine, fgscrolly);
	state_save_register_global(machine, bgscrolly);
	state_save_register_global(machine, twincobr_display_on);
	state_save_register_global(machine, twincobr_fg_rom_bank);
	state_save_register_global(machine, twincobr_bg_ram_bank);
	state_save_register_global(machine, twincobr_flip_screen);
	state_save_register_global(machine, wardner_sprite_hack);
	state_save_register_postload(machine, twincobr_restore_screen, NULL);
}

static STATE_POSTLOAD( twincobr_restore_screen )
{
	twincobr_display(twincobr_display_on);
	twincobr_flipscreen(machine, twincobr_flip_screen);
}


/***************************************************************************
    Video I/O interface
***************************************************************************/

void twincobr_display(int enable)
{
	twincobr_display_on = enable;
	tilemap_set_enable(bg_tilemap, enable);
	tilemap_set_enable(fg_tilemap, enable);
	tilemap_set_enable(tx_tilemap, enable);
}

void twincobr_flipscreen(running_machine *machine, int flip)
{
	tilemap_set_flip_all(machine, (flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
	twincobr_flip_screen = flip;
	if (flip) {
		scroll_x = 0x008;
		scroll_y = 0x0c5;
	}
	else {
		scroll_x = 0x037;
		scroll_y = 0x01e;
	}
}


WRITE16_HANDLER( twincobr_txoffs_w )
{
	COMBINE_DATA(&txoffs);
	txoffs %= twincobr_txvideoram_size;
}
READ16_HANDLER( twincobr_txram_r )
{
	return twincobr_txvideoram16[txoffs];
}
WRITE16_HANDLER( twincobr_txram_w )
{
	COMBINE_DATA(&twincobr_txvideoram16[txoffs]);
	tilemap_mark_tile_dirty(tx_tilemap,txoffs);
}

WRITE16_HANDLER( twincobr_bgoffs_w )
{
	COMBINE_DATA(&bgoffs);
	bgoffs %= (twincobr_bgvideoram_size >> 1);
}
READ16_HANDLER( twincobr_bgram_r )
{
	return twincobr_bgvideoram16[bgoffs+twincobr_bg_ram_bank];
}
WRITE16_HANDLER( twincobr_bgram_w )
{
	COMBINE_DATA(&twincobr_bgvideoram16[bgoffs+twincobr_bg_ram_bank]);
	tilemap_mark_tile_dirty(bg_tilemap,(bgoffs+twincobr_bg_ram_bank));
}

WRITE16_HANDLER( twincobr_fgoffs_w )
{
	COMBINE_DATA(&fgoffs);
	fgoffs %= twincobr_fgvideoram_size;
}
READ16_HANDLER( twincobr_fgram_r )
{
	return twincobr_fgvideoram16[fgoffs];
}
WRITE16_HANDLER( twincobr_fgram_w )
{
	COMBINE_DATA(&twincobr_fgvideoram16[fgoffs]);
	tilemap_mark_tile_dirty(fg_tilemap,fgoffs);
}


WRITE16_HANDLER( twincobr_txscroll_w )
{
	if (offset == 0) {
		COMBINE_DATA(&txscrollx);
		tilemap_set_scrollx(tx_tilemap,0,(txscrollx+scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&txscrolly);
		tilemap_set_scrolly(tx_tilemap,0,(txscrolly+scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_bgscroll_w )
{
	if (offset == 0) {
		COMBINE_DATA(&bgscrollx);
		tilemap_set_scrollx(bg_tilemap,0,(bgscrollx+scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&bgscrolly);
		tilemap_set_scrolly(bg_tilemap,0,(bgscrolly+scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_fgscroll_w )
{
	if (offset == 0) {
		COMBINE_DATA(&fgscrollx);
		tilemap_set_scrollx(fg_tilemap,0,(fgscrollx+scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&fgscrolly);
		tilemap_set_scrolly(fg_tilemap,0,(fgscrolly+scroll_y) & 0x1ff);
	}
}

WRITE16_HANDLER( twincobr_exscroll_w )	/* Extra unused video layer */
{
	if (offset == 0) logerror("PC - write %04x to unknown video scroll Y register\n",data);
	else logerror("PC - write %04x to unknown video scroll X register\n",data);
}

/******************** Wardner interface to this hardware ********************/
WRITE8_HANDLER( wardner_txlayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_txoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_bglayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_bgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_fglayer_w )
{
	int shift = 8 * (offset & 1);
	twincobr_fgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_txscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_txscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_bgscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_bgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_fgscroll_w )
{
	int shift = 8 * (offset & 1);
	twincobr_fgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_HANDLER( wardner_exscroll_w )	/* Extra unused video layer */
{
	switch (offset)
	{
		case 01:	//data <<= 8;
		case 00:	logerror("PC - write %04x to unknown video scroll X register\n",data); break;
		case 03:	//data <<= 8;
		case 02:	logerror("PC - write %04x to unknown video scroll Y register\n",data); break;
	}
}

READ8_HANDLER( wardner_videoram_r )
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: return twincobr_txram_r(space,0,0xffff) >> shift;
		case 1: return twincobr_bgram_r(space,0,0xffff) >> shift;
		case 2: return twincobr_fgram_r(space,0,0xffff) >> shift;
	}
	return 0;
}

WRITE8_HANDLER( wardner_videoram_w )
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: twincobr_txram_w(space,0,data << shift, 0xff << shift); break;
		case 1: twincobr_bgram_w(space,0,data << shift, 0xff << shift); break;
		case 2: twincobr_fgram_w(space,0,data << shift, 0xff << shift); break;
	}
}

READ8_HANDLER( wardner_sprite_r )
{
	int shift = (offset & 1) * 8;
	return space->machine->generic.spriteram.u16[offset/2] >> shift;
}

WRITE8_HANDLER( wardner_sprite_w )
{
	UINT16 *spriteram16 = space->machine->generic.spriteram.u16;
	if (offset & 1)
		spriteram16[offset/2] = (spriteram16[offset/2] & 0x00ff) | (data << 8);
	else
		spriteram16[offset/2] = (spriteram16[offset/2] & 0xff00) | data;
}



/***************************************************************************
    Ugly sprite hack for Wardner when hero is in shop
***************************************************************************/

static void wardner_sprite_priority_hack(running_machine *machine)
{
	if (fgscrollx != bgscrollx) {
		UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
		if ((fgscrollx==0x1c9) || (twincobr_flip_screen && (fgscrollx==0x17a))) {	/* in the shop ? */
			int wardner_hack = buffered_spriteram16[0x0b04/2];
		/* sprite position 0x6300 to 0x8700 -- hero on shop keeper (normal) */
		/* sprite position 0x3900 to 0x5e00 -- hero on shop keeper (flip) */
			if ((wardner_hack > 0x3900) && (wardner_hack < 0x8700)) {	/* hero at shop keeper ? */
				wardner_hack = buffered_spriteram16[0x0b02/2];
				wardner_hack |= 0x0400;			/* make hero top priority */
				buffered_spriteram16[0x0b02/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b0a/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b0a/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b12/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b12/2] = wardner_hack;
				wardner_hack = buffered_spriteram16[0x0b1a/2];
				wardner_hack |= 0x0400;
				buffered_spriteram16[0x0b1a/2] = wardner_hack;
			}
		}
	}
}



static void twincobr_log_vram(running_machine *machine)
{
#ifdef MAME_DEBUG
	if ( input_code_pressed(machine, KEYCODE_M) )
	{
		offs_t tile_voffs;
		int tcode[4];
		while (input_code_pressed(machine, KEYCODE_M)) ;
		logerror("Scrolls             BG-X BG-Y  FG-X FG-Y  TX-X  TX-Y\n");
		logerror("------>             %04x %04x  %04x %04x  %04x  %04x\n",bgscrollx,bgscrolly,fgscrollx,fgscrolly,txscrollx,txscrolly);
		for ( tile_voffs = 0; tile_voffs < (twincobr_txvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = twincobr_bgvideoram16[tile_voffs];
			tcode[2] = twincobr_fgvideoram16[tile_voffs];
			tcode[3] = twincobr_txvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x  TX1:%02x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff,
							tcode[3] & 0xf800 >> 11, tcode[3] & 0x07ff);
		}
		for ( tile_voffs = (twincobr_txvideoram_size/2); tile_voffs < (twincobr_fgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = twincobr_bgvideoram16[tile_voffs];
			tcode[2] = twincobr_fgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff);
		}
		for ( tile_voffs = (twincobr_fgvideoram_size/2); tile_voffs < (twincobr_bgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = twincobr_bgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff);
		}
	}
#endif
}


/***************************************************************************
    Sprite Handlers
***************************************************************************/

static void draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int priority )
{
	int offs;

	if (twincobr_display_on)
	{
		UINT16 *buffered_spriteram16 = machine->generic.buffered_spriteram.u16;
		for (offs = 0;offs < machine->generic.spriteram_size/2;offs += 4)
		{
			int attribute,sx,sy,flipx,flipy;
			int sprite, color;

			attribute = buffered_spriteram16[offs + 1];
			if ((attribute & 0x0c00) == priority) {	/* low priority */
				sy = buffered_spriteram16[offs + 3] >> 7;
				if (sy != 0x0100) {		/* sx = 0x01a0 or 0x0040*/
					sprite = buffered_spriteram16[offs] & 0x7ff;
					color  = attribute & 0x3f;
					sx = buffered_spriteram16[offs + 2] >> 7;
					flipx = attribute & 0x100;
					if (flipx) sx -= 14;		/* should really be 15 */
					flipy = attribute & 0x200;
					drawgfx_transpen(bitmap,cliprect,machine->gfx[3],
						sprite,
						color,
						flipx,flipy,
						sx-32,sy-16,0);
				}
			}
		}
	}
}


/***************************************************************************
    Draw the game screen in the given bitmap_t.
***************************************************************************/

VIDEO_UPDATE( toaplan0 )
{
	twincobr_log_vram(screen->machine);

	if (wardner_sprite_hack) wardner_sprite_priority_hack(screen->machine);

	bitmap_fill(bitmap,cliprect,0);

	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen->machine, bitmap,cliprect,0x0400);
	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect,0x0800);
	tilemap_draw(bitmap,cliprect,tx_tilemap,0,0);
	draw_sprites(screen->machine, bitmap,cliprect,0x0c00);
	return 0;
}


VIDEO_EOF( toaplan0 )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	/* Spriteram is always 1 frame ahead, suggesting spriteram buffering.
        There are no CPU output registers that control this so we
        assume it happens automatically every frame, at the end of vblank */
	buffer_spriteram16_w(space,0,0,0xffff);
}
