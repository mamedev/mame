/*
BMC Bowling (c) 1994.05 BMC, Ltd

- TS 2004.10.22 - analog[at]op.pl

  Game is almost playable, especially with NVRAM_HACK (undefine it
  to get real nvram  access, but sometimes it's impossible to get back to
  title screen ).

 Controls:

 press START(1) OR BUTTON1 to start game , also START(1) or BUTTON1 to bowl / start
        ( 5 to insert coin(s) , B to bet , D to pay out (?)  etc...)

 press ANALIZER(0) durning boot to enter test menu, then :
            STOP1+STOP2 - sound test menu
        BIG(G) - cycle options ,
        DOUBLE(H) - play
            STOP1(X),STOP2(C) - change
            TAKE(A) - color test
            START(1) - exit
        BET(B)+START(1) - other tests
        START(1) - next test

 press START(1)+HP(S) durning boot to see stats

 press CONFIRM(N) durning boot, to enter    settings
        BET(B) - change page
        STOP1(X)/STOP3(V) - modify
        START(1)/SMALL(F) - move
        KEY DOWN(D) - default ?

TODO:

 - scroll (writes to $91800 and VIA port A - not used in game (only in test mode))
 - music - writes  ($20-$30 bytes) to $93000-$93003 range
 - VIA 6522(interrupt gen , ports)
 - Crt
 - interrupts
 - missing gfx elements

---

Chips:
MC68000P10
Goldstar GM68B45S (same as Hitachi HD6845 CTR Controller)*
Winbond WF19054 (same as AY3-8910)
MK28 (appears to be a AD-65, AKA OKI6295) next to rom 10
Synertek SY6522 VIA
9325-AG (Elliptical Filter)
KDA0476BCN-66 (RAMDAC)
part # scratched 64 pin PLCC next to 7EX & 8 EX

Ram:
Goldstar GM76C28A (2Kx8 SRAM) x3
HM62256LP-12 x6

OSC:
GREAT1 21.47727
13.3M
3.579545

DIPS:
Place for 4 8 switch dips
dips 1 & 3 are all connected via resitors
dips 2 & 4 are standard 8 switch dips

EEPROM       Label         Use
----------------------------------------
ST M27C1001  bmc_8ex.bin - 68K code 0x00
ST M27C1001  bmc_7ex.bin - 68K code 0x01
ST M27C512   bmc_3.bin\
ST M27C512   bmc_4.bin | Graphics
ST M27C512   bmc_5.bin |
ST M27C512   bmc_6.bin/
HM27C101AG   bmc_10.bin - Sound samples

BrianT

* There is a MESS driver for this chip (gm68b45s CTR controller):
http://cvs.mess.org:6502/cgi-bin/viewcvs.cgi/mess/video/m6845.c?rev=1.10

Top board:
          --- Edge Connection ---
GS9403      7EX    8EX   Part #      Misc HD74LS374p
                         Scratched   HM62256   HM62256
                         PLC
  OKI                                ST T74L6245BI
                  PEEl 18CV8P
          10



Main board:
          --- Edge Connection ---              JAMMA Connection
  68K     GM76C28A         3   5   GMC76C28A
          GM76C28A         4   6   GM68B45S   BATTERY

21.47727MHz                                 WF19054
                                                      DIP1
13.3Mhz                                               DIP2
  SY6522            HM62256 x4  3.579545MHz 9325-AG   DIP3
                                   KDA0476BCN-66      DIP4
*/

#include "driver.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"

static UINT16 *bmcbowl_vid1;
static UINT16 *bmcbowl_vid2;

static UINT8 *stats_ram;
static size_t	stats_ram_size;
static int clr_offset=0;
static int bmc_input=0;

#define NVRAM_HACK

static VIDEO_START( bmcbowl )
{
}

