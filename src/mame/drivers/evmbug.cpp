// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

Stuart's Breadboard Project. TMS9995 evaluation kit TMAM6095.

2013-06-02 Skeleton driver.

http://www.stuartconner.me.uk/tms9995_eval_module/tms9995_eval_module.htm

It uses TMS9902 UART for comms, but our implementation of that chip is
not ready for rs232.h as yet.

Press any key to get it started. All input to be in uppercase. Haven't found
any commands that work as yet.

****************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
//#include "machine/tms9902.h"
#include "machine/terminal.h"


class evmbug_state : public driver_device
{
public:
	evmbug_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
	{ }

	DECLARE_READ8_MEMBER(rs232_r);
	DECLARE_WRITE8_MEMBER(rs232_w);
	void kbd_put(u8 data);

	void evmbug(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	uint8_t m_term_data;
	uint8_t m_term_out;
	bool m_rin;
	bool m_rbrl;
	required_device<tms9995_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

ADDRESS_MAP_START(evmbug_state::mem_map)
	AM_RANGE(0x0000, 0x17ff) AM_ROM
	AM_RANGE(0xec00, 0xefff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(evmbug_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0x0003) AM_DEVREAD("uart1", tms9902_device, cruread)
	//AM_RANGE(0x0000, 0x001f) AM_DEVWRITE("uart1", tms9902_device, cruwrite)
	AM_RANGE(0x0000, 0x0003) AM_READ(rs232_r)
	AM_RANGE(0x0000, 0x001f) AM_WRITE(rs232_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( evmbug )
INPUT_PORTS_END

READ8_MEMBER( evmbug_state::rs232_r )
{
	if (offset == 0)
		return m_term_data;
	else
	if (offset == 2)
		return (m_rbrl ? 0x20 : 0) | 0xc0;
	else
	{
		m_rin ^= 1;
		return m_rin << 7;
	}
}

WRITE8_MEMBER( evmbug_state::rs232_w )
{
	if (offset < 8)
	{
		if (offset == 0)
			m_term_out = 0;

		m_term_out |= (data << offset);

		if (offset == 7)
			m_terminal->write(space, 0, m_term_out & 0x7f);
	}
	else
	if (offset == 18)
		m_rbrl = 0;
}

void evmbug_state::kbd_put(u8 data)
{
	m_term_data = data;
	m_rbrl = data ? 1 : 0;
}

void evmbug_state::machine_reset()
{
	m_rbrl = 0;
	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);
}

MACHINE_CONFIG_START(evmbug_state::evmbug)
	// basic machine hardware
	// TMS9995 CPU @ 12.0 MHz
	// We have no lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, XTAL(12'000'000), mem_map, io_map )

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(evmbug_state, kbd_put))

	//MCFG_DEVICE_ADD("uart1", TMS9902, XTAL(12'000'000) / 4)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( evmbug )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "evmbug.bin", 0x0000, 0x8000, CRC(a239ec56) SHA1(65b500d7d0f897ce0c320cf3ec32ff4042774599) )
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE          INIT  COMPANY                FULLNAME    FLAGS
COMP( 19??, evmbug, 0,      0,       evmbug,    evmbug, evmbug_state,  0,    "Texas Instruments",   "TMAM6095", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
