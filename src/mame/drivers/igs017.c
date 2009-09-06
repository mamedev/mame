/************************************************************************************************************

                                          -= IGS017 Based Hardware =-

                             driver by   Pierpaolo Prazzoli, Luca Elia (l.elia@tin.it)

IGS017: 2 tilemaps, variable size sprites, protection.

-------------------------------------------------------------------------------------------------------------
Year + Game                      PCB        CPU      Sound         Custom                Other
-------------------------------------------------------------------------------------------------------------
96 Shu Zi Le Yuan                N0-0131-4  HD64180  M6295 YM2413  IGS017 8255           Battery
97 Mahjong Tian Jiang Shen Bing  N0-0157-2  HD64180  M6295 YM2413  IGS017 IGS025         Battery
98 Mahjong Man Guan Cai Shen     N0-0192-1  M68000   M6295         IGS017 IGS025 IGS029  Battery
-------------------------------------------------------------------------------------------------------------

To Do:

- Protection emulation, instead of patching the roms.
- tjsb: finish sprites decryption (data lines scrambling), protection, inputs.
- iqblocka: NVRAM.
- iqblockf: protection.

Notes:

- iqblocka: keep start or test pressed during boot to enter test mode A or B.

************************************************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z180/z180.h"
#include "machine/8255ppi.h"
#include "sound/2413intf.h"
#include "sound/okim6295.h"

/***************************************************************************
                                Video Hardware
***************************************************************************/

static UINT8 *fg_videoram, *bg_videoram;
static tilemap *fg_tilemap, *bg_tilemap;

#define COLOR(_X)	(((_X)>>2))

static TILE_GET_INFO( get_fg_tile_info )
{
	int code = fg_videoram[tile_index*4+0] + (fg_videoram[tile_index*4+1] << 8);
	int attr = fg_videoram[tile_index*4+2] + (fg_videoram[tile_index*4+3] << 8);
	SET_TILE_INFO(0, code, COLOR(attr), 0);
}
static TILE_GET_INFO( get_bg_tile_info )
{
	int code = bg_videoram[tile_index*4+0] + (bg_videoram[tile_index*4+1] << 8);
	int attr = bg_videoram[tile_index*4+2] + (bg_videoram[tile_index*4+3] << 8);
	SET_TILE_INFO(0, code, COLOR(attr)+8, 0);
}

static WRITE8_HANDLER( fg_w )
{
	fg_videoram[offset] = data;
	tilemap_mark_tile_dirty(fg_tilemap,offset/4);
}

static WRITE8_HANDLER( bg_w )
{
	bg_videoram[offset] = data;
	tilemap_mark_tile_dirty(bg_tilemap,offset/4);
}

// 16-bit handlers for an 8-bit chip

static READ16_HANDLER( fg_lsb_r )
{
	return fg_videoram[offset];
}
static WRITE16_HANDLER( fg_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		fg_w(space,offset,data);
}

static READ16_HANDLER( bg_lsb_r )
{
	return bg_videoram[offset];
}
static WRITE16_HANDLER( bg_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		bg_w(space,offset,data);
}

static READ16_HANDLER( spriteram_lsb_r )
{
	return spriteram[offset];
}
static WRITE16_HANDLER( spriteram_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		spriteram[offset] = data;
}


static VIDEO_START( igs017 )
{
	fg_tilemap = tilemap_create(machine, get_fg_tile_info,tilemap_scan_rows,8,8,64,32);
	bg_tilemap = tilemap_create(machine, get_bg_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0xf);
	tilemap_set_transparent_pen(bg_tilemap,0xf);
}


static UINT8 *sprites_gfx;
static int sprites_gfx_size;

// Eeach 16 bit word in the sprites gfx roms contains three 5 bit pens: x-22222-11111-00000.
// This routine expands each word into three bytes.
static void expand_sprites(running_machine *machine)
{
	UINT8 *rom	=	memory_region(machine, "sprites");
	int size	=	memory_region_length(machine, "sprites");
	int i;

	sprites_gfx_size	=	size / 2 * 3;
	sprites_gfx			=	auto_alloc_array(machine, UINT8, sprites_gfx_size);

	for (i = 0; i < size / 2 ; i++)
	{
		UINT16 pens = (rom[i*2+1] << 8) | rom[i*2];

		sprites_gfx[i * 3 + 0] = (pens >>  0) & 0x1f;
		sprites_gfx[i * 3 + 1] = (pens >>  5) & 0x1f;
		sprites_gfx[i * 3 + 2] = (pens >> 10) & 0x1f;
	}
}

