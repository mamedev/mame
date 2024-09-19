// license:BSD-3-Clause
// copyright-holders:AJR
/*

Ameri-Hockey?

One of an unknown number of mechanical games developed by Ameri Corporation,
an Illinois-based company in business between 1988 and 1995.

U3 -  27C512
U8 -  27C020
U9 -  27C020
U10- 27C020

12 MHz crystal

Processor is a ROMless MCU from the Z8 family.

*/

#include "emu.h"
#include "cpu/z8/z8.h"
#include "sound/okim6376.h"
#include "speaker.h"
#include "amerihok.lh"


namespace {

class amerihok_state : public driver_device
{
public:
	amerihok_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_digits(*this, "digit%u", 0U)
		, m_lamp(*this, "lamp")
	{ }

	void amerihok(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void control_w(u8 data);
	void p2_w(u8 data);
	u8 p3_r();

	required_device<z8_device> m_maincpu;
	required_device<okim6376_device> m_oki;
	void amerihok_data_map(address_map &map) ATTR_COLD;
	void amerihok_map(address_map &map) ATTR_COLD;

	u32 m_outputs[2]{};
	u8 m_old_p2 = 0U;

	output_finder<6> m_digits;
	output_finder<> m_lamp;
};

void amerihok_state::control_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	m_lamp = !BIT(data, 0);

	m_oki->st_w(!BIT(data, 4));
	m_oki->ch2_w(!BIT(data, 7));
	if (BIT(data, 5))
		m_oki->reset();
}

void amerihok_state::p2_w(u8 data)
{
	if (BIT(data, 5) && !BIT(m_old_p2, 5))
	{
		m_outputs[1] = (m_outputs[1] << 1) | BIT(m_outputs[0], 31);
		m_outputs[0] = (m_outputs[0] << 1) | BIT(data, 6);
	}

	if (BIT(data, 7))
	{
		m_digits[0] = bitswap<7>(m_outputs[0], 3, 4, 2, 1, 0, 6, 5);
		m_digits[1] = bitswap<7>(m_outputs[0], 27, 30, 31, 26, 25, 28, 29);
		m_digits[2] = bitswap<7>(m_outputs[0], 20, 23, 22, 18, 19, 21, 24);
		m_digits[3] = bitswap<7>(m_outputs[0], 8, 11, 12, 13, 9, 7, 10);
		m_digits[4] = bitswap<7>(m_outputs[1], 5, 1, 2, 7, 6, 3, 0);
		m_digits[5] = bitswap<7>(m_outputs[1], 28, 27, 23, 22, 21, 24, 26);

		// These outputs are inactive during gameplay
		//m_dots[0] = BIT(m_outputs[0], 16);
		//m_dots[1] = BIT(m_outputs[0], 17);

		//logerror("Outputs = %08X%08X\n", m_outputs[1], m_outputs[0]);
	}

	m_old_p2 = data;
}

void amerihok_state::amerihok_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void amerihok_state::amerihok_data_map(address_map &map)
{
	map(0x1000, 0x1000).portr("1000");
	map(0x2000, 0x2000).w(FUNC(amerihok_state::control_w));
	map(0x3000, 0x3000).portr("3000");
	map(0x4000, 0x4000).w(m_oki, FUNC(okim6376_device::write));
}

static INPUT_PORTS_START( amerihok )
	PORT_START("1000")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(KEYCODE_X)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_CODE(KEYCODE_V)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_CODE(KEYCODE_N)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON8) PORT_CODE(KEYCODE_COMMA)

	PORT_START("3000")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Score Visitor")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Score Home")
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON9) PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON10) PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0xee, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("P3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("oki", okim6376_device, nar_r)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN2)
INPUT_PORTS_END



void amerihok_state::machine_start()
{
	m_digits.resolve();
	m_lamp.resolve();

	std::fill(std::begin(m_outputs), std::end(m_outputs), 0);
	m_old_p2 = 0xff;

	save_item(NAME(m_outputs));
	save_item(NAME(m_old_p2));
}

void amerihok_state::machine_reset()
{
}


void amerihok_state::amerihok(machine_config &config)
{
	Z8681(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &amerihok_state::amerihok_map);
	m_maincpu->set_addrmap(AS_DATA, &amerihok_state::amerihok_data_map);
	m_maincpu->p2_in_cb().set_ioport("P2");
	m_maincpu->p2_out_cb().set(FUNC(amerihok_state::p2_w));
	m_maincpu->p3_in_cb().set_ioport("P3");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6376(config, m_oki, 12_MHz_XTAL / 96); // 64-pin QFP, type/clock unverified (probably clocked by Z8681 TOUT)
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( amerihok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "air-h-u3", 0x00000, 0x10000, CRC(f43eaa25) SHA1(b73e3f6db9fe277dab3fd9d1161f3b71b5805048) )

	ROM_REGION( 0xc0000, "oki", 0 )
	ROM_LOAD( "air-h-u8", 0x00000, 0x40000, CRC(17a84f88) SHA1(33a5a66b1e7c8bf79c99e442c62d8ce0c7d1c22c) )
	ROM_LOAD( "air-h-u9", 0x40000, 0x40000, CRC(be01ca4a) SHA1(87513a5c547633d5a3f09e931bd7ec78bcaa94dc) )
	ROM_LOAD( "airh-u10", 0x80000, 0x40000, CRC(71ee6421) SHA1(10131fc7c009158308c4a8bb2b037101622c07a1) )
ROM_END

} // anonymous namespace


GAMEL( 199?, amerihok, 0, amerihok, amerihok, amerihok_state, empty_init, ROT0, "Ameri", "Ameri-Hockey", MACHINE_NOT_WORKING | MACHINE_MECHANICAL, layout_amerihok )
