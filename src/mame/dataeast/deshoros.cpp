// license:BSD-3-Clause
// copyright-holders:Angelo Salese, hap
/***************************************************************************

Destiny (c) 1983 Data East Corporation

driver by Angelo Salese

A fortune-teller machine with 20 green 5x7 LED dot matrix display and a printer.
M6809 CPU, 2KB RAM
It is not Y2K compliant.

Rough cpanel sketch:

    [LED-array display]         1  2  3  M
                                4  5  6  F
                                7  8  9  0
                                CLEAR ENTER

To control system buttons (SYSTEM, lower nibble), hold one down and then
push the main service button F2.


TODO:
- Identify display controller and dump the character ROM,
  then emulate the graphics with genuine artwork display;
- Printer emulation;
- Exact sound & irq frequency;

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/beep.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class destiny_state : public driver_device
{
public:
	destiny_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper")
	{ }

	void destiny(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	required_device<cpu_device> m_maincpu;
	required_device<beep_device> m_beeper;

	char m_led_array[21];

	void firq_ack_w(uint8_t data);
	void nmi_ack_w(uint8_t data);
	uint8_t printer_status_r();
	uint8_t display_ready_r();
	void display_w(uint8_t data);
	void out_w(uint8_t data);
	void bank_select_w(uint8_t data);
	void sound_w(offs_t offset, uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
public:
	uint32_t screen_update_destiny(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*Temporary,to show something on screen...*/

void destiny_state::video_start()
{
	uint8_t i;
	for(i=0;i<20;i++)
		m_led_array[i] = 0x20;
	m_led_array[20] = 0;
}

uint32_t destiny_state::screen_update_destiny(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	popmessage("%s",m_led_array);
	return 0;
}



/***************************************************************************

  I/O

***************************************************************************/

void destiny_state::firq_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

void destiny_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint8_t destiny_state::printer_status_r()
{
	// d2: mark sensor
	// d3: motor stop
	// d4: status
	// d5: /L-SW
	// d6: paper
	// d7: /R-SW
	return 0xff;
}

uint8_t destiny_state::display_ready_r()
{
	// d7: /display ready
	// other bits: N/C
	return 0;
}

void destiny_state::display_w(uint8_t data)
{
	/* this is preliminary, just fills a string and doesn't support control codes etc. */

	// scroll the data
	for (int i = 0; i < 19; i++)
		m_led_array[i] = m_led_array[i+1];

	// update
	m_led_array[19] = data;
}

void destiny_state::out_w(uint8_t data)
{
	// d0: coin blocker
	machine().bookkeeping().coin_lockout_w(0, ~data & 1);

	// d1: paper cutter 1
	// d2: paper cutter 2
	// other bits: N/C?
}

void destiny_state::bank_select_w(uint8_t data)
{
	// d0-d2 and d4: bank (but only up to 4 banks supported)
	membank("bank1")->set_base(memregion("answers")->base() + 0x6000 * (data & 3));
}

INPUT_CHANGED_MEMBER(destiny_state::coin_inserted)
{
	// NMI on Coin SW or Service SW
	if (oldval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// coincounter on coin insert
	if (param == 0)
		machine().bookkeeping().coin_counter_w(0, newval);
}

void destiny_state::sound_w(offs_t offset, uint8_t data)
{
	// a0: sound on/off
	m_beeper->set_state(~offset & 1);
}

void destiny_state::main_map(address_map &map)
{
	map(0x0000, 0x5fff).bankr("bank1");
	map(0x8000, 0x87ff).ram();
	map(0x9000, 0x9000).rw(FUNC(destiny_state::printer_status_r), FUNC(destiny_state::firq_ack_w));
	map(0x9001, 0x9001).portr("SYSTEM").w(FUNC(destiny_state::nmi_ack_w));
	map(0x9002, 0x9002).rw(FUNC(destiny_state::display_ready_r), FUNC(destiny_state::display_w));
	map(0x9003, 0x9003).portr("KEY1");
	map(0x9004, 0x9004).portr("KEY2");
	map(0x9005, 0x9005).portr("DIPSW").w(FUNC(destiny_state::out_w));
//  map(0x9006, 0x9006).noprw(); // printer motor on
//  map(0x9007, 0x9007).noprw(); // printer data
	map(0x900a, 0x900b).w(FUNC(destiny_state::sound_w));
	map(0x900c, 0x900c).w(FUNC(destiny_state::bank_select_w));
//  map(0x900d, 0x900d).noprw(); // printer motor off
//  map(0x900e, 0x900e).noprw(); // printer motor jam reset
	map(0xc000, 0xffff).rom();
}



/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( destiny )
	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key Male") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key Female") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 5") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 4") PORT_CODE(KEYCODE_4_PAD)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Key Clear") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("DIPSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1:5" )
	PORT_DIPNAME( 0x20,   0x00, "Force Start" )         PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0,   0x00, "Operation Mode" )      PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x00, "Normal Mode" )
