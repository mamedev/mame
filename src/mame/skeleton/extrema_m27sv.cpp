// license:BSD-3-Clause
// copyright-holders:

/*

Video slots by Extrema running on the Extrema M27-SV PCB

TODO:
- hangs at startup;
\- $9002 becomes 0x11, loops until that becomes 0 then it throws an halt with irqs disabled.
- video (if above bypassed puts some GFX at $e000);
- confirm palette RGB layout;
- Upper bank of VRAM clearly view selected thru a bit in $fe2a;
- ds5000t (view selected too?)
- I/O (looks non-functional even if they are clearly tested);

The main components (at least for M27-SV rev.1.0) are:
Z84C0010PEC Z80 CPU
DALLAS DS5000T
scratched off rectangular 100-pin chip
scratched off square 200-pin chip, Extrema Ukraine 2006
36 MHz XTAL
2x 11.0592S MHz XTAL
Winbond WF19054
ADV7120 RAMDAC
2x CY7C199 RAM (1 near Z80, 1 near GFX ROMs)

*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

class extrema_m27sv_state : public driver_device
{
public:
	extrema_m27sv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_program_rom(*this, "program_rom")
		, m_bank(*this, "bank")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram")
	{ }

	void extrema_m27sv(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_memory_region m_program_rom;
	required_memory_bank m_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	std::unique_ptr<u8[]> m_palette_ram;
	required_shared_ptr<u8> m_vram;

	u8 palette_r(offs_t offset);
	void palette_w(offs_t offset, u8 data);
	void vram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mmio_map(address_map &map) ATTR_COLD;

	u8 m_unk_fe20 = 0;
	u16 m_pal_bank = 0;
};

void extrema_m27sv_state::machine_start()
{
	m_bank->configure_entries(0, 8, m_program_rom->base(), 0x8000);

	save_item(NAME(m_unk_fe20));
	save_item(NAME(m_pal_bank));
}

void extrema_m27sv_state::machine_reset()
{
	m_unk_fe20 = 0;
	m_pal_bank = 0;
	m_bank->set_entry(0);
}

TILE_GET_INFO_MEMBER(extrema_m27sv_state::get_tile_info)
{
	u8 attr = m_vram[(tile_index * 2 + 1) & 0xfff];
	u16 code = m_vram[(tile_index * 2) & 0xfff] + ((attr & 0x0f) << 8);
	u8 color = (attr >> 6) & 3;
	u8 flags = 0;

	if (BIT(attr, 4))
		flags |= TILE_FLIPX;

	if (BIT(attr, 5))
		flags |= TILE_FLIPY;

	tileinfo.set(0, code, color, flags);
}

void extrema_m27sv_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(extrema_m27sv_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_palette_ram = make_unique_clear<u8[]>(0xc00 * 2);

	save_pointer(NAME(m_palette_ram), 0xc00 * 2);
}

uint32_t extrema_m27sv_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void extrema_m27sv_state::vram_w(offs_t offset, u8 data)
{
	m_vram[offset] = data;
	m_tilemap->mark_tile_dirty(offset >> 1);
}

u8 extrema_m27sv_state::palette_r(offs_t offset)
{
	return m_palette_ram[offset + m_pal_bank];
}

void extrema_m27sv_state::palette_w(offs_t offset, u8 data)
{
	m_palette_ram[offset + m_pal_bank] = data;

	u16 pal_offset = offset & 0x3ff;
	// TODO: determine bank order
	u8 r = m_palette_ram[pal_offset + 0x000 + m_pal_bank];
	u8 g = m_palette_ram[pal_offset + 0x400 + m_pal_bank];
	u8 b = m_palette_ram[pal_offset + 0x800 + m_pal_bank];

	m_palette->set_pen_color(pal_offset | (m_pal_bank & 0x400), pal6bit(r), pal6bit(g), pal6bit(b));
}

void extrema_m27sv_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank");
	map(0x8000, 0xdfff).ram();
	map(0xe000, 0xefff).ram().w(FUNC(extrema_m27sv_state::vram_w)).share("vram");
	map(0xf000, 0xfbff).rw(FUNC(extrema_m27sv_state::palette_r), FUNC(extrema_m27sv_state::palette_w));
	map(0xfe00, 0xfeff).m(*this, FUNC(extrema_m27sv_state::mmio_map));
}

// area $fe00-$feff looks shared between memory and I/O maps
// this is a tracer bullet, in case we really need A8-A15 for I/O.
void extrema_m27sv_state::mmio_map(address_map &map)
{
	map(0x08, 0x08).portr("IN0");
	map(0x09, 0x09).portr("IN1");
	map(0x0a, 0x0a).portr("IN2");
	map(0x0b, 0x0b).portr("IN3");

//  map(0x10, 0x10) read then discarded with a pop af

	// x--- ---- test switch?
	// -xxx ---- program bank
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) { return m_unk_fe20; }),
		NAME([this] (offs_t offset, u8 data) {
			m_unk_fe20 = data & 0xff;
			m_bank->set_entry((data & 0x70) >> 4);
			LOG("$fe20 %02x\n", data);
		})
	);

	// x--- ---- palette r/w bank
	// -x-- ---- <unknown>
	// --x- ---- <unknown>
	// ---- -x-- flipped a lot
	// ---- --x- <unknown>
	// ---- ---x <unknown>
	map(0x2a, 0x2a).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_pal_bank = BIT(data, 7) * 0xc00;
			if (data & 0xfb)
				LOG("$fe2a %02x\n", data);
		})
	);

	// goes $40 up to $4d on second init cycle.
	map(0x40, 0x4f).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x60, 0x60).w("aysnd", FUNC(ay8910_device::address_w));
}

void extrema_m27sv_state::io_map(address_map &map)
{
	//map.unmap_value_high();
	map(0xfe00, 0xfeff).m(*this, FUNC(extrema_m27sv_state::mmio_map));
}


static INPUT_PORTS_START( multgmxl )
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "IN0" )
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

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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

	// no DSW on PCB
INPUT_PORTS_END

// TODO: incomplete
static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0, 1) },
	{ STEP8(0, 8) },
	{ STEP8(0, 8*8) },
	8*8*8
};

static GFXDECODE_START( gfx_extrema_m27sv )
	GFXDECODE_ENTRY( "gfx", 0, charlayout, 0, 8 )
GFXDECODE_END


void extrema_m27sv_state::extrema_m27sv(machine_config &config)
{
	Z80(config, m_maincpu, 36_MHz_XTAL / 8 ); // divider not verified. Or maybe uses one of the 11.0592 XTALs?
	m_maincpu->set_addrmap(AS_PROGRAM, &extrema_m27sv_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &extrema_m27sv_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(extrema_m27sv_state::irq0_line_hold));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(extrema_m27sv_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_extrema_m27sv);

	PALETTE(config, "palette").set_entries(0x800);

	SPEAKER(config, "mono").front_center();

	// Winbond WF19054
	AY8910(config, "aysnd", 36_MHz_XTAL / 8); // divider not verified. Or maybe uses one of the 11.0592 XTALs?
}


ROM_START( multgmxl )
	ROM_REGION( 0x40000, "program_rom", 0 )
	ROM_LOAD( "u8_multigamexl.u8", 0x00000, 0x40000, CRC(735a8b87) SHA1(974a3a6c4de41131021b74d280b409d668c87de9) )

	ROM_REGION( 0x8000, "ds5000t", ROMREGION_ERASE00 )
	ROM_LOAD( "internal_nvram", 0x0000, 0x8000, NO_DUMP )
	// 32K bytes internal NVRAM

	ROM_REGION( 0x200000, "gfx", 0 )
	ROM_LOAD16_BYTE( "u10_multigamexl.u10", 0x000001, 0x100000, CRC(d745758e) SHA1(9aeb8fd8e46b2ee0663f436628c95e73057dfc51) )
	ROM_LOAD16_BYTE( "u11_multigamexl.u11", 0x000000, 0x100000, CRC(ca59894e) SHA1(a97afe937d29e4261cc46e41283e6977b20eafc5) )
ROM_END

} // anonymous namespace


GAME( 2006?, multgmxl, 0, extrema_m27sv, multgmxl, extrema_m27sv_state, empty_init, ROT0, "Extrema", "Multi GAME XL", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
