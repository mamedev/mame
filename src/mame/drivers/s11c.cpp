// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*
    Williams System 11c
*/

#include "emu.h"
#include "includes/s11c.h"

#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "speaker.h"

#include "s11c.lh"


void s11c_state::s11c_main_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x2100, 0x2103).mirror(0x00fc).rw(m_pia21, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // sound+solenoids
	map(0x2200, 0x2200).mirror(0x01ff).w(FUNC(s11c_state::sol3_w)); // solenoids
	map(0x2400, 0x2403).mirror(0x03fc).rw(m_pia24, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // lamps
	map(0x2800, 0x2803).mirror(0x03fc).rw(m_pia28, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // display
	map(0x2c00, 0x2c03).mirror(0x03fc).rw(m_pia2c, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // alphanumeric display
	map(0x3000, 0x3003).mirror(0x03fc).rw(m_pia30, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // inputs
	map(0x3400, 0x3403).mirror(0x0bfc).rw(m_pia34, FUNC(pia6821_device::read), FUNC(pia6821_device::write)); // widget
	map(0x4000, 0xffff).rom();
}

void s11c_state::s11c_audio_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x0800).ram();
	map(0x1000, 0x1fff).w(FUNC(s11c_state::bank_w));
	map(0x2000, 0x2003).mirror(0x0ffc).rw("pias", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xbfff).bankr("bank0");
	map(0xc000, 0xffff).bankr("bank1");
}

void s11c_state::s11c_bg_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();
	map(0x2000, 0x2001).mirror(0x1ffe).rw("ym2151", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x4000, 0x4003).mirror(0x1ffc).rw("pia40", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x67ff).w(FUNC(s11c_state::bg_speech_digit_w));
	map(0x6800, 0x6fff).w(FUNC(s11c_state::bg_speech_clock_w));
	map(0x7800, 0x7fff).w(FUNC(s11c_state::bgbank_w));
	map(0x8000, 0xffff).bankr("bgbank");
}

static INPUT_PORTS_START( s11c )
	PORT_START("X0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("X10")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_DOWN)

	PORT_START("X20")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("X40")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("X80")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAGS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audio Diag") PORT_CODE(KEYCODE_1_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11c_state, audio_nmi, 1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Main Diag") PORT_CODE(KEYCODE_4_PAD) PORT_CHANGED_MEMBER(DEVICE_SELF, s11c_state, main_nmi, 1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Advance") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Up/Down") PORT_CODE(KEYCODE_6_PAD) PORT_TOGGLE
	PORT_CONFNAME( 0x10, 0x10, "Language" )
	PORT_CONFSETTING( 0x00, "German" )
	PORT_CONFSETTING( 0x10, "English" )
INPUT_PORTS_END

/*
WRITE8_MEMBER( s11c_state::bgbank_w )
{
    uint8_t bank = ((data & 0x04) >> 2) | ((data & 0x03) << 1);
    membank("bgbank")->set_entry(bank);
//  popmessage("BG bank set to %02x (%i)",data,bank);
}
*/
void s11c_state::machine_reset()
{
//  membank("bgbank")->set_entry(0);
	// reset the CPUs again, so that the CPUs are starting with the right vectors (otherwise sound may die on reset)
//  m_bgcpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void s11c_state::init_s11c()
{
	emu_timer* timer = timer_alloc(TIMER_IRQ);
//  uint8_t *BGROM = memregion("bgcpu")->base();
//  membank("bgbank")->configure_entries(0, 8, &BGROM[0x10000], 0x8000);
//  membank("bgbank")->set_entry(0);
	set_invert(true);
	set_timer(timer);
	timer->adjust(attotime::from_ticks(S11_IRQ_CYCLES,E_CLOCK),1);
}

void s11c_state::s11c(machine_config &config)
{
	/* basic machine hardware */
	M6808(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &s11c_state::s11c_main_map);

	/* Video */
	config.set_default_layout(layout_s11c);

	/* Sound */
	genpin_audio(config);

	/* Devices */
	PIA6821(config, m_pia21, 0);
	m_pia21->readpa_handler().set(FUNC(s11_state::sound_r));
	m_pia21->writepa_handler().set(FUNC(s11_state::sound_w));
	m_pia21->writepb_handler().set(FUNC(s11_state::sol2_w));
	m_pia21->ca2_handler().set(FUNC(s11_state::pia21_ca2_w));
	m_pia21->cb2_handler().set(FUNC(s11_state::pia21_cb2_w));
	m_pia21->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia21->irqb_handler().set(FUNC(s11_state::pia_irq));

	PIA6821(config, m_pia24, 0);
	m_pia24->writepa_handler().set(FUNC(s11_state::lamp0_w));
	m_pia24->writepb_handler().set(FUNC(s11_state::lamp1_w));
	m_pia24->cb2_handler().set(FUNC(s11_state::pia24_cb2_w));
	m_pia24->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia24->irqb_handler().set(FUNC(s11_state::pia_irq));

	PIA6821(config, m_pia28, 0);
	m_pia28->readpa_handler().set(FUNC(s11_state::pia28_w7_r));
	m_pia28->writepa_handler().set(FUNC(s11a_state::dig0_w));
	m_pia28->writepb_handler().set(FUNC(s11b_state::dig1_w));
	m_pia28->ca2_handler().set(FUNC(s11_state::pia28_ca2_w));
	m_pia28->cb2_handler().set(FUNC(s11_state::pia28_cb2_w));
	m_pia28->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia28->irqb_handler().set(FUNC(s11_state::pia_irq));

	PIA6821(config, m_pia2c, 0);
	m_pia2c->writepa_handler().set(FUNC(s11b_state::pia2c_pa_w));
	m_pia2c->writepb_handler().set(FUNC(s11b_state::pia2c_pb_w));
	m_pia2c->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia2c->irqb_handler().set(FUNC(s11_state::pia_irq));

	PIA6821(config, m_pia30, 0);
	m_pia30->readpa_handler().set(FUNC(s11_state::switch_r));
	m_pia30->writepb_handler().set(FUNC(s11_state::switch_w));
	m_pia30->cb2_handler().set(FUNC(s11_state::pia30_cb2_w));
	m_pia30->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia30->irqb_handler().set(FUNC(s11_state::pia_irq));

	PIA6821(config, m_pia34, 0);
	m_pia34->writepa_handler().set(FUNC(s11b_state::pia34_pa_w));
	m_pia34->writepb_handler().set(FUNC(s11b_state::pia34_pb_w));
	m_pia34->cb2_handler().set(FUNC(s11b_state::pia34_cb2_w));
	m_pia34->irqa_handler().set(FUNC(s11_state::pia_irq));
	m_pia34->irqb_handler().set(FUNC(s11_state::pia_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// generic sound board is not used in System 11C, except for Star Trax

	/* Add the background music card */
	SPEAKER(config, "speaker").front_center();
	S11C_BG(config, m_bg);
	m_bg->set_romregion("bgcpu");
	m_bg->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

/*--------------------
/ Bugs Bunny Birthday Ball 11/90
/--------------------*/
ROM_START(bbnny_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bugs_u26.l2", 0x4000, 0x4000, CRC(b4358920) SHA1(93af1cf5dc2b5442f428a621c0f73b27c197a3df))
	ROM_LOAD("bugs_u27.l2", 0x8000, 0x8000, CRC(8ff29439) SHA1(8fcdcea556e9e01ea8cb7c1548f98af2467c8a5f))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("bugs_u4.l2", 0x10000, 0x10000, CRC(04bc9aa5) SHA1(c3da2dc3e26b88a0ebc6f87e61fc71bec45330c3))
	ROM_LOAD("bugs_u19.l1", 0x20000, 0x10000, CRC(a2084702) SHA1(ffd749387e7b52bad1e98c6a8939fb87bc67524c))
	ROM_LOAD("bugs_u20.l1", 0x30000, 0x10000, CRC(5df734ef) SHA1(c8d153444dd6171c3ebddc8100ab06fde3373cc6))
ROM_END

ROM_START(bbnny_lu)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("bugs_u26.l2", 0x4000, 0x4000, CRC(b4358920) SHA1(93af1cf5dc2b5442f428a621c0f73b27c197a3df))
	ROM_LOAD("u27-lu2.rom", 0x8000, 0x8000, CRC(aaa2c82d) SHA1(b279c87cb2ac90a818eeb1afa6115b8cdab1b0df))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("bugs_u4.l2", 0x10000, 0x10000, CRC(04bc9aa5) SHA1(c3da2dc3e26b88a0ebc6f87e61fc71bec45330c3))
	ROM_LOAD("bugs_u19.l1", 0x20000, 0x10000, CRC(a2084702) SHA1(ffd749387e7b52bad1e98c6a8939fb87bc67524c))
	ROM_LOAD("bugs_u20.l1", 0x30000, 0x10000, CRC(5df734ef) SHA1(c8d153444dd6171c3ebddc8100ab06fde3373cc6))
ROM_END

/*--------------------
/ Diner 8/90
/--------------------*/
ROM_START(diner_l4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinr_u26.l4", 0x4000, 0x4000, CRC(6f187abf) SHA1(8acabbccdf3528a9c5e60cc8939ab960bf4c5512))
	ROM_LOAD("dinr_u27.l4", 0x8000, 0x8000, CRC(d69f9f74) SHA1(88d9b42c2313a90e5d6f50220d3b44331595d86b))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

ROM_START(diner_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinr_u26.l2", 0x4000, 0x4000, CRC(8b6aa22e) SHA1(6b802a85fc2babf5a183fb434df11597363c1c9d))
	ROM_LOAD("u27-la3.rom", 0x8000, 0x8000, CRC(4171451a) SHA1(818e330245691d9ef3181b885c9342880f89d912))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

ROM_START(diner_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinr_u26.l2", 0x4000, 0x4000, CRC(8b6aa22e) SHA1(6b802a85fc2babf5a183fb434df11597363c1c9d))
	ROM_LOAD("dinr_u27.lu2", 0x8000, 0x8000, CRC(ea72f6aa) SHA1(58df02e8353dd9be2ecfbcdc78fc54981dd001e1))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

ROM_START(diner_f2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinr_u26.l2", 0x4000, 0x4000, CRC(8b6aa22e) SHA1(6b802a85fc2babf5a183fb434df11597363c1c9d))
	ROM_LOAD("dinr_u27.lf2", 0x8000, 0x8000, CRC(343af291) SHA1(ecf30dd828537cd68dc6c9f97256a38c820b9afc))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

ROM_START(diner_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-lu1.rom", 0x4000, 0x4000, CRC(259b302f) SHA1(d7e19c2d2ad7805d9158178c24d180d158a59b0c))
	ROM_LOAD("u27-lu1.rom", 0x8000, 0x8000, CRC(35fafbb3) SHA1(0db3d0c9421f4fdcf4d376d543626559e1bf2daa))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

ROM_START(diner_p0)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinr_u26.pa0", 0x4000, 0x4000, CRC(dbf5de09) SHA1(55ca30bdf6e6e482f3ce5891b866103492fa3754))
	ROM_LOAD("dinr_u27.pa0", 0x8000, 0x8000, CRC(9875d16e) SHA1(fbbab61cc9e8e3f7a1bf1428ad50ad3c13f0f120))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dinr_u4.l1", 0x10000, 0x10000, CRC(3bd28368) SHA1(41eec2f5f863039deaabfae8aece4b1cf15e4b78))
	ROM_LOAD("dinr_u19.l1", 0x20000, 0x10000, CRC(278b9a30) SHA1(41e59adb8b6c08caee46c3dd73256480b4041619))
	ROM_LOAD("dinr_u20.l1", 0x30000, 0x10000, CRC(511fb260) SHA1(e6e25b464c5c38f3c0492436f1e8aa2be33dd278))
ROM_END

/*--------------------
/ Dr. Dude 11/90
/--------------------*/
ROM_START(dd_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dude_u26.lu1", 0x4000, 0x4000, CRC(6f6a6e22) SHA1(2d8a1b472eb06a9f7aeea4b2f9a82f83eb4ee08a))
	ROM_LOAD("dude_u27.lu1", 0x8000, 0x8000, CRC(43c2d4f3) SHA1(d61d855fa06394bdc1142e21624bdaac1ee9ca20))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dude_u4.l1", 0x10000, 0x10000, CRC(3eeef714) SHA1(74dcc83958cb62819e0ac36ca83001694faafec7))
	ROM_LOAD("dude_u19.l1", 0x20000, 0x10000, CRC(dc7b985b) SHA1(f672d1f1fe1d1d887113ea6ccd745a78f7760526))
	ROM_LOAD("dude_u20.l1", 0x30000, 0x10000, CRC(a83d53dd) SHA1(92a81069c42c7760888201fb0787fa7ddfbf1658))
ROM_END

ROM_START(dd_lu1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dude_u26.l2", 0x4000, 0x4000, CRC(d1e19fc2) SHA1(800329b5fd563fcd27add14da4522082c01eb86e))
	ROM_LOAD("dude_u27.l2", 0x8000, 0x8000, CRC(654b5d4c) SHA1(e73834dbb35cf78eab68a5966e4049640e16dddf))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dude_u4.l1", 0x10000, 0x10000, CRC(3eeef714) SHA1(74dcc83958cb62819e0ac36ca83001694faafec7))
	ROM_LOAD("dude_u19.l1", 0x20000, 0x10000, CRC(dc7b985b) SHA1(f672d1f1fe1d1d887113ea6ccd745a78f7760526))
	ROM_LOAD("dude_u20.l1", 0x30000, 0x10000, CRC(a83d53dd) SHA1(92a81069c42c7760888201fb0787fa7ddfbf1658))
ROM_END

ROM_START(dd_p6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u26-pa6.11c", 0x4000, 0x4000, CRC(6f6a6e22) SHA1(2d8a1b472eb06a9f7aeea4b2f9a82f83eb4ee08a))
	ROM_LOAD("u27-pa6.11c", 0x8000, 0x8000, CRC(26022273) SHA1(ca66139c3bd0c313d41a396c484d2c1b8f4ae536))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("dude_u4.l1", 0x10000, 0x10000, CRC(3eeef714) SHA1(74dcc83958cb62819e0ac36ca83001694faafec7))
	ROM_LOAD("dude_u19.l1", 0x20000, 0x10000, CRC(dc7b985b) SHA1(f672d1f1fe1d1d887113ea6ccd745a78f7760526))
	ROM_LOAD("dude_u20.l1", 0x30000, 0x10000, CRC(a83d53dd) SHA1(92a81069c42c7760888201fb0787fa7ddfbf1658))
ROM_END

/*--------------------
/ Pool Sharks 6/90
/--------------------*/
ROM_START(pool_l7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pool_u26.l7", 0x4000, 0x4000, CRC(cee98aed) SHA1(5b652684c10ab4945783089d848b2f663d3b2547))
	ROM_LOAD("pool_u27.l7", 0x8000, 0x8000, CRC(356d9a89) SHA1(ce795c535d03a14d28fb3f2071cae48ccdb1a856))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pool_u4.l2", 0x10000, 0x10000, CRC(04e95e10) SHA1(3873b3cd6c2961b3f2f28a1e17f8a63c6db808d2))
	ROM_LOAD("pool_u19.l2", 0x20000, 0x10000, CRC(0f45d02b) SHA1(58bbfdb3b98c43b66e11808cec7cd65a7f2dce6d))
	ROM_LOAD("pool_u20.l2", 0x30000, 0x10000, CRC(925f62d6) SHA1(21b8d6f9a8b98fce8a3cdf7f5f2d40200544a898))
ROM_END

ROM_START(pool_l6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pool_u26.la6", 0x4000, 0x4000, CRC(fec70d5a) SHA1(bc155a590f64f2b43b8799c1a6d2336dde45a10c))
	ROM_LOAD("pool_u27.la6", 0x8000, 0x8000, CRC(91fb5231) SHA1(538ddc66a5885e4b7a840d35a1e62b92f73b39ad))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pool_u4.l2", 0x10000, 0x10000, CRC(04e95e10) SHA1(3873b3cd6c2961b3f2f28a1e17f8a63c6db808d2))
	ROM_LOAD("pool_u19.l2", 0x20000, 0x10000, CRC(0f45d02b) SHA1(58bbfdb3b98c43b66e11808cec7cd65a7f2dce6d))
	ROM_LOAD("pool_u20.l2", 0x30000, 0x10000, CRC(925f62d6) SHA1(21b8d6f9a8b98fce8a3cdf7f5f2d40200544a898))
ROM_END

ROM_START(pool_l5)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pool_u26.l5", 0x4000, 0x4000, CRC(373d18d9) SHA1(320d72777380971c16fe59cb1ed2314dec76705d))
	ROM_LOAD("pool_u27.l5", 0x8000, 0x8000, CRC(1a2f0717) SHA1(cb8a7a62e0aac721ce68ae0bb1157b92edda62cb))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pool_u4.l2", 0x10000, 0x10000, CRC(04e95e10) SHA1(3873b3cd6c2961b3f2f28a1e17f8a63c6db808d2))
	ROM_LOAD("pool_u19.l2", 0x20000, 0x10000, CRC(0f45d02b) SHA1(58bbfdb3b98c43b66e11808cec7cd65a7f2dce6d))
	ROM_LOAD("pool_u20.l2", 0x30000, 0x10000, CRC(925f62d6) SHA1(21b8d6f9a8b98fce8a3cdf7f5f2d40200544a898))
ROM_END

ROM_START(pool_le2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pool_u26.le2", 0x4000, 0x4000, CRC(70526965) SHA1(69c7b74fff8fcc351e8bd9b8fce7655aebf7205c))
	ROM_LOAD("pool_u27.le2", 0x8000, 0x8000, CRC(90911f02) SHA1(ef3d32b3c5bafcd886bbde8b897cb225c0c04464))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pool_u4.l2", 0x10000, 0x10000, CRC(04e95e10) SHA1(3873b3cd6c2961b3f2f28a1e17f8a63c6db808d2))
	ROM_LOAD("pool_u19.l2", 0x20000, 0x10000, CRC(0f45d02b) SHA1(58bbfdb3b98c43b66e11808cec7cd65a7f2dce6d))
	ROM_LOAD("pool_u20.l2", 0x30000, 0x10000, CRC(925f62d6) SHA1(21b8d6f9a8b98fce8a3cdf7f5f2d40200544a898))
ROM_END

ROM_START(pool_p7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("pool_u26.pa7", 0x4000, 0x4000, CRC(91d2aae9) SHA1(b3dbc80809bca8b0435c3d8a0ceb504acab4a04a))
	ROM_LOAD("pool_u27.pa7", 0x8000, 0x8000, CRC(eff6b940) SHA1(e0c3858803bbc5cacedefe57681efc2dc339e16d))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pool_u4.l2", 0x10000, 0x10000, CRC(04e95e10) SHA1(3873b3cd6c2961b3f2f28a1e17f8a63c6db808d2))
	ROM_LOAD("pool_u19.l2", 0x20000, 0x10000, CRC(0f45d02b) SHA1(58bbfdb3b98c43b66e11808cec7cd65a7f2dce6d))
	ROM_LOAD("pool_u20.l2", 0x30000, 0x10000, CRC(925f62d6) SHA1(21b8d6f9a8b98fce8a3cdf7f5f2d40200544a898))
ROM_END
/*--------------------
/ Radical 9/90
/--------------------*/
ROM_START(radcl_l1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rad_u26.l1", 0x4000, 0x4000, CRC(84b1a125) SHA1(dd01fb9189acd2620c57149921aadb051f7a2412))
	ROM_LOAD("rad_u27.l1", 0x8000, 0x8000, CRC(6f6ca382) SHA1(a61055aab97d3fe2ecd0ed4281a9681b1d910269))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rad_u4.l1", 0x10000, 0x10000, CRC(5aafc09c) SHA1(27984bbc91dc7593e6a5b42f74dd6ddf58189bec))
	ROM_LOAD("rad_u19.l1", 0x20000, 0x10000, CRC(7c005e1f) SHA1(bdeea7517f2adf72b4b642bffb25ba5b98453127))
	ROM_LOAD("rad_u20.l1", 0x30000, 0x8000, CRC(05b96292) SHA1(7da0289cf0a0c93768c0706fdedfc3a5f2101e77))
	ROM_RELOAD(0x38000, 0x8000)
ROM_END

ROM_START(radcl_g1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rad_u26.l1", 0x4000, 0x4000, CRC(84b1a125) SHA1(dd01fb9189acd2620c57149921aadb051f7a2412))
	ROM_LOAD("u27-lg1.rom", 0x8000, 0x8000, CRC(4f2eca4b) SHA1(ff44deded1686cfa0351c4499485d6eb4561cbc1))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rad_u4.l1", 0x10000, 0x10000, CRC(5aafc09c) SHA1(27984bbc91dc7593e6a5b42f74dd6ddf58189bec))
	ROM_LOAD("rad_u19.l1", 0x20000, 0x10000, CRC(7c005e1f) SHA1(bdeea7517f2adf72b4b642bffb25ba5b98453127))
	ROM_LOAD("rad_u20.l1", 0x30000, 0x8000, CRC(05b96292) SHA1(7da0289cf0a0c93768c0706fdedfc3a5f2101e77))
	ROM_RELOAD(0x38000, 0x8000)
ROM_END

ROM_START(radcl_p3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rad_u26.p1", 0x4000, 0x4000, CRC(7d736ae9) SHA1(4ea6945fa5cfbd33fcdf780814b0bf5cb3faa388))
	ROM_LOAD("u27-p1.rom", 0x8000, 0x8000, CRC(83b1d928) SHA1(b1bd5d8a93f1ab9fb9bf5c268d8530be438448e6))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rad_u4.p3", 0x10000, 0x10000, CRC(d31b7744) SHA1(7ebcc1503fc322909d32c7c8bda8c0b6505919b3))
	ROM_LOAD("rad_u19.l1", 0x20000, 0x10000, CRC(7c005e1f) SHA1(bdeea7517f2adf72b4b642bffb25ba5b98453127))
	ROM_LOAD("rad_u20.p3", 0x30000, 0x8000, CRC(82f8369c) SHA1(0691a80672fc11d46359f710bd211de7a59de346))
	ROM_RELOAD(0x38000, 0x8000)
ROM_END

/*--------------------
/ Riverboat Gambler 10/90
/--------------------*/
ROM_START(rvrbt_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamb_u26.l3", 0x4000, 0x4000, CRC(a65f6004) SHA1(ea44bb7f8f2ec9e5989be63ba41f674b14d19b8a))
	ROM_LOAD("gamb_u27.l3", 0x8000, 0x8000, CRC(9be0f613) SHA1(1c2b442bc3daef212fe23ff03f5409c354e79989))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamb_u4.l2", 0x10000, 0x10000, CRC(c0cfa9be) SHA1(352df9a4dcbc131ae249416e9e517137a04627ba))
	ROM_LOAD("gamb_u19.l1", 0x20000, 0x10000, CRC(04a3a8c8) SHA1(e72ef767f13282d2335cda3288037610d9bfedf2))
	ROM_LOAD("gamb_u20.l1", 0x30000, 0x10000, CRC(a60c734d) SHA1(76cfcf96276ca4f6b5eee0e0402fab5ee9685366))
ROM_END

ROM_START(rvrbt_p7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gamb_u26.pa7", 0x4000, 0x4000, CRC(594db647) SHA1(914d10755c2a1609d3945554fdd8006f7d294287))
	ROM_LOAD("gamb_u27.pa7", 0x8000, 0x8000, CRC(696248c4) SHA1(892ea48121165792d10d64dcfc188e4a100e6371))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gamb_u4.l2", 0x10000, 0x10000, CRC(c0cfa9be) SHA1(352df9a4dcbc131ae249416e9e517137a04627ba))
	ROM_LOAD("gamb_u19.l1", 0x20000, 0x10000, CRC(04a3a8c8) SHA1(e72ef767f13282d2335cda3288037610d9bfedf2))
	ROM_LOAD("gamb_u20.l1", 0x30000, 0x10000, CRC(a60c734d) SHA1(76cfcf96276ca4f6b5eee0e0402fab5ee9685366))
ROM_END

/*--------------------
/ Rollergames 5/90
/--------------------*/
ROM_START(rollr_l2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr_u26.l2", 0x4000, 0x4000, CRC(cd7cad9e) SHA1(e381fa73895c307a0b3b4b699cfec2a68908f6f7))
	ROM_LOAD("rolr_u27.l2", 0x8000, 0x8000, CRC(f3bac2b8) SHA1(9f0ff32ea83e43097de42065909137a362b29d49))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_ex)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.ea3", 0x4000, 0x4000, CRC(78c3c1ad) SHA1(04e4370548b3ba85c49634402a0ea166e3643f68))
	ROM_LOAD("rolr_u27.ea3", 0x8000, 0x8000, CRC(18685158) SHA1(d1a79fbe1185fb9e1ae1d9e2b2751429f487bb4c))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_e1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr_u26.pe1", 0x4000, 0x4000, CRC(56620505) SHA1(2df9097e52178f246148a40e0ad4a6e6a5cdb5d4))
	ROM_LOAD("rolr_u27.pe1", 0x8000, 0x8000, CRC(724d0af2) SHA1(5de5596f4e594c0e6b8448817de6ff46ffc7194b))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.pe1", 0x10000, 0x10000, CRC(8c383b24) SHA1(5c738e5ec566f7fa5706cd4c33e5d706fa76c72d))
	ROM_LOAD("rolr_u19.pe1", 0x20000, 0x10000, CRC(c6880cff) SHA1(c8ce23d68297d36ef62e508855a478434ff9a592))
	ROM_LOAD("rolr_u20.pe1", 0x30000, 0x10000, CRC(4220812b) SHA1(7071565f1087020d1e1738e801dafb509ea37622))
ROM_END

ROM_START(rollr_p2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr_u26.pa2", 0x4000, 0x4000, CRC(11d96b1c) SHA1(e96991bdef8b14043285feeb4cacc182a6e9dcbd))
	ROM_LOAD("rolr_u27.pa2", 0x8000, 0x8000, CRC(ee547bd5) SHA1(db45bf7a25321ac041f58404f7512bded9ebf11e))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.pa1", 0x10000, 0x10000, CRC(324df946) SHA1(e7ba2b9434baea20a0cf38540fdab1668c058539))
	ROM_LOAD("rolr_u19.pa1", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.pa1", 0x30000, 0x10000, CRC(8ddaaad1) SHA1(33f58c6a9b0e509b7c9a460a687d6e2c388b4b54))
ROM_END

ROM_START(rollr_l3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.lu3", 0x4000, 0x4000, CRC(7d71ed50) SHA1(092aa13706a7fe58ad80e88c1c4a5c1d7d712546))
	ROM_LOAD("rolr_u27.l2", 0x8000, 0x8000, CRC(f3bac2b8) SHA1(9f0ff32ea83e43097de42065909137a362b29d49))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_g3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.lg3", 0x4000, 0x4000, CRC(438d2b94) SHA1(f507a06794563701b6d4fc51ff90a42a6d21d060))
	ROM_LOAD("rolr_u27.l2", 0x8000, 0x8000, CRC(f3bac2b8) SHA1(9f0ff32ea83e43097de42065909137a362b29d49))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_f2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.lf2", 0x4000, 0x4000, CRC(a33a3a39) SHA1(fa67cede5fe9f86ce8772e49dba8d929d2b53ecb))
	ROM_LOAD("rolr_u27.l2", 0x8000, 0x8000, CRC(f3bac2b8) SHA1(9f0ff32ea83e43097de42065909137a362b29d49))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_f3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.lf3", 0x4000, 0x4000, CRC(0c54de2a) SHA1(764a0986bb1966ddfd547ee6380debaaa1ea3769))
	ROM_LOAD("rolr_u27.l2", 0x8000, 0x8000, CRC(f3bac2b8) SHA1(9f0ff32ea83e43097de42065909137a362b29d49))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

ROM_START(rollr_d2) // American Drops 2 - sample/prototype with 8 drop targets
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("rolr-u26.ad2", 0x4000, 0x4000, CRC(913574ac) SHA1(52e2b398d087e998ba5c16a623c7d1a02b99ca55))
	ROM_LOAD("rolr_u27.ad2", 0x8000, 0x8000, CRC(92d2172f) SHA1(73f15d338f2680ee2dd961e9e4d4ea97e9328b6e))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("rolr_u4.l3", 0x10000, 0x10000, CRC(d366c705) SHA1(76018305b5040b2e5d8c45cc81a18f13e1a8f8da))
	ROM_LOAD("rolr_u19.l3", 0x20000, 0x10000, CRC(45a89e55) SHA1(3aff897514d242c83a8e7575d430d594a873736e))
	ROM_LOAD("rolr_u20.l3", 0x30000, 0x10000, CRC(77f89aff) SHA1(dcd9fe233f33ef8f97cdeaaa365532e485a28944))
ROM_END

/*--------------------
/ The Bally Game Show 4/90
/--------------------*/
ROM_START(gs_lu4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gshw_u26.l3", 0x4000, 0x4000, CRC(3419bfb2) SHA1(7ce294a3118d20c7cdc3d5cd946e4c43090c5151))
	ROM_LOAD("u27-lu4.rom", 0x8000, 0x8000, CRC(ba265978) SHA1(66ac8e83e35cdfd72f1d3aa8ce6d92c2c833f304))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gshw_u4.l2", 0x10000, 0x10000, CRC(e89e0116) SHA1(e96bee143d1662d078f21531f405d838fdace693))
	ROM_LOAD("gshw_u19.l1", 0x20000, 0x10000, CRC(8bae0813) SHA1(a2b1beca13796892d8ee1533e395cabdbbb11f88))
	ROM_LOAD("gshw_u20.l1", 0x30000, 0x10000, CRC(75ccbdf7) SHA1(7dce8ae427a621919caad8d8b08b06bb0adad850))
ROM_END

ROM_START(gs_lu3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gameshow_u26-lu3.rom", 0x4000, 0x4000, CRC(4fa15683) SHA1(789bac55090a54d5b5180a0ef49c3903238f407f))
	ROM_LOAD("gameshow_u27-lu3.rom", 0x8000, 0x8000, CRC(22270bd2) SHA1(a24a5e199179fd7c7ae8cd1b24c1d21b2fadf8da))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gshw_u4.l2", 0x10000, 0x10000, CRC(e89e0116) SHA1(e96bee143d1662d078f21531f405d838fdace693))
	ROM_LOAD("gshw_u19.l1", 0x20000, 0x10000, CRC(8bae0813) SHA1(a2b1beca13796892d8ee1533e395cabdbbb11f88))
	ROM_LOAD("gshw_u20.l1", 0x30000, 0x10000, CRC(75ccbdf7) SHA1(7dce8ae427a621919caad8d8b08b06bb0adad850))
ROM_END

ROM_START(gs_la3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gshw_u26.l3", 0x4000, 0x4000, CRC(3419bfb2) SHA1(7ce294a3118d20c7cdc3d5cd946e4c43090c5151))
	ROM_LOAD("gshw_u27.l3", 0x8000, 0x8000, CRC(4f3babb6) SHA1(87091a6786fc6817529cfed7f60396babe153d8d))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gshw_u4.l2", 0x10000, 0x10000, CRC(e89e0116) SHA1(e96bee143d1662d078f21531f405d838fdace693))
	ROM_LOAD("gshw_u19.l1", 0x20000, 0x10000, CRC(8bae0813) SHA1(a2b1beca13796892d8ee1533e395cabdbbb11f88))
	ROM_LOAD("gshw_u20.l1", 0x30000, 0x10000, CRC(75ccbdf7) SHA1(7dce8ae427a621919caad8d8b08b06bb0adad850))
ROM_END

ROM_START(gs_lg6)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("gshw_u26.l3", 0x4000, 0x4000, CRC(3419bfb2) SHA1(7ce294a3118d20c7cdc3d5cd946e4c43090c5151))
	ROM_LOAD("u27-lg6.bin", 0x8000, 0x8000, CRC(02c75b24) SHA1(29bc0404c8598393cb1d0f17fb157e84f3b9a794))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("gshw_u4.l2", 0x10000, 0x10000, CRC(e89e0116) SHA1(e96bee143d1662d078f21531f405d838fdace693))
	ROM_LOAD("gshw_u19.l1", 0x20000, 0x10000, CRC(8bae0813) SHA1(a2b1beca13796892d8ee1533e395cabdbbb11f88))
	ROM_LOAD("gshw_u20.l1", 0x30000, 0x10000, CRC(75ccbdf7) SHA1(7dce8ae427a621919caad8d8b08b06bb0adad850))
ROM_END

/*-----------------------
/ Star Trax 9/90
/-----------------------*/
ROM_START(strax_p7)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("strx_u26.p7", 0x4000, 0x4000, CRC(0d2a401c) SHA1(b0a0899dcde04dc42e4fd5d6baf39bb0e81dbb34))
	ROM_LOAD("strx_u27.p7", 0x8000, 0x8000, CRC(6e9c0632) SHA1(5c0ea2b60dd9001b802d2ecdb5c381ab05f08ec9))
	ROM_REGION(0x10000, "cpu3", ROMREGION_ERASEFF)  // TODO: add generic sound board hardware
	ROM_REGION(0x10000, "sound2", 0)
	ROM_LOAD("strx_u21.l1", 0x0000, 0x8000, CRC(6a323227) SHA1(7c7263754e5672c654a2ee9582f0b278e637a909))
	ROM_LOAD("strx_u22.l1", 0x8000, 0x8000, CRC(58407eb4) SHA1(6bd9b304c88d9470eae5afb6621187f4a8313573))
	ROM_REGION(0x50000, "bgcpu", ROMREGION_ERASEFF)
	ROM_LOAD("pfrc_u4.l2", 0x10000, 0x8000, CRC(8f431529) SHA1(0f479990715a31fd860c000a066cffb70da502c2))
	ROM_RELOAD(0x18000,0x8000)
	ROM_LOAD("pfrc_u19.l1", 0x20000, 0x8000, CRC(abc4caeb) SHA1(6faef2de9a49a1015b4038ab18849de2f25dbded))
	ROM_RELOAD(0x28000,0x8000)
ROM_END


GAME(1990,  bbnny_l2,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Bugs Bunny Birthday Ball (L-2)",               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  bbnny_lu,   bbnny_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Bugs Bunny Birthday Ball (LU-2) European",     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_l4,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (LA-4)",                                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_l3,   diner_l4,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (LA-3)",                                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_l2,   diner_l4,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (LU-2) Europe",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_f2,   diner_l4,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (LF-2) French",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_l1,   diner_l4,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (LU-1) Europe",                          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  diner_p0,   diner_l4,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Diner (PA-0 prototype)",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  dd_l2,      0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Dr. Dude (LA-2)",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  dd_lu1,     dd_l2,      s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Dr. Dude (LU-1) Europe",                       MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  dd_p6,      dd_l2,      s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Dr. Dude (PA-6)",                              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  pool_l7,    0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Pool Sharks (LA-7)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  pool_l6,    pool_l7,    s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Pool Sharks (LA-6)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  pool_l5,    pool_l7,    s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Pool Sharks (LA-5)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  pool_le2,   pool_l7,    s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Pool Sharks (LE-2)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1989,  pool_p7,    pool_l7,    s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Pool Sharks (PA-7)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  radcl_l1,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Radical! (L-1)",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  radcl_g1,   radcl_l1,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Radical! (G-1)",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  radcl_p3,   radcl_l1,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "Radical! (P-3)",                               MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rvrbt_l3,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Riverboat Gambler (L-3)",                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rvrbt_p7,   rvrbt_l3,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Riverboat Gambler (PA-7)",                     MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rollr_l2,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (L-2)",                            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  rollr_ex,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (EXPERIMENTAL)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  rollr_e1,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (PU-1)",                           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  rollr_p2,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (PA-2 / PA-1 Sound)",              MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rollr_l3,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (LU-3) Europe",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rollr_g3,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (LG-3) Germany",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1991,  rollr_f2,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (LF-2) French",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rollr_f3,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (LF-3) French",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  rollr_d2,   rollr_l2,   s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Rollergames (AD-2) Prototype",                 MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  gs_lu4,     0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "The Bally Game Show (LU-4) Europe",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  gs_lu3,     gs_lu4,     s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "The Bally Game Show (LU-3) Europe",            MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  gs_la3,     gs_lu4,     s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "The Bally Game Show (LA-3)",                   MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  gs_lg6,     gs_lu4,     s11c,   s11c, s11c_state, init_s11c, ROT0,   "Bally",                "The Bally Game Show (LG-6) Germany",           MACHINE_IS_SKELETON_MECHANICAL)
GAME(1990,  strax_p7,   0,          s11c,   s11c, s11c_state, init_s11c, ROT0,   "Williams",             "Star Trax (domestic prototype)",               MACHINE_IS_SKELETON_MECHANICAL)
