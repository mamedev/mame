// license:BSD-3-Clause
// copyright-holders:

/*
Hom Inn 980924-1 PCB
JC-10011A

- MC68000P10
- square 100-pin chip with almost unreadable marking (only 17-2414-01 and 97/09)
- square 84-pin chip with no markings
- 16.000 MHz XTAL
- 12.000 MHz XTAL
- 4x HM6116P-3
- 2x LP6264D-70LL
- M5M82C255
- OKIM6295 or clone (markings unreadable, but ROM content reveals it)
- bank of 8 switches (with 2 unpopulated spaces for more banks)


TODO:
- remove PPI1 port B hack;
- correct IRQs (related to the above?);
- correct palette;
- Oki banking;
- switches;
- lamps?;
- simplify descrambling.
*/


#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_PORTS     (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class hominn_980924_state : public driver_device
{
public:
	hominn_980924_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fg_tileram(*this, "fg_tileram"),
		m_bg_datarom(*this, "bgdata")
	{ }

	void qxjl(machine_config &config) ATTR_COLD;

	void init_qxjl() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<uint16_t> m_fg_tileram;
	required_region_ptr<uint16_t> m_bg_datarom;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	uint8_t m_bg_bank = 0;

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	void fg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void counter_w(uint8_t data);

	void program_map(address_map &map) ATTR_COLD;
};


void hominn_980924_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hominn_980924_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(hominn_980924_state::get_bg_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 16);

	m_fg_tilemap->set_transparent_pen(0x0f);
}

TILE_GET_INFO_MEMBER(hominn_980924_state::get_fg_tile_info)
{
	uint16_t const tile = m_fg_tileram[tile_index] & 0xfff;
	uint16_t const color = (m_fg_tileram[tile_index] & 0xf000) >> 12;

	tileinfo.set(0, tile, color, 0);
}

TILE_GET_INFO_MEMBER(hominn_980924_state::get_bg_tile_info)
{
	uint16_t const tile = m_bg_datarom[tile_index | ((m_bg_bank & 0x7f) * 0x200)] & 0x7ff;
	uint16_t const color = (m_bg_datarom[tile_index | ((m_bg_bank & 0x7f) * 0x200)] & 0xf000) >> 12;

	tileinfo.set(1, tile, color, 0);
}

void hominn_980924_state::fg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_fg_tileram[offset]);
	m_fg_tilemap->mark_tile_dirty(offset);
}

uint32_t hominn_980924_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


void hominn_980924_state::machine_start()
{
	save_item(NAME(m_bg_bank));
}

void hominn_980924_state::counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));

	if (data & 0xfe)
		logerror("%s unknown counter_w bits set: %02x\n", machine().describe_context(), data);
}


void hominn_980924_state::program_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x060000, 0x067fff).ram().share("nvram"); // second half only read at start?
	map(0x080000, 0x080007).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x084000, 0x084007).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x088000, 0x088001).portr("SERVICE");
	map(0x08c001, 0x08c001).w(FUNC(hominn_980924_state::counter_w));
	map(0x090001, 0x090001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	// map(0x09c001, 0x09c001); // writes only 0xf0 or 0xb0?
	map(0x0a0000, 0x0a0fff).ram().w(FUNC(hominn_980924_state::fg_tileram_w)).share(m_fg_tileram);
	map(0x0c0000, 0x0c03ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0x0c0400, 0x0c07ff).ram();
	map(0x0e0001, 0x0e0001).lw8(NAME([this] (uint8_t data) { m_bg_bank = data; m_bg_tilemap->mark_all_dirty(); }));
}


static INPUT_PORTS_START( qxjl )
	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE(0x10, IP_ACTIVE_LOW)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:6") // title change
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
	GFXDECODE_ENTRY( "fgtiles", 0, gfx_8x8x4_packed_msb, 0, 16 )
	GFXDECODE_ENTRY( "bgtiles", 0, gfx_16x16x4_packed_lsb, 0x100, 16 )
GFXDECODE_END


