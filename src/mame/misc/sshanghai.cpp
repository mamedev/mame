// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo

/********************************************************************************************

  Super Shanghai 2000.
  Super Shanghai 2000 - Wrestle Fiesta.
  Super Shanghai 2001.

  3-reel 5-liner with an extra reel, plus a front game to hide the slots game.

  Rare stealth video slots machine platform based on a Z80 CPU and a AY-3-8910 for sound.
  5 DIP switches banks.

  Driver by Roberto Fresca & Grull Osgo.


*********************************************************************************************

  Notes:

  This platform hosts various slot games featuring different themes,
  including classic fruit symbols (7s, bells, plums, oranges, grapes, etc.),
  Easter-themed graphics (eggs, rabbits), and fish-themed designs.

  To comply with local gambling restrictions, these games are concealed behind
  a front-game interface. The system initially boots into a Pairs-style front-game.
  Hidden input sequences allow switching between the slots game and the front-game.
  Some machines further protect this mechanism with a 4-digit password, which can
  be entered using the joystick or directional buttons and confirmed with the action button.

  The platform support games with three operational modes:

   * Pairs Mode (front-game only)
   * Stealth Mode (hides the slots game)
   * Direct Slots Mode (boots directly into the slots game)


  If a set request password, use Right, Left, Up, Down, to change the code digits in order.
  Action button to validate and switch to slots mode.


********************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


constexpr XTAL MASTER_CLOCK = 12_MHz_XTAL;
constexpr XTAL CPU_CLOCK    = MASTER_CLOCK / 4;
constexpr XTAL AY_CLOCK     = MASTER_CLOCK / 8;


namespace {

class ssh2000_state : public driver_device
{
public:
	ssh2000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_vidram(*this, "bg_vidram"),
		m_bg_atrram(*this, "bg_atrram"),
		m_fg_vidram(*this, "fg_vidram"),
		m_fg_atrram(*this, "fg_atrram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void ssh2000(machine_config &config);
	void init_2001();

protected:
	void ssh2000_map(address_map &map) ATTR_COLD;
	void ssh2000_portmap(address_map &map) ATTR_COLD;

	void fg_vidram_w(offs_t offset, uint8_t data);
	void fg_atrram_w(offs_t offset, uint8_t data);
	void bg_vidram_w(offs_t offset, uint8_t data);
	void bg_atrram_w(offs_t offset, uint8_t data);
	void ssh2000_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(ssh2000);
	uint32_t screen_update_ssh2000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void portout_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	required_shared_ptr<uint8_t> m_bg_vidram;
	required_shared_ptr<uint8_t> m_bg_atrram;

	required_shared_ptr<uint8_t> m_fg_vidram;
	required_shared_ptr<uint8_t> m_fg_atrram;

	tilemap_t *m_bg_tilemap = nullptr;
	tilemap_t *m_fg_tilemap = nullptr;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/*****************************************************
*              Video Hardware emulation              *
*****************************************************/

VIDEO_START_MEMBER(ssh2000_state, ssh2000)
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssh2000_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(ssh2000_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 64, 32);

	m_bg_tilemap->set_transparent_pen(0);
}


