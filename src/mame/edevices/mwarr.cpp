// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/*

Mighty Warrior
Elettronica Video-Games S.R.L, 19??

PCB Layout
----------

|----------------------------------------------|      |---------------------------|
|     M6295  OKI0      PAL          CN1        |      | SF2-0    SF2-1     SF4-1  |
|     M6295  OKI1      PAL     |------------|  |      |                           |
|         PAL                  |------------|  |      | DW-0     SF3-1     SF4-0  |
|                         6264                 |      |                           |
|      2018               6264                 |      |          DW-1      SF3-0  |
|J     2018         |-------|    PAL           |      |                           |
|A            PAL   |ACTEL  |                  |      | OBM-12   OBM-17    OBM-15 |
|M     |-------|    |A1020A |45MHz             |      |                           |
|M PAL |ACTEL  |    |PL84C  |    PAL           |      | OBM-6    OBM-11    OBM-9  |
|A     |A1020A |2018|-------|PAL               |      |                           |
|62256 |PL84C  |2018                           |      | OBM-0    OBM-5     OBM-3  |
|62256 |-------|                               |      |                           |
|DSW1  62256   62256                           |      | OBM-13   OBM-14    OBM-16 |
|DSW2  PRG_OD  PRG_EV                          |      |                           |
|  |------------|PAL               CN2         |      | OBM-7    OBM-8     OBM-10 |
|  |MC68000P10  |          2018|------------|  |      |                           |
|  |------------|   2018   2018|------------|  |      | OBM-1    OBM-2     OBM-4  |
|12MHz              2018        PAL            |      |                           |
|----------------------------------------------|      |---------------------------|
Notes:
      68000 clock : 12.000MHz
      M6295 clocks: 0.9375MHz (45/48). Sample Rate = 937500 / 132
      VSync       : 54Hz
      CN1/2       : Connector for plug-in ROM daughterboard. The board
                    contains only 26x 27C4001 EPROMs and 2x 74LS139 logic IC's.


 driver by Pierpaolo Prazzoli

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "edevices.h"


namespace {

#define MASTER_CLOCK     XTAL(12'000'000)
#define SOUND_CLOCK      XTAL(45'000'000)


class mwarr_state : public driver_device
{
public:
	mwarr_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mwarr_ram(*this, "mwarr_ram"),
		m_okibank(*this, "okibank"),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "edevices_vid"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void mwarr(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_mwarr_ram;

	required_memory_bank m_okibank;

	void mwarr_brightness_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void oki1_bank_w(uint16_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_mwarr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<edevices_device> m_video;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void mwarr_map(address_map &map) ATTR_COLD;
	void oki2_map(address_map &map) ATTR_COLD;
};

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void mwarr_state::oki1_bank_w(uint16_t data)
{
	m_okibank->set_entry(data & 3);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void mwarr_state::mwarr_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x1007ff).ram().w(m_video, FUNC(edevices_device::bg_videoram_w)).share("bg_videoram");
	map(0x100800, 0x100fff).ram().w(m_video, FUNC(edevices_device::mlow_videoram_w)).share("mlow_videoram");
	map(0x101000, 0x1017ff).ram().w(m_video, FUNC(edevices_device::mhigh_videoram_w)).share("mhigh_videoram");
	map(0x101800, 0x1027ff).ram().w(m_video, FUNC(edevices_device::tx_videoram_w)).share("tx_videoram");
	map(0x103000, 0x1033ff).ram().share("bg_scrollram");
	map(0x103400, 0x1037ff).ram().share("mlow_scrollram");
	map(0x103800, 0x103bff).ram().share("mhigh_scrollram");
	map(0x103c00, 0x103fff).ram().share("vidattrram");
	map(0x104000, 0x104fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x108000, 0x1087ff).ram().share("spriteram");
	map(0x108800, 0x108fff).ram();
	map(0x110000, 0x11ffff).ram().share("mwarr_ram");
	map(0x110000, 0x110001).portr("P1_P2");
	map(0x110002, 0x110003).portr("SYSTEM");
	map(0x110004, 0x110005).portr("DSW");
	map(0x110010, 0x110011).w(FUNC(mwarr_state::oki1_bank_w));
	map(0x110014, 0x110015).w(FUNC(mwarr_state::mwarr_brightness_w));
	map(0x110016, 0x110017).w(m_video, FUNC(edevices_device::sprites_commands_w));
	map(0x180001, 0x180001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x190001, 0x190001).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}

void mwarr_state::oki2_map(address_map &map)
{
	/* $00000-$20000 stays the same in all sound banks, */
	/* the second half of the bank is what gets switched */
	map(0x00000, 0x1ffff).rom().region("oki2", 0);
	map(0x20000, 0x3ffff).bankr("okibank");
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mwarr )
	PORT_START("P1_P2")
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
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_CUSTOM ) // otherwise it doesn't boot (is service switch on stlforce)
	PORT_BIT( 0xfff0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0000, "Mutant" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Freeze" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0700, 0x0700, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x3800, 0x3800, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x4000, 0x0000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_LOW )
INPUT_PORTS_END

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout mwarr_tile8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0,1,2,3 },
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 12, 8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16 },
	8*16
};

