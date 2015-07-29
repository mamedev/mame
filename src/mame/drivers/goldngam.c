// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  GOLDEN GAMES / C+M TECHNICS AG
  ------------------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Swiss Poker ('50 SG-.10', V2.5), 1990, Golden Games / C+M Technics AG.
  * Movie Card,                      1998, Golden Games / C+M Technics AG.


*******************************************************************************


  Hardware Notes:
  ---------------

  Swiss Card CPU Board...
  Stickered "GOLDEN GAMES 288"

  CPU: 1x MC68000P8.               [IC2]
  SND: 1x Microchip AY38910A.      [IC18]
       1x LM380N (audio amp).

  RAM: 2x HM62256LP-10             [IC4, IC5]

  OTHER:
  1x EF6840P (timer)               [IC3]
  1x unknown GAL labeled "CM 1.2"  [IC8]
  1x 8.000 MHz Xtal.               [Q1]


  CPU Board layout:
  ,----------------------------------------------------------------------------------------.
  |  |!!!!!!!!|   |!!!!!!!!|    |!!!!!!!!|   |!!!!!!!!|    |!!!!!!!!!!|     |||||||||||    |
  |  '--------'   '--------'    '--------'   '--------'    '----------'     '---------'    |
  |                                                                                        |
  |                                                                                  .--.  |
  | .--------.    .--------.   .--------.    .--------.                              |LM|  |
  | |ULN2803A|    |ULN2803A|   |ULN2803A|    |ULN2803A|                              |38|  |
  | '--------'    '--------'   '--------'    '--------'                              |0N|  |
  |  .---------.     .---------.       .---------.  .--------. .--------. .--------. '--'  |
  |  |74HC273AP|     |74HC251AP|       |74HC273AP|  |ULN2003A| |ULN2803A| |HC251AP |       |
  |  '---------'     '---------'       '---------'  '--------' '--------' '--------'       |
  |                                                   .--------.  .---------.   .--------. |
  |                                                   |HC259AP |  |74HC273AP|   |ZH 436 E| |
  |                                                   '--------'  '---------'   '--------' |
  |                      GOLDEN GAMES 238                                                  |
  |                         CPU 68000                                     .--------------. |
  |                      .----.  .--.    .--.       .--------. .--.       |              | |
  |                      |ICL1|  |74|    |74|       |        | |..|       |  AY38912A/P  | |
  |                      '----'  |HC|    |HC|       |        | |..|       |              | |
  |  .---------. .---------..--. |13|    |14|       |        | |..|       '--------------' |
  |  |74HC273P | |74HC273AP||74| |8A|    |B1|       |        | |..|                        |
  |  '---------' '---------'|HC| |N |    |  |       |        | |..|       .--------------. |
  |                         |13| '--'    '--'       |        | |..|       |              | |
  |  .---------.            |8A|                    | MC     | |..|       | HM62256LP_10 | |
  |  |ULN2803A |            |N |         .--.       |        | |..|       |              | |
  |  '---------'            '--' .--.    |74|       | 68000  | |..|       '--------------' |
  |             .------.         |G |    |HC|       |        | |..|                        |
  |             |HC238B|    .--. |A |    |20|       |    P8  | |..|                        |
  |             '------'    |74| |L |    |P |       |        | |..|                        |
  |                         |HC| |  |    '--'       |        | |..|                        |
  |                         |13| |20|               |        | |..|       .--------------. |
  |                         |9N| |V8|    .--.       |        | |..|       |              | |
  |                         '--' |  |    |74|       |        | |..|       | HM62256LP_10 | |
  |                              '--'    |HC|       |        | |..|       |              | |
  |                                      |05|       |        | |..|       '--------------' |
  |                                      '--'       |        | |..|                        |
  |                       .--.                      |        | |..|       .--------------. |
  |                       |HC|   .------.           |        | |..|       |              | |
  |                       |16|   |HC32AP|           |        | |..|       |   EF6840P    | |
  |                       |1A|   '------'           '--------' '--'       |              | |
  |                       '--'   .------.                                 '--------------' |
  |                              |HC4066|   .-------.                                      |
  |                              '------'   |HC251AP|    .--.  .-------.                   |
  |                     .------. .------.   '-------'    |SG|  |74HC93P|    .------------. |
  |                     |HC251A| |HC251A|                |51|  '-------'    |            | |
  |                     '------' '------'                |  |               |  MC6850P   | |
  |                                                      '--'  .-------.    |            | |
  |                                                            |74HC08A|    '------------' |
  |                                                            '-------'    .---.          |
  |                                         .--------.                      |DS1|          |
  |                                         |HC244AP |                      '---'          |
  |                                         '--------'                                     |
  |   .------------------.   .---------.   .----------.   .---------.    .----------.      |
  |   |!!!!!!!!!!!!!!!!!!|   |!!!!!!!!!|   |!!!!!!!!!!|   |!!!!!!!!!|    |!!!!!!!!!!|      |
  '----------------------------------------------------------------------------------------'

  ICL1 = ICL7673CPA
  DS1  = DS1232
  SG51 = SG51K 0382 80000 MHz



  Swiss Card Graphics Board...
  Stickered "960210"

  2x Xilinx XC7336-15
  3x TC551001BPL-70L (128K x 8 static RAM).
  2x ROMs.
  1x 3x32 Female connector.
  1x 6.000 MHz. Xtal.


  Graphics Board layout:

  .-------------------------------------------------------------------------.
  |                                                                         |
  |                                                               .-------. |
  | .--. .---------.                                              |V2.5_HI| |
  | |74| | XILINX  |                                              |.PR    | |
  | |HC| |XC7336-15|                                              |       | |
  | |12| |  PC44C  |                                              |       | |
  | |5N| | X64654M |  .-----------------------------------------. |       | |
  | |  | | ACG9539 |  |:::::::::::::::::::::::::::::::::::::::::| |       | |
  | '--' '---------'  '-----------------------------------------' |       | |
  |                                                               |       | |
  |      .--.                                                     |27C256 | |
  |      |74|                                                     '-------' |
  |      |HC|                                                               |
  |.--.  |27|         .---------.    .--. .--.  .--. .--.                   |
  ||..|  |3N|         |IQXO-100C|    |74| |74|  |74| |74|         .-------. |
  ||..|  |  |         |6.000 MHz|    |HC| |HC|  |HC| |HC|         |V2.5_LO| |
  ||..|  |  |         '---------'    |24| |24|  |24| |24|         |.PR    | |
  ||..|  |  |                        |4N| |5N|  |4N| |5N|         |       | |
  |'--'  '--'                   .--. |  | |  |  |  | |  |  .--.   |       | |
  |                             |74| |  | |  |  |  | |  |  |74|   |       | |
  | .-------.   .---------.     |HC| '--' '--'  '--' '--'  |HC|   |       | |
  | |TOSHIBA|   | XILINX  |     |24|                       |24|   |       | |
  | |       |   |XC7336-15|     |4N|   .-------..-------.  |4N|   |       | |
  | |TC     |   |  PC44C  |     |  |   |TOSHIBA||TOSHIBA|  |  |   |27C256 | |
  | |551001 |   | A40490A |     |  |   |       ||       |  '--'   '-------' |
  | |BPL-70L|   | ACG9541 |     '--'   |TC     ||TC     |                   |
  | |       |   '---------'            |551001 ||551001 |                   |
  | |       |                   .--.   |BPL-70L||BPL-70L|  .--.             |
  | |       |                   |74|   |       ||       |  |74|             |
  | |       |                   |HC|   |       ||       |  |HC|             |
  | |       |                   |24|   |       ||       |  |24|             |
  | |       |                   |4N|   |       ||       |  |4N|             |
  | '-------'                   |  |   |       ||       |  |  |             |
  |                             |  |   |       ||       |  |  |             |
  |                             '--'   '-------''-------'  '--'             |
  '-------------------------------------------------------------------------'


  Swiss Card Program Module...

  2x (LO-HI) ROMs.
  1x PALC22V10D.
  1x 3x32 Male connector.

  Program Board layout
  .--------------------------------------.
  |                                      |
  | .--------------.    .--------------. |
  | |V1.0_LO.GR    |    |V1.0_HI.GR    | |
  | |              |    |              | |
  | |        27C512|    |        27C512| |
  | '--------------'    '--------------' |
  |                                      |
  |          .---------------.           |
  |    LOW   |PALC22V10D-15PC|    HIGH   |
  |          '---------------'           |
  |--------------------------------------|
  ||||||||||||||||||||||||||||||||||||||||
  '--------------------------------------'


