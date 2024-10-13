// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************************************

    Roland Sound Canvas SC-55mkII

    Skeleton by R. Belmont

    The Roland SC-55mkII is an expander (synthesizer without the keyboard)
    from 1994.  It has 28 voice polyphony, is 16 part multitimbral, and
    outputs 18-bit stereo samples at 32 kHz.  The synthesis engine uses a
    combination of Roland's LA and straight PCM playback.

    The front panel includes the power switch, a headphone jack with volume knob,
    a second MIDI IN port, a large LCD, ALL and MUTE buttons, and a group of up/down
    buttons for Part, Level, Reverb, Key Shift, Instrument, Pan, Chorus, and MIDI Channel.

    Main PCB:

    20.0 MHz crystal
    Roland R15199848  HD6475328F    Hitachi H8/532 MCU with internal ROM (main CPU)
    Roland R15199849  M37409M2-FP   Mitsubishi M740 series microcontroller (sub CPU)
    Roland R15209463  4M MASK ROM   Main program, version 1.00
    Roland R15209359  16M WAVE ROM
    Roland R15279813  8M WAVE ROM
    Roland R15279543  SRM20256SLM10 SRAM (32K x 8-bit non-volatile work RAM)
    Roland R15179463  TC51832FL-85  PSRAM (32Kword x 8-bit PCM custom chip's work RAM)
    Roland R15219714  uPD63200GS-E2 D/A converter
    PCM custom is IC26, part number is not currently known

    TODO:
    - H8/500 series CPU core
    - M37409M2 MCU support
    - LCD
    - Sound synthesis
*/

#include "emu.h"

#include "cpu/h8500/h8532.h"


namespace {

static INPUT_PORTS_START( sc55mk2 )
INPUT_PORTS_END

class sc55mk2_state : public driver_device
{
public:
	sc55mk2_state(const machine_config &mconfig, device_type type, const char *tag);

	void sc55mk2(machine_config &config);

private:
	required_device<h8532_device> m_maincpu;

	void sc55mk2_map(address_map &map) ATTR_COLD;
};

sc55mk2_state::sc55mk2_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
{
}

void sc55mk2_state::sc55mk2_map(address_map &map)
{
	map(0x40000, 0x7ffff).rom().region("progrom", 0);
}

void sc55mk2_state::sc55mk2(machine_config &config)
{
	HD6435328(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc55mk2_state::sc55mk2_map);
}

ROM_START( sc55mk2 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )  // H8/532 main code
	ROM_LOAD("r15199858_main_mcu.bin", 0x000000, 0x008000, CRC(9b66631f) SHA1(b91bb1d9dccffe831b7cfde7800a3fe32b2fbda6))

	ROM_REGION( 0x1000, "subcpu", 0 )   // M37409M2 sub-CPU with 3 UARTs, clocked at 10 MHz
	ROM_LOAD("r15199880_secondary_mcu.bin", 0x000000, 0x001000, CRC(702c0a82) SHA1(4d48578d811a762a8e7bfaf18989bcac70ae1ba4))

	ROM_REGION( 0x80000, "progrom", 0 ) // additional H8/532 code and patch data
	ROM_LOAD("r00233567_control.bin", 0x000000, 0x080000, CRC(fcee1e8e) SHA1(078cb5feea05e80bb9a1bb857a2163ee434fd053))

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD("r15209359_pcm_1.bin", 0x000000, 0x200000, CRC(1519d3b3) SHA1(96708cb21381c2fd03de4babbf7aea301c7594a6))
	ROM_LOAD("r15279813_pcm_2.bin", 0x200000, 0x100000, CRC(0f826c7f) SHA1(4d91cdeaed048d653dbf846a221003c3a3f08279))
ROM_END

} // anonymous namespace


SYST( 1994, sc55mk2, 0, 0, sc55mk2, sc55mk2, sc55mk2_state, empty_init, "Roland", "Sound Canvas SC-55mkii", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
