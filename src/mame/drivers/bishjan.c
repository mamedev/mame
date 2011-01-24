/************************************************************************************************************

                                 -= Subsino (Newer) Tilemaps Hardware =-

                                  driver by   Luca Elia (l.elia@tin.it)


Two 1024x512 tilemaps. 256 color tiles. Tiles are 8x8 or a multiple (dynamic tile size).
There is RAM for 512 scroll values (line scroll). Video RAM is mirrored on multiple ranges.
One peculiarity is that video RAM access is split into high and low byte. The former is mapped
in program space, the latter in I/O space.

-------------------------------------------------------------------------------------------------------------
Year  Game            CPU         Sound            Custom                            Other
-------------------------------------------------------------------------------------------------------------
1997  Magic Train     HD647180*   U6295            SS9601, SS9602                    HM86171 RAMDAC, Battery
1998  Ying Hua Lian   AM188-EM    M6295 + YM3812?  SS9601, SS9602                    HM86171 RAMDAC, Battery
1999  Bishou Jan      H8/3044     SS9904?          SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
2006  X-Plan          AM188-EM    M6295            SS9601, SS9802, SS9803            HM86171 RAMDAC, Battery
-------------------------------------------------------------------------------------------------------------
*SS9600

To do:

- saklove, xplan: remove IRQ hacks (when an AM188-EM core will be available).
- bishjan, saklove: game is sometimes too fast (can bishjan read the VBLANK state? saklove and xplan can).
- bishjan, saklove, xplan: implement serial communication (used for protection?)
- bishjan: add sound (does it send sound commands to another device? SS9904?).
- mtrain: finish decryption (a variant of that in crsbingo?)
- xplan: rouge tiles are left on the screen

************************************************************************************************************/

#include "emu.h"
#include "cpu/h83002/h8.h"
#include "cpu/i86/i86.h"
#include "cpu/z180/z180.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "machine/nvram.h"

static UINT8 *bishjan_colorram;

/***************************************************************************
                              Tilemaps Access
***************************************************************************/

enum tilesize_t
{
	TILE_8x8,
	TILE_8x32,
	TILE_64x32
};

enum vram_t
{
	VRAM_LO,
	VRAM_HI
};

// Layers
static struct layer_t
{

	UINT8 *videorams[2];

	UINT8 *scrollrams[2];
	int scroll_x;
	int scroll_y;

	tilemap_t *tmap;
	tilesize_t tilesize;

} layers[2];


INLINE void get_tile_info(layer_t *l, running_machine *machine, tile_data *tileinfo, tilemap_memory_index tile_index, void *param)
{
	int addr;
	UINT16 offs;
	switch (l->tilesize)
	{
		default:
		case TILE_8x8:		addr = tile_index;				offs = 0;												break;
		case TILE_8x32:		addr = tile_index & (~0x180);	offs = (tile_index/0x80) & 3;							break;
		case TILE_64x32:	addr = tile_index & (~0x187);	offs = ((tile_index/0x80) & 3) + (tile_index & 7) * 4;	break;
	}
	SET_TILE_INFO(0, (l->videorams[VRAM_HI][addr] << 8) + l->videorams[VRAM_LO][addr] + offs, 0, 0);
}

// Layer 0
static TILE_GET_INFO( get_tile_info_0 )	{	get_tile_info(&layers[0], machine, tileinfo, tile_index, param);	}
// Layer 1
static TILE_GET_INFO( get_tile_info_1 )	{	get_tile_info(&layers[1], machine, tileinfo, tile_index, param);	}


static UINT8 bishjan_byte_lo;
static WRITE8_HANDLER( bishjan_byte_lo_w )
{
	bishjan_byte_lo = data;
}

INLINE void bishjan_videoram_w(layer_t *l, vram_t vram, address_space *space, offs_t offset, UINT8 data)
{
	l->videorams[vram][offset] = data;

	switch (l->tilesize)
	{
		default:
		case TILE_8x8:
			tilemap_mark_tile_dirty(l->tmap, offset);
			break;

		case TILE_8x32:
			offset &= ~0x180;
			for (int y = 0; y < 0x80*4; y += 0x80)
				tilemap_mark_tile_dirty(l->tmap, offset + y);
			break;

		case TILE_64x32:
			offset &= ~0x187;
			for (int x = 0; x < 8; x++)
				for (int y = 0; y < 0x80*4; y += 0x80)
					tilemap_mark_tile_dirty(l->tmap, offset + y + x);
			break;
	}
}

// Layer 0
static WRITE8_HANDLER( bishjan_videoram_0_hi_w )	{	bishjan_videoram_w(&layers[0], VRAM_HI, space, offset, data);				}
static WRITE8_HANDLER( bishjan_videoram_0_lo_w )	{	bishjan_videoram_w(&layers[0], VRAM_LO, space, offset, data);				}
static WRITE8_HANDLER( bishjan_videoram_0_hi_lo_w )	{	bishjan_videoram_w(&layers[0], VRAM_HI, space, offset, data);
														bishjan_videoram_w(&layers[0], VRAM_LO, space, offset, bishjan_byte_lo);	}
static READ8_HANDLER( bishjan_videoram_0_hi_r )		{	return layers[0].videorams[VRAM_HI][offset];								}
static READ8_HANDLER( bishjan_videoram_0_lo_r )		{	return layers[0].videorams[VRAM_LO][offset];								}

// Layer 1
static WRITE8_HANDLER( bishjan_videoram_1_hi_w )	{	bishjan_videoram_w(&layers[1], VRAM_HI, space, offset, data);				}
static WRITE8_HANDLER( bishjan_videoram_1_lo_w )	{	bishjan_videoram_w(&layers[1], VRAM_LO, space, offset, data);				}
static WRITE8_HANDLER( bishjan_videoram_1_hi_lo_w )	{	bishjan_videoram_w(&layers[1], VRAM_HI, space, offset, data);
														bishjan_videoram_w(&layers[1], VRAM_LO, space, offset, bishjan_byte_lo);	}
static READ8_HANDLER( bishjan_videoram_1_hi_r )		{	return layers[1].videorams[VRAM_HI][offset];								}
static READ8_HANDLER( bishjan_videoram_1_lo_r )		{	return layers[1].videorams[VRAM_LO][offset];								}


/***************************************************************************
                              Tilemaps Tile Size

xplan:

80 = 00     40 = FD     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 70     40 = FF     L0 = 64x32  L1 = 8x8    ; shoot'em up demo / gambling demo
80 = 40     40 = BF     L0 = 8x32   L1 = 8x8    ; title screen [L0 line scroll] / 3 planes with scrolling clouds (before title screen)
80 = 00     40 = EF     L0 = 8x8    L1 = 8x8    ; parachutist and cars demo [L1 line scroll]
80 = 00     40 = FF     L0 = 8x8    L1 = 8x8    ; test mode and stat screens

bishjan:

80 = 00     40 = FD     L0 = -      L1 = 8x8    ; ram test (disable = 01 -> L0 disabled)
80 = 00     40 = 0F     L0 = 8x8    L1 = 8x8    ; soft dsw screen
80 = 00     40 = 0x     L0 = 8x8    L1 = 8x8    ; stat screens (40 = 07/0d, seems a don't care)

***************************************************************************/

UINT8 bishjan_tilesize;
UINT8 bishjan_tile_unk;

