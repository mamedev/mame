// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  Number One.
  San Remo Games.

  8bit amusement/gambling hardware.


  Driver by Roberto Fresca.


*******************************************************************************

  *** Hardware notes ***

  - CPU:  1x TMPZ84C00AP-6       [IC7]
  - CRTC: 1x MC68B45P            [IC14]
  - SND:  1x WF19054 (AY-3-8910) [IC32]
  - CLK:  1x crystal @ 18.000 MHz.

  - ROM:  1x NM27C256            [IC26]
          4x 27C512              [IC27, IC28, IC29, IC30]

  - RAM:  1x UMC UM6116-3L        (2K x 8 SRAM) (battery backed) [IC15]
          2x GoldStar GN76C28K-10 (2K x 8 SRAM)                  [IC20, IC21]

  - PLDs: 4x unknown PALCE (DIP-20, read protected)  [IC4, IC24, IC25 & IC31]

  - 1x 8 DIP switches bank.

  - 1x 28x2 JAMMA edge connector [CN3]
  - 1x 4 legs connector          [CN2]
  - 1x 5 legs connector          [CN5]
  - 1x trimmer (volume)          [P1]


  Graphics seems to have the insanely amount of 32 banks of 256 tiles each,
  driven by a 5-bit register/selector.


*******************************************************************************

  Game Notes
  ----------

  You can switch the game to use numbers or cards through the Test Mode.
  To enter the Test Mode, just turn the DIP switch 7 ON. No credits should be
  in the machine to get it working...


*******************************************************************************

  Driver updates:


  [2012/01/19]

  - Initial release. Preliminary driver.


  [2012/01/20]

  - Graphics decode.
  - Proper ROM load.
  - Memory Map.
  - Hooked CPU & interrupts.
  - Added CRTC support.
  - Added AY-3-8910 support.
  - Added input ports.
  - Added output-lamps port.
  - Added Button-Lamps layout.
  - Complete lamps support.
  - NVRAM support.
  - Correct GFX banking.

  [2012/02/03]

  - Fixed graphics issues:
    Latched the bank bits and stored them when the tiles are written.
  - Hooked up intensity layer.
  - Removed the imperfect gfx and game not working flags.


  TODO:

  - Figure out activity in some output ports.


*******************************************************************************/


#define MASTER_CLOCK    XTAL_18MHz

#define CPU_CLOCK       MASTER_CLOCK/3
#define SND_CLOCK       MASTER_CLOCK/12
#define CRTC_CLOCK      MASTER_CLOCK/12

#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "sanremo.lh"


class sanremo_state : public driver_device
{
public:
	sanremo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;

	UINT8 m_attrram[0x800];
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(sanremo_videoram_w);
	TILE_GET_INFO_MEMBER(get_sanremo_tile_info);
	DECLARE_WRITE8_MEMBER(banksel_w);
	DECLARE_WRITE8_MEMBER(lamps_w);
	int banksel;
	virtual void video_start();
	DECLARE_PALETTE_INIT(sanremo);
	UINT32 screen_update_sanremo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*********************************************
*               Video Hardware               *
*********************************************/


WRITE8_MEMBER(sanremo_state::sanremo_videoram_w)
{
	m_videoram[offset] = data;
	m_attrram[offset] = banksel;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(sanremo_state::get_sanremo_tile_info)
{
	int code = m_videoram[tile_index];
	int bank = m_attrram[tile_index];

	SET_TILE_INFO_MEMBER(0, code + bank * 256, 0, 0);
}

void sanremo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(sanremo_state::get_sanremo_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 48, 40);

}

UINT32 sanremo_state::screen_update_sanremo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT_MEMBER(sanremo_state, sanremo)
{
	int index;

	for (index = 0; index < 0x8; index++)
		palette.set_pen_color(index, rgb_t(pal1bit((index >> 0)&1), pal1bit((index >> 1)&1), pal1bit((index >> 2)&1)));

	for (index = 0x8; index < 0x10; index++)
		palette.set_pen_color(index, rgb_t(pal2bit((index >> 0)&1), pal2bit((index >> 1)&1), pal2bit((index >> 2)&1)));
}


/**************************************************
*                   R/W Handlers                  *
**************************************************/

WRITE8_MEMBER(sanremo_state::lamps_w)
{
/*  LAMPS:

    7654 3210
    ---- ---x  DISCARD 1
    ---- --x-  DISCARD 2
    ---- -x--  DISCARD 3
    ---- x---  DISCARD 4
    ---x ----  DISCARD 5
    --x- ----  START
    -x-- ----  BET
    x--- ----  (always on)
*/
	output_set_lamp_value(0, (data >> 0) & 1);  /* DISCARD 1 */
	output_set_lamp_value(1, (data >> 1) & 1);  /* DISCARD 2 */
	output_set_lamp_value(2, (data >> 2) & 1);  /* DISCARD 3 */
	output_set_lamp_value(3, (data >> 3) & 1);  /* DISCARD 4 */
	output_set_lamp_value(4, (data >> 4) & 1);  /* DISCARD 5 */
	output_set_lamp_value(5, (data >> 5) & 1);  /* START */
	output_set_lamp_value(6, (data >> 6) & 1);  /* BET */
}

WRITE8_MEMBER(sanremo_state::banksel_w)
{
/*  GFX banks selector.

    7654 3210
    ---x xxxx  GFX banks selector
    xxx- ----  unknown
*/
	banksel = data & 0x1f;
}


/*********************************************
*           Memory map information           *
*********************************************/

static ADDRESS_MAP_START( sanremo_map, AS_PROGRAM, 8, sanremo_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM_WRITE(sanremo_videoram_w) AM_SHARE("videoram")  // 2x 76C28 (1x accessed directly, latched bank written to other like subsino etc.)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("nvram")                               // battery backed UM6116
ADDRESS_MAP_END

static ADDRESS_MAP_START( sanremo_portmap, AS_IO, 8, sanremo_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN0")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN1")
	AM_RANGE(0x04, 0x04) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x05, 0x05) AM_WRITE(lamps_w)
	AM_RANGE(0x14, 0x14) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x17, 0x17) AM_DEVWRITE("ay8910", ay8910_device, data_w)
	AM_RANGE(0x24, 0x24) AM_WRITE(banksel_w)
	AM_RANGE(0x27, 0x27) AM_DEVREAD("ay8910", ay8910_device, data_r)
	AM_RANGE(0x37, 0x37) AM_DEVWRITE("ay8910", ay8910_device, address_w)
