/***************************************************************************

                        -= Subsino's Gambling Games =-

                     driver by   Luca Elia (l.elia@tin.it)
            additional work by   David Haywood and Angelo Salese

CPU:    Z180 (in a black box)
GFX:    1 Tilemap (8x8 tiles, no scrolling)
CUSTOM: 2 x SUBSINO SS9100, SUBSINO SS9101
SOUND:  M6295, YM2413 or YM3812
OTHER:  Battery

To enter test mode in smoto, keep F2 pressed during boot.

***************************************************************************/

#include "driver.h"
#include "cpu/z180/z180.h"
#include "machine/8255ppi.h"
#include "sound/okim6295.h"
#include "sound/2413intf.h"
#include "sound/3812intf.h"

/***************************************************************************
                                Video Hardware
***************************************************************************/

static UINT8 *videoram;
static UINT8 *colorram;
static tilemap_t *tmap;
static int tiles_offset;

static WRITE8_HANDLER( subsino_tiles_offset_w )
{
	tiles_offset = (data & 1) ? 0x1000: 0;
	tilemap_mark_tile_dirty(tmap, offset);
//  popmessage("gfx %02x",data);
}

static WRITE8_HANDLER( subsino_videoram_w )
{
	videoram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static WRITE8_HANDLER( subsino_colorram_w )
{
	colorram[offset] = data;
	tilemap_mark_tile_dirty(tmap, offset);
}

static TILE_GET_INFO( get_tile_info )
{
	UINT16 code = videoram[ tile_index ] + (colorram[ tile_index ] << 8);
	UINT16 color = (code >> 8) & 0x0f;
	code = ((code & 0xf000) >> 4) + ((code & 0xff) >> 0);
	code += tiles_offset;
	SET_TILE_INFO(0, code, color, 0);
}

static VIDEO_START( subsino )
{
	tmap = tilemap_create(	machine, get_tile_info, tilemap_scan_rows, 8,8, 0x40,0x20 );
	tilemap_set_transparent_pen( tmap, 0 );
	tiles_offset = 0;
}

static VIDEO_UPDATE( subsino )
{
	bitmap_fill(bitmap,cliprect,0);
	tilemap_draw(bitmap,cliprect, tmap, 0, 0);
	return 0;
}

static PALETTE_INIT( subsino_2proms )
{
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x100]) | (color_prom[i+0x000]<<4);

		bit0 = 0;
		bit1 = (val >> 6) & 0x01;
		bit2 = (val >> 7) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 3) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 5) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 0) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

static PALETTE_INIT( subsino_3proms )
{
	int i,r,g,b,val;
	int bit0,bit1,bit2;

	for (i = 0; i < 256; i++)
	{
		val = (color_prom[i+0x000]) | (color_prom[i+0x100]<<3) | (color_prom[i+0x200]<<6);

		bit0 = 0;
		bit1 = (val >> 7) & 0x01;
		bit2 = (val >> 6) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 5) & 0x01;
		bit1 = (val >> 4) & 0x01;
		bit2 = (val >> 3) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (val >> 2) & 0x01;
		bit1 = (val >> 1) & 0x01;
		bit2 = (val >> 0) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

static WRITE8_HANDLER( subsino_out_a_w )
{
	coin_counter_w( space->machine, 0, data & 0x02 );
//  popmessage("Out A %02x",data);
}
static WRITE8_HANDLER( subsino_out_b_w )
{
	// leds
//  popmessage("Out B %02x",data);
}

static ADDRESS_MAP_START( srider_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x00000, 0x0bfff ) AM_ROM

	AM_RANGE( 0x0c000, 0x0cfff ) AM_RAM

	AM_RANGE( 0x0d000, 0x0d000 ) AM_READ_PORT( "DSW1" )
	AM_RANGE( 0x0d001, 0x0d001 ) AM_READ_PORT( "DSW2" )
	AM_RANGE( 0x0d002, 0x0d002 ) AM_READ_PORT( "DSW3" )

	AM_RANGE( 0x0d004, 0x0d004 ) AM_READ_PORT( "DSW4" )
	AM_RANGE( 0x0d005, 0x0d005 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x0d006, 0x0d006 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x0d009, 0x0d009 ) AM_WRITE( subsino_out_b_w )
	AM_RANGE( 0x0d00a, 0x0d00a ) AM_WRITE( subsino_out_a_w )

	AM_RANGE( 0x0d00c, 0x0d00c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x0d016, 0x0d017 ) AM_DEVWRITE( "ymsnd", ym3812_w )

	AM_RANGE( 0x0d018, 0x0d018 ) AM_DEVWRITE( "oki", okim6295_w )

	AM_RANGE( 0x0d01b, 0x0d01b ) AM_WRITE( subsino_tiles_offset_w )

	AM_RANGE( 0x0e000, 0x0e7ff ) AM_RAM_WRITE( subsino_colorram_w ) AM_BASE( &colorram )
	AM_RANGE( 0x0e800, 0x0efff ) AM_RAM_WRITE( subsino_videoram_w ) AM_BASE( &videoram )
ADDRESS_MAP_END


static ADDRESS_MAP_START( sharkpy_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_READ_PORT( "DSW1" )
	AM_RANGE( 0x09001, 0x09001 ) AM_READ_PORT( "DSW2" )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "DSW3" )

	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "DSW4" )
	AM_RANGE( 0x09005, 0x09005 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09006, 0x09006 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x09009, 0x09009 ) AM_WRITE( subsino_out_b_w )
	AM_RANGE( 0x0900a, 0x0900a ) AM_WRITE( subsino_out_a_w )

	AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x09016, 0x09017 ) AM_DEVWRITE( "ymsnd", ym3812_w )

	AM_RANGE( 0x09018, 0x09018 ) AM_DEVWRITE( "oki", okim6295_w )

	AM_RANGE( 0x0901b, 0x0901b ) AM_WRITE( subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE( subsino_colorram_w ) AM_BASE( &colorram )
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE( subsino_videoram_w ) AM_BASE( &videoram )

	AM_RANGE( 0x00000, 0x13fff ) AM_ROM //overlap unmapped regions
ADDRESS_MAP_END

static ADDRESS_MAP_START( victor21_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "INC" )
	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09005, 0x09005 ) AM_READ_PORT( "INB" )

	AM_RANGE( 0x09006, 0x09006 ) AM_READ_PORT( "DSW1" )
	AM_RANGE( 0x09007, 0x09007 ) AM_READ_PORT( "DSW2" )
	AM_RANGE( 0x09008, 0x09008 ) AM_READ_PORT( "DSW3" )

