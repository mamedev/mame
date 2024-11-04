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

    The CM-300 is a cut down version only one midi in and no display.

    The SCC-1 is an ISA board variant of the CM-300 with a MPU-401 frontend added to
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


namespace {

static INPUT_PORTS_START( sc55 )
INPUT_PORTS_END

class sc55_state : public driver_device
{
public:
	sc55_state(const machine_config &mconfig, device_type type, const char *tag);

	void sc55(machine_config &config);

private:
	required_device<h8532_device> m_maincpu;

	void sc55_map(address_map &map) ATTR_COLD;
};

sc55_state::sc55_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
{
}

void sc55_state::sc55_map(address_map &map)
{
	map(0x00000, 0x07fff).rom().region("maincpu", 0);
	map(0x40000, 0x7ffff).rom().region("progrom", 0);
}

void sc55_state::sc55(machine_config &config)
{
	HD6435328(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc55_state::sc55_map);
}

ROM_START( sc55 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "roland_r15199778_6435328a97f.ic30", 0x0000, 0x8000, CRC(4ed0d171) SHA1(dd01ec54027751c2f2f2e47bbb7a0bf3d1ca8ae2) )

	ROM_REGION( 0x40000, "progrom", 0 )
	ROM_LOAD( "roland_r15209363.ic23", 0x000000, 0x040000, CRC(2dc58549) SHA1(9c17f85e784dc1549ac1f98d457b353393331f6b) )

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD( "roland-gss.a_r15209276.ic28", 0x000000, 0x100000, CRC(1ac774d3) SHA1(8cc3c0d7ec0993df81d4ca1970e01a4b0d8d3775) )
	ROM_LOAD( "roland-gss.b_r15209277.ic27", 0x100000, 0x100000, CRC(8dcc592a) SHA1(80e6eb130c18c09955551563f78906163c55cc11) )
	ROM_LOAD( "roland-gss.c_r15209281.ic26", 0x200000, 0x100000, CRC(e21ebc04) SHA1(7454b817778179806f3f9d1985b3a2ef67ace76f) )
ROM_END

ROM_START( sc155 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "roland_r15199799.ic30", 0x0000, 0x8000, CRC(a160da90) SHA1(76f646bc03f66dbee7606f2181d4ea76f05ece7d) )

	ROM_REGION( 0x40000, "progrom", 0 )
	ROM_LOAD( "roland_r15209361.ic15", 0x000000, 0x040000, CRC(e19d4a52) SHA1(e9e1bb1bc2691145ffe17f01a48d6614c9f22225) )

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD( "roland-gss.a_r15209276.ic28", 0x000000, 0x100000, CRC(1ac774d3) SHA1(8cc3c0d7ec0993df81d4ca1970e01a4b0d8d3775) )
	ROM_LOAD( "roland-gss.b_r15209277.ic27", 0x100000, 0x100000, CRC(8dcc592a) SHA1(80e6eb130c18c09955551563f78906163c55cc11) )
	ROM_LOAD( "roland-gss.c_r15209281.ic26", 0x200000, 0x100000, CRC(e21ebc04) SHA1(7454b817778179806f3f9d1985b3a2ef67ace76f) )
ROM_END

ROM_START( cm300 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "roland_r15199774.ic6", 0x0000, 0x8000, NO_DUMP ) // scc-1: ic10

	ROM_REGION( 0x40000, "progrom", 0 )
	// scc-1: ic14
	ROM_DEFAULT_BIOS("120")
	ROM_SYSTEM_BIOS(0, "120", "GS Standard  VER=1.20")
	ROMX_LOAD( "roland_r15279812.ic8", 0x000000, 0x040000, CRC(546542ab) SHA1(b288cef2aa2df60cbaa2c084a77a68b298de9567), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "110", "GS Standard  VER=1.10")
	ROMX_LOAD( "roland_r15279809.ic8", 0x000000, 0x040000, CRC(94d96954) SHA1(32e76286a626cf960b6665792e53dce3d51170d1), ROM_BIOS(1) )

	ROM_REGION( 0x300000, "waverom", 0 )
	ROM_LOAD( "roland_r15279806.ic2", 0x000000, 0x100000, CRC(b1b31a41) SHA1(891cf2ac5ca64f453b370b9076f9fb2b4ebc5dcf) ) // scc-1: ic17
	ROM_LOAD( "roland_r15279807.ic3", 0x100000, 0x100000, CRC(359edfb2) SHA1(49f38f181b444fc39ad86c4ddab3b25bf839d0b4) ) // scc-1: ic18
	ROM_LOAD( "roland_r15279808.ic4", 0x200000, 0x100000, CRC(0f826c7f) SHA1(4d91cdeaed048d653dbf846a221003c3a3f08279) ) // scc-1: ic19

	// Only in the scc-1, should be moved to ISA
	// 4Mhz 6801
	ROM_REGION( 0x1000, "mpu401", 0)
	ROM_LOAD( "roland_r15239182.ic2", 0x0000, 0x1000, CRC(8aea085f) SHA1(3cce3fb328ec4055a53ae976d790ced257ae1f67) )
ROM_END

} // anonymous namespace


SYST( 1991, sc55, 0, 0, sc55, sc55, sc55_state, empty_init, "Roland", "Sound Canvas SC-55", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 1992, sc155, sc55, 0, sc55, sc55, sc55_state, empty_init, "Roland", "Sound Canvas SC-155", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
SYST( 1991, cm300, sc55, 0, sc55, sc55, sc55_state, empty_init, "Roland", "Sound Canvas CM-300", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