// These are written in sequence
static WRITE8_HANDLER( bishjan_tile_unk_w )
{
	bishjan_tile_unk = data;
}

static WRITE8_HANDLER( bishjan_tilesize_w )
{
	bishjan_tilesize = data;

	tilesize_t sizes[2];
	switch (data)
	{
		case 0x00:
			sizes[0] = TILE_8x8;
			sizes[1] = TILE_8x8;
			break;

		case 0x40:
			sizes[0] = TILE_8x32;
			sizes[1] = TILE_8x8;
			break;

		case 0x70:
			sizes[0] = TILE_64x32;
			sizes[1] = TILE_8x8;
			break;

		default:
			sizes[0] = TILE_8x8;
			sizes[1] = TILE_8x8;

			logerror("%s: warning, unknown tilesize = %02x\n", cpuexec_describe_context(space->machine), data);
			popmessage("UNKNOWN TILESIZE %02X", data);
			break;
	}

	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &layers[i];

		if (l->tilesize != sizes[i])
		{
			l->tilesize = sizes[i];
			tilemap_mark_all_tiles_dirty(l->tmap);
		}
	}
}

/***************************************************************************
                              Tilemaps Scroll
***************************************************************************/

static WRITE8_HANDLER( bishjan_scroll_w )
{
	switch ( offset )
	{
		// Layer 0
		case 0:	layers[0].scroll_x = (layers[0].scroll_x & 0xf00) | data;					break;	// x low
		case 1:	layers[0].scroll_y = (layers[0].scroll_y & 0xf00) | data;					break;	// y low
		case 2:	layers[0].scroll_x = (layers[0].scroll_x & 0x0ff) | ((data & 0x0f) << 8);			// y|x high bits
				layers[0].scroll_y = (layers[0].scroll_y & 0x0ff) | ((data & 0xf0) << 4);	break;

		// Layer 1
		case 3:	layers[1].scroll_x = (layers[1].scroll_x & 0xf00) | data;					break;	// x low
		case 4:	layers[1].scroll_y = (layers[1].scroll_y & 0xf00) | data;					break;	// y low
		case 5:	layers[1].scroll_x = (layers[1].scroll_x & 0x0ff) | ((data & 0x0f) << 8);			// y|x high bits
				layers[1].scroll_y = (layers[1].scroll_y & 0x0ff) | ((data & 0xf0) << 4);	break;
	}
}

// Layer 0
static WRITE8_HANDLER( bishjan_scrollram_0_hi_w )		{	layers[0].scrollrams[VRAM_HI][offset] = data;				}
static WRITE8_HANDLER( bishjan_scrollram_0_lo_w )		{	layers[0].scrollrams[VRAM_LO][offset] = data;				}
static WRITE8_HANDLER( bishjan_scrollram_0_hi_lo_w )	{	layers[0].scrollrams[VRAM_HI][offset] = data;
															layers[0].scrollrams[VRAM_LO][offset] = bishjan_byte_lo;	}
static READ8_HANDLER ( bishjan_scrollram_0_hi_r )		{	return layers[0].scrollrams[VRAM_HI][offset];				}
static READ8_HANDLER ( bishjan_scrollram_0_lo_r )		{	return layers[0].scrollrams[VRAM_LO][offset];				}

// Layer 1
static WRITE8_HANDLER( bishjan_scrollram_1_hi_w )		{	layers[1].scrollrams[VRAM_HI][offset] = data;				}
static WRITE8_HANDLER( bishjan_scrollram_1_lo_w )		{	layers[1].scrollrams[VRAM_LO][offset] = data;				}
static WRITE8_HANDLER( bishjan_scrollram_1_hi_lo_w )	{	layers[1].scrollrams[VRAM_HI][offset] = data;
															layers[1].scrollrams[VRAM_LO][offset] = bishjan_byte_lo;	}
static READ8_HANDLER ( bishjan_scrollram_1_hi_r )		{	return layers[1].scrollrams[VRAM_HI][offset];				}
static READ8_HANDLER ( bishjan_scrollram_1_lo_r )		{	return layers[1].scrollrams[VRAM_LO][offset];				}


/***************************************************************************
                              Tilemaps Disable
***************************************************************************/

static int bishjan_disable;

static WRITE8_HANDLER( bishjan_disable_w )
{
	bishjan_disable = data;
}


/***************************************************************************
                                Video Update
***************************************************************************/

static VIDEO_START(bishjan)
{
	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &layers[i];

		l->tmap = tilemap_create(machine, i ? get_tile_info_1 : get_tile_info_0, tilemap_scan_rows, 8,8, 0x80,0x40);

		tilemap_set_transparent_pen(l->tmap, 0);

		// line scroll
		tilemap_set_scroll_rows(l->tmap, 0x200);
		tilemap_set_scrolldy(l->tmap, -1, +1);

		l->videorams[VRAM_HI] = auto_alloc_array(machine, UINT8, 0x80 * 0x40);
		l->videorams[VRAM_LO] = auto_alloc_array(machine, UINT8, 0x80 * 0x40);

		l->scrollrams[VRAM_HI] = auto_alloc_array(machine, UINT8, 0x200);
		l->scrollrams[VRAM_LO] = auto_alloc_array(machine, UINT8, 0x200);
	}

	bishjan_colorram = auto_alloc_array(machine, UINT8, 256*3);
}

static VIDEO_UPDATE( bishjan )
{
	int layers_ctrl = ~bishjan_disable;
	int y;

#ifdef MAME_DEBUG
	if (input_code_pressed(screen->machine, KEYCODE_Z))
	{
		int msk = 0;
		if (input_code_pressed(screen->machine, KEYCODE_Q))	msk |= 1;
		if (input_code_pressed(screen->machine, KEYCODE_W))	msk |= 2;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	// Scroll
	for (int i = 0; i < 2; i++)
	{
		layer_t *l = &layers[i];

		tilemap_set_scrolly(l->tmap, 0, l->scroll_y);

		// line scroll (only one scroll value per tile is read from scrollram)

		int mask_y;
		switch (l->tilesize)
		{
			default:
			case TILE_8x8:		mask_y = ~(8-1);	break;
			case TILE_8x32:		mask_y = ~(32-1);	break;
			case TILE_64x32:	mask_y = ~(32-1);	break;
		}

		for (y = 0; y < 0x200; y++)
		{
			UINT16 scroll_dx = (l->scrollrams[VRAM_HI][y & mask_y] << 8) + l->scrollrams[VRAM_LO][y & mask_y];

			// hack? xplan: scrollram is not cleared after being used in the previous screen
			if ( bishjan_tile_unk & (0x40 >> (i * 2)) )
				scroll_dx = 0;

			tilemap_set_scrollx(l->tmap, y, l->scroll_x + scroll_dx);
		}
	}

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, layers[0].tmap, 0, 0);
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, layers[1].tmap, 0, 0);

//  popmessage("scrl: %03x,%03x - %03x,%03x dis: %02x siz: %02x unk: %02x", layers[0].scroll_x,layers[0].scroll_y, layers[1].scroll_x,layers[1].scroll_y, bishjan_disable, bishjan_tilesize, bishjan_tile_unk);

	return 0;
}


/***************************************************************************
                Palette: HMC HM86171 VGA 256 colour RAMDAC
***************************************************************************/

static int colordac_offs;

