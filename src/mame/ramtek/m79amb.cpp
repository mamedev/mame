// license:BSD-3-Clause
// copyright-holders: Al Kossow

/*          Ramtek M79 Ambush


The following chart explains the settings of the eight switches on
the DIP switch.  A plus(+) in the column means the toggle switch is
up on the plus side of the DIP switch.

                                SWITCHES
                         |  12  |  345  |  678  |
------------------------------------------------|
Length of Game (seconds) |      |       |       |
                  60     |  00  |       |       |
                  90     |  0+  |       |       |
                  90     |  +0  |       |       |
                 120     |  ++  |       |       |
-------------------------+------+-------+-------|
Points for Extended Time |      |       |       |
                1500     |      |  000  |       |
                2500     |      |  +00  |       |
                3500     |      |  0+0  |       |
                5000     |      |  ++0  |       |
    NO extended time     |      |  +++  |       |
-------------------------+------+-------+-------|
Coins Per Game           |      |       |       |
 Free Play - two players |      |       |  0++  |
 One Coin  - two players |      |       |  0+0  |
 One Coin  - each player |      |       |  000  |
 Two Coins - each player |      |       |  +00  |
-------------------------------------------------


Based on extensive tests on location, the factory settings for the
most universal combinations are:
    60 second long game
    2500 points for extended play             12345678
    One coin each player                      00+00000

Ports:
 In:
  8000 DIP SW
  8002 D0=VBlank
  8004 Game Switch Inputs
  8005 Game Switch Inputs

 Out:
  8000
  8001 Mask Sel (Manual calls it "Select All RAM")
  8002 Sound Control (According to Manual)
  8003 D0=SelfTest LED


The cabinet has backdrop artwork, a lightbulb for explosions,
and two large (paddles pretending to be) guns.

*/

#include "emu.h"

#include "m79amb_a.h"

#include "cpu/i8085/i8085.h"
#include "sound/discrete.h"

#include "screen.h"
#include "speaker.h"


namespace {

class m79amb_state : public driver_device
{
public:
	m79amb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_mask(*this, "mask"),
		m_discrete(*this, "discrete"),
		m_maincpu(*this, "maincpu"),
		m_gun_port(*this, "800%u", 4U),
		m_gun_pos(*this, "GUN%u", 1U),
		m_self_test(*this, "SELF_TEST"),
		m_exp_lamp(*this, "EXP_LAMP")
	{ }

	void m79amb(machine_config &config);

	void init_m79amb();

protected:
	void machine_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_mask;

	required_device<discrete_device> m_discrete;
	required_device<i8080_cpu_device> m_maincpu;

	required_ioport_array<2> m_gun_port;
	required_ioport_array<2> m_gun_pos;
	output_finder<> m_self_test;
	output_finder<> m_exp_lamp;

	// misc
	uint8_t m_lut_gun[2][0x100]{};

	void videoram_w(offs_t offset, uint8_t data);
	template <uint8_t Which> uint8_t gray5bit_controller_r();
	void _8000_w(uint8_t data);
	void _8002_w(uint8_t data);
	void _8003_w(uint8_t data);
	uint8_t inta_r();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void main_map(address_map &map) ATTR_COLD;
};


// the ports are guessed from operation
// the schematics do not show the actual hookup

void m79amb_state::_8000_w(uint8_t data)
{
	// these values are not latched
	// they are pulsed when the port is addressed
	// the discrete system will just trigger from them
	m_discrete->write(M79AMB_SHOT_EN, data & 0x01);
	m_discrete->write(M79AMB_BOOM_EN, data & 0x02);
	m_discrete->write(M79AMB_THUD_EN, data & 0x04);
}

void m79amb_state::_8003_w(uint8_t data)
{
	// Self Test goes low on reset and lights LED
	// LED goes off on pass
	m_self_test = BIT(data, 0);
	m_discrete->write(M79AMB_MC_REV_EN, data & 0x02);
	m_discrete->write(M79AMB_MC_CONTROL_EN, data & 0x04);
	m_discrete->write(M79AMB_TANK_TRUCK_JEEP_EN, data & 0x08);
	m_discrete->write(M79AMB_WHISTLE_B_EN, data & 0x10);
	m_discrete->write(M79AMB_WHISTLE_A_EN, data & 0x20);
}

void m79amb_state::machine_start()
{
	m_self_test.resolve();
	m_exp_lamp.resolve();
}

void m79amb_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data & ~*m_mask;
}

