// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Sony BVM-{14|20}{E|F}1{E|U} series monitors.

    List of major ICs:
    - Hitachi HD6435368CP10 H8/536 [IC1]
      or: Hitachi HD6475368CP10 C764
      or: Hitachi HD6435368CP10 W51
      near 20 MHz XTAL
    - CSI CAT28F020P-15 2 Megabit CMOS Flash Memory [IC3]
    - Sony CXK58257AP-10 [IC4]
      or: Sanyo LC35256D-10
      next to: Sony CR2025 3V Lithium Cell [BAT1]
    - Sony CXD1095Q [IC5, IC6]
    - NEC D6453GT-101 On-Screen Character Display [IC7]
    - Sony CXA1727Q [IC102]
    - Sony CXD2343S [IC105]
    - Hitachi HN27C256AG-10 [IC107, IC108]
    - Sony CXD1171M 8-Bit D/A Converter [IC109]
    - Sony CXD1030M Sync Signal Generator [IC120]
    - Zilog Z8622812PSC Line 21 Closed-Caption Controller [IC124]
    - Sony CXD1132Q Time Code Generator/Reader [IC126]

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8534.h"
#include "machine/cxd1095.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"


namespace {

class bvm_state : public driver_device
{
public:
	bvm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void bvm(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8536_device> m_maincpu;
};


void bvm_state::mem_map(address_map &map)
{
	map(0x00000, 0x0f67f).rw("flash", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x10000, 0x17fff).ram().share("nvram");
	map(0x18000, 0x18007).rw("cxdio0", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x18020, 0x18027).rw("cxdio1", FUNC(cxd1095_device::read), FUNC(cxd1095_device::write));
	map(0x40000, 0x7ffff).rw("flash", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
}


static INPUT_PORTS_START(bvm)
INPUT_PORTS_END

void bvm_state::bvm(machine_config &config)
{
	HD6435368(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_mode(3); // internal ROM not used here?
	m_maincpu->set_addrmap(AS_PROGRAM, &bvm_state::mem_map);

	CAT28F020(config, "flash");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	CXD1095(config, "cxdio0");
	CXD1095(config, "cxdio1");
}

ROM_START(bvm20f1e)
	ROM_REGION(0x40000, "flash", 0)
	ROM_LOAD("cat28f020p-15.ic3", 0x00000, 0x40000, CRC(43a1bdf8) SHA1(65cf61921ac86a8baa3a3da4ed0b3e61aa2f03b3))

	ROM_REGION(0x10000, "eproms", 0)
	ROM_LOAD("cd93-27c256.ic107", 0x0000, 0x8000, CRC(f8da1f2f) SHA1(b2f5a730d0f26f2a12fd62a111ee429da8426877))
	ROM_LOAD("541d-27c256.ic108", 0x8000, 0x8000, CRC(9da347f9) SHA1(413096830bdcae6404e9d686abb56e60d58bdc2f))
ROM_END

} // anonymous namespace


SYST(1998, bvm20f1e, 0, 0, bvm, bvm, bvm_state, empty_init, "Sony", "Trinitron Color Video Monitor BVM-20F1E", MACHINE_IS_SKELETON)
