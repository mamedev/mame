// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Steel Force

ELECTRONIC DEVICES 1994 Milano Italy
ECOGAMES S.L. Barcelona, Spain

driver by David Haywood
inputs etc. by stephh

----------------------------------------

68000P12 processor
15MHz crystal next to it

2 of these:

TPC 1020AFN-084c

32MHz crystal close to this.

8 PLDs: 3 x GAL16V8 + 4 x 16S25HBJ + 1 x GAL22V10

27c4001
u1, u27, u28, u29, u30

27c2001
u31,u32, u33, u34

27c010
u104, u105

----------------------------------------

notes:

lev 1 : 0x64 : 0000 0100 - just rte
lev 2 : 0x68 : 0000 0100 - just rte
lev 3 : 0x6c : 0000 0100 - just rte
lev 4 : 0x70 : 0000 CBD6 - vblank
lev 5 : 0x74 : 0000 0100 - just rte
lev 6 : 0x78 : 0000 0100 - just rte
lev 7 : 0x7c : 0000 0100 - just rte


  2002.02.03 : There doesn't seem to be Dip Switches
               (you make the changes in the "test mode")
               Bits 8 to 15 of IN1 seem to be unused
               The 2nd part of the "test mode" ("sound and video") is in Spanish
               Release date and manufacturers according to the title screen

 2004.xx.10 - Pierpaolo Prazzoli
 - fixed bit 4 of IN1. it is vblank and it fixed scroll issue in attract mode
 - fixed sprite glitches with visible flag
 - added rows scroll
 - added eeprom
 - fixed sound banking

TO DO :
  - unknown registers
  - clipping issues? (maybe visible area should just be 384 wide, with glitches expected at edges?)
  - layer offsets (need verifying in different situations for each game, might also be based on layer enables?)
  - priority issues?
  - same sprites buffer used in Mighty Warriors
  - clocks don't match on the games?
  - in mortalr turning on 'TEST MODE' in Service Mode causes game to use the 8x8 tilemap for some kind of debugging info, but no tiles for it exist
    (should it render a corrupt layer or not?)

*/

#include "emu.h"

#include "edevices.h"

#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class stlforce_state : public driver_device
{
public:
	stlforce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void stlforce(machine_config &config);
	void mortalr(machine_config &config);

	void program_map(address_map &map) ATTR_COLD;
	void mortalr_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void eeprom_w(uint8_t data);
};

class twinbrat_state : public stlforce_state
{
public:
	twinbrat_state(const machine_config &mconfig, device_type type, const char *tag) :
		stlforce_state(mconfig, type, tag),
		m_okibank(*this, "okibank")
	{ }

	void twinbrat(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_okibank;

	void oki_bank_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};


void twinbrat_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);
}

void stlforce_state::eeprom_w(uint8_t data)
{
	m_eeprom->di_write(data & 0x01);
	m_eeprom->cs_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE );
	m_eeprom->clk_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE );
}

void twinbrat_state::oki_bank_w(uint8_t data)
{
	m_okibank->set_entry(data & 3);
}

void stlforce_state::program_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1007ff).ram().w("video", FUNC(edevices_device::bg_videoram_w)).share("bg_videoram");
	map(0x100800, 0x100fff).ram().w("video", FUNC(edevices_device::mlow_videoram_w)).share("mlow_videoram");
	map(0x101000, 0x1017ff).ram().w("video", FUNC(edevices_device::mhigh_videoram_w)).share("mhigh_videoram");
	map(0x101800, 0x1027ff).ram().w("video", FUNC(edevices_device::tx_videoram_w)).share("tx_videoram");
	map(0x102800, 0x102fff).ram(); // unknown / RAM
	map(0x103000, 0x1033ff).ram().share("bg_scrollram");
	map(0x103400, 0x1037ff).ram().share("mlow_scrollram");
	map(0x103800, 0x103bff).ram().share("mhigh_scrollram");
	map(0x103c00, 0x103fff).ram().share("vidattrram");
	map(0x104000, 0x104fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x105000, 0x107fff).ram(); // unknown / RAM
	map(0x108000, 0x1087ff).ram().share("spriteram");
	map(0x108800, 0x108fff).ram();
	map(0x109000, 0x11ffff).ram();
	map(0x400000, 0x400001).portr("INPUT");
	map(0x400002, 0x400003).portr("SYSTEM");
	map(0x400011, 0x400011).w(FUNC(stlforce_state::eeprom_w));
	map(0x40001e, 0x40001f).w("video", FUNC(edevices_device::sprites_commands_w));
	map(0x410001, 0x410001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void stlforce_state::mortalr_map(address_map& map)
{
	stlforce_state::program_map(map);

	map(0x120000, 0x12ffff).ram(); // mortal race has piggybacked RAM chips to double RAM capacity
}

void twinbrat_state::program_map(address_map &map)
{
	stlforce_state::program_map(map);

	map(0x400012, 0x400012).w(FUNC(twinbrat_state::oki_bank_w));
}

void twinbrat_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

static INPUT_PORTS_START( stlforce )
	PORT_START("INPUT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout stlforce_bglayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16,16*32+12,16*32+8,16*32+4,16*32+0,16*32+28,16*32+24,16*32+20,16*32+16},
	{STEP16(0, 32)},
	32*32
};

static const gfx_layout stlforce_txlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{0,1,2,3},
	{12,8,4,0,28,24,20,16},
	{0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32 },
	8*32
};