static WRITE8_HANDLER(colordac_w)
{
	switch ( offset )
	{
		case 0:
			colordac_offs = data * 3;
			break;

		case 1:
			bishjan_colorram[colordac_offs] = data;
			palette_set_color_rgb(space->machine, colordac_offs/3,
				pal6bit(bishjan_colorram[(colordac_offs/3)*3+0]),
				pal6bit(bishjan_colorram[(colordac_offs/3)*3+1]),
				pal6bit(bishjan_colorram[(colordac_offs/3)*3+2])
			);
			colordac_offs = (colordac_offs+1) % (256*3);
			break;

		case 2:
			// ff?
			break;

		case 3:
			break;
	}
}


/***************************************************************************
                                Memory Maps
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

static UINT16 bishjan_sel, bishjan_input, bishjan_hopper;

static WRITE16_HANDLER( bishjan_sel_w )
{
	if (ACCESSING_BITS_8_15)	bishjan_sel = data >> 8;
}

static READ16_HANDLER( bishjan_serial_r )
{
	return
		(space->machine->rand() & 0x9800)	|	// bit 7 - serial communication
		(((bishjan_sel==0x12) ? 0x40:0x00) << 8) |
//      (machine->rand() & 0xff);
//      (((space->machine->primary_screen->frame_number()%60)==0)?0x18:0x00);
		0x18;
}

static WRITE16_HANDLER( bishjan_input_w )
{
	if (ACCESSING_BITS_8_15)	bishjan_input = data >> 8;
}

static READ16_HANDLER( bishjan_input_r )
{
	int i;
	UINT16 res = 0xff;
	static const char *const port[] = { "KEYB_0", "KEYB_1", "KEYB_2", "KEYB_3", "KEYB_4" };

	for (i = 0; i < 5; i++)
		if (bishjan_input & (1 << i))
			res = input_port_read(space->machine, port[i]);

	return	(res << 8) |									// high byte
			input_port_read(space->machine, "SYSTEM") |		// low byte
			((bishjan_hopper && !(space->machine->primary_screen->frame_number()%10)) ? 0x00 : 0x04)	// bit 2: hopper sensor
	;
}

static WRITE16_HANDLER( bishjan_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		// coin out         data & 0x01;
		bishjan_hopper	=	data & 0x02;	// hopper
		coin_counter_w(space->machine, 1,	data & 0x10 );
	}
}

static ADDRESS_MAP_START( bishjan_map, ADDRESS_SPACE_PROGRAM, 16 )
	ADDRESS_MAP_GLOBAL_MASK(0xffffff)

	AM_RANGE( 0x000000, 0x07ffff ) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE( 0x080000, 0x0fffff ) AM_ROM AM_REGION("maincpu", 0)

	AM_RANGE( 0x200000, 0x207fff ) AM_RAM AM_SHARE("nvram")	// battery


	// read lo (1)   (only half tilemap?)
	AM_RANGE( 0x412000, 0x412fff ) AM_READ8( bishjan_videoram_1_lo_r, 0xffff )
	AM_RANGE( 0x413000, 0x4131ff ) AM_READWRITE8( bishjan_scrollram_1_lo_r, bishjan_scrollram_1_lo_w, 0xffff )
	// read lo (0)
	AM_RANGE( 0x416000, 0x416fff ) AM_READ8( bishjan_videoram_0_lo_r, 0xffff )
	AM_RANGE( 0x417000, 0x4171ff ) AM_READWRITE8( bishjan_scrollram_0_lo_r, bishjan_scrollram_0_lo_w, 0xffff )

	// read hi (1)
	AM_RANGE( 0x422000, 0x422fff ) AM_READ8( bishjan_videoram_1_hi_r, 0xffff )
	AM_RANGE( 0x423000, 0x4231ff ) AM_READWRITE8( bishjan_scrollram_1_hi_r, bishjan_scrollram_1_hi_w, 0xffff )
	// read hi (0)
	AM_RANGE( 0x426000, 0x426fff ) AM_READ8( bishjan_videoram_0_hi_r, 0xffff )
	AM_RANGE( 0x427000, 0x4271ff ) AM_READWRITE8( bishjan_scrollram_0_hi_r, bishjan_scrollram_0_hi_w, 0xffff )

	// write both (1)
	AM_RANGE( 0x430000, 0x431fff ) AM_WRITE8( bishjan_videoram_1_hi_lo_w, 0xffff )
	AM_RANGE( 0x432000, 0x432fff ) AM_WRITE8( bishjan_videoram_1_hi_lo_w, 0xffff )
	AM_RANGE( 0x433000, 0x4331ff ) AM_WRITE8( bishjan_scrollram_1_hi_lo_w, 0xffff )
	// write both (0)
	AM_RANGE( 0x434000, 0x435fff ) AM_WRITE8( bishjan_videoram_0_hi_lo_w, 0xffff )
	AM_RANGE( 0x436000, 0x436fff ) AM_WRITE8( bishjan_videoram_0_hi_lo_w, 0xffff )
	AM_RANGE( 0x437000, 0x4371ff ) AM_WRITE8( bishjan_scrollram_0_hi_lo_w, 0xffff )


	AM_RANGE( 0x600000, 0x600001 ) AM_READNOP AM_WRITE( bishjan_sel_w )
	AM_RANGE( 0x600040, 0x600041 ) AM_WRITE8( bishjan_tile_unk_w, 0xff00 )	// on screen transitions
	AM_RANGE( 0x600060, 0x600063 ) AM_WRITE8( colordac_w, 0xffff )
	AM_RANGE( 0x600080, 0x600081 ) AM_WRITE8( bishjan_tilesize_w, 0xff00 )	// on screen transitions
	AM_RANGE( 0x6000a0, 0x6000a1 ) AM_WRITE8( bishjan_byte_lo_w, 0xff00 )

	AM_RANGE( 0xa0001e, 0xa0001f ) AM_WRITE8( bishjan_disable_w, 0x00ff )

	AM_RANGE( 0xa00020, 0xa00025 ) AM_WRITE8( bishjan_scroll_w, 0xffff )

	AM_RANGE( 0xc00000, 0xc00001 ) AM_READ_PORT("DSW")								// SW1
	AM_RANGE( 0xc00002, 0xc00003 ) AM_READ_PORT("JOY") AM_WRITE( bishjan_input_w )	// IN C
	AM_RANGE( 0xc00004, 0xc00005 ) AM_READ( bishjan_input_r )						// IN A & B
	AM_RANGE( 0xc00006, 0xc00007 ) AM_READ( bishjan_serial_r )						// IN D
	AM_RANGE( 0xc00008, 0xc00009 ) AM_READ_PORT("RESET") AM_WRITE( bishjan_coin_w )
ADDRESS_MAP_END

/***************************************************************************
                                Magic Train
***************************************************************************/

static ADDRESS_MAP_START( mtrain_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0ffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mtrain_io, ADDRESS_SPACE_IO, 8 )
ADDRESS_MAP_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

static UINT8 saklove_dsw_mask;

static WRITE8_HANDLER( saklove_dsw_mask_w )
{
	saklove_dsw_mask = data;
}

static READ8_HANDLER( saklove_dsw_r )
{
	return	( (input_port_read(space->machine, "DSW1") & saklove_dsw_mask) ? 0x01 : 0 ) |
			( (input_port_read(space->machine, "DSW2") & saklove_dsw_mask) ? 0x02 : 0 ) |
			( (input_port_read(space->machine, "DSW3") & saklove_dsw_mask) ? 0x04 : 0 ) |
			( (input_port_read(space->machine, "DSW4") & saklove_dsw_mask) ? 0x08 : 0 ) ;
}

