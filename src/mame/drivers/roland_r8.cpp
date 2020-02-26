// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland R-8 drum machine.

****************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k2.h"

class roland_r8_state : public driver_device
{
public:
	roland_r8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void r8(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<upd78210_device> m_maincpu;
};


void roland_r8_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START(r8)
INPUT_PORTS_END


void roland_r8_state::r8(machine_config &config)
{
	UPD78210(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_r8_state::mem_map);
}


ROM_START(r8m)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("rolandr8mv104.bin", 0x00000, 0x20000, CRC(5e95e2f6) SHA1(b4e1a8f15f72a9db9aa8fd41ee3c3ebd10460587))
ROM_END


SYST(1990, r8m, 0, 0, r8, r8, roland_r8_state, empty_init, "Roland", "R-8M Total Percussion Sound Module (v1.04)", MACHINE_IS_SKELETON)
