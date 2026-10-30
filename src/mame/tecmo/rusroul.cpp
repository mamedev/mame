// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Russian Roulette (c) 1988 Tecmo

original misc/vroulet.cpp by Curt Coder & Tomasz Slanina

TODO:
- what's the use of the 0 bitplane in GFX;
- verify ball priority vs. tilemap (needs real HW ref);
- payout, winning hangs in rusroul (sits checking bit 5 of $6001, may require NMI?);
- remaining outputs;

===================================================================================================

    Vegas Roulette
    World Games 1989

    LE1000

           6116  PGM  Z80    8255

                             8255  SW1

    GFX  6116                YM2149   SW3
                                      SW2
                        2148
                        2148
                        2148
                N4
    24MHz


**************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "input.h"

namespace {

class rusroul_state : public driver_device
{
public:
	rusroul_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi_%u", 0)
		, m_nvram(*this, "nvram")
		, m_vram(*this, "vram")
		, m_cram(*this, "cram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_paletteram(*this, "paletteram")
		, m_spriteram(*this, "spriteram")
		, m_prom(*this, "prom")
	{ }

	void rusroul(machine_config &config);
	void vroulet(machine_config &config);

protected:
//	virtual void machine_start() override ATTR_COLD;
//	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<nvram_device> m_nvram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_paletteram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_region_ptr<uint8_t> m_prom;

private:
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void vroulet_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_tile_info);
};

void rusroul_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(rusroul_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

TILE_GET_INFO_MEMBER(rusroul_state::get_tile_info)
{
	int const attr = m_cram[tile_index];
	int const code = m_vram[tile_index] + (BIT(attr, 6, 2) << 8);
	int const color = attr & 0x0f;
	// vroulet: right arrow in "Last 20th DATA" screen
	int const flags = BIT(attr, 5) ? TILE_FLIPY : 0;

	tileinfo.set(0, code, color, flags);
}

uint32_t rusroul_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// overlays ball GFXs thru the PROM, as a crude form of sprite handling (1 entry)
	// TODO: X/Y offsets are guessed
	// (limited game use, just a rotation that could work the other way around)
	// seems to go counterclockwise as per "Last 20th DATA" in vroulet
	const int sprite_x = m_spriteram[1] - 4;
	const int sprite_y = 256 - m_spriteram[0];

	for (int y = 0; y < 16; y++)
	{
		const int dst_y = sprite_y + y;

		if (!cliprect.containsy(dst_y))
			continue;

		for (int x = 0; x < 16; x++)
		{
			const int dst_x = sprite_x + x;

			if (!cliprect.containsx(dst_x))
				continue;

			const uint8_t pen = m_prom[y * 16 + x];

			if (pen)
				bitmap.pix(dst_y, dst_x) = pen;
		}
	}

	return 0;
}

void rusroul_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x8000, 0x8000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x9000, 0x93ff).ram().lw8(NAME([this] (offs_t offset, uint8_t data) {
		m_vram[offset] = data;
		m_tilemap->mark_tile_dirty(offset);
	})).share("vram");
	map(0x9400, 0x97ff).ram().lw8(NAME([this] (offs_t offset, uint8_t data) {
		m_cram[offset] = data;
		m_tilemap->mark_tile_dirty(offset);
	})).share("cram");
	map(0xa000, 0xa001).ram().share("spriteram");
	map(0xb000, 0xb0ff).ram().lw8(
		NAME([this] (offs_t offset, uint8_t data) {
			m_paletteram[offset] = data;
			const u32 pal_entry = offset >> 1;
			const u8 b = pal4bit(BIT(m_paletteram[offset & ~1], 0, 4));
			const u8 g = pal4bit(BIT(m_paletteram[offset |  1], 4, 4));
			const u8 r = pal4bit(BIT(m_paletteram[offset |  1], 0, 4));

			m_palette->set_pen_color(pal_entry, r, g, b);
		})
	).share("paletteram");
	// noisy, either SW bug or something specific ...
	map(0xb100, 0xb100).lw8(NAME([this] (offs_t offset, uint8_t data) {
		if (data)
			logerror("$b100 non-zero write %02x\n", data);
	}));

	// writes: priority mixing or lamps
	// - always 0x0f in rusroul
	// - 0 in vroulet when idle and non-zero when spinning
	map(0xc000, 0xc000).portr("SYSTEM");
}

