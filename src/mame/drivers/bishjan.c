/************************************************************************************************************

                                      -= Subsino Tilemaps Hardware =-

                                  driver by   Luca Elia (l.elia@tin.it)


Two 1024x512 tilemaps. Tiles are 8x8x8. There is ram for 512 scroll values (line scroll).

-------------------------------------------------------------------------------------------------------------
Year + Game        CPU         Sound            Custom                            Other
-------------------------------------------------------------------------------------------------------------
98 Ying Hua Lian   AM188-EM    M6295 + YM3812?  SS9601, SS9602                    HM86171 RAMDAC, Battery
99 Bishou Jan      H8/3044     Unknown          SS9601, SS9802, SS9803, SS9904    HM86171 RAMDAC, Battery
-------------------------------------------------------------------------------------------------------------

To do:

[bishjan]

- Add sound (does it send sound commands to another cpu?).
- Game is sometimes too fast (can it read the VBLANK state? saklove can).
- Implement serial communication (used for protection?)

[saklove]

- Verify sound chip (YM3812) and its clock.
- Remove IRQ hacks (when an AM188-EM core will be available).
- Implement serial communication (used for protection?)

************************************************************************************************************/

#include "emu.h"
#include "deprecat.h"
#include "memconv.h"
#include "cpu/h83002/h8.h"
#include "cpu/i86/i86.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "machine/nvram.h"

/***************************************************************************
                              Tilemaps Access
***************************************************************************/

static UINT8	*bishjan_colorram;
static UINT8	*bishjan_videoram_1_lo,	*bishjan_videoram_1_hi;
static UINT8	*bishjan_videoram_2_lo,	*bishjan_videoram_2_hi;

static tilemap_t	*tmap_1;
static tilemap_t	*tmap_2;

static TILE_GET_INFO( get_tile_info_1 )	{	SET_TILE_INFO(0, (bishjan_videoram_1_hi[ tile_index ] << 8) + bishjan_videoram_1_lo[ tile_index ], 0, 0);	}
static TILE_GET_INFO( get_tile_info_2 )	{	SET_TILE_INFO(0, (bishjan_videoram_2_hi[ tile_index ] << 8) + bishjan_videoram_2_lo[ tile_index ], 0, 0);	}

static UINT8 bishjan_byte_lo;

static WRITE8_HANDLER( bishjan_byte_lo_w )
{
	bishjan_byte_lo = data;
}

static WRITE8_HANDLER( bishjan_videoram_1_hi_w )
{
	bishjan_videoram_1_hi[offset] = data;
	tilemap_mark_tile_dirty(tmap_1, offset);
}
static WRITE8_HANDLER( bishjan_videoram_1_lo_w )
{
	bishjan_videoram_1_lo[offset] = data;
	tilemap_mark_tile_dirty(tmap_1, offset);
}
static WRITE8_HANDLER( bishjan_videoram_1_hi_lo_w )
{
	bishjan_videoram_1_hi_w(space, offset, data);
	bishjan_videoram_1_lo_w(space, offset, bishjan_byte_lo);
}


static WRITE8_HANDLER( bishjan_videoram_2_hi_w )
{
	bishjan_videoram_2_hi[offset] = data;
	tilemap_mark_tile_dirty(tmap_2, offset);
}
static WRITE8_HANDLER( bishjan_videoram_2_lo_w )
{
	bishjan_videoram_2_lo[offset] = data;
	tilemap_mark_tile_dirty(tmap_2, offset);
}
static WRITE8_HANDLER( bishjan_videoram_2_hi_lo_w )
{
	bishjan_videoram_2_hi_w(space, offset, data);
	bishjan_videoram_2_lo_w(space, offset, bishjan_byte_lo);
}


static READ8_HANDLER( bishjan_videoram_1_lo_r )	{	return bishjan_videoram_1_lo[offset];	}
static READ8_HANDLER( bishjan_videoram_1_hi_r )	{	return bishjan_videoram_1_hi[offset];	}

static READ8_HANDLER( bishjan_videoram_2_lo_r )	{	return bishjan_videoram_2_lo[offset];	}
static READ8_HANDLER( bishjan_videoram_2_hi_r )	{	return bishjan_videoram_2_hi[offset];	}

// 16-bit handlers for an 8-bit chip

static WRITE8TO16BE_MSB( bishjan_byte_lo_msb,            bishjan_byte_lo_w );

static READ8TO16BE( bishjan_videoram_1_lo_word,    bishjan_videoram_1_lo_r );
static READ8TO16BE( bishjan_videoram_1_hi_word,    bishjan_videoram_1_hi_r );
static WRITE8TO16BE    ( bishjan_videoram_1_hi_lo_word, bishjan_videoram_1_hi_lo_w );

