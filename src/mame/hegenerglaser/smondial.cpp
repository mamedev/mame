// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:yoyo_chessboard
/*******************************************************************************

Mephisto Super Mondial
Mephisto Super Mondial II
Mephisto Mega IV

Hardware notes:
- G65SC02P-4 @ 4MHz
- 8KB RAM(battery-backed), 32KB ROM
- expansion slot at underside (only used for smondial2)
- 2*PCF2112, 2 7seg LCD screens
- 8*8 chessboard buttons, 24 tri-color leds, piezo

Undocumented buttons:
- smondialb: holding CL+INFO on boot runs diagnostics

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/m65sc02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pcf2100.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "mephisto_mega4.lh"
#include "mephisto_smondial.lh"
#include "mephisto_smondial2.lh"


namespace {

// Super Mondial B / shared

class smondialb_state : public driver_device
{
public:
	smondialb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_lcd_latch(*this, "lcd_latch"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd(*this, "lcd%u", 0),
		m_dac(*this, "dac"),
		m_keys(*this, "KEY.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	// machine configs
	void smondial2(machine_config &config);
	void mega4(machine_config &config);
	void smondialb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<hc259_device> m_lcd_latch;
	required_device<pwm_display_device> m_led_pwm;
	required_device_array<pcf2112_device, 2> m_lcd;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_keys;
	output_finder<8> m_digits;

	u8 m_led_data = 0;
	u8 m_board_mux = 0;

	// address maps
	void smondialb_mem(address_map &map) ATTR_COLD;
	void smondial2_mem(address_map &map) ATTR_COLD;

	// I/O handlers
	template<int N> void lcd_output_w(u32 data);
	void update_leds();
	u8 input_r(offs_t offset);
	virtual void led_w(u8 data);
	void board_w(u8 data);
	INTERRUPT_GEN_MEMBER(nmi_handler);
};

void smondialb_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_led_data));
	save_item(NAME(m_board_mux));
}


// Super Mondial A

class smondiala_state : public smondialb_state
{
public:
	smondiala_state(const machine_config &mconfig, device_type type, const char *tag) :
		smondialb_state(mconfig, type, tag)
	{ }

	void smondiala(machine_config &config);

private:
	void smondiala_mem(address_map &map) ATTR_COLD;

	// led row/column is switched around, do a trampoline here instead of making a different .lay file
	virtual void led_w(u8 data) override { smondialb_state::led_w(bitswap<8>(data, 7,6,3,2,5,4,1,0)); }

	void irq_ack_w(u8 data);
};



/*******************************************************************************
    I/O
*******************************************************************************/

template<int N>
void smondialb_state::lcd_output_w(u32 data)
{
	// lcd segment outputs
	for (int i = 0; i < 4; i++)
		m_digits[i + N*4] = bitswap<8>(data >> (8 * i), 7,4,5,0,1,2,3,6);
}

void smondialb_state::update_leds()
{
	m_led_pwm->matrix(m_board_mux, m_led_data);
}

void smondialb_state::board_w(u8 data)
{
	m_board_mux = ~data;
	update_leds();
}

void smondialb_state::led_w(u8 data)
{
	// d0-d5: led data
	m_led_data = data;
	update_leds();

	// d6: nmi enable (does not apply to smondiala)
	// d7: speaker out
	m_dac->write(BIT(data, 7));
}

u8 smondialb_state::input_r(offs_t offset)
{
	u8 data = 0;

	// read keypad
	for (int i = 0; i < 4; i++)
		if (!BIT(m_lcd_latch->output_state(), i))
			data |= BIT(m_keys[i]->read(), offset & 3);

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_board_mux, i))
			data |= BIT(m_board->read_rank(i), offset);

	return ~(data << 7);
}

INTERRUPT_GEN_MEMBER(smondialb_state::nmi_handler)
{
	if (m_led_data & 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void smondiala_state::irq_ack_w(u8 data)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void smondialb_state::smondialb_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2400, 0x2400).w(FUNC(smondialb_state::led_w));
	map(0x2800, 0x2800).w(FUNC(smondialb_state::board_w));
	map(0x2c00, 0x2c07).w(m_lcd_latch, FUNC(hc259_device::write_d7)).nopr();
	map(0x3000, 0x3007).r(FUNC(smondialb_state::input_r));
	map(0x8000, 0xffff).rom();
}

void smondialb_state::smondial2_mem(address_map &map)
{
	smondialb_mem(map);
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}

void smondiala_state::smondiala_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4007).r(FUNC(smondiala_state::input_r));
	map(0x6000, 0x6000).w(FUNC(smondiala_state::irq_ack_w));
	map(0x6400, 0x6407).w("led_latch", FUNC(hc259_device::write_d7)).nopr();
	map(0x6800, 0x6807).w("board_latch", FUNC(hc259_device::write_d7)).nopr();
	map(0x6c00, 0x6c07).w(m_lcd_latch, FUNC(hc259_device::write_d7)).nopr();
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( smondial )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Pawn / 1")   PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Queen / 5")  PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("BOOK / 9")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")         PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Knight / 2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("King / 6")   PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS / 0")    PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_P)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")        PORT_CODE(KEYCODE_L)

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Bishop / 3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Black / 7")  PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")        PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("ENT")        PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("Rook / 4")   PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("White / 8")  PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO")       PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("RES")        PORT_CODE(KEYCODE_F1)
INPUT_PORTS_END

