// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for AT&T Model 3B2 computer.

****************************************************************************/

#include "emu.h"
#include "cpu/we32000/we32100.h"

class att3b2_state : public driver_device
{
public:
	att3b2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void att3b2(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<we32100_device> m_maincpu;
};


void att3b2_state::mem_map(address_map &map)
{
	map(0x00000000, 0x00007fff).rom().region("bootstrap", 0);
}


static INPUT_PORTS_START(att3b2)
INPUT_PORTS_END

void att3b2_state::att3b2(machine_config &config)
{
	WE32100(config, m_maincpu, 10_MHz_XTAL); // special WE32102 XTAL runs at 1x or 2x speed
	m_maincpu->set_addrmap(AS_PROGRAM, &att3b2_state::mem_map);

	// TODO: devices
}

ROM_START(3b2_300)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("3b2300-c.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("3b2300-d.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("3b2300-e.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("3b2300-f.bin", 0x0003, 0x2000, CRC(b8e138c4) SHA1(d2da4a7150150d0f9294814edb7ed357f9341858))
ROM_END

ROM_START(3b2_310)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("aayyc.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("aayyd.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("aayye.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("aayyf.bin", 0x0003, 0x2000, CRC(39dcfd8c) SHA1(2e662b3811f78ab689bc2687b73e3158f33f7f89))
ROM_END

ROM_START(3b2_400)
	ROM_REGION32_BE(0x8000, "bootstrap", 0)
	ROM_LOAD32_BYTE("3b2-aayyc.bin", 0x0000, 0x2000, CRC(b7f955c5) SHA1(54886c4fce5a5681af84538b65de1cc68d0f7af4))
	ROM_LOAD32_BYTE("3b2-aayyd.bin", 0x0001, 0x2000, CRC(5812e262) SHA1(5a69714c0c8f21d7655e43443dee0e76cf219403))
	ROM_LOAD32_BYTE("3b2-aayye.bin", 0x0002, 0x2000, CRC(e28ca685) SHA1(a337a0480218db8c2d984442f7bc560834853152))
	ROM_LOAD32_BYTE("3b2-aayyf-4.bin", 0x0003, 0x2000, CRC(85b8c5d3) SHA1(85bdf3f889f6c14cbf33ce81421f1f1d02328223))
ROM_END

COMP(1985, 3b2_300, 0,       0, att3b2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/300", MACHINE_IS_SKELETON)
COMP(198?, 3b2_310, 3b2_300, 0, att3b2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/310", MACHINE_IS_SKELETON)
COMP(198?, 3b2_400, 3b2_300, 0, att3b2, att3b2, att3b2_state, empty_init, "AT&T", "3B2/400", MACHINE_IS_SKELETON)
