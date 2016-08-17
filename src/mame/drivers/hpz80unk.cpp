// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

HP Z80-based unknown in a large metal cage

2012-05-25 Skeleton driver [Robbbert]

http://www.classiccmp.org/hp/unknown Z80 computer/

Looks like roms are in 2 banks in range C000-FFFF.
BASIC is included, if we can find out how to access it.

Commands:
A disassemble
D
G
H
L
M
P Read Port
R
U
W Punch papertape
X choose Q,V,R,P (Q to quit; others ask for ram and prom ranges)
Y nothing
Z nothing

    ToDo:
    - Almost everything; there are a lot of I/O ports used
    - Hook up rom banking

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class hpz80unk_state : public driver_device
{
public:
	hpz80unk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_p_rom(*this, "p_rom")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(port02_r);
	DECLARE_READ8_MEMBER(port03_r);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_READ8_MEMBER(portfc_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	required_shared_ptr<UINT8> m_p_rom;
	UINT8 m_term_data;
	UINT8 m_port02_data;
	virtual void machine_reset() override;
};

READ8_MEMBER( hpz80unk_state::port02_r )
{
	m_port02_data ^= 1;
	return m_port02_data;
}

READ8_MEMBER( hpz80unk_state::port03_r )
{
	return (m_term_data) ? 0xff : 0xfd;
}

READ8_MEMBER( hpz80unk_state::port04_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( hpz80unk_state::portfc_r )
{
	return 0xfe; // or it halts
}

static ADDRESS_MAP_START( hpz80unk_mem, AS_PROGRAM, 8, hpz80unk_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_ROM AM_SHARE("p_rom")
ADDRESS_MAP_END

static ADDRESS_MAP_START( hpz80unk_io, AS_IO, 8, hpz80unk_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x02, 0x02) AM_READ(port02_r)
	AM_RANGE(0x03, 0x03) AM_READ(port03_r)
	AM_RANGE(0x04, 0x04) AM_READ(port04_r)
	AM_RANGE(0xfc, 0xfc) AM_READ(portfc_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( hpz80unk )
INPUT_PORTS_END


void hpz80unk_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();
	memcpy((UINT8*)m_p_rom, user1, 0x4000);

	// this should be rom/ram banking
}

WRITE8_MEMBER( hpz80unk_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( hpz80unk, hpz80unk_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(hpz80unk_mem)
	MCFG_CPU_IO_MAP(hpz80unk_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(hpz80unk_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hpz80unk )
	ROM_REGION( 0x8000, "user1", 0 )
	// 1st bank
	ROM_LOAD( "u1",        0x0000, 0x0800, CRC(080cd04a) SHA1(42004af65d44e3507a4e0f343c5bf385b6377c40) )
	ROM_LOAD( "u3",        0x0800, 0x0800, CRC(694075e1) SHA1(3db62645ade6a7f454b2d505aecc1661284c8ce2) )
	ROM_LOAD( "u5",        0x1000, 0x0800, CRC(5573bd05) SHA1(68c8f02b3fe9d77ecb83df407ca78430e118004a) )
	ROM_LOAD( "u7",        0x1800, 0x0800, CRC(d18a304a) SHA1(69dd0486bb6e4c2a22ab9da863bfb962016a321b) )
	ROM_LOAD( "u9",        0x2000, 0x0800, CRC(f7a8665c) SHA1(e39d0ba4ce2dc773622d411a25f40a6a24b45449) )
	ROM_LOAD( "u11",       0x2800, 0x0800, CRC(6c1ac77a) SHA1(50ca04ff0a11bd1c7d96f4731cef50978266ecca) )
	ROM_LOAD( "u13",       0x3000, 0x0800, CRC(8b166911) SHA1(4301dcd6840d37ccfa5bff998a0d88bebe99dc31) )
	ROM_LOAD( "u15",       0x3800, 0x0800, CRC(c6300499) SHA1(1b62d2a85c8f0b6a817e4be73ee34e0d90515c00) )
	// 2nd bank
	ROM_LOAD( "u2",        0x4000, 0x0800, CRC(080cd04a) SHA1(42004af65d44e3507a4e0f343c5bf385b6377c40) )
	ROM_LOAD( "u4",        0x4800, 0x0800, CRC(66c3745c) SHA1(d79fe764312a222ac64d325bf5f4abc7ca401d0f) )
	ROM_LOAD( "u6",        0x5000, 0x0800, CRC(80761b4c) SHA1(5f6a12fbba533308b9fe7067c67a836be436a6f0) )
	ROM_LOAD( "u8",        0x5800, 0x0800, CRC(64a2be18) SHA1(b11c08fdc9dc126038559462493f458ecdc78532) )
	ROM_LOAD( "u10",       0x6000, 0x0800, CRC(40244d09) SHA1(106f8f978de36df9f3ebbe1e2c959b60e53273a2) )
	ROM_LOAD( "u12",       0x6800, 0x0800, CRC(6eb01765) SHA1(66f9036a9f86cf3a79493330bbc06fb6932ab771) )
	ROM_LOAD( "u14",       0x7000, 0x0800, CRC(3410e682) SHA1(30d94c0c0b6478dab202a603edaccca943008e35) )
	ROM_LOAD( "u16",       0x7800, 0x0800, CRC(c03fdcab) SHA1(1081d787085add489c6e2a1d450e1a5790d18885) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT   MACHINE    INPUT      INIT     COMPANY                          FULLNAME       FLAGS */
COMP( 1977, hpz80unk, 0,      0,       hpz80unk,  hpz80unk, driver_device,  0,   "Hewlett-Packard", "unknown Z80-based mainframe", MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
