/******************************************************************************

    COINMASTER-GAMING - VGA BOARD
    -----------------------------

    Preliminary driver by Roberto Fresca & Angelo Salese.


    Games running on this hardware:

    * Colorama (standalone),   199x-2001,  Coinmaster-Gaming, Ltd.
    * Roulette (bet station).  199x-2001,  Coinmaster-Gaming, Ltd.
    * Keno, (bet station).     2000-2001,  Coinmaster-Gaming, Ltd.



*******************************************************************************


  Hardware Notes:
  ---------------

  This board is used in each bet station of Coinmaster's Roulette and Keno game.
  Both systems have a phisical electromechanical unit with their own controller
  plus sound. The central units (wheel controller) are routed to the bet stations
  (10 for default) through 2 different kind of networks, depending the wheel system.

  There are 2 multisystems types:


  - System RWC (Roulette Wheel Controller)

  The old version. The central unit is tied to a controller and then routed to
  the bet stations using a custom serial network with RJ9 connectors and regular
  phone cable. Also has a thermal printer attached.


  - System Y2K (Year 2000)

  The newer system. The central unit has 4 trays inside with power supply, roulette
  controller, and a custom HUB board. All the system is interconnected through
  regular RJ45 CAT5 cables from the hub board. Also has a thermal printer attached.


  All systems are controlled through a touch screen.


  Colorama is a standalone roulette system, driven by one VGA board. It has a big lamps
  loom, and a vertical positioned roulette painted on the marquee with one phisical arm
  pointing to the numbers.



  Bet station (VGA board):
  -----------------------

  1x H8 CPU. (HD6413007F20).   (IC01)
  1x AMD MACH 131-15JC CPLD.   (IC29)
  1x AMD MACH 231-7JC CPLD.    (IC12)
  1x ADV471.                   (IC30)

  1x YMZ280B (sound).          (IC33)

  1x PALCE 22V10H25 PC/4.      (IC11)
  1x MSM62X42B.                (IC06)
  1x COM20020.                 (IC31)

  2x K6T10082CE-DB70.          (IC04, IC05)
  2x HY62256BLP-70.            (IC13, IC14)

  1x 14.7456 MHz.Xtal (H8 CPU direct clock).
  1x 20.0000 MHz.Xtal (COM20020 direct clock).
  1x 50.35 Mhz module (MACH 231 direct clock).
  1x 16.9344 MHz.Xtal (YMZ280B direct? clock).

  2x 27c040 --> Program.
  2x 27c040 --> Foreground GFX.
  4x 27c801 --> Background GFX.
  1x 27c801 --> Sound.

  1x 15-pin VGA connector.
  1x Microtouch connector.



  The VGA board is a general-purpose control board with the following features:

  * 24 discrete inputs (pulled up on board).
  * 32 discrete open collector Darlington drivers.
  * 2 RS232 channels.
  * Stereo, digital sound replay and amplification.
  * Arcnet network communications interface.
  * VGA video graphics generation
  * Power supply monitoring and static (spark) discharge detector.
  * Battery backed up RAM


  Input N? | Pin N? | Colours | Ground
  ---------+--------+---------+---------------------------------------------------------------
  00       | 1A     | Brn-Wht | Universal Hopper Early / AWP hopper switch 1 (?1) / Y2K Hop 1.
  01       | 2A     | Red-Wht | Universal Hopper Late / AWP hopper switch 2 (20p) / Y2K Hop 2.
  02       | 3A     | Org-Wht | CC46 Coin / C435 Accept 1.
  03       | 4A     | Yel-Wht | CC46 Error / C435 Accept 2.
  04       | 5A     | Grn-Wht | CC46 Sensor / C435 Accept 3.
  05       | 6A     | Blu-Wht | Lid Switch.
  06       | 7A     | Vio-Wht | Door Switch.
  07       | 8A     | Gry-Wht | Key Switch.
  08       | 17A    | Brn-Pnk | Shooting Button / Start Button.
  09       | 18A    | Red-Pnk | C435 Accept 4.
  10       | 19A    | Org-Pnk | Dumbcard Present.
  11       | 20A    | Yel-Pnk | Small Hopper Return / Y2K Hop 3.
  12       | 21A    | Grn-Pnk | Dumbcard / Bill.
  13       | 22A    | Blu-Pnk | C435 Accept 5.
  14       | 23A    | Vio-Pnk | Cancel Button.
  15       | 24A    | Gry-Pnk | Payout Button.
  16       | 27A    | Red-Brn | Gamble Button.
  17       | 28A    | Org-Brn |
  18       | 29A    | Yel-Brn |
  19       | 30A    | Grn-Brn |
  20       | 31A    | Blu-Brn |
  21       | 32A    | Vio-Brn |
  22       | 33A    | Gry-Brn |
  23       | 34A    | Pnk-Brn |


  Output N? | Pin N? | Colours | Description
  ----------+--------+---------+---------------------------------------------------------------
  00        | 4B     | Brn-Blu | Universal Hopper Drive / AWP hopper drive 1 (?1) / Y2K Hop 1.
  01        | 5B     | Red-Blu | Candle 1.
  02        | 6B     | Org-Blu | Candle 2.
  03        | 7B     | Yel-Blu | Cancel Button Lamp.
  04        | 8B     | Grn-Blu | CC46 Deflector / C435 Inhibit 1.
  05        | 1B     | Vio-Blu | CC46 Inhibit / C435 Inhibit 2.
  06        | 2B     | Gry-Blu | C435 Inhibit 3.
  07        | 3B     | Wht-Blu | C435 Inhibit 4.
  08        | 10B    | Brn-Vio | Small Hopper Drive / AWP hopper drive 2 (20p) / Y2K Hop 2.
  09        | 11B    | Red-Vio | Shooting Button Lamp / Start Button Lamp.
  10        | 12B    | Org-Vio | Payout Button Lamp.
  11        | 13B    | Yel-Vio | Gamble Button Lamp.
  12        | 14B    | Vio-Red | Y2K Hop 3.
  13        | 15B    | Blu-Vio | Candle 3.
  14        | 16B    | Gry-Vio | C435 Overide B.
  15        | 17B    | Wht-Vio | C435 Overide C.
  16        | 24B    | Brn-Gry | C435 Inhibit 5.
  17        | 25B    | Red-Gry | C435 Inhibit 6.
  18        | 26B    | Org-Gry | C435 Inhibit 7.
  19        | 27B    | Yel-Gry | C435 Inhibit 8.
  20        | 28B    | Grn-Gry | C435 Overide D.
  21        | 29B    | Blu-Gry |
  22        | 30B    | Vio-Gry | Dumbcard Trigger.
  23        | 31B    | Wht-Gry | Screen Supply.
  24        | 38B    | Org-Red | Credit In (Coin In).
  25        | 39B    | Yel-Red | Credit Out (Coin Out).
  26        | 40B    | Grn-Red | Cashbox (Cashbox).
  27        | 37B    | Blu-Red | Remote Out (Canceled Credits).
  28        | 36B    | Vio-Red | Games.
  29        | 35B    | Gry-Red | Tot Wins.
  30        | 34B    | Wht-Red | Total Bet.
  31        | 33B    | Pnk-Red | Jackpot.


*******************************************************************************


  *** Games Notes ***


  Nothing, yet...


*******************************************************************************

    --------------------
    ***  Memory Map  ***
    --------------------

    0x000000 - 0x09FFFF    ; ROM space.
    0x210000 - 0x21FFFF    ; NVRAM?.


*******************************************************************************


    DRIVER UPDATES:


    [2009-08-18]

    - Renamed Roulette V75 to Coinmaster Roulette V75.
    - Added Roulette controller program & sound ROMs.
    - Added 2 complete spanish Keno sets.
    - Added technical notes.


    [2009-08-17]

    - Initial release.
    - Added technical notes.


    TODO:

    - Interrupts generation is unknown.
    - Touch screen hook-up.
    - Fully understand why it trigger some RTEs that should be RTS at POST.


*******************************************************************************/


