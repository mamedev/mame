/*******************************************************************************
 Sega System E (834-5803) Driver (video/segasyse.c)
********************************************************************************
 driver by David Haywood

 this is currently only used by megaplay, and will soon be obsolete.

*******************************************************************************/

#include "driver.h"

/*-- Variables --*/

#define CHIPS 2							/* There are 2 VDP Chips */

static UINT8 vdp_cmdpart[CHIPS];		/* VDP Command Part Counter */
static UINT16 vdp_command[CHIPS];		/* VDP Command Word */

static UINT8 vdp_accessmode[CHIPS];		/* VDP Access Mode (VRAM, CRAM) */
static UINT16 vdp_accessaddr[CHIPS];	/* VDP Access Address */
static UINT8 vdp_readbuffer[CHIPS];		/* VDP Read Buffer */

static UINT8 *segae_vdp_vram[CHIPS];			/* Pointer to VRAM */
static UINT8 *vdp_cram[CHIPS];			/* Pointer to the VDP's CRAM */
UINT8 *segae_vdp_regs[CHIPS];			/* Pointer to the VDP's Registers */

static UINT8 segae_vdp_vrambank[CHIPS];		/* Current VRAM Bank number (from writes to Port 0xf7) */

static UINT8 *cache_bitmap;					/* 8bpp bitmap with raw pen values */

static int palette_base; // needed for megatech for now..

UINT8 segae_vintpending;
UINT8 segae_hintpending;

/*-- Prototypes --*/

static void vdp_start(running_machine *machine, UINT8 chip );

static void vdp_processcmd(UINT8 chip, UINT16 cmd);
static void vdp_setregister(UINT8 chip, UINT16 cmd);

static void draw_tiles_line(UINT8 *dest, int line, UINT8 chip, UINT8 pri);
static void draw_sprite_line(UINT8 *dest, UINT8 chip, UINT8 line);
static void segae_drawscanline(running_machine *machine, int line, int chips, int blank);

static void draw_8pix_solid16(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line, UINT8 flipx, UINT8 col);
static void draw_8pix(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line, UINT8 flipx, UINT8 col);
static void draw_8pix_sprite(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line);

/*******************************************************************************
 vhstart, vhstop and vhrefresh functions
*******************************************************************************/

/* starts vdp for bios screen only */
VIDEO_START( megaplay_normal )
{
	palette_base = 0x40;

	vdp_start(machine, 0);

	cache_bitmap = auto_malloc( (16+256+16) * 192); /* 16 pixels either side to simplify drawing */
}

VIDEO_UPDATE( megaplay_normal )
{
	int miny = (cliprect->min_y < 16) ? 16 : cliprect->min_y;
	int maxy = (cliprect->max_y > 16+192-1) ? 16+192-1 : cliprect->max_y;
	int i;

	/*- Draw from cache_bitmap to screen -*/

	for (i = miny; i <= maxy;i++)
		segae_drawscanline(screen->machine, i-16,0,0);

	for (i = miny;i <= maxy;i++)
	{
		UINT16 *dest = BITMAP_ADDR16(bitmap, i, 32);
		int x;
		for (x = 0; x < 256; x++)
		{
			UINT8 pix = cache_bitmap[(i-16) * (16+256+16) + 24 + x];
			if (pix != 0)
				dest[x] = screen->machine->pens[palette_base + pix];
		}
	}

	return 0;
}

/*******************************************************************************
 VDP Start / Stop Functions
********************************************************************************
 note: we really should check after each allocation to make sure it was
       successful then if one allocation fails we can free up the previous ones
*******************************************************************************/

