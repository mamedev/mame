/***************************************************************************

        CCS Model 2422B

        11/12/2009 Skeleton driver.

It requires a floppy disk to boot from.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define MACHINE_RESET_MEMBER(name) void name::machine_reset()

class ccs2422_state : public driver_device
{
public:
	ccs2422_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	,
		m_ccs_ram(*this, "ccs_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER( ccs2422_10_r );
	DECLARE_READ8_MEMBER( ccs2422_11_r );
	DECLARE_WRITE8_MEMBER( ccs2422_10_w );
	DECLARE_WRITE8_MEMBER( kbd_put );
	required_shared_ptr<UINT8> m_ccs_ram;
	UINT8 m_term_data;
	virtual void machine_reset();
};


READ8_MEMBER( ccs2422_state::ccs2422_10_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( ccs2422_state::ccs2422_11_r )
{
	return (m_term_data) ? 5 : 4;
}

WRITE8_MEMBER( ccs2422_state::ccs2422_10_w )
{
	m_terminal->write(space, 0, data & 0x7f);
}

static ADDRESS_MAP_START(ccs2422_mem, AS_PROGRAM, 8, ccs2422_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("ccs_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( ccs2422_io, AS_IO, 8, ccs2422_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READWRITE(ccs2422_10_r,ccs2422_10_w)
	AM_RANGE(0x11, 0x11) AM_READ(ccs2422_11_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( ccs2422 )
INPUT_PORTS_END


MACHINE_RESET_MEMBER(ccs2422_state)
{
	UINT8* user1 = memregion("user1")->base();
	memcpy((UINT8*)m_ccs_ram, user1, 0x0800);

	// this should be rom/ram banking
}

WRITE8_MEMBER( ccs2422_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(ccs2422_state, kbd_put)
};

static MACHINE_CONFIG_START( ccs2422, ccs2422_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(ccs2422_mem)
	MCFG_CPU_IO_MAP(ccs2422_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( ccs2422 )
	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD( "2422b.u24", 0x0000, 0x0800, CRC(6cf22e31) SHA1(9aa3327cd8c23d0eab82cb6519891aff13ebe1d0))
	ROM_LOAD( "2422.u23",  0x0900, 0x0100, CRC(b279cada) SHA1(6cc6e00ec49ba2245c8836d6f09266b09d6e7648))
	ROM_LOAD( "2422.u22",  0x0a00, 0x0100, CRC(e41858bb) SHA1(0be53725032ebea16e32cb720f099551a357e761))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY                          FULLNAME       FLAGS */
COMP( 19??, ccs2422,  0,       0,    ccs2422,   ccs2422, driver_device,  0,   "California Computer Systems", "CCS Model 2422B", GAME_NOT_WORKING | GAME_NO_SOUND)
