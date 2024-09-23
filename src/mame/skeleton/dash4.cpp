// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************
Skeleton driver for Cross Products Ltd. DASH4 Debug Adapter.

DASH4 is the hardware debug adapter of CodeScape development environment for the Hitachi SuperH family of
microprocessors.
It's an intelligent debug pod that offers a complete low-level debug capability. The DASH4 uses the JTAG
interface to communicate with the target at 1.5 Mbytes/s via the Hitachi User Debug Interface (HUDI)
built-in to the SH7750. The DASH4 connects to the host computer via Ethernet to allow high-speed
communications with the debugging software.

The CodeScape Development system encompasses both software tools and hardware interfaces to provide
development support for Hitachi SH microprocessors from initial low-level system troubleshooting through
to the development of complex applications in C++.

It came bundled with the Altera Design Software for PCs "Quartus II":
https://archive.org/details/altera-quartus-ii-version-2.2-manuals-software/

Also, it came with a MemoHASP-1 parallel port security dongle labeled:
"MemoHASP-1 R3b CCYRW NA 23967", "HASP ID= 75BB4C39", "SH4".

DASH4 Rev. B PCB (1999):
      ____________________________________________________________________
     |                                  _____________       ______       |
     |     ____________         Xtal    |NEC         |     |LT1129       |___
     |    |Altera     |      11.059MHz  |D4564323G5__|     |_____|          _|__
    _|_   |Flex       |  __________              __________  __________     |   | <- Power
    | | EPF10K10TC144-3 |_CY2292F_|  __________  S32XR861Q1  S32XR861Q1     |___|
    | |   |           |              S32XR861Q1                              |
    | |   |___________|            _________         _________     ___       |
I/O | |  _______   ____________   |74HCT244|        |_HCT374A|    93LC56B  O <- Yellow LED
    | | |_P92AB|  |SH-2       |   ________                    _________    O <- Green LED
    | |  _______  |HD6417604F28  |AS29F040   ________        Crystal LAN    _|___
    |_| |_P92AB|  |           |  |       |  |GAL22V10D       |CT8900A-IQ    |   |
     |   _______  |           |  |_______|  |       |        |         |    |   | <- LAN RJ45
     |  |74HCT138 |___________|             |_______|        |_________|    |___|
     |_____            __                                   __            ___|
           |___________| |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_| |___________|
                            Unused edge connector

***********************************************************************************************************/

#include "emu.h"
#include "cpu/sh/sh7604.h"


namespace {

class dash4_state : public driver_device
{
public:
	dash4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void dash4(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(dash4)
INPUT_PORTS_END

void dash4_state::dash4(machine_config &config)
{
	SH7604(config, m_maincpu, 11'059'000); // Actually an Hitachi SH-2 HD6417604F28
}


ROM_START(dash4)
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD( "as29f040.u2",       0x00000, 0x80000, CRC(507c2a4c) SHA1(7eea8acf196a6c249fda5fe41b197070d426cffc) )

	ROM_REGION(0x00100, "eeprom", 0)
	ROM_LOAD( "93lc56b.u17",       0x00000, 0x00100, CRC(a05ee68a) SHA1(5dd120cb4a818826ebec8bde59225ccdbf28a57d) )

	ROM_REGION(0x002e5, "pld", 0)
	ROM_LOAD( "gal22v10d-15lj.u6", 0x00000, 0x002e5, CRC(80cc0146) SHA1(e28da16e18d0c9de48a08651f1c670f946d188e7) )
ROM_END

} // anonymous namespace

SYST( 1999, dash4, 0, 0, dash4, dash4, dash4_state, empty_init, "Cross Products", "DASH4 Debug Adapter", MACHINE_IS_SKELETON )
