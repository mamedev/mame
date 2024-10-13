// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

  NSM TMS9995 BASED HARDWARE
  --------------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * NSM Poker,  198?, NSM.


*******************************************************************************


  Hardware Notes:
  ---------------

  CPU:    1x TMS9995.
  Sound:  1x AY8912.

  ROMs:   5x Eproms.
  RAMs:   5x 2116 (2kx8) SRAM.

  one 1400bit(?!) "write rom".

  Clock: 1x 22.1184 MHz. Xtal.

  No bprom, no gals, no pals....


*******************************************************************************


    *** Game Notes ***




*******************************************************************************


    DRIVER UPDATES:


    [2010-10-07]

    - Initial release.
    - Added technical notes.


    TODO:



*******************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"



namespace {

#define MASTER_CLOCK    XTAL(22'118'400)


class nsmpoker_state : public driver_device
{
public:
	nsmpoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void nsmpoker(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<tms9995_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	void nsmpoker_videoram_w(offs_t offset, uint8_t data);
	void nsmpoker_colorram_w(offs_t offset, uint8_t data);
	uint8_t debug_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void nsmpoker_palette(palette_device &palette) const;
	uint32_t screen_update_nsmpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(nsmpoker_interrupt);
	void nsmpoker_map(address_map &map) ATTR_COLD;
	void nsmpoker_portmap(address_map &map) ATTR_COLD;
};


/*************************
*     Video Hardware     *
*************************/


void nsmpoker_state::nsmpoker_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void nsmpoker_state::nsmpoker_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(nsmpoker_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---- ----   bank select.
    ---- ----   color code.
    ---- ----   seems unused.
*/
//  int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index];
//  int bank = (attr & 0x08) >> 3;
//  int color = (attr & 0x03);

	tileinfo.set(0 /* bank */, code, 0 /* color */, 0);
}


void nsmpoker_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(nsmpoker_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t nsmpoker_state::screen_update_nsmpoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void nsmpoker_state::nsmpoker_palette(palette_device &palete) const
{
}


/**************************
*  Read / Write Handlers  *
**************************/

INTERRUPT_GEN_MEMBER(nsmpoker_state::nsmpoker_interrupt)
{
	m_maincpu->set_input_line(INT_9995_INT1, ASSERT_LINE);
	// need to clear the interrupt; maybe right here?
	m_maincpu->set_input_line(INT_9995_INT1, CLEAR_LINE);
}

//void nsmpoker_state::debug_w(uint8_t data)
//{
//  popmessage("written : %02X", data);
//}

uint8_t nsmpoker_state::debug_r(offs_t offset)
{
	return BIT(machine().rand(), offset);
}


/*************************
* Memory Map Information *
*************************/

void nsmpoker_state::nsmpoker_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x9000, 0xafff).ram(); // OK... cleared at beginning.
	map(0xb000, 0xcfff).rom(); // WRONG... just to map the last rom somewhere.
	map(0xe000, 0xefff).ram().w(FUNC(nsmpoker_state::nsmpoker_videoram_w)).share("videoram"); // WRONG... just a placeholder.
	map(0xf000, 0xffff).ram().w(FUNC(nsmpoker_state::nsmpoker_colorram_w)).share("colorram"); // WRONG... just a placeholder.
}

void nsmpoker_state::nsmpoker_portmap(address_map &map)
{
	map.global_mask(0xfff);
	map(0xf00, 0xf0f).r(FUNC(nsmpoker_state::debug_r));   // kind of trap at beginning
}

/* I/O byte R/W


   -----------------

   unknown writes:


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( nsmpoker )
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

	PORT_START("DSW3")
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

	PORT_START("DSW4")
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

/* Only to see possible tiles... */

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },    /* bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(4,4),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_nsmpoker )
	GFXDECODE_ENTRY( "maincpu", 0, charlayout, 0, 16 )
	GFXDECODE_ENTRY( "maincpu", 0, tilelayout, 0, 16 )
GFXDECODE_END


void nsmpoker_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

/*************************
*    Machine Drivers     *
*************************/

void nsmpoker_state::nsmpoker(machine_config &config)
{
	// CPU TMS9995, standard variant; no line connections
	TMS9995(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &nsmpoker_state::nsmpoker_map);
	m_maincpu->set_addrmap(AS_IO, &nsmpoker_state::nsmpoker_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(nsmpoker_state::nsmpoker_interrupt));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(nsmpoker_state::screen_update_nsmpoker));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_nsmpoker);
	PALETTE(config, "palette", FUNC(nsmpoker_state::nsmpoker_palette), 16);
}


/*************************
*        Rom Load        *
*************************/

ROM_START( nsmpoker )
//  ROM_REGION( 0x10000, "maincpu", 0 )
//  ROM_LOAD( "113_277.6e", 0x0000, 0x2000, CRC(247ad554) SHA1(5cfdfb95920d7e89e3e485a06d0099191e8d41a0) )

//  ROM_REGION( 0x8000, "gfx1", 0 )
//  ROM_LOAD( "113_278.6g", 0x0000, 0x2000, CRC(08eb7305) SHA1(4e555aa481c6b4476b71909ddabf405dd6f767ed) )
//  ROM_LOAD( "113_279.5g", 0x2000, 0x2000, CRC(ac6ab327) SHA1(1012dc581b2be7df5e079ace44a721d17d21366a) )
//  ROM_LOAD( "113_280.3g", 0x4000, 0x2000, CRC(9b9be79d) SHA1(8301e74c4869d04eba680d156de9edaadd7ff83b) )
//  ROM_LOAD( "113_281.2g", 0x6000, 0x2000, CRC(4b9b448a) SHA1(3ca1f5714cf5535d2ea1e7e03bca456c89af222c) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "113_277.6e", 0x0000, 0x2000, CRC(247ad554) SHA1(5cfdfb95920d7e89e3e485a06d0099191e8d41a0) )
	ROM_LOAD( "113_278.6g", 0x2000, 0x2000, CRC(08eb7305) SHA1(4e555aa481c6b4476b71909ddabf405dd6f767ed) )
	ROM_LOAD( "113_279.5g", 0x4000, 0x2000, CRC(ac6ab327) SHA1(1012dc581b2be7df5e079ace44a721d17d21366a) )
	ROM_LOAD( "113_280.3g", 0x6000, 0x2000, CRC(9b9be79d) SHA1(8301e74c4869d04eba680d156de9edaadd7ff83b) )
	ROM_LOAD( "113_281.2g", 0xb000, 0x2000, CRC(4b9b448a) SHA1(3ca1f5714cf5535d2ea1e7e03bca456c89af222c) )
ROM_END

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT        ROT   COMPANY   FULLNAME               FLAGS
GAME( 198?, nsmpoker, 0,      nsmpoker, nsmpoker, nsmpoker_state, empty_init, ROT0, "NSM",    "NSM Poker (TMS9995)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