ADDRESS_MAP_END

/*

  00 W -- contents of $C110 --> output port (normal 07, hopper 84).
  01 R -- IN0 (read and xor to port 03h)
  02 R -- IN1
  03 W -- ??? (from IN0)
  04 W -- CRTC address
  05 W -- contents of $C117 --> lamps.
  06 W -- contents of $C118 --> output port (mech pulses?)
  14 W -- CRTC register
  15 W -- contents of $C119 --> unknown
  17 W -- AY8910 data
  24 W -- sequence 05 05 05 05 05 05 05 05 05 06 07 0A 0B 0C 0D (banking)
  27 R -- AY8910 read
  37 W -- AY8910 address

*/


/*********************************************
*                Input ports                 *
*********************************************/

static INPUT_PORTS_START( number1 )
	PORT_START("IN0")   // from I/O port 01h.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("IN1-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("IN1-4") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("IN1-5") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("IN1-6") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )      PORT_NAME("IN1-8") PORT_CODE(KEYCODE_G)

	PORT_START("IN1")   // from I/O port 02h.
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Discard 2 / Basso (Low) / Left Card")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Discard 4 / Alto (High) / Right Card")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("IN0-5")     PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Discard 3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Discard 5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Discard 1")

	PORT_START("DSW")   // from AY-8910 por A.
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Auto Hold" )         PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Test Mode" )         PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( sanremo )
	GFXDECODE_ENTRY( "gfx",  0,   tilelayout, 0, 1 )
GFXDECODE_END


/*********************************************
*              Machine Drivers               *
*********************************************/

static MACHINE_CONFIG_START( sanremo, sanremo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sanremo_map)
	MCFG_CPU_IO_MAP(sanremo_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sanremo_state, irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(70*8, 41*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 48*8-1, 0, 38*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sanremo_state, screen_update_sanremo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)
	// *** MC6845 init ***
	//
	// Register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
	// Value:     0x45  0x30  0x36  0x0A  0x28  0x00  0x26  0x27  0x00  0x07  0x20  0x0B  0x00  0x00  0x00  0x00  0x00  0x00.
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sanremo)
	MCFG_PALETTE_ADD("palette", 0x10)
	MCFG_PALETTE_INIT_OWNER(sanremo_state, sanremo)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, SND_CLOCK)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( number1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "no_g0.ic26", 0x0000, 0x8000, CRC(2d83646f) SHA1(d1fafcce44ed3ec3dd53d84338c42244ebfca820) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD( "no_i4.ic30", 0x00000, 0x10000, CRC(55b351a4) SHA1(b0c8a30dde076520234281da051f21f1b7cb3166) )    // I
	ROM_LOAD( "no_b4.ic27", 0x10000, 0x10000, CRC(e48b1c8a) SHA1(88f60268fd43c06e146d936a1bdc078c44e2a213) )    // B
	ROM_LOAD( "no_g4.ic28", 0x20000, 0x10000, CRC(4eea9a9b) SHA1(c86c083ccf08c3c310028920f9a0fe809fd7ccbe) )    // G
	ROM_LOAD( "no_r4.ic29", 0x30000, 0x10000, CRC(ab08cdaf) SHA1(e0518403039b6bada79ffe4c6bc22fbb64d16e43) )    // R

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "number1_nvram.bin", 0x0000, 0x0800, CRC(4ece7b39) SHA1(49815571d75a39ab67d26691f902dfbd4e05feb4) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce1.bin", 0x0000, 0x0104, NO_DUMP )   /* PALCE is read protected */
	ROM_LOAD( "palce2.bin", 0x0200, 0x0104, NO_DUMP )   /* PALCE is read protected */
	ROM_LOAD( "palce3.bin", 0x0400, 0x0104, NO_DUMP )   /* PALCE is read protected */
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

/*     YEAR  NAME     PARENT  MACHINE  INPUT    STATE          INIT   ROT    COMPANY           FULLNAME      FLAGS  LAYOUT  */
GAMEL( 1996, number1, 0,      sanremo, number1, driver_device, 0,     ROT0, "San Remo Games", "Number One",  0,     layout_sanremo )