/***************************************************************************
                              Sprites Format

    Offset:         Bits:               Value:

        0.b                             Y (low)

        1.b         7654 32--           Size Y (low)
                    ---- --10           Y (high)

        2.b         7654 3---           X (low)
                    ---- -210           Size Y (high)

        3.b         76-- ----           Size X (low)
                    --5- ----
                    ---4 3210           X (high)

        4.b         76-- ----           Code (low)
                    --54 3210           Size X (high)

        5.b                             Code (mid low)

        6.b                             Code (mid high)

        7.b         765- ----           Color
                    ---4 ----           Flip X
                    ---- 3---
                    ---- -210           Code (high)

    Code = ROM Address / 2 = Pixel / 3

***************************************************************************/

static void draw_sprite(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect, int sx, int sy, int dimx, int dimy, int flipx, int flipy, int color, int addr)
{
	// prepare GfxElement on the fly
	gfx_element gfx;

	// Bounds checking
	if ( addr + dimx * dimy >= sprites_gfx_size )
		return;

	gfx_element_build_temporary(&gfx, machine, sprites_gfx + addr, dimx, dimy, dimx, 0x100, 32, 0);

	drawgfx_transpen(	bitmap,cliprect, &gfx,
				0, color,
				flipx, flipy,
				sx, sy, 0x1f	);
}

static void draw_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
	UINT8 *s	=	spriteram;
	UINT8 *end	=	spriteram + 0x800;

	for ( ; s < end; s += 8 )
	{
		int	x,y, sx,sy, dimx,dimy, flipx,flipy, addr,color;

		y		=	s[0] + (s[1] << 8);
		x		=	s[2] + (s[3] << 8);
		addr	=	(s[4] >> 6) | (s[5] << 2) | (s[6] << 10) | ((s[7] & 0x07) << 18);
		addr	*=	3;

		flipx	=	s[7] & 0x10;
		flipy	=	0;

		dimx	=	((((s[4] & 0x3f)<<2) | ((s[3] & 0xc0)>>6))+1) * 3;
		dimy	=	((y >> 10) | ((x & 0x03)<<6))+1;

		x		>>=	3;
		sx		=	(x & 0x1ff) - (x & 0x200);
		sy		=	(y & 0x1ff) - (y & 0x200);

		color = (s[7] & 0xe0) >> 5;

		draw_sprite(machine, bitmap, cliprect, sx, sy, dimx, dimy, flipx, flipy, color, addr);
	}
}

// A simple gfx viewer (toggle with T)
static int debug_viewer(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect)
{
#ifdef MAME_DEBUG
	static int toggle = 0;
	if (input_code_pressed_once(machine, KEYCODE_T))	toggle = 1-toggle;
	if (toggle)	{
		static int a = 0, w = 512;
		int h = 256;

		if (input_code_pressed(machine, KEYCODE_O))		w += 1;
		if (input_code_pressed(machine, KEYCODE_I))		w -= 1;

		if (input_code_pressed(machine, KEYCODE_U))		w += 8;
		if (input_code_pressed(machine, KEYCODE_Y))		w -= 8;

		if (input_code_pressed(machine, KEYCODE_RIGHT))	a += 1;
		if (input_code_pressed(machine, KEYCODE_LEFT))	a -= 1;

		if (input_code_pressed(machine, KEYCODE_DOWN))	a += w;
		if (input_code_pressed(machine, KEYCODE_UP))		a -= w;

		if (input_code_pressed(machine, KEYCODE_PGDN))	a += w * h;
		if (input_code_pressed(machine, KEYCODE_PGUP))	a -= w * h;

		if (a < 0)		a = 0;
		if (a > sprites_gfx_size)	a = sprites_gfx_size;

		if (w <= 0)		w = 0;
		if (w > 1024)	w = 1024;

		bitmap_fill(bitmap,cliprect,0);

		draw_sprite(machine, bitmap, cliprect, 0,0, w,h, 0,0, 0, a);

		popmessage("a: %08X w: %03X p: %02X-%02x-%02x",a,w,sprites_gfx[a/3*3+0],sprites_gfx[a/3*3+1],sprites_gfx[a/3*3+2]);
		osd_sleep(200000);
		return 1;
	}
#endif
	return 0;
}

static VIDEO_UPDATE( igs017 )
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(screen->machine, KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(screen->machine, KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(screen->machine, KEYCODE_W))	mask |= 2;
		if (input_code_pressed(screen->machine, KEYCODE_A))	mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif

	if (debug_viewer(screen->machine, bitmap,cliprect))
		return 0;

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_OPAQUE,0);
	else					bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (layers_ctrl & 4)	draw_sprites(screen->machine,bitmap,cliprect);

	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	return 0;
}

/***************************************************************************
                                Decryption
***************************************************************************/

