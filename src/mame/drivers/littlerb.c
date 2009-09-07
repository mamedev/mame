/* Little Robin */

/* driver by
Pierpaolo Prazzoli
David Haywood
*/

/*

strange vdp/memory access chip..
at the moment gfx / palettes etc. get drawn then erased

maybe it needs read commands working so it knows how many sprites are in
the list?

maybe the vdp 'commands' can be used to store and get back
write addresses?

Dip sw.1
--------
             | Coin 1 | Coin 2  |
              1  2  3   4  5  6   7  8   Coin  Play
---------------------------------------------------
 Coins        -  -  -   -  -  -           1     4
              +  -  -   +  -  -           1     3
              -  +  -   -  +  -           1     2
              +  +  -   +  +  -           1     1
              -  -  +   -  -  +           2     1
              +  -  +   +  -  +           3     1
              -  +  +   -  +  +           4     1
              +  +  +   +  +  +           5     1
 Player                           -  -    2
                                  +  -    3
                                  -  +    4
                                  +  +    5


Dip sw.2
--------          1  2  3  4  5  6  7  8
-----------------------------------------------------------
 Demo Sound       -                        Yes
                  +                        No
 Mode                -                     Test Mode
                     +                     Game Mode
 Difficulty             -  -  -            0 (Easy)
                        +  -  -            1
                        -  +  -            2 (Normal)
                        +  +  -            3
                        -  -  +            4
                        +  -  +            5
                        -  +  +            6
                        +  +  +            7 (Hardest)
 Bonus                           -  -      Every 150000
                                 +  -      Every 200000
                                 -  +      Every 300000
                                 +  +      No Bonus


*/

#include "driver.h"
#include "cpu/m68000/m68000.h"

static UINT16 littlerb_vdp_address_low;
static UINT16 littlerb_vdp_address_high;
static UINT16 littlerb_vdp_writemode;
static UINT32 littlerb_write_address;

static void littlerb_recalc_regs(void)
{
	littlerb_vdp_address_low = littlerb_write_address&0xffff;
	littlerb_vdp_address_high = (littlerb_write_address>>16)&0xffff;
}

static UINT16* littlerb_region1;
static UINT16* littlerb_region2;
static UINT16* littlerb_region3;
static UINT16* littlerb_region4;


static void littlerb_data_write(running_machine *machine, UINT16 data)
{
	UINT32 addr = littlerb_write_address>>4; // is this right? should we shift?

	if (addr>=0x00000000 && addr<=0x0003ffff)
	{
		addr &= 0x0003ffff;

	//  littlerb_region1[addr] = data;
		littlerb_region4[addr] = data; // it writes the 'slime' gfx you cover the enemy in with here
		                               // it might be a mirror, or just another area of ram it can
		                               // access but isn't because of the sprite drawing..
	}
	else if (addr>=0x00400000 && addr<=0x004007ff)
	{
		int x;
		addr &= 0x7ff;
		littlerb_region2[addr] = data;

		for (x=0;x<0x100;x+=3)
		{
			int r,g,b;

			b = littlerb_region2[x];
			r = littlerb_region2[x+1];
			g = littlerb_region2[x+2];

			palette_set_color(machine,x/3,MAKE_RGB(r,g,b));
		}


	}
	else if (addr>=0x0c000000 && addr<=0x0c00001f)
	{
		addr &= 0x1f;
		littlerb_region3[addr] = data;
	}
	else if (addr>=0x0ffc0000 && addr<=0x0fffffff)
	{
		addr &=0x3ffff;
		littlerb_region4[addr] = data;

	}
	else
	{
		mame_printf_debug("write data %04x to %08x\n",data,addr);
	}

	littlerb_write_address+=0x10;
	littlerb_recalc_regs();

}




static void littlerb_recalc_address(void)
{
	littlerb_write_address = littlerb_vdp_address_low | littlerb_vdp_address_high<<16;
}

