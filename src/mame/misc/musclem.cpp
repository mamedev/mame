// license:BSD-3-Clause
// copyright-holders:Phil Bennett
/***************************************************************************

    Inter Geo Muscle Master hardware

    driver by Phil Bennett

    Known bugs:
        * Music does not immediately play on stage 1 following 'Go!'

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"

#include "speaker.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

/*************************************
 *
 *  Definitions
 *
 *************************************/

class musclem_state : public driver_device
{
public:
	musclem_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_vram(*this, "vram%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_oki(*this, "oki%u", 1U)
	{ }

	void musclem(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr_array<uint16_t, 3> m_vram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device_array<okim6295_device, 2> m_oki;

	uint16_t m_paletteram[0x400];
	tilemap_t * m_tilemap[3];

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template <int Which> TILE_GET_INFO_MEMBER(get_tile_info);
	template <int Which> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	template <int Base> void palette_w(offs_t offset, u16 data, u16 mem_mask);

	void oki1_bank_w(uint16_t data);
	void lamps_w(uint16_t data);

	void main_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Video hardware
 *
 *************************************/

void musclem_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(musclem_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(musclem_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(musclem_state::get_tile_info<2>)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);

	m_tilemap[0]->set_scrolldx(16, 0);
	m_tilemap[2]->set_scrolldx(1, 0);

	m_palette->basemem().set(m_paletteram, 0x400, 16, ENDIANNESS_LITTLE, 2);

	save_item(NAME(m_paletteram));
}

uint32_t musclem_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap[2]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(screen, bitmap, cliprect);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

void musclem_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (uint32_t i = 0; i < 0x200; ++i)
	{
		uint16_t code = m_spriteram[0 + i];

		if (code == 0)
			continue;

		int16_t sy = (int16_t)((m_spriteram[0x200 + i] & 0x1ff) << 7) >> 7;
		int16_t sx = (m_spriteram[0x400 + i] & 0x3ff);
		sx -= 1;

		m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, 0, 0, 0, sx, sy, 0);

		// Handle wraparound
		if (sx > 512 - 16)
			m_gfxdecode->gfx(3)->transpen(bitmap, cliprect, code, 0, 0, 0, sx - 512, sy, 0);
	}
}

template <int Which>
TILE_GET_INFO_MEMBER(musclem_state::get_tile_info)
{
	int data = m_vram[Which][tile_index];
	tileinfo.set(Which, data, 0, 0);
}

template <int Which>
void musclem_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_vram[Which][offset] = data;
	m_tilemap[Which]->mark_tile_dirty(offset);
}

template <int Base>
void musclem_state::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_palette->write16(Base + offset, data, mem_mask);
}



/*************************************
 *
 *  Misc
 *
 *************************************/

void musclem_state::oki1_bank_w(uint16_t data)
{
	m_oki[0]->set_rom_bank(data & 3);
}


void musclem_state::lamps_w(uint16_t data)
{
	// ........ ....x... - Lamp 1 (active low)
	// ........ ..x..... - Lamp 2 (active low)
}