uint32_t m79amb_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < 0x2000; offs++)
	{
		uint8_t data = m_videoram[offs];
		int const y = offs >> 5;
		int x = (offs & 0x1f) << 3;

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x) = pen;

			x++;
			data <<= 1;
		}
	}

	return 0;
}


template <uint8_t Which>
uint8_t m79amb_state::gray5bit_controller_r()
{
	uint8_t const port_data = m_gun_port[Which]->read();
	uint8_t const gun_pos = m_gun_pos[Which]->read();

	return (port_data & 0xe0) | m_lut_gun[Which][gun_pos];
}

void m79amb_state::_8002_w(uint8_t data)
{
	// D1 may also be watchdog reset
	// port goes to 0x7f to turn on explosion lamp
	m_exp_lamp = data ? 1 : 0;
}

void m79amb_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x5fff).ram().w(FUNC(m79amb_state::videoram_w)).share(m_videoram);
	map(0x6000, 0x63ff).ram();                 // ??
	map(0x8000, 0x8000).portr("8000").w(FUNC(m79amb_state::_8000_w));
	map(0x8001, 0x8001).writeonly().share("mask");
	map(0x8002, 0x8002).portr("8002").w(FUNC(m79amb_state::_8002_w));
	map(0x8003, 0x8003).w(FUNC(m79amb_state::_8003_w));
	map(0x8004, 0x8004).r(FUNC(m79amb_state::gray5bit_controller_r<0>));
	map(0x8005, 0x8005).r(FUNC(m79amb_state::gray5bit_controller_r<1>));
	map(0xc000, 0xc07f).ram();                 // ??
	map(0xc200, 0xc27f).ram();                 // ??
}



static INPUT_PORTS_START( m79amb )
	PORT_START("8000")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Game_Time ))         PORT_DIPLOCATION("G6:!1,!2")
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x01, "90 Seconds" )
	PORT_DIPSETTING(    0x02, "90 Seconds" )
	PORT_DIPSETTING(    0x03, "120 Seconds" )
	PORT_DIPNAME( 0x1c, 0x04, "Points for Extended Time" )  PORT_DIPLOCATION("G6:!3,!4,!5")
	PORT_DIPSETTING(    0x00, "1500" )
	PORT_DIPSETTING(    0x04, "2500" )
	PORT_DIPSETTING(    0x08, "3500" )
	PORT_DIPSETTING(    0x0c, "5000" )
	PORT_DIPSETTING(    0x1c, "No Extended Time" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Coinage ))           PORT_DIPLOCATION("G6:!6,!7,!8")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x40, "1 Coin/2 Players")
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ))

	PORT_START("8002")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("8004")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED ) // gun 1 here
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("8005")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED ) // gun 2 here
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// fake ports for the guns
	PORT_START("GUN1")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_CROSSHAIR(X, (1.0-(19.0/256.0)), 19.0/256.0, 30.0/224.0) PORT_MINMAX(19,255) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(1)

	PORT_START("GUN2")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_CROSSHAIR(X, (1.0-(22.0/256.0)), 0.0, 30.0/224.0) PORT_MINMAX(0,234) PORT_SENSITIVITY(50) PORT_KEYDELTA(5) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END


uint8_t m79amb_state::inta_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);

	// Vector is RST 1
	return 0xcf;
}