static UINT8 *saklove_outputs;
static WRITE8_HANDLER( saklove_outputs_w )
{
	saklove_outputs[offset] = data;

	switch (offset)
	{
		case 0:
			// bit 0 set on coin in
			// bit 1 set on key in
			break;

		case 1:
			break;

		case 2:
			break;

		case 3:
			// 1, 2, 4
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", saklove_outputs[0], saklove_outputs[1], saklove_outputs[2], saklove_outputs[3]);
}

static WRITE8_DEVICE_HANDLER( saklove_oki_bank_w )
{
	// it writes 0x32 or 0x33
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base((data & 1) * 0x40000);
}

static READ8_HANDLER( saklove_vblank_r )
{
	return space->machine->primary_screen->vblank() ? 0x04 : 0x00;
}

static UINT8 *am188em_regs;

enum
{
	AM188EM_IMASK = 0x28,
	AM188EM_I0CON = 0x38
};

static READ8_HANDLER( am188em_regs_r )
{
	return am188em_regs[offset];
}

static WRITE8_HANDLER( am188em_regs_w )
{
	am188em_regs[offset] = data;
}

static MACHINE_RESET( am188em )
{
	// start with masked interrupts
	am188em_regs[AM188EM_IMASK+0] = 0xfd;
	am188em_regs[AM188EM_IMASK+1] = 0x07;
	am188em_regs[AM188EM_I0CON+0] = 0x0f;
	am188em_regs[AM188EM_I0CON+1] = 0x00;
}

static ADDRESS_MAP_START( saklove_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("nvram")	// battery

	// read lo (1)   (only half tilemap?)
	AM_RANGE(0x12000, 0x12fff) AM_READWRITE( bishjan_videoram_1_lo_r, bishjan_videoram_1_lo_w )
	AM_RANGE(0x13000, 0x131ff) AM_READWRITE( bishjan_scrollram_1_lo_r, bishjan_scrollram_1_lo_w )
	// read lo (0)
	AM_RANGE(0x16000, 0x16fff) AM_READWRITE( bishjan_videoram_0_lo_r, bishjan_videoram_0_lo_w )
	AM_RANGE(0x17000, 0x171ff) AM_READWRITE( bishjan_scrollram_0_lo_r, bishjan_scrollram_0_lo_w )

	// read hi (1)
	AM_RANGE(0x22000, 0x22fff) AM_READWRITE( bishjan_videoram_1_hi_r, bishjan_videoram_1_hi_w )
	AM_RANGE(0x23000, 0x231ff) AM_READWRITE( bishjan_scrollram_1_hi_r, bishjan_scrollram_1_hi_w )
	// read hi (0)
	AM_RANGE(0x26000, 0x26fff) AM_READWRITE( bishjan_videoram_0_hi_r, bishjan_videoram_0_hi_w )
	AM_RANGE(0x27000, 0x271ff) AM_READWRITE( bishjan_scrollram_0_hi_r, bishjan_scrollram_0_hi_w )

	// write both (1)
	AM_RANGE(0x30000, 0x31fff) AM_READWRITE( bishjan_videoram_1_hi_r, bishjan_videoram_1_hi_lo_w )
	// write both (0)
	AM_RANGE(0x34000, 0x35fff) AM_READWRITE( bishjan_videoram_0_hi_r, bishjan_videoram_0_hi_lo_w )

	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( saklove_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x0000) AM_WRITE( bishjan_tile_unk_w )	// on screen transitions

	AM_RANGE(0x0020, 0x0020) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x0040, 0x0041) AM_DEVWRITE( "ymsnd", ym3812_w )

	AM_RANGE(0x0060, 0x0063) AM_WRITE( colordac_w )

	AM_RANGE(0x0080, 0x0080) AM_WRITE( bishjan_tilesize_w )	// on screen transitions
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE( bishjan_byte_lo_w )
	AM_RANGE(0x021f, 0x021f) AM_WRITE( bishjan_disable_w )
	AM_RANGE(0x0220, 0x0225) AM_WRITE( bishjan_scroll_w )

	AM_RANGE(0x0300, 0x0303) AM_WRITE( saklove_outputs_w ) AM_BASE( &saklove_outputs )
	AM_RANGE(0x0303, 0x0303) AM_READ_PORT( "IN D" )	// 0x40 serial out, 0x80 serial in
	AM_RANGE(0x0304, 0x0304) AM_READ_PORT( "IN A" )
	AM_RANGE(0x0305, 0x0305) AM_READ_PORT( "IN B" )
	AM_RANGE(0x0306, 0x0306) AM_READ_PORT( "IN C" )

	AM_RANGE(0x0307, 0x0307) AM_READ ( saklove_dsw_r )
	AM_RANGE(0x0308, 0x0308) AM_WRITE( saklove_dsw_mask_w )

	AM_RANGE(0x0312, 0x0312) AM_READ( saklove_vblank_r ) AM_DEVWRITE( "oki", saklove_oki_bank_w )

	// Peripheral Control Block
	AM_RANGE(0xff00, 0xffff) AM_READWRITE( am188em_regs_r, am188em_regs_w ) AM_BASE( &am188em_regs )
ADDRESS_MAP_END

/***************************************************************************
                                X-Plan
***************************************************************************/

static READ8_HANDLER( xplan_vblank_r )
{
	return space->machine->primary_screen->vblank() ? 0x40 : 0x00;
}

static WRITE8_DEVICE_HANDLER( xplan_oki_bank_w )
{
	// it writes 0x23 or 0x33
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->set_bank_base(((data >> 4) & 1) * 0x40000);
}

static UINT8 *xplan_outputs;
static WRITE8_HANDLER( xplan_outputs_w )
{
	xplan_outputs[offset] = data;

	switch (offset)
	{
		case 0:
			// 40 = seial out ? (at boot, to read test mode)
			break;

		case 1:
			set_led_status(space->machine, 1,	data & 0x02);	// raise
			break;

		case 2:	// B
			set_led_status(space->machine, 2,	data & 0x04);	// hold 1 / big ?
			set_led_status(space->machine, 3,	data & 0x08);	// hold 5 / bet
			set_led_status(space->machine, 4,	data & 0x10);	// hold 4 ?
			set_led_status(space->machine, 5,	data & 0x20);	// hold 2 / double up
			set_led_status(space->machine, 6,	data & 0x40);	// hold 3 / small ?
			break;

		case 3:	// A
			coin_counter_w(space->machine, 0,	data & 0x01 );
			coin_counter_w(space->machine, 1,	data & 0x02 );

			set_led_status(space->machine, 7,	data & 0x10);	// start / take
			break;
	}

//  popmessage("0: %02x - 1: %02x - 2: %02x - 3: %02x", xplan_outputs[0], xplan_outputs[1], xplan_outputs[2], xplan_outputs[3]);
}

static ADDRESS_MAP_START( xplan_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("nvram")	// battery

	// write both (1?)
	AM_RANGE( 0x10000, 0x11fff ) AM_WRITE( bishjan_videoram_1_hi_lo_w )
	// read lo (1)   (only half tilemap?)
	AM_RANGE( 0x12000, 0x12fff ) AM_READ( bishjan_videoram_1_lo_r )
	AM_RANGE( 0x13000, 0x131ff ) AM_READWRITE( bishjan_scrollram_1_lo_r, bishjan_scrollram_1_lo_w )

	// write both (0? backgrounds)
	AM_RANGE( 0x14000, 0x15fff ) AM_WRITE( bishjan_videoram_0_hi_lo_w )
	// read lo (0)
	AM_RANGE( 0x16000, 0x16fff ) AM_READ( bishjan_videoram_0_lo_r )
	AM_RANGE( 0x17000, 0x171ff ) AM_READWRITE( bishjan_scrollram_0_lo_r, bishjan_scrollram_0_lo_w )

	// read hi (1)
	AM_RANGE( 0x22000, 0x22fff ) AM_READ( bishjan_videoram_1_hi_r )
	AM_RANGE( 0x23000, 0x231ff ) AM_READWRITE( bishjan_scrollram_1_hi_r, bishjan_scrollram_1_hi_w )
	// read hi (0)
	AM_RANGE( 0x26000, 0x26fff ) AM_READ( bishjan_videoram_0_hi_r )
	AM_RANGE( 0x27000, 0x271ff ) AM_READWRITE( bishjan_scrollram_0_hi_r, bishjan_scrollram_0_hi_w )

	// write both (1)
	AM_RANGE( 0x30000, 0x31fff ) AM_WRITE( bishjan_videoram_1_hi_lo_w )
	AM_RANGE( 0x32000, 0x32fff ) AM_WRITE( bishjan_videoram_1_hi_lo_w )
	AM_RANGE( 0x33000, 0x331ff ) AM_WRITE( bishjan_scrollram_1_hi_lo_w )
	// write both (0)
	AM_RANGE( 0x34000, 0x35fff ) AM_WRITE( bishjan_videoram_0_hi_lo_w )
	AM_RANGE( 0x36000, 0x36fff ) AM_WRITE( bishjan_videoram_0_hi_lo_w )
	AM_RANGE( 0x37000, 0x371ff ) AM_WRITE( bishjan_scrollram_0_hi_lo_w )

	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( xplan_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0x0000) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)

	AM_RANGE(0x0020, 0x0020) AM_WRITE( bishjan_byte_lo_w )	// mirror

	AM_RANGE(0x0040, 0x0040) AM_WRITE( bishjan_tile_unk_w )	// on screen transitions

	AM_RANGE(0x0060, 0x0063) AM_WRITE( colordac_w )
	AM_RANGE(0x0080, 0x0080) AM_WRITE( bishjan_tilesize_w )	// on screen transitions
	AM_RANGE(0x00a0, 0x00a0) AM_WRITE( bishjan_byte_lo_w )

	AM_RANGE(0x021f, 0x021f) AM_WRITE( bishjan_disable_w )
	AM_RANGE(0x0220, 0x0225) AM_WRITE( bishjan_scroll_w )

	AM_RANGE(0x0235, 0x0235) AM_NOP	// INT0 Ack.?

	AM_RANGE(0x0300, 0x0300) AM_READ( xplan_vblank_r ) AM_DEVWRITE( "oki", xplan_oki_bank_w )
	AM_RANGE(0x0301, 0x0301) AM_WRITE( saklove_dsw_mask_w )
	AM_RANGE(0x0302, 0x0302) AM_READ ( saklove_dsw_r )
	AM_RANGE(0x0303, 0x0303) AM_READ_PORT( "IN C" )
	AM_RANGE(0x0304, 0x0304) AM_READ_PORT( "IN B" )
	AM_RANGE(0x0305, 0x0305) AM_READ_PORT( "IN A" )
	AM_RANGE(0x0306, 0x0306) AM_READ_PORT( "IN D" )	// 0x40 serial out, 0x80 serial in

	// 306 = d, 307 = c, 308 = b, 309 = a
	AM_RANGE(0x0306, 0x0309) AM_WRITE( xplan_outputs_w ) AM_BASE( &xplan_outputs )

	// Peripheral Control Block
	AM_RANGE(0xff00, 0xffff) AM_READWRITE( am188em_regs_r, am188em_regs_w ) AM_BASE( &am188em_regs )
ADDRESS_MAP_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout bishjan_8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8*8*8
};

