// license:BSD-3-Clause
// copyright-holders:David Haywood

/* the games in here run on newer VT platforms that seem to include more capable sprite systems, palette etc. as well as
   backwards compatibility with older VT03/09.  They've been split out of nes_vt.cpp emulation to make them easier to
   study, they might be moved back later. */

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"

class nes_vt_newer_state : public driver_device
{
public:
	nes_vt_newer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void nes_vt_newer(machine_config &config);

	void init_lxcmcypp();

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void nes_vt_newer_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
};

uint32_t nes_vt_newer_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void nes_vt_newer_state::machine_start()
{
}

void nes_vt_newer_state::machine_reset()
{
}

void nes_vt_newer_state::nes_vt_newer_map(address_map &map)
{
	map(0x000000, 0xffff).rom().region("maincpu", 0);
}

static const gfx_layout test_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*1) },
	8 * 1 * 8
};

static GFXDECODE_START( gfx_test )
	GFXDECODE_ENTRY( "maincpu", 0, test_layout,  0x0, 1  )
GFXDECODE_END


void nes_vt_newer_state::nes_vt_newer(machine_config &config)
{
	M6502(config, m_maincpu, 8000000); // unknown, assumed to be a 6502 based CPU as it has NES games, but could be emulating them (like the S+Core units, assuming this isn't one)
	m_maincpu->set_addrmap(AS_PROGRAM, &nes_vt_newer_state::nes_vt_newer_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_refresh_hz(60);
	m_screen->set_size(300, 262);
	m_screen->set_visarea(0, 256-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(nes_vt_newer_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_test);
}

static INPUT_PORTS_START( nes_vt )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
INPUT_PORTS_END

ROM_START( dgun2561 )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	ROM_LOAD( "dgun2561.bin", 0x00000, 0x4000000, CRC(a6e627b4) SHA1(2667d2feb02de349387f9dcfa5418e7ed3afeef6) )
ROM_END

ROM_START( dgun2593 )
	ROM_REGION( 0x8000000, "maincpu", 0 )
	ROM_LOAD( "dreamgear300.bin", 0x00000, 0x8000000, CRC(4fe0ed02) SHA1(a55590557bacca65ed9a17c5bcf0a4e5cb223126) )
ROM_END



ROM_START( rtvgc300 )
	ROM_REGION( 0x8000000, "maincpu", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "lexibook300.bin", 0x00000, 0x4000000, CRC(015c4067) SHA1(a12986c4a366a23c4c7ca7b3d33e421a8dfdffc0) )
ROM_END

ROM_START( rtvgc300fz )
	ROM_REGION( 0x8000000, "maincpu", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "jg7800fz.bin", 0x00000, 0x4000000, CRC(c9d319d2) SHA1(9d0d1435b802f63ce11b94ce54d11f4065b324cc) )
ROM_END

// The maximum address space a VT chip can see is 32MB, so these 64MB roms are actually 2 programs (there are vectors in the first half and the 2nd half)
// there must be a bankswitch bit that switches the whole 32MB space.  Loading the 2nd half in Star Wars does actually boot straight to a game.
ROM_START( lxcmcy )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	ROM_LOAD( "lxcmcy.bin", 0x00000, 0x4000000, CRC(3f3af72c) SHA1(76127054291568fcce1431d21af71f775cfb05a6) )
ROM_END

ROM_START( lxcmcysw )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	ROM_LOAD( "jl2365swr-1.u2", 0x2000000, 0x2000000, CRC(60ece391) SHA1(655de6b36ba596d873de2839522b948ccf45e006) )
	ROM_CONTINUE(0x0000000, 0x2000000)
ROM_END

ROM_START( lxcmcyfz )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "jl2365_frozen.u1", 0x00000, 0x4000000, CRC(64d4c708) SHA1(1bc2d161326ce3039ab9ba46ad62695060cfb2e1) )
ROM_END

ROM_START( lxcmcydp )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cyberarcade-disneyprincess.bin", 0x00000, 0x4000000, CRC(05946f81) SHA1(33eea2b70f5427e7613c836b8a08148731fac231) )
ROM_END

ROM_START( lxcmcysp )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "lexibookspiderman.bin", 0x00000, 0x4000000, CRC(ef6e8847) SHA1(0012df193c52fd48595d85886fd431619c5d5e3e) )
ROM_END

ROM_START( lxcmcycr )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	ROM_LOAD( "lexibook cars.bin", 0x00000, 0x4000000, CRC(198fe11b) SHA1(5e35caa3fc319ec69812c187a3ec89f01749f749) )
ROM_END

ROM_START( lxcmcypp )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	// marked 512mbit, possible A22 / A23 are swapped as they were marked on the board in a different way.
	ROM_LOAD( "pawpatrol_compact.bin", 0x00000, 0x4000000, CRC(bf536762) SHA1(80dde8426a636bae33a82d779e564fa743eb3776) )
ROM_END

