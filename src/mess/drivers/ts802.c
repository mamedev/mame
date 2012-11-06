/***************************************************************************

    Skeleton driver for Televideo TS802

    2012-11-06
    After 5 seconds it shows an error and loops.

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"


class ts802_state : public driver_device
{
public:
	ts802_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(port00_r) { return 0x80; };
	DECLARE_READ8_MEMBER(port0f_r) { return 0x05; };
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	virtual void machine_reset();
};

static ADDRESS_MAP_START(ts802_mem, AS_PROGRAM, 8, ts802_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ts802_io, AS_IO, 8, ts802_state)
	//ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ(port00_r)
	AM_RANGE(0x0d, 0x0d) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x0f, 0x0f) AM_READ(port0f_r)
	AM_RANGE(0x20, 0x20) AM_WRITENOP
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ts802 )
INPUT_PORTS_END


void ts802_state::machine_reset()
{
}

WRITE8_MEMBER( ts802_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ts802_state, kbd_put)
};

static MACHINE_CONFIG_START( ts802, ts802_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000) // more likely a 8080
	MCFG_CPU_PROGRAM_MAP(ts802_mem)
	MCFG_CPU_IO_MAP(ts802_io)

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts802 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ts802.rom", 0x0000, 0x1000, CRC(60bd086a) SHA1(82c5b60223e0d895683d3592a56684ef2dabfba6) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 19??, ts802,   0,       0,     ts802,     ts802, driver_device,  0,  "Televideo", "TS802", GAME_IS_SKELETON)
