// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Piggy Pass

hw platform unknown
game details unknown

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "sound/okim6295.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "piggypas.lh"


namespace {

class piggypas_state : public driver_device
{
public:
	piggypas_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_hd44780(*this, "hd44780"),
		m_ticket(*this, "ticket"),
		m_digits(*this, "digit%u", 0U)
	{ }

	void piggypas(machine_config &config);
	void fidlstix(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(ball_sensor);

private:
	void output_digits();
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void ctrl_w(uint8_t data);
	void port3_w(uint8_t data);
	void led_strobe_w(uint8_t data);
	void lcd_latch_w(uint8_t data);
	void lcd_control_w(uint8_t data);
	HD44780_PIXEL_UPDATE(piggypas_pixel_update);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<hd44780_device> m_hd44780;
	required_device<ticket_dispenser_device> m_ticket;
	output_finder<4> m_digits;
	uint8_t   m_ctrl = 0;
	uint8_t   m_lcd_latch = 0;
	uint32_t  m_digit_latch = 0;
	void piggypas_io(address_map &map) ATTR_COLD;
	void piggypas_map(address_map &map) ATTR_COLD;
	void fidlstix_io(address_map &map) ATTR_COLD;
};


void piggypas_state::output_digits()
{
	// Serial output driver is UCN5833A
	m_digits[0] = bitswap<8>(m_digit_latch & 0xff, 7,6,4,3,2,1,0,5) & 0x7f;
	m_digits[1] = bitswap<8>((m_digit_latch >> 8) & 0xff, 7,6,4,3,2,1,0,5) & 0x7f;
	m_digits[2] = bitswap<8>((m_digit_latch >> 16) & 0xff, 7,6,4,3,2,1,0,5) & 0x7f;
	m_digits[3] = bitswap<8>((m_digit_latch >> 24) & 0xff, 7,6,4,3,2,1,0,5) & 0x7f;

	m_digit_latch = 0;
}

void piggypas_state::ctrl_w(uint8_t data)
{
	if (!BIT(data, 2) && BIT(m_ctrl, 2))
		output_digits();

	m_ticket->motor_w(BIT(data, 6));

	m_ctrl = data;
}

void piggypas_state::port3_w(uint8_t data)
{
	if (!BIT(data, 1))
	{
		m_digit_latch >>= 1;

		if (BIT(data, 0))
			m_digit_latch |= (1U << 31);
	}
}

void piggypas_state::led_strobe_w(uint8_t data)
{
	if (!BIT(data, 0))
		m_digit_latch = 0;
}

void piggypas_state::lcd_latch_w(uint8_t data)
{
	// P1.7 might also be used to reset DS1232 watchdog
	m_lcd_latch = data;
	m_hd44780->db_w(data);
}

void piggypas_state::lcd_control_w(uint8_t data)
{
	// RXD (P3.0) = chip select
	// TXD (P3.1) = register select
	// INT0 (P3.2) = R/W
	m_hd44780->e_w(BIT(data, 0));
	m_hd44780->rs_w(BIT(data, 1));
	m_hd44780->rw_w(BIT(data, 2));

	// T0 (P3.4) = output shift clock (serial data present at P1.0)
	// T1 (P3.5) = output latch enable
	if (BIT(data, 4))
		m_digit_latch = (m_digit_latch >> 1) | (BIT(m_lcd_latch, 0) << 31);
	if (BIT(data, 5) && !BIT(data, 4))
		output_digits();
}

void piggypas_state::piggypas_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void piggypas_state::piggypas_io(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1000, 0x1000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1800, 0x1801).w(m_hd44780, FUNC(hd44780_device::write));
	map(0x1802, 0x1803).r(m_hd44780, FUNC(hd44780_device::read));
}

void piggypas_state::fidlstix_io(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("nvram");
	map(0x0800, 0x0803).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x1000, 0x1000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x1800, 0x1800).nopw(); // input matrix scan?
}