#define CPU_CLOCK	XTAL_14_7456MHz
#define MACH_CLOCK	XTAL_50MHz		// 50.35
#define COM_CLOCK	XTAL_20MHz
#define SND_CLOCK	XTAL_16_9344MHz

#include "emu.h"
#include "cpu/h83002/h8.h"
#include "sound/ymz280b.h"
#include "machine/nvram.h"


class coinmvga_state : public driver_device
{
public:
	coinmvga_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram(*this, "vram"){ }

	required_shared_ptr<UINT16> m_vram;
	struct { int r,g,b,offs,offs_internal; } m_bgpal, m_fgpal;
	DECLARE_WRITE8_MEMBER(debug_w);
	DECLARE_WRITE16_MEMBER(ramdac_bg_w);
	DECLARE_WRITE16_MEMBER(ramdac_fg_w);
	DECLARE_READ16_MEMBER(test_r);
};


/*************************
*     Video Hardware     *
*************************/


static VIDEO_START( coinmvga )
{
}


static SCREEN_UPDATE_IND16( coinmvga )
{
	coinmvga_state *state = screen.machine().driver_data<coinmvga_state>();
	const gfx_element *gfx = screen.machine().gfx[0];
	int count = 0x04000/2;

	int y,x;


	for (y=0;y<64;y++)
	{
		for (x=0;x<128;x++)
		{
			int tile = state->m_vram.target()[count];
			//int colour = tile>>12;
			drawgfx_opaque(bitmap,cliprect,gfx,tile,0,0,0,x*8,y*8);

			count++;
		}
	}


	return 0;
}


