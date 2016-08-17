// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    T400 uController test suite for COP410/420 series CPUs

    http://opencores.org/project,t400

*/

#include "emu.h"
#include "cpu/cop400/cop400.h"

class t400_test_suite_state : public driver_device
{
public:
	t400_test_suite_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	DECLARE_WRITE8_MEMBER( port_l_w );
	required_device<cpu_device> m_maincpu;
};

WRITE8_MEMBER( t400_test_suite_state::port_l_w )
{
//  printf("L: %u\n", data);
}

static MACHINE_CONFIG_START( test_t410, t400_test_suite_state )
	MCFG_CPU_ADD("maincpu", COP410, 1000000)
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false )
	MCFG_COP400_WRITE_L_CB(WRITE8(t400_test_suite_state, port_l_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( test_t420, t400_test_suite_state )
	MCFG_CPU_ADD("maincpu", COP420, 1000000)
	MCFG_COP400_CONFIG( COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true )
	MCFG_COP400_WRITE_L_CB(WRITE8(t400_test_suite_state, port_l_w))
MACHINE_CONFIG_END

ROM_START( test410 )
	ROM_REGION( 0x200, "maincpu", 0 )
	ROM_LOAD( "rom_41x.bin", 0x000, 0x200, NO_DUMP )
ROM_END

ROM_START( test420 )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "rom_42x.bin", 0x000, 0x400, BAD_DUMP CRC(e4e80001) SHA1(8fdca9d08de1cc83387a7d141f6b254117902442) )
ROM_END

COMP( 2008, test410,   0,        0,      test_t410,   0, driver_device, 0,      "T400 uController project",   "T410 test suite", MACHINE_NO_SOUND_HW )
COMP( 2008, test420,   test410,  0,      test_t420,   0, driver_device, 0,      "T400 uController project",   "T420 test suite", MACHINE_NO_SOUND_HW )
