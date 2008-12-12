/*************************************************************************************************

New Magic Card (c) 19?? New High Video?

driver by David Haywood & Angelo Salese

Notes:
-"New Magic Card" is almost certainly an earlier revision of the "Bra$il / Fashion" sets,
 with colour mapped bitmaps instead of RGB565 etc..For sure is "pre-2002" because in-game displays
 "Maximum credit = 10.000 Italian Lire" on title screen.

TODO:
-understand how the blitter deletes some background parts in Bra$il / Fashion and understand how
 the status registers really works;
-inputs are grossly mapped;
-add the sound chip into MAME (OkiM6376),it's an alternative version of the more common OkiM6295.
-NVRAM emulation?
-Hook-up the V-Blank bit (check if it is really a V-Blank etc.)
-I don't really know if the manufacturer is really New High Video,I've got that name from f205v.
-Why Bra$il harass the player with that "memory sub-game"? Broken logic or BTANB?

*************************************************************************************************/

#include "driver.h"
#include "fashion.lh"

static UINT16 *blit_ram;

VIDEO_START(brasil)
{

}

VIDEO_UPDATE(brasil)
{
	int x,y,count;

	count = (0/2);

	for(y=0;y<300;y++)
	{
		for(x=0;x<400;x++)
		{
			UINT32 color;
			UINT32 b;
			UINT32 g;
			UINT32 r;

			color = (blit_ram[count]) & 0xffff;

			b = (color & 0x001f) << 3;
			g = (color & 0x07e0) >> 3;
			r = (color & 0xf800) >> 8;
			if(x<video_screen_get_visible_area(screen)->max_x && y<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, x) = b | (g<<8) | (r<<16);

			count++;
		}
	}

	return 0;
}

VIDEO_UPDATE(vidpokr2)
{
	int x,y,count;

	count = (0/2);

	for(y=0;y<224;y++)
	{
		for(x=0;x<160;x++)
		{
			UINT32 color;

			color = ((blit_ram[count]) & 0x00ff)>>0;

			if((x*2)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+0) = screen->machine->pens[color];

			color = ((blit_ram[count]) & 0xff00)>>8;

			if(((x*2)+1)<video_screen_get_visible_area(screen)->max_x && ((y)+0)<video_screen_get_visible_area(screen)->max_y)
				*BITMAP_ADDR32(bitmap, y, (x*2)+1) = screen->machine->pens[color];

			count++;
		}
	}

	return 0;
}

/*Some sort of status registers,probably blitter/vblank related.*/
static UINT16 unk_latch;
static UINT16 vblank_bit;

/*Bra$il*/
static READ16_HANDLER( blit_status_r )
{
	switch(offset*2)
	{
		case 0:
		vblank_bit^=0x10;

		return 3 | vblank_bit;
		case 2: return (unk_latch & 3); //and 0x3f
	}

	return 0;
}

/*bankaddress might be incorrect.*/
static WRITE16_HANDLER( blit_status_w )
{
	static UINT32 bankaddress;
	UINT8 *ROM = memory_region(space->machine, "user1");

	switch(data & 3) //data & 7?
	{
		case 0: unk_latch = 1; break;
		case 1: unk_latch = 0; break;
		case 2: unk_latch = 2; break;
	}

	bankaddress = (data & 0x07) * 0x40000;

	memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);

//  popmessage("%04x",data);
}

/* New Magic Card */
static READ16_HANDLER( vidpokr2_blit_status_r )
{
	switch(offset*2)
	{
		case 0: return 2; //and $7
		case 2: return 2; //and $7
	}
	return 0;
}

/*bankaddress might be incorrect.*/
static WRITE16_HANDLER( vidpokr2_blit_status_w )
{
	static UINT32 bankaddress;
	UINT8 *ROM = memory_region(space->machine, "user1");

	bankaddress = (data & 0x07) * 0x40000;

	memory_set_bankptr(space->machine, 1, &ROM[bankaddress]);

//  popmessage("%04x",data);
}

