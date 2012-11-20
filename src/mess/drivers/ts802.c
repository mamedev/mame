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
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(ts802_mem)
	MCFG_CPU_IO_MAP(ts802_io)

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts802 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "ts802.rom", 0x0000, 0x1000, CRC(60bd086a) SHA1(82c5b60223e0d895683d3592a56684ef2dabfba6) )
ROM_END

ROM_START( ts802h )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "8000050 050 2732", 0x0000, 0x1000, CRC(7054f384) SHA1(cf0a01a32283272532ed4890c3a3c2082f1618bf) )
	ROM_LOAD( "i800000 047d.a53", 0x1000, 0x1000, CRC(94bfcbc1) SHA1(87c5f8898b0041d012e142ee7f559cb8a90f4dc1) )
	ROM_LOAD( "a64", 0x2000, 0x1000, CRC(41b5feda) SHA1(c9435a97c032ffe457bdb84d5dde8ecf3677b56c) )
	ROM_LOAD( "800000-002a.a67", 0x3000, 0x0800, CRC(4b6c6e29) SHA1(c236e4625bc16062154cbebc4dbc8d62183ef9ab) )
	ROM_LOAD( "800000-003a.a68", 0x3800, 0x0800, CRC(24eeb74d) SHA1(77900937f1492b4c5a70ba3aac55da322d403fbd) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 19??, ts802,   0,       0,     ts802,     ts802, driver_device,  0,  "Televideo", "TS802", GAME_IS_SKELETON )
COMP( 19??, ts802h,  ts802,   0,     ts802,     ts802, driver_device,  0,  "Televideo", "TS802H", GAME_IS_SKELETON )
