// license:BSD-3-Clause
// copyright-holders: David Haywood

// Belatra Russian Fruit Machines (Video?)

/*

SoC is suspected to be an ARM7500 or similar.

The following hardware description comes from PCB with "GS Unterhaltungssysteme Vertriebs- & Produktions GmbH" sticker.
However they were probably only the distributor. The hardware is currently believed to be the same or cloned:

ULN2803A: DD5 & DD6 & DD4
MM74HC273: DD1, DD2, DD3
MM74HC244: DD7, DD8, DD9, DD10
DS1309: DD14
PALCE16V8: DD21
V62C518256L: DD13
29F160: DD11, DD12
CL-PS7500FE56QC: DD15
74AC04: DD19
EPSON 11.0592: DD17
EPSON 56.0000: DD18
CS4334K: DA1
ADM691AR 0228 138074.1: DD20
AT90S2313-10PI: DD16
ADM208EAR 0230 140807.1: DD22

RAM-Modul: 8x MT4C4M4E8DJ

Markings on the board:
CTE 002 94V-0 0245
002BG358A

Markings on the RAM-Modul:
MICRON TECHNOLOGY INC.
099 D U.S.A.
4096-43-73 ET

TODO:
- just a skeleton.
*/

/*

Possible Games:

Multi Poker
Fairy Land
Fairy Land 2
Viking`s Fun
Viking`s Fun SE
Merry Joiner
Spy Tricks
Lucky Drink
An Escape from Al Catraz
Gold Fever
Happy Mine
Siberian Gamble
Tinder Box
Air Mail
Barry Picker
Casino
The Elusive Gonzales
Fairy Land 3
Greengrocery
Merry Joiner
Piggy Bank
The Scrooge
Spirit of Prairies
Viking's Fun Mill

*/


#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/acorn_vidc.h"
#include "machine/arm_iomd.h"

#include "screen.h"
#include "speaker.h"


namespace {

class belatra_state : public driver_device
{
public:
	belatra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vidc(*this, "vidc"),
		m_iomd(*this, "iomd")
	{ }

	void belatra(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<arm_vidc20_device> m_vidc;
	required_device<arm7500fe_iomd_device> m_iomd;

	void program_map(address_map &map) ATTR_COLD;
};

void belatra_state::program_map(address_map &map)
{
	map(0x00000000, 0x001fffff).rom().region("maincpu", 0x000000); // TODO: implement this as proper flash ROM device
	map(0x00800000, 0x009fffff).rom().region("maincpu", 0x200000); // "
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map)); // TODO: the games write to adn read from some unimplemented registers
																			 //       msecr_r is hardcoded to return 0x80 but these games seem to expect 0x20, too
	// map(0x03340000, 0x03340003).lr32(NAME([this] () -> uint32_t { return machine().rand() & 0x01; }));
	// map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	// map(0x08000000, 0x0800000f).r;
	map(0x10000000, 0x13ffffff).ram();
	map(0x14000000, 0x17ffffff).ram();
}

static INPUT_PORTS_START( belatra )
INPUT_PORTS_END

void belatra_state::belatra(machine_config &config)
{
	ARM7500(config, m_maincpu, 56_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &belatra_state::program_map);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	ARM_VIDC20(config, m_vidc, 24'000'000); // chip type and clock guessed
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(m_iomd, FUNC(arm_iomd_device::sound_drq));

	ARM7500FE_IOMD(config, m_iomd, 56_MHz_XTAL);
	m_iomd->set_host_cpu_tag(m_maincpu);
	m_iomd->set_vidc_tag(m_vidc);
	m_iomd->iocr_read_od<0>().set([this] () { logerror("%s: IOCR read OD 0\n", machine().describe_context()); return 0; });
	m_iomd->iocr_read_od<1>().set([this] () { logerror("%s: IOCR read OD 1\n", machine().describe_context()); return 0; });
	m_iomd->iocr_read_id().set([this] () { logerror("%s: IOCR read ID\n", machine().describe_context()); return 0; });
	m_iomd->iocr_write_od<0>().set([this] (int state) { logerror("%s: IOCR write OD 0 %d\n", machine().describe_context(), state); });
	m_iomd->iocr_write_od<1>().set([this] (int state) { logerror("%s: IOCR write OD 1 %d\n", machine().describe_context(), state); });
	m_iomd->iocr_write_id().set([this] (int state) { logerror("%s: IOCR write ID %d\n", machine().describe_context(), state); });
	m_iomd->iolines_read().set([this] () { logerror("%s: IO lines read\n", machine().describe_context()); return uint8_t(0); });
	m_iomd->iolines_write().set([this] (uint8_t data) { logerror("%s: IO lines write %02x\n", machine().describe_context(), data); });

	// AT90S2313(config, "mcu", xxxx); // TODO: AVR 8-bit core, only the fairyl2 set has a dump

	SPEAKER(config, "speaker", 2).front();
	// unknown sound
}