static void vdp_start(running_machine *machine, UINT8 chip)
{
	UINT8 temp;

	/*- VRAM -*/

	segae_vdp_vram[chip] = auto_malloc(0x8000); /* 32kb (2 banks) */
	segae_vdp_vrambank[chip] = 0;

	/*- CRAM -*/

	vdp_cram[chip] = auto_malloc(0x20);

	/*- VDP Registers -*/

	segae_vdp_regs[chip] = auto_malloc(0x20);

	/*- Clear Memory -*/

	memset(segae_vdp_vram[chip], 0, 0x8000);
	memset(vdp_cram[chip], 0, 0x20);
	memset(segae_vdp_regs[chip], 0, 0x20);

	/*- Set Up Some Default Values */

	vdp_accessaddr[chip] = 0;
	vdp_accessmode[chip] = 0;
	vdp_cmdpart[chip] = 0;
	vdp_command[chip] = 0;

	/*- Black the Palette -*/

	for (temp=0;temp<32;temp++)
		palette_set_color(machine, temp + 32*chip+palette_base, MAKE_RGB(0, 0, 0));

	/* Save State Stuff (based on video/taitoic.c) */

	state_save_register_item_pointer(machine, "VDP", NULL, chip, segae_vdp_vram[chip], 0x8000);
	state_save_register_item_pointer(machine, "VDP", NULL, chip, vdp_cram[chip], 0x20);
	state_save_register_item_pointer(machine, "VDP", NULL, chip, segae_vdp_regs[chip], 0x20);
	state_save_register_item(machine, "VDP", NULL, chip, vdp_cmdpart[chip]);
	state_save_register_item(machine, "VDP", NULL, chip, vdp_command[chip]);
	state_save_register_item(machine, "VDP", NULL, chip, vdp_accessmode[chip]);
	state_save_register_item(machine, "VDP", NULL, chip, vdp_accessaddr[chip]);
	state_save_register_item(machine, "VDP", NULL, chip, segae_vdp_vrambank[chip]);
}

/*******************************************************************************
 Core VDP Functions
*******************************************************************************/

/*-- Reads --*/

/***************************************
 segae_vdp_ctrl_r ( UINT8 chip )
****************************************
 reading the vdp control port will
 return the following

 bit:
  7 - vert int pending
  6 - line int pending
  5 - sprite collision (non 0 pixels) *not currently emulated (not needed by these games)*
  4 - always 0
  3 - always 0
  2 - always 0
  1 - always 0
  0 - always 0

  bits 5,6,7 are cleared after a read
***************************************/

UINT8 segae_vdp_ctrl_r ( UINT8 chip )
{
	UINT8 temp;

	temp = 0;

	temp |= (segae_vintpending << 7);
	temp |= (segae_hintpending << 6);

	segae_hintpending = segae_vintpending = 0;

	return temp;
}

UINT8 segae_vdp_data_r ( UINT8 chip )
{
	UINT8 temp;

	vdp_cmdpart[chip] = 0;

	temp = vdp_readbuffer[chip];

	if (vdp_accessmode[chip]==0x03) { /* CRAM Access */
		/* error CRAM can't be read!! */
	} else { /* VRAM */
		vdp_readbuffer[chip] = segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + vdp_accessaddr[chip] ];
		vdp_accessaddr[chip] += 1;
		vdp_accessaddr[chip] &= 0x3fff;
	}
	return temp;
}

/*-- Writes --*/

void segae_vdp_ctrl_w ( UINT8 chip, UINT8 data )
{
	if (!vdp_cmdpart[chip]) {
		vdp_cmdpart[chip] = 1;
		vdp_command[chip] = data;
	} else {
		vdp_cmdpart[chip] = 0;
		vdp_command[chip] |= (data << 8);
		vdp_processcmd (chip, vdp_command[chip]);
	}
}

void segae_vdp_data_w ( running_machine *machine, UINT8 chip, UINT8 data )
{
	vdp_cmdpart[chip] = 0;

	if (vdp_accessmode[chip]==0x03) { /* CRAM Access */
		UINT8 r,g,b, temp;

		temp = vdp_cram[chip][vdp_accessaddr[chip]];

		vdp_cram[chip][vdp_accessaddr[chip]] = data;

		if (temp != data) {
			r = (vdp_cram[chip][vdp_accessaddr[chip]] & 0x03) >> 0;
			g = (vdp_cram[chip][vdp_accessaddr[chip]] & 0x0c) >> 2;
			b = (vdp_cram[chip][vdp_accessaddr[chip]] & 0x30) >> 4;

			palette_set_color_rgb(machine, vdp_accessaddr[chip] + 32*chip+palette_base, pal2bit(r), pal2bit(g), pal2bit(b));
		}

		vdp_accessaddr[chip] += 1;
		vdp_accessaddr[chip] &= 0x1f;
	} else { /* VRAM Accesses */
		segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + vdp_accessaddr[chip] ] = data;
		vdp_accessaddr[chip] += 1;
		vdp_accessaddr[chip] &= 0x3fff;
	}
}

