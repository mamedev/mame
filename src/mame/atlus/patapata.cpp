// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

RW93085 (C) NTC CO.      on main PCB

RW93085-SUB (c)NTC CO.LTD 1993 MADE IN JAPAN     on sub (ROM) PCB

various NMK customs including NMK005 (GPIO controller) near dipswitches, difficult to read the rest from PCB pics

developed by NTC on NMK hardware?

video is probably just 2 tilemap layers
maybe close to jalmah.cpp?

*/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/nmk112.h"
#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class patapata_state : public driver_device
{
public:
	patapata_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_bg_videoram(*this, "bg_videoram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_vregs(*this, "videoregs"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void patapata(machine_config &config);

protected:
	void bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void flipscreen_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_fg_videoram;
	required_shared_ptr<uint16_t> m_vregs;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	/* video-related */
	TILEMAP_MAPPER_MEMBER(pagescan);
	tilemap_t    *m_bg_tilemap = nullptr;
	tilemap_t    *m_fg_tilemap = nullptr;
	int m_fg_bank = 0;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
};

void patapata_state::bg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_videoram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(patapata_state::get_bg_tile_info)
{
	int tileno = m_bg_videoram[tile_index];
	int pal = tileno>>12;
	tileinfo.set(0, tileno&0x1fff, pal, 0);
}

void patapata_state::fg_videoram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_videoram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

void patapata_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 0x01);
}

TILE_GET_INFO_MEMBER(patapata_state::get_fg_tile_info)
{
	int bank = m_fg_bank;

	int tileno = m_fg_videoram[tile_index];
	int pal = tileno>>12;

	tileinfo.set(1, (tileno&0x0fff)+(bank*0x1000), pal, 0);
}

TILEMAP_MAPPER_MEMBER(patapata_state::pagescan)
{
	return (col &0xff) * (num_rows>>1) + (row & 0xf) + ((row & 0x10)<<8) + ((col & 0x300) << 5);
//  return (col &0xff) * (num_rows>>1) + (row & 0xf) + ((row & 0x10)<<8) + ((col & 0x100) << 5); // see comment with tilemap creation
}


void patapata_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(patapata_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(patapata_state::pagescan)), 16, 16, 1024, 16*2);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(patapata_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(patapata_state::pagescan)), 16, 16, 1024, 16*2);

// 2nd half of the ram seems unused, maybe it's actually a mirror meaning this would be the correct tilemap sizes
// m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(patapata_state::get_bg_tile_info)), tilemap_mapper_delegate(*this, FUNC(patapata_state::pagescan)), 16, 16, 1024/2, 16*2);
// m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(patapata_state::get_fg_tile_info)), tilemap_mapper_delegate(*this, FUNC(patapata_state::pagescan)), 16, 16, 1024/2, 16*2);

	m_fg_tilemap->set_transparent_pen(0xf);

	save_item(NAME(m_fg_bank));
}

uint32_t patapata_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// vregs
	// 0/1 are fg scroll?  0x0ff0 , 0x07b0  is no scroll
	// 2/3 are bg scroll?
	// 4 is fg bank

	int scrollx = (m_vregs[2]-0xff0)&0xfff;
	int scrolly = (m_vregs[3]-0x7b0)&0xfff;
	if (scrolly&0x200) scrollx += 0x1000;
	scrolly&=0x1ff;

	m_bg_tilemap->set_scrollx(0, scrollx);
	m_bg_tilemap->set_scrolly(0, scrolly);

	scrollx = (m_vregs[0]-0xff0)&0xfff;
	scrolly = (m_vregs[1]-0x7b0)&0xfff;
	if (scrolly&0x200) scrollx += 0x1000;
	scrolly&=0x1ff;

	m_fg_tilemap->set_scrollx(0, scrollx);
	m_fg_tilemap->set_scrolly(0, scrolly);


	if ((m_vregs[4]&0x3) != m_fg_bank)
	{
		m_fg_bank = m_vregs[4]&0x3;
		m_fg_tilemap->mark_all_dirty();
	}


	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	/*
	popmessage("%04x %04x %04x %04x\n%04x %04x %04x %04x",
	    m_vregs[0], m_vregs[1],
	    m_vregs[2], m_vregs[3],
	    m_vregs[4], m_vregs[5],
	    m_vregs[6], m_vregs[7]);
	*/
	return 0;
}

/*

also

[:maincpu] ':maincpu' (00A284): unmapped program memory write to 110400 = 0000 & FFFF
[:maincpu] ':maincpu' (00A284): unmapped program memory write to 110402 = 0000 & FFFF


*/

void patapata_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x100000, 0x100001).portr("IN0");
	map(0x100002, 0x100003).portr("IN1");
	map(0x100008, 0x100009).portr("DSW1");
	map(0x10000a, 0x10000b).portr("DSW2");
	map(0x100015, 0x100015).w(FUNC(patapata_state::flipscreen_w));
	map(0x110000, 0x1103ff).ram().share("videoregs");
	map(0x120000, 0x1205ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x130000, 0x13ffff).ram().w(FUNC(patapata_state::fg_videoram_w)).share("fg_videoram");
	map(0x140000, 0x14ffff).ram().w(FUNC(patapata_state::bg_videoram_w)).share("bg_videoram");
	map(0x150001, 0x150001).rw("oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x150011, 0x150011).rw("oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x150020, 0x15002f).w("nmk112", FUNC(nmk112_device::okibank_w)).umask16(0x00ff);
	map(0x180000, 0x18ffff).ram(); // mainram?
}

