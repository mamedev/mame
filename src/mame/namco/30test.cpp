// license:BSD-3-Clause
// copyright-holders:Angelo Salese, hap
/***************************************************************************

30 Test (サーティテスト) (c) 1997 Namco, game code M125
This is a remake, the original version was from 1970 and didn't have a CPU.

driver by Angelo Salese

Hardware notes:
- MC68HC11K1
- M6295
- X1 1.056MHz
- OSC1 16.000MHz

TODO:
- inputs are annoying to map (just use the clickable artwork)
- EEPROM in MCU

============================================================================

cheats:
- [0xb0-0xb3] timer

lamps:
- [0x81] really OK! (91+) - 超OKっすよ!!
- [0x82] pretty good (80+) - かなりィ⤴っすヨ
- [0x84] not bad (70+) - あっい〜じゃん
- [0x88] normal (55+) - ふつう
- [0x90] pretty bad (40+) - ちょいたり
- [0xa0] worst (39 or less) - サイテ〜
- [0xe0] game over - ゲームオーバー

settings stored in EEPROM:
- [0xd90] coins per credit (default 0x01)
- [0xd91] games per credit (default 0x02)
- [0xd99] time per game (default 0x0c, 12 seconds)
- [0xda9] language (default 0x03) - 0 = English, 1 = Chinese, 2 = Korean, 3 = Japanese

0xd90-0xda8 can also be changed in-game by holding service coin and then entering
testmode (hold 9, press F2). Voice language cannot be changed here though.

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/okim6295.h"
#include "speaker.h"

#include "30test.lh"


namespace {

class namco_30test_state : public driver_device
{
public:
	namco_30test_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_inputs(*this, "IN%u", 0),
		m_digits(*this, "digit%u", 0U),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void namco_30test(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_ioport_array<4> m_inputs;
	output_finder<72> m_digits;
	output_finder<8> m_lamps;

	void main_map(address_map &map) ATTR_COLD;

	void output_digit(int i, u8 data);
	void led_w(offs_t offset, u8 data);
	void led_rank_w(offs_t offset, u8 data);
	void lamps_w(u8 data);
	u8 mux_r();
	void coin_w(u8 data);
	void mux_w(u8 data);
	void okibank_w(u8 data);

	u8 m_mux_data = 0;
};

void namco_30test_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();

	save_item(NAME(m_mux_data));
}



/******************************************************************************
    I/O
******************************************************************************/

void namco_30test_state::output_digit(int i, u8 data)
{
	// assume it's using a 7448
	static const u8 led_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	m_digits[i] = led_map[data & 0xf];
}

void namco_30test_state::led_w(offs_t offset, u8 data)
{
	// 0-29: playfield
	// 30,31: time
	output_digit(offset * 2, data >> 4);
	output_digit(1 + offset * 2,  data & 0x0f);
}

void namco_30test_state::led_rank_w(offs_t offset, u8 data)
{
	// 0: 1st place
	// 1: 2nd place
	// 2: 3rd place
	// 3: current / last play score
	output_digit(64 + offset * 2, data >> 4);
	output_digit(65 + offset * 2, data & 0x0f);
}

void namco_30test_state::lamps_w(u8 data)
{
	// d0-d5: ranking, d6: game over, d7: assume marquee lamp
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

void namco_30test_state::coin_w(u8 data)
{
	// d2: coincounter
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));

	// d3/d4: ticket dispenser
	// other: ?
}

u8 namco_30test_state::mux_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_mux_data, i))
			data &= m_inputs[i]->read();
	}

	return data;
}

void namco_30test_state::mux_w(u8 data)
{
	m_mux_data = data;
}

void namco_30test_state::okibank_w(u8 data)
{
	m_oki->set_rom_bank(data & 1);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void namco_30test_state::main_map(address_map &map)
{
	map(0x2000, 0x2000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x4000, 0x401f).w(FUNC(namco_30test_state::led_w));
	map(0x6000, 0x6003).w(FUNC(namco_30test_state::led_rank_w));
	map(0x6004, 0x6004).w(FUNC(namco_30test_state::lamps_w));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( 30test )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-1")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-2")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-4")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-1")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-6")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) // service coin + also used in testmode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) // ticket dispenser
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE3 ) // used in testmode to advance test
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void namco_30test_state::namco_30test(machine_config &config)
{
	/* basic machine hardware */
	MC68HC11K1(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &namco_30test_state::main_map);
	m_maincpu->in_pa_callback().set(FUNC(namco_30test_state::mux_r));
	m_maincpu->out_pd_callback().set(FUNC(namco_30test_state::coin_w));
	m_maincpu->in_pe_callback().set_ioport("SYSTEM");
	m_maincpu->out_pg_callback().set(FUNC(namco_30test_state::okibank_w));
	m_maincpu->out_ph_callback().set(FUNC(namco_30test_state::mux_w));

	/* no video hardware */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1.056_MHz_XTAL, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( 30test )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1-mpr0b.8p",   0x0000, 0x10000, CRC(455043d5) SHA1(46b15324d193ee621beabce92c0dc493b608b8dd) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tt1-voi0.7p",   0x0000, 0x80000, CRC(b4fc5921) SHA1(92a88d5adb50dae48715847f12e88a35e37ef78c) )
ROM_END

} // anonymous namespace


/******************************************************************************
    Drivers
******************************************************************************/

/*     YEAR  NAME    PARENT  MACHINE       INPUT   CLASS               INIT        MONITOR  COMPANY, FULLNAME, FLAGS */
GAMEL( 1997, 30test, 0,      namco_30test, 30test, namco_30test_state, empty_init, ROT0,    "Namco", "30 Test (remake)", MACHINE_SUPPORTS_SAVE, layout_30test )
