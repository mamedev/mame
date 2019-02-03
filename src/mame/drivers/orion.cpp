// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Orion driver by Miodrag Milanovic

        22/04/2008 Orion Pro added
        02/04/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/orion.h"

#include "imagedev/cassette.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "formats/smx_dsk.h"
#include "formats/rk_cas.h"


/* Address maps */

/* Orion 128 */
void orion_state::orion128_mem(address_map &map)
{
	map(0x0000, 0xefff).bankrw("bank1");
	map(0xf000, 0xf3ff).bankrw("bank2");
	map(0xf400, 0xf4ff).rw(FUNC(orion_state::orion128_system_r), FUNC(orion_state::orion128_system_w));  // Keyboard and cassette
	map(0xf500, 0xf5ff).rw(FUNC(orion_state::orion128_romdisk_r), FUNC(orion_state::orion128_romdisk_w));
	map(0xf700, 0xf7ff).rw(FUNC(orion_state::orion128_floppy_r), FUNC(orion_state::orion128_floppy_w));
	map(0xf800, 0xffff).rom();
	map(0xf800, 0xf8ff).w(FUNC(orion_state::orion128_video_mode_w));
	map(0xf900, 0xf9ff).w(FUNC(orion_state::orion128_memory_page_w));
	map(0xfa00, 0xfaff).w(FUNC(orion_state::orion128_video_page_w));
}

/* Orion Z80 Card II */
void orion_state::orion128_io(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();
	map(0xf8, 0xf8).w(FUNC(orion_state::orion128_video_mode_w));
	map(0xf9, 0xf9).w(FUNC(orion_state::orion128_memory_page_w));
	map(0xfa, 0xfa).w(FUNC(orion_state::orion128_video_page_w));
}

void orion_state::orionz80_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankrw("bank1");
	map(0x4000, 0xefff).bankrw("bank2");
	map(0xf000, 0xf3ff).bankrw("bank3");
	map(0xf400, 0xf7ff).bankrw("bank4");
	map(0xf800, 0xffff).bankrw("bank5");
}

/* Orion Pro */
void orion_state::orionz80_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(orion_state::orionz80_io_r), FUNC(orion_state::orionz80_io_w));
}

void orion_state::orionpro_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).bankrw("bank1");
	map(0x2000, 0x3fff).bankrw("bank2");
	map(0x4000, 0x7fff).bankrw("bank3");
	map(0x8000, 0xbfff).bankrw("bank4");
	map(0xc000, 0xefff).bankrw("bank5");
	map(0xf000, 0xf3ff).bankrw("bank6");
	map(0xf400, 0xf7ff).bankrw("bank7");
	map(0xf800, 0xffff).bankrw("bank8");
}

void orion_state::orionpro_io(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(orion_state::orionpro_io_r), FUNC(orion_state::orionpro_io_w));
}

FLOPPY_FORMATS_MEMBER( orion_state::orion_floppy_formats )
	FLOPPY_SMX_FORMAT
FLOPPY_FORMATS_END

