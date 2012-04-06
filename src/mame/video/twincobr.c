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


static void twincobr_restore_screen(running_machine &machine);

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
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_bgvideoram16[tile_index+state->m_bg_ram_bank];
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
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_fgvideoram16[tile_index];
	tile_number = (code & 0x0fff) | state->m_fg_rom_bank;
	color = (code & 0xf000) >> 12;
	SET_TILE_INFO(
			1,
			tile_number,
			color,
			0);
}

static TILE_GET_INFO( get_tx_tile_info )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int code, tile_number, color;

	code = state->m_txvideoram16[tile_index];
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

static void twincobr_create_tilemaps(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,64);
	state->m_fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,64,64);
	state->m_tx_tilemap = tilemap_create(machine, get_tx_tile_info,tilemap_scan_rows,8,8,64,32);

	state->m_fg_tilemap->set_transparent_pen(0);
	state->m_tx_tilemap->set_transparent_pen(0);
}

VIDEO_START( toaplan0 )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	/* the video RAM is accessed via ports, it's not memory mapped */
	state->m_txvideoram_size = 0x0800;
	state->m_bgvideoram_size = 0x2000;	/* banked two times 0x1000 */
	state->m_fgvideoram_size = 0x1000;

	twincobr_create_tilemaps(machine);

	state->m_txvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_txvideoram_size);
	state->m_fgvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_fgvideoram_size);
	state->m_bgvideoram16 = auto_alloc_array_clear(machine, UINT16, state->m_bgvideoram_size);

	state->m_display_on = 0;
	twincobr_display(machine, state->m_display_on);

	state_save_register_global_pointer(machine, state->m_txvideoram16, state->m_txvideoram_size);
	state_save_register_global_pointer(machine, state->m_fgvideoram16, state->m_fgvideoram_size);
	state_save_register_global_pointer(machine, state->m_bgvideoram16, state->m_bgvideoram_size);
	state_save_register_global(machine, state->m_txoffs);
	state_save_register_global(machine, state->m_fgoffs);
	state_save_register_global(machine, state->m_bgoffs);
	state_save_register_global(machine, state->m_scroll_x);
	state_save_register_global(machine, state->m_scroll_y);
	state_save_register_global(machine, state->m_txscrollx);
	state_save_register_global(machine, state->m_fgscrollx);
	state_save_register_global(machine, state->m_bgscrollx);
	state_save_register_global(machine, state->m_txscrolly);
	state_save_register_global(machine, state->m_fgscrolly);
	state_save_register_global(machine, state->m_bgscrolly);
	state_save_register_global(machine, state->m_display_on);
	state_save_register_global(machine, state->m_fg_rom_bank);
	state_save_register_global(machine, state->m_bg_ram_bank);
	state_save_register_global(machine, state->m_flip_screen);
	state_save_register_global(machine, state->m_wardner_sprite_hack);
	machine.save().register_postload(save_prepost_delegate(FUNC(twincobr_restore_screen), &machine));
}

static void twincobr_restore_screen(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	twincobr_display(machine, state->m_display_on);
	twincobr_flipscreen(machine, state->m_flip_screen);
}


/***************************************************************************
    Video I/O interface
***************************************************************************/

void twincobr_display(running_machine &machine, int enable)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	state->m_display_on = enable;
	state->m_bg_tilemap->enable(enable);
	state->m_fg_tilemap->enable(enable);
	state->m_tx_tilemap->enable(enable);
}

void twincobr_flipscreen(running_machine &machine, int flip)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	machine.tilemap().set_flip_all((flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0));
	state->m_flip_screen = flip;
	if (flip) {
		state->m_scroll_x = 0x008;
		state->m_scroll_y = 0x0c5;
	}
	else {
		state->m_scroll_x = 0x037;
		state->m_scroll_y = 0x01e;
	}
}


WRITE16_MEMBER(twincobr_state::twincobr_txoffs_w)
{

	COMBINE_DATA(&m_txoffs);
	m_txoffs %= m_txvideoram_size;
}
READ16_MEMBER(twincobr_state::twincobr_txram_r)
{

	return m_txvideoram16[m_txoffs];
}
WRITE16_MEMBER(twincobr_state::twincobr_txram_w)
{

	COMBINE_DATA(&m_txvideoram16[m_txoffs]);
	m_tx_tilemap->mark_tile_dirty(m_txoffs);
}

