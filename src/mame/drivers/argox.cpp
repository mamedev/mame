// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

  Argox Rabbit Printer
  model: 0S-214

  Skeleton driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

***************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"

class os214_state : public driver_device
{
public:
	os214_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

        DECLARE_DRIVER_INIT(os214);
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( os214_prg_map, AS_PROGRAM, 16, os214_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( os214_io_map, AS_IO, 8, os214_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( os214, os214_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H83002, XTAL_16MHz) /* X1 xtal value is correct,
                                                       but there can be some clock divider perhaps ? */
	MCFG_CPU_PROGRAM_MAP(os214_prg_map)
	MCFG_CPU_IO_MAP(os214_io_map)
MACHINE_CONFIG_END

DRIVER_INIT_MEMBER( os214_state, os214 )
{
}

ROM_START( os214 )
    ROM_REGION( 0x080000, "maincpu", 0 )
    ROM_LOAD16_BYTE( "u9_s2a2-4.03_argox_am.u9", 0x000000, 0x040000, CRC(3bd8b2b1) SHA1(546f9fd8d7e1f589f6e594a332a3429041b49eea) )
    ROM_LOAD16_BYTE( "u8_s2a2-4.03_argox_am.u8", 0x000001, 0x040000, CRC(d49f52af) SHA1(0ca5a70c6c3995f275226af26db965f6ba7ed123) )
ROM_END

/*    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT  COMPANY  FULLNAME                         FLAGS   */
COMP( 1996, os214,      0,      0,   os214,     0, os214_state, os214, "Argox", "Rabbit Printer (model OS-214)", MACHINE_IS_SKELETON)