INPUT_CHANGED_MEMBER(piggypas_state::ball_sensor)
{
	m_maincpu->set_input_line(1, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( piggypas )
	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_CUSTOM)  PORT_READ_LINE_DEVICE_MEMBER("ticket", ticket_dispenser_device, line_r)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)  PORT_NAME("Gate sensor")   PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1)

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Decrease")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Exit")       PORT_CODE(KEYCODE_E)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Last")       PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Run")        PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Increase")   PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Enter")      PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_NAME("Next")       PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_SERVICE)  PORT_NAME("Program")

	PORT_START("IN2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, piggypas_state, ball_sensor, 0) // ball sensor
INPUT_PORTS_END


void piggypas_state::machine_start()
{
	m_digits.resolve();

	m_digit_latch = 0;
}

void piggypas_state::machine_reset()
{
}

HD44780_PIXEL_UPDATE(piggypas_state::piggypas_pixel_update)
{
	if (pos < 8)
		bitmap.pix(y, (line * 8 + pos) * 6 + x) = state;
}

void piggypas_state::piggypas(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, 8.448_MHz_XTAL); // OKI M80C31F or M80C154S
	m_maincpu->set_addrmap(AS_PROGRAM, &piggypas_state::piggypas_map);
	m_maincpu->set_addrmap(AS_IO, &piggypas_state::piggypas_io);
	m_maincpu->port_out_cb<1>().set(FUNC(piggypas_state::led_strobe_w));
	m_maincpu->port_in_cb<3>().set_ioport("IN2");
	m_maincpu->port_out_cb<3>().set(FUNC(piggypas_state::port3_w));
//  m_maincpu->set_vblank_int("screen", FUNC(piggypas_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1220AD

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_screen_update(m_hd44780, FUNC(hd44780_device::screen_update));
	screen.set_size(16*6, 8);
	screen.set_visarea(0, 16*6-1, 0, 8-1);
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(2);
	config.set_default_layout(layout_piggypas);

	HD44780(config, m_hd44780, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_hd44780->set_lcd_size(1, 16);
	m_hd44780->set_pixel_update_cb(FUNC(piggypas_state::piggypas_pixel_update));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", 8.448_MHz_XTAL / 8, okim6295_device::PIN7_HIGH)); // clock not verified
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);

	i8255_device &ppi(I8255A(config, "ppi")); // OKI M82C55A-2
	ppi.in_pa_callback().set_ioport("IN1");
	ppi.out_pb_callback().set(FUNC(piggypas_state::ctrl_w));
	ppi.in_pc_callback().set_ioport("IN0");

	TICKET_DISPENSER(config, "ticket", attotime::from_msec(100));
}

void piggypas_state::fidlstix(machine_config &config)
{
	piggypas(config);

	m_maincpu->set_addrmap(AS_IO, &piggypas_state::fidlstix_io);
	m_maincpu->port_in_cb<1>().set(m_hd44780, FUNC(hd44780_device::db_r));
	m_maincpu->port_out_cb<1>().set(FUNC(piggypas_state::lcd_latch_w));
	m_maincpu->port_out_cb<3>().set(FUNC(piggypas_state::lcd_control_w));
}




ROM_START( piggypas )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pigypass.u6", 0x00000, 0x10000, CRC(c8dc4e26) SHA1(f9643945f84fe2679742922abf5a92b77bf59e4c) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "pigypass.u14", 0x00000, 0x40000, CRC(855504c1) SHA1(dfe91943057fa66798c8395348cf703cb11468d2) )
ROM_END


ROM_START( hoopshot )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hoopshot.u6", 0x00000, 0x08000, CRC(fa8ae8aa) SHA1(2266a775fba7c8f8e3e24441aca6c4b89a6d1ec7) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "hoopshot.u14", 0x00000, 0x40000, CRC(748462b5) SHA1(ccb8f1dbb6471b134c1e97699383c3ef139c42c3) )
ROM_END



ROM_START( rndrndqs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "round.u6", 0x00000, 0x08000, CRC(3eb64b10) SHA1(66051cdd6be33f4f7249be1c8d56e5e43c838163) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "round.u14", 0x00000, 0x40000, CRC(36d1c07a) SHA1(3c978d4d03d8dbf79e1afe7dc46209d9ac4d3cc3) )
ROM_END