/*-- Associated Functions --*/

/***************************************
 vdp_processcmd
****************************************

general command format

 M M A A A A A A A A A A A A A A  M=Mode, A=Address

 the command will be one of 3 things according to the upper
 4 bits

 0 0 - - - - - - - - - - - - - -  VRAM Acess Mode (Special Read)

 0 1 - - - - - - - - - - - - - -  VRAM Acesss Mode

 1 0 0 0 - - - - - - - - - - - -  VDP Register Set (current mode & address _not_ changed)

 1 0 x x - - - - - - - - - - - -  VRAM Access Mode (0x1000 - 0x3FFF only, x x is anything but 0 0)

 1 1 - - - - - - - - - - - - - -  CRAM Access Mode

***************************************/

static void vdp_processcmd (UINT8 chip, UINT16 cmd)
{
	if ( (cmd & 0xf000) == 0x8000 ) { /*  1 0 0 0 - - - - - - - - - - - -  VDP Register Set */
		vdp_setregister (chip, cmd);
	} else { /* Anything Else */
		vdp_accessmode[chip] = (cmd & 0xc000) >> 14;
		vdp_accessaddr[chip] = (cmd & 0x3fff);

		if ((vdp_accessmode[chip]==0x03) && (vdp_accessaddr[chip] > 0x1f) ) { /* Check Address is valid for CRAM */
			/* Illegal, CRAM isn't this large! */
			vdp_accessaddr[chip] &= 0x1f;
		}

		if (vdp_accessmode[chip] == 0x00) { /*  0 0 - - - - - - - - - - - - - -  VRAM Acess Mode (Special Read) */
			vdp_readbuffer[chip] = segae_vdp_vram[chip][ segae_vdp_vrambank[chip]*0x4000 + vdp_accessaddr[chip] ];
			vdp_accessaddr[chip] += 1;
			vdp_accessaddr[chip] &= 0x3fff;
		}
	}
}

/***************************************
 vdp_setregister

 general command format

 1 0 0 0 R R R R D D D D D D D D  1/0 = Fixed Values, R = Register # / Address, D = Data

***************************************/

static void vdp_setregister(UINT8 chip, UINT16 cmd)
{
	UINT8 regnumber;
	UINT8 regdata;

	regnumber = (cmd & 0x0f00) >> 8;
	regdata   = (cmd & 0x00ff);

	if (regnumber < 11) {
		segae_vdp_regs[chip][regnumber] = regdata;
	} else {
		/* Illegal, there aren't this many registers! */
	}
}

/*******************************************************************************
 System E Drawing Capabilities Notes
********************************************************************************
 Display Consists of

 VDP0 Backdrop Color (?)
 VDP0 Tiles (Low)
 VDP0 Sprites
 VDP0 Tiles (High)
 VDP1 Tiles (Low)
 VDP1 Sprites
 VDP1 Tiles (High)

 each vdp has its on vram, etc etc.

 the tilemaps are 256x224 in size, 256x192 of this is visible, the tiles
 are 8x8 pixels, so 32x28 of 8x8 tiles make a 256x224 tilemap.

 the tiles are 4bpp (16 colours), video ram can hold upto 512 tile gfx

 tile references are 16 bits (3 bits unused, 1 bit priority, 1 bit palette,
 2 bits for flip, 9 bits for tile number)

 tilemaps can be scrolled horizontally, the top 16 lines of the display can
 have horinzontal scrolling disabled

 tilemaps can be scrolled vertically, the right 64 lines of the display can
 have the vertical scrolling disabled

 the leftmost 8 line of the display can be blanked

*******************************************************************************/

