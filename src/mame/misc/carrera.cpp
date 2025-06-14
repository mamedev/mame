// license:BSD-3-Clause
// copyright-holders: David Haywood, Roberto Fresca, Grull Osgo

/***********************************************************************

  Carrera / Bomberman
  by BS Electronics.

  Video slots stealth games with selectable bomberman or fruits themes.
  The front game is normally a "memory" pairs game, where the objetive
  is to clear the screen.

  You can change the tileset, the game title (Carrera/Bomberman), and
  switch the type of game (amusement/gambling).

  Some sets don't allow the title change.


  PCB Layout
  ----------

  |----------------------------------------------|
  |            22.1184MHz  Z80                   |
  |                                     ROM.IC1  |
  |_  BATTERY              ROM.IC22              |
    |                                            |
   _|                      6116         ROM.IC2  |
  |                                              |
  |                                              |
  |J  AY-3-8910                         ROM.IC3  |
  |A                       DSW1(8)               |
  |M                                             |
  |M                       DSW2(8)      ROM.IC4  |
  |A                                             |
  |                        DSW3(8)               |
  |                                     ROM.IC5  |
  |_  PROM.IC39            DSW4(8)               |
    |                                    6116    |
   _|                                            |
  |                   HD6845             6116    |
  |----------------------------------------------|

  Notes:
        Z80 @ 3.6864MHz [22.1184/6]
        AY-3-8910 @ 1.8432MHz [22.1184/12]


************************************************************************

  Technical notes:
  ----------------

  The two first sets (carrera and bsebman) have a "bug" in the title if
  the NVRAM was created from zero. This "bug" is intended to see at first
  sight if the game NVRAM was manipulated in some way.

  In fact, each boot the code paint the game title properly, and then
  force the graphics bug if a special value hardcoded in NVRAM doesn't
  match the expected.

  $1da3.......$1dd6  ; loops to paint the title/logo on screen. 

  then...

  $1dd7  ld, a ($e300)     3a 00 e3  ; take the hardcoded value from NVRAM.
  $1dda  ld, ($f616), a    32 16 f6  ; place this value as tile index,
                                     ; halfway the title top line ($f616),
                                     ; creating the intended GFX bug.

  $1ddd  ret               c9        ; return from subroutine.


  For other bootleg sets, if the hardcoded value doesn't match, they force
  another tile index very close to the original, so is way less notorious
  and only the technical trained people can see these minimal differences.

  Note: This behavior is by design and should not be interpreted as a software
  defect. It allows operators to instantly identify units with reset or modified
  NVRAM and maintains system integrity without preventing gameplay functionality.


************************************************************************

  Updates:
  
  [2025-05]

  - Fix color bipolar PROM decode and palette calculation,
     based on real games screenshots.
  - Added NVRAM support for all games.
  - Created default NVRAM for carrera and bsebman sets,
     with harcoded critical values/registers needed to get
     the games working.
  - Workaround that fix the titles corruption for both
     carrera and bsebman sets.  
  - Inputs and DIP switches.
  - Fix alantin colors, based on the real game screenshots.
  - Changed bsebman description to:
     Carrera (Version 6.7) / Bomberman (Version 6.6)
  - Added technical and game notes.

  - Traspased the game ncarrera to a new driver, since the MCU draws
    the graphics and animate the reels.


  TODO:

  - Analyze the carrera/bomberman sets and rename them accordingly.


***********************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class carrera_state : public driver_device
{
public:
	carrera_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_tileram(*this, "tileram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void carrera(machine_config &config);

private:
	uint8_t unknown_r();
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_shared_ptr<uint8_t> m_tileram;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*************************************************
*                  Video Hardware                *
*************************************************/

uint32_t carrera_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int tile = m_tileram[count & 0x7ff] | m_tileram[(count & 0x7ff) + 0x800] << 8;

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 8, y * 8);
			count++;
		}
	}
	return 0;
}

void carrera_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x20; ++i)
	{
		int bit0, bit1;
		int const br_bit0 = BIT(color_prom[i], 6);
		int const br_bit1 = BIT(color_prom[i], 7);

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 3);
		int const b = 0x0e * br_bit1 + 0x1f * br_bit0 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 1);
		bit1 = BIT(color_prom[i], 4);
		int const g = 0x0e * br_bit1 + 0x1f * br_bit0 + 0x43 * bit0 + 0x8f * bit1;
		bit0 = BIT(color_prom[i], 2);
		bit1 = BIT(color_prom[i], 5);
		int const r = 0x0e * br_bit1 + 0x1f * br_bit0 + 0x43 * bit0 + 0x8f * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