ROM_START( fidlstix )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fiddle.u6", 0x00000, 0x08000, CRC(48125bf1) SHA1(5772fe3c0987fc6b2508da5efe3c4c3c179b76a1) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "fiddle.u14", 0x00000, 0x40000, CRC(baf4e1cd) SHA1(ae153f832cbd188e9f3f357a1a1f68cc8264d346) )
ROM_END

/*
 _________________________________________________________________________________________________________________
| _______________________________  ||||||||  ||||    ||||   ||||||  ||||||| |||   ||||||||||                     |
||                              |  ___ ___ ___ ___    ___     ___   ___ ___  ___  ___ ___ ___                    |
|| Screen                       | |__||__||__||__|   |__|    |__|  |__||__| |__| |__||__| |__| <- 12 x EPA120    |
||                              |   ____  ____  ____  ____                                                       |
||______________________________|  |   | |   | |   | |   |          _________   _________   _________            |
|                                  |___| |___| |___| |___|         |UDN2595A|  |ILQ621GB|  |ILQ621GB|   ____     |
|                8 keys keyboard -> ____  ____  ____  ____                                             (____)    |
|                                  |   | |   | |   | |   |          _________   _________              L7805CT   |
|                                  |___| |___| |___| |___|         |SN74LS374  |SN74LS374                        |
|      ______           ______    ___                                                                            |
|     |     |          |     |   |  <-SN74LS373N   ___     ___    ______   _________             _____           |
|     |  <-UCN5833A    |     |   |  |     ILQ621GB-> |    |  |   | OKI |   SN74LS04N     ______  LM317T ___      |
|     |     | ___  M80C31F-> |   |  |             ILQ621GB-> |   |M82C55A               |EPROM|         EPA120   |
| ___ |     ||  |  ___ |     |   |__|             |__|    |__|   |     |        _______ |     |         ___     -|
||<-MDP1403 ||  | |_<-DS1232 |    ______             ___   ___   |     |       | OKI  | |     |         EPA120  -|
||  | |MDP1403->|      |     |   |EPROM|  _______   |  |  |  |   |     |       | M6295| |     |         ___     -|
||__| |     | ___      |     |   |     | |DS1220AD  | <-T74LS138B1     |       |______| |     |         EPA120   |
| ___ |MDP1403->| ____ |     |   |     | |      |   |__|  |_<-T74LS138B1            ___ |     |            ___  -|
||<-MDP1403 ||  | Xtal |     |   |     | |      |   _________    |     |           |  | |_____|         EPA120  -|
||  | |_____||__|8.448MHz____|   |     | |      |   SN74LS08N    |     |   BA10324A-> |    ___          ___      |
||__|   _______                  |_____| |      |                |_____|           |__|   |  |         |  |      |
|      74HC393AP                         |______|                                 LM388N-1->_| LM388N-1->_|      |
|________________________________________________________________________________________________________________|

8.448MHz Crystal
P-80C31-16 cpu
M6295 (Verified with DMM that Pin 7 tied to VCC)
DS1220AD-150
M82C55A-2
UCN5832A
2 Line Display LCD
HD44780A00 (underneath board the LCD is mounted to)

-----------------------------------------------------------------------------

Factory Wire Mods

Parts Side:
    - Pin 1 of U25 (74LS373) pulled up and jumper wired to pin 5 of U2 (CM1232P/DS1232)

Solder Side:
    - Ground test point near C32 jumper wired to pin 2 of J8
      (pin 2 of J8 connected to pins 5 and 6 of L6 (CM1232P/DS1232))
    - Pin 1 of J1 jumper wired with a 1K +/-5% 1/4 watt resistor to pin 10 of J1
*/
ROM_START( hoopitup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "us a05449-01c.u6", 0x00000, 0x08000, CRC(8fc6a07d) SHA1(c1c7ad708eb84e9801fb5acea0b3b797923886c3) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound ad5.u14", 0x00000, 0x40000, CRC(e6f647c3) SHA1(8f208af76e5f94b5db479ecbd65922fd834250cf) )
ROM_END

ROM_START( hoopitup21 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "program.u6", 0x00000, 0x08000, CRC(17508556) SHA1(32a4ba732bb6efccb2ce89696f80a8265306b079) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "sound.u14", 0x00000, 0x40000, CRC(cf3eb5ef) SHA1(0ac06f87ab75986d2ca6f8cb80d6ae7bd964e54f) )
ROM_END

// bad dump of program rom
ROM_START( jackbean )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "beanstlk.u6", 0x00000, 0x10000, BAD_DUMP CRC(127c4d6c) SHA1(c864293f42e81a1b8e5dcb12abc1c0019853792e) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "beanstlk.u14", 0x00000, 0x40000, CRC(e33ef0a3) SHA1(337ce3d0c901b0b3241d76601eaad6e3e2724e1a) )
ROM_END