static void segae_drawscanline(running_machine *machine, int line, int chips, int blank)
{

	UINT8* dest;

	if (video_skip_this_frame())
		return;

	dest = cache_bitmap + (16+256+16) * line;

	/* This should be cleared to bg colour, but which vdp determines that !, neither seems to be right, maybe its always the same col? */
	memset(dest, 0, 16+256+16);

	if (segae_vdp_regs[0][1] & 0x40) {
		draw_tiles_line (dest+16, line, 0,0);
		draw_sprite_line(dest+16, 0, line);
		draw_tiles_line (dest+16, line, 0,1);
	}

	if (chips>0) /* we don't want to do this on megatech */
	{
		if (segae_vdp_regs[1][1] & 0x40) {
			draw_tiles_line (dest+16, line, 1,0);
			draw_sprite_line(dest+16, 1, line);
			draw_tiles_line (dest+16, line, 1,1);
		}
	}

	/* FIX ME!! */
	if (blank)
	{
		if (strcmp(machine->gamedrv->name,"tetrisse")) /* and we really don't want to do it on tetrise */
			memset(dest+16, 32+16, 8); /* Clear Leftmost column, there should be a register for this like on the SMS i imagine    */
							   			  /* on the SMS this is bit 5 of register 0 (according to CMD's SMS docs) for system E this  */							   			  /* appears to be incorrect, most games need it blanked 99% of the time so we blank it      */
	}
}

/*-- Drawing a line of tiles --*/

static void draw_tiles_line(UINT8 *dest, int line, UINT8 chip, UINT8 pri)
{
	/* todo: fix vscrolling (or is it something else causing the glitch on the hi-score screen of hangonjr, seems to be ..  */

	UINT8 hscroll;
	UINT8 vscroll;
	UINT16 tmbase;
	UINT8 tilesline, tilesline2;
	UINT8 coloffset, coloffset2;
	UINT8 loopcount;

	hscroll = (256-segae_vdp_regs[chip][8]);
	vscroll = segae_vdp_regs[chip][9];
	if (vscroll > 224) vscroll %= 224;

	tmbase =  (segae_vdp_regs[chip][2] & 0x0e) << 10;
	tmbase += (segae_vdp_vrambank[chip] * 0x4000);

	tilesline = (line + vscroll) >> 3;
	tilesline2= (line + vscroll) % 8;


	coloffset = (hscroll >> 3);
	coloffset2= (hscroll % 8);

	dest -= coloffset2;

	for (loopcount=0;loopcount<33;loopcount++) {

		UINT16 vram_offset, vram_word;
		UINT16 tile_no;
		UINT8  palette, priority, flipx, flipy;

		vram_offset = tmbase
					+ (2 * (32*tilesline + ((coloffset+loopcount)&0x1f) ) );
		vram_word = segae_vdp_vram[chip][vram_offset] | (segae_vdp_vram[chip][vram_offset+1] << 8);

		tile_no = (vram_word & 0x01ff);
		flipx =   (vram_word & 0x0200) >> 9;
		flipy =   (vram_word & 0x0400) >> 10;
		palette = (vram_word & 0x0800) >> 11;
		priority= (vram_word & 0x1000) >> 12;

		tilesline2= (line + vscroll) % 8;
		if (flipy) tilesline2 = 7-tilesline2;

		if (priority == pri) {
			if (chip == 0) draw_8pix_solid16(dest, chip, tile_no,tilesline2,flipx,palette);
			else draw_8pix(dest, chip, tile_no,tilesline2,flipx,palette);
		}
		dest+=8;
	}
}

