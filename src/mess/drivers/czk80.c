/***************************************************************************

        CZK-80

        30/08/2010 Skeleton driver

        On main board there are Z80A CPU, Z80A PIO, Z80A DART and Z80A CTC
            there is 8K ROM and XTAL 16MHz
        FDC board contains Z80A DMA and NEC 765A (XTAL on it is 8MHZ)
        Mega board contains 74LS612 and memory chips

    27/11/2010 Connected to a terminal

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define MACHINE_RESET_MEMBER(name) void name::machine_reset()

class czk80_state : public driver_device
{
public:
	czk80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	,
		m_p_ram(*this, "p_ram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER( czk80_80_r );
	DECLARE_READ8_MEMBER( czk80_81_r );
	DECLARE_READ8_MEMBER( czk80_c0_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	required_shared_ptr<UINT8> m_p_ram;
	UINT8 m_term_data;
	virtual void machine_reset();
};


READ8_MEMBER( czk80_state::czk80_80_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( czk80_state::czk80_c0_r )
{
	return 0x80;
}

READ8_MEMBER( czk80_state::czk80_81_r )
{
	return (m_term_data) ? 3 : 1;
}

static ADDRESS_MAP_START(czk80_mem, AS_PROGRAM, 8, czk80_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_SHARE("p_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START(czk80_io, AS_IO, 8, czk80_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x80, 0x80) AM_READ(czk80_80_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0x81, 0x81) AM_READ(czk80_81_r)
	AM_RANGE(0xc0, 0xc0) AM_READ(czk80_c0_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( czk80 )
INPUT_PORTS_END

MACHINE_RESET_MEMBER(czk80_state)
{
	UINT8* bios = memregion("maincpu")->base() + 0xe000;
	memcpy(m_p_ram, bios, 0x2000);
	memcpy(m_p_ram+0xe000, bios, 0x2000);
}


WRITE8_MEMBER( czk80_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(czk80_state, kbd_put)
};

static MACHINE_CONFIG_START( czk80, czk80_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(czk80_mem)
	MCFG_CPU_IO_MAP(czk80_io)

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( czk80 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "czk80.rom", 0xe000, 0x2000, CRC(7081b7c6) SHA1(13f75b14ea73b252bdfa2384e6eead6e720e49e3))
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT  INIT        COMPANY      FULLNAME       FLAGS */
COMP( 198?, czk80,  0,       0,     czk80,   czk80, driver_device, 0,        "<unknown>",  "CZK-80", GAME_NOT_WORKING | GAME_NO_SOUND_HW)