WRITE16_MEMBER(twincobr_state::twincobr_bgoffs_w)
{

	COMBINE_DATA(&m_bgoffs);
	m_bgoffs %= (m_bgvideoram_size >> 1);
}
READ16_MEMBER(twincobr_state::twincobr_bgram_r)
{

	return m_bgvideoram16[m_bgoffs+m_bg_ram_bank];
}
WRITE16_MEMBER(twincobr_state::twincobr_bgram_w)
{

	COMBINE_DATA(&m_bgvideoram16[m_bgoffs+m_bg_ram_bank]);
	m_bg_tilemap->mark_tile_dirty((m_bgoffs+m_bg_ram_bank));
}

WRITE16_MEMBER(twincobr_state::twincobr_fgoffs_w)
{

	COMBINE_DATA(&m_fgoffs);
	m_fgoffs %= m_fgvideoram_size;
}
READ16_MEMBER(twincobr_state::twincobr_fgram_r)
{

	return m_fgvideoram16[m_fgoffs];
}
WRITE16_MEMBER(twincobr_state::twincobr_fgram_w)
{

	COMBINE_DATA(&m_fgvideoram16[m_fgoffs]);
	m_fg_tilemap->mark_tile_dirty(m_fgoffs);
}


WRITE16_MEMBER(twincobr_state::twincobr_txscroll_w)
{

	if (offset == 0) {
		COMBINE_DATA(&m_txscrollx);
		m_tx_tilemap->set_scrollx(0,(m_txscrollx+m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&m_txscrolly);
		m_tx_tilemap->set_scrolly(0,(m_txscrolly+m_scroll_y) & 0x1ff);
	}
}

WRITE16_MEMBER(twincobr_state::twincobr_bgscroll_w)
{

	if (offset == 0) {
		COMBINE_DATA(&m_bgscrollx);
		m_bg_tilemap->set_scrollx(0,(m_bgscrollx+m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&m_bgscrolly);
		m_bg_tilemap->set_scrolly(0,(m_bgscrolly+m_scroll_y) & 0x1ff);
	}
}

WRITE16_MEMBER(twincobr_state::twincobr_fgscroll_w)
{

	if (offset == 0) {
		COMBINE_DATA(&m_fgscrollx);
		m_fg_tilemap->set_scrollx(0,(m_fgscrollx+m_scroll_x) & 0x1ff);
	}
	else {
		COMBINE_DATA(&m_fgscrolly);
		m_fg_tilemap->set_scrolly(0,(m_fgscrolly+m_scroll_y) & 0x1ff);
	}
}

WRITE16_MEMBER(twincobr_state::twincobr_exscroll_w)/* Extra unused video layer */
{
	if (offset == 0) logerror("PC - write %04x to unknown video scroll Y register\n",data);
	else logerror("PC - write %04x to unknown video scroll X register\n",data);
}

/******************** Wardner interface to this hardware ********************/
WRITE8_MEMBER(twincobr_state::wardner_txlayer_w)
{
	int shift = 8 * (offset & 1);
	twincobr_txoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_bglayer_w)
{
	int shift = 8 * (offset & 1);
	twincobr_bgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_fglayer_w)
{
	int shift = 8 * (offset & 1);
	twincobr_fgoffs_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_txscroll_w)
{
	int shift = 8 * (offset & 1);
	twincobr_txscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_bgscroll_w)
{
	int shift = 8 * (offset & 1);
	twincobr_bgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_fgscroll_w)
{
	int shift = 8 * (offset & 1);
	twincobr_fgscroll_w(space, offset / 2, data << shift, 0xff << shift);
}

WRITE8_MEMBER(twincobr_state::wardner_exscroll_w)/* Extra unused video layer */
{
	switch (offset)
	{
		case 01:	//data <<= 8;
		case 00:	logerror("PC - write %04x to unknown video scroll X register\n",data); break;
		case 03:	//data <<= 8;
		case 02:	logerror("PC - write %04x to unknown video scroll Y register\n",data); break;
	}
}

READ8_MEMBER(twincobr_state::wardner_videoram_r)
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: return twincobr_txram_r(space,0,0xffff) >> shift;
		case 1: return twincobr_bgram_r(space,0,0xffff) >> shift;
		case 2: return twincobr_fgram_r(space,0,0xffff) >> shift;
	}
	return 0;
}

WRITE8_MEMBER(twincobr_state::wardner_videoram_w)
{
	int shift = 8 * (offset & 1);
	switch (offset/2) {
		case 0: twincobr_txram_w(space,0,data << shift, 0xff << shift); break;
		case 1: twincobr_bgram_w(space,0,data << shift, 0xff << shift); break;
		case 2: twincobr_fgram_w(space,0,data << shift, 0xff << shift); break;
	}
}