static INPUT_PORTS_START( patapata )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reset")
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW1:1" )

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW2:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW2:6,5,4")
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW2:3,2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


static GFXDECODE_START( gfx_patapata )
	GFXDECODE_ENTRY( "tilesa", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x000, 16 )
	GFXDECODE_ENTRY( "tilesb", 0, gfx_8x8x4_col_2x2_group_packed_msb, 0x100, 16 )
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(patapata_state::scanline)
{
	// reads inputs (half-frame interrupt like NMK16?)
	if (param==128) m_maincpu->set_input_line(1, HOLD_LINE);
}

void patapata_state::patapata(machine_config &config)
{
	M68000(config, m_maincpu, 16_MHz_XTAL); // 16 MHz XTAL, 16 MHz CPU
	m_maincpu->set_addrmap(AS_PROGRAM, &patapata_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(patapata_state::irq4_line_hold)); // 1 + 4 valid? (4 main VBL)

	TIMER(config, "scantimer").configure_scanline(FUNC(patapata_state::scanline), "screen", 0, 1);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_patapata);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 60*8-1, 0*8, 44*8-1);
	screen.set_screen_update(FUNC(patapata_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::RRRRGGGGBBBBRGBx, 0x600/2);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki1", 16_MHz_XTAL / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.40); // not verified

	OKIM6295(config, "oki2", 16_MHz_XTAL / 4, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.40); // not verified

	nmk112_device &nmk112(NMK112(config, "nmk112", 0)); // or 212? difficult to read (maybe 212 is 2* 112?)
	nmk112.set_rom0_tag("oki1");
	nmk112.set_rom1_tag("oki2");
}

ROM_START( patapata )
	ROM_REGION( 0x80000, "maincpu", 0 )  /* 68000 code */
	ROM_LOAD16_WORD_SWAP( "rw-93085-7.u132",  0x00000, 0x80000, CRC(f8084e30) SHA1(8ca19fb3d348affbcb89fb4fef0be4614edd14f7) )

	ROM_REGION( 0x080000, "tilesa", 0 ) // bg layer - 1 bank
	ROM_LOAD16_BYTE( "rw-93085-9.u5",    0x000001, 0x040000, CRC(c2e243ff) SHA1(492e25ac1f85ac6f815409ce11de9a1fabab6fc1) )
	ROM_LOAD16_BYTE( "rw-93085-10.u15",  0x000000, 0x040000, CRC(546be459) SHA1(f96b139a1b7c021cd9752d626330ffd6201d7441) )

	ROM_REGION( 0x180000, "tilesb", 0 ) // fg layer - 3 banks
	ROM_LOAD16_BYTE( "rw-93085-17.u9",  0x000001, 0x080000, CRC(e19afa04) SHA1(0511ac94faa549706d729678b4f26b738cf19059) )
	ROM_LOAD16_BYTE( "rw-93085-18.u19", 0x000000, 0x080000, CRC(5cf4582e) SHA1(98a5a274589aa048fa5809d5bb38326e287e6905) )
	ROM_LOAD16_BYTE( "rw-93085-19.u19", 0x100001, 0x040000, CRC(dfd7bdcf) SHA1(02e46da9a8c938daa180a57f4aca04b2fd655ee0) )
	ROM_LOAD16_BYTE( "rw-93085-20.u20", 0x100000, 0x040000, CRC(dd821f74) SHA1(a63e9979db30d130449f689cc6ba8b4c7d25085a) )

	ROM_REGION( 0x100000+0x40000, "oki1", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "rw-93085-5.u22", 0x000000+0x40000, 0x080000, CRC(0c0d2835) SHA1(dc14ebea5f4e0d3f2f8e7bc05e16b8d0f92ce588) )
	ROM_LOAD( "rw-93085-6.u23", 0x080000+0x40000, 0x080000, CRC(882c25d0) SHA1(9cbf21bd5940240440025b4481d96e3db45a676c) )

	ROM_REGION( 0x100000+0x40000, "oki2", 0 ) /* OKIM6295 samples */
	ROM_LOAD( "rw-93085-1.u3",  0x000000+0x40000, 0x080000, CRC(d9776d50) SHA1(06e4d2184f687af8380fcb49ce48ce8ec8091050) )
	ROM_LOAD( "rw-93085-2.u4",  0x080000+0x40000, 0x080000, CRC(3698fafa) SHA1(3de54a990478621271285254544f5382d6fd9ca9) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s131n.u119", 0x000, 0x200, CRC(33f63fc8) SHA1(24c4a1a7c06e546571c77c7dc7bd87c57aa088d7) )
	ROM_LOAD( "n82s135n.u137", 0x000, 0x100, CRC(cb90eedc) SHA1(6577cb1999a90b9209b150cbedde11de9ac30018) )
ROM_END

} // anonymous namespace


// cabinet shows Atlus logo, though there's no copyright on the title screen and PCB is NTC / NMK
GAME( 1993, patapata, 0, patapata, patapata, patapata_state, empty_init, ROT0,  "Atlus", "Pata Pata Panic", MACHINE_SUPPORTS_SAVE )