static GFXDECODE_START( bishjan )
	GFXDECODE_ENTRY( "tilemap", 0, bishjan_8x8_layout, 0, 1 )
GFXDECODE_END


/***************************************************************************
                                Input Ports
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

static INPUT_PORTS_START( bishjan )
	PORT_START("RESET")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)

	PORT_START("DSW")	// SW1
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Controls ) )
	PORT_DIPSETTING(      0x0001, "Keyboard" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("JOY")	// IN C
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1			) PORT_NAME("1 Player Start (Joy Mode)")	// start (joy)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN	)	// down (joy)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT	)	// left (joy)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT	)	// right (joy)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1		)	// n (joy)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_MAHJONG_BET	) PORT_NAME("P1 Mahjong Bet (Joy Mode)")	// bet (joy)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON2		)	// select (joy)

	PORT_START("SYSTEM") // IN A
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SERVICE		) PORT_IMPULSE(1)	// service mode
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH,IPT_SPECIAL		)	// hopper sensor
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1		)	// stats
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2		)	// pay out? "hopper empty"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN1			)	PORT_IMPULSE(2)	// coin
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE3		)	// pay out? "hopper empty"
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN2			)	PORT_IMPULSE(2)	// coin

	PORT_START("KEYB_0")	// IN B(0)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_A		)	// a
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_E		)	// e
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_I		)	// i
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_M		)	// m
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// i2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1			)	// b2 (start)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_1")	// IN B(1)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_B		)	// b
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_F		)	// f
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_J		)	// j
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_MAHJONG_N		)	// n
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// l2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MAHJONG_BET	)	// c2 (bet)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_2")	// IN B(2)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_C		)	// c
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_G		)	// g
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_K		)	// k
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// k2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// m2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_3")	// IN B(3)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_MAHJONG_D		)	// d
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_MAHJONG_H		)	// h
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_MAHJONG_L		)	// l
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// j2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)

	PORT_START("KEYB_4")	// IN B(4)
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// g2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// e2
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// d2
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN		)	// f2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN		)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN		)
INPUT_PORTS_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

static INPUT_PORTS_START( saklove )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x00, "Coin" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "100" )
	PORT_DIPSETTING(    0x07, "300" )
	PORT_DIPNAME( 0x38, 0x00, "Key In" )
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x10, "20" )
	PORT_DIPSETTING(    0x18, "25" )
	PORT_DIPSETTING(    0x20, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x28, "300" )
	PORT_DIPSETTING(    0x30, "500" )
	PORT_DIPSETTING(    0x38, "1000" )
	PORT_DIPNAME( 0x40, 0x00, "Pay Out" )
	PORT_DIPSETTING(    0x00, "Coin" )
	PORT_DIPSETTING(    0x40, "Key In" )
	PORT_DIPNAME( 0x80, 0x00, "Key Out" )
	PORT_DIPSETTING(    0x80, "Coin" )
	PORT_DIPSETTING(    0x00, "Key In" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, "Min Bet" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPNAME( 0x0c, 0x00, "Max Bet" )
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "40" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPUNKNOWN( 0x10, 0x00 )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Double Up" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x07, 0x00, "Win Rate (%)" )
	PORT_DIPSETTING(    0x01, "55" )
	PORT_DIPSETTING(    0x02, "60" )
	PORT_DIPSETTING(    0x03, "65" )
	PORT_DIPSETTING(    0x04, "70" )
	PORT_DIPSETTING(    0x05, "75" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPSETTING(    0x06, "85" )
	PORT_DIPSETTING(    0x07, "90" )
	PORT_DIPNAME( 0x18, 0x00, "Game Limit" )
	PORT_DIPSETTING(    0x08, "10k" )
	PORT_DIPSETTING(    0x00, "20k" )
	PORT_DIPSETTING(    0x10, "60k" )
	PORT_DIPSETTING(    0x18, "80k" )
	PORT_DIPUNKNOWN( 0x20, 0x00 )
	PORT_DIPUNKNOWN( 0x40, 0x00 )
	PORT_DIPUNKNOWN( 0x80, 0x00 )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x00, "Double Up Level" )
	PORT_DIPSETTING(    0x03, "0" )
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x0c, 0x00, "Double Up Limit" )
	PORT_DIPSETTING(    0x00, "5k" )
	PORT_DIPSETTING(    0x04, "10k" )
	PORT_DIPSETTING(    0x08, "20k" )
	PORT_DIPSETTING(    0x0c, "30k" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPNAME( 0x20, 0x00, "Coin Type" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPNAME( 0x80, 0x00, "Jamma" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  ) PORT_NAME("Bet 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  ) PORT_NAME("Bet 2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3  ) PORT_NAME("Bet 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1   ) PORT_NAME("Play")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON5  ) PORT_NAME("Big or Small 1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4  ) PORT_NAME("Bet Amount")	// 1-5-10

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2   )			// selects music in system test / exit
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )	// top 10? / double up?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6  ) PORT_NAME("Big or Small 2")	// plays sample or advances music in system test / big or small?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Statistics")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2    )	// key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE  ) PORT_IMPULSE(2)	// service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )	// used?

	PORT_START("IN D")	// bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  ) // used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER    ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )
INPUT_PORTS_END

/***************************************************************************
                                X-Plan
***************************************************************************/

