// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

Microtek International Inc MICE

2013-08-27 Skeleton driver.

This is a CPU emulator for development work.

Each CPU has a plugin card with various chips. The usual complement is
 the selected CPU, a 8085, 8255/8251/8155/6116, eproms and crystals.

The connection to the outside world is via RS232 to a terminal.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class mice_state : public driver_device
{
public:
	mice_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(port50_r);
	DECLARE_READ8_MEMBER(port51_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset() override;
};


READ8_MEMBER( mice_state::port50_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( mice_state::port51_r )
{
	return (m_term_data) ? 5 : 1;
}

static ADDRESS_MAP_START(mice_mem, AS_PROGRAM, 8, mice_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x5fff ) AM_ROM AM_REGION("mice_6502", 0)
	AM_RANGE( 0x6000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mice_io, AS_IO, 8, mice_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x50, 0x50) AM_READ(port50_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x51, 0x51) AM_READ(port51_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mice )
INPUT_PORTS_END


void mice_state::machine_reset()
{
}

WRITE8_MEMBER( mice_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( mice, mice_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8085A, XTAL_6_144MHz)
	MCFG_CPU_PROGRAM_MAP(mice_mem)
	MCFG_CPU_IO_MAP(mice_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(mice_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mice )
	ROM_REGION( 0x10000, "mice2_z80b", 0 )
	ROM_LOAD( "z80_u2_v.3.0",   0x4000, 0x2000, CRC(992b1b53) SHA1(f7b66c49ab26a9f97b2e6ebe45d162daa66d8a67) )
	ROM_LOAD( "z80_u3_v.3.0",   0x2000, 0x2000, CRC(48d0be9b) SHA1(602af21868b1b5e6d488706a831259d78fefad6f) )
	ROM_LOAD( "z80_u4_v.3.0",   0x0000, 0x2000, CRC(4fe2d08d) SHA1(902b98357b8f2e61f68dd171478368a3ac47af6e) )
	ROM_REGION( 0x10000, "mice2_6502", 0 )
	ROM_LOAD( "6502_u1_v.3.2",  0x6000, 0x2000, CRC(0ba10943) SHA1(e7590e2c1d9d2b1ff8cca0f5da366650ea4d50e3) )
	ROM_LOAD( "6502_u2_v.3.2",  0x4000, 0x2000, CRC(f3169423) SHA1(a588a2e1894f523cf11c34d036beadbfe5b10538) )
	ROM_LOAD( "6502_u3_v.3.2",  0x2000, 0x2000, CRC(d5c77c3f) SHA1(71439735ed62db07bee713775ee2189120d1a1e7) )
	ROM_LOAD( "6502_u4_v.3.2",  0x0000, 0x2000, CRC(6acfc3a1) SHA1(3572a4798873c21a247a43da8419e7b9a181c67d) )
	ROM_REGION( 0x10000, "mice2_8085", 0 )
	ROM_LOAD( "8085_u2_v.3.1",  0x4000, 0x2000, CRC(2fce00a5) SHA1(0611f928be663a9279781d9f496fc950fd4ee7e2) )
	ROM_LOAD( "8085_u3_v.3.1",  0x2000, 0x2000, CRC(16ee3018) SHA1(9e215504bcea2c5ebfb7578ecf371eec45cbe5d7) )
	ROM_LOAD( "8085_u4_v.3.1",  0x0000, 0x2000, CRC(5798f2b5) SHA1(e0fe9411394bded8a77bc6a0f71519aad7800125) )
	ROM_REGION( 0x10000, "mice2_6809", 0 )
	ROM_LOAD( "6809_u1_v.3.4",  0x0000, 0x8000, CRC(b94d043d) SHA1(822697485f064286155f2a66cdbdcb0bd66ddb8c) )
	ROM_REGION( 0x10000, "mice_6502", 0 )
	ROM_LOAD( "6502_u10_v.2.0", 0x2000, 0x1000, CRC(496c53a7) SHA1(f28cddef18ab3e0eca1fea125dd678a54817c9df) )
	ROM_LOAD( "6502_u11_v.2.0", 0x1000, 0x1000, CRC(8d655bd2) SHA1(94936553f1692ede0934e3c7b599f3ad6adb6aec) )
	ROM_LOAD( "6502_u12_v.2.0", 0x0000, 0x1000, CRC(cee810ee) SHA1(ab642cda73f4b3f715ddc2909ba2b48cbd474d4d) )
ROM_END

/* Driver */

/*    YEAR  NAME   PARENT  COMPAT   MACHINE   INPUT CLASS          INIT     COMPANY                  FULLNAME       FLAGS */
COMP( 1980, mice,  0,      0,       mice,     mice, driver_device,   0,  "Microtek International Inc", "Mice", MACHINE_IS_SKELETON )
