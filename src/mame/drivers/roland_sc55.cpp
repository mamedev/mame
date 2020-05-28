// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*************************************************************************************************

    Roland Sound Canvas SC-55

    Skeleton by R. Belmont

    Reference and LCD photo: http://www.synthman.com/roland/Roland_SC-55.html
                             http://en.wikipedia.org/wiki/Roland_SCC-1

    The Roland SC55 is an expander (synthesizer without the keyboard)
    from 1991.  It has 24 voice polyphony, is 16 part multitimbral, and
    outputs 16-bit stereo samples at 32 kHz.  The synthesis engine uses a
    combination of Roland's LA and straight PCM playback.

    The front panel includes the power switch, a headphone jack with volume knob,
    a second MIDI IN port, a large LCD, ALL and MUTE buttons, and a group of up/down
    buttons for Part, Level, Reverb, Key Shift, Instrument, Pan, Chorus, and MIDI Channel.

    The SCC-1 is an ISA board variant of the SC55 with a MPU-401 frontend added to
    communicate with the synth.

    Main PCB:

    20.0 MHz crystal
    Roland R15239147  HG62E11B23FS  1L1 Japan
    Roland R15199778  6435328A97F   1M1 R Japan - Hitachi H8/532 MCU with internal ROM (Hitachi p/n HD6435328A97F)
    Roland R15239148  24201F002  9148EAI Japan
    Roland R15209363  LH532H6D   9152 D
    R15239176  BU3910F
    HM62256ALFP-12T   32K by 8-bit RAM
    65256BLFP-12T     32K by 8-bit high-speed pseudo-static RAM
    MB89251A - Serial data transceiver

    LCD controller (on front panel board) is a Toshiba T7934.
*/

#include "emu.h"
#include "machine/ram.h"
#include "cpu/h8500/h8532.h"

static INPUT_PORTS_START( sc55 )
INPUT_PORTS_END

class sc55_state : public driver_device
{
public:
	sc55_state(const machine_config &mconfig, device_type type, const char *tag);

	void sc55(machine_config &config);

private:
	required_device<h8532_device> m_maincpu;

	void sc55_map(address_map &map);
};

sc55_state::sc55_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
{
}

void sc55_state::sc55_map(address_map &map)
{
	map(0x40000, 0x7ffff).rom().region("progrom", 0);
}

void sc55_state::sc55(machine_config &config)
{
	HD6435328(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc55_state::sc55_map);
}

ROM_START( sc55 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "roland_r15199778_6435328a97f.ic30", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x40000, "progrom", 0 ) // additional H8/532 code and patch data - revisions match main CPU revisions
	ROM_LOAD( "roland_r15209363.ic23", 0x000000, 0x040000, CRC(2dc58549) SHA1(9c17f85e784dc1549ac1f98d457b353393331f6b) )

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD( "roland-gss.a_r15209276.ic28", 0x000000, 0x100000, CRC(1ac774d3) SHA1(8cc3c0d7ec0993df81d4ca1970e01a4b0d8d3775) )
	ROM_LOAD( "roland-gss.b_r15209277.ic27", 0x100000, 0x100000, CRC(8dcc592a) SHA1(80e6eb130c18c09955551563f78906163c55cc11) )
	ROM_LOAD( "roland-gss.c_r15209281.ic26", 0x200000, 0x100000, CRC(e21ebc04) SHA1(7454b817778179806f3f9d1985b3a2ef67ace76f) )
ROM_END

SYST( 1991, sc55, 0, 0, sc55, sc55, sc55_state, empty_init, "Roland", "Sound Canvas SC-55", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
