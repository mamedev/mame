/***************************************************************************

    Altos 5-15

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"


class altos5_state : public driver_device
{
public:
	altos5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
			{ }

	DECLARE_READ8_MEMBER(altos_2f_r);
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	required_device<cpu_device> m_maincpu;
	virtual void machine_reset();
};

READ8_MEMBER( altos5_state::altos_2f_r )
{
	return 0x0c;
}

WRITE8_MEMBER( altos5_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(altos5_state, kbd_put)
};


static ADDRESS_MAP_START(altos5_mem, AS_PROGRAM, 8, altos5_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(altos5_io, AS_IO, 8, altos5_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x2e, 0x2e) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x2f, 0x2f) AM_READ(altos_2f_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( altos5 )
INPUT_PORTS_END

void altos5_state::machine_reset()
{
	UINT8 *m_p_maincpu = machine().root_device().memregion("maincpu")->base();
	UINT8 *m_p_roms = machine().root_device().memregion("roms")->base();
	memcpy(m_p_maincpu, m_p_roms, 0x1000);
}

static MACHINE_CONFIG_START( altos5, altos5_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(altos5_mem)
	MCFG_CPU_IO_MAP(altos5_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( altos5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD("2732.bin",   0x0000, 0x1000, CRC(15fdc7eb) SHA1(e15bdf5d5414ad56f8c4bb84edc6f967a5f01ba9))
	ROM_LOAD("82s141.bin", 0x1000, 0x0200, CRC(35c8078c) SHA1(dce24374bfcc5d23959e2c03485d82a119c0c3c9))
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT    INIT   COMPANY    FULLNAME       FLAGS */
COMP(19??, altos5, 0,      0,       altos5,  altos5, driver_device,  0,    "Altos", "Altos 5-15", GAME_NOT_WORKING | GAME_NO_SOUND)
