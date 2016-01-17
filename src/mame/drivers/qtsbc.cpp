// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        QT Computer Systems SBC +2/4

        11/12/2009 Skeleton driver.

    It expects a rom or similar at E377-up, so currently it crashes.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class qtsbc_state : public driver_device
{
public:
	qtsbc_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_p_ram(*this, "p_ram")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER( qtsbc_06_r );
	DECLARE_READ8_MEMBER( qtsbc_43_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	required_shared_ptr<UINT8> m_p_ram;
	UINT8 m_term_data;
	virtual void machine_reset() override;
};


READ8_MEMBER( qtsbc_state::qtsbc_06_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( qtsbc_state::qtsbc_43_r )
{
	return 0;
}

static ADDRESS_MAP_START(qtsbc_mem, AS_PROGRAM, 8, qtsbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xffff ) AM_RAM AM_SHARE("p_ram") AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( qtsbc_io, AS_IO, 8, qtsbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x06, 0x06) AM_READ(qtsbc_06_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x43, 0x43) AM_READ(qtsbc_43_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( qtsbc )
INPUT_PORTS_END


void qtsbc_state::machine_reset()
{
	UINT8* bios = memregion("maincpu")->base()+0x10000;
	memcpy(m_p_ram, bios, 0x800);
}

WRITE8_MEMBER( qtsbc_state::kbd_put )
{
	m_term_data = data;
}

static MACHINE_CONFIG_START( qtsbc, qtsbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz) // Mostek MK3880
	MCFG_CPU_PROGRAM_MAP(qtsbc_mem)
	MCFG_CPU_IO_MAP(qtsbc_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(qtsbc_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( qtsbc )
	ROM_REGION( 0x10800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "qtsbc.bin", 0x10000, 0x0800, CRC(823fd942) SHA1(64c4f74dd069ae4d43d301f5e279185f32a1efa0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY                FULLNAME       FLAGS */
COMP( 19??, qtsbc,  0,       0,      qtsbc,     qtsbc, driver_device,    0,  "Computer Systems Inc.", "QT SBC +2/4", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