static READ16_HANDLER( littlerb_vdp_r )
{
	logerror("%06x littlerb_vdp_r offs %04x mask %04x\n", cpu_get_pc(space->cpu), offset, mem_mask);

	switch (offset)
	{
		case 0:
		return littlerb_vdp_address_low;

		case 1:
		return littlerb_vdp_address_high;

		case 2:
		return 0; // data read? -- startup check expects 0 for something..

		case 3:
		return littlerb_vdp_writemode;
	}

	return -1;
}

int type2_writes = 0;
UINT32 lasttype2pc = 0;
static WRITE16_HANDLER( littlerb_vdp_w )
{

	if (offset!=2)
	{
		if (type2_writes)
		{
			if (type2_writes>2)
			{
				logerror("******************************* BIG WRITE OCCURRED BEFORE THIS!!! ****************************\n");
			}

			logerror("~%06x previously wrote %08x data bytes\n", lasttype2pc, type2_writes*2);
			type2_writes = 0;
		}

		logerror("%06x littlerb_vdp_w offs %04x data %04x mask %04x\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
	}
	else
	{
		if (mem_mask==0xffff)
		{
			if (type2_writes==0)
			{
				logerror("data write started %06x %04x data %04x mask %04x\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
			}

			type2_writes++;
			lasttype2pc = cpu_get_pc(space->cpu);
		}
		else
		{
			logerror("xxx %06x littlerb_vdp_w offs %04x data %04x mask %04x\n", cpu_get_pc(space->cpu), offset, data, mem_mask);
		}
	}


	switch (offset)
	{
		case 0:
		littlerb_vdp_address_low = data;
		littlerb_recalc_address();
		break;

		case 1:
		littlerb_vdp_address_high = data;
		littlerb_recalc_address();
		break;


		case 2:
		littlerb_data_write(space->machine, data);
		break;

		case 3:
		littlerb_vdp_writemode = data;
		break;

	}

}

static ADDRESS_MAP_START( littlerb_main, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000008, 0x000017) AM_WRITENOP
	AM_RANGE(0x000020, 0x00002f) AM_WRITENOP
	AM_RANGE(0x000070, 0x000073) AM_WRITENOP
	AM_RANGE(0x060004, 0x060007) AM_WRITENOP
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x203fff) AM_RAM // main ram?
	AM_RANGE(0x7c0000, 0x7c0001) AM_READ_PORT("DSW")
	AM_RANGE(0x7e0000, 0x7e0001) AM_READ_PORT("P1")
	AM_RANGE(0x7e0002, 0x7e0003) AM_READ_PORT("P2")
	AM_RANGE(0x700000, 0x700007) AM_READ(littlerb_vdp_r) AM_WRITE(littlerb_vdp_w)
	AM_RANGE(0x780000, 0x780001) AM_WRITENOP

	/* below are fake.. just to see the data */
	AM_RANGE(0xc00000, 0xc7ffff) AM_RAM AM_BASE(&littlerb_region1)
	AM_RANGE(0xd00000, 0xd00fff) AM_RAM AM_BASE(&littlerb_region2)
	AM_RANGE(0xe00000, 0xe0003f) AM_RAM AM_BASE(&littlerb_region3)
	AM_RANGE(0xf00000, 0xf7ffff) AM_RAM AM_BASE(&littlerb_region4)




ADDRESS_MAP_END


static INPUT_PORTS_START( littlerb )
	PORT_START("DSW")	/* 16bit */
	PORT_DIPNAME( 0x0003, 0x0001, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0003, "2" )
	PORT_DIPSETTING(      0x0001, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x001c, 0x0004, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x00e0, 0x0020, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_4C ) )
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
	PORT_DIPNAME( 0x4000, 0x4000, "GAME/TEST??" ) // changes what gets uploaded
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P1")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x1000, 0x1000, "???"  )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")	/* 16bit */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