static void draw_sprite_line(UINT8 *dest, UINT8 chip, UINT8 line)
{
	int nosprites;
	int loopcount;

	UINT16 spritebase;

	nosprites = 63; // if there is no end marker we draw everything (ridleofp)

	spritebase =  (segae_vdp_regs[chip][5] & 0x7e) << 7;
	spritebase += (segae_vdp_vrambank[chip] * 0x4000);

	/*- find out how many sprites there are -*/

	for (loopcount=0;loopcount<64;loopcount++) {
		UINT8 ypos;

		ypos = segae_vdp_vram[chip][spritebase+loopcount];

		if (ypos==208) {
			nosprites=loopcount;
			break;
		}
	}

	/*- draw sprites IN REVERSE ORDER -*/

	for (loopcount = nosprites; loopcount >= 0;loopcount--) {
		int ypos;
		UINT8 sheight;

		ypos = segae_vdp_vram[chip][spritebase+loopcount] +1;

		if (segae_vdp_regs[chip][1] & 0x02) sheight=16; else sheight=8;

		if ( (line >= ypos) && (line < ypos+sheight) ) {
			int xpos;
			UINT16 sprnum;
			UINT8 spline;

			spline = line - ypos;

			xpos   = segae_vdp_vram[chip][spritebase+0x80+ (2*loopcount)];
			sprnum = segae_vdp_vram[chip][spritebase+0x81+ (2*loopcount)];

			if (segae_vdp_regs[chip][6] & 0x04)
				sprnum += 0x100;

			draw_8pix_sprite(dest+xpos, chip, sprnum, spline);

		}
	}
}

static void draw_8pix_solid16(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line, UINT8 flipx, UINT8 col)
{

	UINT32 pix8 = *(UINT32 *)&segae_vdp_vram[chip][(32)*tile + (4)*line + (0x4000) * segae_vdp_vrambank[chip]];
	UINT8  pix, coladd;

	if (!pix8 && !col) return; /*note only the colour 0 of each vdp is transparent NOT colour 16???, fixes sky in HangonJr */

	coladd = 16*col;

	if (flipx)	{
		pix = ((pix8 >> 0) & 0x01) | ((pix8 >> 7) & 0x02) | ((pix8 >> 14) & 0x04) | ((pix8 >> 21) & 0x08) ; pix+= coladd ; if (pix) dest[0] = pix+ 32*chip;
		pix = ((pix8 >> 1) & 0x01) | ((pix8 >> 8) & 0x02) | ((pix8 >> 15) & 0x04) | ((pix8 >> 22) & 0x08) ; pix+= coladd ; if (pix) dest[1] = pix+ 32*chip;
		pix = ((pix8 >> 2) & 0x01) | ((pix8 >> 9) & 0x02) | ((pix8 >> 16) & 0x04) | ((pix8 >> 23) & 0x08) ; pix+= coladd ; if (pix) dest[2] = pix+ 32*chip;
		pix = ((pix8 >> 3) & 0x01) | ((pix8 >>10) & 0x02) | ((pix8 >> 17) & 0x04) | ((pix8 >> 24) & 0x08) ; pix+= coladd ; if (pix) dest[3] = pix+ 32*chip;
		pix = ((pix8 >> 4) & 0x01) | ((pix8 >>11) & 0x02) | ((pix8 >> 18) & 0x04) | ((pix8 >> 25) & 0x08) ; pix+= coladd ; if (pix) dest[4] = pix+ 32*chip;
		pix = ((pix8 >> 5) & 0x01) | ((pix8 >>12) & 0x02) | ((pix8 >> 19) & 0x04) | ((pix8 >> 26) & 0x08) ; pix+= coladd ; if (pix) dest[5] = pix+ 32*chip;
		pix = ((pix8 >> 6) & 0x01) | ((pix8 >>13) & 0x02) | ((pix8 >> 20) & 0x04) | ((pix8 >> 27) & 0x08) ; pix+= coladd ; if (pix) dest[6] = pix+ 32*chip;
		pix = ((pix8 >> 7) & 0x01) | ((pix8 >>14) & 0x02) | ((pix8 >> 21) & 0x04) | ((pix8 >> 28) & 0x08) ; pix+= coladd ; if (pix) dest[7] = pix+ 32*chip;
	} else {
		pix = ((pix8 >> 7) & 0x01) | ((pix8 >>14) & 0x02) | ((pix8 >> 21) & 0x04) | ((pix8 >> 28) & 0x08) ; pix+= coladd ; if (pix) dest[0] = pix+ 32*chip;
		pix = ((pix8 >> 6) & 0x01) | ((pix8 >>13) & 0x02) | ((pix8 >> 20) & 0x04) | ((pix8 >> 27) & 0x08) ; pix+= coladd ; if (pix) dest[1] = pix+ 32*chip;
		pix = ((pix8 >> 5) & 0x01) | ((pix8 >>12) & 0x02) | ((pix8 >> 19) & 0x04) | ((pix8 >> 26) & 0x08) ; pix+= coladd ; if (pix) dest[2] = pix+ 32*chip;
		pix = ((pix8 >> 4) & 0x01) | ((pix8 >>11) & 0x02) | ((pix8 >> 18) & 0x04) | ((pix8 >> 25) & 0x08) ; pix+= coladd ; if (pix) dest[3] = pix+ 32*chip;
		pix = ((pix8 >> 3) & 0x01) | ((pix8 >>10) & 0x02) | ((pix8 >> 17) & 0x04) | ((pix8 >> 24) & 0x08) ; pix+= coladd ; if (pix) dest[4] = pix+ 32*chip;
		pix = ((pix8 >> 2) & 0x01) | ((pix8 >> 9) & 0x02) | ((pix8 >> 16) & 0x04) | ((pix8 >> 23) & 0x08) ; pix+= coladd ; if (pix) dest[5] = pix+ 32*chip;
		pix = ((pix8 >> 1) & 0x01) | ((pix8 >> 8) & 0x02) | ((pix8 >> 15) & 0x04) | ((pix8 >> 22) & 0x08) ; pix+= coladd ; if (pix) dest[6] = pix+ 32*chip;
		pix = ((pix8 >> 0) & 0x01) | ((pix8 >> 7) & 0x02) | ((pix8 >> 14) & 0x04) | ((pix8 >> 21) & 0x08) ; pix+= coladd ; if (pix) dest[7] = pix+ 32*chip;
	}
}