static READ8TO16BE( bishjan_videoram_2_lo_word,    bishjan_videoram_2_lo_r );
static READ8TO16BE( bishjan_videoram_2_hi_word,    bishjan_videoram_2_hi_r );
static WRITE8TO16BE    ( bishjan_videoram_2_hi_lo_word, bishjan_videoram_2_hi_lo_w );


/***************************************************************************
                              Tilemaps Scroll
***************************************************************************/

static int bishjan_scroll_1_x, bishjan_scroll_1_y;
static int bishjan_scroll_2_x, bishjan_scroll_2_y;

// line scroll
static UINT8 *bishjan_scrollram_1_lo, *bishjan_scrollram_1_hi;
static UINT8 *bishjan_scrollram_2_lo, *bishjan_scrollram_2_hi;

static WRITE8_HANDLER( bishjan_scroll_w )
{
	switch ( offset )
	{
		// tmap_1
		case 0:	bishjan_scroll_1_x = (bishjan_scroll_1_x & 0xf00) | data;	break;				// x low
		case 1:	bishjan_scroll_1_y = (bishjan_scroll_1_y & 0xf00) | data;	break;				// y low
		case 2:	bishjan_scroll_1_x = (bishjan_scroll_1_x & 0x0ff) | ((data & 0x0f) << 8);		// y|x high bits
				bishjan_scroll_1_y = (bishjan_scroll_1_y & 0x0ff) | ((data & 0xf0) << 4);	break;

		// tmap_2
		case 3:	bishjan_scroll_2_x = (bishjan_scroll_2_x & 0xf00) | data;	break;				// x low
		case 4:	bishjan_scroll_2_y = (bishjan_scroll_2_y & 0xf00) | data;	break;				// y low
		case 5:	bishjan_scroll_2_x = (bishjan_scroll_2_x & 0x0ff) | ((data & 0x0f) << 8);		// y|x high bits
				bishjan_scroll_2_y = (bishjan_scroll_2_y & 0x0ff) | ((data & 0xf0) << 4);	break;
	}
}

static READ8_HANDLER ( bishjan_scrollram_1_lo_r )	{	return bishjan_scrollram_1_lo[offset];	}
static READ8_HANDLER ( bishjan_scrollram_1_hi_r )	{	return bishjan_scrollram_1_hi[offset];	}
static WRITE8_HANDLER( bishjan_scrollram_1_lo_w )	{	bishjan_scrollram_1_lo[offset] = data;	}
static WRITE8_HANDLER( bishjan_scrollram_1_hi_w )	{	bishjan_scrollram_1_hi[offset] = data;	}

static READ8_HANDLER ( bishjan_scrollram_2_lo_r )	{	return bishjan_scrollram_2_lo[offset];	}
static READ8_HANDLER ( bishjan_scrollram_2_hi_r )	{	return bishjan_scrollram_2_hi[offset];	}
static WRITE8_HANDLER( bishjan_scrollram_2_lo_w )	{	bishjan_scrollram_2_lo[offset] = data;	}
static WRITE8_HANDLER( bishjan_scrollram_2_hi_w )	{	bishjan_scrollram_2_hi[offset] = data;	}

static WRITE8_HANDLER( bishjan_scrollram_1_hi_lo_w )
{
	bishjan_scrollram_1_hi[offset] = data;
	bishjan_scrollram_1_lo[offset] = bishjan_byte_lo;
}
static WRITE8_HANDLER( bishjan_scrollram_2_hi_lo_w )
{
	bishjan_scrollram_2_hi[offset] = data;
	bishjan_scrollram_2_lo[offset] = bishjan_byte_lo;
}

// 16-bit handlers for an 8-bit chip

static WRITE8TO16BE( bishjan_scroll_word, bishjan_scroll_w );

static READ8TO16BE( bishjan_scrollram_1_lo_word,    bishjan_scrollram_1_lo_r );
static WRITE8TO16BE( bishjan_scrollram_1_lo_word,   bishjan_scrollram_1_lo_w );
static READ8TO16BE( bishjan_scrollram_1_hi_word,    bishjan_scrollram_1_hi_r );
static WRITE8TO16BE( bishjan_scrollram_1_hi_word,    bishjan_scrollram_1_hi_w );
static WRITE8TO16BE    ( bishjan_scrollram_1_hi_lo_word, bishjan_scrollram_1_hi_lo_w );