static WRITE16_HANDLER( paletteram_io_w )
{
	static int pal_offs,r,g,b,internal_pal_offs;

	switch(offset*2)
	{
		case 0:
			pal_offs = 0;
			break;
		case 2:
			internal_pal_offs = 0;
			break;
		case 4:
			switch(internal_pal_offs)
			{
				case 0:
					r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 1:
					g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					internal_pal_offs++;
					break;
				case 2:
					b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
					palette_set_color(space->machine, pal_offs, MAKE_RGB(r, g, b));
					internal_pal_offs = 0;
					pal_offs++;
					break;
			}

			break;
	}
}

/*There's a reset button located there too.*/
static READ16_HANDLER( vidpokr2_vblank_r )
{
	return vblank_bit; //0x80
}

static WRITE16_HANDLER( vidpokr2_vblank_w )
{
	vblank_bit = data;
}


static UINT16 t1,t3;

static READ16_HANDLER( read1_r )
{
//  return mame_rand(space->machine);
	return input_port_read(space->machine, "IN0");
}

static READ16_HANDLER( read2_r )
{
//  return mame_rand(space->machine);
	return input_port_read(space->machine, "IN1");
}

static READ16_HANDLER( read3_r )
{
//  return mame_rand(space->machine);
	return input_port_read(space->machine, "IN2");
}

static WRITE16_HANDLER( write1_w )
{
	t1 = data;
/*
    - Lbits -
    7654 3210
    =========
    ---- ---x  Hold1 lamp.
    ---- --x-  Hold2 lamp.
    ---- -x--  Hold3 lamp.
    ---- x---  Hold4 lamp.
    ---x ----  Hold5 lamp.
    --x- ----  Start lamp.
*/
	output_set_lamp_value(1, (data & 1));			/* Lamp 1 - HOLD 1 */
	output_set_lamp_value(2, (data >> 1) & 1);		/* Lamp 2 - HOLD 2 */
	output_set_lamp_value(3, (data >> 2) & 1);		/* Lamp 3 - HOLD 3 */
	output_set_lamp_value(4, (data >> 3) & 1);		/* Lamp 4 - HOLD 4 */
	output_set_lamp_value(5, (data >> 4) & 1);		/* Lamp 5 - HOLD 5 */
	output_set_lamp_value(6, (data >> 5) & 1);		/* Lamp 6 - START  */

//	popmessage("%04x %04x",t1,t3);
}

static WRITE16_HANDLER( write2_w )
{
	static int i;

//	popmessage("%04x",data);

	for(i=0;i<4;i++)
	{
		coin_counter_w(i,data & 0x20);
		coin_lockout_w(i,~data & 0x08);
	}
}

/*Different coin-lockout in this one*/
static WRITE16_HANDLER( fashion_write2_w )
{
	static int i;

//	popmessage("%04x",data);

	for(i=0;i<4;i++)
	{
		coin_counter_w(i,data & 0x20);
		coin_lockout_w(i,~data & 0x01);
	}
}

static WRITE16_HANDLER( write3_w )
{
	t3 = data;
// 	popmessage("%04x %04x",t1,t3);
}

static ADDRESS_MAP_START( brasil_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM /*irq vector area + work ram*/
	AM_RANGE(0x40000, 0x7ffff) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x80000, 0xbffff) AM_ROMBANK(1)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( brasil_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0030, 0x0033) AM_READ( blit_status_r )
	AM_RANGE(0x0030, 0x0031) AM_WRITE( blit_status_w )
	AM_RANGE(0x0000, 0x0001) AM_WRITE( write1_w ) // lamps
	AM_RANGE(0x0002, 0x0003) AM_WRITE( write2_w ) // coin counter & coin lockout
 	AM_RANGE(0x0006, 0x0007) AM_WRITE( write3_w ) // sound chip routes here
 	AM_RANGE(0x0008, 0x0009) AM_READ( read1_r )
	AM_RANGE(0x000a, 0x000b) AM_READ( read2_r )
	AM_RANGE(0x000e, 0x000f) AM_READ( read3_r )
//  AM_RANGE(0x000e, 0x000f) AM_WRITE
//  AM_RANGE(0xffa2, 0xffa3) AM_WRITE
ADDRESS_MAP_END

