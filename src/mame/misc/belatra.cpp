// license:BSD-3-Clause
// copyright-holders: David Haywood

// Belatra Russian Fruit Machines (Video?)

/*

SoC is suspected to be an ARM7500 or similar.

TODO:
- just a skeleton and everything is complete guesswork.
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
	map(0x03200000, 0x032001ff).m(m_iomd, FUNC(arm7500fe_iomd_device::map)); // TODO: writes to some unimplemented registers
	//map(0x03340000, 0x03340003).r;
	//map(0x03400000, 0x037fffff).w(m_vidc, FUNC(arm_vidc20_device::write));
	//map(0x08000000, 0x0800000f).r;
	map(0x10000000, 0x13ffffff).ram();
	map(0x14000000, 0x17ffffff).ram();
}

static INPUT_PORTS_START( belatra )
INPUT_PORTS_END

void belatra_state::belatra(machine_config &config)
{
	ARM7500(config, m_maincpu, 54'000'000); // CPU type and clock guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &belatra_state::program_map);

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	ARM_VIDC20(config, m_vidc, 24'000'000); // chip type and clock guessed
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_iomd, FUNC(arm_iomd_device::vblank_irq));
	m_vidc->sound_drq().set(m_iomd, FUNC(arm_iomd_device::sound_drq));

	ARM7500FE_IOMD(config, m_iomd, 54'000'000); // CPU type and clock guessed
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

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
	// unknown sound
}


ROM_START( merryjn )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "am29f160bd_mery_up.bin",   0x000000, 0x200000, CRC(475161a5) SHA1(784904ffb949e56cac1126eaacf09db19a51b2f0) )
	ROM_LOAD( "am29f160bd_mery_down.bin", 0x200000, 0x200000, CRC(f8b6354c) SHA1(6de6e0c7927d2d673ac3fd5dc1c8263a73dcbb14) )
ROM_END

ROM_START( fairyl2 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "zfl2-1belatra.bin", 0x000000, 0x200000, CRC(cb0f3eba) SHA1(a7776810cfe037c25c196bbe900e5e17a2005d2d) )
	ROM_LOAD( "zfl2-2belatra.bin", 0x200000, 0x200000, CRC(755fad4b) SHA1(12243fdf95fcdd9012d1bbde6a18abb00918f560) )

	ROM_REGION( 0x400000, "others", ROMREGION_ERASEFF )
	ROM_LOAD( "at90s2313_fl2.bin", 0x0000, 0x000800, CRC(38e2d37e) SHA1(78178cb3ea219a71d1f15ffde722f9c03ad64dda) )
ROM_END

ROM_START( fairyl2a )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fairy_land2_up_original.bin", 0x000000, 0x200000, CRC(e9c24e75) SHA1(c80acfdcab4ed098596cd3f92a38086881344bb2) )
	ROM_LOAD( "fairy_land2_dw_original.bin", 0x200000, 0x200000, CRC(9d07818f) SHA1(8427b264ef03379cd6f3fbe4f5e141837ffef6fe) )
ROM_END

ROM_START( fairyl2b )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "zfl2-1  china.bin", 0x000000, 0x200000, CRC(3bfd7a0f) SHA1(e67c3fa27e5945de23efb8cb9899bcb23808e4cf) )
	ROM_LOAD( "zfl2-2  china.bin", 0x200000, 0x200000, CRC(1e1e172a) SHA1(6fb2eec95947ec7995f9c3c52f26b2c6dd5fd6b9) )
ROM_END

ROM_START( fairyl2bl )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fairy_land2_up_pirate.bin", 0x000000, 0x200000, CRC(e9c24e75) SHA1(c80acfdcab4ed098596cd3f92a38086881344bb2) )
	ROM_LOAD( "fairy_land2_dw_pirate.bin", 0x200000, 0x200000, CRC(00bf73d0) SHA1(3fa81815b4bbb5ed4dcd64afa1ad799a5a0bd48e) )
ROM_END

ROM_START( ldrink )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "lucky_up.bin",   0x000000, 0x200000, CRC(5d5277cc) SHA1(893369a58ea3ba3c7ba82a4cc4b1ffdbcc6745e2) )
	ROM_LOAD( "lucky_down.bin", 0x200000, 0x200000, CRC(e4c5b32a) SHA1(c8c44a7e35bbc88ba271566f8324b71818aa223c) )
ROM_END

ROM_START( ldrinka )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "zld-0012534-1belatra.bin", 0x000000, 0x200000, CRC(7a9b1e9f) SHA1(d9c7551b812bda509ba5ade09d2f82a77a5215d3) )
	ROM_LOAD( "zld-0012534-2belatra.bin", 0x200000, 0x200000, CRC(e3e74597) SHA1(0157dbf806eb2e5f0c9b9e8ed68d38748a713a6e) )
ROM_END

} // anonymous namespace


GAME( 2004, fairyl2,   0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 1)",   MACHINE_IS_SKELETON )
GAME( 2004, fairyl2a,  fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 2)",   MACHINE_IS_SKELETON )
GAME( 2004, fairyl2b,  fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (set 3)",   MACHINE_IS_SKELETON )
GAME( 2004, fairyl2bl, fairyl2, belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Fairy Land 2 (bootleg)", MACHINE_IS_SKELETON )

GAME( 2004, ldrink,    0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Lucky Drink (set 1)",    MACHINE_IS_SKELETON )
GAME( 2004, ldrinka,   ldrink,  belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Lucky Drink (set 2)",    MACHINE_IS_SKELETON )

GAME( 2004, merryjn,   0,       belatra, belatra, belatra_state, empty_init, ROT0, "Belatra", "Merry Joiner",           MACHINE_IS_SKELETON )