ROM_START( merryjn )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "am29f160bd_mery_up.bin",   0x000000, 0x200000, CRC(475161a5) SHA1(784904ffb949e56cac1126eaacf09db19a51b2f0) )
	ROM_LOAD( "am29f160bd_mery_down.bin", 0x200000, 0x200000, CRC(f8b6354c) SHA1(6de6e0c7927d2d673ac3fd5dc1c8263a73dcbb14) )
ROM_END

ROM_START( fairyl2 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "zfl2-1belatra.bin", 0x000000, 0x200000, CRC(cb0f3eba) SHA1(a7776810cfe037c25c196bbe900e5e17a2005d2d) )
	ROM_LOAD( "zfl2-2belatra.bin", 0x200000, 0x200000, CRC(755fad4b) SHA1(12243fdf95fcdd9012d1bbde6a18abb00918f560) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "at90s2313_fl2.bin", 0x000, 0x800, CRC(38e2d37e) SHA1(78178cb3ea219a71d1f15ffde722f9c03ad64dda) )
ROM_END

ROM_START( fairyl2a )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "fairy_land2_up_original.bin", 0x000000, 0x200000, CRC(e9c24e75) SHA1(c80acfdcab4ed098596cd3f92a38086881344bb2) )
	ROM_LOAD( "fairy_land2_dw_original.bin", 0x200000, 0x200000, CRC(9d07818f) SHA1(8427b264ef03379cd6f3fbe4f5e141837ffef6fe) )
ROM_END

ROM_START( fairyl2b )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "zfl2-1  china.bin", 0x000000, 0x200000, CRC(3bfd7a0f) SHA1(e67c3fa27e5945de23efb8cb9899bcb23808e4cf) )
	ROM_LOAD( "zfl2-2  china.bin", 0x200000, 0x200000, CRC(1e1e172a) SHA1(6fb2eec95947ec7995f9c3c52f26b2c6dd5fd6b9) )
ROM_END

ROM_START( fairyl2bl )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "fairy_land2_up_pirate.bin", 0x000000, 0x200000, CRC(e9c24e75) SHA1(c80acfdcab4ed098596cd3f92a38086881344bb2) )
	ROM_LOAD( "fairy_land2_dw_pirate.bin", 0x200000, 0x200000, CRC(00bf73d0) SHA1(3fa81815b4bbb5ed4dcd64afa1ad799a5a0bd48e) )
ROM_END

ROM_START( ldrink )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "lucky_up.bin",   0x000000, 0x200000, CRC(5d5277cc) SHA1(893369a58ea3ba3c7ba82a4cc4b1ffdbcc6745e2) )
	ROM_LOAD( "lucky_down.bin", 0x200000, 0x200000, CRC(e4c5b32a) SHA1(c8c44a7e35bbc88ba271566f8324b71818aa223c) )
ROM_END

ROM_START( ldrinka )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "zld-0012534-1belatra.bin", 0x000000, 0x200000, CRC(7a9b1e9f) SHA1(d9c7551b812bda509ba5ade09d2f82a77a5215d3) )
	ROM_LOAD( "zld-0012534-2belatra.bin", 0x200000, 0x200000, CRC(e3e74597) SHA1(0157dbf806eb2e5f0c9b9e8ed68d38748a713a6e) )
ROM_END

ROM_START( unkbel )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "29f160.dd11", 0x000000, 0x200000, CRC(cd92dd9d) SHA1(24aee4e3d986750183cb110bb3497da4c8179f73) )
	ROM_LOAD( "29f160.dd12", 0x200000, 0x200000, CRC(d03e57b8) SHA1(557ad08bbc5992f11722c04aba8ff52cf93ffe59) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x800, "mcu", ROMREGION_ERASE00 )
	ROM_LOAD( "at90s2313.dd16", 0x000, 0x800, NO_DUMP ) // protected

	ROM_REGION( 0x117, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "palce16v8.dd21", 0x000, 0x117, CRC(8bafb445) SHA1(5249747c958e14e622361002fd5bfbb1403f0d67) )
ROM_END

} // anonymous namespace


GAME( 2004, fairyl2,   0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 1)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, fairyl2a,  fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 2)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, fairyl2b,  fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 3)",         MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, fairyl2bl, fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (bootleg)",       MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2004, ldrink,    0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Lucky Drink (set 1)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, ldrinka,   ldrink,  belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Lucky Drink (set 2)",          MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 2004, merryjn,   0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Merry Joiner",                 MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

GAME( 200?, unkbel,    0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "unknown Belatra slot machine", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
