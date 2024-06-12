// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

BK driver by Miodrag Milanovic

2008-03-10 Preliminary driver.


TODO:
- Tape motor on/off to be emulated.

- BK0010 - error message at start - thrown into monitor. H for help.
  Can use M to load a tape. How to run it?

- BK001001 - can load its own recordings, but cannot proceed past the header
  of software-list items.
  However...if you enter the Monitor with MON, enter M, enter the filename
  (case sensitive), start tape - it loads. But then, how to run it?

- BK0010FD - continually reboots. The system expects a rom at A000. Can be
  patched by writing 0087 to address A000 in the debugger at start.

- BK0011M - black screen. No emulation of this variant has been done.
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
	map(0x0000, 0xffff).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w));
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0x8000, 0xfeff).rom().region("maincpu",0);
	map(0xffb0, 0xffb3).r(m_kbd, FUNC(k1801vp014_device::read));
	map(0xffb0, 0xffb1).w(m_kbd, FUNC(k1801vp014_device::write));
	map(0xffb4, 0xffb5).rw(FUNC(bk_state::vid_scroll_r), FUNC(bk_state::vid_scroll_w));
	map(0xffcc, 0xffcd).noprw();
	map(0xffce, 0xffcf).rw(FUNC(bk_state::sel1_r), FUNC(bk_state::sel1_w));
}

void bk_state::bk0010fd_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(bk_state::trap_r), FUNC(bk_state::trap_w));
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x7fff).ram().share("videoram");
	map(0x8000, 0x9fff).rom().region("maincpu",0);
	map(0xa000, 0xdfff).ram();
	map(0xe000, 0xfdff).rom().region("maincpu",0x6000);
	map(0xfe58, 0xfe59).rw(FUNC(bk_state::floppy_cmd_r), FUNC(bk_state::floppy_cmd_w));
	map(0xfe5a, 0xfe5b).rw(FUNC(bk_state::floppy_data_r), FUNC(bk_state::floppy_data_w));
	map(0xffb0, 0xffb3).r(m_kbd, FUNC(k1801vp014_device::read));
	map(0xffb0, 0xffb1).w(m_kbd, FUNC(k1801vp014_device::write));
	map(0xffb4, 0xffb5).rw(FUNC(bk_state::vid_scroll_r), FUNC(bk_state::vid_scroll_w));
	map(0xffcc, 0xffcd).noprw();
	map(0xffce, 0xffcf).rw(FUNC(bk_state::sel1_r), FUNC(bk_state::sel1_w));
}

/* Input ports */
static INPUT_PORTS_START( bk0010 )
INPUT_PORTS_END

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

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);

	K1801VP014(config, m_kbd, 0);
	m_kbd->virq_wr_callback().set_inputline(m_maincpu, t11_device::VEC_LINE);
	m_kbd->keydown_wr_callback().set([this] (int state) {
		m_sel1 |= SEL1_UPDATED;
		if (state) m_sel1 &= ~SEL1_KEYDOWN; else m_sel1 |= SEL1_KEYDOWN;
	});
	m_kbd->halt_wr_callback().set([this] (int state) {
		m_maincpu->set_input_line(t11_device::HLT_LINE, state);
	});

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(512, 256);
	screen.set_visarea(0, 512-1, 0, 256-1);
	screen.set_screen_update(FUNC(bk_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_DISABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("bk0010_cass");

	SOFTWARE_LIST(config, "cass_list").set_original("bk0010");
}

void bk_state::bk0010fd(machine_config &config)
{
	bk0010(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &bk_state::bk0010fd_mem);
}


/* ROM definition */

ROM_START( bk0010 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom", 0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) )  // to 8000
	ROM_LOAD( "focal.rom",   0x2000, 0x2000, CRC(717149b7) SHA1(75df26f81ebd281bcb5c55ba81a7d97f31e388b2) )  // to A000
	ROM_LOAD( "tests.rom",   0x6000, 0x1f80, CRC(91aecb4d) SHA1(6b14d552045194a3004bb6b795a2538110921519) )  // to E000
ROM_END

ROM_START( bk001001 )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom",   0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) ) // to 8000
	ROM_LOAD( "basic10-1.rom", 0x2000, 0x2000, CRC(5e3ff5da) SHA1(5ea4db1eaac87bd0ac96e52a608bc783709f5042) ) // to A000
	ROM_LOAD( "basic10-2.rom", 0x4000, 0x2000, CRC(ea63863c) SHA1(acf068925e4052989b05dd5cf736a1dab5438011) ) // to C000
	ROM_LOAD( "basic10-3.rom", 0x6000, 0x1f80, CRC(63f3df2e) SHA1(b5463f08e7c5f9f5aa31a2e7b6c1ed94fe029d65) ) // to E000
ROM_END

ROM_START( bk0010fd )
	ROM_REGION( 0x8000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "monit10.rom",  0x0000, 0x2000, CRC(26c6e8a0) SHA1(4e83a94ae5155bbea14d7331a5a8db82457bd5ae) ) // to 8000
	ROM_LOAD( "disk_327.rom", 0x6000, 0x1000, CRC(ed8a43ae) SHA1(28eefbb63047b26e4aec104aeeca74e2f9d0276c) ) // to E000
ROM_END

ROM_START( bk0011m )
	ROM_REGION( 0x1b000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bas11m_0.rom", 0x10000, 0x4000, CRC(e71f9339) SHA1(9d76f3eefd64e032c763fa1ebf9cd3d9bd22317a) )
	ROM_LOAD( "bas11m_1.rom", 0x14000, 0x2000, CRC(4c68ff59) SHA1(34fa37599f2f9eb607390ef2458a3c22d87f09a9) )
	ROM_LOAD( "b11m_ext.rom", 0x16000, 0x2000, CRC(b2e4c33c) SHA1(f087af69044432a1ef2431a72ac06946e32f2dd3) )
	ROM_LOAD( "b11m_bos.rom", 0x18000, 0x2000, CRC(8ffd5efa) SHA1(7e9a30e38d7b78981999821640a68a201bb6df01) )
	ROM_LOAD( "disk_327.rom", 0x1A000, 0x1000, CRC(ed8a43ae) SHA1(28eefbb63047b26e4aec104aeeca74e2f9d0276c) )
ROM_END

/* Driver */

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS     INIT        COMPANY        FULLNAME       FLAGS */
COMP( 1985, bk0010,   0,      0,      bk0010,   bk0010, bk_state, empty_init, "Elektronika", "BK 0010",     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1986, bk001001, bk0010, 0,      bk0010,   bk0010, bk_state, empty_init, "Elektronika", "BK 0010-01",  MACHINE_SUPPORTS_SAVE )
COMP( 1986, bk0010fd, bk0010, 0,      bk0010fd, bk0010, bk_state, empty_init, "Elektronika", "BK 0010 FDD", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1986, bk0011m,  bk0010, 0,      bk0010fd, bk0010, bk_state, empty_init, "Elektronika", "BK 0011M",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
