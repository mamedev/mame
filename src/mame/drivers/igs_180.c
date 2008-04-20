/***************************************************************************

                        -= IGS017 Based Hardware =-

        driver by   Pierpaolo Prazzoli, Luca Elia (l.elia@tin.it)

CPU:    Z180 (HD64180)
GFX:    IGS017 (2 tilemaps, variable size sprites, protection)
SOUND:  M6295, YM2413
OTHER:  IGS025

To Do:

- Protection emulation, instead of patching the roms.
- tjsb: finish sprites decryption (data lines scrambling), protection, inputs.
- iqblocka: NVRAM.
- iqblockf: protection.

Notes:

- iqblocka: keep start or test pressed during boot to enter test mode A or B.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
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

static VIDEO_START(igs_180)
{
	fg_tilemap = tilemap_create(get_fg_tile_info,tilemap_scan_rows,8,8,64,32);
	bg_tilemap = tilemap_create(get_bg_tile_info,tilemap_scan_rows,8,8,64,32);

	tilemap_set_transparent_pen(fg_tilemap,0xf);
	tilemap_set_transparent_pen(bg_tilemap,0xf);
}


static UINT8 *sprites_gfx;
int sprites_gfx_size;

// Eeach 16 bit word in the sprites gfx roms contains three 5 bit pens: x-22222-11111-00000.
// This routine expands each word into three bytes.
static void expand_sprites(void)
{
	UINT8 *rom	=	memory_region(REGION_GFX1);
	int size	=	memory_region_length(REGION_GFX1);
	int i;

	sprites_gfx_size	=	size / 2 * 3;
	sprites_gfx			=	auto_malloc(sprites_gfx_size);

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
                    ---- 3210			Code (high)

	Code = ROM Address / 2 = Pixel / 3

***************************************************************************/

static void draw_sprite(bitmap_t *bitmap,const rectangle *cliprect, int sx, int sy, int dimx, int dimy, int flipx, int flipy, int color, int addr)
{
	// prepare GfxElement on the fly
	gfx_element gfx;

	gfx.width = dimx;
	gfx.height = dimy;
	gfx.total_elements = 1;
	gfx.color_depth = 32;
	gfx.color_granularity = 32;
	gfx.color_base = 0x100;
	gfx.total_colors = 8;
	gfx.pen_usage = NULL;
	gfx.gfxdata = sprites_gfx + addr;
	gfx.line_modulo = dimx;
	gfx.char_modulo = 0;	// doesn't matter
	gfx.flags = 0;

	// Bounds checking
	if ( addr + dimx * dimy >= sprites_gfx_size )
		return;

	drawgfx(	bitmap,&gfx,
				0, color,
				flipx, flipy,
				sx, sy,
				cliprect, TRANSPARENCY_PEN, 0x1f	);
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
		addr	=	(s[4] >> 6) | (s[5] << 2) | (s[6] << 10) | ((s[7] & 0x03) << 18);
		addr	*=	3;

		flipx	=	s[7] & 0x10;
		flipy	=	0;

		dimx	=	((((s[4] & 0x3f)<<2) | ((s[3] & 0xc0)>>6))+1) * 3;
		dimy	=	((y >> 10) | ((x & 0x03)<<6))+1;

		x		>>=	3;
		sx		=	(x & 0x1ff) - (x & 0x200);
		sy		=	(y & 0x1ff) - (y & 0x200);

		color = (s[7] & 0xe0) >> 5;

		draw_sprite(bitmap, cliprect, sx, sy, dimx, dimy, flipx, flipy, color, addr);
	}
}