static const gfx_layout mwarr_tile16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0,1,2,3 },
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 12, 8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,
		256+4, 256+0, 256+RGN_FRAC(1,2)+4, 256+RGN_FRAC(1,2)+0, 256+12, 256+8, 256+RGN_FRAC(1,2)+12, 256+RGN_FRAC(1,2)+8 },
	{ 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};

static const gfx_layout mwarr_6bpp_sprites =
{
	16,16,
	RGN_FRAC(1,6),
	6,
	{ RGN_FRAC(5,6), RGN_FRAC(4,6), RGN_FRAC(3,6), RGN_FRAC(2,6), RGN_FRAC(1,6), RGN_FRAC(0,6) },
	{ 135,134,133,132,131,130,129,128,7,6,5,4,3,2,1,0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( gfx_mwarr )
	GFXDECODE_ENTRY( "gfx1", 0, mwarr_6bpp_sprites,  1024, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, mwarr_tile8_layout,  384,  8 )
	GFXDECODE_ENTRY( "gfx3", 0, mwarr_tile16_layout,  256,  8 )
	GFXDECODE_ENTRY( "gfx4", 0, mwarr_tile16_layout,  128,  8 )
	GFXDECODE_ENTRY( "gfx5", 0, mwarr_tile16_layout,    0,  8 )
GFXDECODE_END

/*************************************
 *
 *  Video emulation
 *
 *************************************/

uint32_t mwarr_state::screen_update_mwarr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_video->draw(screen, bitmap, cliprect);
}

void mwarr_state::mwarr_brightness_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mwarr_ram[0x14 / 2]);

	double brightness = (double)(data & 0xff);
	for (int i = 0; i < 0x800; i++)
	{
		m_palette->set_pen_contrast(i, brightness/255);
	}
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mwarr_state::machine_start()
{
	m_okibank->configure_entries(0, 4, memregion("oki2")->base(), 0x20000);
}

void mwarr_state::machine_reset()
{
}

void mwarr_state::mwarr(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mwarr_state::mwarr_map);
	m_maincpu->set_vblank_int("screen", FUNC(mwarr_state::irq4_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(54);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(8+1, 48*8-1-8-1, 0, 30*8-1);
	screen.set_screen_update(FUNC(mwarr_state::screen_update_mwarr));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mwarr);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x800);

	EDEVICES_VID(config, m_video, 0);
	m_video->set_bg_videoram_tag("bg_videoram");
	m_video->set_mlow_videoram_tag("mlow_videoram");
	m_video->set_mhigh_videoram_tag("mhigh_videoram");
	m_video->set_tx_videoram_tag("tx_videoram");
	m_video->set_bg_scrollram_tag("bg_scrollram");
	m_video->set_mlow_scrollram_tag("mlow_scrollram");
	m_video->set_mhigh_scrollram_tag("mhigh_scrollram");
	m_video->set_vidattrram_tag("vidattrram");
	m_video->set_spriteram_tag("spriteram");
	m_video->set_gfxdecode_tag("gfxdecode");
	m_video->set_palette_tag("palette");
	m_video->set_spritexoffset(9);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki1", SOUND_CLOCK/48 , okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	okim6295_device &oki2(OKIM6295(config, "oki2", SOUND_CLOCK/48 , okim6295_device::PIN7_HIGH));
	oki2.add_route(ALL_OUTPUTS, "mono", 1.0);
	oki2.set_addrmap(0, &mwarr_state::oki2_map);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// all roms had a handwritten date, possible prototype? newest date is the program roms, 24th January