//  PORT_DIPSETTING(      0x40, "Normal Mode" ) // dupe
	PORT_DIPSETTING(      0x80, "Test Mode" )
	PORT_DIPSETTING(      0xc0, "I/O Test" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_NAME("Paper Cutter Reset")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("Paper Cutter Set")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Paper Cutter Point")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Spear") // starts game
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, destiny_state, coin_inserted, 0)

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CHANGED_MEMBER(DEVICE_SELF, destiny_state, coin_inserted, 1)
INPUT_PORTS_END



/***************************************************************************

  Machine Config(s)

***************************************************************************/

void destiny_state::machine_start()
{
}

void destiny_state::machine_reset()
{
	bank_select_w(0);
}

void destiny_state::destiny(machine_config &config)
{
	/* basic machine hardware */
	M6809(config, m_maincpu, XTAL(4'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &destiny_state::main_map);
	m_maincpu->set_periodic_int(FUNC(destiny_state::irq0_line_hold), attotime::from_hz(50)); // timer irq controls update speed, frequency needs to be determined yet (2MHz through three 74LS390)

	/* video hardware (dummy) */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(6*16, 9*2);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(destiny_state::screen_update_destiny));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 800); // TODO: determine exact frequency thru schematics
	m_beeper->add_route(ALL_OUTPUTS, "mono", 0.50);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( destiny )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ag12-4.16c", 0xc000, 0x2000, CRC(03b2c850) SHA1(4e2c49a8d80bc559d0f406caddddb85bc107aac0) )
	ROM_LOAD( "ag13-4.17c", 0xe000, 0x2000, CRC(36959ef6) SHA1(9b3ed44416fcda6a8e89d11ad6e713abd4f63d83) )

	ROM_REGION( 0x18000, "answers", 0 )
	ROM_LOAD( "ag00.1a",   0x00000, 0x2000, CRC(77f5bce0) SHA1(20b5257710c5e848893fec107f0d87a473a4ba24) )
	ROM_LOAD( "ag01.3a",   0x02000, 0x2000, CRC(c08e6a74) SHA1(88679ed8bd2b6b8698258baddf8433c0f60a1b64) )
	ROM_LOAD( "ag02.4a",   0x04000, 0x2000, CRC(687c72b5) SHA1(3f2768c9b6247e96d11b4159f6f5c0dfeb2c5075) )
	ROM_LOAD( "ag03.6a",   0x06000, 0x2000, CRC(535dbe83) SHA1(29336539c57d1fa7d42a0ce01884b29e1707e9ad) )
	ROM_LOAD( "ag04.7a",   0x08000, 0x2000, CRC(e6ae8eb7) SHA1(d0e20e438dcfeac9d844d1fd98701a443ea5e4f7) )
	ROM_LOAD( "ag05.9a",   0x0a000, 0x2000, CRC(c2485e40) SHA1(03f6d7c63a45d430a7965e28aaf07e053ecac7a1) )
	ROM_LOAD( "ag06.10a",  0x0c000, 0x2000, CRC(e6e0bbd1) SHA1(fe693d038b05ae18a3c0cfb25a4649dbb10ab2c7) )
	ROM_LOAD( "ag07.12a",  0x0e000, 0x2000, CRC(a62d879d) SHA1(94d07e774df4c9e4e34ae386714372b53b255530) )
	ROM_LOAD( "ag08.13a",  0x10000, 0x2000, CRC(f5822738) SHA1(afe53e875057317033cdd5f4b7614c96cd11193b) )
	ROM_LOAD( "ag09.15a",  0x12000, 0x2000, CRC(ad3c9f2c) SHA1(f665efb65c072a3d3d2e19844ebe0b352c0251d3) )
	ROM_LOAD( "ag10.16a",  0x14000, 0x2000, CRC(c498754a) SHA1(90e215e8e41d32237d1f4b074d93e20eade92e4e) )
	ROM_LOAD( "ag11.18a",  0x16000, 0x2000, CRC(5f7bf9f9) SHA1(281f89c0bccfcc2bdc1d4d0a5b9cc9a8ab2e7869) )
ROM_END

} // anonymous namespace


GAME( 1983, destiny, 0, destiny,  destiny, destiny_state, empty_init, ROT0, "Data East Corporation", "Destiny - The Fortuneteller (USA)", MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING|MACHINE_IMPERFECT_GRAPHICS|MACHINE_NODEVICE_PRINTER )