*******************************************************************************


  Movie Card CPU Board...
  Etched: "Golden Games"
  Stickered "JQ 6.01"

  CPU: 1x MC68000P8.
  SND: 1x YM2149F.
       1x LM380N (audio amp).

  RAM: 2x 84256A-10

  OTHER:
  1x EF6840P (timer)
  1x Xilinx XC9572 PC84AEM9917 (In-System Programmable CPLD)
  2x Philips SCN68681C1A44 (Dual asynchronous receiver/transmitter)

  1x DS1307

  1x 8.000 MHz Xtal.
  1x Unreadable Xtal.
  1x Unreadable Xtal.


*******************************************************************************


  *** Game Notes ***




*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------


*******************************************************************************


  DRIVER UPDATES:


  [2010-09-12]

  - Initial release.
  - Added technical notes.


  TODO:



*******************************************************************************/


#define MASTER_CLOCK    XTAL_8MHz   /* from CPU Board */
#define SECONDARY_CLOCK XTAL_6MHz   /* from GFX Board */

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/ay8910.h"


class goldngam_state : public driver_device
{
public:
	goldngam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu") { }

	required_shared_ptr<UINT16> m_videoram;
	DECLARE_READ16_MEMBER(unk_r);
	virtual void video_start();
	DECLARE_PALETTE_INIT(goldngam);
	UINT32 screen_update_goldngam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};