static void decrypt_program_rom(running_machine *machine, int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0)
{
	int length = memory_region_length(machine, "maincpu");
	UINT8 *rom = memory_region(machine, "maincpu");
	UINT8 *tmp = auto_alloc_array(machine, UINT8, length);
	int i;

	// decrypt the program ROM

	// XOR layer
	for (i = 0;i < length;i++)
	{
		if(i & 0x2000)
		{
			if((i & mask) == mask)
				rom[i] ^= 0x01;
		}
		else
		{
			if(i & 0x0100)
			{
				if((i & mask) == mask)
					rom[i] ^= 0x01;
			}
			else
			{
				if(i & 0x0080)
				{
					if((i & mask) == mask)
						rom[i] ^= 0x01;
				}
				else
				{
					if((i & mask) != mask)
						rom[i] ^= 0x01;
				}
			}
		}
	}

	memcpy(tmp,rom,length);

	// address lines swap
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,a7,a6,a5,a4,a3,a2,a1,a0);
		rom[i] = tmp[addr];
	}
}


// iqblocka

static void iqblocka_patch_rom(running_machine *machine)
{
	UINT8 *rom = memory_region(machine, "maincpu");

//  rom[0x7b64] = 0xc9;

	rom[0x010c7] = 0x18;

	// CBEF bank 0a
	rom[0x16bef] = 0x18;

	// C1BD bank 24
	rom[0x301bd] = 0x18;

	// C21B bank 2e
	rom[0x3a21b] = 0x18;

	// DCA9 bank 2e
	rom[0x3bca9] = 0x18;

	// maybe
//  rom[0x18893] = 0x18;
//  rom[0x385b1] = 0x18;
}

static DRIVER_INIT( iqblocka )
{
	decrypt_program_rom(machine, 0x11, 7, 6, 5, 4, 3, 2, 1, 0);
	iqblocka_patch_rom(machine);

	expand_sprites(machine);
}

// iqblockf

static DRIVER_INIT( iqblockf )
{
	decrypt_program_rom(machine, 0x11, 7, 6, 5, 4, 3, 2, 1, 0);
//  iqblockf_patch_rom(machine);

	expand_sprites(machine);
}

// tjsb

static void tjsb_decrypt_sprites(running_machine *machine)
{
	int length = memory_region_length(machine, "sprites");
	UINT8 *rom = memory_region(machine, "sprites");
	UINT8 *tmp = auto_alloc_array(machine, UINT8, length);
	int i;

	// address lines swap (to do: collapse into one bitswap)
	memcpy(tmp,rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,7,6,5,4,1,2,3,0);
		rom[i] = tmp[addr];
	}

	memcpy(tmp,rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,7,6,5,2,4,3,1,0);
		rom[i] = tmp[addr];
	}

	memcpy(tmp,rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xff) | BITSWAP8(i,7,6,5,3,4,2,1,0);
		rom[i] = BITSWAP8(tmp[addr],7,6,5,4,3,2,1,0);
	}
}

static void tjsb_patch_rom(running_machine *machine)
{
	UINT8 *rom = memory_region(machine, "maincpu");
	rom[0x011df] = 0x18;
}

static DRIVER_INIT( tjsb )
{
	decrypt_program_rom(machine, 0x05, 7, 6, 3, 2, 5, 4, 1, 0);
	tjsb_patch_rom(machine);

	tjsb_decrypt_sprites(machine);
	expand_sprites(machine);
}

// mgcs

static void mgcs_decrypt_program_rom(running_machine *machine)
{
	int i;
	UINT16 *src = (UINT16 *)memory_region(machine, "maincpu");

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++)
	{
		UINT16 x = src[i];

		/* bit 0 xor layer */

		if( i & 0x20/2 )
		{
			if( i & 0x02/2 )
			{
				x ^= 0x0001;
			}
		}

		if( !(i & 0x4000/2) )
		{
			if( !(i & 0x300/2) )
			{
				x ^= 0x0001;
			}
		}

		/* bit 8 xor layer */

		if( (i & 0x2000/2) || !(i & 0x80/2) )
		{
			if( i & 0x100/2 )
			{
				if( !(i & 0x20/2) || (i & 0x400/2) )
				{
					x ^= 0x0100;
				}
			}
		}
		else
		{
			x ^= 0x0100;
		}

		src[i] = x;
	}
}

static void mgcs_decrypt_tiles(running_machine *machine)
{
	int length = memory_region_length(machine, "tilemaps");
	UINT8 *rom = memory_region(machine, "tilemaps");
	UINT8 *tmp = alloc_array_or_die(UINT8, length);
	int i;

	memcpy(tmp,rom,length);
	for (i = 0;i < length;i++)
	{
		int addr = (i & ~0xffff) | BITSWAP16(i,15,14,13,12,11,10,6,7,8,9,5,4,3,2,1,0);
		rom[i] = tmp[addr];
	}

	free(tmp);
}

static void mgcs_flip_sprites(running_machine *machine)
{
	int length = memory_region_length(machine, "sprites");
	UINT8 *rom = memory_region(machine, "sprites");
	int i;

	for (i = 0;i < length;i+=2)
	{
		UINT16 pixels = (rom[i+1] << 8) | rom[i+0];

		// flip bits
		pixels = BITSWAP16(pixels,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);

		// flip pixels
		pixels = BITSWAP16(pixels,15, 0,1,2,3,4, 5,6,7,8,9, 10,11,12,13,14);

		rom[i+0] = pixels;
		rom[i+1] = pixels >> 8;
	}
}

