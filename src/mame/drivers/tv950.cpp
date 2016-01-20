// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    2013-09-10 Skeleton driver for Televideo TV950

    TODO:
    - Everything - this is just a skeleton

    Seems there should be a few more roms.


****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"


class tv950_state : public driver_device
{
public:
	tv950_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START(tv950_mem, AS_PROGRAM, 8, tv950_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(tv950_io, AS_IO, 8, tv950_state)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tv950 )
INPUT_PORTS_END


void tv950_state::machine_reset()
{
}

static MACHINE_CONFIG_START( tv950, tv950_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(tv950_mem)
	MCFG_CPU_IO_MAP(tv950_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tv950 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "8000043", "8000043")
	ROMX_LOAD( "tv-043h.rom",  0xf000, 0x1000, CRC(89b826be) SHA1(fd5575be04317682d0c9062702b5932b46f89926), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "8000044", "8000044")
	ROMX_LOAD( "tv-044h.rom",  0xf000, 0x1000, CRC(24b0383d) SHA1(71cabb7f3da8652a36afdf3d505ab8a41651e801), ROM_BIOS(2) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 198?, tv950,  0,      0,       tv950,     tv950, driver_device,  0,  "Televideo", "TV950", MACHINE_IS_SKELETON )
