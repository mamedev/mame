// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/arm7/arm7.h"

#include "screen.h"
#include "softlist_dev.h"


namespace {

class accutime_smart_watch_state : public driver_device
{
public:
	accutime_smart_watch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void atw(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void arm_map(address_map &map) ATTR_COLD;
};

uint32_t accutime_smart_watch_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void accutime_smart_watch_state::machine_start()
{
}

void accutime_smart_watch_state::machine_reset()
{
}

static INPUT_PORTS_START( atw )
INPUT_PORTS_END

void accutime_smart_watch_state::arm_map(address_map &map)
{
}

void accutime_smart_watch_state::atw(machine_config &config)
{
	ARM7(config, m_maincpu, 72000000); // Mediatek ARM MT6260AH, ARM7EJ-STM core, unknown frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &accutime_smart_watch_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(accutime_smart_watch_state::screen_update));
}

ROM_START( atw_avbp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(d7a2f809) SHA1(34df7c533f49bd1149c9c5fb3137293dab1dbf3a) )
ROM_END

ROM_START( atw_barb )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gd25lq128d.bin", 0x000000, 0x1000000, CRC(fb1b3fb3) SHA1(ebb6e5a66b34978eb8ad334c899b00d7b25ea12b) )
ROM_END

ROM_START( atw_batm )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(898a3b44) SHA1(2dc225a421255a015136c334e57cdfb5916ed593) )
ROM_END

ROM_START( atw_dp )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(87d64677) SHA1(3ffaba2c83d27d77215b7a841efd174c31444e70) )
ROM_END

ROM_START( atw_enc )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(cddb7f7a) SHA1(c53e70dc54f1e719c4aaae61cdb55ca450866bea) )
ROM_END

ROM_START( atw_fz )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gd25lq128d.bin", 0x000000, 0x1000000, CRC(4db55d17) SHA1(405fc30c18bc0bc2306b8d071e92b3c31c7dcdc4) )
ROM_END

ROM_START( atw_fz2 )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25m4aa.bin", 0x000000, 0x1000000, CRC(30fe624d) SHA1(a62e399ad50a2850f6d94f452d65c4c522eb584c) )
ROM_END

ROM_START( atw_gaby )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "25q128.bin", 0x000000, 0x1000000, CRC(ee03466b) SHA1(4aa57b73db9559f4e37b9ebf7f2b68ba9b3d9966) )
ROM_END

ROM_START( atw_hpot )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(ca3d8ca1) SHA1(2b32964cadd451ba0e95d1ef66f601b33671994d) )
ROM_END

ROM_START( atw_jwld )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(1e1841e2) SHA1(936bb451f32507292f169a7efa5ed5001a1dc4f4) )
ROM_END

ROM_START( atw_lilo )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gd25lq128d.bin", 0x000000, 0x1000000, CRC(d5ce67c3) SHA1(438be7f47332cb6bc9559c3826604ccc3e28daaa) )
ROM_END

ROM_START( atw_lol )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25m4aa.bin", 0x000000, 0x1000000, CRC(02b8bbc9) SHA1(73580623a61efe6ab7292f61cba6c4cb79c39dbd) )
ROM_END

ROM_START( atw_lola )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(94d64707) SHA1(aaa75b7796d2aed928ce34d4d093005e4a378a49) )
ROM_END

ROM_START( atw_mine )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(2ee6ce30) SHA1(8ac80bb3c238a7e8127dbfb16c3d79f74cb2b15b) )
ROM_END

ROM_START( atw_mv )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25m4aa.bin", 0x000000, 0x1000000, CRC(b987ac0b) SHA1(350f0bfde4ef5b2a52dbb9bbfd25d4f05b710345) )
ROM_END

ROM_START( atw_mva )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25m4aa.bin", 0x000000, 0x1000000, CRC(85086fb5) SHA1(9f232a3c758f7f05e51bcfd65666427ddb488354) )
ROM_END