/*************************************************
*              Read Write Handlers               *
*************************************************/

uint8_t carrera_state::unknown_r()
{
	return machine().rand();
}


/*************************************************
*             Memory map information             *
*************************************************/

void carrera_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xe7ff).ram().share("nvram");
	map(0xe800, 0xe800).w("crtc", FUNC(mc6845_device::address_w));
	map(0xe801, 0xe801).w("crtc", FUNC(mc6845_device::register_w));
	map(0xf000, 0xffff).ram().share(m_tileram);
}

void carrera_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x02, 0x02).portr("IN2");
	map(0x03, 0x03).portr("IN3");
	map(0x04, 0x04).portr("IN4");
	map(0x05, 0x05).portr("IN5");
	map(0x06, 0x06).nopw();  // ?
	map(0x08, 0x09).w("aysnd", FUNC(ay8910_device::address_data_w));
}


/*************************************************
*                  Input ports                   *
*************************************************/

static INPUT_PORTS_START( carrera )
	PORT_START("IN0")   // Port 0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_NAME("Up - Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_NAME("Down - Start")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_NAME("Auto Start")  // autoplay
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("0-6") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Master Reset")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )

	PORT_START("IN1")   // Port 1
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("1-3")   PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("1-4")   PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("1-5")   PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("1-6")   PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )    PORT_NAME("1-7")   PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset")

	// I suspect the 4 below are the 4xDSWs
	PORT_START("IN2")   // Port 2
	PORT_DIPNAME( 0x01, 0x01, "2" )
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
	
	PORT_START("IN3")   // Port 3
	PORT_DIPNAME( 0x01, 0x01, "3" )
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

	PORT_START("IN4")   // Port 4
	PORT_DIPNAME( 0x07, 0x00, "Coinage B" )             PORT_DIPLOCATION("IN4:6,7,8")
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x05, "1000" )
	PORT_DIPSETTING(    0x06, "1100" )
	PORT_DIPNAME( 0x08, 0x08, "Game Name" )             PORT_DIPLOCATION("IN4:5")
	PORT_DIPSETTING(    0x08, "Bomber Man" )
	PORT_DIPSETTING(    0x00, "Carrera" )
	PORT_DIPNAME( 0x10, 0x10, "Bonus 10%" )             PORT_DIPLOCATION("IN4:4")
	PORT_DIPSETTING(    0x10, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Key Out" )               PORT_DIPLOCATION("IN4:3")
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficult ) )    PORT_DIPLOCATION("IN4:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x80, 0x80, "Max Bet" )               PORT_DIPLOCATION("IN4:1")
	PORT_DIPSETTING(    0x80, "9" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("IN5")   // Port 5
	PORT_DIPNAME( 0x07, 0x00, "Coinage A" )             PORT_DIPLOCATION("IN5:1,2,3")
	PORT_DIPSETTING(    0x07, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPSETTING(    0x01, "20" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x04, "500" )
	PORT_DIPSETTING(    0x05, "1000" )
	PORT_DIPSETTING(    0x06, "1100" )
	PORT_DIPNAME( 0x08, 0x00, "Playing Graphics" )      PORT_DIPLOCATION("IN5:4")
	PORT_DIPSETTING(    0x08, "Bricks" )
	PORT_DIPSETTING(    0x00, "Fruits" )
	
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )      PORT_DIPLOCATION("IN5:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )      PORT_DIPLOCATION("IN5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	
	PORT_DIPNAME( 0x40, 0x40, "Game Mode" )             PORT_DIPLOCATION("IN5:7")
	PORT_DIPSETTING(    0x40, "Gamble" )
	PORT_DIPSETTING(    0x00, "Amusement" )

    // this one displays numbers over the game area
	PORT_DIPNAME( 0x80, 0x80, "Debug?" )                PORT_DIPLOCATION("IN5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************************
*                Graphics Layouts                *
*************************************************/

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,5),
	5,
	{ RGN_FRAC(0,5), RGN_FRAC(1,5),RGN_FRAC(2,5),RGN_FRAC(3,5),RGN_FRAC(4,5) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};


/*************************************************
*          Graphics Decode Information           *
*************************************************/

static GFXDECODE_START( gfx_carrera )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


/*************************************************
*                Machine Drivers                 *
*************************************************/

void carrera_state::carrera(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 22.1184_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &carrera_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &carrera_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(carrera_state::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_carrera);
	PALETTE(config, m_palette, FUNC(carrera_state::palette), 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK / 12));
	// these are set as input, but I have no idea which input port it uses is for the AY
	aysnd.port_a_read_callback().set(FUNC(carrera_state::unknown_r));
	aysnd.port_b_read_callback().set(FUNC(carrera_state::unknown_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*************************************************
*                    ROM Load                    *
*************************************************/

ROM_START( carrera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(2385b9c8) SHA1(12d4397779e074096fbb23b114985f104366b79c) )

	ROM_REGION( 0x50000, "tiles", 0 )
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(a16e914e) SHA1(09f2271f193a7bffd62ef6e428ecbf9aa1154860) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(147036a5) SHA1(34b4818fe61c5b13220b0a2001987b68b655b2cb) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(920eee0e) SHA1(85e6d5292b751c57c64d17858bd00292356599e3) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(97433f36) SHA1(39f3c6b76ad540693682832aba6e4fc400ca3753) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(ffa75920) SHA1(aa5619f5aabcdfa250bb24bcad101a8c512a1776) )

	ROM_REGION( 0x800,  "nvram", 0 )  // default NVRAM
	ROM_LOAD( "carrera_nvram.bin",  0x0000, 0x0800, CRC(b2ac3ddd) SHA1(22886fba71badbdfa754489057889b5326bbaf78) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

ROM_START( bsebman )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic22", 0x00000, 0x10000, CRC(294a205f) SHA1(b088617354b6a37520060f19f77d841ad8ee1538) )

	ROM_REGION( 0x50000, "tiles", 0 )
	ROM_LOAD( "ic1", 0x00000, 0x10000, CRC(bf4868e1) SHA1(ca91343dbdb9f43d9b981b9b3f958edb17bf188d) )
	ROM_LOAD( "ic2", 0x10000, 0x10000, CRC(26e1c17e) SHA1(ea0669e87207104eeaa3eae6a6708dbdf94e3c3c) )
	ROM_LOAD( "ic3", 0x20000, 0x10000, CRC(8401248d) SHA1(f8c8ed93d76709ead0b262ab3039df6febe3c005) )
	ROM_LOAD( "ic4", 0x30000, 0x10000, CRC(6b569989) SHA1(e00263fae310094ad5119e3a9673fb342f643ddc) )
	ROM_LOAD( "ic5", 0x40000, 0x10000, CRC(21635791) SHA1(514078694269582c33fb7dddd6171089f9e21ee2) )

	ROM_REGION( 0x800,  "nvram", 0 )  // default NVRAM
	ROM_LOAD( "carrera_nvram.bin",  0x0000, 0x0800, CRC(b2ac3ddd) SHA1(22886fba71badbdfa754489057889b5326bbaf78) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "am27s19.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

ROM_START( bsebmanbl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(7b896b33) SHA1(841d5c7853e18109b74cad44c9f0d91398add146) )  // SLDH

	ROM_REGION( 0x50000, "tiles", 0 )  // still has BS GFX
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(a16e914e) SHA1(09f2271f193a7bffd62ef6e428ecbf9aa1154860) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(147036a5) SHA1(34b4818fe61c5b13220b0a2001987b68b655b2cb) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(920eee0e) SHA1(85e6d5292b751c57c64d17858bd00292356599e3) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(97433f36) SHA1(39f3c6b76ad540693682832aba6e4fc400ca3753) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(ffa75920) SHA1(aa5619f5aabcdfa250bb24bcad101a8c512a1776) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

ROM_START( bsebmanbl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )  // has Carrera, Avraam and Ballas strings
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(ca2c8962) SHA1(140a217bb0365ec55116ba483208bdf1d820a7af) )

	ROM_REGION( 0x50000, "tiles", 0 )  // hacked J.T. GFX instead of BS
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(65ad616a) SHA1(e87d6d187ec5c99628d767a9720dd9d634e39c2d) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(64004dcb) SHA1(f9af56035f00d3d914c8e83e941762cb6153fc16) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(c6e0a838) SHA1(e30d0f28845f331839afc44fb7358be72d2a88cb) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(eccd4da0) SHA1(8171ed1e9a492b804c86b5cd98cc8de91b01608c) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(f3014271) SHA1(97eeea6f2cde8f115d3285398b0083f0d1c95578) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(af16359f) SHA1(1ff5c9d7807e52be09c0ded56fb68a47e41b3fcf) )
ROM_END

ROM_START( alantin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "alpro.ic22", 0x00000, 0x10000, CRC(1eb97b31) SHA1(7097cf344734a2b553356f9b9fd6453e584a6e3f) )  // 27512

	ROM_REGION( 0x50000, "tiles", 0 )  // still has BS GFX
	ROM_LOAD( "alantin1.ic1", 0x00000, 0x10000, CRC(feb49cfd) SHA1(696683375c832b4cd6db9eef0edf4919b90c97ef) )
	ROM_LOAD( "alantin2.ic2", 0x10000, 0x10000, CRC(e79da4b9) SHA1(a80c5f6431fc755645a0c8cd0cb290669f0cbada) )
	ROM_LOAD( "alantin3.ic3", 0x20000, 0x10000, CRC(73d7c748) SHA1(bf688b8f506859ed3c514915676b13cecfec0a81) )
	ROM_LOAD( "alantin4.ic4", 0x30000, 0x10000, CRC(6a061afd) SHA1(f6d736bd284e97ab915adb249c371617daa02a36) )
	ROM_LOAD( "alantin5.ic5", 0x40000, 0x10000, CRC(35d8fb1b) SHA1(5d7ff8089e16ebb792543eeb9cc682f9f5eba6fe) )

	ROM_REGION( 0x20, "proms", 0 )  // swapping bytes through a driver init has no palette response.
	ROM_LOAD16_WORD_SWAP( "82s123.ic39", 0x00, 0x20, CRC(5b0e598f) SHA1(99a8e80229d684f2083634ae2d96bf1d4f13677c) )
ROM_END

ROM_START( drkseal )
	ROM_REGION( 0x10000, "maincpu", 0 )  // still shows Alantin and Avraam strings
	ROM_LOAD( "27512.ic22", 0x00000, 0x10000, CRC(8a1732e5) SHA1(191de15d0ccf439991e3c0c258cbfeb79ef19002) )  // 1xxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x50000, "tiles", 0 )  // shows Vegas copyright
	ROM_LOAD( "27512.ic1", 0x00000, 0x10000, CRC(fd3b6bbc) SHA1(b1fe09772a5d9a07077038244517fc7169266893) )
	ROM_LOAD( "27512.ic2", 0x10000, 0x10000, CRC(d3a048e3) SHA1(687d58b84ca5985f51755b09c8a8b2ef68d16399) )
	ROM_LOAD( "27512.ic3", 0x20000, 0x10000, CRC(50b8f9ee) SHA1(4f31e36eb54fde40e409f0ac18bf87126174be33) )
	ROM_LOAD( "27512.ic4", 0x30000, 0x10000, CRC(15292338) SHA1(95352969d3003eb3451334d569ead730cec9bcec) )
	ROM_LOAD( "27512.ic5", 0x40000, 0x10000, CRC(99fb8640) SHA1(053fdd600e5a120ea8988f3d55d1d444e8b29abe) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "82s123.ic39", 0x00, 0x20, CRC(03aadf73) SHA1(e5baf8c5e7276eb207357e4cbb694c75e8caab6a) )
ROM_END


} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME        PARENT   MACHINE  INPUT    CLASS          INIT        ROT   COMPANY                       FULLNAME                                           FLAGS
GAME( 19??, carrera,    0,       carrera, carrera, carrera_state, empty_init, ROT0, "BS Electronics",             "Carrera (Version 6.7)",                           MACHINE_SUPPORTS_SAVE )
GAME( 19??, bsebman,    carrera, carrera, carrera, carrera_state, empty_init, ROT0, "BS Electronics",             "Carrera (Version 6.7) / Bomberman (Version 6.6)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, bsebmanbl,  carrera, carrera, carrera, carrera_state, empty_init, ROT0, "bootleg (Ballas / Avraam)",  "Bomberman (Version 6.6, Avraam bootleg)",         MACHINE_SUPPORTS_SAVE )
GAME( 1999, bsebmanbl2, carrera, carrera, carrera, carrera_state, empty_init, ROT0, "bootleg (J.T.)",             "Bomberman (Version 6.6, J.T. bootleg)",           MACHINE_SUPPORTS_SAVE )
GAME( 1999, alantin,    0,       carrera, carrera, carrera_state, empty_init, ROT0, "bootleg (Robert / Avraam)",  "Alantin - Aladdin's Magic Lamp",                  MACHINE_SUPPORTS_SAVE )
GAME( 1999, drkseal,    0,       carrera, carrera, carrera_state, empty_init, ROT0, "bootleg (Vegas)",            "Dark Seal (8-liner)",                             MACHINE_SUPPORTS_SAVE )
