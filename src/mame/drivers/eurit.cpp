// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Ascom Eurit Euro-ISDN Telefon.

****************************************************************************/

#include "emu.h"
#include "cpu/m37710/m37710.h"

class eurit_state : public driver_device
{
public:
	eurit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void eurit30(machine_config &mconfig);

private:
	void mem_map(address_map &map);

	required_device<m37730s2_device> m_maincpu;
};


void eurit_state::mem_map(address_map &map)
{
	map(0x000480, 0x007fff).ram();
	map(0x008000, 0x00ffff).rom().region("firmware", 0x8000);
	map(0x040000, 0x05ffff).rom().region("firmware", 0);
	map(0x0c0000, 0x0c0007).noprw(); // ?
}


static INPUT_PORTS_START(eurit30)
INPUT_PORTS_END

void eurit_state::eurit30(machine_config &config)
{
	M37730S2(config, m_maincpu, 8'000'000); // type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &eurit_state::mem_map);
}


ROM_START(eurit30)
	ROM_REGION16_LE(0x20000, "firmware", 0)
	ROM_LOAD("d_2.210", 0x00000, 0x20000, CRC(c77be0ac) SHA1(1eaba66dcb4f64cc33565ca85de25341572ddb2e))
ROM_END


SYST(1996, eurit30, 0, 0, eurit30, eurit30, eurit_state, empty_init, "Ascom", "Eurit 30", MACHINE_IS_SKELETON)