// A simple gfx viewer (toggle with T)
static int debug_viewer(bitmap_t *bitmap,const rectangle *cliprect)
{
#ifdef MAME_DEBUG
	static int toggle = 0;
	if (input_code_pressed_once(KEYCODE_T))	toggle = 1-toggle;
	if (toggle)	{
		static int a = 0, w = 512;
		int h = 256;

		if (input_code_pressed(KEYCODE_O))		w += 1;
		if (input_code_pressed(KEYCODE_I))		w -= 1;

		if (input_code_pressed(KEYCODE_U))		w += 8;
		if (input_code_pressed(KEYCODE_Y))		w -= 8;

		if (input_code_pressed(KEYCODE_RIGHT))	a += 1;
		if (input_code_pressed(KEYCODE_LEFT))	a -= 1;

		if (input_code_pressed(KEYCODE_DOWN))	a += w;
		if (input_code_pressed(KEYCODE_UP))		a -= w;

		if (input_code_pressed(KEYCODE_PGDN))	a += w * h;
		if (input_code_pressed(KEYCODE_PGUP))	a -= w * h;

		if (a < 0)		a = 0;
		if (a > sprites_gfx_size)	a = sprites_gfx_size;

		if (w <= 0)		w = 0;
		if (w > 1024)	w = 1024;

		fillbitmap(bitmap,0,cliprect);

		draw_sprite(bitmap, cliprect, 0,0, w,h, 0,0, 0, a);

		popmessage("a: %08X w: %03X p: %02X-%02x-%02x",a,w,sprites_gfx[a/3*3+0],sprites_gfx[a/3*3+1],sprites_gfx[a/3*3+2]);
		osd_sleep(200000);
		return 1;
	}
#endif
	return 0;
}

static VIDEO_UPDATE(igs_180)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (input_code_pressed(KEYCODE_Z))
	{
		int mask = 0;
		if (input_code_pressed(KEYCODE_Q))	mask |= 1;
		if (input_code_pressed(KEYCODE_W))	mask |= 2;
		if (input_code_pressed(KEYCODE_A))	mask |= 4;
		if (mask != 0) layers_ctrl &= mask;
	}
#endif
	
	if (debug_viewer(bitmap,cliprect))
		return 0;

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect,bg_tilemap,TILEMAP_DRAW_OPAQUE,0);
	else					fillbitmap(bitmap,get_black_pen(screen->machine),cliprect);

	if (layers_ctrl & 4)	draw_sprites(screen->machine,bitmap,cliprect);

	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect,fg_tilemap,0,0);

	return 0;
}

/***************************************************************************
                                Decryption
***************************************************************************/

