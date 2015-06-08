// license:GPL-2.0+
// copyright-holders: Felipe Sanches
/***************************************************************************

    Itautec I7000

    driver by Felipe C. da S. Sanches <juca@members.fsf.org>
    with tech info provided by Alexandre Souza (a.k.a. Tabajara).

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h" //CPU was actually a NSC800 (Z80 compatible)


class i7000_state : public driver_device
{
public:
	i7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)	{ }

//	DECLARE_READ8_MEMBER( i7000_io_r );
//	DECLARE_WRITE8_MEMBER( i7000_io_w );

	DECLARE_DRIVER_INIT(i7000);
};

DRIVER_INIT_MEMBER(i7000_state, i7000)
{
}

static ADDRESS_MAP_START(i7000_mem, AS_PROGRAM, 8, i7000_state)
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x1000, 0xffff) AM_RAM
ADDRESS_MAP_END

/*
static ADDRESS_MAP_START( i7000_io , AS_IO, 8, i7000_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK (0xff)
	AM_RANGE(0x00, 0xff) AM_READWRITE(i7000_io_r, i7000_io_w)
ADDRESS_MAP_END
*/

static MACHINE_CONFIG_START( i7000, i7000_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", NSC800, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(i7000_mem)
//	MCFG_CPU_IO_MAP(i7000_io)

MACHINE_CONFIG_END

ROM_START( i7000 )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "i7000_boot_v1_4r02_15_10_85_d52d.rom",  0x0000, 0x1000, CRC(622412e5) SHA1(bf187a095600fd46a739c35132a85b5f39b2f867) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "i7000_chargen.rom", 0x0000, 0x0800, CRC(7ba75183) SHA1(4af799f4a8bd385e1e4e5ece378df93e1133dc12) )

	ROM_REGION( 0x1000, "drive", 0 )
	ROM_LOAD( "i7000_drive_ci01.rom", 0x0000, 0x1000, CRC(d8d6e5c1) SHA1(93e7db42fbfaa8243973321c7fc8c51ed80780be) )

	ROM_REGION( 0x1000, "telex", 0 )
	ROM_LOAD( "i7000_telex_ci09.rom", 0x0000, 0x1000, CRC(c1c8fcc8) SHA1(cbf5fb600e587b998f190a9e3fb398a51d8a5e87) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT                COMPANY    FULLNAME    FLAGS */
COMP( 1982, i7000,  0,      0,       i7000,     0,       i7000_state, i7000, "Itautec", "I-7000",   GAME_NOT_WORKING | GAME_NO_SOUND)