ROM_START( lxcmc250 )
	ROM_REGION( 0x4000000, "maincpu", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cca250in1.u1", 0x00000, 0x4000000, CRC(6ccd6ad6) SHA1(fafed339097c3d1538faa306021a8373c1b799b3) )
ROM_END

ROM_START( lxccminn )
	ROM_REGION( 0x4000000, "maincpu", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "minnie_lexibook.bin", 0x00000, 0x4000000, CRC(3f8e5a69) SHA1(c9f11f3e5f9b73832a191f4d1620a85c1b70f79e) )
	ROM_IGNORE(0x4000000)
ROM_END

ROM_START( lxccplan )
	ROM_REGION( 0x4000000, "maincpu", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "planes_lexibook.bin", 0x00000, 0x4000000, CRC(76e1a962) SHA1(83b801c0e0e941ceb1c93e565e833b07c09412c3))
	ROM_IGNORE(0x4000000)
ROM_END

ROM_START( red5mam )
	ROM_REGION( 0x8000000, "maincpu", 0 )
	ROM_LOAD( "mam.u3", 0x00000, 0x8000000, CRC(0c0a0ecd) SHA1(2dfd8437de17fc9975698f1933dd81fbac78466d) )
ROM_END


ROM_START( denv150 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "denver150in1.bin", 0x00000, 0x1000000, CRC(6b3819d7) SHA1(b0039945ce44a52ea224ab736d5f3c6980409b5d) ) // 2nd half is blank
ROM_END


ROM_START( zonefusn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "fusion.bin", 0x00000, 0x1000000, CRC(240bf970) SHA1(1b82d95a252c08e52fb8da6320276574a30b60db) )
ROM_END

void nes_vt_newer_state::init_lxcmcypp()
{
	int size = memregion("maincpu")->bytes()/2;
	uint16_t* ROM = (uint16_t*)memregion("maincpu")->base();

	for (int i = 0; i < size; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11);
	}
}

#if 0
/* Lexibook I/O handlers */

uint8_t nes_vt_cy_lexibook_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = m_latch0_bit;
	return ret;
}

uint8_t nes_vt_cy_lexibook_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = m_latch1_bit;
	return ret;
}

void nes_vt_cy_lexibook_state::in0_w(uint8_t data)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if ((!(data & 0x01)) && (m_previous_port0 & 0x01)) // 0x03 -> 0x02 transition
	{
		m_latch0 = m_io0->read();
		m_latch1 = m_io1->read();
	}

	if ((!(data & 0x02)) && (m_previous_port0 & 0x02)) // 0x02 -> 0x00 transition
	{
		m_latch0_bit = m_latch0 & 0x01;
		m_latch0 >>= 1;
		m_latch1_bit = m_latch1 & 0x01;
		m_latch1 >>= 1;
	}

	m_previous_port0 = data;
}
#endif


// don't even get to menu. very enhanced chipset, VT368/9?
CONS( 2012, dgun2561,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init, "dreamGEAR", "My Arcade Portable Gaming System with 140 Games (DGUN-2561)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 2016, dgun2593,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init, "dreamGEAR", "My Arcade Retro Arcade Machine - 300 Handheld Video Games (DGUN-2593)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

CONS( 200?, lxcmcy,    0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmc250,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - 250-in-1 (JL2375)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcysw,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Star Wars Rebels", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcyfz,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Frozen", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcydp,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Disney Princess", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcysp,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Marvel Ultimate Spider-Man", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcycr,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Cars", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
// the data order is swapped for this one, maybe other internal differences?
CONS( 200?, lxcmcypp,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, init_lxcmcypp, "Lexibook", "Lexibook Compact Cyber Arcade - Paw Patrol", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme


CONS( 200?, lxccminn,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Console Colour - Minnie Mouse", MACHINE_NOT_WORKING ) // 64Mbyte (used) ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxccplan,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Console Colour - Disney's Planes", MACHINE_NOT_WORKING ) // 64Mbyte (used) ROM, must be externally banked, or different addressing scheme


// GB-NO13-Main-VT389-2 on PCBs
CONS( 2016, rtvgc300,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Retro TV Game Console - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 2017, rtvgc300fz,0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init,    "Lexibook", "Lexibook Retro TV Game Console - Frozen - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme


/* The following are also confirmed to be NES/VT derived units, most having a standard set of games with a handful of lazy graphic mods thrown in to fit the unit theme

    (handhekd units, use standard AAA batteries)
    Lexibook Compact Cyber Arcade - Cars
    Lexibook Compact Cyber Arcade - Barbie
    Lexibook Compact Cyber Arcade - Finding Dory
    Lexibook Compact Cyber Arcade - PJ Masks

    (Handheld units, but different form factor to Compact Cyber Arcade, charged via USB)
    Lexibook Console Colour - Barbie

    (units for use with TV)
    Lexibook Retro TV Game Console (300 Games) - Cars
    Lexibook Retro TV Game Console (300 Games) - PJ Masks

    (more?)

    There are also updated 'Compact Cyber Arcade' branded units with a large + D-pad and internal battery / USB charger for
    Spiderman
    Frozen
    (generic)
    it isn't verified if these use the same ROMs as the original Compact Cyber Arcade releases, or if the software has been updated

*/

// intial code isn't valid? scrambled?
CONS( 201?, red5mam,  0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init, "Red5", "Mini Arcade Machine (Red5)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

CONS( 201?, denv150,   0,  0,  nes_vt_newer, nes_vt, nes_vt_newer_state, empty_init, "Denver", "Denver Game Console GMP-240C 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, zonefusn,  0,  0,  nes_vt_newer,     nes_vt, nes_vt_newer_state, empty_init, "Ultimate Products / Jungle's Soft", "Zone Fusion",  MACHINE_NOT_WORKING )