static INPUT_PORTS_START( xplan )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Alt. Pinout" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )		// not implemented
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	// not populated

	PORT_START("DSW3")
	// not populated

	PORT_START("DSW4")
	// not populated

	PORT_START("IN A")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1       ) PORT_NAME("Play Gambling 1")				// \__ play gambling game
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2       ) PORT_NAME("Play Gambling 2")				// /
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3       ) PORT_NAME("Play Shoot'Em Up")			// ___ play shoot'em up game
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1        ) PORT_NAME("Start / Take")				// start / take
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3   ) PORT_NAME("Hold 3 / Small")				// hold 3 / small / decrease sample in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD5   ) PORT_NAME("Hold 5 / Bet")				// hold 5 / bet

	PORT_START("IN B")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD4   )											// hold 4
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2   ) PORT_NAME("Hold 2 / Double Up / Right")	// hold 2 / double up / right
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER         ) PORT_NAME("Raise") PORT_CODE(KEYCODE_N)	// raise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD1   ) PORT_NAME("Hold 1 / Big / Left")			// hold 1 / big / increase sample in test mode / left
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1         )											// coin in
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN       )

	PORT_START("IN C")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK   )						// stats
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2         )						// key in
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE       )  PORT_IMPULSE(1)		// service mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )						// pay-out

	PORT_START("IN D")	// bits 3 and 4 shown in test mode
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN       )						// used?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1      ) PORT_NAME("Reset")	// reset
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN       )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL       )						// serial in?
INPUT_PORTS_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

// To be removed when cpu core is updated
static TIMER_DEVICE_CALLBACK( h8_timer_irq )
{
	cputag_set_input_line(timer.machine, "maincpu", H8_METRO_TIMER_HACK, HOLD_LINE);
}

static MACHINE_CONFIG_START( bishjan, driver_device )
	MCFG_CPU_ADD("maincpu", H83044, XTAL_44_1MHz / 3)
	MCFG_CPU_PROGRAM_MAP( bishjan_map )
	MCFG_CPU_VBLANK_INT( "screen", irq0_line_hold )
	MCFG_TIMER_ADD_PERIODIC("timer", h8_timer_irq, HZ(60)) // timer, ?? Hz

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 60 )

	MCFG_GFXDECODE( bishjan )
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_VIDEO_START( bishjan )
	MCFG_VIDEO_UPDATE( bishjan )

	/* sound hardware */
	// SS9904?
MACHINE_CONFIG_END

/***************************************************************************
                                Magic Train
***************************************************************************/

static MACHINE_CONFIG_START( mtrain, driver_device )
	MCFG_CPU_ADD("maincpu", Z180, XTAL_12MHz / 8)	/* Unknown clock */
	MCFG_CPU_PROGRAM_MAP( mtrain_map )
	MCFG_CPU_IO_MAP( mtrain_io )

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )

	MCFG_GFXDECODE( bishjan )
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_VIDEO_START( bishjan )
	MCFG_VIDEO_UPDATE( bishjan )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)	// probably
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

// To be moved to cpu core when it's available
static TIMER_DEVICE_CALLBACK( am188em_timer2_irq )
{
	if ((am188em_regs[AM188EM_IMASK+0] & 0x01) == 0)	// TMR mask
		cputag_set_input_line_and_vector(timer.machine, "maincpu", 0, HOLD_LINE, 0x4c/4);
}

static MACHINE_CONFIG_START( saklove, driver_device )
	MCFG_CPU_ADD("maincpu", I80188, XTAL_20MHz )	// !! AMD AM188-EM !!
	MCFG_CPU_PROGRAM_MAP( saklove_map )
	MCFG_CPU_IO_MAP( saklove_io )
	MCFG_TIMER_ADD_PERIODIC("timer2", am188em_timer2_irq, HZ(60)) // timer 2, ?? Hz

	MCFG_MACHINE_RESET(am188em)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// game reads vblank state

	MCFG_GFXDECODE( bishjan )
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_VIDEO_START( bishjan )
	MCFG_VIDEO_UPDATE( bishjan )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_12MHz / 4)	// ? chip and clock unknown
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END

/***************************************************************************
                                X-Plan
***************************************************************************/

static INTERRUPT_GEN( am188em_int0_irq )
{
	if ( ((am188em_regs[AM188EM_IMASK+0] & 0x10) == 0) ||	// IMASK.I0 mask
		 ((am188em_regs[AM188EM_I0CON+0] & 0x08) == 0) )	// I0CON.MSK mask
		cpu_set_input_line_and_vector(device, 0, HOLD_LINE, 0x0c);	// INT0 (background scrolling in xplan)
}

static MACHINE_CONFIG_START( xplan, driver_device )
	MCFG_CPU_ADD("maincpu", I80188, XTAL_20MHz )	// !! AMD AM188-EM !!
	MCFG_CPU_PROGRAM_MAP( xplan_map )
	MCFG_CPU_IO_MAP( xplan_io )
	MCFG_CPU_VBLANK_INT( "screen", am188em_int0_irq )
	MCFG_TIMER_ADD_PERIODIC("timer2", am188em_timer2_irq, HZ(60)) // timer 2, ?? Hz

	MCFG_MACHINE_RESET(am188em)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 512, 256 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MCFG_SCREEN_REFRESH_RATE( 58.7270 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)	// game reads vblank state

	MCFG_GFXDECODE( bishjan )
	MCFG_PALETTE_LENGTH( 256 )

	MCFG_VIDEO_START( bishjan )
	MCFG_VIDEO_UPDATE( bishjan )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)	// probably
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Bishou Jan (Laugh World)
(C)1999 Subsino

