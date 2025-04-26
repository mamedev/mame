// license:BSD-3-Clause
// copyright-holders:

/*********************************************************************************

    Skeleton driver for VTech MagiBook / LeapFrog LeapStart interactive books.
    Video from the real hardware (VTech): https://youtube.com/shorts/OcZ1ADZ_rTo

*********************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "speaker.h"


namespace {


class magibook_state : public driver_device
{
public:
	magibook_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void magibook(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void magibook_state::machine_start()
{
}

void magibook_state::machine_reset()
{
}

// Wired touch pen plus "+", "-" and power buttons.
static INPUT_PORTS_START( magibook )
INPUT_PORTS_END

void magibook_state::magibook(machine_config &config)
{
	ARM9(config, m_maincpu, 24'000'000); // GeneralPlus GP326813, unknown frequency

	// Screenless

	SPEAKER(config, "mono").front_left();
}

// Spanish machine on VTech 6021 hardware, may be different between regions.
ROM_START( magibooksp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "internal.u200", 0x00000, 0x10000, NO_DUMP ) // Unknown internal ROM size, if any

	ROM_REGION( 0x11000000, "program", 0 )
	ROM_LOAD( "vtech_magibook_tc58nvg1s3hta00.u302", 0x00000000, 0x11000000, CRC(035aa5ba) SHA1(b118954b764daab217d8b6b6785b0cedfcb88780) )
ROM_END

} // anonymous namespace


CONS( 2016, magibooksp, 0, 0, magibook, magibook, magibook_state, empty_init, "VTech", "MagiBook (Spanish)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_REQUIRES_ARTWORK )
