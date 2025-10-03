// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
Driver For fruit cocktail - Roulette gambling board

Notes: (Wip)
K0+K1 = RESET
K0 = credits reset
K1 = Coin in/out display
K2 = Account coins? 
also Shown program date
1999 09 13 2002
K3 = Check account
K1+K3 = 0 to 19  value, Press play to enter, If is Incorrect will shown Error 21
Errors

00 RAM
02 IN/OUT Counter meters not detected at power on
04 ??? after tried to hex edit at 240 nvram
10 IC6116
12 OUT Counter error at payment
23 Also happen if NVRAM Are Not Reset after access credits reset. -
30 COIN error
32 HOPPER error at payment
33 Coin IN account error
34 Coin OUT account error
52 Keyboard pressed at power on, or coin in cheat
35 requires reset k0+k1 to write data on ram

Notes by the Dumper:
keyboard keys MUST NOT be pressed 
counters in-out must be NC (0, LOW)
coin acceptor NC 0 LOW
hopper sensor NC 0 LOW 

Hardware info: (Wip)

Board:
1x 8 dip switch
1x 4 dip switch
k0 k1 k2 k4 - misspelled as k3
at89c52 or Syncmos sm8951ac25p

gm76c28a-10 or p511ssl-10

m27c512-10f1 or w27c512
w27c020-70

?  - altera 7032 for I/O?
u2413 - ym2413
u6295 - oki m6295
?     - 8910 clone

Led board
X2 M82C55A-2
new8279

Todo:
Dip switch
Led Controller
Layout.

if press to insert coin will to error 02 or error 30 

debug:
pc = 424 - bypass error 02 error.

*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "speaker.h"
#include "marywu.lh"


namespace {

class fruitctk_state : public driver_device
{
public:
	fruitctk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "Inputs1", "Inputs2", "Inputs3", "Inputs4" })
		, m_hopper(*this, "hopper")
		, m_oki(*this, "oki")

	{ }

	void fruitctk(machine_config &config);
	void p1_w(uint8_t data);

protected:
	virtual void machine_start() override;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void io_map(address_map &map);
	void program_map(address_map &map);


	uint8_t keyboard_r();
	uint8_t m_selected_7seg_module = 0;
    output_finder<32> m_digits;
	output_finder<30> m_leds;
	required_ioport_array<4> m_inputs;
	required_device<hopper_device> m_hopper;
	required_device<okim6295_device> m_oki;

};


static INPUT_PORTS_START( fruitctk )

	PORT_START("Inputs1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_CODE(KEYCODE_Q) PORT_NAME("Bet 8")  
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_CODE(KEYCODE_W) PORT_NAME("Bet 7")  
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_CODE(KEYCODE_E) PORT_NAME("Bet 6")  
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_CODE(KEYCODE_R) PORT_NAME("Bet 5")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_T) PORT_NAME("Bet 4") 
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_Y) PORT_NAME("Bet 3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_U) PORT_NAME("Bet 2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_I) PORT_NAME("Bet 1")


	PORT_START("Inputs2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_GAMBLE_DEAL ) PORT_CODE(KEYCODE_O) PORT_NAME("Start")     
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_LOW  ) PORT_CODE(KEYCODE_P) // D-UP Low?
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_HIGH ) PORT_CODE(KEYCODE_A) // D-UP High?
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_CODE(KEYCODE_S) // unused?
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_CODE(KEYCODE_D) PORT_NAME("Credit")     
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_CODE(KEYCODE_F) PORT_NAME("Win")     
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_CODE(KEYCODE_G) // Payout?
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CODE(KEYCODE_H)// Coin. will cause error 30

	PORT_START("Inputs3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_J)  
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CODE(KEYCODE_B)

	PORT_START("Inputs4")
    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED  )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED  ) 
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K3") // K0
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K2") // K1
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K1") // K2
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K0") // K3
	
	PORT_START("P1")

    PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
    PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))  // is from hopper? will cause error 02 During reset nvram
    PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) 
    PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) 



PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")


PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
INPUT_PORTS_END




void fruitctk_state::p1_w(uint8_t data)
{

	m_hopper->motor_w(BIT(data, 3)); // ?? from guessed. Wrong
    logerror("P1.0 Write to %02x\n", data);

	
}



void fruitctk_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t fruitctk_state::keyboard_r()
{
	switch (m_selected_7seg_module & 0x07)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();
	default:
		return 0x00;
	}
}

void fruitctk_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // Code was from marywu to decode 7 segment display.

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void fruitctk_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void fruitctk_state::io_map(address_map &map)
{
map(0x8000, 0x87ff).ram().share("nvram");
map(0xc000, 0xc001).w("opll", FUNC(ym2413_device::write));
map(0x9002, 0x9003).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
map(0xb000, 0xb001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
map(0xb004, 0xb004).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
map(0xb008, 0xb00a).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write)); // ???
// map(0xb00b, 0xb00b).nopw(); // ?? Write = 80
// map(0xb00c, 0xb00c).nopw(); // Led
map(0xb00d, 0xb00f).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write)); // ???
}

void fruitctk_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();	
}

void fruitctk_state::fruitctk(machine_config &config)
{
	/* Video */
    config.set_default_layout(layout_marywu);
	
	/* basic machine hardware */
	i8051_device &maincpu(I8051(config, "maincpu", XTAL(12'000'000)));
	maincpu.set_addrmap(AS_PROGRAM, &fruitctk_state::program_map);
	maincpu.set_addrmap(AS_IO, &fruitctk_state::io_map);
	maincpu.port_in_cb<1>().set_ioport("P1");
	maincpu.port_out_cb<1>().set(FUNC(fruitctk_state::p1_w));

	HOPPER(config, m_hopper, attotime::from_msec(10));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
    i8279_device &kbdc(I8279(config, "i8279", XTAL(12'000'000) / 6)); // divisor not verified
	kbdc.out_sl_callback().set(FUNC(fruitctk_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(fruitctk_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(fruitctk_state::display_7seg_data_w));
  
    I8255A(config, "ppi1");
  	I8255A(config, "ppi2");
	

    /* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &ay1(AY8910(config, "ay1", XTAL(12'000'000) / 6)); // divisor not verified
	ay1.add_route(ALL_OUTPUTS, "mono", 1.0);
	ay1.port_a_read_callback().set_ioport("DSW1");
	ay1.port_b_read_callback().set_ioport("DSW2");

	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(12'000'000) / 10, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified

}

ROM_START( fruitctk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w27c512.bin", 0x0000, 0x10000, CRC(2A1B63C1) SHA1(631a95c684db961283f45b4bfe67fa58d79b2191) )


    ROM_REGION( 0x40000, "oki", 0 )
    ROM_LOAD( "w27c02.bin", 0x00000, 0x40000, CRC(A8217121) SHA1(a35004f00632b552b587dfa12c1698abada5e300) ) //  Voices Rom
    ROM_END


} // anonymous namespace


//    YEAR    NAME        PARENT   MACHINE       INPUT      STATE            INIT        ROT    COMPANY        FULLNAME            FLAGS
GAME( 2002?,  fruitctk,   0,       fruitctk,     fruitctk,  fruitctk_state,  empty_init, ROT0,  "<unknown>",  "Fruit Cocktail",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

