// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, Miodrag Milanovic
/*********************************************************************************************
PINBALL
Williams WPC Fliptronics I

The Addams Family (#20017)

Since NVRAM is not working, when it starts factory settings will be applied.
 Press F3, and wait for the game attract mode to commence.

To start, hold ABC hit 1.
To end the ball, Hold ABC until the bonuses have counted and the score starts flashing.

ToDo:
- NVRAM
- Outputs
- Mechanical sounds

*********************************************************************************************/
#include "emu.h"
#include "includes/wpc_dot.h"
#include "screen.h"
#include "speaker.h"


void wpc_flip1_state::wpc_flip1_map(address_map &map)
{
	map(0x0000, 0x2fff).rw(FUNC(wpc_flip1_state::ram_r), FUNC(wpc_flip1_state::ram_w));
	map(0x3000, 0x31ff).bankrw("dmdbank1");
	map(0x3200, 0x33ff).bankrw("dmdbank2");
	map(0x3400, 0x35ff).bankrw("dmdbank3");
	map(0x3600, 0x37ff).bankrw("dmdbank4");
	map(0x3800, 0x39ff).bankrw("dmdbank5");
	map(0x3a00, 0x3bff).bankrw("dmdbank6");
	map(0x3c00, 0x3faf).ram();
	map(0x3fb0, 0x3fff).rw(m_wpc, FUNC(wpc_device::read), FUNC(wpc_device::write)); // WPC device
	map(0x4000, 0x7fff).bankr("cpubank");
	map(0x8000, 0xffff).bankr("fixedbank");
}

static INPUT_PORTS_START( wpc_flip1 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("Left Trough")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("Centre Trough")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("Right Trough")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_TOGGLE PORT_NAME("Coin Door")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Ticket Dispenser")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD )  // always closed
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("Right Flipper Lane")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("Right Outlane")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("Ball Shooter")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("Upper Left Bumper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("Upper Right Bumper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("Centre Left Bumper")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("Centre Right Bumper") // manual shown Left by mistake, layout is correct
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("Lower Bumper")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("Left Sling")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("Right Sling")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("Upper Left Loop")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("Grave G")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("Grave R")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Chair")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("Cousin It")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Lower Swamp")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("Centre Swamp")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("Upper Swamp")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("Shooter Lane")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("Bookcase Opto 1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("Bookcase Opto 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Bookcase Opto3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("Bookcase Opto 4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("Bumper Lane Opto")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("Right Ramp Exit")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("Left Ramp Exit")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Train Wreck")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Thing Eject Lane")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("Right Ramp Enter")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("Right Ramp Top")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Left Ramp Top")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Upper Right Loop")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Vault")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("Swamp Lock Upper")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Swamp Lock Centre")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Swamp Lock Lower")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Lockup Kickout")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("Left Outlane")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("Left Flipper Lane 2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("Thing Kickout")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("Left Flipper Lane 1")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("Bookcase Open")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Bookcase Closed")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Thing Down Opto")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Thing Up Opto")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("Grave A")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("Thing Eject Hole")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Service / Escape") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_VOLUME_DOWN ) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_VOLUME_UP ) PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Begin Test / Enter") PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("FLIP")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right Flipper EOS") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Right Flipper Button") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left Flipper EOS") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Left Flipper Button") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Upper Right Flipper EOS") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Upper Right Flipper Button") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Upper Left Flipper EOS") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Upper Left Flipper Button") PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("DIPS")
	PORT_DIPNAME(0x01,0x01,"Switch 1") PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x01,DEF_STR( On ))
	PORT_DIPNAME(0x02,0x02,"Switch 2") PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x02,DEF_STR( On ))
	PORT_DIPNAME(0x04,0x00,"W20") PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x04,DEF_STR( On ))
	PORT_DIPNAME(0x08,0x00,"W19") PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(0x00,DEF_STR( Off ))
	PORT_DIPSETTING(0x08,DEF_STR( On ))
	PORT_DIPNAME(0xf0,0x00,"Country") PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(0x00,"USA 1")
	PORT_DIPSETTING(0x10,"France 1")
	PORT_DIPSETTING(0x20,"Germany")
	PORT_DIPSETTING(0x30,"France 2")
	PORT_DIPSETTING(0x40,"Unknown 1")
	PORT_DIPSETTING(0x50,"Unknown 2")
	PORT_DIPSETTING(0x60,"Unknown 3")
	PORT_DIPSETTING(0x70,"Unknown 4")
	PORT_DIPSETTING(0x80,"Export 1")
	PORT_DIPSETTING(0x90,"France 3")
	PORT_DIPSETTING(0xa0,"Export 2")
	PORT_DIPSETTING(0xb0,"France 4")
	PORT_DIPSETTING(0xc0,"UK")
	PORT_DIPSETTING(0xd0,"Europe")
	PORT_DIPSETTING(0xe0,"Spain")
	PORT_DIPSETTING(0xf0,"USA 2")