//  AM_RANGE( 0x09009, 0x09009 ) AM_WRITE( subsino_out_b_w )
//  AM_RANGE( 0x0900a, 0x0900a ) AM_WRITE( subsino_out_a_w )

//  AM_RANGE( 0x0900b, 0x0900b ) //"flash" status, bit 0
//  AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x0900e, 0x0900f ) AM_DEVWRITE( "ymsnd", ym2413_w )

//  AM_RANGE( 0x09018, 0x09018 ) AM_DEVWRITE( "oki", okim6295_w )

	AM_RANGE( 0x0900d, 0x0900d ) AM_WRITE( subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE( subsino_videoram_w ) AM_BASE( &videoram )
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE( subsino_colorram_w ) AM_BASE( &colorram )

	AM_RANGE( 0x00000, 0x8fff ) AM_ROM //overlap unmapped regions
	AM_RANGE( 0x10000, 0x13fff ) AM_ROM
ADDRESS_MAP_END


/*
Victor 5 has a protection device that calculates the combinations of the screen.
0x00 = you lose
0x01 = royal flush
0x02 = 5 of a kind
 ...
0x0a = jacks of better

0xd9 is asked at the POST, game refuses to run otherwise.

The fun thing is that the main CPU actually calculates the combinations then it routes to this port
thru packet writes, 0xd* starts the packet, 0xe* ends the packet.
I'm fairly sure that this thing can do a lot more, the POST check makes me think that 0xd9 is actually a result of a RNG.
For now we'll emulate the bare minimum to let this game to work, obviously we need tests/tracing on the real board to understand
what it is exactly and what it can possibly do.
*/

static UINT8 flash_val,flash_packet,flash_packet_start;

static READ8_HANDLER( flash_r )
{
//  printf("R %02x\n",flash_val);

	if(flash_val == 0xff)
		return 0xd9;
	else if(flash_val <= 0xa)
		return flash_val;
	else if(flash_val == 0x92)
		return 0xeb; //protected GFXs in Cross Bingo
	else
		return 0xd9;
}

static WRITE8_HANDLER( flash_w )
{
	switch(flash_packet_start)
	{
		case 0:
			flash_packet = data;
			if((flash_packet & 0xf8) == 0xd0)
				flash_packet_start = 1; //start of packet
			break;
		case 1:
			flash_packet = data;
			if((flash_packet & 0xf8) == 0xe0)
				flash_packet_start = 0; //end of packet
			else
				flash_val = data;
			break;
	}
}

static ADDRESS_MAP_START( victor5_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_IMPORT_FROM( victor21_map )
	AM_RANGE( 0x0900a, 0x0900a ) AM_READWRITE( flash_r, flash_w )
ADDRESS_MAP_END


static READ8_HANDLER( hwcheck_r )
{
	/* Wants this at POST otherwise an "Hardware Error" occurs. */
	return 0x55;
}

static ADDRESS_MAP_START( crsbingo_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE( 0x09800, 0x09fff ) AM_RAM

	AM_RANGE( 0x09000, 0x09000 ) AM_READ_PORT( "SW1" )
	AM_RANGE( 0x09001, 0x09001 ) AM_READ_PORT( "SW2" )
	AM_RANGE( 0x09002, 0x09002 ) AM_READ_PORT( "INA" )
	AM_RANGE( 0x09003, 0x09003 ) AM_READ_PORT( "INB" )
	AM_RANGE( 0x09004, 0x09004 ) AM_READ_PORT( "INC" )

	AM_RANGE( 0x09008, 0x09008 ) AM_READ_PORT( "SW4" )
	AM_RANGE( 0x09009, 0x09009 ) AM_READ_PORT( "SW3" ) AM_WRITE( subsino_out_b_w )
	AM_RANGE( 0x0900a, 0x0900a ) AM_READ( hwcheck_r )

	AM_RANGE( 0x09010, 0x09010 ) AM_READWRITE( flash_r, flash_w )
//  AM_RANGE( 0x09011, 0x09011 ) //"flash" status, bit 0
//  AM_RANGE( 0x0900c, 0x0900c ) AM_READ_PORT( "INC" )
	AM_RANGE( 0x0900c, 0x0900d ) AM_DEVWRITE( "ymsnd", ym2413_w )

//  AM_RANGE( 0x09018, 0x09018 ) AM_DEVWRITE( "oki", okim6295_w )

//  AM_RANGE( 0x0900d, 0x0900d ) AM_WRITE( subsino_tiles_offset_w )

	AM_RANGE( 0x07800, 0x07fff ) AM_RAM
	AM_RANGE( 0x08000, 0x087ff ) AM_RAM_WRITE( subsino_videoram_w ) AM_BASE( &videoram )
	AM_RANGE( 0x08800, 0x08fff ) AM_RAM_WRITE( subsino_colorram_w ) AM_BASE( &colorram )

	AM_RANGE( 0x00000, 0x8fff ) AM_ROM //overlap unmapped regions

	AM_RANGE( 0x10000, 0x13fff ) AM_ROM //overlap unmapped regions

ADDRESS_MAP_END


static ADDRESS_MAP_START( subsino_iomap, ADDRESS_SPACE_IO, 8 )
	AM_RANGE( 0x0000, 0x003f ) AM_RAM // internal regs
ADDRESS_MAP_END

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( smoto )

	PORT_START( "DSW1" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "DSW2" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "DSW3" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "DSW4" )
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )	// 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )	// 2 ->
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )	// 3 <-
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )	// 4 choose
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )	// 5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1  )	// start
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2  )	// bet (power->speed)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1    )	// coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )	// test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	// rate
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_START( "INC" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START4  )	// take
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( victor5 )

	PORT_START( "DSW1" )
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "DSW2" )
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "DSW3" )
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Half Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )
	PORT_DIPNAME( 0x80, 0x80, "INA" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	// coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )	// coin
	PORT_DIPNAME( 0x04, 0x04, "INB" )
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

	PORT_START( "INC" )
	PORT_DIPNAME( 0x01, 0x01, "INC" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
INPUT_PORTS_END

static INPUT_PORTS_START( victor21 )

	PORT_START( "DSW1" )
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "DSW2" )
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "DSW3" )
	PORT_DIPNAME( 0x01, 0x01, "DSW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "INA" )
	PORT_DIPNAME( 0x01, 0x01, "INA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HALF ) PORT_NAME("Split")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Hit")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_STAND )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	// coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )	// coin
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )	// coin
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )	// coin
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )	// test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	PORT_NAME("Win Rate")
	PORT_DIPNAME( 0x40, 0x40, "INB" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INC" )
	PORT_DIPNAME( 0x01, 0x01, "INC" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 )	PORT_NAME("Reset")
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
INPUT_PORTS_END

static INPUT_PORTS_START( crsbingo )

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x01, 0x01, "SW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x01, 0x01, "SW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "SW3" )
	PORT_DIPNAME( 0x01, 0x01, "SW3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "SW4" )
	PORT_DIPNAME( 0x01, 0x01, "SW4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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

	PORT_START( "INA" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3 / Half Take")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / Small")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Big")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_BET )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INB" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )	// coin
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )	// coin
	PORT_DIPNAME( 0x04, 0x04, "INB" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )	// test
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START( "INC" )
	PORT_DIPNAME( 0x01, 0x01, "INC" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
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
INPUT_PORTS_END


/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_8x8x3 =
{
	8, 8,
	RGN_FRAC(1, 3),
	3,
	{ RGN_FRAC(0,3),RGN_FRAC(1,3),RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout layout_8x8x4 =
{
	8, 8,
	RGN_FRAC(1, 4),
	4,
	{ RGN_FRAC(2,4),RGN_FRAC(3,4),RGN_FRAC(0,4),RGN_FRAC(1,4) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( subsino_depth3 )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x3, 0, 16 )
GFXDECODE_END

static GFXDECODE_START( subsino_depth4 )
	GFXDECODE_ENTRY( "tilemap", 0, layout_8x8x4, 0, 16 )
GFXDECODE_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

static MACHINE_DRIVER_START( victor21 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 3)	// 4 MHz?
	MDRV_CPU_PROGRAM_MAP(victor21_map)
	MDRV_CPU_IO_MAP(subsino_iomap)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(subsino_depth3)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(subsino_2proms)

	MDRV_VIDEO_START(subsino)
	MDRV_VIDEO_UPDATE(subsino)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_4_433619MHz / 4)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/* same but with an additional protection. */
static MACHINE_DRIVER_START( victor5 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM( victor21 )
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(victor5_map)
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( srider )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 3)	// 4 MHz?
	MDRV_CPU_PROGRAM_MAP(srider_map)
	MDRV_CPU_IO_MAP(subsino_iomap)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(subsino_depth4)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(subsino_3proms)

	MDRV_VIDEO_START(subsino)
	MDRV_VIDEO_UPDATE(subsino)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, XTAL_3_579545MHz)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("oki", OKIM6295, XTAL_4_433619MHz / 4)
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( sharkpy )
	MDRV_IMPORT_FROM(srider)

	/* basic machine hardware */
	MDRV_CPU_MODIFY("maincpu")
	MDRV_CPU_PROGRAM_MAP(sharkpy_map)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( crsbingo )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z180, XTAL_12MHz / 3)	// Unknown CPU and clock
	MDRV_CPU_PROGRAM_MAP(crsbingo_map)
	MDRV_CPU_IO_MAP(subsino_iomap)

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0, 512-1, 0+16, 256-16-1)

	MDRV_GFXDECODE(subsino_depth4)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(subsino_2proms)

	MDRV_VIDEO_START(subsino)
	MDRV_VIDEO_UPDATE(subsino)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM2413, XTAL_3_579545MHz)	// unknown clock
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_DRIVER_END

