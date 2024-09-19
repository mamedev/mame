// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Orla XM200 PCM sound module.

***************************************************************************/

#include "emu.h"
#include "cpu/st9/st905x.h"


namespace {

class xm200_state : public driver_device
{
public:
	xm200_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void xm200(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<st9_device> m_maincpu;
};


void xm200_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0xe000, 0xefff).ram();
}


static INPUT_PORTS_START(xm200)
INPUT_PORTS_END

void xm200_state::xm200(machine_config &config)
{
	ST90R50(config, m_maincpu, 11'000'000); // type and clock guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &xm200_state::mem_map);
}

// The dump claims to be from "Commander" rather than Orla. This might be some sort of regional branding,
// since Orla released other sound modules using that name.
ROM_START(xm200)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("xm200_st m27c25b-orla ok300_q.bin", 0x0000, 0x8000, CRC(fa5dc621) SHA1(867516171028c278eccdbe65ea750deb07a684ff))
	// 0x0100: "***** Orla OK300 Keyboard ***** (c) Richard Watts Associates July 90, V1.0"

	ROM_REGION(0x20000, "pcm", 0)
	ROM_LOAD("s r_s 27c512-20 fa.bin", 0x00000, 0x10000, CRC(dab36166) SHA1(c98096c699fcf23e42571ce58e46bf40f569aa1f))
	ROM_LOAD("acc_st m27c512-15fi.bin", 0x10000, 0x10000, CRC(15a7c54e) SHA1(e5ed7806060b6b74f0e71960af5eb5307d1c88d7))
ROM_END

} // anonymous namespace


SYST(1990, xm200, 0, 0, xm200, xm200, xm200_state, empty_init, "Orla", "XM200 Orchestra Module", MACHINE_IS_SKELETON)
