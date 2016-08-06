// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/*********************************************************************************

PINBALL
Playmatic MPU 1

Status:
- Main board is emulated and appears to be working (currently in attract mode)
- Displays to add
- Switches, lamps, solenoids to add
- Sound board to emulate
- Mechanical sounds to add

**********************************************************************************/


#include "machine/genpin.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"

class play_1_state : public driver_device
{
public:
	play_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_DRIVER_INIT(play_1);
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port06_r);
	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port02_w);
	DECLARE_WRITE8_MEMBER(port03_w);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port05_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_WRITE8_MEMBER(port07_w);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef2_r);
	DECLARE_READ_LINE_MEMBER(ef3_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	DECLARE_WRITE_LINE_MEMBER(clock_w);

private:
	UINT16 m_resetcnt;
	virtual void machine_reset() override;
	required_device<cosmac_device> m_maincpu;
};

static ADDRESS_MAP_START( play_1_map, AS_PROGRAM, 8, play_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x081f) AM_RAM
	AM_RANGE(0x0c00, 0x0c1f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chance_map, AS_PROGRAM, 8, play_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0c1f) AM_RAM
	AM_RANGE(0x0e00, 0x0e1f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_1_io, AS_IO, 8, play_1_state )
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x01, 0x01) AM_READWRITE(port01_r,port01_w) //segments
	AM_RANGE(0x02, 0x02) AM_WRITE(port02_w) // N1-8
	AM_RANGE(0x03, 0x03) AM_WRITE(port03_w) // D1-4
	AM_RANGE(0x04, 0x04) AM_WRITE(port04_w) // U1-8
	AM_RANGE(0x05, 0x05) AM_WRITE(port05_w) // V1-8
	AM_RANGE(0x06, 0x06) AM_READWRITE(port06_r,port06_w) // W1-8, input selector
	AM_RANGE(0x07, 0x07) AM_READ(port07_r) // another input selector
ADDRESS_MAP_END

static INPUT_PORTS_START( play_1 )
	PORT_START("DSW0")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Coinage ) ) // this is something else, don't know what yet
	PORT_DIPSETTING (  0x00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (  0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME(0x02, 0x00, "Balls")
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x02, "5" )
	PORT_DIPNAME(0x04, 0x00, "Special award")
	PORT_DIPSETTING (  0x00, "Free game" )
	PORT_DIPSETTING (  0x04, "Extra ball" )
	// rotary switches for credits per coin
INPUT_PORTS_END

void play_1_state::machine_reset()
{
	m_resetcnt = 0;
}

READ8_MEMBER( play_1_state::port00_r )
{
	return 0;
}

READ8_MEMBER( play_1_state::port01_r )
{
	return 0;
}

READ8_MEMBER( play_1_state::port06_r )
{
	return 0xff; // Big Town etc check this at boot
}

READ8_MEMBER( play_1_state::port07_r )
{
	return 0;
}

WRITE8_MEMBER( play_1_state::port01_w )
{
}

WRITE8_MEMBER( play_1_state::port02_w )
{
}

WRITE8_MEMBER( play_1_state::port03_w )
{
}

WRITE8_MEMBER( play_1_state::port04_w )
{
}

WRITE8_MEMBER( play_1_state::port05_w )
{
}

WRITE8_MEMBER( play_1_state::port06_w )
{
}

READ_LINE_MEMBER( play_1_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_1_state::ef2_r )
{
	return BIT(ioport("DSW0")->read(), 0); // 1 or 3 games dip (1=1 game)
}

READ_LINE_MEMBER( play_1_state::ef3_r )
{
	return BIT(ioport("DSW0")->read(), 1); // 3 or 5 balls dip (1=5 balls)
}

READ_LINE_MEMBER( play_1_state::ef4_r )
{
	return BIT(ioport("DSW0")->read(), 2); // extra ball or game dip (1=extra ball)
}

WRITE_LINE_MEMBER( play_1_state::clock_w )
{
	m_maincpu->int_w(1);
	m_maincpu->int_w(0); // INT is a pulse-line
	m_maincpu->ef1_w(state);
	// also, state and !state go to display panel
}

DRIVER_INIT_MEMBER(play_1_state,play_1)
{
}

static MACHINE_CONFIG_START( play_1, play_1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 400000)
	MCFG_CPU_PROGRAM_MAP(play_1_map)
	MCFG_CPU_IO_MAP(play_1_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(play_1_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(play_1_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(play_1_state, ef3_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(play_1_state, ef4_r))

	MCFG_DEVICE_ADD("xpoint", CLOCK, 60) // crossing-point detector
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_1_state, clock_w))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( chance, play_1 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(chance_map)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Space Gambler (03/78)
/-------------------------------------------------------------------*/
ROM_START(spcgambl)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spcgamba.bin", 0x0000, 0x0400, CRC(3b6e5287) SHA1(4d2fae779bb4117a99a9311b96ab79799f40067b))
	ROM_LOAD("spcgambb.bin", 0x0400, 0x0400, CRC(5c61f25c) SHA1(44b2d74926bf5678146b6d2347b4147e8a29a660))
ROM_END

/*-------------------------------------------------------------------
/ Big Town  (04/78)
/-------------------------------------------------------------------*/
ROM_START(bigtown)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bigtowna.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("bigtownb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Last Lap (09/78)
/-------------------------------------------------------------------*/
ROM_START(lastlap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("lastlapa.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("lastlapb.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END

/*-------------------------------------------------------------------
/ Chance (09/78)
/-------------------------------------------------------------------*/
ROM_START(chance)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("chance_a.bin", 0x0000, 0x0400, CRC(3cd9d5a6) SHA1(c1d9488495a67198f7f60f70a889a9a3062c71d7))
	ROM_LOAD("chance_b.bin", 0x0400, 0x0400, CRC(a281b0f1) SHA1(1d2d26ce5f50294d5a95f688c82c3bdcec75de95))
	ROM_LOAD("chance_c.bin", 0x0800, 0x0200, CRC(369afee3) SHA1(7fa46c7f255a5ef21b0d1cc018056bc4889d68b8))
ROM_END

/*-------------------------------------------------------------------
/ Party  (05/79)
/-------------------------------------------------------------------*/
ROM_START(party)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("party_a.bin", 0x0000, 0x0400, CRC(253f1b93) SHA1(7ff5267d0dfe6ae19ec6b0412902f4ce83f23ed1))
	ROM_LOAD("party_b.bin", 0x0400, 0x0400, CRC(5e2ba9c0) SHA1(abd285aa5702c7fb84257b4341f64ff83c1fc0ce))
ROM_END


/* Big Town, Last Lap and Party all reportedly share the same roms with different playfield/machine artworks */
GAME(1978,  bigtown,    0,      play_1, play_1, play_1_state,   play_1, ROT0,   "Playmatic",    "Big Town",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  chance,     0,      chance, play_1, play_1_state,   play_1, ROT0,   "Playmatic",    "Chance",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  lastlap,    0,      play_1, play_1, play_1_state,   play_1, ROT0,   "Playmatic",    "Last Lap",             MACHINE_IS_SKELETON_MECHANICAL)
GAME(1978,  spcgambl,   0,      play_1, play_1, play_1_state,   play_1, ROT0,   "Playmatic",    "Space Gambler",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1979,  party,      0,      play_1, play_1, play_1_state,   play_1, ROT0,   "Playmatic",    "Party",                MACHINE_IS_SKELETON_MECHANICAL)
