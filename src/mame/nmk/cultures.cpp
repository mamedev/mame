// license:BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli

/*
    Jibun wo Migaku Culture School Mahjong Hen
    (c)1994 Face

    driver by Pierpaolo Prazzoli

    thanks to David Haywood for some precious advice

    TODO: PCB has a 93C46 but it isn't hooked up in the driver. Is it unused?
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class cultures_state : public driver_device
{
public:
	cultures_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vramview(*this, "vramview"),
		m_prgbank(*this, "prgbank"),
		m_okibank(*this, "okibank"),
		m_bg_rom(*this, "bg%u", 1U),
		m_bg0_videoram(*this, "bg0_videoram"),
		m_bg_regs_x(*this, "bg%u_regs_x", 0U),
		m_bg_regs_y(*this, "bg%u_regs_y", 0U)
	{ }

	void cultures(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	// memory pointers
	memory_view m_vramview;
	required_memory_bank m_prgbank;
	required_memory_bank m_okibank;
	required_region_ptr_array<uint16_t, 2> m_bg_rom;
	required_shared_ptr<uint8_t> m_bg0_videoram;
	required_shared_ptr_array<uint8_t, 3> m_bg_regs_x;
	required_shared_ptr_array<uint8_t, 3> m_bg_regs_y;

	// video-related
	tilemap_t *m_bg_tilemap[3];
	uint8_t m_irq_enable;
	uint8_t m_bg_rombank[2];

	void cpu_bankswitch_w(uint8_t data);
	void bg0_videoram_w(offs_t offset, uint8_t data);
	void misc_w(uint8_t data);
	void bg_bank_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD;
};



TILE_GET_INFO_MEMBER(cultures_state::get_bg1_tile_info)
{
	int const code = m_bg_rom[0][0x200000 / 2 + m_bg_rombank[0] * 0x80000 / 2 + tile_index];
	tileinfo.set(1, code, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(cultures_state::get_bg2_tile_info)
{
	int const code = m_bg_rom[1][0x200000 / 2 + m_bg_rombank[1] * 0x80000 / 2 + tile_index];
	tileinfo.set(2, code, code >> 12, 0);
}

TILE_GET_INFO_MEMBER(cultures_state::get_bg0_tile_info)
{
	int const code = m_bg0_videoram[tile_index * 2] + (m_bg0_videoram[tile_index * 2 + 1] << 8);
	tileinfo.set(0, code, code >> 12, 0);
}

void cultures_state::video_start()
{
	m_bg_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cultures_state::get_bg0_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 128);
	m_bg_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cultures_state::get_bg1_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 512, 512);
	m_bg_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(cultures_state::get_bg2_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 512, 512);

	m_bg_tilemap[1]->set_transparent_pen(0);
	m_bg_tilemap[0]->set_transparent_pen(0);

	m_bg_tilemap[0]->set_scrolldx(502, -118);
	m_bg_tilemap[1]->set_scrolldx(502, -118);
	m_bg_tilemap[2]->set_scrolldx(502, -118);

	m_bg_tilemap[0]->set_scrolldy(255, -16);
	m_bg_tilemap[1]->set_scrolldy(255, -16);
	m_bg_tilemap[2]->set_scrolldy(255, -16);
}

uint32_t cultures_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// tilemaps attributes
	int attr = (m_bg_regs_x[0][3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg_regs_y[0][3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg_tilemap[0]->set_flip(attr);

	attr = (m_bg_regs_x[1][3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg_regs_y[1][3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg_tilemap[1]->set_flip(attr);

	attr = (m_bg_regs_x[2][3] & 1 ? TILEMAP_FLIPX : 0) | (m_bg_regs_y[2][3] & 1 ? TILEMAP_FLIPY : 0);
	m_bg_tilemap[2]->set_flip(attr);

	// tilemaps scrolls
	m_bg_tilemap[0]->set_scrollx(0, (m_bg_regs_x[0][2] << 8) + m_bg_regs_x[0][0]);
	m_bg_tilemap[1]->set_scrollx(0, (m_bg_regs_x[1][2] << 8) + m_bg_regs_x[1][0]);
	m_bg_tilemap[2]->set_scrollx(0, (m_bg_regs_x[2][2] << 8) + m_bg_regs_x[2][0]);
	m_bg_tilemap[0]->set_scrolly(0, (m_bg_regs_y[0][2] << 8) + m_bg_regs_y[0][0]);
	m_bg_tilemap[1]->set_scrolly(0, (m_bg_regs_y[1][2] << 8) + m_bg_regs_y[1][0]);
	m_bg_tilemap[2]->set_scrolly(0, (m_bg_regs_y[2][2] << 8) + m_bg_regs_y[2][0]);

	m_bg_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	m_bg_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void cultures_state::cpu_bankswitch_w(uint8_t data)
{
	m_prgbank->set_entry(data & 0x0f);
	m_vramview.select((data & 0x20) >> 5);
}


void cultures_state::bg0_videoram_w(offs_t offset, uint8_t data)
{
	m_bg0_videoram[offset] = data;
	m_bg_tilemap[0]->mark_tile_dirty(offset >> 1);
}

void cultures_state::misc_w(uint8_t data)
{
	m_okibank->set_entry(data & 0x0f);
	m_irq_enable = data & 0x80;
}

void cultures_state::bg_bank_w(uint8_t data)
{
	if (m_bg_rombank[0] != (data & 3))
	{
		m_bg_rombank[0] = data & 3;
		m_bg_tilemap[1]->mark_all_dirty();
	}

	if (m_bg_rombank[1] != ((data & 0xc) >> 2))
	{
		m_bg_rombank[1] = (data & 0xc) >> 2;
		m_bg_tilemap[2]->mark_all_dirty();
	}
	machine().bookkeeping().coin_counter_w(0, data & 0x10);
}


void cultures_state::oki_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x20000, 0x3ffff).bankr(m_okibank);
}

void cultures_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr(m_prgbank);
	map(0x8000, 0xbfff).view(m_vramview);
	m_vramview[0](0x8000, 0xbfff).ram().w(FUNC(cultures_state::bg0_videoram_w)).share(m_bg0_videoram);
	m_vramview[1](0x8000, 0xafff).ram().w("palette", FUNC(palette_device::write8)).share("palette");
	map(0xc000, 0xdfff).ram();
	map(0xf000, 0xffff).ram();
}

void cultures_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).ram();
	map(0x10, 0x13).ram();
	map(0x20, 0x23).ram().share(m_bg_regs_x[0]);
	map(0x30, 0x33).ram().share(m_bg_regs_y[0]);
	map(0x40, 0x43).ram().share(m_bg_regs_x[1]);
	map(0x50, 0x53).ram().share(m_bg_regs_y[1]);
	map(0x60, 0x63).ram().share(m_bg_regs_x[2]);
	map(0x70, 0x73).ram().share(m_bg_regs_y[2]);
	map(0x80, 0x80).w(FUNC(cultures_state::cpu_bankswitch_w));
	map(0x90, 0x90).w(FUNC(cultures_state::misc_w));
	map(0xa0, 0xa0).w(FUNC(cultures_state::bg_bank_w));
	map(0xc0, 0xc0).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xd0, 0xd0).portr("SW1_A");
	map(0xd1, 0xd1).portr("SW1_B");
	map(0xd2, 0xd2).portr("SW2_A");
	map(0xd3, 0xd3).portr("SW2_B");
	map(0xe0, 0xe0).portr("KEY0");
	map(0xe1, 0xe1).portr("KEY1");
	map(0xe2, 0xe2).portr("KEY2");
	map(0xe3, 0xe3).portr("KEY3");
	map(0xe4, 0xe4).portr("KEY4");
	map(0xe5, 0xe5).portr("START");
	map(0xf0, 0xf0).portr("UNUSED1");
	map(0xf1, 0xf1).portr("UNUSED2");
	map(0xf2, 0xf2).portr("UNUSED3");
	map(0xf3, 0xf3).portr("UNUSED4");
	map(0xf7, 0xf7).portr("COINS");
}


static INPUT_PORTS_START( cultures )
	PORT_START("SW1_A")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "10 Coins / 1 Credit" )
	PORT_DIPSETTING(    0x01, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW1_B")
	PORT_DIPNAME( 0x01, 0x01, "Auto Mode After Reach" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Attract Mode" ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x02, "Partial" )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0x04, 0x04, "Open Hands After Noten" ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "Datsui Count After Continue" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x08, "Not Cleared" )
	PORT_DIPSETTING(    0x00, "Cleared" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW2_A")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Background Music" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW2_B")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Machihai Display" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW2:7" ) // "always off"
	PORT_SERVICE_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:8" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("START")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("UNUSED4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COINS")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) // "Test"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static const gfx_layout gfxlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 1*8, 0*8, 3*8, 2*8, 5*8, 4*8, 7*8, 6*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64,
};

static GFXDECODE_START( gfx_cultures )
	GFXDECODE_ENTRY("bg0", 0, gfxlayout, 0x0000, 16 )
	GFXDECODE_ENTRY("bg1", 0, gfxlayout, 0x1000, 8 )
	GFXDECODE_ENTRY("bg2", 0, gfxlayout, 0x1000, 8 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cultures_state::interrupt)
{
	if (m_irq_enable)
		device.execute().set_input_line(0, HOLD_LINE);
}

void cultures_state::machine_start()
{
	m_prgbank->configure_entries(0, 16, memregion("maincpu")->base(), 0x4000);
	m_okibank->configure_entries(0, 0x200000 / 0x20000, memregion("oki")->base(), 0x20000);
	m_okibank->set_entry(0);

	save_item(NAME(m_irq_enable));
	save_item(NAME(m_bg_rombank));
}

void cultures_state::machine_reset()
{
	m_okibank->set_entry(0);
	m_vramview.select(1);
	m_irq_enable = 0;
	m_bg_rombank[0] = 0;
	m_bg_rombank[1] = 0;
}



void cultures_state::cultures(machine_config &config)
{
	static constexpr XTAL MCLK = 16_MHz_XTAL;

	// basic machine hardware
	Z80(config, m_maincpu, MCLK / 2); // 8.000 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &cultures_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cultures_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(cultures_state::interrupt));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 48*8-1, 0*8, 30*8-1);
	screen.set_screen_update(FUNC(cultures_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_cultures);
	PALETTE(config, "palette").set_format(palette_device::xRGBRRRRGGGGBBBB_bit0, 0x3000/2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", MCLK / 8, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "mono", 0.30);
	oki.set_addrmap(0, &cultures_state::oki_map);
}

/*

Jibun wo Migaku Culture School Mahjong Hen
(c)1994 Face

CPU: Z80
Sound: M6295
OSC: 16.000MHz
EEPROM: 93C46
Custom: FACE AV44 4HA0G (x3)

ROMs:
MA01.U12
PCM.U87
BG0C.U45
BG0C2.U46
BG1C.U80
BG1T.U67
BG2C.U68
BG2T.U79

-----mahjong connector-----
                      empty
amp OSC               BG0C
6295 GAL              BG0C2
G PCM     SS        S
A MA01 G  RR        R
L      A  AA        A
 Z80   L  MM custom M

D GGG
I AAA
P LLL
D
I custom BG2C         BG1C
P custom      custom
93C46    BG1T         BG2T

*/