/***************************************************************************
                                ROMs Loading
***************************************************************************/

/***************************************************************************

Victor 5
(C)1991 Subsino

Chips:

1x unknown big black box
1x M5L8255AP
1x UM3567
1x M6295
1x oscillator 12.000
1x oscillator 4.433619
1x oscillator 3.579545

ROMs:

1x M27C512 (1)
3x 27C256 (2,3,4)

Notes:

1x 36x2 edge connector (con3)
1x 10x2 edge connector (con4)
1x RS232 9pins connector (con5)
2x batteries
3x 8x2 switches dip
1x pushbutton

Sticker on PCB reads V552520

Info by f205v (26/03/2008)

***************************************************************************/

ROM_START( victor5 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u1", 0x10000, 0x4000, CRC(e3ada2fc) SHA1(eddb460dcb80a29fbbe3ed6c4733c75b892baf52) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(1229e951) SHA1(1e548625bb60e2d6f52a376a0dea9e5709f94135) )
	ROM_LOAD( "3.u23", 0x08000, 0x8000, CRC(2d89bbf1) SHA1(d7fda0174a835e88b330dfd09bdb604bfe4c2e44) )
	ROM_LOAD( "4.u22", 0x00000, 0x8000, CRC(ecf840a1) SHA1(9ecf522afb23e3557d37effc3c8568e8a14dad1a) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// rom socket is empty

	ROM_REGION( 0x200, "proms", 0 ) //missing proms in this set, we'll use the same as Victor 21 for now.
	ROM_LOAD( "74s287.u35", 0x000, 0x100, BAD_DUMP CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "74s287.u36", 0x100, 0x100, BAD_DUMP CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END

/***************************************************************************

Victor 21
(C)1990 Subsino

Chips:

1x unknown big black box
1x M5L8255AP
1x UM3567
1x M6295
1x oscillator 12.000
1x oscillator 4.433619
1x oscillator 3.579545

ROMs:

1x M27C512 (1)
3x 27C256 (2,3,4)

Other:

1x 36x2 edge connector (con3)
1x 10x2 edge connector (con4)
1x RS232 9pins connector (con5)
2x batteries
3x 8 switches dips
1x pushbutton

PCB layout is identical to "Victor 5"
Sticker on PCB reads V12040

Info by f205v, Corrado Tomaselli (20/04/2008)

***************************************************************************/

ROM_START( victor21 )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u1", 0x10000, 0x4000, CRC(43999b2d) SHA1(7ce26fd332ffe35fd826a1a6166b228d4bc370b8) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x18000, "tilemap", 0 )
	ROM_LOAD( "2.u24", 0x10000, 0x8000, CRC(f1181b93) SHA1(53cd4d2ce13973495b51d911a4745a69a9784983) )
	ROM_LOAD( "3.u25", 0x08000, 0x8000, CRC(437abb27) SHA1(bd3790807d60a41d58e07f60fb990553076d6e96) )
	ROM_LOAD( "4.u26", 0x00000, 0x8000, CRC(e2f66eee) SHA1(ece924fe626f21fd7d31faabf19225d80e2bcfd3) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE )
	// rom socket is empty

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "74s287.u35", 0x000, 0x100, CRC(40094bed) SHA1(b25d96126b3f7bd06bf76dc9958f8669f83abdb7) )
	ROM_LOAD( "74s287.u36", 0x100, 0x100, CRC(9ca021c5) SHA1(6a1d8d4f958d36e4a676dc4f4aee83d637933bc3) )
ROM_END

/***************************************************************************

Shark Party
(C)1993 Subsino

Chips:

1x unknown big black box
1x custom SUBSINO_SS9101_409235I (DIL42)(u48)
2x KD89C55 (u49,u50)
1x K-665 (u55)(equivalent to M6295)
1x K-664 (u57)(equivalent to YM3014)
1x K-666 (u52)(equivalent to YM3812)
3x 45580D (amplifier)(u58,u59,u60)
2x custom SUBSINO_SS9100_3512204V (SMT 44pins)(u10,u19)
1x oscillator 4.433619MHz
1x oscillator 12.000MHz

ROMs:

1x 27C1001 (u54)
1x 27C512 (u18)
2x 27C010 (u16,u17)
3x N82S129AN (u11,u12,u13)
4x GAL16V8B (u2,u37,u45,u46)(not dumped)
2x TIBPAL16L8 (u43,u44)(not dumped)

Other:

1x 36x2 edge connector (con5)
1x 10x2 edge connector (con4)
1x battery
1x trimmer (volume)
1x pushbutton (sw5)
4x 8x2 switches dip (sw1,sw2,sw3,sw4)

Info by f205v (25/03/2008)

***************************************************************************/

ROM_START( sharkpy )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "shark_n.1.u18", 0x0a000, 0x6000, CRC(25aeac2f) SHA1(d94e3e5cfffd150ac48e1463493a8323f42e7a89) ) // is this mapped correctly? - used during gameplay?
	ROM_CONTINUE(0x0000, 0xa000)


	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "shark_n.3.u16", 0x00000, 0x08000, CRC(a7a715ce) SHA1(38b93e05377d9cb816688f5070e847480f195c6b) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_CONTINUE(              0x08000, 0x08000 )
	ROM_CONTINUE(              0x18000, 0x08000 )
	ROM_LOAD( "shark_n.2.u17", 0x20000, 0x08000, CRC(c27f3d0a) SHA1(77c8eb0322c5b9c89777cb080d26ecf9abe01ae7) )
	ROM_CONTINUE(              0x30000, 0x08000 )
	ROM_CONTINUE(              0x28000, 0x08000 )
	ROM_CONTINUE(              0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "shark(ii)-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "n82s129an.u11", 0x000, 0x100, CRC(daf3657a) SHA1(93005938e2d60d54e7bbf1e234bba3802ee1af21) )
	ROM_LOAD( "n82s129an.u12", 0x100, 0x100, CRC(5a7a25ed) SHA1(eebd679195e6ea50f64f3c46cd06ee21a1550491) )
	ROM_LOAD( "n82s129an.u13", 0x200, 0x100, CRC(0ef5f218) SHA1(a02cf266661385aa078563bd83240d36549c1cf0) )
ROM_END
/***************************************************************************

Shark Party (alt)
(C)1993 Subsino

Chips:

1x unknown big black box
1x custom SUBSINO_SS9101_409235I (DIL42)(u48)
2x KD89C55 (u49,u50)
1x K-665 (u55)(equivalent to M6295)
1x K-664 (u57)(equivalent to YM3014)
1x SM64JBCK (u52)(equivalent to YM3812)
3x 45580D (amplifier)(u58,u59,u60)
2x custom SUBSINO_SS9100_3512201V (SMT 44pins)(u10,u19)
1x oscillator 4.433619MHz
1x oscillator 12.000MHz

ROMs:

2x 27C1001 (u54,u17)
1x 27C512 (u18)
1x 27C010 (u16)
3x N82S129AN (u11,u12,u13)
4x GAL16V8B (u2,u37,u45,u46)(not dumped)
2x TIBPAL16L8 (u43,u44)(not dumped)

Other:

1x 36x2 edge connector (con5)
1x 10x2 edge connector (con4)
1x battery
1x trimmer (volume)
1x pushbutton (sw5)
4x 8x2 switches dip (sw1,sw2,sw3,sw4)

Info by f205v (25/03/2008)

***************************************************************************/


ROM_START( sharkpya )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "shark1.6.u18", 0x0a000, 0x6000, CRC(365312a0) SHA1(de8370b1f35e8d071185d2e5f2fbd2fdf74c55ac) )
	ROM_CONTINUE(0x0000, 0xa000)

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "shark_n.3.u16", 0x00000, 0x08000, CRC(a7a715ce) SHA1(38b93e05377d9cb816688f5070e847480f195c6b) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_CONTINUE(              0x08000, 0x08000 )
	ROM_CONTINUE(              0x18000, 0x08000 )
	ROM_LOAD( "shark_n.2.u17", 0x20000, 0x08000, CRC(c27f3d0a) SHA1(77c8eb0322c5b9c89777cb080d26ecf9abe01ae7) )
	ROM_CONTINUE(              0x30000, 0x08000 )
	ROM_CONTINUE(              0x28000, 0x08000 )
	ROM_CONTINUE(              0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "shark(ii)-italy_4_ver1.0.u54", 0x00000, 0x20000, CRC(9f384c59) SHA1(d2b087b8370b40b6f0944de661ea6aebaebea06f) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "sn82s129an.u11", 0x000, 0x100, CRC(daf3657a) SHA1(93005938e2d60d54e7bbf1e234bba3802ee1af21) )
	ROM_LOAD( "sn82s129an.u12", 0x100, 0x100, CRC(5a7a25ed) SHA1(eebd679195e6ea50f64f3c46cd06ee21a1550491) )
	ROM_LOAD( "sn82s129an.u13", 0x200, 0x100, CRC(0ef5f218) SHA1(a02cf266661385aa078563bd83240d36549c1cf0) )
ROM_END

/***************************************************************************

Super Rider (Italy Ver 1.6)
(C)1996 Subsino

Chips:

2x custom QFP44 label SUBSINOSS9100
1x custom DIP42 label SUBSINOSS9101
2x FILE KD89C55A (equivalent to 8255)
1x custom QFP44 label M28 (sound)(equivalent to M6295)
1x custom DIP24 label K-666 (sound)(equivalent to YM3812)
1x custom DIP8 label K-664 (sound)(equivalent to YM3014)
1x oscillator 12.000MHz (main)
1x oscillator 4.43361MHz (sound)

ROMs:

1x TMS27C512 (1)
2x TMS27C010A (2,3)(main)
1x TMS27C010A (4) (sound)
3x PROM N82S129AN

Other:

1x 10x2 edge connector (looks like a coin payout)
1x 36x2 edge connector
1x battery 3.6V NiCd
1x pushbutton (sw5)
4x 8 switches dips (sw1-4)
1x trimmer (volume)
1x BIG BLACK BOX (on top of the box there is a small door closing a button-battery; for sure there is more in it, but I do not know how to open it / tore it apart)

This game is the official Italian version of "Super Rider" by Subsino

Info by f205v (29/12/2005)

***************************************************************************/

ROM_START( smoto16 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rideritaly_1ver1.6.u18", 0x0000, 0x10000, CRC(c7c0c3e8) SHA1(5dc80bc775f370653135a7b3ea9c8d3c92263804) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "rideritaly_3ver1.6.u16", 0x00000, 0x08000, CRC(998a8feb) SHA1(27f08b23f2dd3736f4f12f489d9a3aa096c99e8a) )
	ROM_CONTINUE(                       0x10000, 0x08000 )
	ROM_CONTINUE(                       0x08000, 0x08000 )
	ROM_CONTINUE(                       0x18000, 0x08000 )
	ROM_LOAD( "rideritaly_2ver1.6.u17", 0x20000, 0x08000, CRC(bdf9bf26) SHA1(49e7c0b99fec06dca5816eb7e38aed025efcaaa7) )
	ROM_CONTINUE(                       0x30000, 0x08000 )
	ROM_CONTINUE(                       0x28000, 0x08000 )
	ROM_CONTINUE(                       0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rideritaly_4ver1.6.u54", 0x00000, 0x20000, CRC(df828563) SHA1(f39324c5c37486ed9512e0ff934394556dd182ae) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "prom-n82s129an.u11", 0x000, 0x100, CRC(e17730a6) SHA1(50c730b24e1d3d205c70f9381e4136e2ba6e499a) )
	ROM_LOAD( "prom-n82s129an.u12", 0x100, 0x100, CRC(df848861) SHA1(f7e382f8b56d6b9f2af6c7a48a19e3631a64bb6d) )
	ROM_LOAD( "prom-n82s129an.u13", 0x200, 0x100, CRC(9cb4a5c0) SHA1(0e0a368329c6d1cb685ed655d699a4894988fdb1) )
ROM_END

static DRIVER_INIT( smoto16 )
{
	UINT8 *rom = memory_region( machine, "maincpu" );
	rom[0x12d0] = 0x20;	// "ERROR 951010"
}

/***************************************************************************

Super Rider (Italy Ver 2.0)
(C)1997 Subsino

Chips:

2x custom QFP44 label SUBSINOSS9100
1x custom DIP42 label SUBSINOSS9101
2x D8255AC-2 (are they 8255 equivalent?)
1x custom QFP44 label K-665 (sound)(equivalent to OKI M6295)
1x custom DIP24 label SM64 (sound)(equivalent to YM3812)
1x custom DIP8 label K-664 (sound)(equivalent to YM3014)
1x oscillator 12.000MHz (main)
1x oscillator 4.433619MHz (sound)

ROMs:

1x 27C512 (1)
2x M27C1001 (2,3)(main)
1x M27C1001 (4) (sound)
3x PROM N82S129AN
3x PALCE16V8H (not dumped)
2x TIBPAL16L8B (not dumped)
1x GAL16V8B (not dumped)

Other:

1x 10x2 edge connector (looks like a coin payout)
1x 36x2 edge connector
1x battery 3.6V NiCd
4x 8 switches dips (sw1-4)
1x trimmer (volume)
1x BIG BLACK BOX (on top of the box there is a small door closing a button-battery; for sure there is more in it, but I do not know how to open it / tore it apart)

This game is the official Italian version of "Super Rider" by Subsino

Info by f205v, Corrado Tomaselli (20/04/2008)

***************************************************************************/

ROM_START( smoto20 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "italyv2.0-25.u18", 0x00000, 0x10000, CRC(91abc76e) SHA1(b0eb3afda1d94111056559017802b16b2e72a9a5) )

	ROM_REGION( 0x40000, "tilemap", 0 )
	ROM_LOAD( "3.u16", 0x00000, 0x08000, CRC(44b44385) SHA1(27c2865e52ab67aa8e077e8e1202cbf2addc0dfc) )
	ROM_CONTINUE(      0x10000, 0x08000 )
	ROM_CONTINUE(      0x08000, 0x08000 )
	ROM_CONTINUE(      0x18000, 0x08000 )
	ROM_LOAD( "2.u17", 0x20000, 0x08000, CRC(380fc964) SHA1(4a5076d90cb94e2ffeec7534ce64d4cdb320f374) )
	ROM_CONTINUE(      0x30000, 0x08000 )
	ROM_CONTINUE(      0x28000, 0x08000 )
	ROM_CONTINUE(      0x38000, 0x08000 )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "rom4ver1.0.u54", 0x00000, 0x20000, CRC(df828563) SHA1(f39324c5c37486ed9512e0ff934394556dd182ae) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "82s129.u11", 0x000, 0x100, CRC(e17730a6) SHA1(50c730b24e1d3d205c70f9381e4136e2ba6e499a) )
	ROM_LOAD( "82s129.u12", 0x100, 0x100, CRC(df848861) SHA1(f7e382f8b56d6b9f2af6c7a48a19e3631a64bb6d) )
	ROM_LOAD( "82s129.u13", 0x200, 0x100, CRC(9cb4a5c0) SHA1(0e0a368329c6d1cb685ed655d699a4894988fdb1) )
ROM_END

static DRIVER_INIT( smoto20 )
{
	UINT8 *rom = memory_region( machine, "maincpu" );
	rom[0x12e1] = 0x20;	// "ERROR 951010"
}

/***************************************************************************

Cross Bingo
(C)1991 Subsino

Chips:

1x big unknown epoxy block (main)
1x unknown Subsino SS9101-173001 (DIP42)
1x unknown blank quad chip (QFP68)
1x UM3567 (sound)
3x ULN2003AN (sound)
1x LM324N (sound)
1x oscillator 12.000
1x oscillator 3.579545

ROMs:

3x M27512
2x PROM N82S129N
2x PLD 18CV8PC (read protected)

Other:

1x 22x2 edge connector
1x 11x2 edge connector
1x 10x2 edge connector (payout system)
1x RS232 connector
1x trimmer (volume)
1x pushbutton
4x 8x2 switches dip
1x battery

Info by f205v (14/12/2008)

***************************************************************************/

ROM_START( crsbingo )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.u36", 0x10000, 0x4000, CRC(c5aff4eb) SHA1(74f06d7735975657fca9be5fff9e7d53f38fcd02) )
	ROM_CONTINUE(0x0000,0xc000)

	ROM_REGION( 0x20000, "tilemap", 0 )
	ROM_LOAD( "2.u4",  0x00000, 0x10000, CRC(ce527722) SHA1(f3759cefab902259eb25f8d4be2fcafc1afd90b9) )
	ROM_LOAD( "3.u15", 0x10000, 0x10000, CRC(23785451) SHA1(8574e59aa816a644ff4b102bd5754ef1284deea0) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "82s129.u13", 0x000, 0x100, CRC(89c06859) SHA1(b98d5335f36ea3842086677aca47b605030d442f) )
	ROM_LOAD( "82s129.u14", 0x100, 0x100, CRC(eaddb54f) SHA1(51fbf31e910a93315204a892d10bcf982a6ed099) )

	ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )

	ROM_REGION( 0x155 * 2, "plds", 0 )
	ROM_LOAD( "18cv8.u22", 0x000, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
	ROM_LOAD( "18cv8.u29", 0x155, 0x155, BAD_DUMP CRC(996e8f59) SHA1(630d9b91f6e8eda781061e2a8ff6fb0fecaf034c) )
ROM_END

/****************************
 Decryption
****************************/

#if 0
void dump_decrypted(running_machine* machine, UINT8* decrypt)
{
    FILE *fp;
    char filename[256];
    sprintf(filename,"dat_%s", machine->gamedrv->name);
    fp=fopen(filename, "w+b");
    if (fp)
    {
        fwrite(decrypt, 0x10000, 1, fp);
        fclose(fp);
    }
}
#endif

static DRIVER_INIT( victor5 )
{
	unsigned char xors[8] = { 0x99, 0x99, 0x33, 0x44, 0xBB, 0x88, 0x88, 0xBB };

	int i;
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* region = memory_region(machine,"maincpu");
	for (i=0;i<0x10000;i++)
	{
		if (i<0xc000)
		{
			decrypt[i] = region[i]^xors[i&7];

			if ((i&7) == 0) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 1) decrypt[i] = BITSWAP8(decrypt[i],7,6,5,0,3,2,1,4);
			if ((i&7) == 2) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
			if ((i&7) == 3) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
			if ((i&7) == 4) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,0,7,6,5,4);
			if ((i&7) == 5) decrypt[i] = BITSWAP8(decrypt[i],7,6,5,0,3,2,1,4);
			if ((i&7) == 6) decrypt[i] = BITSWAP8(decrypt[i],3,6,1,0,7,2,5,4);
			if ((i&7) == 7) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,4,3,6,5,0);


		}
		else
		{
			decrypt[i] = region[i];
		}
	}