/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( musclem )
	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Demo Lamp" ) PORT_DIPLOCATION("DSW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unused ) ) PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("PORT1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Pump sensor 1")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Pump sensor 2")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pump sensor 3")
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PORT2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0xfc00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void musclem_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x20ffff).ram();
	map(0x800000, 0x8007ff).ram().w(FUNC(musclem_state::vram_w<0>)).share("vram0");
	map(0x840000, 0x840fff).ram().share("spriteram");
	map(0x880000, 0x8807ff).ram().w(FUNC(musclem_state::vram_w<1>)).share("vram1");
	map(0x8c0000, 0x8c07ff).ram().w(FUNC(musclem_state::vram_w<2>)).share("vram2");
	map(0x900000, 0x9001ff).w(FUNC(musclem_state::palette_w<0>));
	map(0x900400, 0x9005ff).w(FUNC(musclem_state::palette_w<0x100>));
	map(0x900800, 0x9009ff).w(FUNC(musclem_state::palette_w<0x200>));
	map(0x900c00, 0x900dff).w(FUNC(musclem_state::palette_w<0x300>));
	map(0xc00000, 0xc00001).portr("DSW");
	map(0xc00080, 0xc00081).lw16(NAME([this](u16 data) { m_tilemap[1]->set_scrollx(data); }));
	map(0xc00082, 0xc00083).lw16(NAME([this](u16 data) { m_tilemap[1]->set_scrolly(data); }));
	map(0xc00084, 0xc00085).lw16(NAME([this](u16 data) { m_tilemap[2]->set_scrollx(data); }));
	map(0xc00086, 0xc00087).lw16(NAME([this](u16 data) { m_tilemap[2]->set_scrolly(data); }));
	map(0xc00088, 0xc00089).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0xc0008a, 0xc0008b).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write)).umask16(0x00ff);
	map(0xc0008c, 0xc0008d).w(FUNC(musclem_state::oki1_bank_w));
	map(0xc0008e, 0xc0008f).nopw(); // Written only once with 0
	map(0xc00090, 0xc00091).portr("PORT1");
	map(0xc00092, 0xc00093).portr("PORT2");
	map(0xc00094, 0xc00095).w(FUNC(musclem_state::lamps_w));
}



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static GFXDECODE_START( gfx_musclem )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x8_raw, 0x000, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x8_raw, 0x200, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, gfx_16x16x8_raw, 0x300, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, gfx_16x16x8_raw, 0x100, 1 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void musclem_state::musclem(machine_config &config)
{
	/* Basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &musclem_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(musclem_state::irq4_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(14'318'181) / 2, 440, 16, 328, 273, 0, 240);

	screen.set_screen_update(FUNC(musclem_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_musclem);

	PALETTE(config, "palette").set_format(palette_device::xBBBBBGGGGGRRRRR, 0x400);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki[0], XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
	OKIM6295(config, m_oki[1], XTAL(4'000'000) / 4, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( musclem )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u-71.bin",   0x000000, 0x20000, CRC(da0648ad) SHA1(e7de583d5e6de4e2f0d617527d168fe691243728) )
	ROM_LOAD16_BYTE( "u-67.bin",   0x000001, 0x20000, CRC(77f4b6ad) SHA1(8de11378903d48f0c57c6dde3923d8c5fa614ccc) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u-148.bin", 0x000000, 0x100000, CRC(97aa308a) SHA1(54b2f5a25dcd3c35330f384a9010fbf75a72b8d0) )
	ROM_LOAD16_BYTE( "u-150.bin", 0x000001, 0x100000, CRC(0c7247bd) SHA1(cab4f83f56ac6567feb27c397d31116c213d0a86) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "u-129.bin", 0x000000, 0x100000, CRC(8268bba6) SHA1(bef3ee971ddf452346c6a7877e1aba577cdaf82c) )
	ROM_LOAD16_BYTE( "u-130.bin", 0x000001, 0x100000, CRC(be4755e6) SHA1(66a7444bad5def32d44031680a4e8e2f8c1bdc81) )

	ROM_REGION( 0x100000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "u-134.bin", 0x000000, 0x080000, CRC(79c8f776) SHA1(830a3905fc4ae88af48946ffc9bc0d58a8906d81) )
	ROM_LOAD16_BYTE( "u-135.bin", 0x000001, 0x080000, CRC(56b89595) SHA1(ea6efac73857189faa1aa46a31e480770b1b14c7) )

	ROM_REGION( 0x200000, "gfx4", 0 )
	ROM_LOAD32_BYTE( "u-21.bin", 0x000000, 0x80000, CRC(efc2ba0d) SHA1(a72e5943dc71870f7c7eec30015d38ab06ed1846) )
	ROM_LOAD32_BYTE( "u-23.bin", 0x000001, 0x80000, CRC(a8f96912) SHA1(69bc5c7c9a391e8a4b5b260bf9f05ffd44b9cadc) )
	ROM_LOAD32_BYTE( "u-25.bin", 0x000002, 0x80000, CRC(711563c0) SHA1(f0ddd102e5c6dc7286bbeac868c3f08012c54abd) )
	ROM_LOAD32_BYTE( "u-27.bin", 0x000003, 0x80000, CRC(5d6026f3) SHA1(2afc247c7c6129c184c3ae2f98a39781183c99a5) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "u-173.bin", 0x00000, 0x100000, CRC(d93bc7c2) SHA1(bfa3e3936e6fec4f94bfc0bf53cf8b04c085dc92) )

	ROM_REGION( 0x040000, "oki2", 0 )
	ROM_LOAD( "u-177.bin", 0x00000, 0x40000, CRC(622c7b2f) SHA1(6b1721b80f71e5a5b5463c3320efa2e400fd9eac) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1997, musclem, 0, musclem, musclem, musclem_state, empty_init, ROT0, "Inter Geo", "Muscle Master", MACHINE_SUPPORTS_SAVE )
