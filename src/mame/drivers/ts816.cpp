// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    2013-09-10 Skeleton driver for Televideo ts816

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class ts816_state : public driver_device
{
public:
	ts816_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);

private:
	UINT8 m_term_data;
	UINT8 m_status;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};

static ADDRESS_MAP_START(ts816_mem, AS_PROGRAM, 8, ts816_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x0fff) AM_ROM
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ts816_io, AS_IO, 8, ts816_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x50, 0x50) AM_READ(keyin_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x52, 0x52) AM_READ(status_r)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( ts816 )
INPUT_PORTS_END


READ8_MEMBER( ts816_state::keyin_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ts816_state::status_r )
{
	if (m_status)
	{
		m_status--;
		return 5;
	}
	else
		return 4;
}

WRITE8_MEMBER( ts816_state::kbd_put )
{
	m_term_data = data;
	m_status = 3;
}

void ts816_state::machine_reset()
{
	m_term_data = 0;
	m_status = 1;
}

static MACHINE_CONFIG_START( ts816, ts816_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(ts816_mem)
	MCFG_CPU_IO_MAP(ts816_io)

	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(ts816_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ts816 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "81640v11.rom", 0x0000, 0x1000, CRC(295a15e7) SHA1(6f49078ab3cd49aecd2afafcbed3af0e3bcfd48c) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1980, ts816,  0,      0,       ts816,     ts816, driver_device,  0,  "Televideo", "TS816", MACHINE_IS_SKELETON )
