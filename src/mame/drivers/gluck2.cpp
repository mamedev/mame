// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  Good Luck II
  Yung Yu / CYE, 1992.


  Driver by Roberto Fresca.


  Hardware based on Golden Poker / Cal Omega, but with a lot of improvements...


*******************************************************************************


  *** Hardware notes ***

  - CPU:  1x 6502 (U8). Empty socket.

  - CRTC: 1x HD6845SP (U13)

  - RAM:  2x 6116. 2K*8 SRAM. (U6, U17). Empty sockets.

  - ROMs: 1x ST27C512 (U7)
          3x M27C256 (U31, U32, U33)

  - PROMs: 3x Bipolar PROMs (U25, U26, U27)

  - CLK:  1x crystal @ 10.000 MHz. (for CPU clock)
          1x crystal @ 3.5795 MHz. (for sound circuitry)

  - SOUND: 1x AY-3-8910 (U38)
           1x UM3567 (clone of Yamaha YM2413) (U36)
           1x 2904D JRC (Dual Operational Amplifier) (U40)


  Other components:

  - 1x battery (not present)
  - 1x switch (S1)
  - 3x 8 DIP switches banks (DIP1, DIP2, DIP3)
  - 1x 10x2 edge connector.
  - 1x 22x2 edge connector.

  PCB silkscreened: "YUNGYU" "920210" "Made in Taiwan"
  also has a chinese copyright string silkscreened backwards...

  Original stickers from CYE (Chang Yu Electronic Company) in PCB and ROMs.


  PCB Layout:

  .----------------------------------------------------------------------------------------------.
  |  U12                                U8                           U2            BT1           |
  | .-------------------------.        .-------------------------.  .----------.  .-------.      |
  | |                         |        |                         |  |   ????   |  |       |      |
  | |        HD6845SP         |        |   6502 (empty socket)   |  '----------'  |BATTERY|  .---'
  | |                         |        |                         | X1             |       |  |
  | '-------------------------'        '-------------------------'.------.        '-------'  |
  |  U18                 U14            U7                        |10MHz.|        .-----.    |
  | .------------.      .----------.   .------------------.       '------'        |     |    |
  | |  74LS245B  |      |HD74LS157P|   |4CYE              |                       | SW1 ===O '---.
  | '------------'      '----------'   |     ST27C512     |                       |     |     ---|
  |  U17                               |                  |                       '-----'     ---|
  | .----------------.                 '------------------'                                   ---|
  | |      6116      |   U15              U6                                     U3           ---|
  ! | (empty socket) |  .----------.     .----------------.                     .----------.  ---|
  | |                |  |HD74LS157P|     |      6116      |                     | 74HC04AP |  ---|
  | '----------------'  '----------'     | (empty socket) |                     '----------'  ---|
  |                      U16             |                |                      U46          ---|
  |                     .----------.     '----------------'                     .----------.  ---|
  |                     |HD74LS157P|                                            | 74LS08?  | .---'
  |                     '----------'                                            '----------' |
  |  U21                 U9             U5                      U45                          |
  | .------------.      .----------.   .----------------.      .----------.                  |
  | |  SN74S86N  |      |HD74LS08P |   |   GAL22V16?    |      |SN74LS273N|                  |
  | '------------'      '----------'   '----------------'      '----------'                  |
  |  U20                 U10            U4                      U44                          |
  | .------------.      .----------.   .--------------.        .----------.                  '---.
  | |    ???     |      |HD74LS32P |   |     ????     |        |HD74LS244P|                   ---|
  | '------------'      '----------'   '--------------'        '----------'                   ---|
  |  U1                  U22            U11                     U43                           ---|
  | .------------.      .----------.   .------------.          .----------.                   ---|
  | |    ???     |      |HD74LS04P |   |  BU74HC00  |          |T74LS245B1|                   ---|
  | '------------'      '----------'   '------------'          '----------'                   ---|
  |  U24                 U47            U35                     U42                           ---|
  | .------------.      .----------.   .------------.          .----------.                   ---|
  | | GD74LS174  |      | HD74LS02 |   | GD74LS174  |          |T74LS245B1|                   ---|
  | '------------'      '----------'   '------------'          '----------'                   ---|
  |  U12                 U25            U34                     U41                           ---|
  | .------------.      .----------.   .------------.          .----------.                   ---|
  | | SN74LS273N |      |   BP?    |   | GD74LS174  |          |T74LS245B1|                   ---|
  | '------------'      '----------'   '------------'          '----------'                   ---|
  |                      U26            U36                                                   ---|
  |                     .----------.   .---------------.                                      ---|
  |  U33                |   BP?    |   |    UM3567     |                                      ---|
  | .----------------.  '----------'   |               |                                      ---|
  | |3               |   U27           |   (YM2413)    |                                      ---|
  | |     M27256     |  .----------.   '---------------'                                      ---|
  | |                |  |   BP?    |    U37           X2                                      ---|
  | '----------------'  '----------'   .----------.  .------.                                .---'
  |  U32                 U30           |T74LS245B1|  |3.5795|                                |
  | .----------------.  .----------.   '----------'  | MHz. |                U40             |
  | |2               |  |GD74LS166 |                 '------'               .-----.          |
  | |     M27256     |  '----------'    U38                                 |2904D|          |
  | |                |   U29           .-------------------------.          | JRC |          '---.
  | '----------------'  .----------.   |                         |          '-----'              |
  |  U31                |GD74LS166 |   | AY-3-8910 (empty socket)|                               |
  | .----------------.  '----------'   |                         |                               |
  | |1               |   U28           '-------------------------'                               |
  | |     M27256     |  .----------.    DIP3       DIP2       DIP1                               |
  | |                |  |   ???    |   .--------. .--------. .--------.         .-||||||||-.     |
  | '----------------'  '----------'   |oooooooo| |oooooooo| |oooooooo| .---.   |   NEC    |     |
  |                                    '--------' '--------' '--------' |VR1|   |          |     |
  |                                                                     '---'   '----------'     |
  '----------------------------------------------------------------------------------------------'

  Need ID:

  U2 - U4 - U5 - U20 - U25 - U26 - U27 - U28 - U43 - U46 - NEC (bottom right)