/* Machine driver */
void orion_state::orion128(machine_config &config)
{
	I8080(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &orion_state::orion128_mem);
	m_maincpu->set_addrmap(AS_IO, &orion_state::orion128_io);

	auto &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set(FUNC(orion_state::orion_romdisk_porta_r));
	ppi1.out_pb_callback().set(FUNC(orion_state::orion_romdisk_portb_w));
	ppi1.out_pc_callback().set(FUNC(orion_state::orion_romdisk_portc_w));

	auto &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(radio86_state::radio86_8255_porta_w2));
	ppi2.in_pb_callback().set(FUNC(radio86_state::radio86_8255_portb_r2));
	ppi2.in_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_r2));
	ppi2.out_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_w2));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(384, 256);
	m_screen->set_visarea(0, 384-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(orion_state::screen_update_orion128));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(orion_state::orion128_palette), 18);

	SPEAKER(config, "mono").front_center();
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);

	auto &cassette(CASSETTE(config, "cassette"));
	cassette.set_formats(rko_cassette_formats);
	cassette.set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	cassette.set_interface("orion_cass");

	SOFTWARE_LIST(config, "cass_list").set_type("orion_cass", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);

	FLOPPY_CONNECTOR(config, "fd0", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd2", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd3", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_type("orion_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	auto &cart(GENERIC_CARTSLOT(config, "cartslot"));
	generic_plain_slot(cart);
	cart.set_interface("orion_cart");

	SOFTWARE_LIST(config, "cart_list").set_type("orion_cart", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("256K");
	m_ram->set_default_value(0x00);
}

void orion_state::orion128ms(machine_config &config)
{
	orion128(config);
	auto &ppi2(I8255A(config.replace(), "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(radio86_state::radio86_8255_porta_w2));
	ppi2.in_pb_callback().set(FUNC(radio86_state::radio86_8255_portb_r2));
	ppi2.in_pc_callback().set(FUNC(radio86_state::rk7007_8255_portc_r));
	ppi2.out_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_w2));
}


void orion_z80_state::orionz80(machine_config &config)
{
	Z80(config, m_maincpu, 2500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &orion_z80_state::orionz80_mem);
	m_maincpu->set_addrmap(AS_IO, &orion_z80_state::orionz80_io);
	m_maincpu->set_vblank_int("screen", FUNC(orion_z80_state::orionz80_interrupt));

	auto &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set(FUNC(orion_z80_state::orion_romdisk_porta_r));
	ppi1.out_pb_callback().set(FUNC(orion_z80_state::orion_romdisk_portb_w));
	ppi1.out_pc_callback().set(FUNC(orion_z80_state::orion_romdisk_portc_w));

	auto &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(radio86_state::radio86_8255_porta_w2));
	ppi2.in_pb_callback().set(FUNC(radio86_state::radio86_8255_portb_r2));
	ppi2.in_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_r2));
	ppi2.out_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_w2));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(384, 256);
	m_screen->set_visarea(0, 384-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(orion_z80_state::screen_update_orion128));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(orion_z80_state::orion128_palette), 18);

	MC146818(config, "rtc", 4.194304_MHz_XTAL);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.0);
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);

	auto &ay8912(AY8912(config, "ay8912", 1773400));
	ay8912.add_route(ALL_OUTPUTS, "mono", 1.00);

	auto &cassette(CASSETTE(config, "cassette"));
	cassette.set_formats(rko_cassette_formats);
	cassette.set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	cassette.set_interface("orion_cass");

	SOFTWARE_LIST(config, "cass_list").set_type("orion_cass", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);

	FLOPPY_CONNECTOR(config, "fd0", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd2", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd3", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_type("orion_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	auto &cart(GENERIC_CARTSLOT(config, "cartslot"));
	generic_plain_slot(cart);
	cart.set_interface("orion_cart");

	SOFTWARE_LIST(config, "cart_list").set_type("orion_cart", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("512K");
	m_ram->set_default_value(0x00);
}

void orion_z80_state::orionz80ms(machine_config &config)
{
	orionz80(config);

	auto &ppi2(I8255A(config.replace(), "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(radio86_state::radio86_8255_porta_w2));
	ppi2.in_pb_callback().set(FUNC(radio86_state::radio86_8255_portb_r2));
	ppi2.in_pc_callback().set(FUNC(radio86_state::rk7007_8255_portc_r));
	ppi2.out_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_w2));
}

void orion_pro_state::orionpro(machine_config &config)
{
	Z80(config, m_maincpu, 5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &orion_pro_state::orionpro_mem);
	m_maincpu->set_addrmap(AS_IO, &orion_pro_state::orionpro_io);

	auto &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.in_pa_callback().set(FUNC(orion_pro_state::orion_romdisk_porta_r));
	ppi1.out_pb_callback().set(FUNC(orion_pro_state::orion_romdisk_portb_w));
	ppi1.out_pc_callback().set(FUNC(orion_pro_state::orion_romdisk_portc_w));

	auto &ppi2(I8255A(config, "ppi8255_2"));
	ppi2.out_pa_callback().set(FUNC(radio86_state::radio86_8255_porta_w2));
	ppi2.in_pb_callback().set(FUNC(radio86_state::radio86_8255_portb_r2));
	ppi2.in_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_r2));
	ppi2.out_pc_callback().set(FUNC(radio86_state::radio86_8255_portc_w2));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(384, 256);
	m_screen->set_visarea(0, 384-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(orion_pro_state::screen_update_orion128));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(orion_pro_state::orion128_palette), 18);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.0);
	WAVE(config, "wave", "cassette").add_route(ALL_OUTPUTS, "mono", 0.25);
	auto &ay8912(AY8912(config, "ay8912", 1773400));
	ay8912.add_route(ALL_OUTPUTS, "mono", 1.00);

	auto &cassette(CASSETTE(config, "cassette"));
	cassette.set_formats(rko_cassette_formats);
	cassette.set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	cassette.set_interface("orion_cass");

	SOFTWARE_LIST(config, "cass_list").set_type("orion_cass", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	FD1793(config, m_fdc, 8_MHz_XTAL / 8);

	FLOPPY_CONNECTOR(config, "fd0", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd1", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd2", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	FLOPPY_CONNECTOR(config, "fd3", "525qd", FLOPPY_525_QD, true, orion_floppy_formats);
	SOFTWARE_LIST(config, "flop_list").set_type("orion_flop", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	auto &cart(GENERIC_CARTSLOT(config, "cartslot"));
	generic_plain_slot(cart);
	cart.set_interface("orion_cart");

	SOFTWARE_LIST(config, "cart_list").set_type("orion_cart", SOFTWARE_LIST_ORIGINAL_SYSTEM);

	/* internal ram */
	RAM(config, m_ram);
	m_ram->set_default_size("512K");
	m_ram->set_default_value(0x00);
}

/* ROM definition */

ROM_START( orion128 )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "m2rk", "Version 3.2 rk" )
	ROMX_LOAD( "m2rk.bin",    0x0f800, 0x0800, CRC(2025c234) SHA1(caf86918629be951fe698cddcdf4589f07e2fb96), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "m2_2rk", "Version 3.2.2 rk" )
	ROMX_LOAD( "m2_2rk.bin",  0x0f800, 0x0800, CRC(fc662351) SHA1(7c6de67127fae5869281449de1c503597c0c058e), ROM_BIOS(1) )
ROM_END

ROM_START( orionms )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ms7007.bin",   0x0f800, 0x0800, CRC(c6174ba3) SHA1(8f9a42c3e09684718fe4121a8408e7860129d26f) )
ROM_END

ROM_START( orionz80 )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "m31", "Version 3.1" )
	ROMX_LOAD( "m31.bin",     0x0f800, 0x0800, CRC(007c6dc6) SHA1(338ff95497c820338f7f79c75f65bc540a5541c4), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "m32zrk", "Version 3.2 zrk" )
	ROMX_LOAD( "m32zrk.bin",  0x0f800, 0x0800, CRC(4ec3f012) SHA1(6b0b2bfc515a80e7caf72c3c33cf2dcf192d4711), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "m33zrkd", "Version 3.3 zrkd" )
	ROMX_LOAD( "m33zrkd.bin", 0x0f800, 0x0800, CRC(f404032d) SHA1(088cd9ed05f0dda4fa0a005c609208d9f57ad3d9), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "m34zrk", "Version 3.4 zrk" )
	ROMX_LOAD( "m34zrk.bin",  0x0f800, 0x0800, CRC(787c3903) SHA1(476c1c0b88e5efb582292eebec15e24d054c8851), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "m35zrkd", "Version 3.5 zrkd" )
	ROMX_LOAD( "m35zrkd.bin", 0x0f800, 0x0800, CRC(9368b38f) SHA1(64a77f22119d40c9b18b64d78ad12acc6fff9efb), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "peter", "Peterburg '91" )
	ROMX_LOAD( "peter.bin",   0x0f800, 0x0800, CRC(df9b1d8c) SHA1(c7f1e074e58ad1c1799cf522161b4f4cffa5aefa), ROM_BIOS(5) )
