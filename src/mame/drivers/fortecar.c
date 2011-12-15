/*************************************************************************************************

  Forte Card

  Driver by Angelo Salese.
  Additional work by Roberto Fresca & Rob Ragon.


  TODO:

  - Improve serial EEPROM support.
  - RTC needs its own core.
  - Inputs


  English set: bp 512 do pc=53e
  Spanish set: bp 512 do pc=562

-------------------------------------------------------------------------------------------------

  Forte Card (POKER GAME)

  CPU  SGS Z8400AB1 (Z80ACPU)
  VIDEO CM607P
  PIO M5L8255AP
  snd ay38910/P (microchip)
  + 8251A

  RAM 6116 + GOLDSTAR GM76C256ALL-70
  dip 1X8

-------------------------------------------------------------------------------------------------

  Forte Card - By Fortex Ltd.
  (Spanish Version)

  Sound    AY 3-8910 @1.5Mhz
  USART    8251
  uProc    Z80 @3MHz
  Video    6845 @1.5MHz
  PIO      8255
  NVRam    6116
  RAM      84256 (Video, I think)
  EEPROM   NM93CS56N - (MICROWIRETM Bus Interface)
           256-/1024-/2048-/4096-Bit Serial EEPROM
           with Data Protect and Sequential Read.
  RTC      V3021
  Dip 1x8

  Xtal: 12 MHz.


  NOTE:

  The CM607P IC is an exact clone of Motorola's MC6845P CRT Controller circuit
  from MC6800 family. Used to perform the interface to raster scan CRT displays.
  Made in Bulgaria in the Pravetz factory, where Pravetz-8 and Pravetz-16 compu-
  ters were made in 1980's.

  FULL equivalent to MC6845P, UM6845R, EF6845P, HD6845P, etc.

  The ST93CS56 and ST93CS57 are 2K bit Electrically Erasable Programmable Memory (EEPROM)
  fabricated with SGS-THOMSON?s High Endurance Single Polysilicon CMOS technology. The memory
  is accessed through a serial input D and output Q. The 2K bit memory is organized as 128 x 16 bit
  words.The memory is accessed by a set of instructions which include Read, Write, Page Write, Write
  All and instructions used to set the memory protection. A Read instruction loads the address of the
  first word to be read into an internal address pointer.


-------------------------------------------------------------------------------------------------

  From the manual (sic)...

  Initialization of the Forte Card circuit board
  (Init machine)

  1) Open the door of the machine, switch on the game and wait until the message
     'Permanent RAM test failed' appears.
  2) Turn the Main Control and hold it in this position.
  3) Enter the serial number of the circuit board with the eight keys.
  4) Press STAR key and wait until the message 'Machine initialization completed' appears.
  5) Release the Main Control, switch off the game and close the door.
  6) Switch on the game and wait until the demonstration displays appear.
  7) Turn the Main Control, adjust the time. This is the last step of the initialization procedure.


-------------------------------------------------------------------------------------------------

  Edge Connector / Pinouts....

  -------------------+--+------------------
     Components Side |PN| Solder Side
  -------------------+--+------------------
                 GND |01| GND
                 GND |02| GND
                 +5V |03| +5V
                 +5V |04| +5V
                +12V |05| +12V
      CUR. LOOP IN + |06| CUR. LOOP OUT +
      CUR. LOOP IN - |07| CUR. LOOP OUT -
         DOOR SWITCH |08| COUNTER IN
             PAYMENT |09| COUNTER OUT
                COIN |10| COUNTER KEY IN
        HOPPER COUNT |11| HOPPER DRIVE
         BANKNOTE IN |12| RESERVED
              CREDIT |13| RESERVED
             MANAGER |14| RESERVED
              PAGE 1 |15| RESERVE OUT
          RED BUTTON |16| RED LAMP
        BLACK BUTTON |17| BLACK LAMP
       HOLD 1 BUTTON |18| HOLD 1 LAMP
       HOLD 2 BUTTON |19| HOLD 2 LAMP
       HOLD 3 BUTTON |20| HOLD 3 LAMP
       HOLD 4 BUTTON |21| HOLD 4 LAMP
       HOLD 5 BUTTON |22| HOLD 5 LAMP
        START BUTTON |23| START LAMP
          GAIN SOUND |24| SPEAKER OUT
           GND SOUND |25| SP.OUT R/INTENSITY
  COMP.SYNC / H SYNC |26| RED
              V SYNC |27| GREEN
         GND MONITOR |28| BLUE


  Note: MANAGER and PAGE 1, are "Owner" and "Rental" modes.


**************************************************************************************************/