uint32_t ssh2000_state::screen_update_ssh2000(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void ssh2000_state::bg_vidram_w(offs_t offset, uint8_t data)
{
	m_bg_vidram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ssh2000_state::bg_atrram_w(offs_t offset, uint8_t data)
{
	m_bg_atrram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void ssh2000_state::fg_vidram_w(offs_t offset, uint8_t data)
{
	m_fg_vidram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

void ssh2000_state::fg_atrram_w(offs_t offset, uint8_t data)
{
	m_fg_atrram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(ssh2000_state::get_fg_tile_info)
{
	int const code = m_fg_vidram[tile_index];
	int const attr = m_fg_atrram[tile_index];
	int tilepos = code | (attr & 0x1f) << 8;
	uint8_t color = (attr & 0xe0) >> 5;
	uint8_t bank = 0;

	tileinfo.set(bank, tilepos, color, 0);
}

TILE_GET_INFO_MEMBER(ssh2000_state::get_bg_tile_info)
{
	int const code = m_bg_vidram[tile_index];
	int const attr = m_bg_atrram[tile_index];
	int tilepos = code | (attr & 0x1f) << 8;
	uint8_t color = (attr & 0xe0) >> 5;
	uint8_t bank = 1;

	tileinfo.set(bank, tilepos, color, 0);
}


void ssh2000_state::ssh2000_palette(palette_device &palette) const
{
	// BBGGGRRR
	uint8_t const *const proms = memregion("proms")->base();
	for (int i = 0; i < 0x100; i++)
	{
		uint8_t const data = proms[0x000 + i] | (proms[0x100 + i] << 4);
		palette.set_pen_color(i, pal3bit(data >> 0), pal3bit(data >> 3), pal2bit(data >> 6));
	}
}


/*****************************************************
*            Graphics Layouts & Decode               *
*****************************************************/

static const gfx_layout tiles8x8x4_layout =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },  // bitplanes are separated
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8  // every char takes 8 consecutive bytes
};


static GFXDECODE_START( gfx_ss2001 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x4_layout,  0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x8x4_layout,  0, 16 )
GFXDECODE_END


/*****************************************************
*            Memory Maps / Addressing                *
*****************************************************/

void ssh2000_state::ssh2000_map(address_map &map)
{
	map(0x0000, 0xcfff).rom().nopw();

	map(0xd000, 0xd7ff).ram().share("nvram");
	map(0xd800, 0xdfff).ram();

	map(0xe000, 0xe7ff).ram().w(FUNC(ssh2000_state::fg_vidram_w)).share("fg_vidram");
	map(0xe800, 0xefff).ram().w(FUNC(ssh2000_state::fg_atrram_w)).share("fg_atrram");

	map(0xf000, 0xf7ff).ram().w(FUNC(ssh2000_state::bg_vidram_w)).share("bg_vidram");
	map(0xf800, 0xffff).ram().w(FUNC(ssh2000_state::bg_atrram_w)).share("bg_atrram");
}

void ssh2000_state::ssh2000_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).nopw(); // watchdog

	map(0x12, 0x12).w(FUNC(ssh2000_state::portout_w));
	map(0x13, 0x13).portr("IN0");
	map(0x14, 0x14).portr("IN1");
	map(0x15, 0x15).portr("DSWB");
	map(0x16, 0x16).portr("DSWC");
	map(0x17, 0x17).portr("DSWA");
}

/*****************************************************
*                    Output Ports                    *
*****************************************************/

void ssh2000_state::portout_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));  // Coins in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 0));  // Coins Out
}


/*****************************************************
*                    Input Ports                     *
*****************************************************/