READ8_MEMBER(twincobr_state::wardner_sprite_r)
{
	int shift = (offset & 1) * 8;
	return m_spriteram->live()[offset/2] >> shift;
}

WRITE8_MEMBER(twincobr_state::wardner_sprite_w)
{
	UINT16 *spriteram16 = m_spriteram->live();
	if (offset & 1)
		spriteram16[offset/2] = (spriteram16[offset/2] & 0x00ff) | (data << 8);
	else
		spriteram16[offset/2] = (spriteram16[offset/2] & 0xff00) | data;
}



/***************************************************************************
    Ugly sprite hack for Wardner when hero is in shop
***************************************************************************/

static void wardner_sprite_priority_hack(running_machine &machine)
{
	twincobr_state *state = machine.driver_data<twincobr_state>();

	if (state->m_fgscrollx != state->m_bgscrollx) {
		UINT16 *buffered_spriteram16 = state->m_spriteram->buffer();
		if ((state->m_fgscrollx==0x1c9) || (state->m_flip_screen && (state->m_fgscrollx==0x17a))) {	/* in the shop ? */
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



static void twincobr_log_vram(running_machine &machine)
{
#ifdef MAME_DEBUG
	twincobr_state *state = machine.driver_data<twincobr_state>();

	if ( machine.input().code_pressed(KEYCODE_M) )
	{
		offs_t tile_voffs;
		int tcode[4];
		while (machine.input().code_pressed(KEYCODE_M)) ;
		logerror("Scrolls             BG-X BG-Y  FG-X FG-Y  TX-X  TX-Y\n");
		logerror("------>             %04x %04x  %04x %04x  %04x  %04x\n",state->m_bgscrollx,state->m_bgscrolly,state->m_fgscrollx,state->m_fgscrolly,state->m_txscrollx,state->m_txscrolly);
		for ( tile_voffs = 0; tile_voffs < (state->m_txvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			tcode[2] = state->m_fgvideoram16[tile_voffs];
			tcode[3] = state->m_txvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x  TX1:%02x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff,
							tcode[3] & 0xf800 >> 11, tcode[3] & 0x07ff);
		}
		for ( tile_voffs = (state->m_txvideoram_size/2); tile_voffs < (state->m_fgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			tcode[2] = state->m_fgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x  FG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff,
							tcode[2] & 0xf000 >> 12, tcode[2] & 0x0fff);
		}
		for ( tile_voffs = (state->m_fgvideoram_size/2); tile_voffs < (state->m_bgvideoram_size/2); tile_voffs++ )
		{
			tcode[1] = state->m_bgvideoram16[tile_voffs];
			logerror("$(%04x)  (Col-Tile) BG1:%01x-%03x\n", tile_voffs,
							tcode[1] & 0xf000 >> 12, tcode[1] & 0x0fff);
		}
	}
#endif
}


/***************************************************************************
    Sprite Handlers
***************************************************************************/

static void draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, int priority )
{
	twincobr_state *state = machine.driver_data<twincobr_state>();
	int offs;

	if (state->m_display_on)
	{
		UINT16 *buffered_spriteram16 = state->m_spriteram->buffer();
		for (offs = 0;offs < state->m_spriteram->bytes()/2;offs += 4)
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
					drawgfx_transpen(bitmap,cliprect,machine.gfx[3],
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
    Draw the game screen in the given bitmap_ind16.
***************************************************************************/

SCREEN_UPDATE_IND16( toaplan0 )
{
	twincobr_state *state = screen.machine().driver_data<twincobr_state>();
	twincobr_log_vram(screen.machine());

	if (state->m_wardner_sprite_hack) wardner_sprite_priority_hack(screen.machine());

	bitmap.fill(0, cliprect);

	state->m_bg_tilemap->draw(bitmap, cliprect, TILEMAP_DRAW_OPAQUE,0);
	draw_sprites(screen.machine(), bitmap,cliprect,0x0400);
	state->m_fg_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect,0x0800);
	state->m_tx_tilemap->draw(bitmap, cliprect, 0,0);
	draw_sprites(screen.machine(), bitmap,cliprect,0x0c00);
	return 0;
}

/* Spriteram is always 1 frame ahead, suggesting spriteram buffering.
  There are no CPU output registers that control this so we
  assume it happens automatically every frame, at the end of vblank */
