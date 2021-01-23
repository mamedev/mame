// license:BSD-3-Clause
// copyright-holders:

/*
Unknown video slot (?)

PCB marked 'ROLLA REV. 1' and 'CTE 001 94V-0 0205'

1 Winbond W77E58P on a small sub board (beefed up 8052 with undumped internal EPROM)
1 ispLSI 1016E
1 JFC 95101 (AY8910 compatible)
4 8-dip banks
1 24.00 MHz XTAL (on the daughterboard)

TODO:
- decap and / or dump the main CPU.
*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class rolla_state : public driver_device
{
public:
	rolla_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void rolla(machine_config &config);

private:
	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void rolla_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( rolla )
	PORT_START("IN0")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW4:8")
INPUT_PORTS_END


uint32_t rolla_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static GFXDECODE_START( gfx_rolla ) // TODO: fix decoding
	GFXDECODE_ENTRY( "gfx1", 0, gfx_16x16x4_packed_msb, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 0, 1 )
GFXDECODE_END


void rolla_state::rolla(machine_config &config)
{
	// basic machine hardware
	I8052(config, m_maincpu, 24_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rolla_state::main_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: all wrong
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(13*8, (64-13)*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(rolla_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_555, 512); // TODO: all wrong
	GFXDECODE(config, "gfxdecode", "palette", gfx_rolla);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	AY8910(config, "ay8910", 24_MHz_XTAL / 16).add_route(ALL_OUTPUTS, "speaker", 0.5); // divider guessed, actually JFC 95101 (compatible)
}


ROM_START( rolla )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "545a", 0x0000, 0x8000, NO_DUMP ) // internal flash EPROM, security bit set

	ROM_REGION( 0x80000, "gfx1", 0 ) // TODO: fix ROM loading
	ROM_LOAD32_BYTE( "4w s1.u29", 0x00000, 0x20000, CRC(8b02800e) SHA1(22ae265e8d0bca403be4b4a82b43c526d9407dcf) )
	ROM_LOAD32_BYTE( "4w s2.u31", 0x00001, 0x20000, CRC(efa631ca) SHA1(c924277c64fc20e0d841be8fd1e2a4718dd229f1) )
	ROM_LOAD32_BYTE( "4w s3.u33", 0x00002, 0x20000, CRC(8fdc2d33) SHA1(61cc36caf52487ac863dbec1c40f3ef93e57a505) )
	ROM_LOAD32_BYTE( "4w s4.u35", 0x00003, 0x20000, CRC(9e86abfa) SHA1(fe7e8222051810fd5e0a584967764d71d99bf8fe) )

	ROM_REGION( 0x80000, "gfx2", 0 ) // TODO: fix ROM loading
	ROM_LOAD32_BYTE( "4w g1.u52", 0x00000, 0x20000, CRC(88036f9e) SHA1(fa93bce2c6eccf70ee23fdd320ee56dcdb6d99e3) )
	ROM_LOAD32_BYTE( "4w g2.u54", 0x00001, 0x20000, CRC(0142ba31) SHA1(e9aed07f112f68fb598bbc841666d5516479b06c) )
	ROM_LOAD32_BYTE( "4w g3.u56", 0x00002, 0x20000, CRC(4324d5c0) SHA1(9c57385eda6a069e455b8cd9fb45db90f1966ecf) )
	ROM_LOAD32_BYTE( "4w g4.u58", 0x00003, 0x20000, CRC(1cce85f1) SHA1(f35709b29a13917fe3c69240b4ab03aa6bdc3a3b) )
ROM_END

} // Anonymous namespace


GAME( 199?, rolla,  0, rolla, rolla, rolla_state, empty_init, ROT0, "<unknown>", "unknown 'Rolla' slot machine", MACHINE_IS_SKELETON )
