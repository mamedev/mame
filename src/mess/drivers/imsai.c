/***************************************************************************

    Imsai MPU-B. One of the earliest single-board computers on a S100 card.

    2013-09-11 Skeleton driver.

    Chips used: i8085, i8251, i8253, 3622 fusable prom. XTAL 6MHz

    Press any key to start the monitor program.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"


class imsai_state : public driver_device
{
public:
	imsai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
	{ }

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


static ADDRESS_MAP_START(imsai_mem, AS_PROGRAM, 8, imsai_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM
	AM_RANGE(0xd800, 0xdfff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(imsai_io, AS_IO, 8, imsai_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x03, 0x03) AM_READ(status_r)
	AM_RANGE(0x12, 0x12) AM_READ(keyin_r)
	AM_RANGE(0x13, 0x13) AM_READ(status_r)
	AM_RANGE(0x14, 0x14) AM_READ(keyin_r)
	AM_RANGE(0x15, 0x15) AM_READ(status_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( imsai )
INPUT_PORTS_END

READ8_MEMBER( imsai_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( imsai_state::status_r )
{
	return (m_term_data) ? 3 : 1;
}

WRITE8_MEMBER( imsai_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(imsai_state, kbd_put)
};

void imsai_state::machine_reset()
{
	m_term_data = 0;
}

static MACHINE_CONFIG_START( imsai, imsai_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_6MHz)
	MCFG_CPU_PROGRAM_MAP(imsai_mem)
	MCFG_CPU_IO_MAP(imsai_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( imsai )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "vdb-80.rom",   0x0000, 0x0800, CRC(0afc4683) SHA1(a5419aaee00badf339d7c627f50ef8b2538e42e2) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT  CLASS         INIT  COMPANY  FULLNAME   FLAGS */
COMP( 1978, imsai,   0,      0,       imsai,     imsai, driver_device, 0,   "Imsai", "MPU-B", GAME_NOT_WORKING | GAME_NO_SOUND_HW )
