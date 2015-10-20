// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Jade JGZ80 Single board computer on a S100 card.

    2013-09-12 Skeleton driver.

    No info found as yet.

    It takes about 8 seconds to start up.
    Type HE to get a list of commands.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8251.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class jade_state : public driver_device
{
public:
	jade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_uart(*this, "uart")
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_READ8_MEMBER(data_r);
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8251_device> m_uart;
};


static ADDRESS_MAP_START(jade_mem, AS_PROGRAM, 8, jade_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xe000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(jade_io, AS_IO, 8, jade_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x30, 0x30) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	//AM_RANGE(0x32, 0x32) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x30, 0x30) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x32, 0x32) AM_READ(status_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( jade )
INPUT_PORTS_END

READ8_MEMBER( jade_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return (ret) ? ret : 0x13;
}

READ8_MEMBER( jade_state::status_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( jade_state::kbd_put )
{
	m_term_data = data;
}

void jade_state::machine_reset()
{
	m_term_data = 0;
}

static MACHINE_CONFIG_START( jade, jade_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(jade_mem)
	MCFG_CPU_IO_MAP(jade_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(jade_state, kbd_put))

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( jade )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "jgz80.rom",   0x0000, 0x0800, CRC(90c4a1ef) SHA1(8a93a11051cc27f3edca24f0f4297ebe0099964e) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT  CLASS         INIT  COMPANY  FULLNAME   FLAGS */
COMP( 19??, jade,    0,      0,       jade,      jade,  driver_device, 0,   "Jade", "JGZ80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