//  dump_decrypted(machine, decrypt);

	memcpy(region, decrypt, 0x10000);
}


static DRIVER_INIT( victor21 )
{
	unsigned char xors[8] = { 0x44, 0xBB, 0x66, 0x44, 0xAA, 0x55, 0x88, 0x22 };



	int i;
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* region = memory_region(machine,"maincpu");
	for (i=0;i<0x10000;i++)
	{
		if (i<0xc000)
		{
			decrypt[i] = region[i]^xors[i&7];

			if ((i&7) == 0) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
			if ((i&7) == 1) decrypt[i] = BITSWAP8(decrypt[i],3,6,1,4,7,2,5,0);
			if ((i&7) == 2) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,4,7,6,5,0);
			if ((i&7) == 3) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 4) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 5) decrypt[i] = BITSWAP8(decrypt[i],3,6,5,0,7,2,1,4);
			if ((i&7) == 6) decrypt[i] = BITSWAP8(decrypt[i],7,6,5,4,3,2,1,0);
			if ((i&7) == 7) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,4,7,6,5,0);

		}
		else
		{
			decrypt[i] = region[i];
		}
	}

//  dump_decrypted(machine, decrypt);

	memcpy(region, decrypt, 0x10000);
}

static DRIVER_INIT( crsbingo )
{
	unsigned char xors[8] = { 0x44, 0x33, 0x33, 0x22, 0x55, 0xEE, 0xBB, 0x11 };

	int i;
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* region = memory_region(machine,"maincpu");
	for (i=0;i<0x10000;i++)
	{
		if (i<0xc000)
		{
			decrypt[i] = region[i]^xors[i&7]^0xff;

			if ((i&7) == 0) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 1) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
			if ((i&7) == 2) decrypt[i] = BITSWAP8(decrypt[i],3,2,5,0,7,6,1,4);
			if ((i&7) == 3) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,0,3,6,1,4);
			if ((i&7) == 4) decrypt[i] = BITSWAP8(decrypt[i],7,6,5,0,3,2,1,4);
			if ((i&7) == 5) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,4,3,6,5,0);
			if ((i&7) == 6) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,0,3,6,5,4);
			if ((i&7) == 7) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,0,7,6,5,4);
		}
		else
		{
			decrypt[i] = region[i];
		}
	}

