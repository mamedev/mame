// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Imsai MPU-B. One of the earliest single-board computers on a S100 card.

    2013-09-11 Skeleton driver.

    Chips used: i8085, i8251, i8253, 3622 fusable prom. XTAL 6MHz

    Press any key to start the monitor program.

    ToDo:
    - Banking
    - Dipswitches

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class imsai_state : public driver_device
{
public:
	imsai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG),
		m_uart(*this, "uart"),
		m_pit(*this, "pit")
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE_LINE_MEMBER(write_uart_clock);

private:
	UINT8 m_term_data;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<i8251_device> m_uart;
	required_device<pit8253_device> m_pit;
};


static ADDRESS_MAP_START(imsai_mem, AS_PROGRAM, 8, imsai_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0xd000, 0xd0ff) AM_RAM
	AM_RANGE(0xd100, 0xd103) AM_DEVREADWRITE("pit", pit8253_device, read, write)
	AM_RANGE(0xd800, 0xdfff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(imsai_io, AS_IO, 8, imsai_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x02, 0x02) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x03, 0x03) AM_READ(status_r)
	AM_RANGE(0x04, 0x04) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x05, 0x05) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x12, 0x12) AM_DEVREADWRITE("uart", i8251_device, data_r, data_w)
	AM_RANGE(0x13, 0x13) AM_DEVREADWRITE("uart", i8251_device, status_r, control_w)
	AM_RANGE(0x14, 0x14) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x15, 0x15) AM_READ(status_r)
	AM_RANGE(0xf3, 0xf3) AM_WRITE(control_w)
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

WRITE_LINE_MEMBER(imsai_state::write_uart_clock)
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

WRITE8_MEMBER( imsai_state::control_w )
{
}

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
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(imsai_state, kbd_put))

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_6MHz / 3) /* Timer 0: baud rate gen for 8251 */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(imsai_state, write_uart_clock))
	MCFG_PIT8253_CLK1(XTAL_6MHz / 3) /* Timer 1: user */
	MCFG_PIT8253_CLK2(XTAL_6MHz / 3) /* Timer 2: user */
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( imsai )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "vdb-80.rom",   0x0000, 0x0800, CRC(0afc4683) SHA1(a5419aaee00badf339d7c627f50ef8b2538e42e2) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT  CLASS         INIT  COMPANY  FULLNAME   FLAGS */
COMP( 1978, imsai,   0,      0,       imsai,     imsai, driver_device, 0,   "Imsai", "MPU-B", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