static VIDEO_UPDATE( bmcbowl )
{
/*
      280x230,4 bitmap layers, 8bpp,
        missing scroll and priorities   (maybe fixed ones)
*/

	int x,y,z,pixdat;
	fillbitmap(bitmap,get_black_pen(machine),cliprect);

	z=0;
	for (y=0;y<230;y++)
	{
		for (x=0;x<280;x+=2)
		{
			pixdat = bmcbowl_vid2[0x8000+z];

			if(pixdat&0xff)
				*BITMAP_ADDR16(bitmap, y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				*BITMAP_ADDR16(bitmap, y, x) = (pixdat>>8);

			pixdat = bmcbowl_vid2[z];

			if(pixdat&0xff)
				*BITMAP_ADDR16(bitmap, y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				*BITMAP_ADDR16(bitmap, y, x) = (pixdat>>8);

			pixdat = bmcbowl_vid1[0x8000+z];

			if(pixdat&0xff)
				*BITMAP_ADDR16(bitmap, y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				*BITMAP_ADDR16(bitmap, y, x) = (pixdat>>8);

			pixdat = bmcbowl_vid1[z];

			if(pixdat&0xff)
				*BITMAP_ADDR16(bitmap, y, x+1) = (pixdat&0xff);
			if(pixdat>>8)
				*BITMAP_ADDR16(bitmap, y, x) = (pixdat>>8);

			z++;
		}
	}
	return 0;
}

static READ16_HANDLER( bmc_random_read )
{
	return mame_rand(Machine);
}

static READ16_HANDLER( bmc_protection_r )
{
	switch(activecpu_get_previouspc())
	{
		case 0xca68:
			switch(activecpu_get_reg(M68K_D2))
			{
				case 0: 		 return 0x37<<8;
				case 0x1013: return 0;
				default:		 return 0x46<<8;
			}
			break;
	}
	logerror("Protection read @ %X\n",activecpu_get_previouspc());
	return mame_rand(Machine);
}

static WRITE16_HANDLER( bmc_RAMDAC_offset_w )
{
		clr_offset=data*3;
}

static WRITE16_HANDLER( bmc_RAMDAC_color_w )
{
		colorram[clr_offset]=data;
		palette_set_color_rgb(Machine,clr_offset/3,pal6bit(colorram[(clr_offset/3)*3]),pal6bit(colorram[(clr_offset/3)*3+1]),pal6bit(colorram[(clr_offset/3)*3+2]));
		clr_offset=(clr_offset+1)%768;
}

static WRITE16_HANDLER( scroll_w )
{
	//TODO - scroll
}


static READ16_HANDLER(via_r)
{
	return via_0_r(offset);
}

static WRITE16_HANDLER(via_w)
{
	via_0_w(offset,data);
}

static READ8_HANDLER(via_b_in)
{
	return input_port_3_r(0);
}


static WRITE8_HANDLER(via_a_out)
{
	// related to video hw ? BG scroll ?
}

static WRITE8_HANDLER(via_b_out)
{
	//used
}

static WRITE8_HANDLER(via_ca2_out)
{
	//used
}


static void via_irq(int state)
{
	//used
}


// 'working' NVRAM

static int bmc_nv1[]=
{
 0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x00,0x00,0x55,0x55,0x13,0x88,0x46,0xDD,0x0F,0xA0,
 0x5A,0xF5,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x6E,0x55,
 0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,
 0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x6E,0x55,0x55,0x55,0x3B,0x00,0x00,0x00,0x06,0x55,0x55,0x55,0x53,0x00,
 0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,
 0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0x00,0x00,0x00,0x00,0x55,0x55,0x55,0x55,0xFF,0x00,0x0A,0x00,0x0A,
 0x00,0x32,0x00,0x02,0x28,0x32,0x5C,0x0A,0x03,0x03,0xD6,0x66,0xFF,0xFF,0xFF,0xFF,0x5D,0xED,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x25,0xD5,0x25,0x1C,0x00,0x00,0x00,0x00,
 0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,
 0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x96,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0xDC,0x00,0xFF,0xFF,0xFF,0xFF,-1
};

static int bmc_nv2[]=
{
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x03,0x00,0x09,0x00,0x00,0x2B,0xF1,
 0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,-1
};

static int bmc_nv3[]=
{
 0xFA,0xFF,0x01,0x02,0x04,0x0A,0x1E,0xC8,0x02,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,-1
};


static void init_stats(int *table , int address)
{
	while(*table>=0)
	{
		stats_ram[address]=*table;
		address+=2;
		table++;
	}
}

static NVRAM_HANDLER( bmcbowl )
{
	int i;

	if (read_or_write)
		mame_fwrite(file, stats_ram, stats_ram_size);
	else

#ifdef NVRAM_HACK
	for (i = 0; i < stats_ram_size; i++)
		{
			stats_ram[i] = 0xff;
			init_stats(bmc_nv1,0);
			init_stats(bmc_nv2,0x3b0);
			init_stats(bmc_nv3,0xfe2);
		}
#else
	if (file)
		mame_fread(file, stats_ram, stats_ram_size);
	else

		for (i = 0; i < stats_ram_size; i++)
			stats_ram[i] = 0xff;
#endif

}

static ADDRESS_MAP_START( bmcbowl_mem, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM

	AM_RANGE(0x090000, 0x090001) AM_WRITE(bmc_RAMDAC_offset_w)
	AM_RANGE(0x090002, 0x090003) AM_WRITE(bmc_RAMDAC_color_w)
	AM_RANGE(0x090004, 0x090005) AM_WRITE(MWA16_NOP)//RAMDAC

	AM_RANGE(0x090800, 0x090803) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x091000, 0x091001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x091800, 0x091801) AM_WRITE(scroll_w)

	AM_RANGE(0x092000, 0x09201f) AM_READWRITE(via_r,via_w)

	AM_RANGE(0x093000, 0x093003) AM_WRITE(MWA16_NOP)  // related to music
	AM_RANGE(0x092800, 0x092801) AM_WRITE(AY8910_write_port_0_msb_w		)
	AM_RANGE(0x092802, 0x092803) AM_READ(AY8910_read_port_0_msb_r) AM_WRITE(AY8910_control_port_0_msb_w	)
	AM_RANGE(0x093802, 0x093803) AM_READ(input_port_0_word_r)
	AM_RANGE(0x095000, 0x095fff) AM_RAM AM_BASE((UINT16 **)&stats_ram) AM_SIZE(&stats_ram_size) /* 8 bit */
	AM_RANGE(0x097000, 0x097001) AM_READ(MRA16_NOP)
	AM_RANGE(0x140000, 0x1bffff) AM_ROM
	AM_RANGE(0x1c0000, 0x1effff) AM_RAM AM_BASE(&bmcbowl_vid1)
	AM_RANGE(0x1f0000, 0x1fffff) AM_RAM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_BASE(&bmcbowl_vid2)

	AM_RANGE(0x28c000, 0x28c001) AM_READWRITE(OKIM6295_status_0_msb_r,OKIM6295_data_0_msb_w)

	/* protection device*/
	AM_RANGE(0x30c000, 0x30c001) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x30c040, 0x30c041) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x30c080, 0x30c081) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x30c0c0, 0x30c0c1) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x30c100, 0x30c101) AM_READ(bmc_protection_r)
	AM_RANGE(0x30c140, 0x30c141) AM_WRITE(MWA16_NOP)
	AM_RANGE(0x30ca00, 0x30ca01) AM_READ(bmc_random_read) AM_WRITE(MWA16_NOP)