//  dump_decrypted(machine, decrypt);

	memcpy(region, decrypt, 0x10000);
}


static DRIVER_INIT( sharkpy )
{
	unsigned char xors[8] = { 0x33, 0x55, 0x99, 0x55, 0x11, 0xCC, 0x00, 0x00 };
	int i;
	UINT8 *decrypt = auto_alloc_array(machine, UINT8, 0x10000);
	UINT8* region = memory_region(machine,"maincpu");
	for (i=0;i<0x10000;i++)
	{
		if (i<0xa000)
		{
			decrypt[i] = region[i]^xors[i&7]^0xff;
			if ((i&7) == 0) decrypt[i] = BITSWAP8(decrypt[i],3,2,1,0,7,6,5,4);
			if ((i&7) == 1) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 2) decrypt[i] = BITSWAP8(decrypt[i],7,2,1,4,3,6,5,0);
			if ((i&7) == 3) decrypt[i] = BITSWAP8(decrypt[i],3,6,1,0,7,2,5,4);
			if ((i&7) == 4) decrypt[i] = BITSWAP8(decrypt[i],7,2,5,4,3,6,1,0);
			if ((i&7) == 5) decrypt[i] = BITSWAP8(decrypt[i],3,2,5,4,7,6,1,0);
			if ((i&7) == 6) decrypt[i] = BITSWAP8(decrypt[i],7,6,1,4,3,2,5,0);
			if ((i&7) == 7) decrypt[i] = BITSWAP8(decrypt[i],3,6,1,4,7,2,5,0);
		}
		else
		{
			decrypt[i] = region[i];
		}
	}