static void decrypt_program_rom(int mask, int a7, int a6, int a5, int a4, int a3, int a2, int a1, int a0)
{
	int length = memory_region_length(REGION_CPU1);
	UINT8 *rom = memory_region(REGION_CPU1);
	UINT8 *tmp = auto_malloc(length);
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

static void iqblocka_patch_rom(void)
{
	UINT8 *rom = memory_region(REGION_CPU1);

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
	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
	iqblocka_patch_rom();

	expand_sprites();
}

// iqblockf

static DRIVER_INIT( iqblockf )
{
	decrypt_program_rom(0x11, 7, 6, 5, 4, 3, 2, 1, 0);
//  iqblockf_patch_rom();

	expand_sprites();
}

// tjsb

static void tjsb_decrypt_sprites(void)
{
	int length = memory_region_length(REGION_GFX1);
	UINT8 *rom = memory_region(REGION_GFX1);
	UINT8 *tmp = auto_malloc(length);
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

static void tjsb_patch_rom(void)
{
	UINT8 *rom = memory_region(REGION_CPU1);
	rom[0x011df] = 0x18;
}

static DRIVER_INIT( tjsb )
{
	decrypt_program_rom(0x05, 7, 6, 3, 2, 5, 4, 1, 0);
	tjsb_patch_rom();

	tjsb_decrypt_sprites();
	expand_sprites();
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static ADDRESS_MAP_START( igs_180_map, ADDRESS_SPACE_PROGRAM, 8 )
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
		logerror("PC %06X: nmi_enable = %02x\n",activecpu_get_pc(),data);
}

static int irq_enable;
static WRITE8_HANDLER( irq_enable_w )
{
	irq_enable = data & 1;
	if (data & (~1))
		logerror("PC %06X: irq_enable = %02x\n",activecpu_get_pc(),data);
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
		case 0x00:	return input_port_read(machine, "PLAYER1");
		case 0x01:	return input_port_read(machine, "PLAYER2");
		case 0x02:	return input_port_read(machine, "COINS");

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
			logerror("PC %06X: input %02x read\n",activecpu_get_pc(),input_select);
			return 0xff;
	}
}

static ADDRESS_MAP_START( igs_180_portmap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs

	AM_RANGE( 0x1000, 0x17ff ) AM_RAM AM_BASE( &spriteram )
	AM_RANGE( 0x1800, 0x1bff ) AM_READWRITE( SMH_RAM, paletteram_xRRRRRGGGGGBBBBB_le_w ) AM_BASE(&paletteram)
	AM_RANGE( 0x1c00, 0x1fff ) AM_RAM

//  AM_RANGE(0x200a, 0x200a) AM_WRITENOP

	AM_RANGE( 0x2010, 0x2013 ) AM_DEVREADWRITE(PPI8255, "ppi8255", ppi8255_r, ppi8255_w)

	AM_RANGE( 0x2014, 0x2014 ) AM_WRITE( nmi_enable_w )
	AM_RANGE( 0x2015, 0x2015 ) AM_WRITE( irq_enable_w )

	AM_RANGE( 0x4000, 0x5fff ) AM_READWRITE( SMH_RAM, fg_w ) AM_BASE( &fg_videoram )
	AM_RANGE( 0x6000, 0x7fff ) AM_READWRITE( SMH_RAM, bg_w ) AM_BASE( &bg_videoram )

	AM_RANGE( 0x8000, 0x8000 ) AM_WRITE( input_select_w )
	AM_RANGE( 0x8001, 0x8001 ) AM_READ ( input_r )

	AM_RANGE( 0x9000, 0x9000 ) AM_READWRITE( OKIM6295_status_0_r, OKIM6295_data_0_w )

	AM_RANGE( 0xa000, 0xa000 ) AM_READ_PORT( "BUTTONS" )

	AM_RANGE( 0xb000, 0xb000 ) AM_WRITE( YM2413_register_port_0_w )
	AM_RANGE( 0xb001, 0xb001 ) AM_WRITE( YM2413_data_port_0_w )
ADDRESS_MAP_END


/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( iqblocka )

	PORT_START_TAG("DSW0")	// IN0
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

	PORT_START_TAG("DSW1")	// IN1
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

	PORT_START_TAG("DSW2")	// IN2
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

	PORT_START_TAG("PLAYER1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1         )	PORT_NAME( "Start / Test" )	// keep pressed while booting
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_NAME( "Help / Big" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START_TAG("PLAYER2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2         )	PORT_NAME( "Bet" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START_TAG("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1     ) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2     ) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN   )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW  )	// keep pressed while booting
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 )	// this seems to toggle between videogame and gambling

	PORT_START_TAG("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Small" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3   ) PORT_IMPULSE(2)	// coin error
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME( "Score -> Time" )	// converts score into time
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
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

static GFXDECODE_START( igs_180 )
	GFXDECODE_ENTRY( REGION_GFX2, 0, layout_8x8x4, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static INTERRUPT_GEN( igs_180_interrupt )
{
	if (cpu_getiloops() & 1)
	{
		 if (nmi_enable)
			cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, PULSE_LINE);
	}
	else
	{
		 if (irq_enable)
			cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
	}
}

// Dips are read through the 8255
static const ppi8255_interface ppi8255_intf =
{
	input_port_0_r,	// Port A read
	input_port_1_r,	// Port B read
	input_port_2_r,	// Port C read
	0,				// Port A write
	0,				// Port B write
	0,				// Port C write
};