static PALETTE_INIT( coinmvga )
{

}


/**************************
*  Read / Write Handlers  *
**************************/

//WRITE8_MEMBER(coinmvga_state::debug_w)
//{
//  popmessage("written : %02X", data);
//}

WRITE16_MEMBER(coinmvga_state::ramdac_bg_w)
{
	if(ACCESSING_BITS_8_15)
	{
		m_bgpal.offs = data >> 8;
		m_bgpal.offs_internal = 0;
	}
	else //if(mem_mask == 0x00ff)
	{
		switch(m_bgpal.offs_internal)
		{
			case 0:
				m_bgpal.r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				m_bgpal.offs_internal++;
				break;
			case 1:
				m_bgpal.g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				m_bgpal.offs_internal++;
				break;
			case 2:
				m_bgpal.b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				palette_set_color(machine(), m_bgpal.offs, MAKE_RGB(m_bgpal.r, m_bgpal.g, m_bgpal.b));
				m_bgpal.offs_internal = 0;
				m_bgpal.offs++;
				break;
		}
	}
}


WRITE16_MEMBER(coinmvga_state::ramdac_fg_w)
{
	if(ACCESSING_BITS_8_15)
	{
		m_fgpal.offs = data >> 8;
		m_fgpal.offs_internal = 0;
	}
	else
	{
		switch(m_fgpal.offs_internal)
		{
			case 0:
				m_fgpal.r = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				m_fgpal.offs_internal++;
				break;
			case 1:
				m_fgpal.g = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				m_fgpal.offs_internal++;
				break;
			case 2:
				m_fgpal.b = ((data & 0x3f) << 2) | ((data & 0x30) >> 4);
				palette_set_color(machine(), 0x100+m_fgpal.offs, MAKE_RGB(m_fgpal.r, m_fgpal.g, m_fgpal.b));
				m_fgpal.offs_internal = 0;
				m_fgpal.offs++;
				break;
		}
	}
}

/*
READ16_MEMBER(coinmvga_state::test_r)
{
    return machine().rand();
}*/

/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( coinmvga_map, AS_PROGRAM, 16, coinmvga_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0) //maybe not

//  AM_RANGE(0x0a0000, 0x0fffff) AM_RAM
//  AM_RANGE(0x100000, 0x1fffff) AM_RAM //colorama
	AM_RANGE(0x210000, 0x21ffff) AM_RAM AM_SHARE("vram")
