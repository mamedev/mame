// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/***************************************************************************

        CM-1800
        (note name is in cyrilic letters)

        more info at http://ru.wikipedia.org/wiki/%D0%A1%D0%9C_%D0%AD%D0%92%D0%9C
            and http://sapr.lti-gti.ru/index.php?id=66

        26/04/2011 Skeleton driver.

Commands:
C Compare
D Dump
F Fill
G
I
L
M Move
S Edit
T
X Registers

For most commands, enter all 4 digits of each hex address, the system will
add the appropriate spacing as you type. No need to press Enter.

The L command looks like it might be for loading a file, for example
L ABC will read/write to port 70,71,73 and eventually time out if you wait
a while. No idea if it wants to read a disk or a tape. There doesn't seem
to be a save command.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class cm1800_state : public driver_device
{
public:
	cm1800_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_terminal(*this, TERMINAL_TAG) ,
		m_maincpu(*this, "maincpu")
	{
	}

	DECLARE_READ8_MEMBER( term_status_r );
	DECLARE_READ8_MEMBER( term_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;

	required_device<generic_terminal_device> m_terminal;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

READ8_MEMBER( cm1800_state::term_status_r )
{
	return (m_term_data) ? 5 : 4;
}

READ8_MEMBER( cm1800_state::term_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( cm1800_state::kbd_put )
{
	m_term_data = data;
}

static ADDRESS_MAP_START(cm1800_mem, AS_PROGRAM, 8, cm1800_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	AM_RANGE( 0x0800, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cm1800_io , AS_IO, 8, cm1800_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_READ(term_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x01, 0x01) AM_READ(term_status_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cm1800 )
INPUT_PORTS_END


void cm1800_state::machine_reset()
{
}

static MACHINE_CONFIG_START( cm1800, cm1800_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(cm1800_mem)
	MCFG_CPU_IO_MAP(cm1800_io)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(cm1800_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cm1800 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "cm1800.rom", 0x0000, 0x0800, CRC(85d71d25) SHA1(42dc87d2eddc2906fa26d35db88a2e29d50fb481) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1981, cm1800,  0,       0,    cm1800,    cm1800, driver_device,     0,  "<unknown>", "CM-1800", MACHINE_NO_SOUND_HW)