static READ8TO16BE( bishjan_scrollram_2_lo_word,    bishjan_scrollram_2_lo_r );
static WRITE8TO16BE( bishjan_scrollram_2_lo_word,    bishjan_scrollram_2_lo_w );
static READ8TO16BE( bishjan_scrollram_2_hi_word,    bishjan_scrollram_2_hi_r );
static WRITE8TO16BE( bishjan_scrollram_2_hi_word,    bishjan_scrollram_2_hi_w );
static WRITE8TO16BE    ( bishjan_scrollram_2_hi_lo_word, bishjan_scrollram_2_hi_lo_w );


/***************************************************************************
                              Tilemaps Disable
***************************************************************************/

static int bishjan_disable;

static WRITE8_HANDLER( bishjan_disable_w )
{
	bishjan_disable = data;
}

// 16-bit handlers for an 8-bit chip

static WRITE8TO16BE_LSB( bishjan_disable_lsb, bishjan_disable_w );


/***************************************************************************
                                Video Update
***************************************************************************/

static VIDEO_START(bishjan)
{
	tmap_1 = tilemap_create(	machine, get_tile_info_1, tilemap_scan_rows, 8,8, 0x80,0x40	);
	tmap_2 = tilemap_create(	machine, get_tile_info_2, tilemap_scan_rows, 8,8, 0x80,0x40	);

	tilemap_set_transparent_pen(tmap_1, 0);
	tilemap_set_transparent_pen(tmap_2, 0);

	// line scroll
	tilemap_set_scroll_rows(tmap_1, 0x200);
	tilemap_set_scroll_rows(tmap_2, 0x200);

	tilemap_set_scrolldy( tmap_1, -1, +1 );
	tilemap_set_scrolldy( tmap_2, -1, +1 );

	bishjan_videoram_1_lo = auto_alloc_array(machine, UINT8, 0x80 * 0x40);
	bishjan_videoram_1_hi = auto_alloc_array(machine, UINT8, 0x80 * 0x40);

	bishjan_videoram_2_lo = auto_alloc_array(machine, UINT8, 0x80 * 0x40);
	bishjan_videoram_2_hi = auto_alloc_array(machine, UINT8, 0x80 * 0x40);

	bishjan_scrollram_1_lo = auto_alloc_array(machine, UINT8, 0x200);
	bishjan_scrollram_1_hi = auto_alloc_array(machine, UINT8, 0x200);

	bishjan_scrollram_2_lo = auto_alloc_array(machine, UINT8, 0x200);
	bishjan_scrollram_2_hi = auto_alloc_array(machine, UINT8, 0x200);

	bishjan_videoram_2_hi = auto_alloc_array(machine, UINT8, 0x80 * 0x40);

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

	for (y = 0; y < 0x200; y++)
	{
		// line scroll

		tilemap_set_scrollx( tmap_1, y, bishjan_scroll_1_x + bishjan_scrollram_1_lo[y] + (bishjan_scrollram_1_hi[y]<<8) );
		tilemap_set_scrolly( tmap_1, 0, bishjan_scroll_1_y );

		tilemap_set_scrollx( tmap_2, y, bishjan_scroll_2_x + bishjan_scrollram_2_lo[y] + (bishjan_scrollram_2_hi[y]<<8) );
		tilemap_set_scrolly( tmap_2, 0, bishjan_scroll_2_y );
	}

	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine));

	if (layers_ctrl & 1)	tilemap_draw(bitmap,cliprect, tmap_1, 0, 0);
	if (layers_ctrl & 2)	tilemap_draw(bitmap,cliprect, tmap_2, 0, 0);

//  popmessage("SCROLL: %03x,%03x - %03x,%03x DISABLE: %02x", bishjan_scroll_1_x,bishjan_scroll_1_y, bishjan_scroll_2_x,bishjan_scroll_2_y, bishjan_disable);

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

// 16-bit handlers for an 8-bit chip

static WRITE8TO16BE( colordac_word, colordac_w );


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
		(mame_rand(space->machine) & 0x9800)	|	// bit 7 - serial communication
		(((bishjan_sel==0x12) ? 0x40:0x00) << 8) |
