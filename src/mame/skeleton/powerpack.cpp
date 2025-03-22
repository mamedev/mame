// license:BSD-3-Clause
// copyright-holders:
/****************************************************************************

Skeleton driver for Microtek Powerpack In-Circuit Emulator model PP-SW+ PIII

Main POD with two PCBs connected with a small backplane (also connects
with the pass-through CPU adapter using a flat cable):
 - PCB 1:
  * Side A
   · 2 x Xilinx XC3190A.
   · 5 x OKI M5416283-60.
   · 1 x Intel KU80386EX33 i386 CPU 33MHz.
   · 1 x AM29F040B.
   · 1 x Xilinx XC7336.
   · 1 x GAL16V8D.
   · 1 x 93C46 SEEPROM.
   · 1 x MAX786CAI.
  * Side B
   · 2 x Xilinx XC3190A
   · 5 x OKI M5416283-60.
   · 1 x PC-Card slot (with a D-Link JITI DE660 Ethernet PC Card).
   · 1 x RS-232 port.
   · 1 x Reset button.
 - PCB 2:
  * 1 x Xilinx XCV100.
  * 1 x 16 MHz crystal.
  * 4 x LEDs (Power, Selftest, Emulating, Bus Active).

Pass-through CPU adapter for Pentium III:
 - 1 x Xilinx XCV100.

****************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"


namespace {

class powerpack_state : public driver_device
{
public:
	powerpack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void powerpack(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(powerpack)
INPUT_PORTS_END

void powerpack_state::powerpack(machine_config &config)
{
	I386(config, m_maincpu, 33'000'000); // Intel KU80386EX33
}


ROM_START(powerpack)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "fls-am29f040b.u51",         0x00000, 0x80000, CRC(4bc45cf8) SHA1(429044f0910c92e2afde49639d7caa4c798597e5) )

	ROM_REGION(0x00080, "eeprom", 0)
	ROM_LOAD( "93c46.u52",                 0x00000, 0x00080, CRC(cc9821ae) SHA1(88719489f653fb11b059ec59e7c2dcad27539c3d) )

	ROM_REGION(0x04000, "pld", 0)
	ROM_LOAD( "drm_1.5_xilinx_xc7336.u59", 0x00000, 0x04000, CRC(c6bcbf58) SHA1(d6a2922c3af3bd50bd54c3b2b80e34c96aba8e7a) )
	ROM_LOAD( "vck_1.0-gal16v8d.u27",      0x00000, 0x00117, CRC(dac50f29) SHA1(78e607a580b8c3563b5e3868c5fc77a451cfafb7) )
ROM_END

} // anonymous namespace

SYST( 1999, powerpack, 0, 0, powerpack, powerpack, powerpack_state, empty_init, "Microtek", "Powerpack In-Circuit Emulator PP-SW+ PIII", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