#define MASTER_CLOCK	XTAL_12MHz
#define CPU_CLOCK		(MASTER_CLOCK/4)
#define CRTC_CLOCK		(MASTER_CLOCK/8)
#define AY_CLOCK		(MASTER_CLOCK/8)

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eeprom.h"
#include "sound/ay8910.h"
#include "machine/8255ppi.h"
#include "machine/v3021.h"
#include "video/mc6845.h"
#include "machine/nvram.h"
#include "video/resnet.h"

#include "fortecrd.lh"


class fortecar_state : public driver_device
{
public:
	fortecar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_vram;

	/* calendar */
	UINT8        m_cal_val;
	UINT8        m_cal_mask;
	UINT8        m_cal_com;
	UINT8        m_cal_cnt;
	system_time  m_systime;

};


static VIDEO_START(fortecar)
{
}

static SCREEN_UPDATE(fortecar)
{
	fortecar_state *state = screen->machine().driver_data<fortecar_state>();
	int x,y,count;
	count = 0;

	for (y=0;y<0x1e;y++)
	{
		for(x=0;x<0x4b;x++)
		{
			int tile,color,bpp;

			tile = (state->m_vram[(count*4)+1] | (state->m_vram[(count*4)+2]<<8)) & 0xfff;
			color = state->m_vram[(count*4)+3] & 0x1f;
			bpp = (state->m_vram[(count*4)+3] & 0x20) >> 5;

			if(bpp)
				color&=0x3;

			drawgfx_opaque(bitmap,cliprect,screen->machine().gfx[bpp],tile,color,0,0,x*8,y*8);
			count++;

		}
	}

	return 0;
}

static PALETTE_INIT( fortecar )
{
/* Video resistors...

O1 (LS374) R1K  RED
O2 (LS374) R510 RED
O3 (LS374) R220 RED
O4 (LS374) R1K  GREEN
O5 (LS374) R510 GREEN
O6 (LS374) R220 GREEN
O7 (LS374) R510 BLUE
O8 (LS374) R220 BLUE

R = 82 Ohms Pull Down.
*/
	int i;
	static const int resistances_rg[3] = { 1000, 510, 220 };
	static const int resistances_b [2] = { 510, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0,	255,	-1.0,
			3,	resistances_rg,	weights_r,	82,	0,
			3,	resistances_rg,	weights_g,	82,	0,
			2,	resistances_b,	weights_b,	82,	0);

	for (i = 0; i < 512; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette_set_color(machine, i, MAKE_RGB(r, g, b));
	}
}