void m79amb_state::m79amb(machine_config &config)
{
	// basic machine hardware
	I8080(config, m_maincpu, 19.6608_MHz_XTAL / 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &m79amb_state::main_map);
	m_maincpu->in_inta_func().set(FUNC(m79amb_state::inta_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(19.6608_MHz_XTAL / 4, 320, 0, 256, 262, 32, 256);
	screen.set_screen_update(FUNC(m79amb_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 0, ASSERT_LINE);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, m79amb_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( m79amb )
	ROM_REGION( 0x2000, "maincpu", ROMREGION_INVERT )
	ROM_LOAD( "m79.10t",      0x0000, 0x0200, CRC(ccf30b1e) SHA1(c1a77f8dc81c491928f81121ca5c9b7f8753794f) )
	ROM_LOAD( "m79.9t",       0x0200, 0x0200, CRC(daf807dd) SHA1(16cd9d553bfb111c8380966cbde39dbddd5fe58c) )
	ROM_LOAD( "m79.8t",       0x0400, 0x0200, CRC(79fafa02) SHA1(440620f5be44febdd7c64014739dc71fb570cc92) )
	ROM_LOAD( "m79.7t",       0x0600, 0x0200, CRC(06f511f8) SHA1(a8dcaec0723b8ac9ad9474e3e931a23d7aa4ec23) )
	ROM_LOAD( "m79.6t",       0x0800, 0x0200, CRC(24634390) SHA1(bc5f2ae5a49904dde1bd5e6b134571bf1d912735) )
	ROM_LOAD( "m79.5t",       0x0a00, 0x0200, CRC(95252aa6) SHA1(e7ea598f864510557b511682a5d2223a512ff029) )
	ROM_LOAD( "m79.4t",       0x0c00, 0x0200, CRC(54cffb0f) SHA1(4a4ad921ef6324c927a2e4a9da624d8096b6d87b) )
	ROM_LOAD( "m79.3ta",      0x0e00, 0x0200, CRC(27db5ede) SHA1(890587181497ed6e1d45ed501790a6d4d3315f00) )
	ROM_LOAD( "m79.10u",      0x1000, 0x0200, CRC(e41d13d2) SHA1(cc2911f46a0465305e4c7bc08f55acd065f93534) )
	ROM_LOAD( "m79.9u",       0x1200, 0x0200, CRC(e35f5616) SHA1(394ad92ad7dd233ece17335cf20aef8861b41508) )
	ROM_LOAD( "m79.8u",       0x1400, 0x0200, CRC(14eafd7c) SHA1(ca2d17f6ae1c3ff461a1b2bc6f37622e70cdaae8) )
	ROM_LOAD( "m79.7u",       0x1600, 0x0200, CRC(b9864f25) SHA1(9330cf96b7bce13e0ee3ad746b00e82ef10c3989) )
	ROM_LOAD( "m79.6u",       0x1800, 0x0200, CRC(dd25197f) SHA1(13eaf40251de82e817f488a9de738aadd8f6715e) )
	ROM_LOAD( "m79.5u",       0x1a00, 0x0200, CRC(251545e2) SHA1(05a0d5e8f143ea376fb3c517cf5e9d0d3534b933) )
	ROM_LOAD( "m79.4u",       0x1c00, 0x0200, CRC(b5f55c75) SHA1(f478cde73ae961be7b58c769f035eef58fd45555) )
	ROM_LOAD( "m79.3u",       0x1e00, 0x0200, CRC(e968691a) SHA1(7024d10f2af195fc4050861706b1f3d22cb22a9c) )
ROM_END



/* grenade trajectory per gun position is inconsistent and sloppy in the game:
     0,     1,     3,     2,     6,     7,     5,     4     - gun position
    90.00, 90.00, 90.00, 90.00, 86.42, 86.42, 86.42, 86.42  - grenade trajectory (angle, est)
     18.0,  18.0,  18.0,  18.0,  27.2,  27.2,  27.2,  31.2  - crosses with y=28 (x, est)

    12,    13,    15,    14,    10,    11,     9,     8,
    84.39, 84.39, 84.39, 80.87, 79.00, 80.87, 79.00, 79.00
     41.9,  48.9,  56.8,  75.8,  87.2,  88.8, 101.6, 107.6

    24,    25,    27,    26,    30,    31,    29,    28,
    79.00, 79.00, 75.59, 75.59, 75.59, 73.72, 73.72, 73.72
    114.1, 121.5, 138.8, 146.0, 152.7, 162.6, 167.6, 172.7

    20,    21,    23,    22,    18,    19,    17,    16
    73.72, 70.08, 70.08, 70.08, 67.97, 67.97, 64.34, 64.34
    181.6, 199.9, 205.4, 211.9, 223.5, 232.4, 254.0, 254.0
*/

void m79amb_state::init_m79amb()
{
	static constexpr uint8_t lut_cross[0x20] = {
		19,    20,    21,    23,    25,    27,    29,    37,
		45,    53,    66,    82,    88,    95,    105,   111,
		118,   130,   142,   149,   158,   165,   170,   177,
		191,   203,   209,   218,   228,   243,   249,   255,
	};

	// gun positions
	for (int i = 0, j = 0, k = 0xff; i < 0x20; i++)
	{
		const uint8_t gray = i ^ (i >> 1) ^ 0x1f; // 5-bit Gray code, inverted

		// gun 1, start at left 18
		while (j < 0x100 && j <= lut_cross[i])
			m_lut_gun[0][j++] = gray;

		// gun 2, start at right 235
		while (k >= 0 && k >= 253 - lut_cross[i])
			m_lut_gun[1][k--] = gray;
	}
}

} // anonymous namespace


GAME( 1977, m79amb, 0, m79amb, m79amb, m79amb_state, init_m79amb, ROT0, "Ramtek", "M-79 Ambush", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