//  AM_RANGE(0x40746e, 0x40746f) AM_READ(test_r) AM_WRITENOP //touch screen related, colorama
//  AM_RANGE(0x403afa, 0x403afb) AM_READ(test_r) AM_WRITENOP //touch screen related, cmrltv75
	AM_RANGE(0x400000, 0x40ffff) AM_RAM

	AM_RANGE(0x600000, 0x600001) AM_WRITE(ramdac_bg_w)
	AM_RANGE(0x600004, 0x600005) AM_WRITE(ramdac_fg_w)
	AM_RANGE(0x600008, 0x600009) AM_DEVREADWRITE8_LEGACY("ymz", ymz280b_r, ymz280b_w, 0xffff)
	AM_RANGE(0x610000, 0x61000f) AM_RAM //touch screen i/o

	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1") //"arrow" r?
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	//0x800008 "arrow" w?
ADDRESS_MAP_END

static ADDRESS_MAP_START( coinmvga_io_map, AS_IO, 8, coinmvga_state )
/*  Digital I/O ports (ports 4-B are valid on 16-bit H8/3xx) */
//  AM_RANGE(H8_PORT_4, H8_PORT_4)
//  AM_RANGE(H8_PORT_5, H8_PORT_5)
//  AM_RANGE(H8_PORT_6, H8_PORT_6)
//  AM_RANGE(H8_PORT_7, H8_PORT_7) <---- 0006 RW colorama
//  AM_RANGE(H8_PORT_8, H8_PORT_8)
//  AM_RANGE(H8_PORT_9, H8_PORT_9)
//  AM_RANGE(H8_PORT_A, H8_PORT_A)
//  AM_RANGE(H8_PORT_B, H8_PORT_B)

/*  Analog Inputs */
//  AM_RANGE(H8_ADC_0_H, H8_ADC_0_L)
//  AM_RANGE(H8_ADC_1_H, H8_ADC_1_L)
//  AM_RANGE(H8_ADC_2_H, H8_ADC_2_L)
//  AM_RANGE(H8_ADC_3_H, H8_ADC_3_L)

/*  Serial ports */
//  AM_RANGE(H8_SERIAL_0, H8_SERIAL_0)
//  AM_RANGE(H8_SERIAL_1, H8_SERIAL_1)

ADDRESS_MAP_END

/*  unknown writes (cmrltv75):

CPU 'main' (PC=00000218): unmapped program memory word write to 00FFFFF2 = 001E & 00FF
CPU 'main' (PC=0000021C): unmapped program memory word write to 00FFFFEC = E800 & FF00
CPU 'main' (PC=00000220): unmapped program memory word write to 00FFFFEC = 00FF & 00FF
CPU 'main' (PC=00000224): unmapped program memory word write to 00FFFFEE = 00FD & 00FF
CPU 'main' (PC=00000228): unmapped program memory word write to 00FFFFEE = F300 & FF00
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040FBF2 = 0000 & FFFF
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040FBF4 = 023E & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040FBEE = 0000 & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040FBF0 = 0254 & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040FBEE & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040FBF0 & FFFF


**  unknown writes (colorama):


CPU 'main' (PC=00000218): unmapped program memory word write to 00FFFFF2 = 001E & 00FF
CPU 'main' (PC=0000021C): unmapped program memory word write to 00FFFFEC = E800 & FF00
CPU 'main' (PC=00000220): unmapped program memory word write to 00FFFFEC = 00FF & 00FF
CPU 'main' (PC=00000224): unmapped program memory word write to 00FFFFEE = 00FD & 00FF
CPU 'main' (PC=00000228): unmapped program memory word write to 00FFFFEE = F300 & FF00
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040B572 = 0000 & FFFF
CPU 'main' (PC=0000023C): unmapped program memory word write to 0040B574 = 023E & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040B56E = 0000 & FFFF
CPU 'main' (PC=00000252): unmapped program memory word write to 0040B570 = 0254 & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040B56E & FFFF
CPU 'main' (PC=00000330): unmapped program memory word read from 0040B570 & FFFF
CPU 'main' (PC=000000D4): unmapped program memory word write to 0040B5FA = 0000 & FFFF
CPU 'main' (PC=000000D4): unmapped program memory word write to 0040B5FC = 00D6 & FFFF


000214: F81E    ;mov.b #1e, R0L
000216: 38F3    ;mov.b R0L, @0ffffff3
000218: F8E8    ;mov.b #e8, R0L
00021A: 38EC    ;mov.b R0L, @0fffffec
00021C: F8FF    ;mov.b #ff, R0L
00021E: 38ED    ;mov.b R0L, @0fffffed
000220: F8FD    ;mov.b #fd, R0L
000222: 38EF    ;mov.b R0L, @0fffffef
000224: F8F3    ;mov.b #f3, R0L
000226: 38EE    ;mov.b R0L, @0fffffee
000228: F8F8    ;mov.b #f8, R0L
00022A: 38CD    ;mov.b R0L, @0fffffcd

600000 W \
600002 W  | seems registers...
600004 W  |
600008 W /

210000-21ffff W (NVRAM?)


I/O

0006 RW


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( coinmvga )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-0001")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN0-0002")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN0-0004")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN0-0008")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN0-0010")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN0-0020")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN0-0040")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN0-0080")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN0-0100")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN0-0200")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN0-0400")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN0-0800")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN0-1000")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN0-2000")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN0-4000")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN0-8000")

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN1-0001")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN1-0002")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN1-0004")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN1-0008")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN1-0010")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN1-0020")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN1-0040")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN1-0080")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN1-0100")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN1-0200")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN1-0400")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN1-0800")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN1-1000")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN1-2000")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN1-4000")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN1-8000")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
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


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout tiles8x8_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 3,2,1,0 },
	{ 12, 8, 4, 0, 28, 24, 20, 16  },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};

/* FIX ME */
static const gfx_layout tiles16x16_layout =
{
	8,8,
	RGN_FRAC(1,2),
	8,
	{ RGN_FRAC(1,2)+3,3,RGN_FRAC(1,2)+2,2,RGN_FRAC(1,2)+1,1,RGN_FRAC(1,2)+0,0 },
	{ 12, 4, 28, 20, 12+32*8, 4+32*8, 28+32*8, 20+32*8  },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8*2
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( coinmvga )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout,   0x100, 16 )	/* Foreground GFX */
	GFXDECODE_ENTRY( "gfx2", 0, tiles16x16_layout, 0x000, 16 )	/* Background GFX */