//      (mame_rand() & 0xff);
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


	// read lo (2)   (only half tilemap?)
	AM_RANGE( 0x412000, 0x412fff ) AM_READ( bishjan_videoram_2_lo_word_r )
	AM_RANGE( 0x413000, 0x4131ff ) AM_READWRITE( bishjan_scrollram_2_lo_word_r, bishjan_scrollram_2_lo_word_w )
	// read lo (1)
	AM_RANGE( 0x416000, 0x416fff ) AM_READ( bishjan_videoram_1_lo_word_r )
	AM_RANGE( 0x417000, 0x4171ff ) AM_READWRITE( bishjan_scrollram_1_lo_word_r, bishjan_scrollram_1_lo_word_w )

	// read hi (2)
	AM_RANGE( 0x422000, 0x422fff ) AM_READ( bishjan_videoram_2_hi_word_r )
	AM_RANGE( 0x423000, 0x4231ff ) AM_READWRITE( bishjan_scrollram_2_hi_word_r, bishjan_scrollram_2_hi_word_w )
	// read hi (1)
	AM_RANGE( 0x426000, 0x426fff ) AM_READ( bishjan_videoram_1_hi_word_r )
	AM_RANGE( 0x427000, 0x4271ff ) AM_READWRITE( bishjan_scrollram_1_hi_word_r, bishjan_scrollram_1_hi_word_w )

	// write both (2)
	AM_RANGE( 0x430000, 0x431fff ) AM_WRITE( bishjan_videoram_2_hi_lo_word_w )
	AM_RANGE( 0x432000, 0x432fff ) AM_WRITE( bishjan_videoram_2_hi_lo_word_w )
	AM_RANGE( 0x433000, 0x4331ff ) AM_WRITE( bishjan_scrollram_2_hi_lo_word_w )
	// write both (1)
	AM_RANGE( 0x434000, 0x435fff ) AM_WRITE( bishjan_videoram_1_hi_lo_word_w )
	AM_RANGE( 0x436000, 0x436fff ) AM_WRITE( bishjan_videoram_1_hi_lo_word_w )
	AM_RANGE( 0x437000, 0x4371ff ) AM_WRITE( bishjan_scrollram_1_hi_lo_word_w )


	AM_RANGE( 0x600000, 0x600001 ) AM_READNOP AM_WRITE( bishjan_sel_w )
	AM_RANGE( 0x600060, 0x600063 ) AM_WRITE( colordac_word_w )
	AM_RANGE( 0x6000a0, 0x6000a1 ) AM_WRITE( bishjan_byte_lo_msb_w )

	AM_RANGE( 0xa0001e, 0xa0001f ) AM_WRITE( bishjan_disable_lsb_w )

	AM_RANGE( 0xa00020, 0xa00025 ) AM_WRITE( bishjan_scroll_word_w )

	AM_RANGE( 0xc00000, 0xc00001 ) AM_READ_PORT("DSW")								// SW1
	AM_RANGE( 0xc00002, 0xc00003 ) AM_READ_PORT("JOY") AM_WRITE( bishjan_input_w )	// IN C
	AM_RANGE( 0xc00004, 0xc00005 ) AM_READ( bishjan_input_r )						// IN A & B
	AM_RANGE( 0xc00006, 0xc00007 ) AM_READ( bishjan_serial_r )						// IN D
	AM_RANGE( 0xc00008, 0xc00009 ) AM_READ_PORT("RESET") AM_WRITE( bishjan_coin_w )
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
	AM188EM_IMASK = 0x28
};

static READ8_HANDLER( am188em_regs_r )
{
	return am188em_regs[offset];
}

static WRITE8_HANDLER( am188em_regs_w )
{
	am188em_regs[offset] = data;
}

static MACHINE_RESET( saklove )
{
	// start with masked interrupts
	am188em_regs[AM188EM_IMASK+0] = 0xfd;
	am188em_regs[AM188EM_IMASK+1] = 0x07;
}