INPUT_PORTS_END

void wpc_flip1_state::init_wpc_flip1()
{
	wpc_dot_state::init_wpc_dot();
}

void wpc_flip1_state::wpc_flip1(machine_config &config)
{
	/* basic machine hardware */
	M6809(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &wpc_flip1_state::wpc_flip1_map);

	WPCASIC(config, m_wpc, 0);
	m_wpc->irq_callback().set(FUNC(wpc_flip1_state::wpc_irq_w));
	m_wpc->firq_callback().set(FUNC(wpc_flip1_state::wpc_firq_w));
	m_wpc->bank_write().set(FUNC(wpc_flip1_state::wpc_rombank_w));
	m_wpc->sound_ctrl_read().set(m_wpcsnd, FUNC(wpcsnd_device::ctrl_r));
	m_wpc->sound_ctrl_write().set(m_wpcsnd, FUNC(wpcsnd_device::ctrl_w));
	m_wpc->sound_data_read().set(m_wpcsnd, FUNC(wpcsnd_device::data_r));
	m_wpc->sound_data_write().set(m_wpcsnd, FUNC(wpcsnd_device::data_w));
	m_wpc->dmdbank_write().set(FUNC(wpc_flip1_state::wpc_dmdbank_w));

	SPEAKER(config, "speaker").front_center();
	WPCSND(config, m_wpcsnd);
	m_wpcsnd->set_romregion("sound1");
	m_wpcsnd->reply_callback().set(FUNC(wpc_flip1_state::wpcsnd_reply_w));
	m_wpcsnd->add_route(ALL_OUTPUTS, "speaker", 1.0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_native_aspect();
	screen.set_size(128, 32);
	screen.set_visarea(0, 128-1, 0, 32-1);
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(wpc_flip1_state::screen_update));
}

/*---------------------------
/  The Addams Family #20017
/---------------------------*/
ROM_START(taf_p2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("addam_p2.rom", 0x00000, 0x40000, CRC(eabf0e72) SHA1(5b84d0315702b39b90beb6a92fb7ad9aba7e620c))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("afsnd_p2.rom", 0x100000, 0x80000, CRC(73d19698) SHA1(d14a6ea36a93db185a599a7810dfbef2deb0adc0))
ROM_END

ROM_START(taf_l1)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("addam_l1.rom", 0x00000, 0x40000, CRC(db287bf7) SHA1(51574c7c04d85aa816a0bc6e9db74f2d2b407525))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("addam_l2.rom", 0x00000, 0x40000, CRC(952bfc92) SHA1(d95b4b9e6c496a9ce4ceb1aa368c862b2beeffd9))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l3)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("addam_l3.rom", 0x00000, 0x40000, CRC(d428a760) SHA1(29afee7b1ae64d7a41faf813cdfa1ab7cef1f247))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x40000, "code", 0)
	ROM_LOAD("addam_l4.rom", 0x00000, 0x40000, CRC(ea29935f) SHA1(9f711396728026546c8bd1f69a0833d15e02c192))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l7)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("addam_l7.rom", 0x00000, 0x80000, CRC(4401b43a) SHA1(64e9678334cc900d1f44b95d25bb90c1fff566f8))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l5)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("addam_l5.rom", 0x00000, 0x80000, CRC(4c071564) SHA1(d643506db1b3ba1ea20f34ddb38837df379fb5ab))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_l6)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("taf_l6.u6", 0x00000, 0x80000, CRC(06b37e65) SHA1(ce6f9cc45df08f50f5ece2a4c9376ecf67b0466a))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

ROM_START(taf_h4)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_REGION(0x80000, "code", 0)
	ROM_LOAD("addam_h4.rom", 0x00000, 0x80000, CRC(d0bbd679) SHA1(ebd8c4981dd68a4f8e2dea90144486cb3cbd6b84))
	ROM_REGION(0x180000, "sound1", 0)
	ROM_LOAD("tafu18l1.rom", 0x100000, 0x80000, CRC(131ae471) SHA1(5ed03b521dfef56cbb99814539d4c74da4216f67))
ROM_END

/*--------------
/  Game drivers
/---------------*/
GAME(1992,  taf_l5,  0,       wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-5)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_p2,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (Prototype) (P-2)",        MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l1,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-1)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l2,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-2)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l3,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-3)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l4,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-4)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l7,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (Prototype L-5) (L-7)",    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_l6,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (L-6)",                    MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992,  taf_h4,  taf_l5,  wpc_flip1,  wpc_flip1, wpc_flip1_state, init_wpc_flip1, ROT0, "Bally",    "The Addams Family (H-4)",                    MACHINE_IS_SKELETON_MECHANICAL)