PCB Layout
----------

|------------------------------------------------------|
|TDA1519A           28-WAY                             |
|     VOL                                              |
|                HM86171                       ULN2003 |
|   LM324                                              |
|           S-1                                ULN2003 |
|                                                      |
|                                   |-------|  DSW1(8) |
|                       |-------|   |SUBSINO|          |
|            2-V201.U9  |SUBSINO|   |SS9802 |          |
|                       |SS9904 |   |       |          |
|                       |       |   |-------|          |
|                       |-------|                      |
|                                                      |
|                         44.1MHz             CXK58257 |
|  3-V201.U25                                          |
|                                  1-V203.U21          |
|  4-V201.U26                                       SW1|
|             |-------|    |-------|   |-----|         |
|  5-V201.U27 |SUBSINO|    |SUBSINO|   |H8   |         |
|             |SS9601 |    |SS9803 |   |3044 |         |
|  6-V201.U28 |       |    |       |   |-----|         |
|             |-------|    |-------|                   |
|          62256  62256   BATTERY                      |
|------------------------------------------------------|
Notes:
      H8/3044 - Subsino re-badged Hitachi H8/3044 HD6433044A22F Microcontroller (QFP100)
                The H8/3044 is a H8/3002 with 24bit address bus and has 32k MASKROM and 2k RAM, clock input is 14.7MHz [44.1/3]
                MD0,MD1 & MD2 are configured to MODE 6 16MByte Expanded Mode with the on-chip 32k MASKROM enabled.
     CXK58257 - Sony CXK58257 32k x8 SRAM (SOP28)
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
          S-1 - ?? Probably some kind of audio OP AMP or DAC? (DIP8)
          SW1 - Push Button Test Switch
        HSync - 15.75kHz
        VSync - 60Hz

***************************************************************************/

ROM_START( bishjan )
	ROM_REGION( 0x100000, "maincpu", 0 )	// H8/3044
	ROM_LOAD( "1-v203.u21", 0x000000, 0x080000, CRC(1f891d48) SHA1(0b6a5aa8b781ba8fc133289790419aa8ea21c400) )

	ROM_REGION( 0x400000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "3-v201.u25", 0x000000, 0x100000, CRC(e013e647) SHA1(a5b0f82f3454393c1ea5e635b0d37735a25e2ea5) )
	ROM_LOAD32_BYTE( "4-v201.u26", 0x000001, 0x100000, CRC(e0d40ef1) SHA1(95f80889103a7b93080b46387274cb1ffe0c8768) )
	ROM_LOAD32_BYTE( "5-v201.u27", 0x000002, 0x100000, CRC(85067d40) SHA1(3ecf7851311a77a0dfca90775fcbf6faabe9c2ab) )
	ROM_LOAD32_BYTE( "6-v201.u28", 0x000003, 0x100000, CRC(430bd9d7) SHA1(dadf5a7eb90cf2dc20f97dbf20a4b6c8e7734fb1) )

	ROM_REGION( 0x100000, "samples", 0 )	// SS9904?
	ROM_LOAD( "2-v201.u9", 0x000000, 0x100000, CRC(ea42764d) SHA1(13fe1cd30e474f4b092949c440068e9ddca79976) )
ROM_END

static DRIVER_INIT(bishjan)
{
	UINT16 *rom = (UINT16*)machine->region("maincpu")->base();

	// check
	rom[0x042EA/2] = 0x4008;

	// rts -> rte
	rom[0x33386/2] = 0x5670;
	rom[0x0CC5C/2] = 0x5670;
}

/***************************************************************************

  Magic Train
  -----------

  Board silkscreened: "SUBSINO" (logo), "CS186P012". Stickered "1056439".


  CPU:   1x Hitachi HD647180X0CP6 - 6D1R (Subsino - SS9600) (U23).
  SND:   1x U6295 (OKI compatible) (U25).
         1x TDA1519A (PHILIPS, 22W BTL or 2x 11W stereo car radio power amplifier (U34).

  NVRAM:     1x SANYO LC36256AML (SMD) (U16).
  VRAM:      2x UMC UM62256E-70LL (U7-U8, next to gfx ROMs).
  Other RAM: 1x HMC HM86171-80 (U29, next to sound ROM).

  Video: Subsino (SMD-40PX40P) SS9601 - 9732WX011 (U1).
  I/O:   Subsino (SMD-30PX20P) SS9602 - 9732LX006 (U11).

  PRG ROM:  Stickered "M-TRAIN-N OUT_1 V1.31".

  GFX ROMs: 1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_1 V1.0" (U5).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_2 V1.0" (U4).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_3 V1.0" (U3).
            1x 27C2000DC-12  Stickered "M-TRAIN-N ROM_4 V1.0" (U2).

  SND ROM:  1x 27C2000DC-12 (U27, no sticker).

  PLDs: 1x GAL16V8D (U31, next to sound ROM).
        3x GAL16V8D (U18-U19-U6, next to CPU, program ROM and NVRAM).
        1x GAL16V8D (U26, near sound amp)

  Battery: 1x VARTA 3.6v, 60mAh.

  Xtal: 12 MHz.

  4x 8 DIP switches banks (SW1-SW2-SW3-SW4).
  1x Push button (S1, next to battery).

  1x 2x36 Edge connector.
  1x 2x10 Edge connector.

  U12, U13 & U14 are Darlington arrays.

***************************************************************************/

ROM_START( mtrain )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// code starts at 0x8100???
	ROM_LOAD( "out_1v131.u17", 0x00000, 0x10000, CRC(6761be7f) SHA1(a492f8179d461a454516dde33ff04473d4cfbb27) )

	ROM_REGION( 0x100000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "rom_4.u02", 0x00000, 0x40000, CRC(b7e65d04) SHA1(5eea1b8c1129963b3b83a59410cd0e1de70621e4) )
	ROM_LOAD32_BYTE( "rom_3.u03", 0x00001, 0x40000, CRC(cef2c079) SHA1(9ee54a08ef8db90a80a4b3568bb82ce09ee41e65) )
	ROM_LOAD32_BYTE( "rom_2.u04", 0x00002, 0x40000, CRC(a794f287) SHA1(7b9c0d57224a700f49e55ba5aeb7ed9d35a71e02) )
	ROM_LOAD32_BYTE( "rom_1.u05", 0x00003, 0x40000, CRC(96067e95) SHA1(bec7dffaf6920ff2bd85a43fb001a997583e25ee) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom_5.u27", 0x00000, 0x40000, CRC(51cae476) SHA1(d1da4e5c3d53d18d8b69dfb57796d0ae311d99bf) )
ROM_END

/***************************************************************************

  Incomplete decryption of mtrain (lifted from subsino.c)

  Notes:

  0000-8100? is not encrypted

  rom     addr
  8100 -> 0000 (code start)
  8200 -> 0100
  8d97 -> 0c97
  1346 -> b346

***************************************************************************/

static void crsbingo_bitswaps(UINT8* decrypt, int i)
{
	if ((i&7) == 0) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
	if ((i&7) == 1) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
	if ((i&7) == 2) decrypt[i] = BITSWAP8(decrypt[i],3,2,5,0,7,6,1,4);
	if ((i&7) == 3) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,0,3,6,1,4);
	if ((i&7) == 4) decrypt[i] = BITSWAP8(decrypt[i],7,6,5,0,3,2,1,4);
	if ((i&7) == 5) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,4,3,6,5,0);
	if ((i&7) == 6) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
	if ((i&7) == 7) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,0,7,6,5,4);
}

