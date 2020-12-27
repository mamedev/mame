// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Hungry Hungry Hippos redemption game by I.C.E. Inc.

****************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"

class ice_hhhippos_state : public driver_device
{
public:
	ice_hhhippos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hhhippos(machine_config &config);

private:
	required_device<m68hc05_device> m_maincpu;
};

static INPUT_PORTS_START(hhhippos)
INPUT_PORTS_END

void ice_hhhippos_state::hhhippos(machine_config &config)
{
	M68HC705C8A(config, m_maincpu, 2_MHz_XTAL);

	// TODO: sound (R2R DACs streamed from ROMs using HCMOS ripple counters)
}

ROM_START(hhhippos)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("68hc705c8.bin", 0x0000, 0x2000, CRC(5c74bcd7) SHA1(3c30ae38647c8f69f7bbcdbeb35b748c8f4c4cd8))

	ROM_REGION(0x10000, "audio0", 0)
	ROM_LOAD("u119.bin", 0x00000, 0x10000, CRC(77c8bd90) SHA1(e9a044d83f39fb617961f8985bc4bed06a03e07b))

	ROM_REGION(0x20000, "audio1", 0)
	ROM_LOAD("u122.bin", 0x00000, 0x20000, CRC(fc188905) SHA1(7bab8feb1f304c9fe7cde31aff4b40e2db56d525))
ROM_END

GAME(1991, hhhippos, 0, hhhippos, hhhippos, ice_hhhippos_state, empty_init, ROT0, "ICE (Innovative Concepts in Entertainment)", "Hungry Hungry Hippos (redemption game)", MACHINE_IS_SKELETON_MECHANICAL)
