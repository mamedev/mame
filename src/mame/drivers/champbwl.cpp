// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Championship Bowling
Romstar Inc., 1989

Driver by Pierpaolo Prazzoli

To Do:
  Hook up player 2 controls for cocktail mode

-----------------------------------------------------------

This game runs on Seta Hardware.

PCB Layout
----------

P0-052A
|---------------------------------------------------------|
| MB3712     SW1        AB001009  AB001007  AB001005      |
|                 X1-007                                  |
|        VOL                          AB001006  AB001004  |
|                                                         |
|                       AB001008                          |
|                                                         |
|        4050                                             |
|                                                         |
|      CN1                             X1-002A            |
|                                                         |
|J                                                3V_BATT |
|A                        2063         X1-001A            |
|M                                                        |
|M                                                  SW2   |
|A         16MHz                                          |
|                            4520                         |
|                                                         |
|                                                  6116   |
|       X1-010                                            |
|                                                         |
|      |---------------ROM-Sub-Board----------------|     |
|  3404|                                            |     |
|      |    DIP32                                   |     |
|      |                                          AB001001|
|      |          6116                       Z80    |     |
|      |                                            |     |
|      |                                            |     |
|------|--------------------------------------------|-----|


ROM Sub Board (plugs into DIP32 socket on main board)
-------------

P0-047A
|--------------------------------------------|
|                                            |
|     AB003003           AB002003            |
|                                            |
|              AB003002            AB002002  |
|                                            |
| 74HC139                                    |
|--------------------------------------------|

Notes:
      Z80 clock     - 4.000MHz [16/4]
      X1-010 clocks - pin1 16.000MHz, pin2 8.000MHz, pin79 4.000MHz, pin80 2.000MHz
      VSync         - 57.5Hz
      2063          - Toshiba TMM2063AP-10 8K x8 SRAM (DIP28)
      6116          - Hitachi 6116LP-2 2K x8 SRAM (DIP24)
      MB3712        - Fujitsu MB3712 5.7 Watt Power AMP (SIP8)
      4050          - Toshiba TC4050BP Non-Inverting Hex Buffer / Converter (DIP16)
      4520          - Hitachi HD14520 Dual Up Counter (DIP16)
      3404          - Japan Radio Co. JRC3404 Low Power Quad Op AMP (DIP8)
      SW1           - Reset Switch
      SW2           - 2-Position Switch (when ON, powers 6116 RAM near it)
      3V_BATT       - Sony CR2032 3V Lithium Coin Battery
      DIP32         - Empty DIP32 socket for connection of ROM Sub Board
      CN1           - 4-pin Connector

      Custom IC's -
                    X1-001A (SDIP64) \ Sprite Generators
                    X1-002A (SDIP64) /
                    X1-007  (SDIP42)   Video DAC? (connected to RGB output)
                    X1-010  (QFP80)    Sound Chip, 16Bit PCM

      ROMs -
            Filename        Type                Use
            -----------------------------------------------
            AB001001.U1    27C1000 (DIP32)      Z80 Program

            AB001004.U7    512K MaskROM (DIP28) \
            AB001005.U9    512K MaskROM (DIP28) | GFX
            AB001006.U15   512K MaskROM (DIP28) |
            AB001007.U22   512K MaskROM (DIP28) /

            AB001008.U26   82S147 PROM (DIP20)
            AB001009.U27   82S147 PROM (DIP20)

            AB002002.2-2   2M MaskROM (DIP32)   \
            AB002003.2-3   2M MaskROM (DIP32)   | PCM Samples (Connected to X1-010 via a sub-board)
            AB003002.3-2   2M MaskROM (DIP32)   |
            AB003003.3-3   2M MaskROM (DIP32)   /



                          Main Jamma Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5V             | C | 3 |             +5V
             +5V             | D | 4 |             +5V
             -5V             | E | 5 |             -5V
             +12V            | F | 6 |             +12V