ADDRESS_MAP_END


static INPUT_PORTS_START( bmcbowl )
	PORT_START	/* DSW 1 */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SERVICE1 )	PORT_NAME("Note")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE2 )	PORT_NAME("Analizer")

	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pay") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 2") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Bet") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Confirm") PORT_CODE(KEYCODE_N)


	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start")
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Take") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("HP") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Key Down") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Small") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Big") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Double") PORT_CODE(KEYCODE_H)

	PORT_START	/* DSW 2 */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x20, "1 COIN 10 CREDITS")
	PORT_DIPSETTING(    0x00, "2 COINS 10 CREDITS")

	PORT_DIPNAME( 0x01, 0x00, "DSW2 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW2 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x00, "DSW2 4" )
	PORT_DIPSETTING(    0x040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START	/* DSW 4 */
	PORT_DIPNAME( 0x01, 0x00, "DSW4 8" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW4 7" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW4 6" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW4 5" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW4 4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW4 3" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW4 2" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW4 1" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1)
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_VBLANK )

INPUT_PORTS_END

static READ8_HANDLER(dips1_r)
{
	switch(bmc_input)
	{
			case 0x00:	return  input_port_1_r(0);
			case 0x40:	return  input_port_2_r(0);
	}
	logerror("unknown input - %X (PC=%X)\n",bmc_input,activecpu_get_previouspc());
	return 0xff;
}


static WRITE8_HANDLER(input_mux_w)
{
	bmc_input=data;
}


static struct AY8910interface ay8910_interface =
{
	dips1_r,
	0,
	0,
	input_mux_w
};


static struct via6522_interface via_interface =
{
	/*inputs : A/B         */ 0, via_b_in,
	/*inputs : CA/B1,CA/B2 */ 0, 0, 0, 0,
	/*outputs: A/B         */ via_a_out, via_b_out,
	/*outputs: CA/B1,CA/B2 */ 0, 0, via_ca2_out, 0,
	/*irq                  */ via_irq
};

static MACHINE_RESET( bmcbowl )
{
	via_reset();
}

static INTERRUPT_GEN( bmc_interrupt )
{
	if (cpu_getiloops())
		cpunum_set_input_line(0, 4, HOLD_LINE);
	else
		cpunum_set_input_line(0, 2, HOLD_LINE);
}

static MACHINE_DRIVER_START( bmcbowl )
	MDRV_CPU_ADD_TAG("main", M68000, 21477270/2 )
	MDRV_CPU_PROGRAM_MAP(bmcbowl_mem,0)
	MDRV_CPU_VBLANK_INT(bmc_interrupt,2)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(DEFAULT_REAL_60HZ_VBLANK_DURATION)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_TYPE_RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(35*8, 30*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 35*8-1, 0*8, 29*8-1)
	MDRV_PALETTE_LENGTH(256)

	MDRV_VIDEO_START(bmcbowl)
	MDRV_VIDEO_UPDATE(bmcbowl)

	MDRV_NVRAM_HANDLER(bmcbowl)
	MDRV_MACHINE_RESET(bmcbowl)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(AY8910, 3579545/2)
	MDRV_SOUND_CONFIG(ay8910_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)

	MDRV_SOUND_ADD(OKIM6295, 1122000)
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7high) // clock frequency & pin 7 not verified
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
MACHINE_DRIVER_END

