// license:BSD-3-Clause
// copyright-holders:

/********************************************************************

 Skeleton driver for Gaelco programming devices for M68705-based "Loto-Play" PCBs.
 The M68705 ROM is embedded on the Z80 ROM.

********************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"

#include "sound/ay8910.h"

namespace
{

class lotoplay_prog_state : public driver_device
{
public:
	lotoplay_prog_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay(*this, "ay%u", 1U)
	{
	}

	void lotoplay_prog(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_ay;
};

INPUT_PORTS_START(lotoplay_prog)
INPUT_PORTS_END

void lotoplay_prog_state::lotoplay_prog(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // unknown clock

	AY8910(config, m_ay[0], 4'000'000 / 2); // unknown clock
	AY8910(config, m_ay[1], 4'000'000 / 2); // unknown clock
}


// Devices for programming MC68705-based Loto-Play boards. The MC68705 ROM is at 0x1000 on each Z80 ROM

ROM_START(lotoplayz)
	ROM_REGION(0x2000, "maincpu", 0)
	// holds the 'lotoplay' image: CRC(112645cd) SHA1(f2ad6b2fbec36d0bfe034d7bfb036ef6bf4ee395)
	ROM_LOAD("grab._lp_sp_ultima_27c64.bin", 0x0000, 0x2000, CRC(980e14ac) SHA1(3f6dc75a8cb3fe38941b8a7900ecccdafabc14e9))
ROM_END

ROM_START(lotoplayza)
	ROM_REGION(0x2000, "maincpu", 0)
	// holds the 'lotoplaya' image: CRC(9b77603c) SHA1(6799b930f9805332bf20c6146b044222fe49d243)
	ROM_LOAD("grab._lp_s_3b4c_27c64.bin",    0x0000, 0x2000, CRC(556e2c35) SHA1(612f160592fd122e5a91914618e19eade5b52c3e))
ROM_END

ROM_START(lotoplayzb)
	ROM_REGION(0x2000, "maincpu", 0)
	// holds the 'lotoplayc' image: CRC(20a0e0d0) SHA1(832ed64dfa5f5f150f0e9918b40e9fb4e8e4260d)
	ROM_LOAD("multn_0.0_27c64.bin",          0x0000, 0x2000, CRC(e74bce2a) SHA1(66a09f5df3a27b0c4bc964b19450734b736dc768))
ROM_END


} // anonymous namespace

//    YEAR   NAME        PARENT     MACHINE        INPUT          CLASS                INIT        ROT   COMPANY              FULLNAME                                FLAGS
GAME( 1990?, lotoplayz,  0,         lotoplay_prog, lotoplay_prog, lotoplay_prog_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play programming device (set 1)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplayza, lotoplayz, lotoplay_prog, lotoplay_prog, lotoplay_prog_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play programming device (set 2)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
GAME( 1990?, lotoplayzb, lotoplayz, lotoplay_prog, lotoplay_prog, lotoplay_prog_state, empty_init, ROT0, "Gaelco / Covielsa", "Loto-Play programming device (set 3)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
