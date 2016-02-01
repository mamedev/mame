// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/*************************************************************************

  Lucky 9, Nibble.

  Driver by Roberto Fresca.

  Seems some sort of gambling game with playing cards (no poker)
  and girls graphics...


**************************************************************************
  
  Specs:
  
  1x UM6845 (U67).
  1x AY38910A/p (U75).

  3x HY6264P-12 (U153, U154, U25).
  2x IMSG171P-50G (U32, U104).

  2 Chips with no markings!

  8x 64K Graphics ROMs (U139, U141, U149, U147, U137, U143, U145, U152).
  1x 64K Program ROM (U123).
  1x 128K unknown ROM (U138).

  1x TI LF347N (U158) Operational Amplifier.
  1x LM380N (U82) 2.5W Audio Power Amplifier.

  2x XTAL - 11.98135 KDS9C
  2x 8 DIP switches banks.
  1x 3.6V lithium battery.


**************************************************************************

  Tech notes...

  About the unknown ICs:
  DIP64 (U101) CPU with Xtal tied to pins 30 % 31. --> TMS9900? (ROM 9)
  DIP40 (U64) CPU or sound IC driving 128k (ROM 10) data? (pin 20 tied to GND)


*************************************************************************/


#define MASTER_CLOCK    XTAL_12MHz

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
//#include "cpu/tms9900/tms9900.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"


class nibble_state : public driver_device
{
public:
	nibble_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(nibble_videoram_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(nibble);
	UINT32 screen_update_nibble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(nibble_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************
*     Video Hardware     *
*************************/

WRITE8_MEMBER(nibble_state::nibble_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(nibble_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---- ----   bank select.
    ---- ----   color code.
    ---- ----   seems unused.
*/
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0 /* bank */, code, 0 /* color */, 0);
}

void nibble_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(nibble_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 nibble_state::screen_update_nibble(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT_MEMBER(nibble_state, nibble)
{
}


/**************************
*  Read / Write Handlers  *
**************************/

INTERRUPT_GEN_MEMBER(nibble_state::nibble_interrupt)
{
}


/************************
*     Start & Reset     *
************************/

void nibble_state::machine_start()
{
}


void nibble_state::machine_reset()
{
}


/*************************
* Memory Map Information *
*************************/

static ADDRESS_MAP_START( nibble_map, AS_PROGRAM, 8, nibble_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc3ff) AM_WRITE(nibble_videoram_w) AM_SHARE("videoram")	// placeholder
//	AM_RANGE(0xff00, 0xff01) AM_DEVWRITE("crtc", mc6845_device, address_w)
//	AM_RANGE(0xff02, 0xff03) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( nibble_cru_map, AS_IO, 8, nibble_state )
ADDRESS_MAP_END


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( nibble )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
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

	PORT_START("DSW2")
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

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(0,8), RGN_FRAC(1,8), RGN_FRAC(2,8), RGN_FRAC(3,8), RGN_FRAC(4,8), RGN_FRAC(5,8), RGN_FRAC(6,8), RGN_FRAC(7,8) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( nibble )
	GFXDECODE_ENTRY( "gfx", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( nibble, nibble_state )

	// CPU should be switched to TMS9900
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, MASTER_CLOCK/4, nibble_map, nibble_cru_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", nibble_state,  nibble_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(nibble_state, screen_update_nibble)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", nibble)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(nibble_state, nibble)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/8) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( l9nibble )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "09.U123", 0x00000, 0x10000, CRC(dfef685d) SHA1(0aeb4257e408e8549df629a0cdb5f2b6790e32de) ) // tms9900 code?

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "01.u139", 0x00000, 0x10000, CRC(aba06e58) SHA1(5841beec122613eed2ba9f48cb1d51bfa0ff450c) )
	ROM_LOAD( "02.u141", 0x10000, 0x10000, CRC(a1e5d6d1) SHA1(8ec85b0544dd75bcb13600bae503ad2b20978281) )
	ROM_LOAD( "03.u149", 0x20000, 0x10000, CRC(ae66f77c) SHA1(6c9e98cc00b72252cb238f14686c0faef47134df) )
	ROM_LOAD( "04.u147", 0x30000, 0x10000, CRC(f1864094) SHA1(b439f9e8c2cc4575f9edbda45b9e724257015a73) )
	ROM_LOAD( "05.u137", 0x40000, 0x10000, CRC(2e8ae9de) SHA1(5f2831f71b351e34df82af37041c9aa815eb372c) )
	ROM_LOAD( "06.u143", 0x50000, 0x10000, CRC(8a56f324) SHA1(68790a12ca57c999bd7b7f26adc206aab3c06976) )
	ROM_LOAD( "07.u145", 0x60000, 0x10000, CRC(4f757912) SHA1(63e5fc2672552463060680b7a5a94df45f3d4b68) )
	ROM_LOAD( "08.u152", 0x70000, 0x10000, CRC(4f878ee4) SHA1(215f3ead0c358cc09c21515981cbb0a1e58c2ca6) )

	ROM_REGION( 0x20000, "user", 0 )
	ROM_LOAD( "10.u138", 0x00000, 0x20000, CRC(ed831d2a) SHA1(ce5c3b24979d220215d7f0e8d50f45550aec15bd) ) // unknown data...

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "pal16l8acn.u23",  0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "pal16l8acn.uxx",  0x0200, 0x0104, NO_DUMP )

ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE  INPUT   STATE          INIT  ROT    COMPANY   FULLNAME   FLAGS... */
GAME( 19??, l9nibble, 0,      nibble,  nibble, driver_device, 0,    ROT0, "Nibble", "Lucky 9",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