static MACHINE_RESET( igs_180 )
{
	nmi_enable = 0;
	irq_enable = 0;
}

static MACHINE_DRIVER_START( igs_180 )

	/* basic machine hardware */
	MDRV_CPU_ADD(Z180, XTAL_16MHz / 2)
	MDRV_CPU_PROGRAM_MAP(igs_180_map,0)
	MDRV_CPU_IO_MAP(igs_180_portmap,0)
	MDRV_CPU_VBLANK_INT_HACK(igs_180_interrupt,2)

	MDRV_DEVICE_ADD( "ppi8255", PPI8255 )
	MDRV_DEVICE_CONFIG( ppi8255_intf )

	MDRV_MACHINE_RESET(igs_180)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0, 240-1)

	MDRV_GFXDECODE(igs_180)
	MDRV_PALETTE_LENGTH(0x100*2)

	MDRV_VIDEO_START(igs_180)
	MDRV_VIDEO_UPDATE(igs_180)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

	MDRV_SOUND_ADD(OKIM6295, XTAL_16MHz / 16)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_DRIVER_END


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
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD( "v.u18", 0x00000, 0x40000, CRC(2e2b7d43) SHA1(cc73f4c8f9a6e2219ee04c9910725558a80b4eb2) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE)
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )
	ROM_LOAD( "speech.u17", 0x00000, 0x40000, CRC(d9e3d39f) SHA1(bec85d1ac2dfca77453cbca0e7dd53fee8fb438b) )
ROM_END

ROM_START( iqblockf )
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD( "v113fr.u18", 0x00000, 0x40000, CRC(346c68af) SHA1(ceae4c0143c288dc9c1dd1e8a51f1e3371ffa439) )

	ROM_REGION( 0x80000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "cg.u7", 0x000000, 0x080000, CRC(cb48a66e) SHA1(6d597193d1333a97957d5ceec8179a24bedfd928) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "text.u8", 0x000000, 0x080000, CRC(48c4f4e6) SHA1(b1e1ca62cf6a99c11a5cc56705eef7e22a3b2740) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )
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
	ROM_REGION( 0x40000, REGION_CPU1, 0 )
	ROM_LOAD( "p0700.u16", 0x00000, 0x40000,CRC(1b2a50df) SHA1(95a272e624f727df9523667864f933118d9e633c) )

	ROM_REGION( 0x200000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "a0701.u3", 0x000000, 0x200000, CRC(aa182140) SHA1(37c2053386c183ff726ba417d13f2063cf9a22df) )	// FIXED BITS (xxxxxxxx0xxxxxxx)

	ROM_REGION( 0x80000, REGION_GFX2, ROMREGION_DISPOSE )
	ROM_LOAD( "text.u6", 0x000000, 0x080000,  CRC(3be886b8) SHA1(15b3624ed076640c1828d065b01306a8656f5a9b) )	// BADADDR     --xxxxxxxxxxxxxxxxx

	ROM_REGION( 0x80000, REGION_SOUND1, 0 )
	ROM_LOAD( "s0703.u15", 0x00000, 0x80000,  CRC(c6f94d29) SHA1(ec413580240711fc4977dd3c96c288501aa7ef6c) )
ROM_END


GAME( 1996, iqblocka, iqblock, igs_180, iqblocka, iqblocka, ROT0, "IGS", "Shu Zi Le Yuan (V127M)",       GAME_NOT_WORKING )
GAME( 1996, iqblockf, iqblock, igs_180, iqblocka, iqblockf, ROT0, "IGS", "Shu Zi Le Yuan (V113FR)",      GAME_NOT_WORKING )
GAME( 1997, tjsb,     0,       igs_180, iqblocka, tjsb,     ROT0, "IGS", "Mahjong Tian Jiang Shen Bing", GAME_NOT_WORKING )
