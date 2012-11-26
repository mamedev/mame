/***************************************************************************

        NorthStar Horizon

        07/12/2009 Skeleton driver.

        It appears these machines say nothing until a floppy disk is
        succesfully loaded. The memory range EA00-EB40 appears to be
        used by devices, particularly the FDC.

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"


class horizon_state : public driver_device
{
public:
	horizon_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_WRITE8_MEMBER( kbd_put );
	//UINT8 m_term_data;
	virtual void machine_reset();
	DECLARE_MACHINE_RESET(horizon_sd);
};


static ADDRESS_MAP_START(horizon_mem, AS_PROGRAM, 8, horizon_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xe7ff) AM_RAM
	AM_RANGE(0xe800, 0xe8ff) AM_ROM
	AM_RANGE(0xec00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(horizon_io, AS_IO, 8, horizon_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(horizon_sd_mem, AS_PROGRAM, 8, horizon_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xe8ff) AM_RAM
	AM_RANGE(0xe900, 0xe9ff) AM_ROM
	AM_RANGE(0xec00, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(horizon_sd_io, AS_IO, 8, horizon_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( horizon )
INPUT_PORTS_END


void horizon_state::machine_reset()
{
	machine().device("maincpu")->state().set_state_int(Z80_PC, 0xe800);
}

MACHINE_RESET_MEMBER(horizon_state,horizon_sd)
{
	machine().device("maincpu")->state().set_state_int(Z80_PC, 0xe900);
}

WRITE8_MEMBER( horizon_state::kbd_put )
{
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(horizon_state, kbd_put)
};

static MACHINE_CONFIG_START( horizon, horizon_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(horizon_mem)
	MCFG_CPU_IO_MAP(horizon_io)


	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( horizsd, horizon )
	MCFG_CPU_MODIFY( "maincpu" )
	MCFG_CPU_PROGRAM_MAP( horizon_sd_mem)
	MCFG_CPU_IO_MAP(horizon_sd_io)

	MCFG_MACHINE_RESET_OVERRIDE(horizon_state,horizon_sd)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( horizdd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "horizon.bin", 0xe800, 0x0100, CRC(7aafa134) SHA1(bf1552c4818f30473798af4f54e65e1957e0db48))
ROM_END

ROM_START( horizsd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "horizon-sd.bin", 0xe900, 0x0100, CRC(754e53e5) SHA1(875e42942d639b972252b87d86c3dc2133304967))
ROM_END

ROM_START( vector1 ) // This one have different I/O
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "horizon.bin", 0xe800, 0x0100, CRC(7aafa134) SHA1(bf1552c4818f30473798af4f54e65e1957e0db48))
ROM_END
/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1979, horizdd,  0,       0,    horizon,   horizon, driver_device, 0,  "NorthStar", "Horizon (DD drive)", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1979, horizsd,  horizdd, 0,    horizsd,   horizon, driver_device, 0,  "NorthStar", "Horizon (SD drive)", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1979, vector1,  horizdd, 0,    horizon,   horizon, driver_device, 0,  "Vector Graphic", "Vector 1+ (DD drive)", GAME_NOT_WORKING | GAME_NO_SOUND)