//  dump_decrypted(machine, decrypt);

	memcpy(region, decrypt, 0x10000);
}


GAME( 1990, victor21, 0,        victor21, victor21, victor21, ROT0, "Subsino / Buffy", "Victor 21",                 0 )
GAME( 1991, victor5,  0,        victor5,  victor5,  victor5,  ROT0, "Subsino",         "Victor 5",                  0 ) // board sticker says Victor 5, in-game says G.E.A with no manufacturer info?
GAME( 1991, crsbingo, 0,        crsbingo, crsbingo, crsbingo, ROT0, "Subsino",         "Poker Carnival",            0 ) // alt version of Cross Bingo?
GAME( 1996, sharkpy,  0,        sharkpy,  smoto,    sharkpy,  ROT0, "Subsino",         "Shark Party (Italy, v1.3)", 0 ) // missing POST messages?
GAME( 1996, sharkpya, sharkpy,  sharkpy,  smoto,    sharkpy,  ROT0, "Subsino",         "Shark Party (Italy, v1.6)", 0 ) // missing POST messages?
GAME( 1996, smoto20,  0,        srider,   smoto,    smoto20,  ROT0, "Subsino",         "Super Rider (Italy, v2.0)", 0 )
GAME( 1996, smoto16,  smoto20,  srider,   smoto,    smoto16,  ROT0, "Subsino",         "Super Moto (Italy, v1.6)",  0 )