static ADDRESS_MAP_START( saklove_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x00000, 0x07fff) AM_RAM AM_SHARE("nvram")	// battery

	// read lo (2)   (only half tilemap?)
	AM_RANGE(0x12000, 0x12fff) AM_READWRITE( bishjan_videoram_2_lo_r, bishjan_videoram_2_lo_w )
	AM_RANGE(0x13000, 0x131ff) AM_READWRITE( bishjan_scrollram_2_lo_r, bishjan_scrollram_2_lo_w )
	// read lo (1)
	AM_RANGE(0x16000, 0x16fff) AM_READWRITE( bishjan_videoram_1_lo_r, bishjan_videoram_1_lo_w )
	AM_RANGE(0x17000, 0x171ff) AM_READWRITE( bishjan_scrollram_1_lo_r, bishjan_scrollram_1_lo_w )

	// read hi (2)
	AM_RANGE(0x22000, 0x22fff) AM_READWRITE( bishjan_videoram_2_hi_r, bishjan_videoram_2_hi_w )
	AM_RANGE(0x23000, 0x231ff) AM_READWRITE( bishjan_scrollram_2_hi_r, bishjan_scrollram_2_hi_w )
	// read hi (1)
	AM_RANGE(0x26000, 0x26fff) AM_READWRITE( bishjan_videoram_1_hi_r, bishjan_videoram_1_hi_w )
	AM_RANGE(0x27000, 0x271ff) AM_READWRITE( bishjan_scrollram_1_hi_r, bishjan_scrollram_1_hi_w )

	// write both (2)
	AM_RANGE(0x30000, 0x31fff) AM_READWRITE( bishjan_videoram_2_hi_r, bishjan_videoram_2_hi_lo_w )
	// write both (1)
	AM_RANGE(0x34000, 0x35fff) AM_READWRITE( bishjan_videoram_1_hi_r, bishjan_videoram_1_hi_lo_w )

	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("maincpu",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( saklove_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0020, 0x0020) AM_DEVREADWRITE_MODERN("oki", okim6295_device, read, write)
	AM_RANGE(0x0040, 0x0041) AM_DEVWRITE( "ymsnd", ym3812_w )

	AM_RANGE(0x0060, 0x0062) AM_WRITE( colordac_w )

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
                                Machine Drivers
***************************************************************************/

/***************************************************************************
                                Bishou Jan
***************************************************************************/

static INTERRUPT_GEN( bishjan_interrupt )
{
	switch (cpu_getiloops(device))
	{
		case 0:
			generic_pulse_irq_line(device, 0);
			break;
		default:
			cputag_set_input_line(device->machine, "maincpu", H8_METRO_TIMER_HACK, HOLD_LINE);
			break;
	}
}

static MACHINE_CONFIG_START( bishjan, driver_device )
	MDRV_CPU_ADD("maincpu", H83044, XTAL_44_1MHz / 3)
	MDRV_CPU_PROGRAM_MAP( bishjan_map)
	MDRV_CPU_VBLANK_INT_HACK(bishjan_interrupt,2)

	MDRV_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 512, 256 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MDRV_SCREEN_REFRESH_RATE( 60 )

	MDRV_GFXDECODE(bishjan)
	MDRV_PALETTE_LENGTH( 256 )

	MDRV_VIDEO_START( bishjan )
	MDRV_VIDEO_UPDATE( bishjan )
MACHINE_CONFIG_END

/***************************************************************************
                          Sakura Love - Ying Hua Lian
***************************************************************************/

static INTERRUPT_GEN( saklove_interrupt )
{
	if ((am188em_regs[AM188EM_IMASK+0] & 0x01) == 0)	// TMR mask
		cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x4c/4);
}

static MACHINE_CONFIG_START( saklove, driver_device )
	MDRV_CPU_ADD("maincpu", I80188, XTAL_20MHz )	// !! AMD AM188-EM !!
	MDRV_CPU_PROGRAM_MAP( saklove_map)
	MDRV_CPU_IO_MAP( saklove_io)
	MDRV_CPU_VBLANK_INT( "screen", saklove_interrupt )

	MDRV_MACHINE_RESET(saklove)
	MDRV_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE( 512, 256 )
	MDRV_SCREEN_VISIBLE_AREA( 0, 512-1, 0, 256-16-1 )
	MDRV_SCREEN_REFRESH_RATE( 58.7270 )

	MDRV_GFXDECODE(bishjan)
	MDRV_PALETTE_LENGTH( 256 )

	MDRV_VIDEO_START( bishjan )
	MDRV_VIDEO_UPDATE( bishjan )

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_OKIM6295_ADD("oki", XTAL_8_4672MHz / 8, OKIM6295_PIN7_HIGH)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MDRV_SOUND_ADD("ymsnd", YM3812, XTAL_12MHz / 4)	// ? chip and clock unknown
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
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

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "2-v201.u9", 0x000000, 0x100000, CRC(ea42764d) SHA1(13fe1cd30e474f4b092949c440068e9ddca79976) )
ROM_END

static DRIVER_INIT(bishjan)
{
	UINT16 *rom = (UINT16*)memory_region(machine, "maincpu");

	// check
	rom[0x042EA/2] = 0x4008;

	// rts -> rte
	rom[0x33386/2] = 0x5670;
	rom[0x0CC5C/2] = 0x5670;
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
	UINT8 *rom = memory_region(machine, "maincpu");

	// patch protection test (it always enters test mode on boot otherwise)
	rom[0x0e029] = 0xeb;
}


GAME( 1998, saklove, 0, saklove, saklove, saklove, ROT0, "Subsino", "Ying Hua Lian 2.0 (China 1.02)", 0 )
GAME( 1999, bishjan, 0, bishjan, bishjan, bishjan, ROT0, "Subsino", "Bishou Jan (Japan 203)", GAME_NO_SOUND )
