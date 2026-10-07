// license::BSD-3-Clause
// copyright-holders:flama12333

/*************************************************************************

hw info:
pcb labeled xml1-a
x1 nec 82c55ac 2 221
x1 z084004psc  z80 cpu
x1 hm6116p-3
x1hm6116lp-3
x2 jfc 95101
x1 m5l8253p-5
x1 m5l8279p-5

Todo:
Meter in and out. Hopper hook up.
Inputs are from mscbar.cpp

**************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "speaker.h"



namespace {

class maryz80_state : public driver_device
{
public:
	maryz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "KEYS3", "DSW" })
        , m_hopper(*this, "hopper")

	{ }

	void maryz80(machine_config &config);
	

protected:
	virtual void machine_start() override;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);
	void ppi_port_a_w(uint8_t data);
	void ppi_port_b_w(uint8_t data);
	void ppi_port_c_w(uint8_t data);

//	void out_w(u8 data) ATTR_COLD;

	uint8_t keyboard_r();
	void io_map(address_map &map);
	void program_map(address_map &map);
	void program_map2(address_map &map);
	uint8_t m_selected_7seg_module = 0;
	uint8_t m_p1_out = 0xff;
	
	output_finder<32> m_digits;
	output_finder<80> m_leds;
	required_ioport_array<4> m_inputs;
    required_device<hopper_device> m_hopper;

};

static INPUT_PORTS_START( maryz80 )

	PORT_START("KEYS1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 9") PORT_CODE( KEYCODE_9_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 8") PORT_CODE( KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 7") PORT_CODE( KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 6")PORT_CODE( KEYCODE_6_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 1") PORT_CODE( KEYCODE_1_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 2") PORT_CODE( KEYCODE_2_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 3") PORT_CODE( KEYCODE_3_PAD)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Bet 4") PORT_CODE( KEYCODE_4_PAD)
	
	PORT_START("KEYS2")
	PORT_BIT(0x01, IP_ACTIVE_LOW,  IPT_START1) PORT_NAME("Start / Take Score")
	PORT_BIT(0x02, IP_ACTIVE_LOW,  IPT_GAMBLE_LOW)
	PORT_BIT(0x04, IP_ACTIVE_LOW,  IPT_GAMBLE_HIGH) 
	PORT_BIT(0x08, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT ) PORT_NAME("Payout") 
	PORT_BIT(0x10, IP_ACTIVE_LOW,  IPT_KEYPAD)  PORT_NAME("Bet 5") PORT_CODE( KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW,  IPT_BUTTON5 ) PORT_NAME( "Unknown" )
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_BUTTON6)  PORT_NAME("Bonus") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_BUTTON7)  PORT_NAME("Credits") PORT_CODE(KEYCODE_Q) 

	PORT_START("KEYS3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K0") // K0
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K1") // K1
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K2") // K2
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K3") // K3
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("pa")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "pa:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "pa:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "pa:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "pa:4") 
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "pa:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "pa:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "pa:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "pa:8")
	
	PORT_START("pb")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "pb:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "pb:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "pb:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "pb:4") 
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "pb:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "pb:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "pb:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "pb:8")

	PORT_START("pc")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "pc:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "pc:2")
   	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "pc:3")  // Coin?
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "pc:4") // For Hopper?.
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "pc:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "pc:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "pc:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "pc:8")
	
	PORT_START("DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "DSW:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "DSW:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "DSW:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "DSW:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW:8")


INPUT_PORTS_END



void maryz80_state::ay1_port_a_w(uint8_t data) // roulette 1
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);

}

void maryz80_state::ay1_port_b_w(uint8_t data) // for bet win blink and double up
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(data, i);
}

void maryz80_state::ay2_port_a_w(uint8_t data)  // roulette 2
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(data, i);
}
 
void maryz80_state::ay2_port_b_w(uint8_t data) // roulette 3?
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 24] = BIT(data, i);
}

void maryz80_state::ppi_port_a_w(uint8_t data) 
{
	for (uint8_t i = 4; i < 8; i++)
	 logerror("Port a Write to %02x\n", data);

}

void maryz80_state::ppi_port_b_w(uint8_t data) 
{
	for (uint8_t i = 4; i < 8; i++)
		 logerror("Port b Write to %02x\n", data);

}
void maryz80_state::ppi_port_c_w(uint8_t data) 
{
	for (uint8_t i = 4; i < 8; i++)
		 logerror("Port c Write to %02x\n", data);
	
}

void maryz80_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t maryz80_state::keyboard_r()
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

void maryz80_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // from marywu code.

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}


void maryz80_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
    map(0xc000, 0xc7ff).ram().share("ram1");
    map(0xe000, 0xe7ff).ram().share("ram2");

}

void maryz80_state::program_map2(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);

}

void maryz80_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x00).ram();
	map(0x40, 0x43).rw("pit1", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x80, 0x81).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x82, 0x83).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xa0, 0xa3).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write)); // for hopper?
	map(0xc0, 0xc1).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

void maryz80_state::machine_start()
{
	save_item(NAME(m_selected_7seg_module));
	save_item(NAME(m_p1_out));

}

void maryz80_state::maryz80(machine_config &config)
{

	/* basic machine hardware */
	z80_device &maincpu(Z80(config, "maincpu", XTAL(3'580'000)));
	maincpu.set_addrmap(AS_PROGRAM, &maryz80_state::program_map);
	maincpu.set_addrmap(AS_IO, &maryz80_state::io_map);
    NVRAM(config, "ram1", nvram_device::DEFAULT_ALL_0);
    NVRAM(config, "ram2", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi(I8255(config, "ppi1"));
	ppi.in_pa_callback().set_ioport("pa");
	ppi.in_pa_callback().set_ioport("pb");
    ppi.in_pc_callback().set_ioport("pc");
	
	ppi.out_pa_callback().set(FUNC(maryz80_state::ppi_port_a_w));
	ppi.out_pa_callback().set(FUNC(maryz80_state::ppi_port_b_w));
	ppi.out_pa_callback().set(FUNC(maryz80_state::ppi_port_c_w));

	pit8253_device &pit(PIT8253(config, "pit1", 0)); // m5l8253p-5; unknown clocks
	pit.set_clk<0>(XTAL(3'580'000) / 2);
	pit.out_handler<0>().set_inputline("maincpu", INPUT_LINE_IRQ0);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(3'580'000) / 2));
	kbdc.out_sl_callback().set(FUNC(maryz80_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(maryz80_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(maryz80_state::display_7seg_data_w));

  /* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(3'580'000) / 2));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
  ay1.port_a_write_callback().set(FUNC(maryz80_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(maryz80_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(3'580'000) / 2));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(maryz80_state::ay2_port_a_w)); 
	ay2.port_b_write_callback().set(FUNC(maryz80_state::ay2_port_b_w));

	HOPPER(config, m_hopper, attotime::from_msec(10));

}

ROM_START( xiaomali )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "xiaomali.bin", 0x0000, 0x4000, CRC(4A2A2C15) SHA1(7329ee768d938b723596c2ff97f83ae1c2d60366) ) // 16kb. 
ROM_END

ROM_START( xiaomali2 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "xiaomali2.bin", 0x0000, 0x4000, CRC(8E0ECF6F) SHA1(b21b2cc5893723c1efc97d6363fb6391b2b0ca5e) ) // 16kb. 
ROM_END

ROM_START( xiaomali2b )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "xiaomali2b.bin", 0x0000, 0x4000, CRC(006DED91) SHA1(7bc8e41638d2c490ad06b3c01d3c4574e3056e50) ) // 16kb. 
ROM_END

} // anonymous namespace

//    YEAR  NAME        PARENT   MACHINE       INPUT    STATE          INIT        ROT   COMPANY      FULLNAME                  FLAGS
GAME( ????, xiaomali,   0,       maryz80,      maryz80, maryz80_state, empty_init, ROT0, "<unknown>", "Xiao mali 1 ",           MACHINE_NOT_WORKING )
GAME( ????, xiaomali2,  0,       maryz80,      maryz80, maryz80_state, empty_init, ROT0, "<unknown>", "Xiao mali 2 ",           MACHINE_NOT_WORKING )
GAME( ????, xiaomali2b, 0,       maryz80,      maryz80, maryz80_state, empty_init, ROT0, "<unknown>", "Xiao mali 2 (sets 2)",   MACHINE_NOT_WORKING )
