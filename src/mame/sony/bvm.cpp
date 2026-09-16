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

#include "multibyte.h"


namespace {

class bvm_state : public driver_device
{
public:
	bvm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void bvm(machine_config &config) ATTR_COLD;

	void init_bvm() ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8536_device> m_maincpu;
};


void bvm_state::mem_map(address_map &map)
{
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
	m_maincpu->set_mode(4);
	m_maincpu->set_addrmap(AS_PROGRAM, &bvm_state::mem_map);

	CAT28F020(config, "flash");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	CXD1095(config, "cxdio0");
	CXD1095(config, "cxdio1");
}

ROM_START(bvm20f1e)
	ROM_REGION(0xf680, "maincpu", 0)
	ROM_LOAD("hd6435368cp10.ic1", 0x0000, 0xf680, NO_DUMP)

	ROM_REGION(0x40000, "flash", 0)
	ROM_LOAD("cat28f020p-15.ic3", 0x00000, 0x40000, CRC(43a1bdf8) SHA1(65cf61921ac86a8baa3a3da4ed0b3e61aa2f03b3))

	ROM_REGION(0x10000, "eproms", 0)
	ROM_LOAD("cd93-27c256.ic107", 0x0000, 0x8000, CRC(f8da1f2f) SHA1(b2f5a730d0f26f2a12fd62a111ee429da8426877))
	ROM_LOAD("541d-27c256.ic108", 0x8000, 0x8000, CRC(9da347f9) SHA1(413096830bdcae6404e9d686abb56e60d58bdc2f))
ROM_END

void bvm_state::init_bvm()
{
	// Generate a fake internal ROM that does nothing but call exception handlers using PJSR
	u8 *rom = memregion("maincpu")->base();
	put_u32be(&rom[0], 0x00000200);
	put_u24be(&rom[0x200], 0x04048d); // LDC.B #H'04, DP
	put_u32be(&rom[0x203], 0x15000180); // MOV.B @H'0001:16, R0
	put_u32be(&rom[0x207], 0x1d000281); // MOV.W @H'0002:16, R1
	put_u16be(&rom[0x20b], 0x11c0); // PJMP @R0
	u32 dst = 0x20d;
	for (u16 v = 0x0004; v < 0x0200; v += 4)
	{
		put_u32be(&rom[v], dst);
		put_u16be(&rom[dst], 0x1203); // STM.W (R0-R1), @-SP
		put_u16be(&rom[dst + 2], 0xb79d); // STC.B DP, @-SP
		put_u24be(&rom[dst + 4], 0x04048d); // LDC.B #H'04, DP
		put_u32be(&rom[dst + 7], 0x15000180 | u32(v) << 8); // MOV.B @H'0vv1:16, R0
		put_u32be(&rom[dst + 11], 0x1d000281 | u32(v) << 8); // MOV.W @H'0vv2:16, R1
		put_u16be(&rom[dst + 15], 0x11c8); // PJSR @R0
		put_u16be(&rom[dst + 17], 0xc78d); // LDC.B @SP+, DP
		put_u16be(&rom[dst + 19], 0x0203); // LDM.W @SP+, (R0-R1)
		rom[dst + 21] = 0x0a; // RTE
		dst += 22;
	}
}

} // anonymous namespace


SYST(1998, bvm20f1e, 0, 0, bvm, bvm, bvm_state, init_bvm, "Sony", "Trinitron Color Video Monitor BVM-20F1E", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
