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

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "sanremo.lh"


namespace {

#define MASTER_CLOCK    XTAL(18'000'000)

#define CPU_CLOCK       MASTER_CLOCK/3
#define SND_CLOCK       MASTER_CLOCK/12
#define CRTC_CLOCK      MASTER_CLOCK/12


class sanremo_state : public driver_device
{
public:
	sanremo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void roadstar(machine_config &config);
	void sanremo(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<7> m_lamps;

	uint8_t m_attrram[0x800]{};
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_banksel = 0;

	void videoram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void banksel_w(uint8_t data);
	void lamps_w(uint8_t data);
	void sanremo_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void number1_map(address_map &map) ATTR_COLD;
	void roadstar_map(address_map &map) ATTR_COLD;
	void sanremo_portmap(address_map &map) ATTR_COLD;
};


/*********************************************
*               Video Hardware               *
*********************************************/


void sanremo_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_attrram[offset] = m_banksel;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(sanremo_state::get_tile_info)
{
	int code = m_videoram[tile_index];
	int bank = m_attrram[tile_index];

	tileinfo.set(0, code + bank * 256, 0, 0);
}

void sanremo_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sanremo_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 48, 40);

	m_lamps.resolve();

	save_item(NAME(m_attrram));
	save_item(NAME(m_banksel));
}

uint32_t sanremo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void sanremo_state::sanremo_palette(palette_device &palette) const
{
	// high intensity BGR
	for (int index = 0; index < 0x8; index++)
		palette.set_pen_color(index, rgb_t(pal1bit(BIT(index, 0)), pal1bit(BIT(index, 1)), pal1bit(BIT(index, 2))));

	// low intensity BGR
	for (int index = 0x8; index < 0x10; index++)
		palette.set_pen_color(index, rgb_t(pal2bit(BIT(index, 0)), pal2bit(BIT(index, 1)), pal2bit(BIT(index, 2))));
}


/**************************************************
*                   R/W Handlers                  *
**************************************************/

void sanremo_state::lamps_w(uint8_t data)
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
	for (int n = 0; n < 7; n++)
		m_lamps[n] = BIT(data, n);
}

void sanremo_state::banksel_w(uint8_t data)
{
	/*  GFX banks selector.

	    7654 3210
	    ---x xxxx  GFX banks selector
	    xxx- ----  unknown
	*/
	m_banksel = data & 0x1f;
}


/*********************************************
*           Memory map information           *
*********************************************/

void sanremo_state::number1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(sanremo_state::videoram_w)).share("videoram");  // 2x 76C28 (1x accessed directly, latched bank written to other like subsino etc.)
	map(0xc000, 0xc7ff).ram().share("nvram");                               // battery backed UM6116
}

void sanremo_state::roadstar_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).ram().w(FUNC(sanremo_state::videoram_w)).share("videoram");
	map(0xd000, 0xd7ff).ram().share("nvram");
}

void sanremo_state::sanremo_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).portr("IN0");
	map(0x02, 0x02).portr("IN1");
	map(0x04, 0x04).w("crtc", FUNC(mc6845_device::address_w));
	map(0x05, 0x05).w(FUNC(sanremo_state::lamps_w));
	map(0x14, 0x14).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x17, 0x17).w("ay8910", FUNC(ay8910_device::data_w));
	map(0x24, 0x24).w(FUNC(sanremo_state::banksel_w));
	map(0x27, 0x27).r("ay8910", FUNC(ay8910_device::data_r));
	map(0x37, 0x37).w("ay8910", FUNC(ay8910_device::address_w));
}

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

static GFXDECODE_START( gfx_sanremo )
	GFXDECODE_ENTRY( "gfx",  0,   tilelayout, 0, 1 )
GFXDECODE_END


/*********************************************
*              Machine Drivers               *
*********************************************/

void sanremo_state::sanremo(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &sanremo_state::number1_map);
	m_maincpu->set_addrmap(AS_IO, &sanremo_state::sanremo_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(sanremo_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(70*8, 41*8);
	screen.set_visarea(0, 48*8-1, 0, 38*8-1);
	screen.set_screen_update(FUNC(sanremo_state::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));
	// *** MC6845 init ***
	//
	// Register:   00    01    02    03    04    05    06    07    08    09    10    11    12    13    14    15    16    17
	// Value:     0x45  0x30  0x36  0x0A  0x28  0x00  0x26  0x27  0x00  0x07  0x20  0x0B  0x00  0x00  0x00  0x00  0x00  0x00.
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_sanremo);
	PALETTE(config, "palette", FUNC(sanremo_state::sanremo_palette), 0x10);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay8910(AY8910(config, "ay8910", SND_CLOCK));
	ay8910.port_a_read_callback().set_ioport("DSW");
	ay8910.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void sanremo_state::roadstar(machine_config &config)
{
	sanremo(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sanremo_state::roadstar_map);
}


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

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce1.ic4",  0x0000, 0x0104, NO_DUMP )   /* PALCE is read protected */
	ROM_LOAD( "palce2.ic24", 0x0200, 0x0104, NO_DUMP )   /* PALCE is read protected */
	ROM_LOAD( "palce3.ic25", 0x0400, 0x0104, NO_DUMP )   /* PALCE is read protected */
	ROM_LOAD( "palce2.ic31", 0x0600, 0x0104, NO_DUMP )   /* PALCE is read protected */
ROM_END

ROM_START( roadstar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic26", 0x0000, 0x8000, CRC(a5b85c28) SHA1(7f62ab82be18a094834529b9600867befa090782) )

	ROM_REGION( 0x40000, "gfx", 0 )
	ROM_LOAD( "rp i.ic30", 0x00000, 0x8000, CRC(13a47975) SHA1(d2c210afb44e2615fe216a913172851f723660ab) )    // I
	ROM_LOAD( "rp b.ic27", 0x10000, 0x8000, CRC(990df77e) SHA1(78755b8fd3a0b5f8453a1427b960844e1428cff7) )    // B
	ROM_LOAD( "rp g.ic28", 0x20000, 0x8000, CRC(a836434c) SHA1(f8960591589932b1998d9b1c3816bc1409cabbe2) )    // G
	ROM_LOAD( "rp r.ic29", 0x30000, 0x8000, CRC(6994ad22) SHA1(8288d50aa4db5c7da064a3932a0fd5b6e4070d8f) )    // R

	ROM_REGION( 0x0800, "plds", 0 )
	ROM_LOAD( "palce1.ic4",  0x0000, 0x0104, NO_DUMP )
	ROM_LOAD( "palce2.ic24", 0x0200, 0x0104, NO_DUMP )
	ROM_LOAD( "palce3.ic25", 0x0400, 0x0104, NO_DUMP )
	ROM_LOAD( "palce2.ic31", 0x0600, 0x0104, NO_DUMP )
ROM_END

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME      PARENT  MACHINE   INPUT    CLASS          INIT        ROT   COMPANY           FULLNAME       FLAGS                                        LAYOUT
GAMEL( 1996, number1,  0,      sanremo,  number1, sanremo_state, empty_init, ROT0, "San Remo Games", "Number One",  MACHINE_SUPPORTS_SAVE,                       layout_sanremo )
GAMEL( 199?, roadstar, 0,      roadstar, number1, sanremo_state, empty_init, ROT0, "San Remo Games", "Road Star",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE, layout_sanremo ) // different I/O map? or does it need special init?
