// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Sergey Svishchev
/***************************************************************************

BK driver by Miodrag Milanovic

2008-03-10 Preliminary driver.


- BK0010 - error message at start - thrown into monitor. H for help.
  Can use M to load a tape and S to run it.

- BK001001 - can load its own recordings, but cannot proceed past the header
  of software-list items.
  However...if you enter the Monitor with MON, enter M, enter the filename
  (case sensitive), start tape - it loads. Use S to run it.

- BK0010FD - can boot ANDOS.  Use S160000 in Monitor to run it.

- BK0011M - 128KB RAM, clock speed 4MHz by default, floppy drive facility.

****************************************************************************/


#include "emu.h"
#include "bk.h"

#include "formats/rk_cas.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


/* Address maps */
void bk_state::bk0010_mem(address_map &map)
{
	map(0000000, 0177777).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w));

	map(0000000, 0037777).ram();
	map(0040000, 0077777).ram().share(m_ram[0]);
	map(0100000, 0177577).rom().region("maincpu", 0);
	map(0177660, 0177663).r(m_kbd, FUNC(k1801vp014_device::read));
	map(0177660, 0177661).w(m_kbd, FUNC(k1801vp014_device::write));
	map(0177664, 0177665).rw(FUNC(bk_state::vid_scroll_r), FUNC(bk_state::vid_scroll_w));
	map(0177706, 0177713).rw(m_timer, FUNC(k1801vm1_timer_device::read), FUNC(k1801vm1_timer_device::write));
	map(0177714, 0177715).rw(m_up, FUNC(bk_parallel_slot_device::read), FUNC(bk_parallel_slot_device::write));
	map(0177716, 0177717).rw(FUNC(bk_state::sel1_r), FUNC(bk_state::sel1_w));
}

void bk_state::bk0010fd_mem(address_map &map)
{
	bk0010_mem(map);
	map(0120000, 0157777).ram();
}

void bk_state::bk0011_mem(address_map &map)
{
	map(0000000, 0177777).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w));

	map(0000000, 0037777).ram().share(m_ram[0]);

	map(0040000, 0077777).view(m_view1);
	m_view1[6](0040000, 0077777).ram().share(m_ram[0]);
	m_view1[0](0040000, 0077777).ram().share(m_ram[1]);
	m_view1[2](0040000, 0077777).ram().share(m_ram[2]);
	m_view1[3](0040000, 0077777).ram().share(m_ram[3]);
	m_view1[4](0040000, 0077777).ram().share(m_ram[4]);
	m_view1[1](0040000, 0077777).ram().share(m_ram[5]);
	m_view1[7](0040000, 0077777).ram().share(m_ram[6]);
	m_view1[5](0040000, 0077777).ram().share(m_ram[7]);

	map(0100000, 0137777).view(m_view2);
	m_view2[6](0100000, 0137777).ram().share(m_ram[0]);
	m_view2[0](0100000, 0137777).ram().share(m_ram[1]);
	m_view2[2](0100000, 0137777).ram().share(m_ram[2]);
	m_view2[3](0100000, 0137777).ram().share(m_ram[3]);
	m_view2[4](0100000, 0137777).ram().share(m_ram[4]);
	m_view2[1](0100000, 0137777).ram().share(m_ram[5]);
	m_view2[7](0100000, 0137777).ram().share(m_ram[6]);
	m_view2[5](0100000, 0137777).ram().share(m_ram[7]);
	m_view2[8](0100000, 0137777).rom().region("maincpu", 0); // BASIC (chips 2 and 3)
	m_view2[9](0100000, 0137777).rom().region("maincpu", 040000); // BASIC (chip 1) and EXT
	m_view2[10](0100000, 0127777).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w)); // ROM3
	m_view2[11](0100000, 0127777).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w)); // ROM4

	map(0140000, 0157777).rom().region("maincpu", 0100000);

	map(0177660, 0177663).r(m_kbd, FUNC(k1801vp014_device::read));
	map(0177660, 0177661).w(m_kbd, FUNC(k1801vp014_device::write));
	map(0177662, 0177663).lw16(NAME([this](off_t offset, uint16_t data) {
		m_misc = data >> 8;
		m_stop_disabled = BIT(data, 15);
	}));
	map(0177664, 0177665).rw(FUNC(bk_state::vid_scroll_r), FUNC(bk_state::vid_scroll_w));
	map(0177706, 0177713).rw(m_timer, FUNC(k1801vm1_timer_device::read), FUNC(k1801vm1_timer_device::write));
	map(0177714, 0177715).rw(m_up, FUNC(bk_parallel_slot_device::read), FUNC(bk_parallel_slot_device::write));
	map(0177716, 0177717).rw(FUNC(bk_state::bk11_sel1_r), FUNC(bk_state::bk11_sel1_w));
}

