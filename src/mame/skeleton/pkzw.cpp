// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************
No hw available.
w78e065.
i8279.
x2 ay8910.
ym2413.
x1 6116.
Four 74273 equivalent to two chips of 8255.
**************************************************************************/

#include "emu.h"

#include "cpu/mcs51/i80c52.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8279.h"

#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "speaker.h"

#include "pkzw.lh"


namespace {

class pkzw_state : public driver_device
{
public:
	pkzw_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "DSW", "PUSHBUTTONS" })
		, m_p1(*this, "P1")
		, m_hopper(*this, "hopper")
	
	{ }

	void pkzw(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void leda_w(uint8_t data) ATTR_COLD;
    void ledb_w(uint8_t data) ATTR_COLD;
    void ledc_w(uint8_t data) ATTR_COLD;
    void ledd_w(uint8_t data) ATTR_COLD;
	
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);
	uint8_t keyboard_r();
	uint8_t i80c31_p1_r();
	void i80c31_p1_w(uint8_t data);
		void p3_port_w(uint8_t data);

	void data_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	uint8_t m_selected_7seg_module = 0;
	uint8_t m_p1_out = 0xff;

	output_finder<32> m_digits;
	output_finder<72> m_leds;
	required_ioport_array<4> m_inputs;
	required_ioport m_p1;
	required_device<hopper_device> m_hopper;

};

static INPUT_PORTS_START( pkzw )
	PORT_START("KEYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 1" ) PORT_CODE( KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 2" ) PORT_CODE( KEYCODE_2_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 3" ) PORT_CODE( KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 4" ) PORT_CODE( KEYCODE_4_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 5" ) PORT_CODE( KEYCODE_5_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 6" ) PORT_CODE( KEYCODE_6_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 7" ) PORT_CODE( KEYCODE_7_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 8" ) PORT_CODE( KEYCODE_8_PAD)
	PORT_START("KEYS2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) 
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("Bonus")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON7) PORT_NAME("Credit")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH) 
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) 
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START ) PORT_NAME("Start")

	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) // K0
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) // K1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) // K2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) // K3
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM)  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))  
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYOUT ) PORT_CODE(KEYCODE_7)

INPUT_PORTS_END

void pkzw_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

void pkzw_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void pkzw_state::ay2_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(data, i);
}

void pkzw_state::ay2_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 24] = BIT(data, i);

}

void pkzw_state::leda_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 32] = BIT(~data, i);
}

void pkzw_state::ledb_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 40] = BIT(~data, i);
}

void pkzw_state::ledc_w(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
	m_leds[i + 48] = BIT(~data, i);
}

void pkzw_state::ledd_w(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
	m_leds[i + 56] = BIT(~data, i);
}



void pkzw_state::p3_port_w(uint8_t data)  
{
for (uint8_t i = 0; i < 8; i++)
	m_leds[i + 64] = BIT(data, i);
}
void pkzw_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t pkzw_state::keyboard_r()
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

uint8_t pkzw_state::i80c31_p1_r()
{
	// meter feedback is read here. Fails with error 02 if it doesn't get the expected value.
	uint8_t const ioport_val = m_p1->read();
	uint8_t meter_fb = 0x00;

	if (!BIT(m_p1_out, 0))
		meter_fb = (BIT(m_p1_out, 1) << 4) | (BIT(m_p1_out, 2) << 5);

	return (ioport_val & 0xcf) | meter_fb;
}

void pkzw_state::i80c31_p1_w(uint8_t data)
{
	m_hopper->motor_w(BIT(data, 3));

	m_p1_out = data;
}

void pkzw_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // HEF4511BP (7 seg display driver)

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void pkzw_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void pkzw_state::data_map(address_map &map)
{
map(0x8000, 0x87ff).ram().share("nvram");
map(0xb000, 0xb001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
map(0x9002, 0x9003).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
map(0x9006, 0x9007).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
map(0xa220, 0xa22f).ram();
map(0xc000, 0xc001).w("opll", FUNC(ym2413_device::write));
map(0xd000, 0xd000).w(FUNC(pkzw_state::leda_w));
map(0xd001, 0xd001).w(FUNC(pkzw_state::ledb_w));
map(0xd002, 0xd002).w(FUNC(pkzw_state::ledc_w));
map(0xd003, 0xd003).w(FUNC(pkzw_state::ledd_w));
}

void pkzw_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_selected_7seg_module));
	save_item(NAME(m_p1_out));
}

void pkzw_state::pkzw(machine_config &config)
{

	/* basic machine hardware */
	i80c52_device &maincpu(I80C52(config, "maincpu", XTAL(10'738'635))); //actual CPU is a Winbond w78e065
	maincpu.set_addrmap(AS_PROGRAM, &pkzw_state::program_map);
	maincpu.set_addrmap(AS_DATA, &pkzw_state::data_map);
	maincpu.port_in_cb<1>().set(FUNC(pkzw_state::i80c31_p1_r));
	maincpu.port_out_cb<1>().set(FUNC(pkzw_state::i80c31_p1_w));
	maincpu.port_out_cb<3>().set(FUNC(pkzw_state::p3_port_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, m_hopper, attotime::from_msec(100));

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'738'635) / 6));
	kbdc.out_sl_callback().set(FUNC(pkzw_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(pkzw_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(pkzw_state::display_7seg_data_w));

	/* Video */
	config.set_default_layout(layout_pkzw);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(pkzw_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(pkzw_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(10'738'635) / 6));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(pkzw_state::ay2_port_a_w));
	ay2.port_b_write_callback().set(FUNC(pkzw_state::ay2_port_b_w));
    ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);


}

// PK之王 (PK Zhīwáng) (King of PK)
ROM_START( pkzw )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "w78e065", 0x0000, 0x8000, CRC(9bc248c8) SHA1(42574f765f38dbb50380dffd6227c064a75df5ca) )

	ROM_REGION( 0x800, "nvram", 0 )
	ROM_LOAD( "nvram", 0x000, 0x800, CRC(d67da182) SHA1(12d51eae0eb0c15b04db58b7419dedf793cf44f7) )
ROM_END

} // anonymous namespace


//    YEAR  NAME  PARENT   MACHINE  INPUT  STATE       INIT        ROT   COMPANY      FULLNAME       FLAGS
GAME( 199?, pkzw, 0,       pkzw,    pkzw,  pkzw_state, empty_init, ROT0, "<unknown>", "PK Zhiwang",  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
