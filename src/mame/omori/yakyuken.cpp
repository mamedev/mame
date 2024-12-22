// license:BSD-3-Clause
// copyright-holders: hap

/*

Bootleg of Omori's 野球拳 - The Yakyuken

It's a strip rock-paper-scissors game in a cocktail cabinet, each side has
7 buttons. One of the buttons apparently is for relinquishing controls to
the other side.

PCB is marked 20282 and LC (stands for "lato componenti", so components side)
with a small riser board marked W 15482 plugged into one of the main CPU ROMs'
sockets

Main components are:
SGS Z80CPUB1 main CPU (clock measured 3.07 MHz)
18.432 MHz XTAL
SGS Z80CPUB1 audio CPU (clock measured 1.53 MHz)
AY-3-8910 sound chip
MK4802 RAM (near audio CPU)
6x 2114 RAM (near GFX ROMs)
Bank of 8 switches

The riser board has a pair of HM4334 1K*4 static RAMs and a quad 2-input NAND gate.

TODO:
- dump/add Omori's version, it's assumed it will have a title screen, as seen
  inside the gfx roms (OEC logo is still in there too)
- find out win rate dipswitch values, or is it max payout rate?
- doesn't it have a hopper? or maybe the bootleg version removed that?
- game sometimes leaves gaps when the lady is undressing
- colors aren't 100% correct (see i.e. the stripes in the curtains), reference video:
  https://www.youtube.com/watch?v=zTOFIhuwR2w

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class yakyuken_state : public driver_device
{
public:
	yakyuken_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_ay(*this, "ay"),
		m_vram(*this, "vram", 0x400*2, ENDIANNESS_LITTLE)
	{ }

	void yakyuken(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<ay8910_device> m_ay;

	memory_share_creator<uint16_t> m_vram;

	uint8_t m_ay_data = 0;
	tilemap_t *m_tilemap = nullptr;

	TILEMAP_MAPPER_MEMBER(tilemap_scan_rows);
	TILE_GET_INFO_MEMBER(get_tile_info);

	void vram_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_program_map(address_map &map) ATTR_COLD;
	void sound_program_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
};

void yakyuken_state::machine_start()
{
	save_item(NAME(m_ay_data));
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

void yakyuken_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		palette.set_pen_color(i, pal1bit(BIT(i, 0)), pal1bit(BIT(i, 1)), pal1bit(BIT(i, 2)));

		// second half is brighter
		palette.set_pen_color(i | 8, pal1bit(BIT(i, 0)) | 0x80, pal1bit(BIT(i, 1)) | 0x80, pal1bit(BIT(i, 2)) | 0x80);
	}
}

TILEMAP_MAPPER_MEMBER(yakyuken_state::tilemap_scan_rows)
{
	row += 2;
	col -= 1;

	// upper 2 rows are left and right columns
	if (col & 0x20)
		return (col & 1) << 5 | row;
	else
		return row << 5 | col;
}

TILE_GET_INFO_MEMBER(yakyuken_state::get_tile_info)
{
	int const code = m_vram[tile_index];
	tileinfo.set(0, code, 0, 0);
}

static GFXDECODE_START( gfx_yakyuken )
	GFXDECODE_ENTRY( "tiles", 0, gfx_8x8x4_planar, 0, 1 )
GFXDECODE_END


void yakyuken_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(yakyuken_state::get_tile_info)), tilemap_mapper_delegate(*this, FUNC(yakyuken_state::tilemap_scan_rows)), 8, 8, 34, 28);
}

void yakyuken_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset & 0x3ff] = (offset >> 2 & 0x300) | data;
	m_tilemap->mark_tile_dirty(offset & 0x3ff);
}

uint32_t yakyuken_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

void yakyuken_state::main_program_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x6400, 0x67ff).ram().share("nvram");
	map(0x7000, 0x73ff).select(0xc00).w(FUNC(yakyuken_state::vram_w));
}

void yakyuken_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("SW");
	map(0x01, 0x01).portr("IN0");
	map(0x02, 0x02).portr("IN1");
	map(0x03, 0x03).portr("IN2");
	map(0x17, 0x17).lw8(NAME([this] (uint8_t data) { flip_screen_set(BIT(data, 0)); }));
	map(0x30, 0x30).w("soundlatch", FUNC(generic_latch_8_device::write));
}

void yakyuken_state::sound_program_map(address_map &map)
{
	map(0x0000, 0x07ff).rom();
	map(0x2000, 0x23ff).ram();
}

void yakyuken_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x01, 0x01).w("soundlatch", FUNC(generic_latch_8_device::clear_w));
	map(0x02, 0x02).lw8(NAME([this] (uint8_t data) { m_ay_data = data; }));
	map(0x03, 0x03).lw8(NAME([this] (uint8_t data) { m_ay->write_bc1_bc2(data & 3, m_ay_data); }));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( yakyuken )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // 100 in book-keeping
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // 500 in book-keeping
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // 1000 in book-keeping
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("P1 Control")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("P2 Control")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) // only works without credits inserted
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1 Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gu (rock)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Choki (scissors)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Pa (paper)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Take Score")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_COCKTAIL PORT_NAME("P2 Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Take Score")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(    0x03, "A 1C / B 5C / C 10C" )
	PORT_DIPSETTING(    0x02, "A 2C / B 10C / C 20C" )
	PORT_DIPSETTING(    0x01, "A 4C / B 20C / C 40C" )
	PORT_DIPSETTING(    0x00, "A 5C / B 25C / C 50C" )
	PORT_DIPNAME( 0x04, 0x04, "Max Bet" )               PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "30" )
	PORT_DIPNAME( 0x38, 0x38, "Win Rate" )              PORT_DIPLOCATION("SW:4,5,6")
	PORT_DIPSETTING(    0x38, "?%" )
	PORT_DIPSETTING(    0x30, "?%" )
	PORT_DIPSETTING(    0x28, "?%" )
	PORT_DIPSETTING(    0x20, "?%" )
	PORT_DIPSETTING(    0x18, "?%" )
	PORT_DIPSETTING(    0x10, "?%" )
	PORT_DIPSETTING(    0x08, "?%" )
	PORT_DIPSETTING(    0x00, "100%" ) // test
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine config
 *
 *************************************/

