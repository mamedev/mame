// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

PINBALL
Playmatic MPU 5

Status:
- Main board is emulated and working (currently runs the initial test mode)
- Displays to add
- Switches, lamps, solenoids to add
- Sound board to emulate
- Mechanical sounds to add

(note to self: MPU3 and MPU5 appear at first glance to be identical apart from
cpu clock. MPU2 also appears to be identical to MPU3 apart from the RAM address.
If the sound cards are sufficiently similar, we should be able to merge all 3.)

***********************************************************************************/


#include "machine/genpin.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "machine/7474.h"

class play_5_state : public driver_device
{
public:
	play_5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
	{ }

	DECLARE_DRIVER_INIT(play_5);
	DECLARE_WRITE8_MEMBER(port01_w);
	DECLARE_WRITE8_MEMBER(port02_w);
	DECLARE_WRITE8_MEMBER(port03_w);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(port05_r);
	DECLARE_WRITE8_MEMBER(port06_w);
	DECLARE_WRITE8_MEMBER(port07_w);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef1_r);
	DECLARE_READ_LINE_MEMBER(ef4_r);
	DECLARE_WRITE_LINE_MEMBER(q4013a_w);
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	DECLARE_WRITE_LINE_MEMBER(clock2_w);

private:
	UINT16 m_clockcnt;
	UINT16 m_resetcnt;
	virtual void machine_reset() override;
	required_device<cosmac_device> m_maincpu;
	required_device<ttl7474_device> m_4013a;
	required_device<ttl7474_device> m_4013b;
};


static ADDRESS_MAP_START( play_5_map, AS_PROGRAM, 8, play_5_state )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
	AM_RANGE(0x8000, 0x80ff) AM_RAM AM_SHARE("nvram") // pair of 5101, battery-backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_5_io, AS_IO, 8, play_5_state )
	AM_RANGE(0x01, 0x01) AM_WRITE(port01_w) // digits
	AM_RANGE(0x02, 0x02) AM_WRITE(port02_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(port03_w)
	AM_RANGE(0x04, 0x04) AM_READ(port04_r)
	AM_RANGE(0x05, 0x05) AM_READ(port05_r)
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w) // segments
	AM_RANGE(0x07, 0x07) AM_WRITE(port07_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( play_5 )
INPUT_PORTS_END

void play_5_state::machine_reset()
{
	m_clockcnt = 0;
	m_resetcnt = 0;
	m_4013b->d_w(1);
}

WRITE8_MEMBER( play_5_state::port01_w )
{
}

WRITE8_MEMBER( play_5_state::port02_w )
{
}

WRITE8_MEMBER( play_5_state::port03_w )
{
}

READ8_MEMBER( play_5_state::port04_r )
{
	return 0xff;
}

READ8_MEMBER( play_5_state::port05_r )
{
	return 0xff;
}

WRITE8_MEMBER( play_5_state::port06_w )
{
}

WRITE8_MEMBER( play_5_state::port07_w )
{
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
}

READ_LINE_MEMBER( play_5_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_5_state::ef1_r )
{
	return BIT(m_clockcnt, 10);
}

READ_LINE_MEMBER( play_5_state::ef4_r )
{
	return 1; // reset button
}

DRIVER_INIT_MEMBER( play_5_state, play_5 )
{
}

WRITE_LINE_MEMBER( play_5_state::clock_w )
{
	m_4013a->clock_w(state);

	if (!state)
	{
		m_clockcnt++;
		// simulate 4020 chip
		if ((m_clockcnt & 0x3ff) == 0)
			m_4013b->preset_w(BIT(m_clockcnt, 10)); // Q10 output
	}
}

WRITE_LINE_MEMBER( play_5_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(!state);
}

WRITE_LINE_MEMBER( play_5_state::q4013a_w )
{
	m_clockcnt = 0;
}

static MACHINE_CONFIG_START( play_5, play_5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(play_5_map)
	MCFG_CPU_IO_MAP(play_5_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(play_5_state, clear_r))
	MCFG_COSMAC_EF1_CALLBACK(READLINE(play_5_state, ef1_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(play_5_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(DEVWRITELINE("4013a", ttl7474_device, clear_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("tpb_clock", CLOCK, XTAL_3_579545MHz / 8) // TPB line from CPU
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_5_state, clock_w))

	MCFG_DEVICE_ADD("xpoint", CLOCK, 60) // crossing-point detector
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_5_state, clock2_w))

	// This is actually a 4013 chip (has 2 RS flipflops)
	MCFG_DEVICE_ADD("4013a", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("4013a", ttl7474_device, d_w))
	MCFG_7474_OUTPUT_CB(WRITELINE(play_5_state, q4013a_w))

	MCFG_DEVICE_ADD("4013b", TTL7474, 0)
	MCFG_7474_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, ef2_w))
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, int_w)) MCFG_DEVCB_INVERT // int is reversed in mame

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ KZ-26 (1984)
/-------------------------------------------------------------------*/
ROM_START(kz26)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("kz26.cpu", 0x0000, 0x2000, CRC(8030a699) SHA1(4f86b325801d8ce16011f7b6ba2f3633e2f2af35))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sound1.su3", 0x0000, 0x2000, CRC(8ad1a804) SHA1(6177619f09af4302ffddd8c0c1b374dab7f47e91))
	ROM_LOAD("sound2.su4", 0x2000, 0x0800, CRC(355dc9ad) SHA1(eac8bc27157afd908f9bc5b5a7c40be5b9427269))