static void mgcs_patch_rom(running_machine *machine)
{
	UINT16 *rom = (UINT16 *)memory_region(machine, "maincpu");

	rom[0x4e036/2] = 0x6006;

	// IGS029 reads the dips?
	rom[0x4e00e/2] = 0x4e75;

	rom[0x4dfce/2] = 0x6010;	// 04DFCE: 6610    bne $4dfe0
	rom[0x20666/2] = 0x601e;	// 020666: 671E    beq $20686 (rom check)
}

static DRIVER_INIT( mgcs )
{
	mgcs_decrypt_program_rom(machine);
	mgcs_patch_rom(machine);

	mgcs_decrypt_tiles(machine);
	mgcs_flip_sprites(machine);

	expand_sprites(machine);
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( iqblocka_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0dfff ) AM_ROM
	AM_RANGE( 0x0e000, 0x0efff ) AM_RAM
	AM_RANGE( 0x0f000, 0x0ffff ) AM_RAM
	AM_RANGE( 0x10000, 0x3ffff ) AM_ROM
ADDRESS_MAP_END

static int nmi_enable;
static WRITE8_HANDLER( nmi_enable_w )
{
	nmi_enable = data & 1;
	if (data & (~1))
		logerror("PC %06X: nmi_enable = %02x\n",cpu_get_pc(space->cpu),data);
}

static int irq_enable;
static WRITE8_HANDLER( irq_enable_w )
{
	irq_enable = data & 1;
	if (data & (~1))
		logerror("PC %06X: irq_enable = %02x\n",cpu_get_pc(space->cpu),data);
}

static UINT8 input_select;
static WRITE8_HANDLER( input_select_w )
{
	input_select = data;
}

static READ8_HANDLER( input_r )
{
	switch (input_select)
	{
		case 0x00:	return input_port_read(space->machine, "PLAYER1");
		case 0x01:	return input_port_read(space->machine, "PLAYER2");
		case 0x02:	return input_port_read(space->machine, "COINS");

		case 0x03:	return 01;

		case 0x20:	return 0x49;
		case 0x21:	return 0x47;
		case 0x22:	return 0x53;

		case 0x24:	return 0x41;
		case 0x25:	return 0x41;
		case 0x26:	return 0x7f;
		case 0x27:	return 0x41;
		case 0x28:	return 0x41;

		case 0x2a:	return 0x3e;
		case 0x2b:	return 0x41;
		case 0x2c:	return 0x49;
		case 0x2d:	return 0xf9;
		case 0x2e:	return 0x0a;

		case 0x30:	return 0x26;
		case 0x31:	return 0x49;
		case 0x32:	return 0x49;
		case 0x33:	return 0x49;
		case 0x34:	return 0x32;

		default:
			logerror("PC %06X: input %02x read\n",cpu_get_pc(space->cpu),input_select);
			return 0xff;
	}
}

static ADDRESS_MAP_START( iqblocka_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs

	AM_RANGE( 0x1000, 0x17ff ) AM_RAM AM_BASE( &spriteram )
	AM_RANGE( 0x1800, 0x1bff ) AM_READWRITE( SMH_RAM, paletteram_xRRRRRGGGGGBBBBB_le_w ) AM_BASE(&paletteram)
	AM_RANGE( 0x1c00, 0x1fff ) AM_RAM

//  AM_RANGE(0x200a, 0x200a) AM_WRITENOP

	AM_RANGE( 0x2010, 0x2013 ) AM_DEVREADWRITE("ppi8255", ppi8255_r, ppi8255_w)

	AM_RANGE( 0x2014, 0x2014 ) AM_WRITE( nmi_enable_w )
	AM_RANGE( 0x2015, 0x2015 ) AM_WRITE( irq_enable_w )

	AM_RANGE( 0x4000, 0x5fff ) AM_RAM_WRITE( fg_w ) AM_BASE( &fg_videoram )
	AM_RANGE( 0x6000, 0x7fff ) AM_RAM_WRITE( bg_w ) AM_BASE( &bg_videoram )

	AM_RANGE( 0x8000, 0x8000 ) AM_WRITE( input_select_w )
	AM_RANGE( 0x8001, 0x8001 ) AM_READ ( input_r )

	AM_RANGE( 0x9000, 0x9000 ) AM_DEVREADWRITE( "oki", okim6295_r, okim6295_w )

	AM_RANGE( 0xa000, 0xa000 ) AM_READ_PORT( "BUTTONS" )

	AM_RANGE( 0xb000, 0xb001 ) AM_DEVWRITE( "ym", ym2413_w )
ADDRESS_MAP_END


// mgcs

static UINT16 igs_magic[2];
static UINT8 scramble_data;

static WRITE16_HANDLER( mgcs_magic_w )
{
	COMBINE_DATA(&igs_magic[offset]);

	if (offset == 0)
		return;

	switch(igs_magic[0])
	{
		case 0x00:
			if (ACCESSING_BITS_0_7)
			{
				input_select = data & 0xff;
			}

			if ( input_select & ~0xfc )
				logerror("%06x: warning, unknown bits written in input_select = %02x\n", cpu_get_pc(space->cpu), input_select);
			break;

		case 0x01:
			if (ACCESSING_BITS_0_7)
			{
				scramble_data = data & 0xff;
			}
			break;

		default:
			logerror("%06x: warning, writing to igs_magic %02x = %02x\n", cpu_get_pc(space->cpu), igs_magic[0], data);
	}
}

static READ16_HANDLER( mgcs_magic_r )
{
	switch(igs_magic[0])
	{
		case 0x01:
			return BITSWAP8(scramble_data, 4,5,6,7, 0,1,2,3);

		default:
			logerror("%06x: warning, reading with igs_magic = %02x\n", cpu_get_pc(space->cpu), igs_magic[0]);
			break;
	}

	return 0xffff;
}

static READ8_DEVICE_HANDLER( mgcs_keys_r )
{
	if (~input_select & 0x08)	return input_port_read(device->machine, "KEY0");
	if (~input_select & 0x10)	return input_port_read(device->machine, "KEY1");
	if (~input_select & 0x20)	return input_port_read(device->machine, "KEY2");
	if (~input_select & 0x40)	return input_port_read(device->machine, "KEY3");
	if (~input_select & 0x80)	return input_port_read(device->machine, "KEY4");

	logerror("%s: warning, reading key with input_select = %02x\n", cpuexec_describe_context(device->machine), input_select);
	return 0xff;
}

static int irq1_enable;
static WRITE16_HANDLER( irq1_enable_w )
{
	if (ACCESSING_BITS_0_7)
		irq1_enable = data & 1;

	if (data != 0 && data != 0xff)
		logerror("PC %06X: irq1_enable = %04x\n",cpu_get_pc(space->cpu),data);
}

static int irq2_enable;
static WRITE16_HANDLER( irq2_enable_w )
{
	if (ACCESSING_BITS_0_7)
		irq2_enable = data & 1;

	if (data != 0 && data != 0xff)
		logerror("PC %06X: irq2_enable = %04x\n",cpu_get_pc(space->cpu),data);
}

static WRITE16_HANDLER( mgcs_paletteram_xRRRRRGGGGGBBBBB_w )
{
	int rgb;

	COMBINE_DATA(&paletteram16[offset]);

	rgb = ((paletteram16[offset/2*2+0] & 0xff) << 8) | (paletteram16[offset/2*2+1] & 0xff);

	// bitswap
	rgb = BITSWAP16(rgb,7,8,9,2,14,3,13,15,12,11,10,0,1,4,5,6);

	palette_set_color_rgb(space->machine, offset/2, pal5bit(rgb >> 0), pal5bit(rgb >> 5), pal5bit(rgb >> 10));
}

static READ16_DEVICE_HANDLER ( mgcs_ppi8255_r )	{	return ppi8255_r(device, offset);	}
static WRITE16_DEVICE_HANDLER( mgcs_ppi8255_w )	{	if (ACCESSING_BITS_0_7)	ppi8255_w(device, offset, data);	}

static ADDRESS_MAP_START( mgcs, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM
	AM_RANGE( 0x300000, 0x303fff ) AM_RAM

	AM_RANGE( 0x49c000, 0x49c003 ) AM_WRITE( mgcs_magic_w )
	AM_RANGE( 0x49c002, 0x49c003 ) AM_READ ( mgcs_magic_r )

	AM_RANGE( 0xa02000, 0xa02fff ) AM_READWRITE( spriteram_lsb_r, spriteram_lsb_w ) AM_BASE( (UINT16**)&spriteram )
	AM_RANGE( 0xa03000, 0xa037ff ) AM_RAM_WRITE( mgcs_paletteram_xRRRRRGGGGGBBBBB_w ) AM_BASE( &paletteram16 )

	AM_RANGE( 0xa04020, 0xa04027 ) AM_DEVREADWRITE("ppi8255", mgcs_ppi8255_r, mgcs_ppi8255_w)

	AM_RANGE( 0xa04028, 0xa04029 ) AM_WRITE( irq2_enable_w )
	AM_RANGE( 0xa0402a, 0xa0402b ) AM_WRITE( irq1_enable_w )

	AM_RANGE( 0xa08000, 0xa0bfff ) AM_READWRITE( fg_lsb_r, fg_lsb_w ) AM_BASE( (UINT16**)&fg_videoram )
	AM_RANGE( 0xa0c000, 0xa0ffff ) AM_READWRITE( bg_lsb_r, bg_lsb_w ) AM_BASE( (UINT16**)&bg_videoram )

	// oki banking?
	AM_RANGE( 0xa12000, 0xa12001 ) AM_DEVREADWRITE8( "oki", okim6295_r, okim6295_w, 0x00ff )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( iqblocka )

	PORT_START("DSW0")	// IN0
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Hold Mode" )
	PORT_DIPSETTING(    0x02, "In Win" )
	PORT_DIPSETTING(    0x00, "Always" )
	PORT_DIPNAME( 0x04, 0x04, "Max Credit" )
	PORT_DIPSETTING(    0x04, "4000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x38, 0x38, "Cigarette Bet" )
	PORT_DIPSETTING(    0x38, "1" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0x28, "20" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x18, "80" )
	PORT_DIPSETTING(    0x10, "100" )
	PORT_DIPSETTING(    0x08, "120" )
	PORT_DIPSETTING(    0x00, "150" )
	PORT_DIPNAME( 0xc0, 0xc0, "Min Bet" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x00, "50" )

	PORT_START("DSW1")	// IN1
	PORT_DIPNAME( 0x07, 0x07, "Key In" )
	PORT_DIPSETTING(    0x07, "10" )
	PORT_DIPSETTING(    0x06, "20" )
	PORT_DIPSETTING(    0x05, "40" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "250" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x08, 0x08, "Key Out" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Open Mode" )
	PORT_DIPSETTING(    0x10, "Gaming" )
	PORT_DIPSETTING(    0x00, "Amuse" )
	PORT_DIPNAME( 0x20, 0x20, "Demo Game" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Bonus Base" )
	PORT_DIPSETTING(    0xc0, "100" )
	PORT_DIPSETTING(    0x80, "200" )
	PORT_DIPSETTING(    0x40, "300" )
	PORT_DIPSETTING(    0x00, "400" )

	PORT_START("DSW2")	// IN2
	PORT_DIPNAME( 0x03, 0x03, "Win Up Pool" )
	PORT_DIPSETTING(    0x03, "300" )
	PORT_DIPSETTING(    0x02, "500" )
	PORT_DIPSETTING(    0x01, "800" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Max Double Up" )
	PORT_DIPSETTING(    0x0c, "20000" )
	PORT_DIPSETTING(    0x08, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x10, 0x10, "Cards" )
	PORT_DIPSETTING(    0x10, "A,J,Q,K" )
	PORT_DIPSETTING(    0x00, "Number" )
	PORT_DIPNAME( 0x20, 0x20, "Title Name" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Double" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "CG Select" )
	PORT_DIPSETTING(    0x80, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_START("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         )	PORT_NAME( "Start / Test" )	// keep pressed while booting
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME( "Help / Big" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         )	PORT_NAME( "Bet" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1     ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2     ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW  )	// keep pressed while booting
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )	// this seems to toggle between videogame and gambling

	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Small" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3   ) PORT_IMPULSE(2)	// coin error
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME( "Score -> Time" )	// converts score into time
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( mgcs )

	// DSWs don't work: they are read through a protection device (IGS029? see code at 1CF16)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Credits Per Note" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Max Note Credits" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x40, 0x40, "Pay Out Type" )
	PORT_DIPSETTING(    0x40, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0x80, 0x80, "Double Up Limit" )
	PORT_DIPSETTING(    0x80, "1000" )
	PORT_DIPSETTING(    0x00, "2000" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Min Bet" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x04, 0x04, "Double Up" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Type" )
	PORT_DIPSETTING(    0x10, "Double" )
	PORT_DIPSETTING(    0x00, DEF_STR( Single ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Controls ) )
	PORT_DIPSETTING(    0x20, "Keyboard" )
	PORT_DIPSETTING(    0x00, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x40, 0x40, "Number Type" )
	PORT_DIPSETTING(    0x40, "Number" )
	PORT_DIPSETTING(    0x00, "Tile" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Number" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	// the top 2 bits of COINS and KEYx are read and combined (see code at 1C83A)

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE2 ) // ? (shown in service mode)
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW ) // service mode (keep pressed during boot too)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Statistics")	// press with the above for sound test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1    ) PORT_IMPULSE(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Pay Out") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) // ? (shown in service mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x4 =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8*3,8*2,8*1,8*0 },
	{ STEP8(0, 1) },
	{ STEP8(0, 8*4) },
	8*8*4
};

static const gfx_layout layout_8x8x4_flipped =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 8*2,8*3,8*0,8*1 },
	{ STEP8(7, -1) },
	{ STEP8(0, 8*4) },
	8*8*4
};

static GFXDECODE_START( igs017_flipped )
	GFXDECODE_ENTRY( "tilemaps", 0, layout_8x8x4_flipped, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( igs017 )
	GFXDECODE_ENTRY( "tilemaps", 0, layout_8x8x4, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static INTERRUPT_GEN( iqblocka_interrupt )
{
	if (cpu_getiloops(device) & 1)
	{
		 if (nmi_enable)
			cpu_set_input_line(device, INPUT_LINE_NMI, PULSE_LINE);
	}
	else
	{
		 if (irq_enable)
			cpu_set_input_line(device, 0, HOLD_LINE);
	}
}

// Dips are read through the 8255
static const ppi8255_interface iqblocka_ppi8255_intf =
{
	DEVCB_INPUT_PORT("DSW0"),	// Port A read
	DEVCB_INPUT_PORT("DSW1"),	// Port B read
	DEVCB_INPUT_PORT("DSW2"),	// Port C read

	DEVCB_NULL,						// Port A write
	DEVCB_NULL,						// Port B write
	DEVCB_NULL,						// Port C write
};

static MACHINE_RESET( iqblocka )
{
	nmi_enable = 0;
	irq_enable = 0;
}

static MACHINE_DRIVER_START( iqblocka )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_16MHz / 2)
	MDRV_CPU_PROGRAM_MAP(iqblocka_map)
	MDRV_CPU_IO_MAP(iqblocka_io)
	MDRV_CPU_VBLANK_INT_HACK(iqblocka_interrupt,2)

	MDRV_PPI8255_ADD( "ppi8255", iqblocka_ppi8255_intf )

	MDRV_MACHINE_RESET(iqblocka)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)

	MDRV_GFXDECODE(igs017)
	MDRV_PALETTE_LENGTH(0x100*2)

	MDRV_VIDEO_START(igs017)
	MDRV_VIDEO_UPDATE(igs017)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym", YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_16MHz / 16)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END



// mgcs

static INTERRUPT_GEN( mgcs_interrupt )
{
	if (cpu_getiloops(device) & 1)
	{
		 if (irq2_enable)
			cpu_set_input_line(device, 2, HOLD_LINE);
	}
	else
	{
		 if (irq1_enable)
			cpu_set_input_line(device, 1, HOLD_LINE);
	}
}

static MACHINE_RESET( mgcs )
{
	irq1_enable = 0;
	irq2_enable = 0;
}

static const ppi8255_interface mgcs_ppi8255_intf =
{
	DEVCB_INPUT_PORT("COINS"),		// Port A read
	DEVCB_HANDLER(mgcs_keys_r),		// Port B read
	DEVCB_NULL,						// Port C read (see code at 1C83A)

	DEVCB_NULL,						// Port A write
	DEVCB_NULL,						// Port B write
	DEVCB_NULL,						// Port C write (with 0/1)
};

static MACHINE_DRIVER_START( mgcs )
	MDRV_CPU_ADD("maincpu", M68000, XTAL_22MHz / 2)
	MDRV_CPU_PROGRAM_MAP(mgcs)
	MDRV_CPU_VBLANK_INT_HACK(mgcs_interrupt,2)

	MDRV_MACHINE_RESET(mgcs)

	MDRV_PPI8255_ADD( "ppi8255", mgcs_ppi8255_intf )

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)

	MDRV_GFXDECODE(igs017_flipped)
	MDRV_PALETTE_LENGTH(0x100*2)

	MDRV_VIDEO_START(igs017)
	MDRV_VIDEO_UPDATE(igs017)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_8MHz / 8)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/*

IQ Block (alt hardware)
IGS, 1996

PCB Layout
----------

IGS PCB N0- 0131-4
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                               AR17961 |
|   HD64180RP8                          |
|  16MHz                         BATTERY|
|                                       |
|                         SPEECH.U17    |
|                                       |
|J                        6264          |
|A                                      |
|M      8255              V.U18         |
|M                                      |
|A                                      |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |        |
|       CG.U7          |IGS017 |        |
|                      |       |        |
|       TEXT.U8        |-------|   PAL  |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.31kHz

*/

ROM_START( iqblocka )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v.u18", 0x00000, 0x40000, CRC(2e2b7d43) SHA1(cc73f4c8f9a6e2219ee04c9910725558a80b4eb2) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0)
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )
ROM_END

ROM_START( iqblockf )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "v113fr.u18", 0x00000, 0x40000, CRC(346c68af) SHA1(ceae4c0143c288dc9c1dd1e8a51f1e3371ffa439) )

	ROM_REGION( 0x80000, "sprites", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sp.u17", 0x00000, 0x40000, CRC(71357845) SHA1(25f4f7aebdcc0706018f041d3696322df569b0a3) )
ROM_END

/*

Mahjong Tian Jiang Shen Bing
IGS, 1997

This PCB is almost the same as IQBlock (IGS, 1996)
but the 8255 has been replaced with the IGS025 IC

PCB Layout
----------

IGS PCB N0- 0157-2
|---------------------------------------|
|uPD1242H     VOL    U3567   3.579545MHz|
|                         AR17961       |
|   HD64180RP8                   SPDT_SW|
|  16MHz                         BATTERY|
|                                       |
|                         S0703.U15     |
|                                       |
|J     |-------|          6264          |
|A     |       |                        |
|M     |IGS025 |          P0700.U16     |
|M     |       |                        |
|A     |-------|                        |
|                                       |
|                                       |
|                      |-------|        |
|                      |       |   PAL  |
|       A0701.U3       |IGS017 |        |
|                      |       |   PAL  |
|       TEXT.U6        |-------|        |
|            22MHz               61256  |
|                   DSW1  DSW2  DSW3    |
|---------------------------------------|
Notes:
      HD64180RP8 - Hitachi HD64180 CPU. Clocks 16MHz (pins 2 & 3), 8MHz (pin 64)
      61256   - 32k x8 SRAM (DIP28)
      6264    - 8k x8 SRAM (DIP28)
      IGS017  - Custom IGS IC (QFP208)
      IGS025  - Custom IGS IC (PLCC68)
      AR17961 - == Oki M6295 (QFP44). Clock 1.000MHz [16/16]. pin 7 = high
      U3567   - == YM2413. Clock 3.579545MHz
      VSync   - 60Hz
      HSync   - 15.30kHz

*/

ROM_START( tjsb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "p0700.u16", 0x00000, 0x40000,CRC(1b2a50df) SHA1(95a272e624f727df9523667864f933118d9e633c) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD( "a0701.u3", 0x000000, 0x200000, CRC(aa182140) SHA1(37c2053386c183ff726ba417d13f2063cf9a22df) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u6", 0x00000, 0x80000,  CRC(3be886b8) SHA1(15b3624ed076640c1828d065b01306a8656f5a9b) )	// BADADDR     --xxxxxxxxxxxxxxxxx

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0703.u15", 0x00000, 0x80000,  CRC(c6f94d29) SHA1(ec413580240711fc4977dd3c96c288501aa7ef6c) )
ROM_END

/*

Mahjong Man Guan Cai Shen
IGS, 1998


PCB Layout
----------

IGS PCB NO-0192-1
|---------------------------------------|
|              JAMMA            UPC1242 |
|                                       |
|               S1502.U10               |
|                          K668    VOL  |
|                                       |
|                                       |
|                       22MHz           |
|1     61256                            |
|8              |-------|      TEXT.U25 |
|W     PAL      |       |               |
|A              |IGS017 |               |
|Y              |       |      M1501.U23|
|               |-------|               |
|   |-------|                           |
|   |       |                           |
|   |IGS025 |   P1500.U8                |
|   |       |              PAL    6264  |
|1  |-------|                           |
|0  |----|                 PAL    6264  |
|W  |IGS |                 PAL          |
|A  |029 |  8MHz                 SPDT_SW|
|Y  |----|                 68000        |
|T DSW1  DSW2                   BATTERY |
|---------------------------------------|
Notes:
      Uses JAMMA & common 10-way/18-way Mahjong pinout
      68000 clock 11.000MHz [22/2]
      K668  == Oki M6295 (QFP44). Clock 1MHz [8/8]. pin7 = High
      VSync - 60Hz
      HSync - 15.3kHz

*/

ROM_START( mgcs )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "p1500.u8", 0x00000, 0x80000, CRC(a8cb5905) SHA1(37be7d926a1352869632d43943763accd4dec4b7) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD( "m1501.u23", 0x000000, 0x400000, CRC(96fce058) SHA1(6b87f47d646bad9b3061bdc8a9af65467fdbbc9f) )	// FIXED BITS (xxxxxxx0xxxxxxxx)

	ROM_REGION( 0x80000, "tilemaps", 0 )
	ROM_LOAD( "text.u25", 0x00000, 0x80000, CRC(a37f9613) SHA1(812f060ca98a34540c48a180c359c3d0f1c0b5bb) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s1502.u10", 0x00000, 0x80000, CRC(a8a6ba58) SHA1(59276a8ab4a31812600816c2a43b74bd71394419) )
ROM_END

GAME( 1996, iqblocka, iqblock, iqblocka, iqblocka, iqblocka, ROT0, "IGS", "Shu Zi Le Yuan (V127M)",             GAME_NOT_WORKING )
GAME( 1996, iqblockf, iqblock, iqblocka, iqblocka, iqblockf, ROT0, "IGS", "Shu Zi Le Yuan (V113FR)",            GAME_NOT_WORKING )
GAME( 1997, tjsb,     0,       iqblocka, iqblocka, tjsb,     ROT0, "IGS", "Mahjong Tian Jiang Shen Bing",       GAME_NOT_WORKING )
GAME( 1998, mgcs,     0,       mgcs,     mgcs,     mgcs,     ROT0, "IGS", "Mahjong Man Guan Cai Shen (V103CS)", GAME_NOT_WORKING )