void bk_state::bk0011m_mem(address_map &map)
{
	bk0011_mem(map);
	map(0177662, 0177663).lw16(NAME([this](off_t offset, uint16_t data) {
		m_misc = data >> 8;
		m_video_page = BIT(data, 15);
	}));
	map(0177716, 0177717).rw(FUNC(bk_state::bk11_sel1_r), FUNC(bk_state::bk11m_sel1_w));
}

/* Input ports */
static INPUT_PORTS_START( bk0010 )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Monitor type") PORT_WRITE_LINE_MEMBER(FUNC(bk_state::update_monitor))
	PORT_CONFSETTING(0x00, "B&W")
	PORT_CONFSETTING(0x01, "Color")
INPUT_PORTS_END

void bk_state::screen_vblank(int state)
{
	if (!BIT(m_misc, 6))
		m_maincpu->set_input_line(t11_device::CP2_LINE, state);
}

static const z80_daisy_config daisy_chain[] =
{
	{ "keyboard" },
	{ "qbus" },
	{ nullptr }
};

void bk_state::bk0010(machine_config &config)
{
	/* basic machine hardware */
	K1801VM1(config, m_maincpu, 3000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bk_state::bk0010_mem);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(bk_state::reset_w));

	K1801VM1_TIMER(config, m_timer, 3000000);

	K1801VP014(config, m_kbd, 0);
	m_kbd->virq_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_kbd->keydown_wr_callback().set(
			[this] (int state)
			{
				if (state)
					m_sel1 &= ~SEL1_KEYDOWN;
				else
					m_sel1 |= SEL1_KEYDOWN;
			});
	m_kbd->halt_wr_callback().set_inputline(m_maincpu, t11_device::HLT_LINE);

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":2", qbus_cards, nullptr); // some cards have passthrough

	BK_PARALLEL_SLOT(config, m_up, 0, bk_parallel_devices, nullptr);
	m_up->irq2_handler().set_inputline(m_maincpu, t11_device::CP2_LINE);
	m_up->irq3_handler().set_inputline(m_maincpu, t11_device::CP3_LINE);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(4000000.0/81920.0);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256); // 768x320 raster, 12 mhz pixel clock
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(bk_state::screen_update_bk10));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(bk_state::bk0010_palette), 5);

	// built-in piezo
	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("bk0010_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("bk0010");
	QUICKLOAD(config, "quickload", "bin", attotime::from_seconds(6)).set_load_callback(FUNC(bk_state::quickload_cb));
}

void bk_state::bk0010fd(machine_config &config)
{
	bk0010(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &bk_state::bk0010fd_mem);

	subdevice<qbus_slot_device>("qbus:1")->set_default_option("by");
}

void bk_state::bk0011(machine_config &config)
{
	bk0010(config);
	K1801VM1(config.replace(), m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bk_state::bk0011_mem);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(bk_state::reset_w));

	K1801VM1_TIMER(config.replace(), m_timer, 4000000);

	m_kbd->halt_wr_callback().set([this] (int state) {
		if (!m_stop_disabled) m_maincpu->set_input_line(t11_device::HLT_LINE, state);
	});

	subdevice<qbus_slot_device>("qbus:1")->set_default_option("by");
	subdevice<qbus_slot_device>("qbus:1")->set_option_default_bios("by", "253");

	/* video hardware */
	config.device_remove("screen");
	config.device_remove("palette");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(4000000.0/81920.0);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(4010)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(bk_state::screen_update_bk11));
	screen.screen_vblank().set(FUNC(bk_state::screen_vblank));

	TIMER(config, "scantimer").configure_scanline(FUNC(bk_state::scanline_callback_bk11), "screen", 0, 1);

	PALETTE(config, m_palette, FUNC(bk_state::bk0011_palette), 64);
}

void bk_state::bk0011m(machine_config &config)
{
	bk0011(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &bk_state::bk0011m_mem);
	subdevice<qbus_slot_device>("qbus:1")->set_option_default_bios("by", "326");
}

/* ROM definition */