static WRITE8_DEVICE_HANDLER( ppi0_portc_w )
{
/*
NM93CS56N Serial EEPROM

CS   PPI_PC0
CK   PPI_PC1
DIN  PPI_PC2
DOUT PPI_PC4
*/
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	eeprom->write_bit((data & 0x04) >> 2);
	eeprom->set_cs_line((data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	eeprom->set_clock_line((data & 0x02) ? CLEAR_LINE : ASSERT_LINE);
}

static READ8_DEVICE_HANDLER( ppi0_portc_r )
{
//  popmessage("%s",device->machine().describe_context());
	eeprom_device *eeprom = downcast<eeprom_device *>(device);
	return ((eeprom->read_bit()<<4) & 0x10);
}

static const ppi8255_interface ppi0intf =
{
/*  Init with 0x9a... A, B and high C as input
    Serial Eprom connected to Port C
*/

	DEVCB_INPUT_PORT("SYSTEM"),	/* Port A read */
	DEVCB_INPUT_PORT("INPUT"),	/* Port B read */
	DEVCB_DEVICE_HANDLER("eeprom", ppi0_portc_r),	/* Port C read */
	DEVCB_NULL,					/* Port A write */
	DEVCB_NULL,					/* Port B write */
	DEVCB_DEVICE_HANDLER("eeprom", ppi0_portc_w)	/* Port C write */
};


static WRITE8_DEVICE_HANDLER( ayporta_w )
{
/*  System Lamps...

    - bits -
    7654 3210
    ---- ---x   START lamp.
    ---- --x-   HOLD5 lamp.
    ---- -x--   HOLD4 lamp.
    ---- x---   HOLD3 lamp.
    ---x ----   HOLD2 lamp.
    --x- ----   HOLD1 lamp.
    -x-- ----   BLACK lamp.
    x--- ----   RED/BET lamp.

    Also used for POST?...

    0x01 (start): RAM test d000-d7ff
    0x02 (hold5): VRAM test d800-ffff
    0x04 (hold4): Video SYNC test
    0x08 (hold3): ROM check
    0x10 (hold2): NVRAM check
    0x20 (hold1): IRQ test
    0x40 (black): Stack RAM check
*/

	output_set_lamp_value(0, (data >> 0) & 1);	/* START lamp */
	output_set_lamp_value(1, (data >> 1) & 1);	/* HOLD5 lamp */
	output_set_lamp_value(2, (data >> 2) & 1);	/* HOLD4 lamp */
	output_set_lamp_value(3, (data >> 3) & 1);	/* HOLD3 lamp */
	output_set_lamp_value(4, (data >> 4) & 1);	/* HOLD2 lamp */
	output_set_lamp_value(5, (data >> 5) & 1);	/* HOLD1 lamp */
	output_set_lamp_value(6, (data >> 6) & 1);	/* BLACK lamp */
	output_set_lamp_value(7, (data >> 7) & 1);	/* RED/BET lamp */
}


static WRITE8_DEVICE_HANDLER( ayportb_w )
{
/*

There is a RC to 7705's Reset.
Bit7 of port B is a watchdog.

A square wave is fed to through resistor R to capacitor C, with a constant charge/discharge
time relative to the value of resistor R and value of capacitor C. If the square wave halts,
capacitor C will charge beyond the hysteresis threshhold of the TL7705 (leg 6), causing it to
trigger a reset.

Seems to work properly, but must be checked closely...

*/
	if (((data >> 7) & 0x01) == 0)		/* check for bit7 */
	{
		watchdog_reset(device->machine());
	}

//  logerror("AY port B write %02x\n",data);
}


static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(ayporta_w),
	DEVCB_HANDLER(ayportb_w)
};


static const mc6845_interface mc6845_intf =
{
	"screen",	/* screen we are acting on */
	8,			/* number of pixels per video memory address */
	NULL,		/* before pixel update callback */
	NULL,		/* row update callback */
	NULL,		/* after pixel update callback */
	DEVCB_NULL,	/* callback for display state changes */
	DEVCB_NULL,	/* callback for cursor state changes */
	DEVCB_NULL,	/* HSYNC callback */
	DEVCB_NULL,	/* VSYNC callback */
	NULL		/* update address callback */
};


static const eeprom_interface forte_eeprom_intf =
{/*
    Preliminary interface for NM93CS56N Serial EEPROM.
    Correct address & data. Using 93C46 similar protocol.
*/
	7,                /* address bits */
	16,               /* data bits */
	"*110",           /* read command */
	"*101",           /* write command */
	"*111",           /* erase command */
	"*10000xxxxxx",   /* lock command */
	"*10011xxxxxx",   /* unlock command */
};


static ADDRESS_MAP_START( fortecar_map, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xd800, 0xffff) AM_RAM AM_BASE_MEMBER(fortecar_state, m_vram)
ADDRESS_MAP_END

static ADDRESS_MAP_START( fortecar_ports, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)	// pc=444
	AM_RANGE(0x21, 0x21) AM_DEVWRITE_MODERN("crtc", mc6845_device, register_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("fcppi0", ppi8255_r, ppi8255_w)//M5L8255AP
//  AM_RANGE(0x80, 0x81) //8251A UART
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE_MODERN("rtc", v3021_device, read, write)
	AM_RANGE(0xa1, 0xa1) AM_READ_PORT("DSW")
ADDRESS_MAP_END
/*

CRTC REGISTER: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
CRTC DATA    : 5f 4b 50 08 21 05 1e 1f 00 07 20 00 06 00 00 00

Error messages:

"FALSA PRUEBA NVR"              (NVRAM new, no serial EEPROM connected)
"FALLO EN NVR"                  (NVRAM ok, no serial EEPROM connected)
"FALSA PRUEBA NVRAM PERMANENTE" (NVRAM new, serial EEPROM connected)

*/

static INPUT_PORTS_START( fortecar )
	PORT_START("DSW")	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "DSW-1" )				PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Attract Mode" )		PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW-3" )				PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW-4" )				PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW-5" )				PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW-6" )				PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW-7" )				PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW-8" )				PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INPUT")	/* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red / Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("SYSTEM")	/* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "Rear Door" ) // key in
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )  PORT_NAME("Key In")
//	PORT_DIPNAME( 0x20, 0x20, "Credit Key" )
//	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
//	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Owner" ) // full service
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Rental" ) // page 1
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout_3bpp =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3)+4, RGN_FRAC(1,3)+4, RGN_FRAC(0,3)+4 },
	{ 8,9,10,11,0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tiles8x8_layout_6bpp =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(1,3)+0, RGN_FRAC(0,3)+0, RGN_FRAC(2,3)+4, RGN_FRAC(1,3)+4, RGN_FRAC(0,3)+4 },
	{ 8,9,10,11,0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static GFXDECODE_START( fortecar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout_3bpp, 0x000, 0x20 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout_6bpp, 0x100, 0x04 )
GFXDECODE_END



static MACHINE_RESET(fortecar)
{
//  fortecar_state *state = machine.driver_data<fortecar_state>();

}


static MACHINE_CONFIG_START( fortecar, fortecar_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)		 /* 3 MHz, measured */
	MCFG_CPU_PROGRAM_MAP(fortecar_map)
	MCFG_CPU_IO_MAP(fortecar_ports)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(200))	/* guess */

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 600-1, 0, 240-1)	/* driven by CRTC */
	MCFG_SCREEN_UPDATE(fortecar)

	MCFG_MACHINE_RESET(fortecar)

	MCFG_EEPROM_ADD("eeprom", forte_eeprom_intf)
	MCFG_EEPROM_DEFAULT_VALUE(0)

	MCFG_PPI8255_ADD("fcppi0", ppi0intf)
	MCFG_V3021_ADD("rtc")

	MCFG_GFXDECODE(fortecar)
	MCFG_PALETTE_LENGTH(0x200)
	MCFG_PALETTE_INIT(fortecar)

	MCFG_VIDEO_START(fortecar)

	MCFG_MC6845_ADD("crtc", MC6845, CRTC_CLOCK, mc6845_intf)	/* 1.5 MHz, measured */

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, AY_CLOCK)	/* 1.5 MHz, measured */
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( fortecar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fortecar.u7", 0x00000, 0x010000, CRC(2a4b3429) SHA1(8fa630dac949e758678a1a36b05b3412abe8ae16)  )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fortecar.u38", 0x00000, 0x10000, CRC(c2090690) SHA1(f0aa8935b90a2ab6043555ece69f926372246648) )
	ROM_LOAD( "fortecar.u39", 0x10000, 0x10000, CRC(fc3ddf4f) SHA1(4a95b24c4edb67f6d59f795f86dfbd12899e01b0) )
	ROM_LOAD( "fortecar.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )

	/* took from the Spanish version, these are likely to be identical anyway */
	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, BAD_DUMP CRC(fd5be302) SHA1(862f584aa8073bcefeeb290b99643020413fb7ef) )