void rusroul_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("ssg", FUNC(ym2149_device::data_r));
	map(0x00, 0x01).w("ssg", FUNC(ym2149_device::data_address_w));
	map(0x10, 0x13).rw("ppi_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x80, 0x83).rw("ppi_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void rusroul_state::vroulet_map(address_map &map)
{
	main_map(map);
	map(0x8000, 0x8000).noprw();
}

static INPUT_PORTS_START( rusroul )
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x01, 0x00, "SYSTEM" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	// Following two looks related to payout
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2" )
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

	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPNAME( 0x02, 0x02, "IN0" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Token")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Bet 2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Bet 4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bet 6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Bet 8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Bet 10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Bet 15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Bet 20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )
INPUT_PORTS_END

// TODO: merge some with above
static INPUT_PORTS_START( vroulet )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Reset Machine")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("SYSTEM")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_10C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_10C ) )
	PORT_DIPNAME( 0xc0, 0xc0, "Revolutions" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Max Payout Adjust" )
	PORT_DIPSETTING(    0x02, "48" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x04, "Extra Payout Control" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Red & Blue Select" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Winning %" )
	PORT_DIPSETTING(    0xc0, "50%" )
	PORT_DIPSETTING(    0xa0, "60%" )
	PORT_DIPSETTING(    0x80, "65%" )
	PORT_DIPSETTING(    0x60, "70%" )
	PORT_DIPSETTING(    0xe0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x20, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
INPUT_PORTS_END

// bitplane 0 looks left-over, contains "some" GFX which looks meaningless ...
static gfx_layout const layout_8x8_3bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	3,
	{ 1, 2, 3 },
	{ STEP8(0, 4) },
	{ STEP8(0, 8 * 4) },
	8 * 8 * 4
};

static gfx_layout const layout_8x8_1bpp =
{
	8, 8,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(0, 4) },
	{ STEP8(0, 8 * 4) },
	8 * 8 * 4
};

static GFXDECODE_START( gfx_rusroul )
	GFXDECODE_ENTRY( "gfx",  0x0000, layout_8x8_3bpp,   0, 16 )
	GFXDECODE_ENTRY( "gfx",  0x0000, layout_8x8_1bpp,   0x76, 1 )
//	GFXDECODE_ENTRY( "prom", 0x0000, layout_8x8_sprite, 0, 16 )
GFXDECODE_END


void rusroul_state::rusroul(machine_config &config)
{
	// Sharp LH0080A (Z80A)
	Z80(config, m_maincpu, 24_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &rusroul_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &rusroul_state::main_io);
	m_maincpu->set_vblank_int("screen", FUNC(rusroul_state::irq0_line_hold));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);

	// TODO: DSW3 hook
	I8255(config, m_ppi[0]);
	m_ppi[0]->in_pa_callback().set_ioport("IN0");
	m_ppi[0]->in_pb_callback().set_ioport("IN1");

	I8255(config, m_ppi[1]);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_rusroul);
	PALETTE(config, m_palette).set_entries(128);

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*0, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(rusroul_state::screen_update));
	screen.set_palette("palette");

	SPEAKER(config, "mono").front_center();

	ym2149_device &ssg(YM2149(config, "ssg", 24_MHz_XTAL / 16));
	ssg.port_a_read_callback().set_ioport("DSW1");
	ssg.port_b_read_callback().set_ioport("DSW2");
	ssg.add_route(ALL_OUTPUTS, "mono", 0.25);

	// TODO: clock frequency & pin 7 not verified
	OKIM6295(config, "oki",  24_MHz_XTAL / 24, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.25);
}

void rusroul_state::vroulet(machine_config &config)
{
	rusroul(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &rusroul_state::vroulet_map);

	// TODO: remove oki
	// config.device_remove("oki");
}

ROM_START( rusroul )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "5c.5c",    0x0000, 0x8000, CRC(e5eb33bb) SHA1(b36decbcb8561dc91747df142390e0dd36c1c75c) )

	ROM_REGION( 0x8000, "gfx", 0 )
	ROM_LOAD( "9l.9l",    0x0000, 0x8000, CRC(37549f24) SHA1(9c066778a74409779fd27c6cfab9832060eb753f) )

	ROM_REGION( 0x10000, "oki", 0 )
	ROM_LOAD( "3r.3r",    0x0000, 0x10000, CRC(7951189d) SHA1(82b6ab76989ed9f6cd6bc7219f3108d7d3a858f1) )

	ROM_REGION( 0x800, "prom", 0 )
	// FIXED BITS (00000xxx)
	// BADADDR ---xxxxxxxx
	// contains ball shaped GFXs, identical for every $100 bank
	ROM_LOAD( "prom.6p",  0x000, 0x800, CRC(7d315248) SHA1(6834a72d45822cb2f6b801b7a4194cc262abb002) )
ROM_END

ROM_START( vroulet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roul1.bin", 0x0000, 0x2000, CRC(0cff99e5) SHA1(0aa6680c4b8d780d71b3e6c6fe511f86f40abc4c) )
	ROM_LOAD( "roul2.bin", 0x2000, 0x2000, CRC(61924d9f) SHA1(8334d6825ed40e8347909817b8b73be97d23faf8) )
	ROM_LOAD( "roul3.bin", 0x4000, 0x2000, CRC(73dedff6) SHA1(d01c4fc99ac8dc03bd6e0cf779c221d403b2b648) )

	ROM_REGION( 0x8000, "gfx", 0 )
	ROM_LOAD( "roul.gfx", 0x0000, 0x8000, CRC(4e4f46d2) SHA1(efd00e2b564ff4a9013c67ffaaf91124089b310b) )

	ROM_REGION( 0x10000, "oki", ROMREGION_ERASEFF )

	// TODO: unconfirmed for this set
	ROM_REGION( 0x800, "prom", 0 )
	ROM_LOAD( "prom.6p",  0x000, 0x800, BAD_DUMP CRC(7d315248) SHA1(6834a72d45822cb2f6b801b7a4194cc262abb002) )
ROM_END

} // anonymous namespace

GAME( 1988, rusroul, 0,       rusroul, rusroul, rusroul_state, empty_init,   ROT90, "Tecmo",                "Russian Roulette", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1989, vroulet, rusroul, vroulet, vroulet, rusroul_state, empty_init,   ROT90, "bootleg (World Game)", "Vegas Roulette (bootleg)", MACHINE_IMPERFECT_GRAPHICS )