static INPUT_PORTS_START( ssh2000 )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_NAME("Start / Down")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_9) PORT_NAME("Clear/Reset")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(KEYCODE_Z) PORT_NAME("Action")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CODE(KEYCODE_0) PORT_NAME("Port Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(KEYCODE_X) PORT_NAME("Switch to Slots (gambling game)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(KEYCODE_C) PORT_NAME("Switch to Pairs (front Game)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service Coin")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("Test 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("Test 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_NAME("Coin A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(2) PORT_NAME("Coin B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start (Automatic Mode)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_NAME("Bet / Stop / Up")

	PORT_START("DSWA")
	PORT_DIPNAME( 0x03, 0x00, "Percentage" )        PORT_DIPLOCATION("DSWA:8,7")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x01, "75%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x03, "65%" )
	PORT_DIPNAME( 0x04, 0x04, "Sprites" )           PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x04, "Fruits")
	PORT_DIPSETTING(    0x00, "Tiles")
	PORT_DIPNAME( 0x08, 0x00, "Game Name" )         PORT_DIPLOCATION("DSWA:5")
	PORT_DIPSETTING(    0x08, "NAME 2")
	PORT_DIPSETTING(    0x00, "NAME 1")
	PORT_DIPNAME( 0x10, 0x00, "Extra Take")         PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(    0x10, "OFF:  NO" )
	PORT_DIPSETTING(    0x00, "ON:  YES" )
	PORT_DIPNAME( 0x20, 0x00, "Demo Sound" )        PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(    0x20, "OFF:  NO" )
	PORT_DIPSETTING(    0x00, "ON:  YES" )
	PORT_DIPNAME( 0x40, 0x00, "Max. BET" )          PORT_DIPLOCATION("DSWA:2")
	PORT_DIPSETTING(    0x40, "OFF: 9" )
	PORT_DIPSETTING(    0x00, "ON:  5" )
	PORT_DIPNAME( 0x80, 0x00, "Demostration")       PORT_DIPLOCATION("DSWA:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x00, "Coinage A")          PORT_DIPLOCATION("DSWB:8,7,6,5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x09, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x0a, "8" )
	PORT_DIPSETTING(    0x03, "10" )
	PORT_DIPSETTING(    0x0f, "10" )
	PORT_DIPSETTING(    0x04, "20" )
	PORT_DIPSETTING(    0x0b, "20" )
	PORT_DIPSETTING(    0x0c, "40" )
	PORT_DIPSETTING(    0x05, "50" )
	PORT_DIPSETTING(    0x06, "100" )
	PORT_DIPSETTING(    0x0d, "200" )
	PORT_DIPSETTING(    0x0e, "400" )
	PORT_DIPSETTING(    0x07, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPNAME( 0xf0, 0x00, "Coinage B")          PORT_DIPLOCATION("DSWB:4,3,2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x90, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0xa0, "8" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0xf0, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0xb0, "20" )
	PORT_DIPSETTING(    0xc0, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "100" )
	PORT_DIPSETTING(    0xd0, "200" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0x70, "500" )
	PORT_DIPSETTING(    0x80, "1000" )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x00, "Found")              PORT_DIPLOCATION("DSWC:8")
	PORT_DIPSETTING(    0x01, "Auto" )
	PORT_DIPSETTING(    0x00, "Hand" )
	PORT_DIPNAME( 0x02, 0x00, "Found Pay" )         PORT_DIPLOCATION("DSWC:7")
	PORT_DIPSETTING(    0x02, "By One" )
	PORT_DIPSETTING(    0x00, "All" )
	PORT_DIPNAME( 0x04, 0x00, "Papas Pay" )         PORT_DIPLOCATION("DSWC:6")
	PORT_DIPSETTING(    0x04, "By One" )
	PORT_DIPSETTING(    0x00, "All" )
	PORT_DIPNAME( 0x08, 0x00, "Flip Type" )         PORT_DIPLOCATION("DSWC:5")
	PORT_DIPSETTING(    0x08, "on Tape" )
	PORT_DIPSETTING(    0x00, "on Kare" )
	PORT_DIPNAME( 0xf0, 0x00, "Coinage C")          PORT_DIPLOCATION("DSWC:4,3,2,1")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x90, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0xa0, "8" )
	PORT_DIPSETTING(    0x30, "10" )
	PORT_DIPSETTING(    0xf0, "10" )
	PORT_DIPSETTING(    0x40, "20" )
	PORT_DIPSETTING(    0xb0, "20" )
	PORT_DIPSETTING(    0xc0, "40" )
	PORT_DIPSETTING(    0x50, "50" )
	PORT_DIPSETTING(    0x60, "100" )
	PORT_DIPSETTING(    0xd0, "200" )
	PORT_DIPSETTING(    0xe0, "400" )
	PORT_DIPSETTING(    0x70, "500" )
	PORT_DIPSETTING(    0x80, "1000" )

	PORT_START("DSWD")
	PORT_DIPNAME( 0x0f, 0x00, "Credit Limit")           PORT_DIPLOCATION("DSWD:8,7,6,5")
	PORT_DIPSETTING(    0x00, " 5000" )
	PORT_DIPSETTING(    0x01, " 7500" )
	PORT_DIPSETTING(    0x02, "10000" )
	PORT_DIPSETTING(    0x03, "15000" )
	PORT_DIPSETTING(    0x04, "20000" )
	PORT_DIPSETTING(    0x0f, "20000" )
	PORT_DIPSETTING(    0x05, "25000" )
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPSETTING(    0x07, "35000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x09, "45000" )
	PORT_DIPSETTING(    0x0a, "50000" )
	PORT_DIPSETTING(    0x0b, "60000" )
	PORT_DIPSETTING(    0x0c, "70000" )
	PORT_DIPSETTING(    0x0d, "80000" )
	PORT_DIPSETTING(    0x0e, "90000" )
	PORT_DIPNAME( 0x10, 0x00, "Swap Code" )             PORT_DIPLOCATION("DSWD:4")
	PORT_DIPSETTING(    0x10, "Code 2" )
	PORT_DIPSETTING(    0x00, "Code 1" )
	PORT_DIPNAME( 0x20, 0x00, "Return In" )             PORT_DIPLOCATION("DSWD:3")
	PORT_DIPSETTING(    0x20, "Main" )
	PORT_DIPSETTING(    0x00, "Amusement" )
	PORT_DIPNAME( 0x40, 0x00, "Must be Off" )           PORT_DIPLOCATION("DSWD:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, "Amusement" )             PORT_DIPLOCATION("DSWD:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWE")
	PORT_DIPNAME( 0x0f, 0x00, "CoinIn Limit")           PORT_DIPLOCATION("DSWE:8,7,6,5")
	PORT_DIPSETTING(    0x00, " 1000" )
	PORT_DIPSETTING(    0x01, " 2000" )
	PORT_DIPSETTING(    0x02, " 3000" )
	PORT_DIPSETTING(    0x03, " 4000" )
	PORT_DIPSETTING(    0x04, " 5000" )
	PORT_DIPSETTING(    0x05, " 6000" )
	PORT_DIPSETTING(    0x06, " 7000" )
	PORT_DIPSETTING(    0x07, " 8000" )
	PORT_DIPSETTING(    0x08, " 9000" )
	PORT_DIPSETTING(    0x09, "10000" )
	PORT_DIPSETTING(    0x0a, "15000" )
	PORT_DIPSETTING(    0x0b, "20000" )
	PORT_DIPSETTING(    0x0c, "30000" )
	PORT_DIPSETTING(    0x0d, "40000" )
	PORT_DIPSETTING(    0x0e, "50000" )
	PORT_DIPSETTING(    0x0f, " 5000" )
	PORT_DIPNAME( 0x30, 0x00, "Preset BONUS" )         PORT_DIPLOCATION("DSWE:4,3")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x10, "2000" )
	PORT_DIPSETTING(    0x20, "3000" )
	PORT_DIPSETTING(    0x30, "4000" )
	PORT_DIPNAME( 0xc0, 0x00, "Preset LEFT" )          PORT_DIPLOCATION("DSWE:2,1")
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPSETTING(    0x40, "2000" )
	PORT_DIPSETTING(    0x80, "3000" )
	PORT_DIPSETTING(    0xc0, "6000" )
INPUT_PORTS_END


static INPUT_PORTS_START( ssh2000a )
	PORT_INCLUDE( ssh2000 )

	// alt gfx (easter and fish themes)
	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x04, 0x00, "Sprites" )               PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x04, "Fruits")
	PORT_DIPSETTING(    0x00, "Tiles")
INPUT_PORTS_END


/*****************************************************
*                   Machine Config                   *
*****************************************************/

void ssh2000_state::ssh2000(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &ssh2000_state::ssh2000_map);
	m_maincpu->set_addrmap(AS_IO, &ssh2000_state::ssh2000_portmap);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(ssh2000_state::screen_update_ssh2000));
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_ss2001);

	PALETTE(config, m_palette, FUNC(ssh2000_state::ssh2000_palette), 256);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	MCFG_VIDEO_START_OVERRIDE(ssh2000_state, ssh2000)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	ay8910_device &aysnd(AY8910(config, "aysnd", AY_CLOCK));
	aysnd.port_a_read_callback().set_ioport("DSWD");
	aysnd.port_b_read_callback().set_ioport("DSWE");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.50);
}