void hominn_980924_state::qxjl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL); // TODO: XTAL could also be the 16 MHz one
	m_maincpu->set_addrmap(AS_PROGRAM, &hominn_980924_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(hominn_980924_state::irq2_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// 82C255 (actual chip on PCB) is equivalent to two 8255s
	i8255_device &ppi0(I8255(config, "ppi0"));
	ppi0.in_pa_callback().set_ioport("P1");
	ppi0.in_pb_callback().set_ioport("P2");
	ppi0.in_pc_callback().set_ioport("DSW");
	ppi0.out_pa_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI0 port A out %02x\n", machine().describe_context(), data); });
	ppi0.out_pb_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI0 port B out %02x\n", machine().describe_context(), data); });
	ppi0.out_pc_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI0 port C out %02x\n", machine().describe_context(), data); });


	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.in_pa_callback().set([this] () { LOGPORTS("%s: PPI1 port A in\n", machine().describe_context()); return uint8_t(0xff); });
	ppi1.in_pb_callback().set([this] () { return (machine().rand() & 0x01) | 0xfe; }); // TODO: won't boot otherwise
	ppi1.in_pc_callback().set([this] () { LOGPORTS("%s: PPI1 port C in\n", machine().describe_context()); return uint8_t(0xff); });
	ppi1.out_pa_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port A out %02x\n", machine().describe_context(), data); });
	ppi1.out_pb_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port B out %02x\n", machine().describe_context(), data); });
	ppi1.out_pc_callback().set([this] (uint8_t data) { LOGPORTS("%s: PPI1 port C out %02x\n", machine().describe_context(), data); });

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(hominn_980924_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx);

	PALETTE(config, "palette").set_format(palette_device::xGRB_555, 0x200); // TODO: incorrect

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 16_MHz_XTAL / 16, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // XTAL could also be the 12 MHz one, divider and pin 7 not verified
}


// 千禧接龙 (Qiānxǐ Jiēlóng)
ROM_START( qxjl )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1.u17", 0x00000, 0x20000, CRC(50c0d1fb) SHA1(4d535c8e3032e651ed0d9b10206530fa5ddebf85) )
	ROM_LOAD16_BYTE( "2.u16", 0x00001, 0x20000, CRC(36eb936c) SHA1(348f842928177ab72ee62d93a9fb74e9d0cf5fb1) )

	ROM_REGION( 0x20000, "fgtiles", 0 )
	ROM_LOAD( "3.uz10", 0x00000, 0x20000, CRC(35cbe0cd) SHA1(f2c11d6e12097e281df6a6bd2ce35c15ef482377) )

	ROM_REGION( 0x40000, "bgtiles", 0 )
	ROM_LOAD( "5.ub9", 0x00000, 0x40000, CRC(f12e5c72) SHA1(e4d791bb623a10ee5041f8b52ba82b3e5bb7f5b7) )

	ROM_REGION16_BE( 0x10000, "bgdata", 0 )
	ROM_LOAD( "4.ub4", 0x00000, 0x10000, CRC(7b44beed) SHA1(9cbdb5dc388665ded2c46d29e19ebcde194e7bc1) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "6.u31", 0x00000, 0x80000, CRC(fad9be9f) SHA1(d58a51b09560edffebe52ec22080a29767273ed3) )
ROM_END


void hominn_980924_state::init_qxjl()
{
	// TODO: reduce this monstrosity

	uint8_t *rom = memregion("maincpu")->base();
	std::vector<uint8_t> buffer(0x40000);

	memcpy(&buffer[0], rom, 0x40000);

	for (int i = 0; i < 0x40000; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 8, 3, 14, 13, 12, 15, 10, 11, 9, 2, 7, 16, 5, 4, 17, 6, 1, 0)];

	for (int i = 0; i < 0x40000; i += 0x100)
	{
		std::swap_ranges(&rom[i + 0x10], &rom[i + 0x14], &rom[i + 0x40]);
		std::swap_ranges(&rom[i + 0x18], &rom[i + 0x1c], &rom[i + 0x48]);
		std::swap_ranges(&rom[i + 0x30], &rom[i + 0x34], &rom[i + 0x60]);
		std::swap_ranges(&rom[i + 0x38], &rom[i + 0x3c], &rom[i + 0x68]);
		std::swap_ranges(&rom[i + 0x44], &rom[i + 0x48], &rom[i + 0x84]);
		std::swap_ranges(&rom[i + 0x4c], &rom[i + 0x50], &rom[i + 0x8c]);
		std::swap_ranges(&rom[i + 0x54], &rom[i + 0x58], &rom[i + 0x94]);
		std::swap_ranges(&rom[i + 0x5c], &rom[i + 0x60], &rom[i + 0x9c]);
		std::swap_ranges(&rom[i + 0x64], &rom[i + 0x68], &rom[i + 0xa4]);
		std::swap_ranges(&rom[i + 0x6c], &rom[i + 0x70], &rom[i + 0xac]);
		std::swap_ranges(&rom[i + 0x74], &rom[i + 0x78], &rom[i + 0xb4]);
		std::swap_ranges(&rom[i + 0x7c], &rom[i + 0x80], &rom[i + 0xbc]);
	}

	memcpy(&buffer[0], rom, 0x40000);

	for (int i = 0; i < 0x40000; i++)
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 11, 15, 14, 13, 12, 9, 10, 8, 7, 16, 6, 5, 4, 3, 2, 1, 0)];
}

} // anonymous namespace


GAME( 199?, qxjl, 0, qxjl, qxjl, hominn_980924_state, init_qxjl, ROT0, "Hom Inn", "Qianxi Jielong", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_NOT_WORKING )