------------ KEY ------------| H | 7 |------------ KEY -----------
       Coin Counter 2        | J | 8 |      Coin Counter 1
       Coin Lock Out 2       | K | 9 |      Coin Lock Out 1
         Speaker (-)         | L | 10|        Speaker (+)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
        Service SW           | R | 14|        Video GND
        Tilt Switch          | S | 15|        Test Switch
        Coin Switch 2        | T | 16|        Coin Switch 1
        Player 2 Start       | U | 17|        Player 1 Start
  #2 Trackball X Direction*  | V | 18|      #1 Trackball X Direction
  #2 Trackball Y Direction*  | W | 19|      #1 Trackball Y Direction
  #2 Trackball X Clock*      | X | 20|      #1 Trackball X Clock
  #2 Trackball Y Clock*      | Y | 21|      #1 Trackball Y Clock
  Player 2 Hook Right*       | Z | 22|      Player 1 Hook Right
  Player 2 Hook Left*        | a | 23|      Player 1 Hook Left
                             | b | 24|        Player Select
                             | c | 25|
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND

* Cocktail Mode only.

 Standard 6 pin Trackball connector

  Pin  Wire  Funtion
------------------------------
   1 | BLK | Ground
   2 | RED | +5 Volts DC
   3 | YEL | Y Clock
   4 | GRN | Y Direction
   5 | BLU | X Direction
   6 | PUR | X Clock


*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/x1_010.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "video/seta001.h"

class champbwl_state : public driver_device
{
public:
	champbwl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_seta001(*this, "spritegen"),
		m_palette(*this, "palette"),
		m_x1(*this, "x1snd") { }

	int      m_screenflip;

	required_device<cpu_device> m_maincpu;
	required_device<seta001_device> m_seta001;
	required_device<palette_device> m_palette;
	required_device<x1_010_device> m_x1;
	UINT8    m_last_trackball_val[2];
	DECLARE_READ8_MEMBER(trackball_r);
	DECLARE_WRITE8_MEMBER(champbwl_misc_w);
	DECLARE_WRITE8_MEMBER(doraemon_outputs_w);
	DECLARE_MACHINE_START(champbwl);
	DECLARE_MACHINE_RESET(champbwl);
	DECLARE_MACHINE_START(doraemon);
	DECLARE_PALETTE_INIT(champbwl);
	UINT32 screen_update_champbwl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_doraemon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_champbwl(screen_device &screen, bool state);
	void screen_eof_doraemon(screen_device &screen, bool state);
};

PALETTE_INIT_MEMBER(champbwl_state,champbwl)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i, col;

	for (i = 0; i < palette.entries(); i++)
	{
		col = (color_prom[i] << 8) + color_prom[i + 512];
		palette.set_pen_color(i, pal5bit(col >> 10), pal5bit(col >> 5), pal5bit(col >> 0));
	}
}


READ8_MEMBER(champbwl_state::trackball_r)
{
	UINT8 ret;
	UINT8 port4 = ioport("FAKEX")->read();
	UINT8 port5 = ioport("FAKEY")->read();

	ret = (((port4 - m_last_trackball_val[0]) & 0x0f)<<4) | ((port5 - m_last_trackball_val[1]) & 0x0f);

	m_last_trackball_val[0] = port4;
	m_last_trackball_val[1] = port5;

	return ret;
}

WRITE8_MEMBER(champbwl_state::champbwl_misc_w)
{
	coin_counter_w(machine(), 0, data & 1);
	coin_counter_w(machine(), 1, data & 2);

	coin_lockout_w(machine(), 0, ~data & 8);
	coin_lockout_w(machine(), 1, ~data & 4);

	membank("bank1")->set_entry((data & 0x30) >> 4);
}

static ADDRESS_MAP_START( champbwl_map, AS_PROGRAM, 8, champbwl_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xafff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodelow_r8, spritecodelow_w8)
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodehigh_r8, spritecodehigh_w8)
	AM_RANGE(0xc000, 0xdfff) AM_DEVREADWRITE("x1snd", x1_010_device, read, write)
	AM_RANGE(0xe000, 0xe2ff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spriteylow_r8, spriteylow_w8)
	AM_RANGE(0xe300, 0xe303) AM_MIRROR(0xfc) AM_DEVWRITE("spritegen", seta001_device, spritectrl_w8) /* control registers (0x80 mirror used by Arkanoid 2) */
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE("spritegen", seta001_device, spritebgflag_w8)   /* enable / disable background transparency */

	AM_RANGE(0xf000, 0xf000) AM_READ(trackball_r)
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("IN0")
	AM_RANGE(0xf004, 0xf004) AM_READ_PORT("IN1")
	AM_RANGE(0xf006, 0xf006) AM_READ_PORT("IN2")
	AM_RANGE(0xf007, 0xf007) AM_READ_PORT("IN3")

	AM_RANGE(0xf000, 0xf000) AM_WRITE(champbwl_misc_w)
	AM_RANGE(0xf002, 0xf002) AM_WRITENOP //buttons light?
	AM_RANGE(0xf004, 0xf004) AM_WRITENOP //buttons light?
	AM_RANGE(0xf006, 0xf006) AM_WRITENOP //buttons light?
	AM_RANGE(0xf800, 0xf800) AM_WRITENOP