ROM_END

/*-------------------------------------------------------------------
/ Spain 82 (10/82)
/-------------------------------------------------------------------*/
ROM_START(spain82)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spaic12.bin", 0x0000, 0x1000, CRC(cd37ecdc) SHA1(ff2d406b6ac150daef868121e5857a956aabf005))
	ROM_LOAD("spaic11.bin", 0x1000, 0x0800, CRC(c86c0801) SHA1(1b52539538dae883f9c8fe5bc6454f9224780d11))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("spasnd.bin", 0x0000, 0x2000, CRC(62412e2e) SHA1(9e48dc3295e78e1024f726906be6e8c3fe3e61b1))
ROM_END

/*-------------------------------------------------------------------
/ ??/84 Nautilus
/-------------------------------------------------------------------*/
ROM_START(nautilus)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("nautilus.rom", 0x0000, 0x2000, CRC(197e5492) SHA1(0f83fc2e742fd0cca0bd162add4bef68c6620067))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("nautilus.snd", 0x0000, 0x2000, CRC(413d110f) SHA1(8360f652296c46339a70861efb34c41e92b25d0e))
ROM_END

/*-------------------------------------------------------------------
/ ??/84 The Raid
/-------------------------------------------------------------------*/
ROM_START(theraid)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("theraid.rom", 0x0000, 0x2000, CRC(97aa1489) SHA1(6b691b287138cc78cfc1010f380ff8c66342c39b))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("theraid.snd", 0x0000, 0x2000, CRC(e33f8363) SHA1(e7f251c334b15e12b1eb7e079c2e9a5f64338052))
ROM_END

/*-------------------------------------------------------------------
/ 11/84 UFO-X
/-------------------------------------------------------------------*/
ROM_START(ufo_x)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ufoxcpu.rom", 0x0000, 0x2000, CRC(cf0f7c52) SHA1(ce52da05b310ac84bdd57609e21b0401ee3a2564))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ufoxu3.rom", 0x0000, 0x2000, CRC(6ebd8ee1) SHA1(83522b76a755556fd38d7b292273b4c68bfc0ddf))
	ROM_LOAD("ufoxu4.rom", 0x2000, 0x0800, CRC(aa54ede6) SHA1(7dd7e2852d42aa0f971936dbb84c7708727ce0e7))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Rock 2500