/*************************
*     Video Hardware     *
*************************/

void goldngam_state::video_start()
{
}

UINT32 goldngam_state::screen_update_goldngam(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int index = 0;

	for(int y = 0; y < 512; ++y)
	{
		for(int x = 0; x < 384; x += 2)
		{
			UINT16 word = m_videoram[index];
			bitmap.pix16(y, x) = word >> 8;
			bitmap.pix16(y, x+1) = word & 0xff;
			++index;
		}
	}

	return 0;
}


PALETTE_INIT_MEMBER(goldngam_state, goldngam)
{
}



/*************************
* Memory Map Information *
*************************/

READ16_MEMBER(goldngam_state::unk_r)
{
	int test1 = (machine().rand() & 0xae00);
//  popmessage("VAL = %02x", test1);

	return test1;
}

static ADDRESS_MAP_START( swisspkr_map, AS_PROGRAM, 16, goldngam_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0x400002, 0x400003) AM_NOP // hopper status read ?
	AM_RANGE(0x40000c, 0x40000d) AM_READ(unk_r)
	AM_RANGE(0x40000e, 0x40000f) AM_READ_PORT("DSW2")   // not sure...
	AM_RANGE(0x402000, 0x402001) AM_DEVREAD8("aysnd", ay8910_device, data_r, 0x00ff)
	AM_RANGE(0x402000, 0x402003) AM_DEVWRITE8("aysnd", ay8910_device, address_data_w, 0x00ff) //wrong

	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x500200, 0x50020f) AM_RAM //?
	AM_RANGE(0x503000, 0x503001) AM_RAM //int ack ?
	AM_RANGE(0x503002, 0x503003) AM_RAM //int ack ?
	AM_RANGE(0x503006, 0x503007) AM_RAM //int ack ?
ADDRESS_MAP_END


/* unknown R/W:

'maincpu' (0000C8): unmapped program memory word write to 501500 = 0040 & 00FF
'maincpu' (01BDB6): unmapped program memory word write to 401000 = 0300 & FF00
'maincpu' (01BDBE): unmapped program memory word write to 401000 = 5500 & FF00
'maincpu' (01BDC4): unmapped program memory word read from 401002 & FF00
'maincpu' (01BB82): unmapped program memory word write to 402000 = 0008 & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BB82): unmapped program memory word write to 402000 = 0009 & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BB82): unmapped program memory word write to 402000 = 000A & 00FF
'maincpu' (01BB88): unmapped program memory word write to 402002 = 0000 & 00FF
'maincpu' (01BA5E): unmapped program memory word write to 402000 = 0007 & 00FF
'maincpu' (01BA66): unmapped program memory word write to 402002 = 007F & 00FF
'maincpu' (01C072): unmapped program memory word write to 400002 = 0100 & FF00
'maincpu' (01C076): unmapped program memory word write to 400000 = 8200 & FF00
'maincpu' (01C07E): unmapped program memory word write to 400004 = 0B00 & FF00
'maincpu' (01C07E): unmapped program memory word write to 400006 = B700 & FF00
'maincpu' (01C084): unmapped program memory word write to 400002 = 0000 & FF00
'maincpu' (01C088): unmapped program memory word write to 400000 = 8300 & FF00
'maincpu' (01C090): unmapped program memory word write to 40000C = FF00 & FF00
'maincpu' (01C090): unmapped program memory word write to 40000E = FF00 & FF00
'maincpu' (01C096): unmapped program memory word write to 400002 = C200 & FF00
'maincpu' (01C09E): unmapped program memory word write to 400008 = 0600 & FF00
'maincpu' (01C09E): unmapped program memory word write to 40000A = 4000 & FF00
'maincpu' (0160DC): unmapped program memory word write to 501500 = 0040 & 00FF
'maincpu' (015102): unmapped program memory word read from 500400 & 00FF
'maincpu' (0151BE): unmapped program memory word write to 401000 = 0300 & FF00
'maincpu' (0151C6): unmapped program memory word write to 401000 = 1500 & FF00
'maincpu' (0151CC): unmapped program memory word read from 401002 & FF00

*/