static INPUT_PORTS_START( smondial2 )
	PORT_INCLUDE( smondial )

	PORT_MODIFY("KEY.0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("HELP / 9")   PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("LEV")        PORT_CODE(KEYCODE_L)

	PORT_MODIFY("KEY.1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("INFO / 0")   PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("CL")         PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_MODIFY("KEY.2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("POS")        PORT_CODE(KEYCODE_P)

	PORT_MODIFY("KEY.3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)    PORT_NAME("MEM")        PORT_CODE(KEYCODE_M)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void smondialb_state::smondialb(machine_config &config)
{
	// basic machine hardware
	M65SC02(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &smondialb_state::smondialb_mem);

	const attotime nmi_period = attotime::from_hz(4_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(smondialb_state::nmi_handler), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	HC259(config, m_lcd_latch);
	m_lcd_latch->q_out_cb<4>().set(m_lcd[0], FUNC(pcf2112_device::data_w));
	m_lcd_latch->q_out_cb<4>().append(m_lcd[1], FUNC(pcf2112_device::data_w));
	m_lcd_latch->q_out_cb<5>().set(m_lcd[1], FUNC(pcf2112_device::dlen_w));
	m_lcd_latch->q_out_cb<6>().set(m_lcd[0], FUNC(pcf2112_device::clb_w));
	m_lcd_latch->q_out_cb<6>().append(m_lcd[1], FUNC(pcf2112_device::clb_w));
	m_lcd_latch->q_out_cb<7>().set(m_lcd[0], FUNC(pcf2112_device::dlen_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));

	// video hardware
	PCF2112(config, m_lcd[0], 50); // frequency guessed
	m_lcd[0]->write_segs().set(FUNC(smondialb_state::lcd_output_w<0>));
	PCF2112(config, m_lcd[1], 50); // "
	m_lcd[1]->write_segs().set(FUNC(smondialb_state::lcd_output_w<1>));

	PWM_DISPLAY(config, m_led_pwm).set_size(8, 6);
	config.set_default_layout(layout_mephisto_smondial);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void smondiala_state::smondiala(machine_config &config)
{
	smondialb(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &smondiala_state::smondiala_mem);

	const attotime irq_period = attotime::from_hz(4_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(smondiala_state::irq0_line_assert), irq_period);

	HC259(config, "led_latch").parallel_out_cb().set(FUNC(smondiala_state::led_w)).invert();
	HC259(config, "board_latch").parallel_out_cb().set(FUNC(smondiala_state::board_w)).invert();
}

void smondialb_state::smondial2(machine_config &config)
{
	smondialb(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &smondialb_state::smondial2_mem);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "smondial2_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_smondial2");

	config.set_default_layout(layout_mephisto_smondial2);
}

void smondialb_state::mega4(machine_config &config)
{
	smondialb(config);

	// basic machine hardware
	R65C02(config.replace(), m_maincpu, 4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &smondialb_state::smondialb_mem);

	const attotime nmi_period = attotime::from_hz(4.9152_MHz_XTAL / 0x2000);
	m_maincpu->set_periodic_int(FUNC(smondialb_state::nmi_handler), nmi_period);

	config.set_default_layout(layout_mephisto_mega4);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( smondial )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_a.bin", 0x8000, 0x8000, CRC(c1d7d0a5) SHA1(d7f0da6938458c06925f0936e63915319144d7e0) )
ROM_END

ROM_START( smondialab )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_ab.bin", 0x8000, 0x8000, CRC(a8781685) SHA1(fd4c97e13bd398dc4c85e3e1778bf7e59fccd71e) )
ROM_END

ROM_START( smondialb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_b.bin", 0x8000, 0x8000, CRC(6fb89e97) SHA1(b001e657b4fdc097322b28a25c31814f3da7b124) )
ROM_END


ROM_START( smondial2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("supermondial_ii.bin", 0x8000, 0x8000, CRC(cd73df4a) SHA1(bad786074be613d7f48bf98b6fdf8178a4a85f5b) )
ROM_END


ROM_START( mega4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mega_iv.bin", 0x8000, 0x8000, CRC(dee355d2) SHA1(6bc79c0fb169020f017412f5f9696b9ecafbf99f) )
ROM_END

ROM_START( mega4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mega_iv_17.03.88", 0x8000, 0x8000, CRC(85267e82) SHA1(654c9cf84bf2165fc94f8c4cf9c662786ef3283b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME        PARENT    COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, smondial,   0,        0,      smondiala, smondial,  smondiala_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. A)",  MACHINE_SUPPORTS_SAVE )
SYST( 1986, smondialab, smondial, 0,      smondiala, smondial,  smondiala_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. AB)", MACHINE_SUPPORTS_SAVE )
SYST( 1986, smondialb,  smondial, 0,      smondialb, smondial,  smondialb_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial (ver. B)",  MACHINE_SUPPORTS_SAVE )

SYST( 1988, smondial2,  0,        0,      smondial2, smondial2, smondialb_state, empty_init, "Hegener + Glaser", "Mephisto Super Mondial II", MACHINE_SUPPORTS_SAVE )

SYST( 1988, mega4,      0,        0,      mega4,     smondial,  smondialb_state, empty_init, "Hegener + Glaser", "Mephisto Mega IV (set 1)",  MACHINE_SUPPORTS_SAVE )
SYST( 1988, mega4a,     mega4,    0,      mega4,     smondial,  smondialb_state, empty_init, "Hegener + Glaser", "Mephisto Mega IV (set 2)",  MACHINE_SUPPORTS_SAVE )