ROM_START( mwarr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "prg_ev", 0x00000, 0x80000, CRC(d1d5e0a6) SHA1(f47955459d41c904b96de000b32cae156ee3bcba) ) // date 24/1
	ROM_LOAD16_BYTE( "prg_od", 0x00001, 0x80000, CRC(e5217d91) SHA1(6a5d282e8e5b98628f98530e3c47b9b398e9334e) ) // date 24/1

	ROM_REGION( 0x900000, "gfx1", 0 )
	ROM_LOAD( "obm-0",  0x000000, 0x80000, CRC(b4707ba1) SHA1(35330a31e9837e5f848a21fa6f589412b35a04a0) ) // 7/11
	ROM_LOAD( "obm-6",  0x080000, 0x80000, CRC(f9675acc) SHA1(06e0c0c0928ace331ebd08cfeeaa2c8b5603457f) ) // 7/11
	ROM_LOAD( "obm-12", 0x100000, 0x80000, CRC(6239c4dd) SHA1(128040e9517151faf15c75dc1f2d79c5a66b9e1c) ) // 22/11
	ROM_LOAD( "obm-1",  0x180000, 0x80000, CRC(817dcead) SHA1(697b4b3e18e022e0635a3b02cbce1e4d2959a732) ) // 7/11
	ROM_LOAD( "obm-7",  0x200000, 0x80000, CRC(3a93c499) SHA1(9ecd72c5ef4f0edbdc19946bd33aa4e74690756d) ) // 7/11
	ROM_LOAD( "obm-13", 0x280000, 0x80000, CRC(bac42f06) SHA1(6998e605db732e6be9d8213e96bfb04a258eae8f) ) // 22/11
	ROM_LOAD( "obm-2",  0x300000, 0x80000, CRC(68cd29b0) SHA1(02f7bf463cd15eaf4713d33494f19c4fcd199e87) ) // 7/11
	ROM_LOAD( "obm-8",  0x380000, 0x80000, CRC(f9482638) SHA1(ea6256136362a12a40d6b168157c28a14236fcc1) ) // 7/11
	ROM_LOAD( "obm-14", 0x400000, 0x80000, CRC(79ed46b8) SHA1(93b503b58a316be312a74f2da7df3dbcd275884b) ) // 22/11

	ROM_LOAD( "obm-3",  0x480000, 0x80000, CRC(6e924cb8) SHA1(3c56dfcd042108b1cd16395bcdda0fd92a6ab0f7) ) // 7/11
	ROM_LOAD( "obm-9",  0x500000, 0x80000, CRC(be1fb64e) SHA1(4141b6b78fa9830cf5dc4f4f0b29e87e57f70ccb) ) // 7/11
	ROM_LOAD( "obm-15", 0x580000, 0x80000, CRC(5e0efb71) SHA1(d556ed9307a9a9f59f5235981b4b091a88399c98) ) // 22/11
	ROM_LOAD( "obm-4",  0x600000, 0x80000, CRC(f34b67bd) SHA1(91d6553144e45ea1b96bf59403b3e26224b79a7d) ) // 7/11
	ROM_LOAD( "obm-10", 0x680000, 0x80000, CRC(00c68a23) SHA1(a4984932ac3fae368700f77ad16b35b0138dbc21) ) // 7/11
	ROM_LOAD( "obm-16", 0x700000, 0x80000, CRC(e9516379) SHA1(8d9aaa2ee1331dd3eb8951a1008811703dd9f41d) ) // 22/11
	ROM_LOAD( "obm-5",  0x780000, 0x80000, CRC(b2b976f3) SHA1(77dee06ac1187c8a1c4188951bd7b0a62ec84350) ) // 7/11
	ROM_LOAD( "obm-11", 0x800000, 0x80000, CRC(7bf1e4da) SHA1(4aeef3b7c23303580a851dc793e9671a2a0f421f) ) // 7/11
	ROM_LOAD( "obm-17", 0x880000, 0x80000, CRC(47bd56e8) SHA1(e10569e89083165a7efe29f84167a1c15171ccaf) ) // 22/11

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD( "sf4-0",  0x000000, 0x80000, CRC(25938b2d) SHA1(6336e41eee58cab9a524b9bca08965786cc133d3) ) // 7/11
	ROM_LOAD( "sf4-1",  0x080000, 0x80000, CRC(2269ce5c) SHA1(4c6169acf17bba94dc5684f5db60d5bcf73ad068) ) // 7/11

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD( "sf3-0",  0x000000, 0x80000, CRC(86cd162c) SHA1(95d5f300e3671ebe29b2331325f4d80b96988619) ) // 7/11
	ROM_LOAD( "sf3-1",  0x080000, 0x80000, CRC(2e755e54) SHA1(74b1e099358a07848f7c22c71fbe2661e1ebb417) ) // 7/11

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "sf2-0",  0x000000, 0x80000, CRC(622a1816) SHA1(b7b88a90ff69e8f2e291e1f9299708ec97ef9b77) ) // 7/11
	ROM_LOAD( "sf2-1",  0x080000, 0x80000, CRC(545f89e9) SHA1(e7d52dc2da3770d7310698af47da9ff7ec32388c) ) // 7/11

	ROM_REGION( 0x100000, "gfx5", 0 )
	ROM_LOAD( "dw-0",   0x000000, 0x80000, CRC(b9b18d00) SHA1(4f38502c75eae88916bc58bfd5d255bac59d0813) ) // 22/11
	ROM_LOAD( "dw-1",   0x080000, 0x80000, CRC(7aea0b12) SHA1(07cbcd6ddcd9ead068b0f5763829e8474b699085) ) // 22/11

	ROM_REGION( 0x40000, "oki1", 0 ) /* Samples */
	ROM_LOAD( "oki0",   0x000000, 0x40000, CRC(005811ce) SHA1(9149bc8e9cc16ce3db4e22f8cb7ea8a57a66980e) ) // no date

	ROM_REGION( 0x80000, "oki2", 0 ) /* Samples */
	ROM_LOAD( "oki1",   0x000000, 0x80000, CRC(bcde2330) SHA1(452d871360fa907d2e4ebad93c3fba9a3fa32fa7) ) // no date
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 199?, mwarr, 0, mwarr, mwarr, mwarr_state, empty_init, ROT0,  "Elettronica Video-Games", "Mighty Warriors (24/1)", MACHINE_SUPPORTS_SAVE )
