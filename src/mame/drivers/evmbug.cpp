// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Stuart's Breadboard Project. TMS9995 evaluation kit TMAM6095.

        2013-06-02 Skeleton driver.

        http://www.avjd51.dsl.pipex.com/tms9995_eval_module/tms9995_eval_module.htm


****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class evmbug_state : public driver_device
{
public:
	evmbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_READ8_MEMBER(rs232_r);
	DECLARE_WRITE8_MEMBER(rs232_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	virtual void machine_reset() override;
	UINT8 m_term_data;
	UINT8 m_term_out;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START( evmbug_mem, AS_PROGRAM, 8, evmbug_state )
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0xec00, 0xefff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( evmbug_io, AS_IO, 8, evmbug_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0007) AM_WRITE(rs232_w)
	AM_RANGE(0x0000, 0x0002) AM_READ(rs232_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( evmbug )
INPUT_PORTS_END

READ8_MEMBER( evmbug_state::rs232_r )
{
	static UINT8 temp = 0;
	temp^=0xff;
	if (offset == 1)
		return temp;

	if (offset == 2)
	{
		return 0xff;//(m_term_data) ? 0 : 0xff;
	}

	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

WRITE8_MEMBER( evmbug_state::rs232_w )
{
	if (offset == 0)
		m_term_out = 0;

	m_term_out |= (data << offset);

	if (offset == 7)
		m_terminal->write(space, 0, m_term_out & 0x7f);
}

WRITE8_MEMBER( evmbug_state::kbd_put )
{
	m_term_data = data;
}

void evmbug_state::machine_reset()
{
	m_term_data = 0;
	// Disable auto wait state generation by raising the READY line on reset
	static_cast<tms9995_device*>(machine().device("maincpu"))->ready_line(ASSERT_LINE);
}

static MACHINE_CONFIG_START( evmbug, evmbug_state )
	// basic machine hardware
	// TMS9995 CPU @ 12.0 MHz
	// We have no lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, 12000000, evmbug_mem, evmbug_io )

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(evmbug_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( evmbug )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
//  ROM_LOAD( "u8.bin", 0x0000, 0x1000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f) )
//  ROM_LOAD( "u9.bin", 0x1000, 0x0800, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9) )
	ROM_LOAD( "evmbug.bin", 0x0000, 0x8000, CRC(a239ec56) SHA1(65b500d7d0f897ce0c320cf3ec32ff4042774599) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                  FULLNAME       FLAGS */
COMP( 19??, evmbug, 0,      0,       evmbug,    evmbug, driver_device,  0,    "Texas Instruments",   "TMAM6095", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