/-------------------------------------------------------------------*/
ROM_START(rock2500)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("r2500cpu.rom", 0x0000, 0x2000, CRC(9c07e373) SHA1(5bd4e69d11e69fdb911a6e65b3d0a7192075abc8))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("r2500snd.rom", 0x0000, 0x2000, CRC(24fbaeae) SHA1(20ff35ed689291f321e483287a977c02e84d4524))
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Star Fire
/-------------------------------------------------------------------*/
ROM_START(starfirp)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starfcpu.rom", 0x0000, 0x2000, CRC(450ddf20) SHA1(c63c4e3833ffc1f69fcec39bafecae9c80befb2a))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

ROM_START(starfirpa)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("starcpua.rom", 0x0000, 0x2000, CRC(29bac350) SHA1(ab3e3ea4881be954f7fa7278800ffd791c4581da))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("starfu3.rom", 0x0000, 0x2000, CRC(5d602d80) SHA1(19d21adbcbd0067c051f3033468eda8c5af57be1))
	ROM_LOAD("starfu4.rom", 0x2000, 0x0800, CRC(9af8be9a) SHA1(da6db3716db73baf8e1493aba91d4d85c5d613b4))
ROM_END

/*-------------------------------------------------------------------
/ ??/86 Flash Dragon
/-------------------------------------------------------------------*/
ROM_START(fldragon)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("fldrcpu1.rom", 0x0000, 0x2000, CRC(e513ded0) SHA1(64ed3dcff53311fb93bd50d105a4c1186043fdd7))
	ROM_LOAD("fldraudiocpu.rom", 0x2000, 0x2000, CRC(6ff2b276) SHA1(040b614f0b0587521ef5550b5587b94a7f3f178b))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("fdsndu3.rom", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("fdsndu4.rom", 0x2000, 0x0800, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ ??/85 Stop Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ ??/87 Skill Flight
/-------------------------------------------------------------------*/
ROM_START(sklflite)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("skflcpu1.rom", 0x0000, 0x2000, CRC(8f833b55) SHA1(1729203582c22b51d1cc401aa8f270aa5cdadabe))
	ROM_LOAD("skflaudiocpu.rom", 0x2000, 0x2000, CRC(ffc497aa) SHA1(3e88539ae1688322b9268f502d8ca41cffb28df3))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("sfsndu3.rom", 0x0000, 0x2000, NO_DUMP)
	ROM_LOAD("sfsndu4.rom", 0x2000, 0x0800, NO_DUMP)
ROM_END

/*-------------------------------------------------------------------
/ ??/87 Phantom Ship
/-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
/ Trailer (1985)
/-------------------------------------------------------------------*/
ROM_START(trailer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("trcpu.rom", 0x0000, 0x2000, CRC(cc81f84d) SHA1(7a3282a47de271fde84cfddbaceb118add0df116))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("trsndu3.rom", 0x0000, 0x2000, CRC(05975c29) SHA1(e54d3a5613c3e39fc0338a53dbadc2e91c09ffe3))
	ROM_LOAD("trsndu4.rom", 0x2000, 0x0800, CRC(bda2a735) SHA1(134b5abb813ed8bf2eeac0861b4c88c7176582d8))
ROM_END

GAME(1982,  spain82,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Spain '82",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  nautilus,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Nautilus",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  theraid,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "The Raid",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  ufo_x,      0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "UFO-X",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1984,  kz26,       0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "KZ-26",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  rock2500,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Rock 2500",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  starfirp,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Star Fire",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  starfirpa,  starfirp,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Star Fire (alternate set)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1985,  trailer,    0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Trailer",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1986,  fldragon,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Flash Dragon",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1987,  sklflite,   0,  play_5, play_5, play_5_state,   play_5, ROT0,   "Playmatic",        "Skill Flight (Playmatic)",      MACHINE_IS_SKELETON_MECHANICAL)