ROM_START( bmcbowl )
	ROM_REGION( 0x200000, REGION_CPU1, 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "bmc_8ex.bin", 0x000000, 0x10000, CRC(8b1aa5db) SHA1(879df950bedf2c163ba89d983ca4a0691b01c46e) )
	ROM_LOAD16_BYTE( "bmc_7ex.bin", 0x000001, 0x10000, CRC(7726d47a) SHA1(8438c3345847c2913c640a29145ec8502f6b01e7) )

	ROM_LOAD16_BYTE( "bmc_4.bin", 0x140000, 0x10000, CRC(f43880d6) SHA1(9e73a29baa84d417ff88026896d852567a38e714) )
	ROM_RELOAD(0x160000,0x10000)
	ROM_LOAD16_BYTE( "bmc_3.bin", 0x140001, 0x10000, CRC(d1af9410) SHA1(e66b3ddd9d9e3c567fdb140c4c8972c766f2b975) )
	ROM_RELOAD(0x160001,0x10000)

	ROM_LOAD16_BYTE( "bmc_6.bin", 0x180000, 0x20000, CRC(7b9e0d77) SHA1(1ec1c92c6d4c512f7292b77e9663518085684ba9) )
	ROM_LOAD16_BYTE( "bmc_5.bin", 0x180001, 0x20000, CRC(708b6f8b) SHA1(4a910126d87c11fed99f44b61d51849067eddc02) )

	ROM_REGION( 0x040000, REGION_SOUND1, 0 ) /* Samples */
	ROM_LOAD( "bmc_10.bin", 0x00000, 0x20000,  CRC(f840c17f) SHA1(82891a85c8dc60f727b5a8c8e8ab09e8e4bd8af4) )

ROM_END

static DRIVER_INIT(bmcbowl)
{
	via_config(0, &via_interface);
	via_set_clock(0, 1000000);//1 MHz ?
	colorram=auto_malloc(768);
}

GAME( 1994, bmcbowl,    0, bmcbowl,    bmcbowl,    bmcbowl, ROT0,  "BMC", "BMC Bowling", GAME_IMPERFECT_SOUND | GAME_IMPERFECT_GRAPHICS )
