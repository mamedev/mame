// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/**********************************************************************************

PINBALL
Playmatic MPU 3

Status:
- Main board is emulated and working (currently runs the initial test mode)
- Displays to add
- Switches, lamps, solenoids to add
- Sound board to emulate
- Mechanical sounds to add

***********************************************************************************/


#include "machine/genpin.h"
#include "cpu/cosmac/cosmac.h"
#include "machine/clock.h"
#include "machine/7474.h"

class play_3_state : public driver_device
{
public:
	play_3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_4013a(*this, "4013a")
		, m_4013b(*this, "4013b")
	{ }

	DECLARE_DRIVER_INIT(play_3);
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


static ADDRESS_MAP_START( play_3_map, AS_PROGRAM, 8, play_3_state )
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x4000) AM_ROM // 2x 2732
	//AM_RANGE(0x3000, 0x30ff) AM_MIRROR(0x4f00) AM_ROM // undumped PAL
	AM_RANGE(0x8000, 0x80ff) AM_MIRROR(0x7f00) AM_RAM AM_SHARE("nvram") // pair of 5101, battery-backed
ADDRESS_MAP_END

static ADDRESS_MAP_START( play_3_io, AS_IO, 8, play_3_state )
	AM_RANGE(0x01, 0x01) AM_WRITE(port01_w) // digits
	AM_RANGE(0x02, 0x02) AM_WRITE(port02_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(port03_w)
	AM_RANGE(0x04, 0x04) AM_READ(port04_r)
	AM_RANGE(0x05, 0x05) AM_READ(port05_r)
	AM_RANGE(0x06, 0x06) AM_WRITE(port06_w) // segments
	AM_RANGE(0x07, 0x07) AM_WRITE(port07_w)
ADDRESS_MAP_END


static INPUT_PORTS_START( play_3 )
INPUT_PORTS_END

void play_3_state::machine_reset()
{
	m_clockcnt = 0;
	m_resetcnt = 0;
	m_4013b->d_w(1);
}

WRITE8_MEMBER( play_3_state::port01_w )
{
}

WRITE8_MEMBER( play_3_state::port02_w )
{
}

WRITE8_MEMBER( play_3_state::port03_w )
{
}

READ8_MEMBER( play_3_state::port04_r )
{
	return 0xff;
}

READ8_MEMBER( play_3_state::port05_r )
{
	return 0xff;
}

WRITE8_MEMBER( play_3_state::port06_w )
{
}

WRITE8_MEMBER( play_3_state::port07_w )
{
	m_4013b->clear_w(0);
	m_4013b->clear_w(1);
}

READ_LINE_MEMBER( play_3_state::clear_r )
{
	// A hack to make the machine reset itself on boot
	if (m_resetcnt < 0xffff)
		m_resetcnt++;
	return (m_resetcnt == 0xff00) ? 0 : 1;
}

READ_LINE_MEMBER( play_3_state::ef1_r )
{
	return BIT(m_clockcnt, 10);
}

READ_LINE_MEMBER( play_3_state::ef4_r )
{
	return 1; // reset button
}

DRIVER_INIT_MEMBER( play_3_state, play_3 )
{
}

WRITE_LINE_MEMBER( play_3_state::clock_w )
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

WRITE_LINE_MEMBER( play_3_state::clock2_w )
{
	m_4013b->clock_w(state);
	m_maincpu->ef3_w(!state);
}

WRITE_LINE_MEMBER( play_3_state::q4013a_w )
{
	m_clockcnt = 0;
}

static MACHINE_CONFIG_START( play_3, play_3_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", CDP1802, 2950000)
	MCFG_CPU_PROGRAM_MAP(play_3_map)
	MCFG_CPU_IO_MAP(play_3_io)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(play_3_state, clear_r))
	MCFG_COSMAC_EF1_CALLBACK(READLINE(play_3_state, ef1_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(play_3_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(DEVWRITELINE("4013a", ttl7474_device, clear_w))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("tpb_clock", CLOCK, 2950000 / 8) // TPB line from CPU
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_3_state, clock_w))

	MCFG_DEVICE_ADD("xpoint", CLOCK, 60) // crossing-point detector
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(play_3_state, clock2_w))

	// This is actually a 4013 chip (has 2 RS flipflops)
	MCFG_DEVICE_ADD("4013a", TTL7474, 0)
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("4013a", ttl7474_device, d_w))
	MCFG_7474_OUTPUT_CB(WRITELINE(play_3_state, q4013a_w))

	MCFG_DEVICE_ADD("4013b", TTL7474, 0)
	MCFG_7474_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, ef2_w))
	MCFG_7474_COMP_OUTPUT_CB(DEVWRITELINE("maincpu", cosmac_device, int_w)) MCFG_DEVCB_INVERT // int is reversed in mame

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Meg Aaton (1983)
/-------------------------------------------------------------------*/

ROM_START(megaaton)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpumegat.bin", 0x0000, 0x2000, CRC(7e7a4ede) SHA1(3194b367cbbf6e0cb2629cd5d82ddee6fe36985a))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

ROM_START(megaatona)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mega_u12.bin", 0x0000, 0x1000, CRC(65761b02) SHA1(dd9586eaf70698ef7a80ce1be293322f64829aea))
	ROM_LOAD("mega_u11.bin", 0x1000, 0x1000, CRC(513f3683) SHA1(0f080a33426df1ffdb14e9b2e6382304e201e335))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("smogot.bin", 0x0000, 0x2000, CRC(fefc3ab2) SHA1(e748d9b443a69fcdd587f22c87d41818b6c0e436))
	ROM_LOAD("smegat.bin", 0x2000, 0x1000, CRC(910ab7fe) SHA1(0ddfd15c9c25f43b8fcfc4e11bc8ea180f6bd761))
ROM_END

GAME(1983,  megaaton,  0,         play_3,  play_3, play_3_state,  play_3,  ROT0,  "Playmatic",    "Meg-Aaton",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1983,  megaatona, megaaton,  play_3,  play_3, play_3_state,  play_3,  ROT0,  "Playmatic",    "Meg-Aaton (alternate set)",     MACHINE_IS_SKELETON_MECHANICAL)
