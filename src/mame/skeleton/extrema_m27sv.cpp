// license:BSD-3-Clause
// copyright-holders:

/*

Video slots by Extrema running on the Extrema M27-SV PCB

TODO:
- hangs at startup;
\- $9002 becomes 0x11, loops until that becomes 0 then it throws an halt with irqs disabled.
- video (if above bypassed puts some GFX at $e000);
- palette banks;
- I/O;

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
		, m_palette(*this, "palette")
		, m_palette_ram(*this, "paletteram")
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
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_palette_ram;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void mmio_map(address_map &map) ATTR_COLD;

	u8 m_unk_fe20 = 0;
	void palette_w(offs_t offset, u8 data);
};

void extrema_m27sv_state::machine_start()
{
	m_bank->configure_entries(0, 8, m_program_rom->base(), 0x8000);

	save_item(NAME(m_unk_fe20));
}

void extrema_m27sv_state::machine_reset()
{
	m_unk_fe20 = 0;
	m_bank->set_entry(0);
}

void extrema_m27sv_state::video_start()
{
}

uint32_t extrema_m27sv_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void extrema_m27sv_state::palette_w(offs_t offset, u8 data)
{
	m_palette_ram[offset] = data;

	u16 pal_offset = offset & 0x3ff;
	// TODO: determine bank order
	u8 r = m_palette_ram[pal_offset | 0x000];
	u8 g = m_palette_ram[pal_offset | 0x400];
	u8 b = m_palette_ram[pal_offset | 0x800];
	m_palette->set_pen_color(pal_offset, pal6bit(r), pal6bit(g), pal6bit(b));
}

void extrema_m27sv_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank");
	map(0x8000, 0xdfff).ram();
	map(0xe000, 0xefff).ram(); // video
	map(0xf000, 0xfbff).ram().w(FUNC(extrema_m27sv_state::palette_w)).share("paletteram");
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
	map(0x2a, 0x2a).nopw(); // flips bit 2 a lot, watchdog?

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
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

// no DSW on PCB
INPUT_PORTS_END

// TODO: incomplete
static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1, 2),
	8,
	{ STEP4(0, 1), STEP4(RGN_FRAC(1, 2), 1) },
	{ STEP8(0, 4) },
	{ STEP8(0, 4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_extrema_m27sv )
	GFXDECODE_ENTRY( "gfx", 0, charlayout, 0, 4 )
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

	GFXDECODE(config, "gfxdecode", "palette", gfx_extrema_m27sv);

	PALETTE(config, "palette").set_entries(0x400);

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
	ROM_LOAD( "u10_multigamexl.u10", 0x000000, 0x080000, CRC(d745758e) SHA1(9aeb8fd8e46b2ee0663f436628c95e73057dfc51) )
	ROM_CONTINUE( 0x100000, 0x080000 )
	ROM_LOAD( "u11_multigamexl.u11", 0x080000, 0x080000, CRC(ca59894e) SHA1(a97afe937d29e4261cc46e41283e6977b20eafc5) )
	ROM_CONTINUE( 0x180000, 0x080000 )
ROM_END

} // anonymous namespace


GAME( 2006?, multgmxl, 0, extrema_m27sv, multgmxl, extrema_m27sv_state, empty_init, ROT0, "Extrema", "Multi GAME XL", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