#if 0
PALETTE_INIT( littlerb )
{
	int i;
	for(i = 0; i < 256; i++)
		palette_set_color(machine,i,MAKE_RGB(i,i,i));
}
#endif

static void draw_sprite(bitmap_t *bitmap, int xsize,int ysize, int offset, int xpos, int ypos )
{
	UINT16* spritegfx = littlerb_region4;
	int x,y;

	for (y=0;y<ysize;y++)
	{
		for (x=0;x<xsize;x++)
		{
			int drawxpos, drawypos;
			UINT8 pix1 = spritegfx[offset]&0x0f;
			UINT8 pix2 = (spritegfx[offset]>>8)&0x0f;
			drawxpos = xpos+x*2;
			drawypos = ypos+y;

			if ((drawxpos < 320) && (drawypos < 256) && (drawxpos >= 0) && (drawypos >=0))
			{
				if(pix1&0xf) *BITMAP_ADDR16(bitmap, drawypos, drawxpos) = pix1;
			}
			drawxpos++;
			if ((drawxpos < 320) && (drawypos < 256) && (drawxpos >= 0) && (drawypos >=0))
			{
				if(pix2&0xf) *BITMAP_ADDR16(bitmap, drawypos, drawxpos) = pix2;
			}

			offset++;

			offset&=0x3ffff;
		}
	}
}

static VIDEO_UPDATE(littlerb)
{
	int x,y,offs, code;
	int xsize,ysize;
	UINT16* spriteregion = &littlerb_region4[0x400];
	bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));

	/* the spriteram format is something like this .. */
	for (offs=0x26/2;offs<0xc00;offs+=6) // start at 00x26?
	{
		x = spriteregion[offs+2] & 0x01ff;
		ysize = (spriteregion[offs+5] & 0x007f);
		y = (spriteregion[offs+3] & 0x01ff); // 1?
		xsize = (spriteregion[offs+4] & 0x007f)/2;

		// the code seems to be the same address as the blitter writes
		// e.g  ffc010000
		code =  (spriteregion[offs+0] & 0xfff0)>>4;
		code |=  (spriteregion[offs+1] & 0x003f)<<12;
		draw_sprite(bitmap,xsize,ysize,code,x-8,y-16);
	}

	return 0;
}

static INTERRUPT_GEN( littlerb )
{
	logerror("IRQ\n");
	cpu_set_input_line(device, 4, HOLD_LINE);
}

static MACHINE_DRIVER_START( littlerb )
	MDRV_CPU_ADD("maincpu", M68000, 12000000)
	MDRV_CPU_PROGRAM_MAP(littlerb_main)
	MDRV_CPU_VBLANK_INT("screen", littlerb)


	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(512, 256)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 320-1, 0*8, 256-1)

	MDRV_PALETTE_LENGTH(256)

//  MDRV_PALETTE_INIT(littlerb)
	MDRV_VIDEO_UPDATE(littlerb)
MACHINE_DRIVER_END

ROM_START( littlerb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "roma.u53", 0x00001, 0x80000, CRC(172fbc13) SHA1(cd165ca0d0546e2634cf182dc98004cbfb02cf9f) )
	ROM_LOAD16_BYTE( "romb.u29", 0x00000, 0x80000, CRC(b2fb1d61) SHA1(9a9d7176c241928d07af651e5f7f21d4f019701d) )

		ROM_REGION( 0x80000, "samples", 0 ) /* sound samples */
	ROM_LOAD16_BYTE( "romc.u26", 0x00001, 0x40000, CRC(f193c5b6) SHA1(95548a40e2b5064c558b36cabbf507d23678b1b2) )
	ROM_LOAD16_BYTE( "romd.u32", 0x00000, 0x40000, CRC(d6b81583) SHA1(b7a63d18a41ccac4d3db9211de0b0cdbc914317a) )
ROM_END


GAME( 1993, littlerb, 0, littlerb, littlerb, 0, ROT0, "TCH", "Little Robin", GAME_NOT_WORKING|GAME_NO_SOUND )