void yakyuken_state::yakyuken(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 3 / 2); // 3.072 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &yakyuken_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &yakyuken_state::main_io_map);
	m_maincpu->set_periodic_int(FUNC(yakyuken_state::irq0_line_hold), attotime::from_hz(4*60));

	Z80(config, m_audiocpu, 18.432_MHz_XTAL / 3 / 4); // 1.536 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &yakyuken_state::sound_program_map);
	m_audiocpu->set_addrmap(AS_IO, &yakyuken_state::sound_io_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(34*8, 28*8);
	m_screen->set_visarea(0*8, 34*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(yakyuken_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_yakyuken);
	PALETTE(config, "palette", FUNC(yakyuken_state::palette), 0x10);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	AY8910(config, m_ay, 18.432_MHz_XTAL / 3 / 4); // 1.536 MHz
	m_ay->add_route(ALL_OUTPUTS, "mono", 0.35);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( yakyuken )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "4210.bt1",  0x0000, 0x1000, CRC(6cf83735) SHA1(3d0a978adb1c1b4526fa00dedd08e2879f4af283) )
	ROM_LOAD( "4211.bg12", 0x1000, 0x1000, CRC(098bf9ff) SHA1(17da91457a6e8154e09361d0600b37156c05f7c2) )
	ROM_LOAD( "4212.bg13", 0x2000, 0x1000, CRC(6d02e421) SHA1(489a822a52db39f348282ba92fb1c1d3cbc68710) )
	ROM_LOAD( "4213.bg14", 0x3000, 0x0800, CRC(364891c8) SHA1(220cae58877cb6a80d01305375a5945c7dc5cfcc) )

	ROM_REGION( 0x800, "audiocpu", 0 )
	ROM_LOAD( "4209.bg11", 0x000, 0x800, CRC(44b2b7d1) SHA1(672931ff572ac6361b493dc9a49f6146bdc26b78) )

	ROM_REGION( 0x8000, "tiles", 0 )
	ROM_LOAD( "4203.r1", 0x0000, 0x1000, CRC(8769aad5) SHA1(71f3d22e8e0006ba89329ac4a48f09e11ab67875) )
	ROM_LOAD( "4207.r2", 0x1000, 0x1000, CRC(9b78e95e) SHA1(2f3fdcc3bb92b2eb3a967de39ffe4eee74cac8e0) )
	ROM_LOAD( "4202.g1", 0x2000, 0x1000, CRC(a45d5258) SHA1(9080c51b2dc5d6bc4d01cc29deed0e2a5ea78dbd) )
	ROM_LOAD( "4206.g2", 0x3000, 0x1000, CRC(e18d50f7) SHA1(c922a019c13c904701abe5a9e42be955d80a7ecb) )
	ROM_LOAD( "4201.b1", 0x4000, 0x1000, CRC(838726ab) SHA1(5bcfb3c6badc8f7b7bea17a228137e4bff39a0e5) )
	ROM_LOAD( "4205.b2", 0x5000, 0x1000, CRC(58efc253) SHA1(b3344df68c665da996f3332f43030a664931db80) )
	ROM_LOAD( "4204.t1", 0x6000, 0x1000, CRC(da77a765) SHA1(e8626548909b5e735cdb603964324482848ce476) )
	ROM_LOAD( "4208.t2", 0x7000, 0x1000, CRC(be97d733) SHA1(ff3b199c8d1203d9d6c0060f217bbd7de32a8152) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, yakyuken, 0, yakyuken, yakyuken, yakyuken_state, empty_init, ROT0, "bootleg", "The Yakyuken", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