ROM_START( atw_paw )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(1d9db6a5) SHA1(0c1bae98db6e161e43ba7b8a9a0e115689f1c5bf) )
ROM_END

ROM_START( atw_pepa )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(39f5f804) SHA1(7144c66927f88e79027b7a029292c3ca488fde6d) )
ROM_END

ROM_START( atw_pok )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(fe8346c6) SHA1(72105180b5de02e747192bb4413892f4500fe420) )
ROM_END

ROM_START( atw_rbh )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gd25lq128d.bin", 0x000000, 0x1000000, CRC(86b28790) SHA1(170585f22e3440e7605bc0308789c1e1b9f34a09) )
ROM_END

ROM_START( atw_ryan )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(bf40ee2c) SHA1(47d16b7b9243d4b4a52522e2373c1a0225461d27) )
ROM_END

ROM_START( atw_soni )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jw.bin", 0x000000, 0x1000000, CRC(1d29c63c) SHA1(7e36b351dcbb8ecf0e5e709127db29e3a4779879) )
ROM_END

ROM_START( atw_spdm )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "fm25m4aa.bin", 0x000000, 0x1000000, CRC(65f2047a) SHA1(33537b1ec422b635cb6f4a4fcd6caaf8288774b1) )
ROM_END

ROM_START( atw_swst )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(fb8e83af) SHA1(f1babbecac1cb2270908f1da77d215a9c5b9f6cc) )
ROM_END

ROM_START( atw_swy )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(ab3128fd) SHA1(4bc3bf6946b6fbd7c0d783ab48eff5d37828a876) )
ROM_END

ROM_START( atw_toys )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "w25q128jwiq.bin", 0x000000, 0x1000000, CRC(a748c0f4) SHA1(fa21e16f85cc73109d2ac16286bc345d654e5b94) )
ROM_END

} // anonymous namespace

// these are all basically the same thing but with different wallpapers / alarm sounds etc.

// part name extensions
// PH = Peers Hardy UK
// AZ = ? (Amazon?)
// TK = ? (TK Maxx?)
//

// blue PCB
CONS( 2021, atw_barb, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Barbie (Accutime, BAB4064PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_batm, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Batman (Accutime, BAT4732PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_dp,   0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Disney Princess (Accutime, PN425?)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_enc,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Encanto (Accutime, ENC4000)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_fz,   0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Frozen (Accutime, FZN4151PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_fz2,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Frozen (Accutime, FZN4587)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_gaby, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Gabby's Dollhouse (Accutime, GAB4007PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_lilo, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Lilo & Stitch (Accutime, LS4027PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_lol,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: L.O.L. Surprise (Accutime, LOL4264PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_mine, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Minecraft (Accutime, MIN4045)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_mv,   0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Marvel Avengers (Accutime, AVG4597PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_mva,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Marvel Avengers (Accutime, AVG4597)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_paw,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Paw Patrol (Accutime, PAW4275PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_pok,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Pokemon (Accutime, POK4231PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_rbh,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Rainbow High (Accutime, RNB4017PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_ryan, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Ryan's World (Accutime, RYW4003AZ)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_soni, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Sonic the Hedgehog (Accutime, SNC4055)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_spdm, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Spider-Man (Accutime, SPD4588)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2021, atw_toys, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Toy Story (Accutime, LTY4000PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// black PCB
CONS( 2022, atw_avbp, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Marvel Avengers Black Panther (Accutime, AVG4608PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_hpot, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Harry Potter (Accutime, HP4107PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_jwld, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Jurassic World (Accutime, JRW4041)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_lola, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: L.O.L. Surprise (Accutime, LOL4588TK)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_pepa, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Peppa Pig (Accutime, PPG4086PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// red PCB
CONS( 2022, atw_swst, 0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Star Wars: Stormtropper (Accutime, STM4353PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
CONS( 2022, atw_swy,  0, 0, atw, atw, accutime_smart_watch_state, empty_init, "Accutime", "Kids Smart Watch: Star Wars: The Mandalorian (Accutime, MNL4023PH)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

