// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Skeleton driver for ITT Courier 9216 IBM-compatible color display terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/mcs48/mcs48.h"

class itt9216_state : public driver_device
{
public:
	itt9216_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_chargen(*this, "chargen")
	{
	}

	void itt9216(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<u8> m_chargen;
};

void itt9216_state::mem_map(address_map &map)
{
	map(0x00000, 0x00007).rom().region("program", 0);
	map(0x00008, 0x0ffff).ram();
	map(0x58000, 0x59fff).rom().region("program", 0);
	map(0x62000, 0x62fff).ram();
}

static INPUT_PORTS_START(itt9216)
INPUT_PORTS_END

void itt9216_state::itt9216(machine_config &config)
{
	M68008(config, m_maincpu, 8000000); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &itt9216_state::mem_map);

	I8741(config, "upi", 6000000).set_disable(); // clock unknown
}

// XTALs: 38.080 MHz (U7), 11.250(?) MHz (U1)
// ICs on main board: MC68008P8 (U33), 701188-001 PLCC ASIC (U58), empty PLCC (U45), beeper
// ICs on keyboard simulator board: 700881-001 PLCC ASIC (U12), DP8340N (U16), DP8341N (U15), D8741A (U1)
ROM_START(itt9216)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("174054-007.u52", 0x0000, 0x2000, CRC(be1f85c8) SHA1(8c44ff6166c43b524f41133053fa82f5c48047d8))

	ROM_REGION(0x400, "upi", 0)
	ROM_LOAD("174065-003.u1", 0x000, 0x400, NO_DUMP)

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("174055-004.u40", 0x0000, 0x2000, CRC(c8611425) SHA1(31fbdd6ff72a96c59277b6edac9a6360f6e1e49e))
ROM_END

COMP(1986, itt9216, 0, 0, itt9216, itt9216, itt9216_state, empty_init, "ITT Courier", "ITT 9216-X", MACHINE_IS_SKELETON)