/*****************************************************
*                     ROMs Load                      *
*****************************************************/

/*
  Super Shanghai 2000 (set 1)
  No code to enter the game.

  type:  SHG-47
  version: 2000

  green board.

  Zilog Z80,
  Winbond WF19054,
  5x 8 DSW banks

*/
ROM_START( ssh2000 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "9.u26",   0x00000, 0x10000, CRC(9ddb0239) SHA1(15733481996de9becccd5b920e40220c6101becf) )

	ROM_REGION( 0x40000, "gfx1", 0 )  // all devices are double size, with identical halves.
	ROM_LOAD( "4.u4",  0x00000, 0x10000, CRC(bdca2cd5) SHA1(6139a697321475ac6fefac5b5a9f76c8de30f2f8) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "3.u3",  0x10000, 0x10000, CRC(c8db92f7) SHA1(3a77d9a21125e47396b06dc8c54625aa8ad54386) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "2.u2",  0x20000, 0x10000, CRC(7df72d1b) SHA1(ad61c3de89be547014ca123b72bb9775bad55cdd) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "1.u1",  0x30000, 0x10000, CRC(f4075f1c) SHA1(16513735e181e14a01fe24be6de0b22ef5a894eb) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x40000, "gfx2", 0 )  // all devices are double size, with identical halves.
	ROM_LOAD( "8.u8",  0x00000, 0x10000, CRC(2ad9778b) SHA1(05d8e0be028060cbe78ff1ff927f15f8114f8508) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "7.u7",  0x10000, 0x10000, CRC(03375cf9) SHA1(d336c4dff53a7368c9264a02fbc53afd0776a9cd) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "6.u6",  0x20000, 0x10000, CRC(a6194545) SHA1(74ddda87ad55e8bce1e986dde8fd371ab8843ceb) )
	ROM_IGNORE(0x10000)
	ROM_LOAD( "5.u5",  0x30000, 0x10000, CRC(c01acca1) SHA1(f2caed0b8624fb224f41323f15eaa9ce3fadd886) )
	ROM_IGNORE(0x10000)

	ROM_REGION( 0x2000, "other", 0 )  // bipolar PROM replacement, colour data is at 0x1800-0x18ff
	ROM_LOAD( "d27hc65d.bin",   0x0000, 0x02000, CRC(af8ce88d) SHA1(59d59b6b739aed4f6ee618db04af9ab9d2873bed) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_COPY( "other",   0x1800, 0x0000, 0x0100 )
ROM_END

/*
  Super Shanghai 2000 (set 2)
  Code 1234 to enter the game.

  type:  SHG-47
  version: 2000

  green board.

  Zilog Z80,
  Winbond WF19054,
  5x 8 DSW banks

*/
ROM_START( ssh2000a )  // main program (green board)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "main_program_set_2_green_board.bin", 0x00000, 0x10000, CRC(a5017fa0) SHA1(af985a15fe2c6195b3310ece8dc9e431d1b8c673) )

	ROM_REGION( 0x40000, "gfx1", 0 )  // all 27c512, same gfx set as ss2001 but half size roms
	ROM_LOAD( "rom4_m27c512.u4",  0x00000, 0x10000, CRC(2cd3eb2d) SHA1(31f804cf9ddcd7dd8501946f9bf2f8a13b5b48f3) )
	ROM_LOAD( "rom3_m27c512.u3",  0x10000, 0x10000, CRC(56e6e11b) SHA1(0a20ef1e05ded62b0aece5a94565736cbc83edf7) )
	ROM_LOAD( "rom2_m27c512.u2",  0x20000, 0x10000, CRC(d5d50ef8) SHA1(60016d62922b369ce78130b8a94d967585657cd1) )
	ROM_LOAD( "rom1_m27c512.u1",  0x30000, 0x10000, CRC(52d1914b) SHA1(f8e5e7ba8809006e4109b82dd84e019d0d1e5447) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "rom8_m27c512.u8",  0x00000, 0x10000, CRC(f6ef214c) SHA1(1f50ea87214b86416feda58fb5065175eb18df6a) )
	ROM_LOAD( "rom7_m27c512.u7",  0x10000, 0x10000, CRC(a7c2a38b) SHA1(149ccc4b36bca6a149f3a26f5a580f34f4020ef3) )
	ROM_LOAD( "rom6_m27c512.u6",  0x20000, 0x10000, CRC(dd228316) SHA1(2d618758e870224284efc0dc8d3a83f3447d0e48) )
	ROM_LOAD( "rom5_m27c512.u5",  0x30000, 0x10000, CRC(57328ec5) SHA1(0f5cddb4b45ddef8f6d352220e32cff27edac2b4) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.bin",   0x0000, 0x0200, CRC(3ad40503) SHA1(5f7516001ac4286df3ca4f6ab36882a15019546a) )
ROM_END

/*
  Super Shanghai 2000
  Wrestle Fiesta.
  Main program 30% bonus by Vegas (red board)
  No code to enter the game.

  type:  SHG-47
  version: 2000

  red board.

  TMPZ84C00AP-8,
  Winbond WF19054,
  5x 8 DSW banks

*/
ROM_START( ssh2000wf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "main_program_set_3_red_board.bin", 0x00000, 0x10000, CRC(4aaa348c) SHA1(a33656798807906ce72351a1740f77563c5b2640) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "27c512.u4",  0x00000, 0x10000, CRC(e3adb317) SHA1(817a885236c28dc5dee02cc1d7e9e1c780560cae) )
	ROM_LOAD( "27c512.u3",  0x10000, 0x10000, CRC(0c2cd067) SHA1(a1c749ee0c0c3d25a80af6d56c41057430646ce9) )
	ROM_LOAD( "27c512.u2",  0x20000, 0x10000, CRC(5983a654) SHA1(6374e1fb0eda2454370e54d4dd8bf6c82b2f177e) )
	ROM_LOAD( "27c512.u1",  0x30000, 0x10000, CRC(26df6ee7) SHA1(3221639174b432ea93e0db33b7554f2ce8125052) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "27c512.u8",  0x00000, 0x10000, CRC(1444ee50) SHA1(6e6137869d4c46762bab92ed54071d9292243532) )
	ROM_LOAD( "27c512.u7",  0x10000, 0x10000, CRC(f26a3eeb) SHA1(83bbe0b068549f3bceb083d17894fb080e9b58a1) )
	ROM_LOAD( "27c512.u6",  0x20000, 0x10000, CRC(096c691a) SHA1(59a45ad654981cdfecaa3390253850491baccb9f) )
	ROM_LOAD( "27c512.u5",  0x30000, 0x10000, CRC(e693e419) SHA1(8834419d02eb0f4f4a1619195a87b00e3f315a39) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.bin",   0x0000, 0x0200, CRC(3ad40503) SHA1(5f7516001ac4286df3ca4f6ab36882a15019546a) )
ROM_END


/*

  Super Shanghai 2001

  Even when the graphics set matches the 2000 version,
  the program shows:

  SHG-47C
  2001

*/
ROM_START( ssh2001 )  // main program (red board)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ss2001_shg-47c.bin",  0x0000, 0x8000, CRC(301a09af) SHA1(0cda3ffedb45e36b00c5c0daeb315f609a6035bc) )

	ROM_REGION( 0x40000, "gfx1", 0 )  // all 27c512, same gfx set as ss2000a.
	ROM_LOAD( "m27c512.u4",  0x00000, 0x10000, CRC(2cd3eb2d) SHA1(31f804cf9ddcd7dd8501946f9bf2f8a13b5b48f3) )
	ROM_LOAD( "m27c512.u3",  0x10000, 0x10000, CRC(56e6e11b) SHA1(0a20ef1e05ded62b0aece5a94565736cbc83edf7) )
	ROM_LOAD( "m27c512.u2",  0x20000, 0x10000, CRC(d5d50ef8) SHA1(60016d62922b369ce78130b8a94d967585657cd1) )
	ROM_LOAD( "m27c512.u1",  0x30000, 0x10000, CRC(52d1914b) SHA1(f8e5e7ba8809006e4109b82dd84e019d0d1e5447) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "m27c512.u8",  0x00000, 0x10000, CRC(f6ef214c) SHA1(1f50ea87214b86416feda58fb5065175eb18df6a) )
	ROM_LOAD( "m27c512.u7",  0x10000, 0x10000, CRC(a7c2a38b) SHA1(149ccc4b36bca6a149f3a26f5a580f34f4020ef3) )
	ROM_LOAD( "m27c512.u6",  0x20000, 0x10000, CRC(dd228316) SHA1(2d618758e870224284efc0dc8d3a83f3447d0e48) )
	ROM_LOAD( "m27c512.u5",  0x30000, 0x10000, CRC(57328ec5) SHA1(0f5cddb4b45ddef8f6d352220e32cff27edac2b4) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u29",   0x0000, 0x0200, CRC(3ad40503) SHA1(5f7516001ac4286df3ca4f6ab36882a15019546a) )
ROM_END

ROM_START( ssh2001a )  // main program (red board)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ss2001_shg-47c_mastroras-zahos.bin",  0x0000, 0x8000, CRC(4daa417d) SHA1(76d2c5ee1a9c85e40fc017f9a50d936eeac64944) )

	ROM_REGION( 0x40000, "gfx1", 0 )  // all 27c512, same gfx set as ss2000a.
	ROM_LOAD( "m27c512.u4",  0x00000, 0x10000, CRC(2cd3eb2d) SHA1(31f804cf9ddcd7dd8501946f9bf2f8a13b5b48f3) )
	ROM_LOAD( "m27c512.u3",  0x10000, 0x10000, CRC(56e6e11b) SHA1(0a20ef1e05ded62b0aece5a94565736cbc83edf7) )
	ROM_LOAD( "m27c512.u2",  0x20000, 0x10000, CRC(d5d50ef8) SHA1(60016d62922b369ce78130b8a94d967585657cd1) )
	ROM_LOAD( "m27c512.u1",  0x30000, 0x10000, CRC(52d1914b) SHA1(f8e5e7ba8809006e4109b82dd84e019d0d1e5447) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "m27c512.u8",  0x00000, 0x10000, CRC(f6ef214c) SHA1(1f50ea87214b86416feda58fb5065175eb18df6a) )
	ROM_LOAD( "m27c512.u7",  0x10000, 0x10000, CRC(a7c2a38b) SHA1(149ccc4b36bca6a149f3a26f5a580f34f4020ef3) )
	ROM_LOAD( "m27c512.u6",  0x20000, 0x10000, CRC(dd228316) SHA1(2d618758e870224284efc0dc8d3a83f3447d0e48) )
	ROM_LOAD( "m27c512.u5",  0x30000, 0x10000, CRC(57328ec5) SHA1(0f5cddb4b45ddef8f6d352220e32cff27edac2b4) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "am27s29.u29",   0x0000, 0x0200, CRC(3ad40503) SHA1(5f7516001ac4286df3ca4f6ab36882a15019546a) )
ROM_END


/*********************************************************************************************************************/

void ssh2000_state::init_2001()
{
	uint8_t *rom = memregion("maincpu")->base();

	rom[0x63ba] = 0xc9;
	rom[0x0202] = 0x08;  // skip amusement game
	rom[0x0213] = 0x00;  // signature

}


} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME       PARENT   MACHINE  INPUT     STATE          INIT        ROT   COMPANY    FULLNAME                                                       FLAGS
GAME( 2001, ssh2000,   0,       ssh2000, ssh2000,  ssh2000_state, empty_init, ROT0, "bootleg", "Super Shanghai 2000 (set 1, green board)",                    0 )
GAME( 2000, ssh2000a,  ssh2000, ssh2000, ssh2000a, ssh2000_state, empty_init, ROT0, "bootleg", "Super Shanghai 2000 (set 2, green board)",                    0 )
GAME( 2000, ssh2000wf, 0,       ssh2000, ssh2000a, ssh2000_state, empty_init, ROT0, "bootleg", "Super Shanghai 2000 - Wrestle Fiesta (30% bonus, red board)", 0 )
GAME( 2001, ssh2001,   0,       ssh2000, ssh2000,  ssh2000_state, init_2001 , ROT0, "bootleg", "Super Shanghai 2001 (set 1, red board)",                      0 )
GAME( 2001, ssh2001a,  ssh2001, ssh2000, ssh2000,  ssh2000_state, empty_init, ROT0, "bootleg", "Super Shanghai 2001 (set 2, red board)",                      0 )