static const gfx_layout stlforce_splayout =
{
	16,16,
	RGN_FRAC(1,4),
	4,
	{RGN_FRAC(3,4),RGN_FRAC(2,4),RGN_FRAC(1,4),RGN_FRAC(0,4)},
	{16*8+7,16*8+6,16*8+5,16*8+4,16*8+3,16*8+2,16*8+1,16*8+0,7,6,5,4,3,2,1,0},
	{0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8},
	32*8
};

static GFXDECODE_START( gfx_stlforce )
	GFXDECODE_ENTRY( "sprites",     0x000000, stlforce_splayout, 1024,  16 )
	GFXDECODE_ENTRY( "txtile",      0x000000, stlforce_txlayout, 384,   8  )
	GFXDECODE_ENTRY( "midhightile", 0x000000, stlforce_bglayout, 256,   8  )
	GFXDECODE_ENTRY( "midlowtile",  0x000000, stlforce_bglayout, 128,   8  )
	GFXDECODE_ENTRY( "bgtile",      0x000000, stlforce_bglayout, 0,     8  )
GFXDECODE_END

void stlforce_state::stlforce(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 15_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stlforce_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(stlforce_state::irq4_line_hold));

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8, 48*8-1-8-2, 0, 30*8-1);
	screen.set_screen_update("video", FUNC(edevices_sforce_device::draw));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_stlforce);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	edevices_sforce_device &video(EDEVICES_SFORCE_VID(config, "video", 0));
	video.set_bg_videoram_tag("bg_videoram");
	video.set_mlow_videoram_tag("mlow_videoram");
	video.set_mhigh_videoram_tag("mhigh_videoram");
	video.set_tx_videoram_tag("tx_videoram");
	video.set_bg_scrollram_tag("bg_scrollram");
	video.set_mlow_scrollram_tag("mlow_scrollram");
	video.set_mhigh_scrollram_tag("mhigh_scrollram");
	video.set_vidattrram_tag("vidattrram");
	video.set_spriteram_tag("spriteram");
	video.set_gfxdecode_tag("gfxdecode");
	video.set_palette_tag("palette");
	video.set_spritexoffset(7);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 32_MHz_XTAL / 32, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void twinbrat_state::twinbrat(machine_config &config)
{
	stlforce(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &twinbrat_state::program_map);
	m_maincpu->set_clock(14.7456_MHz_XTAL);

	subdevice<screen_device>("screen")->set_visarea(3*8, 44*8-1, 0*8, 30*8-1);

	subdevice<edevices_sforce_device>("video")->set_spritexoffset(-2);

	subdevice<okim6295_device>("oki")->set_clock(XTAL(30'000'000) / 32); // verified on 2 PCBs
	subdevice<okim6295_device>("oki")->set_addrmap(0, &twinbrat_state::oki_map);
}

void stlforce_state::mortalr(machine_config &config)
{
	stlforce(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &stlforce_state::mortalr_map);

	subdevice<okim6295_device>("oki")->set_clock(32.22_MHz_XTAL / 32);
}