static const unsigned char crsbingo_xors[8] = { 0xbb, 0xcc, 0xcc, 0xdd, 0xaa, 0x11, 0x44, 0xee };

static void subsino_decrypt(running_machine* machine, void (*bitswaps)(UINT8* decrypt, int i), const UINT8* xors, int size)
{
	int i;
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* region = machine->region("maincpu")->base();

	for (i=0;i<0x10000;i++)
	{
		if ((i<size)/* && (i >= 0x8100)*/)
		{
			decrypt[i] = region[i]^xors[i&7];
			bitswaps(decrypt, i);
		}
		else
		{
			decrypt[i] = region[i];
		}
	}
//  dump_decrypted(machine, decrypt);
	memcpy(region, decrypt, 0x10000);
}

DRIVER_INIT( mtrain )
{
	// this one is odd
	// the code clearly starts at 0x8100 in the rom, not 0x8000
	subsino_decrypt(machine, crsbingo_bitswaps, crsbingo_xors, 0xc000);
}

/***************************************************************************

Sakura Love
Subsino, 1998

PCB Layout
----------

|------------------------------------|
|     ULN2003 ULN2003 ULN2003 ULN2003|
|LM358     HM86171     |-----|    PAL|
| M6295       8.4672MHz|SS9602    PAL|
| LM324                |-----|    PAL|
|J              |-------|         PAL|
|A              |SUBSINO|         PAL|
|M  2   12MHz   |SS9601 |         SW5|
|M  3   20MHz   |       |            |
|A  4           |-------|            |
|   5        1  LC36256              |
|   6      AM188-EM    62256  SW3 SW4|
|BATTERY               62256  SW1 SW2|
|------------------------------------|
Notes:
      AM188-EM - AMD AM188 Main CPU (QFP100) Clock 20.0MHz
      M6295    - clock 1.0584MHz [8.4672/8]. Pin 7 HIGH
      SW1-SW4  - Dip switches with 8 positions
      SW5      - Reset switch
      VSync    - 58.7270Hz
      HSync    - 15.3234kHz

***************************************************************************/

ROM_START( saklove )
	ROM_REGION( 0x20000, "maincpu", 0 )	// AM188-EM
	ROM_LOAD( "1.u23", 0x00000, 0x20000, CRC(02319bfb) SHA1(1a425dcdeecae92d8b7457d1897c700ac7856a9d) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "3.u27", 0x000000, 0x80000, CRC(01aa8fbd) SHA1(d1d19ef52c8077ccf17cc2fde96fd56c626e33db) )
	ROM_LOAD32_BYTE( "4.u28", 0x000001, 0x80000, CRC(f8db7ab6) SHA1(3af4e92ab27edc980eccecdbbbb431e1d2101059) )
	ROM_LOAD32_BYTE( "5.u29", 0x000002, 0x80000, CRC(c6ca1764) SHA1(92bfa19e116d358b03164f2448a28e7524e3cc62) )
	ROM_LOAD32_BYTE( "6.u30", 0x000003, 0x80000, CRC(5823c39e) SHA1(257e862ac736ff403ce9c70bbdeed340dfe168af) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "2.u10", 0x00000, 0x80000, CRC(4f70125c) SHA1(edd5e6bd47b9a4fa3c4057cb4a85544241fe483d) )
ROM_END

static DRIVER_INIT(saklove)
{
	UINT8 *rom = machine->region("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0x0e029] = 0xeb;
}

/***************************************************************************

X-Plan
(c) 2006 Subsino

PCB:
  SUBSINO SFWO2

CPU:
  AMD Am188 EM-20KC F 9912F6B (@U10)
  Osc. 20.00000 MHz (@OSC20)
  Sony CXK58257AM-10L (@U13) - 32k x 8 Bit High Speed CMOS Static RAM

Video:
  Subsino SS9601 0035WK007 (@U16)
  Subsino SS9802 0448 (@U1)
  Subsino SS9803 0020 (@U29)
  HM86171-80 (@U26) - RAMDAC
  2 x Winbond W24M257AK-15 (@U21-U22) - 32k x 8 Low Voltage CMOS Static RAM

Sound:
  U6295 0214 B923826 (@U6)
  Osc. 8.4672 MHz (@OSC12)
  Philips TDA1519A? (@U9) - 22 W BTL or 2 x 11 W stereo power amplifier

Other:
  Osc. 12.000 MHz
  Battery (button cell)
  Reset switch
  Volume trimmer
  4 x DSW8 (@SW1-SW4, only SW1 is populated)
  4 x ULN2003A (@U3-U5) - High Voltage, High Current Darlington Transistor Arrays
  10 pin edge connector
  36 pin edge connector
  Jamma connector

***************************************************************************/

ROM_START( xplan )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "x-plan_v101.u14", 0x00000, 0x40000, CRC(5a05fcb3) SHA1(9dffffd868e777f9436c38df76fa5247f4dd6daf) )

	ROM_REGION( 0x200000, "tilemap", 0 )
	ROM_LOAD32_BYTE( "x-plan_rom_3_v102b.u20", 0x00000, 0x80000, CRC(a027cbd1) SHA1(dac4226014794ef5bff84ddafee7da6691c00ece) )
	ROM_LOAD32_BYTE( "x-plan_rom_4_v102b.u19", 0x00001, 0x80000, CRC(744be318) SHA1(1c1f2a9e1da77d9bc1bf897072df44a681a53079) )
	ROM_LOAD32_BYTE( "x-plan_rom_5_v102b.u18", 0x00002, 0x80000, CRC(7e89c9b3) SHA1(9e3fea0d74cac48c068a15595f2342a2b0b3f747) )
	ROM_LOAD32_BYTE( "x-plan_rom_6_v102b.u17", 0x00003, 0x80000, CRC(a86ca3b9) SHA1(46aa86b9c62aa0a4e519eb06c72c2d540489afee) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "x-plan_rom_2_v100.u7", 0x00000, 0x80000, CRC(c742b5c8) SHA1(646960508be738824bfc578c1b21355c17e05010) )
ROM_END

static DRIVER_INIT(xplan)
{
	UINT8 *rom = machine->region("maincpu")->base();

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0xeded9-0xc0000] = 0xeb;
}


GAME( 1997, mtrain,  0, mtrain,  xplan,   mtrain,  ROT0, "Subsino", "Magic Train",                    GAME_NOT_WORKING )
GAME( 1998, saklove, 0, saklove, saklove, saklove, ROT0, "Subsino", "Ying Hua Lian 2.0 (China 1.02)", 0 )
GAME( 1999, bishjan, 0, bishjan, bishjan, bishjan, ROT0, "Subsino", "Bishou Jan (Japan 203)",         GAME_NO_SOUND )
GAME( 2006, xplan,   0, xplan,   xplan,   xplan,   ROT0, "Subsino", "X-Plan (Alpha 101)",             GAME_IMPERFECT_GRAPHICS )