*******************************************************************************


  Memory Map
  ----------

  $0000 - $00FF    RAM    ; Zero page (pointers and registers) (NVRAM).
  $0100 - $01FF    RAM    ; 6502 Stack Pointer.
  $0200 - $07FF    RAM    ; General purpose RAM.

  $0800 - $0801    MC6845 ; MC6845 use $0800 for register addressing and $0801 for register values.

  $0844 - $0847    PIA leftover.  ; Code access these offsets like if a PIA 6821 exist there...
  $0848 - $084B    PIA leftover.  ; Code access these offsets like if a PIA 6821 exist there...

  $1000 - $13FF    Video RAM.
  $1800 - $1BFF    Color RAM.

  $2000 - $2000    DIP switches bank #1.

  $2D00 - $2D00    YM2413 Address.
  $2D01 - $2D01    YM2413 Register.

  $3400 - $3400    Input Port #1. \
  $3500 - $3500    Input Port #2.  | General input system.
  $3600 - $3600    Input Port #3. /

  $3700 - $3700    Output Port #1 (coin/note counters).

  $3D00 - $3D00    AY-3-8910 Address.
  $3D01 - $3D01    AY-3-8910 Register.
  $3D01 - $3D01    AY-3-8910 Read (Ports A & B for DIP switches banks #2 & #3).

  $4000 - $FFFF    ROM space.  ; Program ROMs.


*******************************************************************************

  Technical Notes...

  The graphics system is composed of 8 graphics banks of 256 tiles each.
  Tiles are 3bpp, and the color information is given by 3 bipolar PROMs
  (one for each RGB channel).

  There is a AY-3-8910, but only for input port purposes (ports A & B for
  DIP switches banks #3 and #2). Is not used as sound device, at least for
  Good Luck II.

  There are pieces of code that initialize 2x PIA 6821, that are not included
  in the PCB. Seems a leftover. Also the program try to handle a suppossed lamps
  system through these 'phantom' PIAs...

  The sound system is through a YM2413, and sound a lot nicer than the former
  games from these series, improving the ingame sounds with cool nice short tunes.


*******************************************************************************


  TODO:

  - Figure out the remaining DIP switches.
  - Nothing at all... :)


*******************************************************************************/


#define MASTER_CLOCK    XTAL_10MHz
#define SND_CLOCK       XTAL_3_579545MHz

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/2413intf.h"
#include "video/mc6845.h"
#include "machine/nvram.h"


class gluck2_state : public driver_device
{
public:
	gluck2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram") { }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	tilemap_t *m_bg_tilemap;

	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(counters_w);
	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void video_start();
	DECLARE_PALETTE_INIT(gluck2);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*********************************************
*               Video Hardware               *
*********************************************/


WRITE8_MEMBER(gluck2_state::videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

WRITE8_MEMBER(gluck2_state::colorram_w)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(gluck2_state::get_tile_info)
{
/*  - bits -
    7654 3210
    --xx xx--   tiles color.
    xx-- --x-   tiles bank.
    ---- ---x   seems unused.
*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
	int bank = ((attr & 0xc0) >> 5 ) + ((attr & 0x02) >> 1 );   /* bits 1-6-7 handle the gfx banks */
	int color = (attr & 0x3c) >> 2;                             /* bits 2-3-4-5 handle the color */

	SET_TILE_INFO_MEMBER(bank, code, color, 0);
}


void gluck2_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(gluck2_state::get_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 gluck2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


PALETTE_INIT_MEMBER(gluck2_state, gluck2)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0; i < 0x100; i++)
	{
		int bit0, bit1, bit2, bit3;
		int r, g, b;

		/* red component */
		bit0 = (color_prom[i + 0x000] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x000] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x000] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x000] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* green component */
		bit0 = (color_prom[i + 0x100] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x100] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x100] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		/* blue component */
		bit0 = (color_prom[i + 0x200] >> 0) & 0x01;
		bit1 = (color_prom[i + 0x200] >> 1) & 0x01;
		bit2 = (color_prom[i + 0x200] >> 2) & 0x01;
		bit3 = (color_prom[i + 0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_pen_color(i, rgb_t(r, g, b));

	}
}


/**********************************************
*                R/W Handlers                 *
**********************************************/

WRITE8_MEMBER(gluck2_state::counters_w)
{
/*  - bits -
    7654 3210
    ---- --x-   notes.
    ---- -x--   payout.
    ---x ----   coins.
    xxx- x--x   seems unused...

*/
	data = data ^ 0xff; // inverted

	coin_counter_w(machine(), 0, data & 0x10);  /* coins */
	coin_counter_w(machine(), 1, data & 0x02);  /* notes */
	coin_counter_w(machine(), 2, data & 0x04);  /* payout */
}


/*********************************************
*           Memory map information           *
*********************************************/

static ADDRESS_MAP_START( gluck2_map, AS_PROGRAM, 8, gluck2_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x0800, 0x0800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x0801, 0x0801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x0844, 0x084b) AM_NOP /* see below */
	AM_RANGE(0x1000, 0x13ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram") /* 6116 #1 (2K x 8) RAM (only 1st half used) */
	AM_RANGE(0x1800, 0x1bff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram") /* 6116 #2 (2K x 8) RAM (only 1st half used) */
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("SW1")
	AM_RANGE(0x2d00, 0x2d01) AM_DEVWRITE("ymsnd", ym2413_device, write)
	AM_RANGE(0x3400, 0x3400) AM_READ_PORT("IN0")
	AM_RANGE(0x3500, 0x3500) AM_READ_PORT("IN1")
	AM_RANGE(0x3600, 0x3600) AM_READ_PORT("IN2")
	AM_RANGE(0x3700, 0x3700) AM_WRITE(counters_w )
	AM_RANGE(0x3d00, 0x3d01) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, address_data_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*
  0844-0847    PIA0 leftover???
  0848-084b    PIA1 leftover???

*/

/*********************************************
*                Input ports                 *
*********************************************/

static INPUT_PORTS_START( gluck2 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  PORT_NAME("Note In")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Service")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")   // 2000
	PORT_DIPNAME( 0x01, 0x01, "Paytable" )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "Strings and Numbers" )
	PORT_DIPSETTING(    0x00, "Only Numbers" )
	PORT_DIPNAME( 0x02, 0x02, "SW1:2" )         PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "SW1:3" )         PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "SW1:4" )         PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "SW1:5" )         PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "SW1:6" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "SW1:7" )         PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "SW1:8" )         PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")   // 3D01: AY8910 port B
	PORT_DIPNAME( 0x01, 0x01, "SW2:8" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, "Bet Max" )       PORT_DIPLOCATION("SW2:7, 6")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x02, "20" )
	PORT_DIPSETTING(    0x04, "30" )
	PORT_DIPSETTING(    0x06, "40" )
	PORT_DIPNAME( 0x08, 0x08, "SW2:5" )         PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "SW2:4" )         PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "SW2:3" )         PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Note In" )       PORT_DIPLOCATION("SW2:2, 1")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0x80, "50" )
	PORT_DIPSETTING(    0xc0, "100" )

	PORT_START("SW3")   // 3D01: AY8910 port A
	PORT_DIPNAME( 0x01, 0x01, "SW3:1" )         PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "SW3:8" )         PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Graphics" )      PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x04, "Turtles" )
	PORT_DIPSETTING(    0x00, "Cards" )
	PORT_DIPNAME( 0x18, 0x18, "Coin In" )       PORT_DIPLOCATION("SW3:6, 5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x18, "10" )
	PORT_DIPNAME( 0x20, 0x20, "SW3:4" )         PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "SW3:3" )         PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "SW3:2" )         PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	256,    // 0x100 tiles per bank.
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gluck2 )
	GFXDECODE_ENTRY( "gfx", 0x0000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x0800, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x1000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x1800, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x2000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x2800, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x3000, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx", 0x3800, tilelayout, 0, 16 )
GFXDECODE_END

/*********************************************
*              Machine Drivers               *
*********************************************/

static MACHINE_CONFIG_START( gluck2, gluck2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/16) /* guess */
	MCFG_CPU_PROGRAM_MAP(gluck2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gluck2_state,  nmi_line_pulse)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))

