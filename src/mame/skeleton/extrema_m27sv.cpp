// license:BSD-3-Clause
// copyright-holders: Angelo Salese

/**************************************************************************************************

Video slots by Extrema running on the Extrema M27-SV PCB

TODO:
- Emulate DS5000T properly (i8051 dump or HLE + DS1215).
- confirm palette RGB layout;
- Understand how attribute RAM view select really works;
- Extract "Hopper Service" password;
- Extract "Rent" password (under preferences -> Page 2C);

===================================================================================================

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

**************************************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
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
		, m_work_ram(*this, "work_ram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_vram(*this, "vram")
		, m_aram(*this, "aram")
		, m_aram_view(*this, "aram_view")
		, m_nvram_view(*this, "nvram_view")
		, m_nvram(*this, "nvram")
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
	required_shared_ptr<u8> m_work_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	std::unique_ptr<u8[]> m_palette_ram;
	required_shared_ptr<u8> m_vram;
	required_shared_ptr<u8> m_aram;
	memory_view m_aram_view;
	memory_view m_nvram_view;
	// TODO: temporary
	required_device<nvram_device> m_nvram;
	std::unique_ptr<u8[]> m_nvram_ptr;

	u8 palette_r(offs_t offset);
	void palette_w(offs_t offset, u8 data);
	void vram_w(offs_t offset, u8 data);
	void aram_w(offs_t offset, u8 data);

	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	tilemap_t *m_tilemap;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mmio_map(address_map &map) ATTR_COLD;

	u8 m_unk_fe20 = 0;
	u8 m_unk_fe2a = 0;
	u16 m_pal_bank = 0;
};

void extrema_m27sv_state::machine_start()
{
	m_bank->configure_entries(0, 8, m_program_rom->base(), 0x8000);
	m_nvram_ptr = make_unique_clear<u8[]>(0x2000);
	m_nvram->set_base(&m_nvram_ptr[0], 0x2000);

	save_item(NAME(m_unk_fe20));
	save_item(NAME(m_unk_fe2a));
	save_item(NAME(m_pal_bank));
}

void extrema_m27sv_state::machine_reset()
{
	m_unk_fe20 = 0;
	m_unk_fe2a = 0;
	m_pal_bank = 0;
	m_nvram_ptr[0x1002] = 0xff;
	m_bank->set_entry(0);
	m_nvram_view.disable();
}

TILE_GET_INFO_MEMBER(extrema_m27sv_state::get_tile_info)
{
	u8 attr = m_vram[(tile_index * 2 + 1) & 0xfff];
	u16 code = m_vram[(tile_index * 2) & 0xfff] + ((attr & 0x0f) << 8);
	code |= (m_aram[(tile_index * 2) + 1] & 0x7) << 12;
	u8 color = (attr >> 6) & 3;
	u8 flags = 0;

	if (BIT(attr, 4))
		flags |= TILE_FLIPX;

	if (BIT(attr, 5))
		flags |= TILE_FLIPY;

	tileinfo.set(1, code, color, flags);
}

void extrema_m27sv_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(extrema_m27sv_state::get_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 64, 32);

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

void extrema_m27sv_state::aram_w(offs_t offset, u8 data)
{
	m_aram[offset] = data;
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
	map(0x8000, 0xdfff).ram().share("work_ram");
	map(0x8000, 0xdfff).view(m_nvram_view);
	m_nvram_view[0](0x8000, 0x9fff).lrw8(
		NAME([this] (offs_t offset) { return m_nvram_ptr[offset]; }),
		NAME([this] (offs_t offset, u8 data) { m_nvram_ptr[offset] = data; })
	);
	map(0xe000, 0xefff).ram().w(FUNC(extrema_m27sv_state::vram_w)).share("vram");
	map(0xe000, 0xefff).view(m_aram_view);
	m_aram_view[0](0xe000, 0xefff).ram().w(FUNC(extrema_m27sv_state::aram_w)).share("aram");
	map(0xf000, 0xfbff).rw(FUNC(extrema_m27sv_state::palette_r), FUNC(extrema_m27sv_state::palette_w));
	map(0xfe00, 0xfeff).m(*this, FUNC(extrema_m27sv_state::mmio_map));
}

// area $fe00-$feff looks shared between memory and I/O maps
// this is a tracer bullet, in case we really need A8-A15 for I/O.
void extrema_m27sv_state::mmio_map(address_map &map)
{
	map(0x02, 0x03).nopw(); // lamps and misc outputs

	map(0x08, 0x08).portr("IN0");
	map(0x09, 0x09).portr("IN1");
	map(0x0a, 0x0a).portr("IN2");
	map(0x0b, 0x0b).portr("IN3");

	map(0x10, 0x10).nopr(); // irq ack

	// x--- ---- NVRAM view + bus grant for DS5000T?
	// -xxx ---- program bank
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) { return m_unk_fe20; }),
		NAME([this] (offs_t offset, u8 data) {
			m_unk_fe20 = data & 0xff;
			if (BIT(data, 7))
				m_nvram_view.disable();
			else
				m_nvram_view.select(0);
			m_bank->set_entry((data & 0x70) >> 4);
			LOG("$fe20 %02x\n", data);
		})
	);

	// x--- ---- palette r/w bank
	// -xx- ---- Attribute VRAM select?
	// ---- -x-- execution latch for DS5000T?
	// ---- --x- <unknown>
	// ---- ---x <unknown>
	map(0x2a, 0x2a).lw8(
		NAME([this] (offs_t offset, u8 data) {
			m_pal_bank = BIT(data, 7) * 0xc00;

			// TODO: not perfect, causes attribute drift in places
			if ((data & 0x60) == 0x60)
				m_aram_view.select(0);
			else
				m_aram_view.disable();
			if (!BIT(data, 2) && BIT(m_unk_fe2a, 2) && !BIT(m_unk_fe20, 7))
			{
				// HACK: enough to make it enter in some service
				m_nvram_ptr[0x1002] = 0;
				m_work_ram[0x1063] = 0x01;
				m_work_ram[0x1062] = 0x28;
				LOG("Transfer from DS5000T?\n");
			}
			m_unk_fe2a = data;

			if (data & 0x7b)
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

// TODO: coins (depends on proper NVRAM initialized?)
static INPUT_PORTS_START( multgmxl )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // + High
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // + Low
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) // "Cash Out Key"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Cash Door SW")

	PORT_START("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Menu Key")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME("Booking Key")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Service Key")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Main Door Key")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Logic Door Key")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Full Key")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

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

// TODO: weird, it really replicates the GFXs with minor mods, why?
static const gfx_layout charlayout =
{
	4, 8,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(0, 2) },
	{ 0, 2*8, 4*8, 6*8 },
	{ STEP8(0, 8*8) },
	8*8*8
};

static const gfx_layout charlayout2 =
{
	4, 8,
	RGN_FRAC(1, 1),
	8,
	{ STEP8(1, 2) },
	{ 0, 2*8, 4*8, 6*8 },
	{ STEP8(0, 8*8) },
	8*8*8
};


static GFXDECODE_START( gfx_extrema_m27sv )
	GFXDECODE_ENTRY( "gfx", 0, charlayout, 0, 8 )
	GFXDECODE_ENTRY( "gfx", 0, charlayout2, 0, 8 )
GFXDECODE_END


void extrema_m27sv_state::extrema_m27sv(machine_config &config)
{
	Z80(config, m_maincpu, 36_MHz_XTAL / 8 ); // divider not verified. Or maybe uses one of the 11.0592 XTALs?
	m_maincpu->set_addrmap(AS_PROGRAM, &extrema_m27sv_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &extrema_m27sv_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(extrema_m27sv_state::irq0_line_hold));

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0, 32*8-1, 0, 32*8-1);
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


GAME( 2006?, multgmxl, 0, extrema_m27sv, multgmxl, extrema_m27sv_state, empty_init, ROT0, "Extrema", "Multi GAME XL (V.11.35 A)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
