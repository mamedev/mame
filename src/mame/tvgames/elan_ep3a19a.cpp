// license:BSD-3-Clause
// copyright-holders:David Haywood

// The TV Board Game units have "Programmed by E.I. HK Development LTD." in the graphics

// To perform the hidden ROM check do Up + Button A while booting up, then on the black screen Down + Button B.
// This is probably impossible on the single button units using real hardware as the 'B' input isn't connected
// Currently these checksums fail in MAME due to the interrupt hack mapping over ROM, if you remove that hack they pass

#include "emu.h"

#include "screen.h"

#include "machine/elan_ep3a19a_soc.h"

namespace {

class elan_ep3a19a_state : public driver_device
{
public:
	elan_ep3a19a_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void elan_ep3a19a(machine_config &config);
	void elan_ep3a19a_1mb(machine_config &config);

	void init_tvbg();

	required_device<elan_ep3a19a_soc_device> m_maincpu;
	required_device<screen_device> m_screen;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);

	void elan_ep3a19a_extmap_2mb(address_map &map) ATTR_COLD;
	void elan_ep3a19a_extmap_1mb(address_map &map) ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
};

void elan_ep3a19a_state::video_start()
{
}

uint32_t elan_ep3a19a_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return m_maincpu->screen_update(screen, bitmap, cliprect);
}

void elan_ep3a19a_state::elan_ep3a19a_extmap_2mb(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0xe00000).rom().region("maincpu", 0);
}

void elan_ep3a19a_state::elan_ep3a19a_extmap_1mb(address_map &map)
{
	map(0x000000, 0x0fffff).mirror(0xf00000).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( tvbg_1button )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) // the 6-in-1 units have a single button marked with both A and B (unless it depends which side of the button you press?)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tvbg_2button )
	PORT_INCLUDE( tvbg_1button )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Boggle uses 2 buttons for gameplay, other units do read this to enter secret test mode, but none of the games need it?
INPUT_PORTS_END


void elan_ep3a19a_state::machine_start()
{
}

void elan_ep3a19a_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(elan_ep3a19a_state::interrupt)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void elan_ep3a19a_state::elan_ep3a19a(machine_config &config)
{
	ELAN_EP3A19A_SOC(config, m_maincpu, XTAL(21'477'272)/8);
	m_maincpu->set_addrmap(elan_ep3a19a_soc_device::AS_EXTERNAL, &elan_ep3a19a_state::elan_ep3a19a_extmap_2mb);
	m_maincpu->set_vblank_int("screen", FUNC(elan_ep3a19a_state::interrupt));
	m_maincpu->read_callback<0>().set_ioport("IN0");
	m_maincpu->read_callback<1>().set_ioport("IN1");
	m_maincpu->read_callback<2>().set_ioport("IN2");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(elan_ep3a19a_state::screen_update));
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
}

void elan_ep3a19a_state::elan_ep3a19a_1mb(machine_config &config)
{
	elan_ep3a19a(config);
	m_maincpu->set_addrmap(elan_ep3a19a_soc_device::AS_EXTERNAL, &elan_ep3a19a_state::elan_ep3a19a_extmap_1mb);
}

ROM_START( tvbg6a )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "candyland_hhh_silly6.bin", 0x00000, 0x200000, CRC(8b16d725) SHA1(06af509d03df0e5a2ca502743797af9f4a5dc6f1) )
ROM_END

ROM_START( tvbg6b )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "bship_simon_mousetrap.bin", 0x00000, 0x200000, CRC(b0627a98) SHA1(6157e26916bb415037a4d122d3075cbfb8e61dcf) )
ROM_END

ROM_START( tvbg3a )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hhhssp.bin", 0x00000, 0x100000, CRC(7e23a5a0) SHA1(2cd0f7572df30d2565b64fa0936715f71312ab1a) )
ROM_END

ROM_START( tvbg3b )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "simonbship.bin", 0x00000, 0x100000, CRC(9b10a87a) SHA1(f2022ac07468d911cfb3d32887d6e59e60d48d51) )
ROM_END

ROM_START( tvbg3c )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "boggle_connect4.bin", 0x00000, 0x100000, CRC(c2374eea) SHA1(c6971cb5108828bc72fd1cf7edeb53915d196db7) )
ROM_END

void elan_ep3a19a_state::init_tvbg()
{
	// is this swapping internal to the ep3a19a type ELAN, or external; ROM glob had standard TSOP pinout pads that were used for dumping.
	uint8_t *ROM = memregion("maincpu")->base();
	size_t len = memregion("maincpu")->bytes();

	for (int i = 0; i < len; i++)
	{
		ROM[i] = bitswap<8>(ROM[i], 6, 5, 7, 0, 2, 3, 1, 4);
	}
}

} // anonymous namespace


CONS( 2007, tvbg6a, 0, 0, elan_ep3a19a, tvbg_1button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 6-in-1: Silly 6 Pins, Candy Land, Hungry Hungry Hippos, Match 'em, Mixin' Pics, Checkers", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=zajzQo47YYA
CONS( 2007, tvbg6b, 0, 0, elan_ep3a19a, tvbg_1button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 6-in-1: Simon, Battleship, Mouse Trap, Checkers, Link-a-Line, Roll Over", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=JbrR67kY8MI

CONS( 2007, tvbg3a, 0, 0, elan_ep3a19a_1mb, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Silly 6 Pins, Hungry Hungry Hippos, Match 'em", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2007, tvbg3b, 0, 0, elan_ep3a19a_1mb, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Simon, Battleship, Checkers", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=Q7nwKJfVavU
CONS( 2007, tvbg3c, 0, 0, elan_ep3a19a_1mb, tvbg_2button, elan_ep3a19a_state, init_tvbg, "NSI International / Mammoth Toys (Licensed by Hasbro)", "TV Board Games 3-in-1: Boggle, Connect 4, Roll Over", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // https://www.youtube.com/watch?v=SoKKIKSDGhY

// The back of the Silly 6 Pins 3-in-1 packaging suggests a Monopoly TV Board Game device was planned, but this does not appear to have been released.