ADDRESS_MAP_END



WRITE8_MEMBER(champbwl_state::doraemon_outputs_w)
{
	coin_counter_w(machine(), 0, data & 1); // coin in counter
	coin_counter_w(machine(), 1, data & 2); // gift out counter

	coin_lockout_w(machine(), 0, ~data & 8);    // coin lockout
	machine().device<ticket_dispenser_device>("hopper")->write(space, 0, (data & 0x04) ? 0x00 : 0x80);  // gift out motor

	membank("bank1")->set_entry((data & 0x30) >> 4);

//  popmessage("%02x", data);
}

static ADDRESS_MAP_START( doraemon, AS_PROGRAM, 8, champbwl_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x87ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xafff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodelow_r8, spritecodelow_w8)
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spritecodehigh_r8, spritecodehigh_w8)
	AM_RANGE(0xc000, 0xc07f) AM_DEVREADWRITE("x1snd", x1_010_device, read, write) // Sound
	AM_RANGE(0xe000, 0xe2ff) AM_RAM AM_DEVREADWRITE("spritegen", seta001_device, spriteylow_r8, spriteylow_w8)
	AM_RANGE(0xe300, 0xe303) AM_DEVWRITE("spritegen", seta001_device, spritectrl_w8)
	AM_RANGE(0xe800, 0xe800) AM_DEVWRITE("spritegen", seta001_device, spritebgflag_w8)   /* enable / disable background transparency */
	AM_RANGE(0xf000, 0xf000) AM_READ_PORT("IN0") AM_WRITE(doraemon_outputs_w)
	AM_RANGE(0xf002, 0xf002) AM_READ_PORT("IN1") AM_WRITENOP    // Ack?
	AM_RANGE(0xf004, 0xf004) AM_WRITENOP                        // Ack?
	AM_RANGE(0xf006, 0xf006) AM_READ_PORT("DSW") AM_WRITENOP    // Ack?
	AM_RANGE(0xf800, 0xf800) AM_WRITENOP                        // 0
ADDRESS_MAP_END



static INPUT_PORTS_START( champbwl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Player Change")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) // INT( 4M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL ) // INT(16M)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("IN2")
	PORT_SERVICE_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "License" ) PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "Romstar (1)")
	PORT_DIPSETTING(    0x04, "Romstar (2)")
	PORT_DIPSETTING(    0x08, "Seta U.S.A, Romstar License" )
	PORT_DIPSETTING(    0x0c, "Seta" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("FAKEX")     /* FAKE */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X )PORT_SENSITIVITY(50) PORT_KEYDELTA(50) PORT_CENTERDELTA(0)

	PORT_START("FAKEY")     /* FAKE */
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(45) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END



static INPUT_PORTS_START( doraemon )
	PORT_START("DSW")   // f006
	PORT_DIPNAME( 0x0f, 0x09, "Gift Out" )
	PORT_DIPSETTING(    0x0f,   "2 %" )
	PORT_DIPSETTING(    0x0e,   "5 %" )
	PORT_DIPSETTING(    0x0d,   "7 %" )
	PORT_DIPSETTING(    0x0c,  "10 %" )
	PORT_DIPSETTING(    0x0b,  "12 %" )
	PORT_DIPSETTING(    0x0a,  "15 %" )
	PORT_DIPSETTING(    0x09,  "20 %" )
	PORT_DIPSETTING(    0x08,  "25 %" )
	PORT_DIPSETTING(    0x07,  "30 %" )
	PORT_DIPSETTING(    0x06,  "40 %" )
	PORT_DIPSETTING(    0x05,  "60 %" )
	PORT_DIPSETTING(    0x04,  "80 %" )
