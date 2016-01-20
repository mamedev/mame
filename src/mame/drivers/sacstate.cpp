// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SacState 8008

        23/02/2009 Skeleton driver.

http://www.digibarn.com/stories/bill-pentz-story/index.html

The year is listed as '1972/73'.

All input to be in lowercase.
The weird characters that show on screen are various escape sequences.
These are different depending on the terminal type chosen. The codes
need to be understood and emulated before this system makes sense.

Known Monitor commands: (from the disassembly)
!   write buffer to AD000
*   set RAM036 flag
+   increment AD000 by 1
+n  increment AD000 by n
-   decrement AD000 by 1
-n  decrement AD000 by n
:   clear RAM036 flag
=   display AD000
=nnn    set AD000 to nnn
@   fill buffer with 026

unknown commands: / & d e l r s t u z \ ^ | ~

Other input will either result in '!' message, or halt.


****************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class sacstate_state : public driver_device
{
public:
	sacstate_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(port00_r);
	DECLARE_READ8_MEMBER(port01_r);
	DECLARE_READ8_MEMBER(port04_r);
	DECLARE_WRITE8_MEMBER(port08_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	UINT8 m_val;
	virtual void machine_reset() override;
};

READ8_MEMBER( sacstate_state::port01_r )
{
	UINT8 ret = m_val;
	if (m_term_data)
		ret |= 0x04; // data in
	return ret;
}

READ8_MEMBER( sacstate_state::port00_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( sacstate_state::port04_r )
{
	logerror("unknown_r\n");
	return 0;
}

WRITE8_MEMBER( sacstate_state::port08_w )
{
	if (data == 0x40)
		m_val = 0x40;
	else
	if (data == 0x04)
		m_val = 0;
}

static ADDRESS_MAP_START(sacstate_mem, AS_PROGRAM, 8, sacstate_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x000,0x7ff) AM_ROM
	AM_RANGE(0x800,0xfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sacstate_io, AS_IO, 8, sacstate_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00,0x00) AM_READ(port00_r)
	AM_RANGE(0x01,0x01) AM_READ(port01_r)
	AM_RANGE(0x04,0x04) AM_READ(port04_r)
	AM_RANGE(0x08,0x08) AM_WRITE(port08_w)
	AM_RANGE(0x16,0x16) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x17,0x1f) AM_WRITENOP
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sacstate )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x08, 0x08, "Terminal Type") // no idea of actual terminal types
	PORT_CONFSETTING(    0x00, "A")
	PORT_CONFSETTING(    0x08, "B")
INPUT_PORTS_END

WRITE8_MEMBER( sacstate_state::kbd_put )
{
	m_term_data = data;
}

void sacstate_state::machine_reset()
{
	m_term_data = 0;
	m_val = ioport("CONFIG")->read();
}

static MACHINE_CONFIG_START( sacstate, sacstate_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8008, 800000)
	MCFG_CPU_PROGRAM_MAP(sacstate_mem)
	MCFG_CPU_IO_MAP(sacstate_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(sacstate_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sacstate )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sacst1.bin", 0x0700, 0x0100, CRC(ba020160) SHA1(6337cdf65583808768664653c937e50040aec6d4))
	ROM_LOAD( "sacst2.bin", 0x0600, 0x0100, CRC(26f3e505) SHA1(3526060dbd1bf885c2e686bc9a6082387630952a))
	ROM_LOAD( "sacst3.bin", 0x0500, 0x0100, CRC(965b3474) SHA1(6d9142e68d375fb000fd6ea48369d0801274ded6))
	ROM_LOAD( "sacst4.bin", 0x0400, 0x0100, CRC(3cd3e169) SHA1(75a99e8e4dbd6e054209a4979bb498f37e962697))
	ROM_LOAD( "sacst5.bin", 0x0300, 0x0100, CRC(30619454) SHA1(cb498880bec27c9adc44dc1267858555000452c6))
	ROM_LOAD( "sacst6.bin", 0x0200, 0x0100, CRC(a4cd2ff6) SHA1(3f4da5510c0778eb770c96c01f91f5cb7f5285fa))
	ROM_LOAD( "sacst7.bin", 0x0100, 0x0100, CRC(33971d8b) SHA1(9e0bbeef6a6a15107f270e8b285300284ee7f63f))
	ROM_LOAD( "sacst8.bin", 0x0000, 0x0100, CRC(931252ef) SHA1(e06ea6947f432f0a4ce944de74978d929920fb53))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     CLASS          INIT    COMPANY      FULLNAME       FLAGS */
COMP( 1973, sacstate,  0,       0,   sacstate,  sacstate, driver_device,  0,  "SacState", "SacState 8008", MACHINE_NO_SOUND_HW)
