// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Siemens Bitel and similar BtX terminals.

    "Bitel" apparently stands for "Bildschirmtext + Telefon."
    "Fe Ap" is short for "Fernsprechapparat."

    Both of these terminals have an integrated membrane keyboard and a
    telephone receiver mounted on top of the black-and-white monitor.

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"
//#include "video/saa5350.h"


namespace {

class bitel_state : public driver_device
{
public:
	bitel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void t3210(machine_config &config);
	void feap90(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;
	void sub_prog_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
};

void bitel_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void bitel_state::sub_prog_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("sub", 0);
}

static INPUT_PORTS_START(bitel)
INPUT_PORTS_END

void bitel_state::t3210(machine_config &config)
{
	I8031(config, m_maincpu, 12000000); // main board clocks unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &bitel_state::prog_map);

	I8742(config, "upi", 6000000).set_disable();

	i8039_device &submcu(I8039(config, "submcu", 4.194304_MHz_XTAL));
	submcu.set_addrmap(AS_PROGRAM, &bitel_state::sub_prog_map);
}

void bitel_state::feap90(machine_config &config)
{
	I8031(config, m_maincpu, 12000000); // XTAL illegible
	m_maincpu->set_addrmap(AS_PROGRAM, &bitel_state::prog_map);

	I8042AH(config, "upi", 6000000).set_disable(); // XTAL illegible
}

ROM_START(t3210) // i8031, 8742, D80C39C // 4+2k ram onboard; 24kb in battery-backed expansion
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("s22723_r115-c1-6_ct.d6", 0x0000, 0x8000, CRC(d09fea94) SHA1(52168060093dfe964c0316d9ff335cd59da01d48))
	ROM_LOAD("s22723_r115-c2-6_ct.d7", 0x8000, 0x8000, CRC(6e1eaacd) SHA1(cfda25dbbeddc7c75379c4b0dc97addb602d79ef))

	ROM_REGION(0x800, "upi", 0)
	ROM_LOAD("d8742_s22723_r118-c1.d16", 0x000, 0x800, CRC(f334a2a3) SHA1(c1cd4d775c2984252e6869a4c8f99d56646b89e9) BAD_DUMP) // BADADDR xx-xxxxxxxx

	ROM_REGION(0x800, "sub", 0) // lock card
	ROM_LOAD("s22723_r121-c2-2.d11", 0x000, 0x800, CRC(f0eda00e) SHA1(6b0d9f5e9d99644c3be16cbf0c0d3b1ea05aabee))

	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("prom_s22723_r120-c1.bin", 0x000, 0x100, CRC(4460cd50) SHA1(fe36d758d64493cb5f8217fe51bbbe8203424fbe))
ROM_END


ROM_START(feap90) // i8031, 80C42C121 (+SAA5351) // 4+2k ram onboard; 24kb in battery-backed expansion
	ROM_REGION(0x18000, "program", 0)
	ROM_LOAD("s22723-r116-c25-6 ex.d6", 0x00000, 0x10000, CRC(8362778d) SHA1(30fbe45eaedc1ed2e7b189f12e2ba7c23ab75de7))
	ROM_LOAD("s22723-r116-c26-6 ex.d2", 0x10000, 0x08000, CRC(121622ba) SHA1(c447da13f88772ec7d26e55ca8822e2c2dc3ecef))

	ROM_REGION(0x800, "upi", 0)
	ROM_LOAD("8838p8-80c42c121-a85.d16", 0x000, 0x800, NO_DUMP)
ROM_END

} // anonymous namespace


COMP(1986, t3210,  0, 0, t3210,  bitel, bitel_state, empty_init, "Siemens", "Bitel T3210", MACHINE_IS_SKELETON)
COMP(1989, feap90, 0, 0, feap90, bitel, bitel_state, empty_init, "Siemens", "Multitel Fe Ap 90-1.1", MACHINE_IS_SKELETON)