//  PORT_DIPSETTING(    0x03, "100 %" )
//  PORT_DIPSETTING(    0x02, "100 %" )
//  PORT_DIPSETTING(    0x01, "100 %" )
	PORT_DIPSETTING(    0x00, "100 %" )
	PORT_DIPNAME( 0x10, 0x10, "Games For 100 Yen" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")   // f000
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE2 )  PORT_NAME( "Data Clear" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_COIN1    )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNKNOWN  )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE3 )  PORT_NAME( "Freeze" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNKNOWN  )

	PORT_START("IN1")   // f002
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3  )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SPECIAL  ) PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)   // sensor
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8
};

static GFXDECODE_START( champbwl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 32 )
GFXDECODE_END

MACHINE_START_MEMBER(champbwl_state,champbwl)
{
	UINT8 *ROM = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);

	save_item(NAME(m_screenflip));
	save_item(NAME(m_last_trackball_val));
}

MACHINE_RESET_MEMBER(champbwl_state,champbwl)
{
	m_screenflip = 0;
	m_last_trackball_val[0] = 0;
	m_last_trackball_val[1] = 0;

}

UINT32 champbwl_state::screen_update_champbwl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->set_fg_yoffsets( -0x12, 0x0e );
	m_seta001->set_bg_yoffsets( 0x1, -0x1 );

	m_seta001->draw_sprites(screen, bitmap, cliprect, 0x800, 1 );
	return 0;
}

void champbwl_state::screen_eof_champbwl(screen_device &screen, bool state)
{
	// rising edge
	if (state)
		m_seta001->tnzs_eof();
}


static MACHINE_CONFIG_START( champbwl, champbwl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 16000000/4) /* 4MHz */
	MCFG_CPU_PROGRAM_MAP(champbwl_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbwl_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_MACHINE_START_OVERRIDE(champbwl_state,champbwl)
	MCFG_MACHINE_RESET_OVERRIDE(champbwl_state,champbwl)

	MCFG_DEVICE_ADD("spritegen", SETA001_SPRITE, 0)
	MCFG_SETA001_SPRITE_GFXDECODE("gfxdecode")
	MCFG_SETA001_SPRITE_PALETTE("palette")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.5)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 48*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbwl_state, screen_update_champbwl)
	MCFG_SCREEN_VBLANK_DRIVER(champbwl_state, screen_eof_champbwl)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", champbwl)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_PALETTE_INIT_OWNER(champbwl_state,champbwl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("x1snd", X1_010, 16000000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END




UINT32 champbwl_state::screen_update_doraemon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0x1f0, cliprect);

	m_seta001->set_bg_yoffsets( 0x00, 0x01 );
	m_seta001->set_fg_yoffsets( 0x00, 0x10 );

	m_seta001->draw_sprites(screen, bitmap, cliprect, 0x800, 1 );
	return 0;
}

void champbwl_state::screen_eof_doraemon(screen_device &screen, bool state)
{
	// rising edge
	if (state)
		m_seta001->setac_eof();
}

MACHINE_START_MEMBER(champbwl_state,doraemon)
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 4, &ROM[0x10000], 0x4000);
}