static ADDRESS_MAP_START( vidpokr2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000, 0x0ffff) AM_RAM /*irq vector area + work ram*/
	AM_RANGE(0x40000, 0x7ffff) AM_RAM AM_BASE(&blit_ram) /*blitter ram*/
	AM_RANGE(0x80000, 0xbffff) AM_ROMBANK(1)
	AM_RANGE(0xc0000, 0xfffff) AM_ROM AM_REGION("boot_prg",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( vidpokr2_io, ADDRESS_SPACE_IO, 16 )
	AM_RANGE(0x0030, 0x0033) AM_READ( vidpokr2_blit_status_r )
	AM_RANGE(0x0030, 0x0031) AM_WRITE( vidpokr2_blit_status_w )
	AM_RANGE(0x0000, 0x0001) AM_WRITE( write1_w ) // lamps
	AM_RANGE(0x0002, 0x0003) AM_WRITE( write2_w ) // coin counter & coin lockout
	AM_RANGE(0x0004, 0x0005) AM_WRITE( vidpokr2_vblank_w )
 	AM_RANGE(0x0006, 0x0007) AM_WRITE( write3_w ) // sound chip routes here
 	AM_RANGE(0x0008, 0x0009) AM_READ( read1_r )
	AM_RANGE(0x000a, 0x000b) AM_READ( read2_r )
	AM_RANGE(0x000c, 0x000d) AM_READ( vidpokr2_vblank_r )
	AM_RANGE(0x000e, 0x000f) AM_READ( read3_r )
	AM_RANGE(0x0010, 0x0015) AM_WRITE( paletteram_io_w )
//  AM_RANGE(0x000e, 0x000f) AM_WRITE
//  AM_RANGE(0xffa2, 0xffa3) AM_WRITE
ADDRESS_MAP_END

static INPUT_PORTS_START( brasil )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Take Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Risk Button") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) // note
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) ) //ticket
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) ) //hopper
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*Slightly different inputs*/
static INPUT_PORTS_START( fashion )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Take Button") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Stock 2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 3 / Risk Button") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Hold 5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hold 1") PORT_CODE(KEYCODE_Z)
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("Stock 3 / Note") PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Stock 1 / Ticket") PORT_CODE(KEYCODE_Q)
	PORT_SERVICE( 0x0020, IP_ACTIVE_LOW )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Stock 5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Stock 4") PORT_CODE(KEYCODE_R)
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN2" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
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
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INTERRUPT_GEN( vblank_irq )
{
	cpu_set_input_line_and_vector(device,0,HOLD_LINE,0x08/4);
}

static MACHINE_DRIVER_START( brasil )
	MDRV_CPU_ADD("main", I80186, 20000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(brasil_map,0)
	MDRV_CPU_IO_MAP(brasil_io,0)
	MDRV_CPU_VBLANK_INT("main", vblank_irq)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 400-1, 0, 300-1)

	MDRV_PALETTE_LENGTH(0x200)

	MDRV_VIDEO_START(brasil)
	MDRV_VIDEO_UPDATE(brasil)

	//OkiM6376
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( vidpokr2 )
	MDRV_CPU_ADD("main", V30, 8000000 )	// ?
	MDRV_CPU_PROGRAM_MAP(vidpokr2_map,0)
	MDRV_CPU_IO_MAP(vidpokr2_io,0)
	MDRV_CPU_VBLANK_INT("main", vblank_irq)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_SIZE(400, 300)
	MDRV_SCREEN_VISIBLE_AREA(0, 320-1, 0, 200-1)

	MDRV_PALETTE_LENGTH(0x100)

	MDRV_VIDEO_START(brasil)
	MDRV_VIDEO_UPDATE(vidpokr2)

	//OkiM6376
MACHINE_DRIVER_END

/*
CPU

1x NEC 9145N5-V30-D70116C-8 (main)
1x OKI M6376 (sound)
1x ispLSI2032-80LJ-H013J05 (main)
1x ispLSI1032E-70LJ-E013S09 (main)
1x ADV476KP35-9948-F112720.1 (GFX)
1x oscillator 16.000MHz

ROMs
1x M27C2001 (ic31)
2x M27C4001 (ic32,ic33)

Note

1x 28x2 edge connector (not JAMMA)
1x 8 legs connector
1x 3 legs jumper
1x pushbutton
1x battery
1x trimmer (volume)

PCB markings: "V150500 CE type 001/v0"
PCB n. E178247

*/