ROM_END

ROM_START( orionide )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m35zrkh.bin", 0x0f800, 0x0800, CRC(b7745f28) SHA1(c3bd3e662db7ec56ecbab54bf6b3a4c26200d0bb) )
ROM_END

ROM_START( orionzms )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "m32zms", "Version 3.2 zms" )
	ROMX_LOAD( "m32zms.bin",  0x0f800, 0x0800, CRC(44cfd2ae) SHA1(84d53fbc249938c56be76ee4e6ab297f0461835b), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "m34zms", "Version 3.4 zms" )
	ROMX_LOAD( "m34zms.bin",  0x0f800, 0x0800, CRC(0f87a80b) SHA1(ab1121092e61268d8162ed8a7d4fd081016a409a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "m35zmsd", "Version 3.5 zmsd" )
	ROMX_LOAD( "m35zmsd.bin", 0x0f800, 0x0800, CRC(f714ff37) SHA1(fbe9514adb3384aff146cbedd4fede37ce9591e1), ROM_BIOS(2) )
ROM_END

ROM_START( orionidm )
	ROM_REGION( 0x30000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "m35zmsh.bin", 0x0f800, 0x0800, CRC(01e66df4) SHA1(8c785a3c32fe3eacda73ec79157b41a6e4b63ba8) )
ROM_END

ROM_START( orionpro )
	ROM_REGION( 0x32000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "ver21", "Version 2.1" )
	ROMX_LOAD( "rom1-210.bin", 0x20000, 0x2000,  CRC(8e1a0c78) SHA1(61c8a5ed596ce7e3fd32da920dcc80dc5375b421), ROM_BIOS(0) )
	ROMX_LOAD( "rom2-210.bin", 0x22000, 0x10000, CRC(7cb7a49b) SHA1(601f3dd61db323407c4874fd7f23c10dccac0209), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "ver20", "Version 2.0" )
	ROMX_LOAD( "rom1-200.bin", 0x20000, 0x2000,  CRC(4fbe83cc) SHA1(9884d43770b4c0fbeb519b96618b01957c0b8511), ROM_BIOS(1) )
	ROMX_LOAD( "rom2-200.bin", 0x22000, 0x10000, CRC(618aaeb7) SHA1(3e7e5d3ff9d2c683708928558e69aa62db877811), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "ver10", "Version 1.0" )
	ROMX_LOAD( "rom1-100.bin", 0x20000, 0x2000, CRC(4fd6c408) SHA1(b0c2e4fb5be5a74a7efa9bba14b746865122af1d), ROM_BIOS(2) )
	ROMX_LOAD( "rom2-100.bin", 0x22000, 0x8000, CRC(370ffdca) SHA1(169e2acac2d0b382e2d0a144da0af18bfa38db5c), ROM_BIOS(2) )
ROM_END

/* Driver */

//    YEAR  NAME      PARENT    COMPAT  MACHINE     INPUT    CLASS            INIT        COMPANY      FULLNAME                                  FLAGS
COMP( 1990, orion128, 0,        0,      orion128,   radio86, orion_state,     empty_init, "<unknown>", "Orion 128",                              0 )
COMP( 1990, orionms,  orion128, 0,      orion128ms, ms7007,  orion_state,     empty_init, "<unknown>", "Orion 128 (MS7007)",                     0 )
COMP( 1990, orionz80, orion128, 0,      orionz80,   radio86, orion_z80_state, empty_init, "<unknown>", "Orion 128 + Z80 Card II",                0 )
COMP( 1990, orionide, orion128, 0,      orionz80,   radio86, orion_z80_state, empty_init, "<unknown>", "Orion 128 + Z80 Card II + IDE",          0 )
COMP( 1990, orionzms, orion128, 0,      orionz80ms, ms7007,  orion_z80_state, empty_init, "<unknown>", "Orion 128 + Z80 Card II (MS7007)",       0 )
COMP( 1990, orionidm, orion128, 0,      orionz80ms, ms7007,  orion_z80_state, empty_init, "<unknown>", "Orion 128 + Z80 Card II + IDE (MS7007)", 0 )
COMP( 1994, orionpro, orion128, 0,      orionpro,   radio86, orion_pro_state, empty_init, "<unknown>", "Orion Pro",                              0 )