ROM_START( cultures )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "ma01.u12",     0x000000, 0x040000, CRC(f57417b3) SHA1(9a2a50222f54e5da9bc5c66863b8be16e33b171f) )

	ROM_REGION( 0x400000, "bg0", ROMREGION_ERASE00 )
	ROM_LOAD( "bg0c.u45",     0x000000, 0x200000, CRC(ad2e1263) SHA1(b28a3d82aaa0421a7b4df837814147b109e7d1a5) )
	ROM_LOAD( "bg0c2.u46",    0x200000, 0x100000, CRC(97c71c09) SHA1(ffbcee1d9cb39d0824f3aa652c3a24579113cf2e) )
	// 0x300000 - 0x3fffff empty

	ROM_REGION16_LE( 0x400000, "bg1", ROMREGION_ERASE00 )
	ROM_LOAD( "bg2c.u68",     0x000000, 0x200000, CRC(fa598644) SHA1(532249e456c34f18a787d5a028df82f2170f604d) )
	ROM_LOAD( "bg1t.u67",     0x200000, 0x100000, CRC(d2e594ee) SHA1(a84b5ab62dec1867d433ccaeb1381e7593958cf0) )
	// 0x300000 - 0x3fffff empty

	ROM_REGION16_LE( 0x400000, "bg2", ROMREGION_ERASE00 )
	ROM_LOAD( "bg1c.u80",     0x000000, 0x200000, CRC(9ab99bd9) SHA1(bce41b6f5d83c8262ba8d37b2dfcd5d7a5e7ace7) )
	ROM_LOAD( "bg2t.u79",     0x200000, 0x100000, CRC(0610a79f) SHA1(9fc6b2e5c573ed682b2f7fa462c8f42ff99da5ba) )
	// 0x300000 - 0x3fffff empty

	ROM_REGION( 0x200000, "oki", 0 )
	ROM_LOAD( "pcm.u87",      0x000000, 0x200000, CRC(84206475) SHA1(d1423bd5c7425e121fb4e7845cf57801e9afa7b3) )
ROM_END

} // anonymous namespace


GAME( 1994, cultures, 0, cultures, cultures, cultures_state, empty_init, ROT0, "Face", "Jibun wo Migaku Culture School Mahjong Hen", MACHINE_SUPPORTS_SAVE )