// bad dump of program rom
ROM_START( dumpump )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dump-ump.u6", 0x00000, 0x08000, BAD_DUMP CRC(410fc27e) SHA1(d9505c11f4844b9b58c12b3ff6b860357a4be75e))

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "dump-ump.u14", 0x00000, 0x20000, CRC(08bc7bb5) SHA1(2355783ec614d8f4e1dca3cb415a97a28411157b))
ROM_END

// bad dump of program rom
ROM_START( 3lilpigs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "3-pigs.u6", 0x00000, 0x10000, BAD_DUMP CRC(1db9d754) SHA1(9b1db9c9bb155ebb5509970476b20b9dda6d3021) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "3-pigs.u14", 0x00000, 0x40000, CRC(62eb76e2) SHA1(c4cad241dedf2c290f9bf80038415fe39b3ce17d) )
ROM_END

} // anonymous namespace


// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 04.40
GAME( 1992, piggypas,   0,    piggypas, piggypas, piggypas_state,     empty_init, ROT0, "Doyle & Assoc.",               "Piggy Pass (version 04.40)",              MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// COPYRIGHT (c) 1990, 1991, 1992, DOYLE & ASSOC., INC.   VERSION 05.22
GAME( 1992, hoopshot,   0,    piggypas, piggypas, piggypas_state,     empty_init, ROT0, "Doyle & Assoc.",               "Hoop Shot (version 05.22)",               MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// Quick $ilver   Development Co.    10/08/96      ROUND  REV 6
GAME( 1996, rndrndqs,   0,    fidlstix, piggypas, piggypas_state,     empty_init, ROT0, "Quick $ilver Development Co.", "Round and Round (Rev 6) (Quick $ilver)",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// Quick$ilver   Development Co.    10/02/95      -FIDDLESTIX-       REV 15T
GAME( 1995, fidlstix,   0,    fidlstix, piggypas, piggypas_state,     empty_init, ROT0, "Quick $ilver Development Co.", "Fiddle Stix (1st Rev)",                   MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// Quick $ilver   Development Co.    11/20/95     HoopItUp REV 23
GAME( 1995, hoopitup,   0,    fidlstix, piggypas, piggypas_state,     empty_init, ROT0, "Atari Games",                  "Hoop it Up World Tour - 3 on 3 (Rev 23)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// Quick $ilver   Development Co.    07/25/95     HoopItUp REV 21
GAME( 1995, hoopitup21, hoopitup, fidlstix, piggypas, piggypas_state, empty_init, ROT0, "Atari Games",                  "Hoop it Up World Tour - 3 on 3 (Rev 21)", MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, jackbean,   0,    piggypas, piggypas, piggypas_state,     empty_init, ROT0, "Doyle & Assoc.",               "Jack & The Beanstalk (Doyle & Assoc.?)",  MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, dumpump,    0,    piggypas, piggypas, piggypas_state,     empty_init, ROT0, "Doyle & Assoc.",               "Dump The Ump",                            MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
// bad dump, so version unknown
GAME( 199?, 3lilpigs,   0,    piggypas, piggypas, piggypas_state,     empty_init, ROT0,                                 "Doyle & Assoc.", "3 Lil' Pigs",           MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