static MACHINE_CONFIG_START( doraemon, champbwl_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_14_31818MHz/4)
	MCFG_CPU_PROGRAM_MAP(doraemon)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", champbwl_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_DEVICE_ADD("spritegen", SETA001_SPRITE, 0)
	MCFG_SETA001_SPRITE_GFXDECODE("gfxdecode")
	MCFG_SETA001_SPRITE_PALETTE("palette")
	MCFG_TICKET_DISPENSER_ADD("hopper", attotime::from_msec(2000), TICKET_MOTOR_ACTIVE_LOW, TICKET_STATUS_ACTIVE_LOW )

	MCFG_MACHINE_START_OVERRIDE(champbwl_state,doraemon)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(champbwl_state, screen_update_doraemon)
	MCFG_SCREEN_VBLANK_DRIVER(champbwl_state, screen_eof_doraemon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", champbwl)
	MCFG_PALETTE_ADD("palette", 512)

	MCFG_PALETTE_INIT_OWNER(champbwl_state,champbwl)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("x1snd", X1_010, XTAL_14_31818MHz)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END



ROM_START( champbwl )
	ROM_REGION( 0x20000, "maincpu", 0 )     /* Z80 Code */
	ROM_LOAD( "ab001001.u1",  0x10000, 0x10000, CRC(6c6f7675) SHA1(19834f25f2644ae5d156c1e1bbb3fc50cae10fd2) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "ab001007.u22", 0x00000, 0x20000, CRC(1ee9f6b1) SHA1(1a67e969b1f471ec7ada294b89185c15cde8c1ab) )
	ROM_LOAD( "ab001006.u15", 0x20000, 0x20000, CRC(37baf753) SHA1(efa57d915a9e14393b62b161e1ac807b8fcb8501) )
	ROM_LOAD( "ab001005.u9",  0x40000, 0x20000, CRC(b80a9ed6) SHA1(ac7a31ad82a60c4d2034770c59cf383b8a036e6a) )
	ROM_LOAD( "ab001004.u7",  0x60000, 0x20000, CRC(584477b1) SHA1(296f96526044e9bd13673e5d817260e3f98f696c) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "ab001008.u26", 0x0000, 0x0200, CRC(30ac8d48) SHA1(af034de3f3b8548534effdf4e3717fe3838b7754) )
	ROM_LOAD( "ab001009.u27", 0x0200, 0x0200, CRC(3bbd4bcd) SHA1(8c87ccc42ece2432b8ad25f8679cdf886e12a43c) )

	ROM_REGION( 0x100000, "x1snd", 0 )  /* Samples */
	ROM_LOAD( "ab002002.2-2", 0x00000, 0x40000, CRC(42ebe997) SHA1(1808b9e5e996a395c1d48ac001067f736f96feec) ) // jingles (for strike, spare etc.)
	ROM_LOAD( "ab003002.3-2", 0x40000, 0x40000, CRC(7ede8f28) SHA1(b5519c09b4f0019dc76cadca725da1d581912540) ) // basic coin + ball sounds
	ROM_LOAD( "ab002003.2-3", 0x80000, 0x40000, CRC(3051b8c3) SHA1(5f53596d7af1c79db1dde4bdca3878e07c67b5d1) ) // 'welcome to.. , strike' speech etc.
	ROM_LOAD( "ab003003.3-3", 0xc0000, 0x40000, CRC(ad40ad10) SHA1(db0e5744ea3fcda87345b545031f82fcb3fec175) ) // 'spare' speech etc.

	ROM_REGION( 0x800, "nvram", 0 ) /* default settings, allows game to boot first time without having to reset it */
	ROM_LOAD( "champbwl.nv",  0x000, 0x800, CRC(1d46aa8e) SHA1(a733cf86cfb26d98fb4c491d7f779a7a1c8ff228) )
ROM_END

/*

Doraemon no Eawase Montage (prototype)
Sunsoft / Epoch / Fujiko - Shogakukan - TV Asahi

This PCB was obtained from a collector who was invited to purchase some inventory when Sunsoft closed its U.S. offices in 2001.
Among the items he acquired was a small JAMMA cabinet with no keys to open its locked door.
When the lock was drilled out, this game was discovered inside the cabinet.

PCB Layout
----------

P0-094A Sun Electronics Corporation
|---------------------------------------------------------|
| MB3712     SW2         U27-01    U22   U15   U9   U7    |
|                 X1-007 U26-01                           |
|      VOL                                                |
|                                                         |
|         DSW2(8)  DSW1(8)                                |
|                                                         |
|        4050                                             |
|                                                         |
|                                      X1-002A            |
|      CN1  X1-004                                        |
|J                                                3V_BATT |
|A                        2063         X1-001A            |
|M                                                        |
|M                                                   SW3  |
|A         14.31818MHz                                    |
|                            4520                         |
|                                                         |
|                                                   2128  |
|       X1-010                                            |
|                                                         |
|                                                         |
| 3404  U51   U43                                         |
|                                                         |
|                                          Z8400AB1   U1  |
|---------------------------------------------------------|

Notes:
      2063          - Toshiba TMM2063AP-10 8K x8 SRAM (DIP28)
      2128          - MSM2128-15RS 2K x8 SRAM (DIP24)
      MB3712        - Fujitsu MB3712 5.7 Watt Power AMP (SIP8)
      4050          - Toshiba TC4050BCP Non-Inverting Hex Buffer / Converter (DIP16)
      4520          - Hitachi HD14520 Dual Up Counter (DIP16)
      3404          - Japan Radio Co. JRC3404 Low Power Quad Op AMP (DIP8)
      SW2           - Reset Switch
      SW3           - 2-Position Switch (when ON, powers 6116 RAM near it)
      3V_BATT       - 3V Lithium Coin Battery
      CN1           - 10-pin Connector

      Custom IC's -
                    X1-001A (SDIP64) \ Sprite Generators
                    X1-002A (SDIP64) /
                    X1-004  (SDIP52)   Inputs [4 wires going to small board with DSWA(10) & DSWB(10)]
                    X1-007  (SDIP42)   Video DAC
                    X1-010  (QFP80)    Sound Chip, 16Bit PCM

      ROMs -
            Filename  Type                          Use
            -----------------------------------------------
            U1        27C1001 UV EEPROM (FDIP32W)   Z80 Program

            U7        27C1001 UV EEPROM (FDIP32W) \
            U9        27C1001 UV EEPROM (FDIP32W) | GFX
            U15       27C1001 UV EEPROM (FDIP32W) |
            U22       27C1001 UV EEPROM (FDIP32W) /

            U26-01    82S147 PROM (DIP20)
            U27-01    82S147 PROM (DIP20)

            U43       27C4001 UV EEPROM (DIP32)   \
            U51       27C4001 UV EEPROM (DIP32)   / PCM Samples
*/

ROM_START( doraemon )
	ROM_REGION( 0x30000, "maincpu", 0 )
	ROM_LOAD( "u1.bin", 0x00000, 0x20000, CRC(d338b9ca) SHA1(5f59c994db81577dc6074362c8b6b93f8fe592f6) )
	ROM_RELOAD(         0x10000, 0x20000 )      /* banked at 4000-7fff */

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "u22.bin", 0x00000, 0x20000, CRC(b264ac2d) SHA1(0529fd1b88ba61dcf72019c7b01e9b939b6e3f2e) )
	ROM_LOAD( "u15.bin", 0x20000, 0x20000, CRC(2985df6f) SHA1(fd2dae7169c14d07beaf870b0b8c248deb6b6c03) )
	ROM_LOAD( "u9.bin",  0x40000, 0x20000, CRC(8b774b0e) SHA1(bfcf63b955f74b226a325b0409167e95c2489134) )
	ROM_LOAD( "u7.bin",  0x60000, 0x20000, CRC(2f850973) SHA1(7dbad160aefaf8b0a85e64f58b9fa0fb4049e65d) )

	ROM_REGION( 0x100000, "x1snd", 0 )
	ROM_LOAD( "u43.bin", 0x00000, 0x80000, CRC(d684d92a) SHA1(935f39e5efb923a8c7cd0caa6fa6b78a5d78ef30) )
	ROM_LOAD( "u51.bin", 0x80000, 0x80000, CRC(35cbcb31) SHA1(4ab59e5d5ba917fa2d809e2dc6216c801d3927e7) )

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "u26-01.bin", 0x00000, 0x200, CRC(9d431542) SHA1(d8895052c5016574f13bf4c096d191534062b9a1) )
	ROM_LOAD( "u27-01.bin", 0x00200, 0x200, CRC(66245fc7) SHA1(c94d9dce7b557c21a3dc1f3f8a1b29594715c994) )
ROM_END

GAME( 1993?,doraemon, 0, doraemon, doraemon, driver_device, 0, ROT0,   "Sunsoft / Epoch", "Doraemon no Eawase Montage (prototype)", MACHINE_SUPPORTS_SAVE ) // year not shown, datecodes on pcb suggests late-1993
GAME( 1989, champbwl, 0, champbwl, champbwl, driver_device, 0, ROT270, "Seta / Romstar Inc.", "Championship Bowling", MACHINE_SUPPORTS_SAVE )