static ADDRESS_MAP_START( moviecrd_map, AS_PROGRAM, 16, goldngam_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20ffff) AM_RAM
	AM_RANGE(0xc00000, 0xc3ffff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x503000, 0x5031ff) AM_RAM //int ack ?
ADDRESS_MAP_END

/*

  502100-502102  YM2149?


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( goldngam )

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
	PORT_DIPNAME( 0x0001, 0x0001, "switch 2" )
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

/* Used 1bpp gfx just to see the tiles.
   See tiles 1d30 onward...
*/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,2),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( goldngam )
	GFXDECODE_ENTRY( "maincpu", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/


static MACHINE_CONFIG_START( swisspkr, goldngam_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(swisspkr_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", goldngam_state,  irq2_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(4*8, 43*8-1, 1*8, 37*8-1)  // 312x288
	MCFG_SCREEN_UPDATE_DRIVER(goldngam_state, screen_update_goldngam)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", goldngam)

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(goldngam_state, goldngam)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, MASTER_CLOCK/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( moviecrd, swisspkr )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(moviecrd_map)
MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( swisspkr )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "v2.5_hi.pr", 0x00000, 0x10000, CRC(a7f85661) SHA1(aa307bcfe0dfb07120b9711d65916b8689626b00) )
	ROM_LOAD16_BYTE( "v2.5_lo.pr", 0x00001, 0x10000, CRC(142db5d0) SHA1(cc6481a206ed1b0f19cccaab7d6158e81e483c9b) )

	ROM_REGION( 0x40000, "gfx1", 0 )    /* The following ROMs have code for 'Super Cherry' */
	ROM_LOAD16_BYTE( "v1.0_hi.gr", 0x20000, 0x10000, CRC(ea750ab1) SHA1(d1284e7f2628c3aa3de9246e475d45e6be48890e) )
	ROM_LOAD16_BYTE( "v1.0_lo.gr", 0x20001, 0x10000, CRC(d885b965) SHA1(5f2ae3e21cf4e0d20c99cec2dfd3a6f72358535a) )
ROM_END

ROM_START( moviecrd )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "v1.2_hi.pro", 0x20000, 0x20000, CRC(2210dd79) SHA1(007e9930965d6281889fca487f00a1edaac54d85) )
	ROM_CONTINUE(                   0x00000, 0x20000)
	ROM_LOAD16_BYTE( "v1.2_lo.pro", 0x20001, 0x20000, CRC(adb6060f) SHA1(1aff8be7830f61f97720e773eab7985956c7569d) )
	ROM_CONTINUE(                   0x00001, 0x20000)

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "v1.2_hi.gfx", 0x00000, 0x10000, CRC(0b62d1a8) SHA1(4156379cc000cbea997b1c21cebea9021fa697b2) )
	ROM_LOAD16_BYTE( "v1.2_lo.gfx", 0x00001, 0x10000, CRC(70e8e9d5) SHA1(c026493b4bd302d389219ba564aafa42fca86491) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT    MACHINE    INPUT      INIT  ROT    COMPANY                           FULLNAME                          FLAGS */
GAME( 1990, swisspkr, 0,        swisspkr,  goldngam, driver_device,  0,    ROT0, "Golden Games / C+M Technics AG", "Swiss Poker ('50 SG-.10', V2.5)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1998, moviecrd, 0,        moviecrd,  goldngam, driver_device,  0,    ROT0, "Golden Games / C+M Technics AG", "Movie Card",                      MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
