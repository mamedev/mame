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
#include "30test.lh"

#define MAIN_CLOCK XTAL_16MHz

class namco_30test_state : public driver_device
{
public:
	namco_30test_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki") { }

	UINT8 m_mux_data;
	UINT8 m_oki_bank;
	DECLARE_WRITE8_MEMBER(namco_30test_led_w);
	DECLARE_WRITE8_MEMBER(namco_30test_led_rank_w);
	DECLARE_WRITE8_MEMBER(namco_30test_lamps_w);
	DECLARE_READ8_MEMBER(namco_30test_mux_r);
	DECLARE_READ8_MEMBER(hc11_mux_r);
	DECLARE_WRITE8_MEMBER(hc11_mux_w);
	DECLARE_READ8_MEMBER(hc11_okibank_r);
	DECLARE_WRITE8_MEMBER(hc11_okibank_w);
	virtual void machine_start();
	virtual void machine_reset();
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
};


static const UINT8 led_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x77,0x7c,0x39,0x5e,0x79,0x00 };

WRITE8_MEMBER(namco_30test_state::namco_30test_led_w)
{
	output_set_digit_value(0 + offset * 2, led_map[(data & 0xf0) >> 4]);
	output_set_digit_value(1 + offset * 2, led_map[(data & 0x0f) >> 0]);
}

WRITE8_MEMBER(namco_30test_state::namco_30test_led_rank_w)
{
	output_set_digit_value(64 + offset * 2, led_map[(data & 0xf0) >> 4]);
	output_set_digit_value(65 + offset * 2, led_map[(data & 0x0f) >> 0]);
}

WRITE8_MEMBER(namco_30test_state::namco_30test_lamps_w)
{
	// d0-d5: ranking, d6: game over, d7: assume marquee lamp
	for (int i = 0; i < 8; i++)
		output_set_lamp_value(i, data >> i & 1);
}

READ8_MEMBER(namco_30test_state::namco_30test_mux_r)
{
	UINT8 res = 0xff;

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
	m_oki->set_bank_base((data & 1) ? 0x40000 : 0);
}


static ADDRESS_MAP_START( namco_30test_map, AS_PROGRAM, 8, namco_30test_state )
	AM_RANGE(0x0000, 0x003f) AM_RAM // internal I/O
	AM_RANGE(0x007c, 0x007c) AM_READWRITE(hc11_mux_r,hc11_mux_w)
	AM_RANGE(0x007e, 0x007e) AM_READWRITE(hc11_okibank_r,hc11_okibank_w)
	AM_RANGE(0x0040, 0x007f) AM_RAM // more internal I/O, HC11 change pending
	AM_RANGE(0x0080, 0x037f) AM_RAM // internal RAM
	AM_RANGE(0x0d80, 0x0dbf) AM_RAM // EEPROM read-back data goes there
	AM_RANGE(0x2000, 0x2000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	/* 0x401e-0x401f: time */
	AM_RANGE(0x4000, 0x401f) AM_WRITE(namco_30test_led_w) // 7-seg leds
	/* 0x6000: 1st place 7-seg led */
	/* 0x6001: 2nd place 7-seg led */
	/* 0x6002: 3rd place 7-seg led */
	/* 0x6003: current / last play score */
	/* 0x6004: lamps */
	AM_RANGE(0x6000, 0x6003) AM_WRITE(namco_30test_led_rank_w)
	AM_RANGE(0x6004, 0x6004) AM_WRITE(namco_30test_lamps_w)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( namco_30test_io, AS_IO, 8, namco_30test_state )
	AM_RANGE(MC68HC11_IO_PORTA,MC68HC11_IO_PORTA) AM_READ(namco_30test_mux_r)
//  AM_RANGE(MC68HC11_IO_PORTD,MC68HC11_IO_PORTD) AM_RAM
	AM_RANGE(MC68HC11_IO_PORTE,MC68HC11_IO_PORTE) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END


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
	save_item(NAME(m_mux_data));
	save_item(NAME(m_oki_bank));
}

void namco_30test_state::machine_reset()
{
}


static MACHINE_CONFIG_START( 30test, namco_30test_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", MC68HC11,MAIN_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(namco_30test_map)
	MCFG_CPU_IO_MAP(namco_30test_io)
	MCFG_MC68HC11_CONFIG( 0, 768, 0x00 )


	/* no video! */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( 30test )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tt1-mpr0b.8p",   0x0000, 0x10000, CRC(455043d5) SHA1(46b15324d193ee621beabce92c0dc493b608b8dd) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tt1-voi0.7p",   0x0000, 0x80000, CRC(b4fc5921) SHA1(92a88d5adb50dae48715847f12e88a35e37ef78c) )
ROM_END

GAMEL( 1997, 30test,  0,   30test,  30test, driver_device,  0, ROT0, "Namco", "30 Test (Remake)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK, layout_30test )