GFXDECODE_END


/*************************
*    Sound Interface     *
*************************/

static const ymz280b_interface ymz280b_intf =
{
	0	// irq ?
};

static INTERRUPT_GEN( vblank_irq )
{
	//printf("1\n");
	device_set_input_line(device, 2, HOLD_LINE);
}


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( coinmvga, coinmvga_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H83007, CPU_CLOCK)	/* xtal */
	MCFG_CPU_PROGRAM_MAP(coinmvga_map)
	MCFG_CPU_IO_MAP(coinmvga_io_map)
	MCFG_CPU_VBLANK_INT("screen", vblank_irq)	/* wrong, fix me */

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640,480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_STATIC(coinmvga)

	MCFG_GFXDECODE(coinmvga)

	MCFG_PALETTE_INIT(coinmvga)
	MCFG_PALETTE_LENGTH(512)

	MCFG_VIDEO_START(coinmvga)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, SND_CLOCK)
	MCFG_SOUND_CONFIG(ymz280b_intf)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

/*
   Colorama (english)
   Standalone. Phisical arm on marquee + bet station.
*/

ROM_START( colorama )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "p521_prg1.cp1", 0x00001, 0x80000, CRC(f5da12cb) SHA1(d75fdda09e57c8a4637b0bcc98931796b5449435) )
	ROM_LOAD16_BYTE( "p521_prg2.cp2", 0x00000, 0x80000, CRC(6db85d66) SHA1(21009aa01db5193d1be588deaeba8f89582d53dd) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p521_fore1.fg1",	0x00001, 0x80000, CRC(0a8fe27a) SHA1(29500040e2bd0b6f349abaf51bb7f8aaac73e8cf) )
	ROM_LOAD16_BYTE( "p521_fore2.fg2",	0x00000, 0x80000, CRC(3ae78445) SHA1(ef590a6042969718d88732244d2639b7cd8ab507) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p521_back1.bg1",	0x200001, 0x100000, CRC(0c1a7a2d) SHA1(a7827c6091d0f78e146419261eca427cd229d445) )
	ROM_LOAD16_BYTE( "p521_back2.bg2",	0x200000, 0x100000, CRC(218912d7) SHA1(64e3dc22ff6ae296e1843b6d6bfb02eb0d202db5) )
	ROM_LOAD16_BYTE( "p521_back3.bg3",	0x000001, 0x100000, CRC(8ddad7d1) SHA1(0a41ca166c8a9eca2ee27d35a3ae41ddb8759dce) )
	ROM_LOAD16_BYTE( "p521_back4.bg4",	0x000000, 0x100000, CRC(28d54ce1) SHA1(0dadae2e11f9b86dddb6a0c33abfbdb8b6f2d862) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "p521_snd.sp1",	0x00000, 0x100000, CRC(5c87bb98) SHA1(bc1b8c090fbae166e3a7e1da74bfd2e84c1a03f6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",	0x0000, 0x0200, NO_DUMP )

ROM_END


/*
   Coinmaster Roulette V75 (y2k, spanish)
   Phisical Unit + 10/15 bet stations.
*/

ROM_START( cmrltv75 )

    /*** Bet Station ***/

	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet.cp1",   0x00001, 0x80000, CRC(2dc1c899) SHA1(2be488d23df5e50bbcfa4e66a49a455c617b29b4) )
	ROM_LOAD16_BYTE( "bet.cp2",   0x00000, 0x80000, CRC(fcab8825) SHA1(79cb862ac5363ab90e91184efd9cfaec86bb82a5) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "p497.fg1",	0x00001, 0x80000, CRC(ce5f9fe9) SHA1(a30f5f375eaa651ede4057449c1648c64d207577) )
	ROM_LOAD16_BYTE( "p497.fg2",	0x00000, 0x80000, CRC(3846fad0) SHA1(409725ab8c9353a8d5774c5f010ace1077b3fd35) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "p497.bg1",	0x200001, 0x100000, CRC(fadf2a5a) SHA1(ac5413ff213ef5c6210e716a24cd41519b81a54a) )
	ROM_LOAD16_BYTE( "p497.bg2",	0x200000, 0x100000, CRC(5d648914) SHA1(2a4a2839293529aee500aacfbf1d6b12b328b2eb) )
	ROM_LOAD16_BYTE( "p497.bg3",	0x000001, 0x100000, CRC(627e236c) SHA1(a4bd8b482cbac2bf2ab1723ee61d32480ede8985) )
	ROM_LOAD16_BYTE( "p497.bg4",	0x000000, 0x100000, CRC(3698f748) SHA1(856eeed8eff79273ba3aafbbd5d0b1d89e9cff5b) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",	0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",	0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/

	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel.cp1",   0x00001, 0x80000, CRC(57f8a869) SHA1(2a3cdae1bad5e94506b9b42b7f2630c52ffcbbef) )
	ROM_LOAD16_BYTE( "wheel.cp2",   0x00000, 0x80000, CRC(a8441b04) SHA1(cc8f10390947c2a15b2c94b11574c5eeb69fded5) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",	0x000000, 0x100000, CRC(13d6cff5) SHA1(ad1858f251e11017a427cbf7219d78bb2b854528) )
	ROM_LOAD( "rwc497ym.sp2",	0x100000, 0x100000, CRC(f8c7efd1) SHA1(e86a7ef0617c85415334e1f39a9059d5b16bc7d1) )
	ROM_LOAD( "rwc497ym.sp3",	0x200000, 0x100000, CRC(a1977dff) SHA1(c405bf1f1721ae864a2ff91ec7d637f03e431ad4) )
	ROM_LOAD( "rwc497ym.sp4",	0x300000, 0x100000, CRC(f8cb0fb8) SHA1(3ea8f268bc8745a257eb4b20d7e79196d0f1fb9e) )
	ROM_LOAD( "rwc497ym.sp5",	0x400000, 0x100000, CRC(788b52f7) SHA1(1b339cb984b807a08e6fde260b5ee2bc8ca66f62) )
	ROM_LOAD( "rwc497ym.sp6",	0x500000, 0x100000, CRC(be94fd18) SHA1(2884cae7cf96008a78e77f42e8efb5c3ca8f4a4d) )

ROM_END


/*
   Coinmaster Keno (y2k, spanish, 2000-12-14)
   Phisical Unit + 10 bet stations.
*/

ROM_START( cmkenosp )

    /*** Bet Station ***/

	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet1.cp1",   0x00001, 0x80000, CRC(ee04b815) SHA1(cea29973cf9caa5c06bc312fc3b19e146c1ae063) )
	ROM_LOAD16_BYTE( "bet2.cp2",   0x00000, 0x80000, CRC(32071845) SHA1(278217a70ea777f82ae91d11d51b832383eafdbe) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fore1.fg1",	0x00001, 0x80000, CRC(a3548c2a) SHA1(02f98ee09581a235df3704951683f9d2aab3b1e8) )
	ROM_LOAD16_BYTE( "fore2.fg2",	0x00000, 0x80000, CRC(8b1afa73) SHA1(efd176dfb55f047b8e01b9460469936c86953417) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "back1.bg1",	0x200001, 0x100000, CRC(8e9d1753) SHA1(4a733bc6b284571b2dae9e80ba8b88724e9dbffb) )
	ROM_LOAD16_BYTE( "back2.bg2",	0x200000, 0x100000, CRC(aa4fe1ba) SHA1(241cf6ee13664d4cf0c559c26170cb561afca016) )
	ROM_LOAD16_BYTE( "back3.bg3",	0x000001, 0x100000, CRC(9f26f0e0) SHA1(0c3d78e2befc6fdeb8c3534f5278d2d275106219) )
	ROM_LOAD16_BYTE( "back4.bg4",	0x000000, 0x100000, CRC(96d33887) SHA1(ca7eb9f2cfeb65c69e837246c8c78ea56c057e66) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",	0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",	0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/

	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel_prog_1.cp1",   0x00001, 0x80000, CRC(d56b0d2c) SHA1(d8a79fecf8fca92cc4856d9dad9f1d16d51b68a3) )
	ROM_LOAD16_BYTE( "wheel_prog_2.cp2",   0x00000, 0x80000, CRC(cfc02d3e) SHA1(09e41b26c62137b31f8673184dad565932881f47) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",	0x000000, 0x100000, CRC(90a93951) SHA1(73603f402eb3b62e69a745af9d45738f35bc0b4e) )
	ROM_LOAD( "rwc497ym.sp2",	0x100000, 0x100000, CRC(f5d0a6e7) SHA1(c4a1c333854c95e37c0040fed35b72ac1e853832) )
	ROM_LOAD( "rwc497ym.sp3",	0x200000, 0x100000, CRC(0e53c1a9) SHA1(0785c52b24277c9ba24d0fbf0ac335acb0235e23) )
	ROM_LOAD( "rwc497ym.sp4",	0x300000, 0x100000, CRC(b5729ae7) SHA1(0e63fbb81ff5f2fef3c653f769db8073dff1214b) )

ROM_END


/*
   Coinmaster Keno (y2k, spanish, 2000-12-02)
   Phisical Unit + 10 bet stations.
*/

ROM_START( cmkenospa )

    /*** Bet Station ***/

	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bet.cp1",   0x00001, 0x80000, CRC(ffdc69a0) SHA1(2ba6a36cb0953474164d4fb80a60bf8ca27e9a0c) )
	ROM_LOAD16_BYTE( "bet.cp2",   0x00000, 0x80000, CRC(c46f237c) SHA1(75a60ace7277a90b3d7acd7838d1271fd41517f1) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "fore1.fg1",	0x00001, 0x80000, CRC(a3548c2a) SHA1(02f98ee09581a235df3704951683f9d2aab3b1e8) )
	ROM_LOAD16_BYTE( "fore2.fg2",	0x00000, 0x80000, CRC(8b1afa73) SHA1(efd176dfb55f047b8e01b9460469936c86953417) )

	ROM_REGION( 0x400000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "back1.bg1",	0x200001, 0x100000, CRC(8e9d1753) SHA1(4a733bc6b284571b2dae9e80ba8b88724e9dbffb) )
	ROM_LOAD16_BYTE( "back2.bg2",	0x200000, 0x100000, CRC(aa4fe1ba) SHA1(241cf6ee13664d4cf0c559c26170cb561afca016) )
	ROM_LOAD16_BYTE( "back3.bg3",	0x000001, 0x100000, CRC(9f26f0e0) SHA1(0c3d78e2befc6fdeb8c3534f5278d2d275106219) )
	ROM_LOAD16_BYTE( "back4.bg4",	0x000000, 0x100000, CRC(96d33887) SHA1(ca7eb9f2cfeb65c69e837246c8c78ea56c057e66) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "betsound.sp1",	0x00000, 0x100000, CRC(979ecd0e) SHA1(827e8c86b27e5252368960fffe42ace167aa4495) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "palce22v10h25.u11",	0x0000, 0x0200, NO_DUMP )

	/*** Wheel Controller ***/

	ROM_REGION( 0x100000, "wheelcpu", 0 )
	ROM_LOAD16_BYTE( "wheel.cp1",   0x00001, 0x80000, CRC(16f750b7) SHA1(b1c99ec659cda1b2fe6ef9ec39f4cdd8f0ddecec) )
	ROM_LOAD16_BYTE( "wheel.cp2",   0x00000, 0x80000, CRC(49a50ae7) SHA1(89857ebd94ebbfa040d99648a46779c9ba8f85dd) )

	ROM_REGION( 0x800000, "wheelsnd", 0 ) /* the wheel controller has 8 sockets */
	ROM_LOAD( "rwc497ym.sp1",	0x000000, 0x100000, CRC(90a93951) SHA1(73603f402eb3b62e69a745af9d45738f35bc0b4e) )
	ROM_LOAD( "rwc497ym.sp2",	0x100000, 0x100000, CRC(f5d0a6e7) SHA1(c4a1c333854c95e37c0040fed35b72ac1e853832) )
	ROM_LOAD( "rwc497ym.sp3",	0x200000, 0x100000, CRC(0e53c1a9) SHA1(0785c52b24277c9ba24d0fbf0ac335acb0235e23) )
	ROM_LOAD( "rwc497ym.sp4",	0x300000, 0x100000, CRC(b5729ae7) SHA1(0e63fbb81ff5f2fef3c653f769db8073dff1214b) )

ROM_END


/*************************
*      Driver Init       *
*************************/

static DRIVER_INIT( colorama )
{
	UINT16 *ROM;
	ROM = (UINT16 *)machine.region("maincpu")->base();

	// rte in non-irq routines? wtf? patch them to rts...
	ROM[0x02B476/2] = 0x5470;
	ROM[0x02AE3A/2] = 0x5470;
	ROM[0x02A9FC/2] = 0x5470;
	ROM[0x02AA3A/2] = 0x5470;

	ROM[0x02729e/2] = 0x5470;
	ROM[0x029fb4/2] = 0x5470;
	ROM[0x02a224/2] = 0x5470;
	ROM[0x02a94e/2] = 0x5470;
}

static DRIVER_INIT( cmrltv75 )
{
	UINT16 *ROM;
	ROM = (UINT16 *)machine.region("maincpu")->base();

	// rte in non-irq routines? wtf? patch them to rts...
	ROM[0x056fd6/2] = 0x5470;
	ROM[0x05655c/2] = 0x5470;
	ROM[0x05659a/2] = 0x5470;
	ROM[0x05699a/2] = 0x5470;

	//...
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT      ROT     COMPANY                    FULLNAME                                     FLAGS */
GAME( 2001, colorama,  0,        coinmvga, coinmvga, colorama, ROT0,  "Coinmaster-Gaming, Ltd.", "Colorama (English)",                         GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2001, cmrltv75,  0,        coinmvga, coinmvga, cmrltv75, ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Roulette V75 (Y2K, Spanish)",     GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000, cmkenosp,  0,        coinmvga, coinmvga, 0,        ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Keno (Y2K, Spanish, 2000-12-14)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 2000, cmkenospa, cmkenosp, coinmvga, coinmvga, 0,        ROT90, "Coinmaster-Gaming, Ltd.", "Coinmaster Keno (Y2K, Spanish, 2000-12-02)", GAME_IMPERFECT_GRAPHICS | GAME_NO_SOUND | GAME_NOT_WORKING )
