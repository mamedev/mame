/***************************************************************************

    Skeleton driver for Husky Hunter 2

    TODO:
    - Everything - this is just a skeleton


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"


class hunter2_state : public driver_device
{
public:
	hunter2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(hunter2_mem, AS_PROGRAM, 8, hunter2_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x8000, 0xbfff) AM_RAM
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hunter2_io, AS_IO, 8, hunter2_state)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( hunter2 )
INPUT_PORTS_END


void hunter2_state::machine_reset()
{
}

static MACHINE_CONFIG_START( hunter2, hunter2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, 4000000)
	MCFG_CPU_PROGRAM_MAP(hunter2_mem)
	MCFG_CPU_IO_MAP(hunter2_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( hunter2 )
	ROM_REGION(0x10000, "roms", 0)
	ROM_LOAD( "tr032kx8mrom0.ic50", 0x0000, 0x8000, CRC(694d252c) SHA1(b11dbf24faf648596d92b1823e25a8e4fb7f542c) )
	ROM_LOAD( "tr032kx8mrom1.ic51", 0x8000, 0x8000, CRC(82901642) SHA1(d84f2bbd2e9e052bd161a313c240a67918f774ad) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE         INIT  COMPANY    FULLNAME       FLAGS */
COMP( 1981, hunter2, 0,      0,       hunter2,   hunter2, driver_device,  0,  "Husky", "Hunter 2", GAME_IS_SKELETON )
