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
#include "sound/speaker.h"
#include "play_1.lh"

class play_1_state : public driver_device
{
public:
	play_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dips(*this, "X")
		, m_monotone(*this, "monotone")
	{ }

	DECLARE_READ8_MEMBER(port07_r);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port02_w);
	DECLARE_WRITE8_MEMBER(port03_w);
	DECLARE_WRITE8_MEMBER(port04_w);
	DECLARE_WRITE8_MEMBER(port05_w);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef2_r);
	DECLARE_READ_LINE_MEMBER(ef3_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	DECLARE_WRITE_LINE_MEMBER(clock_w);

private:
	UINT16 m_resetcnt;
	UINT8 m_segment;
	UINT8 m_digit;
	UINT8 m_match;
	virtual void machine_reset() override;
	required_device<cosmac_device> m_maincpu;
	required_ioport_array<4> m_dips;
	required_device<clock_device> m_monotone;
};

static ADDRESS_MAP_START( play_1_map, AS_PROGRAM, 8, play_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x081f) AM_RAM AM_SHARE("nvram") // capacitor acting as a 3-month "battery"
	AM_RANGE(0x0c00, 0x0c1f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chance_map, AS_PROGRAM, 8, play_1_state )
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x0bff) AM_ROM
	AM_RANGE(0x0c00, 0x0c1f) AM_RAM AM_SHARE("nvram") // capacitor acting as a 3-month "battery"
	AM_RANGE(0x0e00, 0x0e1f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_1_io, AS_IO, 8, play_1_state )
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(port01_w) //segments
	AM_RANGE(0x02, 0x02) AM_READ_PORT("IN2") AM_WRITE(port02_w) // N1-8
	AM_RANGE(0x03, 0x03) AM_READ_PORT("IN3") AM_WRITE(port03_w) // D1-4
	AM_RANGE(0x04, 0x04) AM_READ_PORT("IN4") AM_WRITE(port04_w) // U1-8
	AM_RANGE(0x05, 0x05) AM_READ_PORT("IN5") AM_WRITE(port05_w) // V1-8
	AM_RANGE(0x06, 0x06) AM_READ_PORT("IN6") AM_WRITE(port06_w) // W1-8
	AM_RANGE(0x07, 0x07) AM_READ(port07_r)
ADDRESS_MAP_END

static INPUT_PORTS_START( play_1 )
	PORT_START("X.0")
