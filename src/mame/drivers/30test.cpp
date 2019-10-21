// license:BSD-3-Clause
// copyright-holders:Angelo Salese, hap
/***************************************************************************

    30 Test (Remake) (c) 1997 Namco

    driver by Angelo Salese

    TODO:
    - portd meaning is a mystery
    - inputs are annoying to map;
    - EEPROM

============================================================================

cheats:
- [0xb0-0xb3] timer

lamps:
?OK???!! = really OK! (91+) (0x81)
???????? = pretty good (80+) (0x82)
???~??? = not bad (70+) (0x84)
??? = normal (55+) (0x88)
????? = pretty bad (40+) (0x90)
???~ = worst (39 or less) (0xa0)
??????? = game over (0xe0)


============================================================================

30-TEST (Remake)
NAMCO 1997
GAME CODE M125

MC68HC11K1
M6295
X1 1.056MHz
OSC1 16.000MHz


cabinet photo
http://blogs.yahoo.co.jp/nadegatayosoyuki/59285865.html

***************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"
#include "sound/okim6295.h"
#include "speaker.h"

#include "30test.lh"

#define MAIN_CLOCK XTAL(16'000'000)

class namco_30test_state : public driver_device
{
public:
	namco_30test_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		{ }

	void _30test(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<mc68hc11_cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	output_finder<72> m_digits;
	output_finder<8> m_lamps;

	uint8_t m_mux_data;
	uint8_t m_oki_bank;

	DECLARE_WRITE8_MEMBER(namco_30test_led_w);
	DECLARE_WRITE8_MEMBER(namco_30test_led_rank_w);
	DECLARE_WRITE8_MEMBER(namco_30test_lamps_w);
	DECLARE_READ8_MEMBER(namco_30test_mux_r);
	DECLARE_READ8_MEMBER(hc11_mux_r);
	DECLARE_WRITE8_MEMBER(hc11_mux_w);
	DECLARE_READ8_MEMBER(hc11_okibank_r);
	DECLARE_WRITE8_MEMBER(hc11_okibank_w);

	void namco_30test_map(address_map &map);
};


static const uint8_t led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };

WRITE8_MEMBER(namco_30test_state::namco_30test_led_w)
{
	m_digits[offset * 2] = led_map[data >> 4];
	m_digits[1 + offset * 2] =  led_map[data & 0x0f];
}

WRITE8_MEMBER(namco_30test_state::namco_30test_led_rank_w)
{
	if (offset < 4)
	{
		m_digits[64 + offset * 2] = led_map[data >> 4];
		m_digits[65 + offset * 2] = led_map[data & 0x0f];
	}
}

WRITE8_MEMBER(namco_30test_state::namco_30test_lamps_w)
{
	// d0-d5: ranking, d6: game over, d7: assume marquee lamp
	for (int i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
}

READ8_MEMBER(namco_30test_state::namco_30test_mux_r)
{
	uint8_t res = 0xff;

	switch(m_mux_data)
	{
		case 0x01: res = ioport("IN0")->read(); break;
		case 0x02: res = ioport("IN1")->read(); break;
		case 0x04: res = ioport("IN2")->read(); break;
		case 0x08: res = ioport("IN3")->read(); break;
	}

	return res;
}

READ8_MEMBER(namco_30test_state::hc11_mux_r)
{
	return m_mux_data;
}

WRITE8_MEMBER(namco_30test_state::hc11_mux_w)
{
	m_mux_data = data;
}

READ8_MEMBER(namco_30test_state::hc11_okibank_r)
{
	return m_oki_bank;
}

WRITE8_MEMBER(namco_30test_state::hc11_okibank_w)
{
	m_oki_bank = data;
	m_oki->set_rom_bank(data & 1);
}


void namco_30test_state::namco_30test_map(address_map &map)
{
	map(0x0000, 0x003f).ram(); // internal I/O
	map(0x0040, 0x007f).ram(); // more internal I/O, HC11 change pending
	map(0x007c, 0x007c).rw(FUNC(namco_30test_state::hc11_mux_r), FUNC(namco_30test_state::hc11_mux_w));
	map(0x007e, 0x007e).rw(FUNC(namco_30test_state::hc11_okibank_r), FUNC(namco_30test_state::hc11_okibank_w));
	map(0x0080, 0x037f).ram(); // internal RAM
	map(0x0d80, 0x0dbf).ram(); // EEPROM read-back data goes there
	map(0x2000, 0x2000).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	/* 0x401e-0x401f: time */
	map(0x4000, 0x401f).w(FUNC(namco_30test_state::namco_30test_led_w)); // 7-seg leds
	/* 0x6000: 1st place 7-seg led */
	/* 0x6001: 2nd place 7-seg led */
	/* 0x6002: 3rd place 7-seg led */
	/* 0x6003: current / last play score */
	/* 0x6004: lamps */
	map(0x6000, 0x6003).w(FUNC(namco_30test_state::namco_30test_led_rank_w));
	map(0x6004, 0x6004).w(FUNC(namco_30test_state::namco_30test_lamps_w));
	map(0x8000, 0xffff).rom();
}


static INPUT_PORTS_START( 30test )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 1-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-2") PORT_CODE(KEYCODE_W)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 2-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-4") PORT_CODE(KEYCODE_F)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 3-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 4-6") PORT_CODE(KEYCODE_N)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-1") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-2") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-3") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-4") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-5") PORT_CODE(KEYCODE_RALT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Button 5-6") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CODE(KEYCODE_8)
	PORT_DIPNAME( 0x08, 0x08, "UNK3" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "UNK4" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "UNK5" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "UNK6" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "UNK7" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void namco_30test_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();
	save_item(NAME(m_mux_data));
	save_item(NAME(m_oki_bank));
}

void namco_30test_state::_30test(machine_config &config)
{
	/* basic machine hardware */
	MC68HC11K1(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &namco_30test_state::namco_30test_map);
	m_maincpu->in_pa_callback().set(FUNC(namco_30test_state::namco_30test_mux_r));
	//m_maincpu->in_pd_callback().set_ram();
	m_maincpu->in_pe_callback().set_ioport("SYSTEM");

	/* no video hardware */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 1056000, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 30test )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1-mpr0b.8p",   0x0000, 0x10000, CRC(455043d5) SHA1(46b15324d193ee621beabce92c0dc493b608b8dd) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tt1-voi0.7p",   0x0000, 0x80000, CRC(b4fc5921) SHA1(92a88d5adb50dae48715847f12e88a35e37ef78c) )
ROM_END

GAMEL( 1997, 30test, 0, _30test, 30test, namco_30test_state, empty_init, ROT0, "Namco", "30 Test (Remake)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK, layout_30test )
