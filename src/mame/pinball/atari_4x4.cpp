// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

PINBALL
Atari 4X4

Unlike all the other Atari pinballs, this one seems to use a 6502.
There's no manuals or full schematics.

The game has 8 captive balls in 2 banks of 4, and they can be released by hitting a
 target (one per ball). Therefore, most of the time you're playing multiball.

Status:
- Skeleton

ToDo:
- Everything
- The audio prom is the same one used in Atari System 2, so the sound card should be the
  same.


*****************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/m6502/m6502.h"
//#include "machine/timer.h"
//#include "sound/dac.h"
//#include "speaker.h"
//#include "atari_s2.lh"

namespace {

class atari_4x4_state : public genpin_class
{
public:
	atari_4x4_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void fourx4(machine_config &config);

private:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	required_device<m6502_device> m_maincpu;
	output_finder<68> m_digits;  // don't know how many
	output_finder<80> m_io_outputs;   // ?? solenoids + ?? lamps
};

void atari_4x4_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram();
	map(0x1000, 0x17ff).ram();
	map(0x2000, 0x27ff).ram();
	map(0x3000, 0x37ff).ram();
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( atari_4x4 )
INPUT_PORTS_END

void atari_4x4_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	//save_item(NAME(m_segment));
}

void atari_4x4_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
}


void atari_4x4_state::fourx4(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 1'000'000);  // guess
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_4x4_state::mem_map);

	/* Video */
	//config.set_default_layout(layout_fourx4);

	/* Sound */
	genpin_audio(config);
	//SPEAKER(config, "speaker").front_center();
	//DAC_4BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // r23-r26 (68k,33k,18k,8.2k)
	//DAC_3BIT_BINARY_WEIGHTED(config, m_dac1, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // r18-r20 (100k,47k,100k)
	//TIMER(config, "timer_s").configure_periodic(FUNC(atari_s2_state::timer_s), attotime::from_hz(150000));
}

/*-------------------------------------------------------------------
/ 4x4 (10/1982)
/-------------------------------------------------------------------*/
ROM_START(fourx4)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("8000ce65.bin", 0x0000, 0x2000, CRC(27341155) SHA1(c0da1fbf64f93ab163b2ea6bfbfc7b778cea819f))
	ROM_LOAD("a0004c37.bin", 0x2000, 0x2000, CRC(6f93102f) SHA1(d6520987ed5805b0e6b5da5653fc7cb063e86dda))
	ROM_LOAD("c000a70c.bin", 0x4000, 0x2000, CRC(c31ca8d3) SHA1(53f20eff0084771dc61d19db7ddae52e4423e75e))
	ROM_RELOAD(0x6000, 0x2000)

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20967-01.j3", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

} // Anonymous namespace

GAME( 1982, fourx4, 0, fourx4, atari_4x4, atari_4x4_state, empty_init, ROT0, "Atari", "4x4", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