//  ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, CRC(71f70589) SHA1(020e17617f9545cab6d174c5577c0158922d2186) )

	ROM_REGION( 0x0100,	"eeprom", 0 )	/* default serial EEPROM */
	ROM_LOAD( "forte_card_93cs56.u13", 0x0000, 0x0100, BAD_DUMP CRC(13180f47) SHA1(bb04ea1eac5e53831aece3cfdf593ae824219c0e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "forte_card_82s147.u47", 0x0000, 0x0200, BAD_DUMP CRC(7e631818) SHA1(ac08b0de30260278af3a1c5dee5810d4304cb9ca) )
ROM_END

ROM_START( fortecrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "forte_card.u7", 0x00000, 0x010000, CRC(79fc6dd3) SHA1(5454f2ee12b62d573b61c54e48398f43332b000e) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "forte_card.u38", 0x00000, 0x10000, CRC(258fb7bf) SHA1(cd75001fe40836b2dc229caddfc38f6076df7a79) )
	ROM_LOAD( "forte_card.u39", 0x10000, 0x10000, CRC(3d9c478e) SHA1(eb86115d1c36038f2c80cd116f5aeddd94036424) )
	ROM_LOAD( "forte_card.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )

	ROM_REGION( 0x0800,	"nvram", 0 )	/* default NVRAM */
	ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, CRC(fd5be302) SHA1(862f584aa8073bcefeeb290b99643020413fb7ef) )
//  ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, CRC(71f70589) SHA1(020e17617f9545cab6d174c5577c0158922d2186) )

	ROM_REGION( 0x0100,	"eeprom", 0 )	/* default serial EEPROM */
	ROM_LOAD( "forte_card_93cs56.u13", 0x0000, 0x0100, CRC(13180f47) SHA1(bb04ea1eac5e53831aece3cfdf593ae824219c0e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "forte_card_82s147.u47", 0x0000, 0x0200, CRC(7e631818) SHA1(ac08b0de30260278af3a1c5dee5810d4304cb9ca) )
ROM_END


static DRIVER_INIT( fortecar )
{
}


/*     YEAR  NAME      PARENT    MACHINE   INPUT     INIT      ROT    COMPANY       FULLNAME               FLAGS             LAYOUT */
GAMEL( 19??, fortecar, 0,        fortecar, fortecar, fortecar, ROT0, "Fortex Ltd", "Forte Card (English)", GAME_NOT_WORKING, layout_fortecrd )
GAMEL( 19??, fortecrd, fortecar, fortecar, fortecar, fortecar, ROT0, "Fortex Ltd", "Forte Card (Spanish)", GAME_NOT_WORKING, layout_fortecrd )
