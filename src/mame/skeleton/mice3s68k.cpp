// license:BSD-3-Clause
// copyright-holders:

/**********************************************************************************************

Skeleton driver for Microtek MICE IIIS 68000 in-circuit emulator.

Three hardware components.

1.- Main unit with three PCBs:

CPM PCB (CPU):
 - 4 x EPROM.
 - 1 x PROM.
 - 1 x Intel R80186 near a 16 MHz crystal.
 - 2 x Mitsubishi M5M5256BP-10L (RAM).
 - 1 x Zilog Z0853006PSC near a 3.3864 MHz crystal.
 - Small suboard with one Serial RS-232C (DCE) port (DB25) and one Parallel port (DB25).
 - 1 x Reset switch.

EPM-6800A PCB:
 - 4 x NEC D8255AC-2.
 - 2 x GAL16V8.
 - 1 x Cypress CY7C166-25PC (RAM).
 - 2 x Sony CXK5814P-35 (RAM).
 - 2 x BNC connector (start slave out, sync start in).
 - 1 LED (EP run).

HEMM PCB:
 - 4 x NEC D8255AC-2.
 - 33 x Mitsubishi M5M5256BP-70L (RAM).

LAM-IIS PCB:
 - 8 x NEC 71055L.
 - 2 x AMC MACH110-12JC.
 - 1 x 40 MHz crystal.
 - 11 x GAL16V8.
 - 8 x P4C198-15JC (RAM).
 - 3 x BNC connector (sync 1 output, sync 2 output, ext trigger input).
 - 4 x Cypress CY7C196-25VC (RAM).
 - 16 x IDT 71256 (RAM).
 - 1 x Trace Bits port (DB9).
 - 1 LED (+5V power).

2.- EPOD-68000, external box for connecting the main unit to the CPU pass-through apapter,
with connectors, one GAL16V8 and some logic.

3.- CPU pass-through apapter.

**********************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"


namespace {

class mice3s68k_state : public driver_device
{
public:
	mice3s68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void mice3s68k(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(mice3s68k)
INPUT_PORTS_END

void mice3s68k_state::mice3s68k(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL); // Intel R80186
}


// FW V5.2
ROM_START(mice3s68k)
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "iiis_68000_v5.2.u11",        0x00000, 0x08000, CRC(cd94dd5d) SHA1(3a609f2dca9bc22a764188704eaa8ca952d1637c) )
	ROM_LOAD( "iiis_68000_v5.2.u12",        0x00800, 0x08000, CRC(fb9c9185) SHA1(6463274989f5e25cba08dbcac485eda583c7c602) )
	ROM_LOAD( "iiis_68000_v5.2_27c512.u14", 0x01000, 0x10000, CRC(75f8841f) SHA1(3ac20ff642f36145f0310fe96791bae74305e7ec) )
	ROM_LOAD( "iiis_68000_v5.2_27c512.u15", 0x02000, 0x10000, CRC(d2dbf91f) SHA1(90ce54ba3950944bb077e95e96b1388370baec32) )

	ROM_REGION(0x00100, "prom", 0)
	ROM_LOAD( "x2212p.u5",                  0x00000, 0x00100, CRC(c4089451) SHA1(4997cd46ddeab498ced5243e84d4f7fbe5c4597e) )

	ROM_REGION(0x00117, "pld", 0)
	ROM_LOAD( "54983012_gal16v8.u24",       0x00000, 0x00117, NO_DUMP ) // EPOD-68000
	ROM_LOAD( "54982021_gal16v8.u58",       0x00000, 0x00117, NO_DUMP ) // EPM-68000A
	ROM_LOAD( "54982011_gal16v8.u46",       0x00000, 0x00117, NO_DUMP ) // EPM-68000A
	ROM_LOAD( "08309011_gal16v8.u3",        0x00000, 0x00117, NO_DUMP ) // HEMM
	ROM_LOAD( "55005121_mach110.u106",      0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005131_mach110.u128",      0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005062_gal16v8.u55",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005071_gal16v8.u56",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005081_gal16v8.u58",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005091_gal16v8.u59",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005031_gal16v8.u30",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005041_gal16v8.u31",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005051_gal16v8.u44",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005011_gal16v8.u9",        0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005021_gal16v8.u10",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005101_gal16v8.u78",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
	ROM_LOAD( "55005111_gal16v8.u32",       0x00000, 0x00117, NO_DUMP ) // LAM-IIS
ROM_END

} // anonymous namespace

SYST( 1992, mice3s68k, 0, 0, mice3s68k, mice3s68k, mice3s68k_state, empty_init, "Microtek", "MICE-IIIS 68000", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
