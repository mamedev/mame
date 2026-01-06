// license:BSD-3-Clause
// copyright-holders:

/*
Hom Inn GMC90C52 based hardware
PCB was heavily corroded and 2 of the ROMs appear bad

- GMS90C52-GB08 HOM INN 99060 MCU with 0x2000 internal ROM and 0x100 internal RAM
- XTAL with unreadable value
- 44-pin square chip with scratched off markings (possibly an Altera EPM70**)
- HM6264ALP-15
- U6295 (Oki M6295 clone)
- bank 8 of switches
- 4x push-button
*/


#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class hominn_mcs_state : public driver_device
{
public:
	hominn_mcs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void unkhomin(machine_config &config) ATTR_COLD;


private:
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


uint32_t hominn_mcs_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t::black(), cliprect);

	return 0;
}


void hominn_mcs_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}


static INPUT_PORTS_START( unkhomin )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


// TODO
static GFXDECODE_START( gfx )
GFXDECODE_END


void hominn_mcs_state::unkhomin(machine_config &config)
{
	// basic machine hardware
	I80C52(config, m_maincpu, 12'000'000); // TODO: actually GMC90C52. XTAL unreadable. Chip is rated for 12/24/40 MHz operation.
	m_maincpu->set_addrmap(AS_PROGRAM, &hominn_mcs_state::program_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: verify everything once emulation works
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(hominn_mcs_state::screen_update));

	GFXDECODE(config, "gfxdecode", "palette", gfx);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, "oki", 12'000'000 / 12, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // XTAL, divider and pin 7 not verified
}


ROM_START( unkhomin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am27c512.u7",     0x00000, 0x10000, CRC(81d7762b) SHA1(da7207a3d53f6af2b0676672d922cb3e1aa13a8a) ) // first 0x2000 are 0xff filled
	ROM_LOAD( "internal_rom.u4", 0x00000, 0x02000, NO_DUMP )

	ROM_REGION( 0x80000, "gfx", 0 )
	ROM_LOAD( "am27c040.u12", 0x00000, 0x80000, BAD_DUMP CRC(7efad171) SHA1(f5fdbeed2e261f26050417b44c3136b55a0ecce2) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "am27c040.u11", 0x00000, 0x80000, BAD_DUMP CRC(0f011c09) SHA1(ca6d5f2ccc591b17b9391c05d5a12ef109855d99) ) // FIXED BITS (xxxxxxxx00000000)
ROM_END

} // anonymous namespace


GAME( 199?, unkhomin, 0, unkhomin, unkhomin, hominn_mcs_state, empty_init, ROT0, "Hom Inn", "unknown Hom Inn cards game", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
