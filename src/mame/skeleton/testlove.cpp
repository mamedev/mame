// license:BSD-3-Clause
// copyright-holders:

/*
Love tester machine
unknown manufacturer

'Test your love power' on cabinet
PCB marked '1003-C' and 'WT 30' on solder side
'Kessler Ellis Products CO.' sticker on solder side, probably the coin counter producer

Main components:
D8748
3.579545 MHz XTAL
piezo (?) speaker
26 red diodes
2 9-segment LEDs
2 7-segment LEDs
*/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"

#include "speaker.h"


// configurable logging
#define LOG_PORTS     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PORTS)

#include "logmacro.h"

#define LOGPORTS(...)     LOGMASKED(LOG_PORTS,     __VA_ARGS__)


namespace {

class testlove_state : public driver_device
{
public:
	testlove_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void testlove(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<i8748_device> m_maincpu;
};


void testlove_state::machine_start()
{
}


static INPUT_PORTS_START(testlove)
INPUT_PORTS_END


void testlove_state::testlove(machine_config &config)
{
	I8748(config, m_maincpu, 3.579545_MHz_XTAL);
	m_maincpu->p1_in_cb().set([this] () { LOGPORTS("%s: P1 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p2_in_cb().set([this] () { LOGPORTS("%s: P2 in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->p1_out_cb().set([this] (uint8_t data) { LOGPORTS("%s: P1 out %02x\n", machine().describe_context(), data); });
	m_maincpu->p2_out_cb().set([this] (uint8_t data) { LOGPORTS("%s: P2 out %02x\n", machine().describe_context(), data); });
	m_maincpu->bus_in_cb().set([this] () { LOGPORTS("%s: bus in\n", machine().describe_context()); return uint8_t(0); });
	m_maincpu->bus_out_cb().set([this] (uint8_t data) { LOGPORTS("%s: bus out %02x\n", machine().describe_context(), data); });
	m_maincpu->t0_in_cb().set([this] () { LOGPORTS("%s: T0 in\n", machine().describe_context()); return int(0); });
	m_maincpu->t1_in_cb().set([this] () { LOGPORTS("%s: T1 in\n", machine().describe_context()); return int(0); });

	SPEAKER(config, "speaker").front_center();

	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);
}


ROM_START( testlove )
	ROM_REGION( 0x400, "maincpu", 0 )
	ROM_LOAD( "hrtvtc_v1.7.bin", 0x000, 0x400, CRC(fc08e84b) SHA1(d631a0c773e8b8e1cd9031eae4686902e1873eeb) )
ROM_END

} // anonymous namespace


GAME( 198?, testlove, 0, testlove, testlove, testlove_state, empty_init, ROT0, "<unknown>", "Love Power", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // title may be wrong