ROM_START( newmcard )
	ROM_REGION( 0x100000, "user1", 0 ) /* V30 Code */
	ROM_LOAD16_BYTE( "mc32.ic4", 0x00000, 0x80000, CRC(d9817f48) SHA1(c523a8248b487081ea2e0e326dcc660b051c23c1) )
	ROM_LOAD16_BYTE( "mc33.ic5", 0x00001, 0x80000, CRC(83a855ab) SHA1(7f9384c875b951d17caa91f8a7365edaf7f9afe1) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x0c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "samples", 0 ) /* M6376 Samples */
	ROM_LOAD( "mc33.ic5", 0x00000, 0x80000, CRC(83a855ab) SHA1(7f9384c875b951d17caa91f8a7365edaf7f9afe1) )
ROM_END

/*
CPUs
N80C186XL25 (main)(u1)
1x ispLSI2032-80LJ (u13)(not dumped)
1x ispLSI1032E-70LJ (u18)(not dumped)
1x M6376 (sound)(u17)
1x oscillator 40.000MHz

ROMs
1x MX27C4000 (u16)
2x M27C801 (u7,u8)

Note

1x 28x2 edge connector (cn1)
1x 5 legs connector (cn2)
1x 8 legs connector (cn3)
1x trimmer (volume)
1x pushbutton (k1)
1x battery (b1)


cpu is 80186 based (with extras), see
http://media.digikey.com/pdf/Data%20Sheets/Intel%20PDFs/80C186XL,%2080C188XL.pdf

*/

ROM_START( brasil )
	ROM_REGION( 0x200000, "user1", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "record_brasil_hrc7_vers.3.u7", 0x000000, 0x100000, CRC(627e0d58) SHA1(6ff8ba7b21e1ea5c88de3f02a057906c9a7cd808) )
	ROM_LOAD16_BYTE( "record_brasil_hrc8_vers.3.u8", 0x000001, 0x100000, CRC(47f7ba2a) SHA1(0add7bbf771fd0bf205a05e910cb388cf052b09f) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x1c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "samples", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound_brasil_hbr_vers.1.u16", 0x00000, 0x80000, CRC(d71a5566) SHA1(2f7aefc06e39ce211e31b15aadf6338b679e7a31) )
ROM_END

ROM_START( fashion )
	ROM_REGION( 0x200000, "user1", 0 ) /* N80C186XL25 Code */
	ROM_LOAD16_BYTE( "fashion1-hfs7v2.14.high-video8m.u7", 0x000000, 0x100000, CRC(20411b89) SHA1(3ed6336978e5046eeef26115614cb74e3ffe134a) )
	ROM_LOAD16_BYTE( "fashion1-hfs8v2.14.high-video8m.u8", 0x000001, 0x100000, CRC(521f34f3) SHA1(91edc90fcd895a096955ac031a42da04510df1e6) )

	ROM_REGION( 0x040000, "boot_prg", 0 ) /*copy for program code*/
	ROM_COPY( "user1", 0x1c0000, 0x000000, 0x40000 )

	ROM_REGION( 0x080000, "samples", 0 ) /* M6376 Samples */
	ROM_LOAD( "sound-fashion-v-1-memory4m.u16", 0x00000, 0x80000, CRC(2927c799) SHA1(f11cad096a23fee10bfdff5bf944c96e30f4a8b8) )
ROM_END

static DRIVER_INIT( fashion )
{
	memory_install_write16_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_IO), 0x0002, 0x0003, 0, 0, fashion_write2_w );
}

GAMEL( 19??, newmcard,  0,      vidpokr2,    brasil,   0,       ROT0,  "New High Video?", "New Magic Card",         GAME_NO_SOUND,                           layout_fashion )
GAMEL( 2000, brasil,    0,      brasil,      brasil,   0,       ROT0,  "New High Video?", "Bra$il (Version 3)",     GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND, layout_fashion )
GAMEL( 2000, fashion,   brasil, brasil,      fashion,  fashion, ROT0,  "New High Video?", "Fashion (Version 2.14)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND, layout_fashion )
