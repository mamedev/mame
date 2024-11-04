// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland RA-30 Realtime Arranger.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"


namespace {

class roland_ra30_state : public driver_device
{
public:
	roland_ra30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ra30(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8510_device> m_maincpu;
};

void roland_ra30_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).mirror(0x100000).rom().region("progrom", 0);
}


static INPUT_PORTS_START(ra30)
INPUT_PORTS_END

void roland_ra30_state::ra30(machine_config &config)
{
	HD6415108(config, m_maincpu, 20'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_ra30_state::mem_map);
}

ROM_START(ra30)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_LOAD("roland_r00892534_hn62444bp_e85.ic22", 0x00000, 0x80000, CRC(c8b75d6f) SHA1(68472f2fa6ee2cab77b153c0131dcf2d76e0dce2))

	ROM_REGION(0x80000, "stylerom", 0)
	ROM_LOAD("roland_r00679623_hn62444bp_e76.ic18", 0x00000, 0x80000, CRC(3c1670a6) SHA1(d0ef77143f64a20c6a151746eb0bfad15a98b2e5))
ROM_END

} // anonymous namespace


SYST(1995, ra30, 0, 0, ra30, ra30, roland_ra30_state, empty_init, "Roland", "RA-30 Realtime Arranger", MACHINE_IS_SKELETON)
