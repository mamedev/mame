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

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

#define MASTER_CLOCK    XTAL(10'000'000)
#define SND_CLOCK       XTAL(3'579'545)


class gluck2_state : public driver_device
{
public:
	gluck2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram")
	{ }

	void gluck2(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;

	tilemap_t *m_bg_tilemap = nullptr;

	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void counters_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void gluck2_map(address_map &map) ATTR_COLD;
};


/*********************************************
*               Video Hardware               *
*********************************************/


void gluck2_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void gluck2_state::colorram_w(offs_t offset, uint8_t data)
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

	tileinfo.set(bank, code, color, 0);
}


void gluck2_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(gluck2_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t gluck2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/**********************************************
*                R/W Handlers                 *
**********************************************/

void gluck2_state::counters_w(uint8_t data)
{
/*  - bits -
    7654 3210
    ---- --x-   notes.
    ---- -x--   payout.
    ---x ----   coins.
    xxx- x--x   seems unused...

*/
	data = data ^ 0xff; // inverted

	machine().bookkeeping().coin_counter_w(0, data & 0x10);  /* coins */
	machine().bookkeeping().coin_counter_w(1, data & 0x02);  /* notes */
	machine().bookkeeping().coin_counter_w(2, data & 0x04);  /* payout */
}


/*********************************************
*           Memory map information           *
*********************************************/

void gluck2_state::gluck2_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0800).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x0844, 0x084b).noprw(); /* see below */
	map(0x1000, 0x13ff).ram().w(FUNC(gluck2_state::videoram_w)).share("videoram"); /* 6116 #1 (2K x 8) RAM (only 1st half used) */
	map(0x1800, 0x1bff).ram().w(FUNC(gluck2_state::colorram_w)).share("colorram"); /* 6116 #2 (2K x 8) RAM (only 1st half used) */
	map(0x2000, 0x2000).portr("SW1");
	map(0x2d00, 0x2d01).w("ymsnd", FUNC(ym2413_device::write));
	map(0x3400, 0x3400).portr("IN0");
	map(0x3500, 0x3500).portr("IN1");
	map(0x3600, 0x3600).portr("IN2");
	map(0x3700, 0x3700).w(FUNC(gluck2_state::counters_w));
	map(0x3d00, 0x3d01).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0x4000, 0xffff).rom();
}

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

static GFXDECODE_START( gfx_gluck2 )
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

void gluck2_state::gluck2(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, MASTER_CLOCK/16); /* guess */
	m_maincpu->set_addrmap(AS_PROGRAM, &gluck2_state::gluck2_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));

/* CRTC Register:  00   01   02   03   04   05   06
   CRTC Value   : 0x27 0x20 0x23 0x03 0x26 0x00 0x20
*/
	screen.set_size((39+1)*8, (38+1)*8);                /* from MC6845 init, registers 00 & 04. (value - 1) */
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);  /* from MC6845 init, registers 01 & 06. */
	screen.set_screen_update(FUNC(gluck2_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_gluck2);
	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 256);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK/16));    /* guess */
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay8910(AY8910(config, "ay8910", MASTER_CLOCK/8));    /* guess */
	ay8910.port_a_read_callback().set_ioport("SW3");
	ay8910.port_b_read_callback().set_ioport("SW2");
/*  Output ports have a minimal activity during init.
    They seems unused (at least for Good Luck II)
*/
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.0);

	YM2413(config, "ymsnd", SND_CLOCK).add_route(ALL_OUTPUTS, "mono", 1.0);
}


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

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME    PARENT  MACHINE   INPUT   STATE         INIT        ROT    COMPANY          FULLNAME       FLAGS...
GAME( 1992, gluck2, 0,      gluck2,   gluck2, gluck2_state, empty_init, ROT0, "Yung Yu / CYE", "Good Luck II", MACHINE_SUPPORTS_SAVE )
