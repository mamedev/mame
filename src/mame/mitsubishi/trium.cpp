// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Mitsubishi Trium phone.

    The SoC on the Trium Eclipse PCB has Philips and ARM logos, and is labeled:

        VP40577A
        Y03552.Y1
        KS0150 A
        VP40577A

****************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"


namespace {

class trium_state : public driver_device
{
public:
	trium_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void trium(machine_config &config);

private:
	u16 one_r();

	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

u16 trium_state::one_r()
{
	return 1;
}

void trium_state::mem_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("firmware", 0);
	map(0x01000000, 0x0103ffff).ram();
	map(0x03000000, 0x03007fff).ram();
	map(0x04001158, 0x0400115b).ram();
	map(0x04001698, 0x04001699).r(FUNC(trium_state::one_r));
}

static INPUT_PORTS_START(trium)
INPUT_PORTS_END

void trium_state::trium(machine_config &config)
{
	ARM7(config, m_maincpu, 50'000'000); // unknown type and clock
	m_maincpu->set_addrmap(AS_PROGRAM, &trium_state::mem_map);
}


ROM_START(triumec)
	ROM_REGION32_LE(0x400000, "firmware", 0)
	ROM_LOAD("2eclipse_pl_path_l10_4raqa060.bin", 0x000000, 0x400000, CRC(886de4ae) SHA1(44e627f0d6aee3e066ec877dfd40c2bfd9bfeb79))
ROM_END

} // anonymous namespace

SYST(2002, triumec, 0, 0, trium, trium, trium_state, empty_init, "Mitsubishi", "Trium Eclipse", MACHINE_IS_SKELETON)
