// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for SuperStars CD jukebox by Sound Leisure Ltd.

****************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "machine/timekpr.h"


namespace {

class slsstars_state : public driver_device
{
public:
	slsstars_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void slsstars(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<mc68hc11_cpu_device> m_maincpu;
};


void slsstars_state::mem_map(address_map &map)
{
	map(0x10f8, 0x10ff).nopw(); // some kind of serial output?
	map(0x1800, 0x1800).nopw(); // watchdog?
	map(0x4000, 0x5fff).rw("timekpr", FUNC(m48t58_device::read), FUNC(m48t58_device::write));
	map(0x8000, 0xffff).rom().region("program", 0);
}


static INPUT_PORTS_START(slsstars)
	// all unknown
INPUT_PORTS_END


void slsstars_state::slsstars(machine_config &config)
{
	MC68HC11F1(config, m_maincpu, 8'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &slsstars_state::mem_map);
	m_maincpu->set_default_config(0x3f);

	M48T58(config, "timekpr"); // U3
}


ROM_START(slsstars)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("superstars-eprom.u2", 0x0000, 0x8000, CRC(59d26dda) SHA1(4ce5fa5b7f317bdada889f66dbf5a106b1907b30))
ROM_END

} // anonymous namespace


SYST(199?, slsstars, 0, 0, slsstars, slsstars, slsstars_state, empty_init, "Sound Leisure", "SuperStars (CD jukebox)", MACHINE_IS_SKELETON_MECHANICAL)