static void draw_8pix(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line, UINT8 flipx, UINT8 col)
{

	UINT32 pix8 = *(UINT32 *)&segae_vdp_vram[chip][(32)*tile + (4)*line + (0x4000) * segae_vdp_vrambank[chip]];
	UINT8  pix, coladd;

	if (!pix8) return;

	coladd = 16*col+32*chip;

	if (flipx)	{
		pix = ((pix8 >> 0) & 0x01) | ((pix8 >> 7) & 0x02) | ((pix8 >> 14) & 0x04) | ((pix8 >> 21) & 0x08) ; if (pix) dest[0] = pix+ coladd;
		pix = ((pix8 >> 1) & 0x01) | ((pix8 >> 8) & 0x02) | ((pix8 >> 15) & 0x04) | ((pix8 >> 22) & 0x08) ; if (pix) dest[1] = pix+ coladd;
		pix = ((pix8 >> 2) & 0x01) | ((pix8 >> 9) & 0x02) | ((pix8 >> 16) & 0x04) | ((pix8 >> 23) & 0x08) ; if (pix) dest[2] = pix+ coladd;
		pix = ((pix8 >> 3) & 0x01) | ((pix8 >>10) & 0x02) | ((pix8 >> 17) & 0x04) | ((pix8 >> 24) & 0x08) ; if (pix) dest[3] = pix+ coladd;
		pix = ((pix8 >> 4) & 0x01) | ((pix8 >>11) & 0x02) | ((pix8 >> 18) & 0x04) | ((pix8 >> 25) & 0x08) ; if (pix) dest[4] = pix+ coladd;
		pix = ((pix8 >> 5) & 0x01) | ((pix8 >>12) & 0x02) | ((pix8 >> 19) & 0x04) | ((pix8 >> 26) & 0x08) ; if (pix) dest[5] = pix+ coladd;
		pix = ((pix8 >> 6) & 0x01) | ((pix8 >>13) & 0x02) | ((pix8 >> 20) & 0x04) | ((pix8 >> 27) & 0x08) ; if (pix) dest[6] = pix+ coladd;
		pix = ((pix8 >> 7) & 0x01) | ((pix8 >>14) & 0x02) | ((pix8 >> 21) & 0x04) | ((pix8 >> 28) & 0x08) ; if (pix) dest[7] = pix+ coladd;
	} else {
		pix = ((pix8 >> 7) & 0x01) | ((pix8 >>14) & 0x02) | ((pix8 >> 21) & 0x04) | ((pix8 >> 28) & 0x08) ; if (pix) dest[0] = pix+ coladd;
		pix = ((pix8 >> 6) & 0x01) | ((pix8 >>13) & 0x02) | ((pix8 >> 20) & 0x04) | ((pix8 >> 27) & 0x08) ; if (pix) dest[1] = pix+ coladd;
		pix = ((pix8 >> 5) & 0x01) | ((pix8 >>12) & 0x02) | ((pix8 >> 19) & 0x04) | ((pix8 >> 26) & 0x08) ; if (pix) dest[2] = pix+ coladd;
		pix = ((pix8 >> 4) & 0x01) | ((pix8 >>11) & 0x02) | ((pix8 >> 18) & 0x04) | ((pix8 >> 25) & 0x08) ; if (pix) dest[3] = pix+ coladd;
		pix = ((pix8 >> 3) & 0x01) | ((pix8 >>10) & 0x02) | ((pix8 >> 17) & 0x04) | ((pix8 >> 24) & 0x08) ; if (pix) dest[4] = pix+ coladd;
		pix = ((pix8 >> 2) & 0x01) | ((pix8 >> 9) & 0x02) | ((pix8 >> 16) & 0x04) | ((pix8 >> 23) & 0x08) ; if (pix) dest[5] = pix+ coladd;
		pix = ((pix8 >> 1) & 0x01) | ((pix8 >> 8) & 0x02) | ((pix8 >> 15) & 0x04) | ((pix8 >> 22) & 0x08) ; if (pix) dest[6] = pix+ coladd;
		pix = ((pix8 >> 0) & 0x01) | ((pix8 >> 7) & 0x02) | ((pix8 >> 14) & 0x04) | ((pix8 >> 21) & 0x08) ; if (pix) dest[7] = pix+ coladd;
	}
}