ROM_START( stlforce )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // 68000 code
	ROM_LOAD16_BYTE( "stlforce.105", 0x00000, 0x20000, CRC(3ec804ca) SHA1(4efcf3321b7111644ac3ee0a83ad95d0571a4021) )
	ROM_LOAD16_BYTE( "stlforce.104", 0x00001, 0x20000, CRC(69b5f429) SHA1(5bd20fad91a22f4d62f85a5190d72dd824ee26a5) )

	ROM_REGION( 0x200000, "tiles", 0 ) // 16x16 bg tiles & 8x8 tx tiles merged
	ROM_LOAD16_BYTE( "stlforce.u27", 0x000001, 0x080000, CRC(c42ef365) SHA1(40e9ee29ea14b3bc2fbfa4e6acb7d680cf72f01a) )
	ROM_LOAD16_BYTE( "stlforce.u28", 0x000000, 0x080000, CRC(6a4b7c98) SHA1(004d7f3c703c6abc79286fa58a4c6793d66fca39) )
	ROM_LOAD16_BYTE( "stlforce.u29", 0x100001, 0x080000, CRC(30488f44) SHA1(af0d92d8952ce3cd893ab9569afdda12e17795e7) )
	ROM_LOAD16_BYTE( "stlforce.u30", 0x100000, 0x080000, CRC(cf19d43a) SHA1(dc04930548ac5b7e2b74c6041325eac06e773ed5) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_COPY( "tiles", 0x000000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midlowtile", 0 )
	ROM_COPY( "tiles", 0x080000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midhightile", 0 )
	ROM_COPY( "tiles", 0x100000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "txtile", 0 )
	ROM_COPY( "tiles", 0x180000, 0x000000, 0x080000)

	ROM_REGION( 0x100000, "sprites", 0 ) // 16x16
	ROM_LOAD( "stlforce.u36", 0x00000, 0x40000, CRC(037dfa9f) SHA1(224f5cd1a95d55b065aef5c0bd03b50cabcb619b) )
	ROM_LOAD( "stlforce.u31", 0x40000, 0x40000, CRC(305a8eb5) SHA1(3a8d26f8bc4ec2e8246d1c59115e21cad876630d) )
	ROM_LOAD( "stlforce.u32", 0x80000, 0x40000, CRC(760e8601) SHA1(a61f1d8566e09ce811382c6e23f3881e6c438f15) )
	ROM_LOAD( "stlforce.u33", 0xc0000, 0x40000, CRC(19415cf3) SHA1(31490a1f3321558f82667b63f3963b2ec3fa0c59) )

	// only one bank
	ROM_REGION( 0x80000, "oki", 0 ) // samples, second half 0xff filled
	ROM_LOAD( "stlforce.u1", 0x00000, 0x80000, CRC(0a55edf1) SHA1(091f12e8110c62df22b370a2e710c930ba06e8ca) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-stlforce.bin", 0x0000, 0x0080, CRC(3fb83951) SHA1(0cbf09751e46f100db847cf0594a4440126a7b6e) )
ROM_END

/* PCB has the following markings ('Steell' typo is correct)

Elettronica Video Games S.r.l

Made in Italy    Steell Force

*/

ROM_START( mortalr )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u105", 0x00000, 0x80000, CRC(550c48e3) SHA1(cdd2a00a6377273c73f37944f1ee6acfb4d41e82) )
	ROM_LOAD16_BYTE( "3.u104", 0x00001, 0x80000, CRC(92fad747) SHA1(0b41f31e2f14607b572ef56751b3cb201cec1bf2) )

	ROM_REGION( 0x300000, "tiles", ROMREGION_ERASE00 ) // 16x16 bg tiles
	// 2 pairs of piggyback ROMs to give double usual capacity
	ROM_LOAD16_BYTE( "8_bot.u27",  0x000001, 0x080000, CRC(042297f3) SHA1(08640cb7997d10baae776f377a605fa70499f6ef) )
	ROM_LOAD16_BYTE( "9_bot.u28",  0x000000, 0x080000, CRC(ab330185) SHA1(6403d472499897395e47a05f73e3760ef632ab8a) )
	ROM_LOAD16_BYTE( "12_top.u27", 0x100001, 0x080000, CRC(fa95773c) SHA1(849f3ab4950b34200e3043d849273622e4bdbfa3) )
	ROM_LOAD16_BYTE( "13_top.u28", 0x100000, 0x080000, CRC(f2342348) SHA1(0f197e88a1911715d3b98af9e303fd1f137e5fe3) )
	ROM_LOAD16_BYTE( "10.u29",     0x200001, 0x080000, CRC(fb39b032) SHA1(c2dfb24fccd4b588d92214addee2a9bbb6e45065) )
	ROM_LOAD16_BYTE( "11.u30",     0x200000, 0x080000, CRC(a82f2421) SHA1(b0787decd1b668af5b2ed032947ca5c0ccc020e8) )

	ROM_REGION( 0x100000, "bgtile", 0 )
	ROM_COPY( "tiles", 0x000000, 0x000000, 0x100000)

	ROM_REGION( 0x100000, "midlowtile", 0 )
	ROM_COPY( "tiles", 0x100000, 0x000000, 0x100000)

	ROM_REGION( 0x100000, "midhightile", 0 )
	ROM_COPY( "tiles", 0x200000, 0x000000, 0x100000)

	ROM_REGION( 0x100000, "txtile", ROMREGION_ERASE00 )
	// no 8x8 tiles present, but layer gets enabled if you turn on the debug mode

	ROM_REGION( 0x200000, "sprites", 0 ) // 16x16
	ROM_LOAD( "4.u36", 0x000000, 0x80000, CRC(6d1e6367) SHA1(4e6d315206b4ebc75abe9cbec1a53a9ca0b29128) )
	ROM_LOAD( "5.u31", 0x080000, 0x80000, CRC(54b223bf) SHA1(43e2a7f1d56f341f08cb04b979c4d930b58c4587) )
	ROM_LOAD( "6.u32", 0x100000, 0x80000, CRC(dab08a04) SHA1(68e26cf52ebf86a6b1e96b35fb86fcafc57c9805) )
	ROM_LOAD( "7.u33", 0x180000, 0x80000, CRC(9a856797) SHA1(265628d3b5c137ae8260ed530b7778496d863fc2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "1.u1", 0x00000, 0x80000, CRC(e5c730c2) SHA1(a153a204c1452a0c95fe207d750b2df07c5e63f3) )

	ROM_REGION( 0x02e5, "plds", 0 )
	ROM_LOAD( "1_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "2_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "3_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "4_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "5_gal22v10-25lnc.bin", 0x00000, 0x002e5, NO_DUMP )
	ROM_LOAD( "6_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "7_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
	ROM_LOAD( "8_16s25hb1.bin",       0x00000, 0x00117, NO_DUMP )
ROM_END


/*

Twin Brats
Elettronica Video-Games S.R.L, 1995

PCB Layout
----------

|----------------------------------------------|
|  1.BIN                                       |
|      M6295  PAL                  6116   6116 |
|                                  6116   6116 |
|   62256                PAL                   |
|   62256                                      |
|                        6116                  |
|J  6116   |---------|   6116                  |
|A  6116   |ACTEL    |                         |
|M         |A1020A   |                         |
|M         |PL84C    |                         |
|A    PAL  |         |       30MHz       11.BIN|
|          |---------|6264   PAL         10.BIN|
|     62256    62256  6264   |---------| 9.BIN |
|     2.BIN    3.BIN         |ACTEL    | 8.BIN |
|   |-----------------|      |A1020A   | 7.BIN |
|   |   MC68000P12    |      |PL84C    | 6.BIN |
| * |                 | PAL  |         | 5.BIN |
|   |-----------------| PAL  |---------| 4.BIN |
| 93C46  14.7456MHz                            |
|----------------------------------------------|
Notes:
      68000 clock : 14.7456MHz
      M6295 clock : 0.9375MHz (30/32). Sample Rate = 937500 / 132
      VSync       : 58Hz
      *           : Push button test switch

*/


ROM_START( twinbrat )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // 68000 code
	ROM_LOAD16_BYTE( "12.u105", 0x00000, 0x20000, CRC(552529b1) SHA1(bf23680335e1c5b05b80ab139609bee9f239b910) ) // higher numbers are newer??
	ROM_LOAD16_BYTE( "13.u104", 0x00001, 0x20000, CRC(9805ba90) SHA1(cdc188fa38220d18c60c9f438520ee574e6ce0f7) ) // higher numbers are newer??

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_COPY( "tiles", 0x000000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midlowtile", 0 )
	ROM_COPY( "tiles", 0x080000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midhightile", 0 )
	ROM_COPY( "tiles", 0x100000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "txtile", 0 )
	ROM_COPY( "tiles", 0x180000, 0x000000, 0x080000)

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

ROM_START( twinbrata )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // 68000 code
	ROM_LOAD16_BYTE( "2.u105", 0x00000, 0x20000, CRC(33a9bb82) SHA1(0f54239397c93e264b9b211f67bf626acf1246a9) )
	ROM_LOAD16_BYTE( "3.u104", 0x00001, 0x20000, CRC(b1186a67) SHA1(502074063101885874db76ae707db1082313efcf) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_COPY( "tiles", 0x000000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midlowtile", 0 )
	ROM_COPY( "tiles", 0x080000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midhightile", 0 )
	ROM_COPY( "tiles", 0x100000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "txtile", 0 )
	ROM_COPY( "tiles", 0x180000, 0x000000, 0x080000)

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "4.bin", 0x000000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x000001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

ROM_START( twinbratb )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF ) // 68000 code
	ROM_LOAD16_BYTE( "2.bin", 0x00000, 0x20000, CRC(5e75f568) SHA1(f42d2a73d737e6b01dd049eea2a10fc8c8096d8f) )
	ROM_LOAD16_BYTE( "3.bin", 0x00001, 0x20000, CRC(0e3fa9b0) SHA1(0148cc616eac84dc16415e1557ec6040d14392d4) )

	ROM_REGION( 0x200000, "tiles", 0 )
	ROM_LOAD16_BYTE( "6.bin", 0x000000, 0x80000, CRC(af10ddfd) SHA1(e5e83044f20d6cbbc1b4ef1812ac57b6dc958a8a) )
	ROM_LOAD16_BYTE( "7.bin", 0x000001, 0x80000, CRC(3696345a) SHA1(ea38be3586757527b2a1aad2e22b83937f8602da) )
	ROM_LOAD16_BYTE( "4.bin", 0x100000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x100001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x080000, "bgtile", 0 )
	ROM_COPY( "tiles", 0x000000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midlowtile", 0 )
	ROM_COPY( "tiles", 0x080000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "midhightile", 0 )
	ROM_COPY( "tiles", 0x100000, 0x000000, 0x080000)

	ROM_REGION( 0x080000, "txtile", 0 )
	ROM_COPY( "tiles", 0x180000, 0x000000, 0x080000)

	ROM_REGION( 0x100000, "tiles2", 0 )
	ROM_LOAD16_BYTE( "4.bin", 0x000000, 0x80000, CRC(1ae8a751) SHA1(5f30306580c6ab4af0ddbdc4519eb4e0ab9bd23a) )
	ROM_LOAD16_BYTE( "5.bin", 0x000001, 0x80000, CRC(cf235eeb) SHA1(d067e2dd4f28a8986dd76ec0eba90e1adbf5787c) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "11.bin", 0x000000, 0x40000, CRC(00eecb03) SHA1(5913da4d2ad97c1ce5e8e601a22b499cd93af744) )
	ROM_LOAD( "10.bin", 0x040000, 0x40000, CRC(7556bee9) SHA1(3fe99c7e9378791b79c43b04f5d0a36404448beb) )
	ROM_LOAD( "9.bin",  0x080000, 0x40000, CRC(13194d89) SHA1(95c35b6012f98a64630abb40fd55b24ff8a5e031) )
	ROM_LOAD( "8.bin",  0x0c0000, 0x40000, CRC(79f14528) SHA1(9c07d9a9e59f69a525bbaec05d74eb8d21bb9563) )

	ROM_REGION( 0x080000, "oki", 0 ) // Samples, 0x00000 - 0x20000 fixed, 0x20000 - 0x40000 banked
	ROM_LOAD( "1.bin", 0x00000, 0x80000, CRC(76296578) SHA1(04eca78abe60b283269464c0d12815579126ac08) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "eeprom-twinbrat.bin", 0x0000, 0x0080, CRC(9366263d) SHA1(ff5155498ed0b349ecc1ce98a39566b642201cf2) )
ROM_END

} // anonymous namespace


GAME( 1994, stlforce,  0,        stlforce, stlforce, stlforce_state, empty_init, ROT0, "Electronic Devices Italy / Ecogames S.L. Spain", "Steel Force", MACHINE_SUPPORTS_SAVE )

GAME( 1995, mortalr,   0,        mortalr,  stlforce, stlforce_state, empty_init, ROT0, "New Dream Games", "Mortal Race", MACHINE_SUPPORTS_SAVE ) // based on the same rough codebase as Top Driving tch/topdrive.cpp but not the same game, so not a clone

GAME( 1995, twinbrat,  0,        twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games", "Twin Brats (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbrata, twinbrat, twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games", "Twin Brats (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, twinbratb, twinbrat, twinbrat, stlforce, twinbrat_state, empty_init, ROT0, "Elettronica Video-Games", "Twin Brats (set 3)", MACHINE_SUPPORTS_SAVE )