/* CRTC Register:  00   01   02   03   04   05   06
   CRTC Value   : 0x27 0x20 0x23 0x03 0x26 0x00 0x20
*/
	MCFG_SCREEN_SIZE((39+1)*8, (38+1)*8)                /* from MC6845 init, registers 00 & 04. (value - 1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)  /* from MC6845 init, registers 01 & 06. */
	MCFG_SCREEN_UPDATE_DRIVER(gluck2_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", gluck2)
	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_PALETTE_INIT_OWNER(gluck2_state, gluck2)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/16) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)    /* guess */
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("SW3"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("SW2"))
/*  Output ports have a minimal activity during init.
    They seems unused (at least for Good Luck II)
*/
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ymsnd", YM2413, SND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( gluck2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.u7",    0x00000, 0x10000, CRC(09e7a220) SHA1(1be7ba3eb864da097f9e45a80f2db61ff0e9bc0b) )

	ROM_REGION( 0x18000, "gfx", 0 )
	ROM_LOAD( "3.u33",  0x00000, 0x8000, CRC(f752f0b2) SHA1(7bec624282c74d6f88815cc28cf5a58f6e3ce2dd) )
	ROM_LOAD( "2.u32",  0x08000, 0x8000, CRC(6a621a98) SHA1(9c83eab9f0858735e0176e5335651dd2dc620229) )
	ROM_LOAD( "1.u31",  0x10000, 0x8000, CRC(ea33db1a) SHA1(69c67944f5e8bd060335b5e14628c0e0828271a4) )

	ROM_REGION( 0x0300, "proms", 0 )    // RGB
	ROM_LOAD( "v1.u27",  0x0000, 0x0100, CRC(1aa5479f) SHA1(246cc99e7b351d5546060807b8a0b8acfe2f8e39) )
	ROM_LOAD( "v2.u26",  0x0100, 0x0100, CRC(8da53489) SHA1(b90f5dd4bc5b64009e8bfad8f79f23d4020e537b) )
	ROM_LOAD( "v3.u25",  0x0200, 0x0100, CRC(a4d2c9c3) SHA1(a799875b8b92391696419081244da2e56216e024) )
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE          INIT   ROT    COMPANY          FULLNAME       FLAGS... */
GAME( 1992, gluck2,   0,      gluck2,   gluck2,   driver_device, 0,     ROT0, "Yung Yu / CYE", "Good Luck II", MACHINE_SUPPORTS_SAVE )
