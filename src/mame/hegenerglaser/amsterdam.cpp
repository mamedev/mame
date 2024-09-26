// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Cowering, hap
/*******************************************************************************

Mephisto Amsterdam (2-ROM hardware version)

Hardware notes:

Amsterdam:
- same as Glasgow, but 2*27C256 EPROMs

Dallas 68020:
- MC68020RC12B @ 14MHz
- 64KB ROM(27C512), 64KB RAM(8*M5M5165P-10L)
- rest is similar to 16-bit version

TODO:
- waitstates, same as glasgow.cpp

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68020.h"
#include "sound/dac.h"

#include "speaker.h"

// internal artwork
#include "mephisto_amsterdam.lh"


namespace {

class amsterdam_state : public driver_device
{
public:
	amsterdam_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_keys(*this, "KEY.%u", 0),
		m_reset(*this, "RESET")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void amsterdam(machine_config &config);
	void dallas32(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<mephisto_display1_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<2> m_keys;
	required_ioport m_reset;

	void amsterd_mem(address_map &map) ATTR_COLD;
	void dallas32_mem(address_map &map) ATTR_COLD;

	void led_w(offs_t offset, u8 data);
	void dac_w(u8 data);
	u8 keys_r();
};

INPUT_CHANGED_MEMBER(amsterdam_state::reset_button)
{
	// RES buttons in serial tied to CPU RESET
	if (m_reset->read() == 3)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		m_display->reset();
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

void amsterdam_state::led_w(offs_t offset, u8 data)
{
	// d0-d7: board leds
	m_board->led_w(data);

	// a8: lcd strobe
	m_display->strobe_w(BIT(offset, 7));
}

void amsterdam_state::dac_w(u8 data)
{
	// d0: speaker out
	m_dac->write(BIT(data, 0));
}

u8 amsterdam_state::keys_r()
{
	// lcd strobe is shared with keypad select
	return m_keys[m_display->strobe_r()]->read();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void amsterdam_state::amsterd_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x800002, 0x800002).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0x800004, 0x800004).w(FUNC(amsterdam_state::dac_w));
	map(0x800008, 0x800009).nopr(); // clr.b
	map(0x800008, 0x800008).select(0x80).w(FUNC(amsterdam_state::led_w));
	map(0x800010, 0x800010).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x800020, 0x800020).r(m_board, FUNC(mephisto_board_device::input_r));
	map(0x800040, 0x800040).r(FUNC(amsterdam_state::keys_r));
	map(0xffc000, 0xffffff).ram(); // 16KB
}

void amsterdam_state::dallas32_mem(address_map &map)
{
	map(0x000000, 0x00ffff).rom();
	map(0x010000, 0x01ffff).ram(); // 64KB
	map(0x800002, 0x800002).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0x800004, 0x800004).w(FUNC(amsterdam_state::dac_w));
	map(0x800008, 0x800008).select(0x80).w(FUNC(amsterdam_state::led_w));
	map(0x800010, 0x800010).w(m_board, FUNC(mephisto_board_device::mux_w));
	map(0x800020, 0x800020).r(m_board, FUNC(mephisto_board_device::input_r));
	map(0x800040, 0x800040).r(FUNC(amsterdam_state::keys_r));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( amsterdam )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7 / King") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Pawn") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Knight") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Bishop") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Rook") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / Queen") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 1") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, amsterdam_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 2") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, amsterdam_state, reset_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void amsterdam_state::amsterdam(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_periodic_int(FUNC(amsterdam_state::irq5_line_hold), attotime::from_hz(50));
	m_maincpu->set_addrmap(AS_PROGRAM, &amsterdam_state::amsterd_mem);

	MEPHISTO_SENSORS_BOARD(config, m_board);
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	MEPHISTO_DISPLAY_MODULE1(config, m_display);
	config.set_default_layout(layout_mephisto_amsterdam);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void amsterdam_state::dallas32(machine_config &config)
{
	amsterdam(config);

	// basic machine hardware
	M68020(config.replace(), m_maincpu, 14_MHz_XTAL);
	m_maincpu->set_periodic_int(FUNC(amsterdam_state::irq4_line_hold), attotime::from_hz(50));
	m_maincpu->set_addrmap(AS_PROGRAM, &amsterdam_state::dallas32_mem);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( amsterd )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("amsterda-u.bin", 0x00000, 0x08000, CRC(0a75514e) SHA1(27daf78b0aba4d7a293b96b3c1fa92f6ee9bcb59) )
	ROM_LOAD16_BYTE("amsterda-l.bin", 0x00001, 0x08000, CRC(6e17d8fa) SHA1(e0f9e57aaa445f6ff7cbe868658ed7bcfa7e31fb) )
ROM_END


ROM_START( dallas32 ) // serial 06053xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dallas_68020", 0x00000, 0x10000, CRC(00ab8e11) SHA1(5e0a2f5e6b5a65d4997d6a999f23f9c30460f2e3) ) // MBM27C512-25
ROM_END

ROM_START( dallas32a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("dallas32a.bin", 0x00000, 0x10000, CRC(83b9ff3f) SHA1(97bf4cb3c61f8ec328735b3c98281bba44b30a28) )
ROM_END

ROM_START( dallas16 )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("dallas-u.bin", 0x00000, 0x08000, CRC(70b741f7) SHA1(23d55ed0fea127b727d725c367ee1932ff5af39f) )
	ROM_LOAD16_BYTE("dallas-l.bin", 0x00001, 0x08000, CRC(69300ad3) SHA1(57ec1b955b1ddfe722011ff5da68a0cd71af9251) )
ROM_END


ROM_START( roma32 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("roma32.bin", 0x00000, 0x10000, CRC(587d03bf) SHA1(504e9ff958084700076d633f9c306fc7baf64ffd) )
ROM_END

ROM_START( roma16 )
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD16_BYTE("roma16-u.bin", 0x00000, 0x08000, CRC(111d030f) SHA1(e027f7e7018d28ab794e7730392506056809db6b) )
	ROM_LOAD16_BYTE("roma16-l.bin", 0x00001, 0x08000, CRC(736e1c8d) SHA1(3a71f9185406ab5237c912ec8563b88b01ad50e8) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT    COMPAT  MACHINE     INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, amsterd,   0,        0,      amsterdam,  amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Amsterdam", MACHINE_SUPPORTS_SAVE )

SYST( 1986, dallas32,  0,        0,      dallas32,   amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68020 (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, dallas32a, dallas32, 0,      dallas32,   amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68020 (set 2)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, dallas16,  dallas32, 0,      amsterdam,  amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Dallas 68000", MACHINE_SUPPORTS_SAVE )

SYST( 1987, roma32,    0,        0,      dallas32,   amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68020", MACHINE_SUPPORTS_SAVE )
SYST( 1987, roma16,    roma32,   0,      amsterdam,  amsterdam, amsterdam_state, empty_init, "Hegener + Glaser", "Mephisto Roma 68000", MACHINE_SUPPORTS_SAVE )