static void draw_8pix_sprite(UINT8 *dest, UINT8 chip, UINT16 tile, UINT8 line)
{

	UINT32 pix8 = *(UINT32 *)&segae_vdp_vram[chip][(32)*tile + (4)*line + (0x4000) * segae_vdp_vrambank[chip]];
	UINT8  pix;

	if (!pix8) return; /*note only the colour 0 of each vdp is transparent NOT colour 16, fixes sky in HangonJr */

	pix = ((pix8 >> 7) & 0x01) | ((pix8 >>14) & 0x02) | ((pix8 >> 21) & 0x04) | ((pix8 >> 28) & 0x08) ; if (pix) dest[0] = pix+16+32*chip;
	pix = ((pix8 >> 6) & 0x01) | ((pix8 >>13) & 0x02) | ((pix8 >> 20) & 0x04) | ((pix8 >> 27) & 0x08) ; if (pix) dest[1] = pix+16+32*chip;
	pix = ((pix8 >> 5) & 0x01) | ((pix8 >>12) & 0x02) | ((pix8 >> 19) & 0x04) | ((pix8 >> 26) & 0x08) ; if (pix) dest[2] = pix+16+32*chip;
	pix = ((pix8 >> 4) & 0x01) | ((pix8 >>11) & 0x02) | ((pix8 >> 18) & 0x04) | ((pix8 >> 25) & 0x08) ; if (pix) dest[3] = pix+16+32*chip;
	pix = ((pix8 >> 3) & 0x01) | ((pix8 >>10) & 0x02) | ((pix8 >> 17) & 0x04) | ((pix8 >> 24) & 0x08) ; if (pix) dest[4] = pix+16+32*chip;
	pix = ((pix8 >> 2) & 0x01) | ((pix8 >> 9) & 0x02) | ((pix8 >> 16) & 0x04) | ((pix8 >> 23) & 0x08) ; if (pix) dest[5] = pix+16+32*chip;
	pix = ((pix8 >> 1) & 0x01) | ((pix8 >> 8) & 0x02) | ((pix8 >> 15) & 0x04) | ((pix8 >> 22) & 0x08) ; if (pix) dest[6] = pix+16+32*chip;
	pix = ((pix8 >> 0) & 0x01) | ((pix8 >> 7) & 0x02) | ((pix8 >> 14) & 0x04) | ((pix8 >> 21) & 0x08) ; if (pix) dest[7] = pix+16+32*chip;

}
