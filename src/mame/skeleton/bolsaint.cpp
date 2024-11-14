// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************

80188-based slot machines from Sleic/Petaco.

For "Bolsa Internacional", a complete manual with schematics can be downloaded from
  https://www.recreativas.org/manuales

Hardware info for "Bolsa Internacional":
  CPU PCB
    2 x OKI M82C55A-2
    Dallas DS1644-120 timekeeper RAM
    1 x OKI M82C51A-2
    Xtal 20.000 MHz
    AMD N80C188-20
  Sound PCB
    OKI M6376
    Xtal 5.0000 MHz

****************************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/i8255.h"
#include "sound/okim6376.h"
#include "speaker.h"

namespace {

class bolsaint_state : public driver_device
{
public:
	bolsaint_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_okim6376(*this, "oki")
	{
	}

	void bolsaint(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i80188_cpu_device> m_maincpu;
	required_device<okim6376_device> m_okim6376;
};

void bolsaint_state::machine_start()
{
}

static INPUT_PORTS_START(bolsaint)
INPUT_PORTS_END

void bolsaint_state::bolsaint(machine_config &config)
{
	I80188(config, m_maincpu, 20.0000_MHz_XTAL);

	I8255A(config, "ppi1"); // OKI M82C55A-2, on CPU PCB
	I8255A(config, "ppi2"); // OKI M82C55A-2, on CPU PCB

	// Sound hardware

	SPEAKER(config, "mono").front_center();

	// There is a jumper on the sound PCB for selecting between 16KHz and 32KHz,
	// but is fixed (soldered) for 32KHz
	OKIM6376(config, m_okim6376, 5.0000_MHz_XTAL/8/2).add_route(ALL_OUTPUTS, "mono", 1.0); // Guess
}

// Bolsa Internacional (Euro).
// Only two units were ever made for the euro-adapted version (there's a previous version which only supports pesetas).
ROM_START(bolsaint)
	ROM_REGION(0x080000, "maincpu", 0)
	// The machine serial number is hardcoded on the program ROM
	ROM_LOAD("sleic_bolsa_internacional_bi001-v3.22.ic6", 0x00000, 0x80000, CRC(3cfb3da0) SHA1(d003ec8edf1d85f03502bad33a286ec4fccb6ce2))

	ROM_REGION(0x100000, "oki", 0)
	// Four sockets, only two populated
	ROM_LOAD("bolsa_internacional_bisound01-1.ic4",       0x00000, 0x80000, CRC(4210a665) SHA1(2a8af1be8d8adfc1c630b8dd2babaa9e0f06f696))
	ROM_LOAD("bolsa_internacional_bisound02-2.ic5",       0x80000, 0x80000, CRC(910fa2d3) SHA1(0d48e22ef01865947d4716926406f40c126d639a))

	ROM_REGION(0x117, "plds", 0)
	// On the "roulette" PCB ("SLEIC-PETACO 011-114")
	ROM_LOAD("palce16v8h.bin", 0x000, 0x117, NO_DUMP)
ROM_END

} // anonymous namespace

//   YEAR  NAME       PARENT MACHINE   INPUT     CLASS            INIT        ROT   COMPANY         FULLNAME                      FLAGS
GAME(2000, bolsaint,  0,     bolsaint, bolsaint, bolsaint_state,  empty_init, ROT0, "Sleic/Petaco", "Bolsa Internacional (euro)", MACHINE_IS_SKELETON_MECHANICAL) // VER.1.0 found on ROM string, but EPROM label reads V3.22