ROM_START( bk0010 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom", 0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) )  // to 8000, mask 017
	ROM_LOAD( "focal.rom",   0x2000, 0x2000, CRC(717149b7) SHA1(75df26f81ebd281bcb5c55ba81a7d97f31e388b2) )  // to A000, mask 018 (socketed)
	ROM_LOAD( "tests.rom",   0x6000, 0x1f80, CRC(91aecb4d) SHA1(6b14d552045194a3004bb6b795a2538110921519) )  // to E000, mask 019 (socketed)
ROM_END

ROM_START( bk001001 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom",   0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) ) // to 8000, mask 017
	ROM_LOAD( "basic10-1.rom", 0x2000, 0x2000, CRC(5e3ff5da) SHA1(5ea4db1eaac87bd0ac96e52a608bc783709f5042) ) // to A000, mask 106
	ROM_LOAD( "basic10-2.rom", 0x4000, 0x2000, CRC(ea63863c) SHA1(acf068925e4052989b05dd5cf736a1dab5438011) ) // to C000, mask 107
	ROM_LOAD( "basic10-3.rom", 0x6000, 0x1f80, CRC(63f3df2e) SHA1(b5463f08e7c5f9f5aa31a2e7b6c1ed94fe029d65) ) // to E000, mask 108
ROM_END

ROM_START( bk0010fd )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom",  0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) ) // to 8000
	ROM_LOAD( "focal.rom",    0x2000, 0x2000, CRC(717149b7) SHA1(75df26f81ebd281bcb5c55ba81a7d97f31e388b2) ) // to A000
ROM_END

ROM_START( bk0011 )
	ROM_REGION( 0xb000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bk11_199_basic2.rom", 0x0000, 0x2000, CRC(34273976) SHA1(7c9418107da967e440c8ac9f29c3341851069cd0) ) // rom page 8
	ROM_LOAD( "bk11_200_basic3.rom", 0x2000, 0x2000, CRC(d29731bc) SHA1(53bbe868677456c3a942a21c686d19988c3a36ad) ) // rom page 8
	ROM_LOAD( "bk11_198_basic1.rom", 0x4000, 0x2000, CRC(4992c478) SHA1(2a967cb48ff084d65912512a0549ff660f64ab6a) ) // rom page 9
	ROM_LOAD( "bk11_202_ext.rom", 0x6000, 0x2000, CRC(1a0b6b59) SHA1(74cbc66b5c9376876c9a791448c43ff26f2eb9b4) ) // rom page 9
	ROM_LOAD( "bk11_201_bos.rom", 0x8000, 0x2000, CRC(6d927f81) SHA1(4ae5510cd4bd53b7f1ad994863af631794be813e) ) // always at 140000
ROM_END

ROM_START( bk0011m )
	ROM_REGION( 0xb000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bk11m_328_basic2.rom", 0x0000, 0x2000, CRC(deb0b29f) SHA1(cb89c5a451b6713fc4e932773004860e5222f531) ) // rom page 8
	ROM_LOAD( "bk11m_329_basic3.rom", 0x2000, 0x2000, CRC(bb645dfb) SHA1(986f9d70610a0031368bb4ef40d82760103d2e15) ) // rom page 8
	ROM_LOAD( "bk11m_327_basic1.rom", 0x4000, 0x2000, CRC(4c68ff59) SHA1(34fa37599f2f9eb607390ef2458a3c22d87f09a9) ) // rom page 9
	ROM_LOAD( "bk11m_325_ext.rom", 0x6000, 0x2000, CRC(b2e4c33c) SHA1(f087af69044432a1ef2431a72ac06946e32f2dd3) ) // rom page 9
	ROM_LOAD( "bk11m_324_bos.rom", 0x8000, 0x2000, CRC(8ffd5efa) SHA1(7e9a30e38d7b78981999821640a68a201bb6df01) ) // always at 140000
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS     INIT        COMPANY        FULLNAME       FLAGS */
COMP( 1985, bk0010,   0,      0,      bk0010,   bk0010, bk_state, empty_init, "Elektronika", "BK 0010",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1987, bk001001, bk0010, 0,      bk0010,   bk0010, bk_state, empty_init, "Elektronika", "BK 0010.01",  MACHINE_SUPPORTS_SAVE )
COMP( 1987, bk0010fd, bk0010, 0,      bk0010fd, bk0010, bk_state, empty_init, "Elektronika", "BK 0010 FDD", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1989, bk0011,   bk0010, 0,      bk0011,   bk0010, bk_state, empty_init, "Elektronika", "BK 0011",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1990, bk0011m,  bk0010, 0,      bk0011m,  bk0010, bk_state, empty_init, "Elektronika", "BK 0011M",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