//	PORT_DIPNAME(0x01, 0x01, DEF_STR( Coinage ) ) // this is something else, don't know what yet
//	PORT_DIPSETTING (  0x00, DEF_STR( 1C_3C ) )
//	PORT_DIPSETTING (  0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME(0x02, 0x00, "Balls")
	PORT_DIPSETTING (  0x00, "3" )
	PORT_DIPSETTING (  0x02, "5" )
	PORT_DIPNAME(0x04, 0x00, "Special award")
	PORT_DIPSETTING (  0x00, "Free game" )
	PORT_DIPSETTING (  0x04, "Extra ball" )

	PORT_START("X.1")
	PORT_DIPNAME(0xff, 0x20, "Coinage for slot 2" )
	PORT_DIPSETTING (  0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (  0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING (  0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (  0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (  0x80, DEF_STR( 1C_6C ) )

	PORT_START("X.2")
	PORT_DIPNAME(0xff, 0x20, "Coinage for slot 3" )
	PORT_DIPSETTING (  0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING (  0x02, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING (  0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING (  0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING (  0x10, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING (  0x20, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING (  0x40, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING (  0x80, "1 coin 10 credits" )

	PORT_START("IN1") // 11-18
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("IN2") // 21-28
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("IN3") // 31-38
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("IN4") // 41-48
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("IN5") // 51-58
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("IN6") // 61-68
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X.3") // 71-78
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6)
INPUT_PORTS_END

void play_1_state::machine_reset()
{
	m_resetcnt = 0;
	m_segment = 0;
	m_digit = 0;
	m_match = 0;
}

READ8_MEMBER( play_1_state::port07_r )
{
	UINT8 data = m_dips[3]->read();
	data |= (m_segment & m_dips[1]->read()) ? 0x40 : 0;
	data |= (m_segment & m_dips[2]->read()) ? 0x80 : 0;
	return data;
}

WRITE8_MEMBER( play_1_state::port01_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	// d0-1 via 4013 to match-game board
	// d4-7 via 4511 to match-game board
	if BIT(data, 0)
		output().set_digit_value(40, patterns[1]);
	else
		output().set_digit_value(40, 0);

	if BIT(data, 1)
		output().set_digit_value(45, patterns[0]);
	else
		output().set_digit_value(45, 0);

	m_match = patterns[data>>4] & 0x7f;
}

WRITE8_MEMBER( play_1_state::port02_w )
{
	// N1-8, segments and other
	m_segment = data;
}

WRITE8_MEMBER( play_1_state::port03_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	// D1-4, digit select
	// according to the schematic, there's nothing to light up the tens and units for players 2,3,4
	m_digit = data & 15;
	switch (m_digit)
	{
		case 1:
			// a combination of bits could set higher frequencies, but that isn't documented
			if BIT(m_segment, 0)
				m_monotone->set_unscaled_clock(523);
			else
			if BIT(m_segment, 1)
				m_monotone->set_unscaled_clock(659);
			else
			if BIT(m_segment, 2)
				m_monotone->set_unscaled_clock(784);
			else
			if BIT(m_segment, 3)
				m_monotone->set_unscaled_clock(988);
			else
				m_monotone->set_unscaled_clock(0);
			break;
		case 2:
			output().set_digit_value(0, patterns[m_segment>>4]);
			output().set_digit_value(1, patterns[m_segment&15]);
			break;
		case 3:
			output().set_digit_value(2, patterns[m_segment>>4]);
			output().set_digit_value(3, patterns[m_segment&15]);
			break;
		case 4:
			output().set_digit_value(4, patterns[m_segment>>4]);
			output().set_digit_value(5, patterns[m_segment&15]);
			break;
		case 5:
			output().set_digit_value(10, patterns[m_segment>>4]);
			output().set_digit_value(11, patterns[m_segment&15]);
			break;
		case 6:
			output().set_digit_value(12, patterns[m_segment>>4]);
			output().set_digit_value(13, patterns[m_segment&15]);
			break;
		case 7:
			output().set_digit_value(20, patterns[m_segment>>4]);
			output().set_digit_value(21, patterns[m_segment&15]);
			break;
		case 8:
			output().set_digit_value(22, patterns[m_segment>>4]);
			output().set_digit_value(23, patterns[m_segment&15]);
			break;
		case 9:
			output().set_digit_value(30, patterns[m_segment>>4]);
			output().set_digit_value(31, patterns[m_segment&15]);
			break;
		case 10:
		case 11:
			output().set_digit_value(32, patterns[m_segment>>4]);
			output().set_digit_value(33, patterns[m_segment&15]);
			break;
		default:
			break;
	}
}

WRITE8_MEMBER( play_1_state::port04_w )
{
	// U1-8
}

WRITE8_MEMBER( play_1_state::port05_w )
{
	// V1-8
}

WRITE8_MEMBER( play_1_state::port06_w )
{
	// W1-8
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
	return !BIT(m_dips[0]->read(), 0); // 1 or 3 games dip (1=1 game) inverted
}

READ_LINE_MEMBER( play_1_state::ef3_r )
{
	return !BIT(m_dips[0]->read(), 1); // 3 or 5 balls dip (1=5 balls) inverted
}

READ_LINE_MEMBER( play_1_state::ef4_r )
{
	return !BIT(m_dips[0]->read(), 2); // extra ball or game dip (1=extra ball) inverted
}

WRITE_LINE_MEMBER( play_1_state::clock_w )
{
	m_maincpu->int_w(0);
	m_maincpu->ef1_w(!state); // inverted
	if (state)
		output().set_digit_value(41, m_match);
	else
		output().set_digit_value(44, m_match);
	m_maincpu->int_w(1); // INT is a pulse-line - inverted

	// also, state and !state go to display panel
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

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_play_1)

	MCFG_DEVICE_ADD("xpoint", CLOCK, 60) // crossing-point detector
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_1_state, clock_w))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEVICE_ADD("monotone", CLOCK, 0) // sound device
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("speaker", speaker_sound_device, level_w))
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
GAME(1978,  bigtown,  0,       play_1, play_1, driver_device, 0, ROT0, "Playmatic", "Big Town",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1978,  chance,   0,       chance, play_1, driver_device, 0, ROT0, "Playmatic", "Chance",        MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1978,  lastlap,  bigtown, play_1, play_1, driver_device, 0, ROT0, "Playmatic", "Last Lap",      MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1978,  spcgambl, 0,       play_1, play_1, driver_device, 0, ROT0, "Playmatic", "Space Gambler", MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME(1979,  party,    bigtown, play_1, play_1, driver_device, 0, ROT0, "Playmatic", "Party",         MACHINE_MECHANICAL | MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
