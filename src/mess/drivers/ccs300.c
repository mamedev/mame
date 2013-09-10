/***************************************************************************

        CCS Model 300

        2009-12-11 Skeleton driver.

It requires a floppy disk to boot from.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"


class ccs300_state : public driver_device
{
public:
	ccs300_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_ccs_ram(*this, "ccs_ram")
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, TERMINAL_TAG)
	{ }

	DECLARE_READ8_MEMBER(port10_r);
	DECLARE_READ8_MEMBER(port11_r);
	DECLARE_WRITE8_MEMBER(port10_w);
	DECLARE_WRITE8_MEMBER(kbd_put);
	required_shared_ptr<UINT8> m_ccs_ram;
private:
	UINT8 m_term_data;
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
};


READ8_MEMBER( ccs300_state::port10_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ccs300_state::port11_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( ccs300_state::port10_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

static ADDRESS_MAP_START(ccs300_mem, AS_PROGRAM, 8, ccs300_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM AM_SHARE("ccs_ram")
	AM_RANGE(0xf000, 0xf7ff) AM_ROM AM_REGION("user1", 0xf000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ccs300_io, AS_IO, 8, ccs300_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READWRITE(port10_r,port10_w)
	AM_RANGE(0x11, 0x11) AM_READ(port11_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs300 )
INPUT_PORTS_END


void ccs300_state::machine_reset()
{
	UINT8* user1 = memregion("user1")->base();
	memcpy((UINT8*)m_ccs_ram, user1, 0x0800);

	// this should be rom/ram banking
}

WRITE8_MEMBER( ccs300_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ccs300_state, kbd_put)
};

static MACHINE_CONFIG_START( ccs300, ccs300_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ccs300_mem)
	MCFG_CPU_IO_MAP(ccs300_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs300 )
	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "ccs300.rom", 0x0000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT   COMPAT   MACHINE    INPUT    CLASS         INIT     COMPANY                          FULLNAME       FLAGS */
COMP( 19??, ccs300, ccs2810, 0,       ccs300,    ccs300,  driver_device,  0,   "California Computer Systems", "CCS Model 300", GAME_IS_SKELETON )
