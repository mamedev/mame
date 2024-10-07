// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

AVE Micro Systems ARB chess computer driver, in some regions redistributed
by Chafitz, and in Germany by Sandy Electronic.

Auto Response Board (ARB) overview:
- R6502P CPU @ 2MHz(4MHz XTAL), R6522P VIA
- 2KB RAM(4*2114), cartridge port
- magnetic chessboard, 8*8+12 leds
- PCB label AV001C01 REV A

The electronic magnetic chessboard is the first of its kind. AVE later licensed
it to Fidelity (see fidelity/elite.cpp).

ARB is a romless system, the program ROM is on a cartridge.

Known chess modules:
- Grand Master Series 3
- Grand Master Series 4.0
- Sargon 2.5
- Sargon 3.5 (unofficial)

Other games:
- Avelan (checkers)

Sandy Electronic renamed GMS 3 and GMS 4.0 to "3000 GMS" and "4,0 - 50 S".
Sargon 3.5 was an unofficial module published by them. It was also a free EPROM
upgrade for their customers who were unhappy with GMS 3.

GMS 4.0 included button label stickers for OPTIONS, Verify, Take Back, Clear.

Around 2012, Steve Braid(aka Trilobyte/Steve UK) started manufacturing ARB V2
boards without a module slot. CPU and VIA were replaced with new WDC 14MHz-rated
chips, running at 16MHz.

TODO:
- avelan, gms3, gms4, sargon35 rom labels

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/w65c02s.h"
#include "video/pwm.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"

#include "speaker.h"
#include "softlist_dev.h"

// internal artwork
#include "ave_arb.lh"


namespace {

class arb_state : public driver_device
{
public:
	arb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_board(*this, "board"),
		m_via(*this, "via"),
		m_extram(*this, "extram", 0x800, ENDIANNESS_LITTLE),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// halt button is tied to NMI, reset button to RESET(but only if halt button is held)
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { update_reset(); }
	DECLARE_INPUT_CHANGED_MEMBER(halt_button) { m_maincpu->set_input_line(M6502_NMI_LINE, newval ? ASSERT_LINE : CLEAR_LINE); update_reset(); }
	void update_reset();

	void arb(machine_config &config);
	void v2(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_device<sensorboard_device> m_board;
	required_device<via6522_device> m_via;
	memory_share_creator<u8> m_extram;
	required_device<dac_1bit_device> m_dac;
	optional_device<generic_slot_device> m_cart;
	required_ioport_array<2> m_inputs;

	u16 m_inp_mux = 0;
	u16 m_led_select = 0;
	u8 m_led_group = 0;
	u8 m_led_latch = 0;
	u16 m_led_data = 0;

	bool m_altboard = false;

	void main_map(address_map &map) ATTR_COLD;
	void v2_map(address_map &map) ATTR_COLD;

	void init_board(u8 data);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void update_display();
	void leds_w(u8 data);
	void control_w(u8 data);
	u8 input_r();
};

void arb_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_group));
	save_item(NAME(m_led_latch));
	save_item(NAME(m_led_data));
}

void arb_state::update_reset()
{
	bool state = m_inputs[1]->read() == 3;

	// RESET goes to 6502+6522
	m_maincpu->set_input_line(INPUT_LINE_RESET, state ? ASSERT_LINE : CLEAR_LINE);
	if (state)
		m_via->reset();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// sensorboard

void arb_state::init_board(u8 data)
{
	// different board setup for checkers
	if (m_altboard)
	{
		for (int i = 0; i < 12; i++)
		{
			m_board->write_piece((i % 4) * 2 + ((i / 4) & 1), i / 4, 13); // white
			m_board->write_piece((i % 4) * 2 + (~(i / 4) & 1), i / 4 + 5, 14); // black
		}
	}
	else
		m_board->preset_chess(data);
}


// cartridge

DEVICE_IMAGE_LOAD_MEMBER(arb_state::cart_load)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	// extra ram (optional)
	if (image.get_feature("ram"))
		m_maincpu->space(AS_PROGRAM).install_ram(0x0800, 0x0fff, 0x1000, m_extram);

	m_altboard = bool(image.get_feature("altboard"));

	return std::make_pair(std::error_condition(), std::string());
}


// R6522 ports

void arb_state::update_display()
{
	// 12 led column data lines via 3 7475
	u16 mask = 0;
	mask |= (m_led_group & 1) ? 0xf00 : 0;
	mask |= (m_led_group & 2) ? 0x0ff : 0;

	m_led_data = (m_led_data & ~mask) | ((m_led_latch << 8 | m_led_latch) & mask);
	m_display->matrix(m_led_select | 0x200, m_led_data);
}

void arb_state::leds_w(u8 data)
{
	// PA0-PA7: led latch input
	m_led_latch = ~data & 0xff;
	update_display();
}

void arb_state::control_w(u8 data)
{
	// PB0-PB3: 74145 A-D
	// 74145 0-8: input mux, led row select
	m_inp_mux = data & 0xf;
	m_led_select = 1 << (data & 0xf) & 0x1ff;

	// PB4,PB5: led group select
	m_led_group = data >> 4 & 3;
	update_display();

	// PB7: speaker out
	m_dac->write(BIT(data, 7));
}

u8 arb_state::input_r()
{
	u8 data = 0;

	// PA0-PA7: multiplexed inputs
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);
	else if (m_inp_mux < 9)
		data = m_inputs[m_inp_mux - 8]->read();

	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void arb_state::main_map(address_map &map)
{
	// external slot is A0-A14, potential bus conflict with RAM/VIA
	map(0x0000, 0x7fff).mirror(0x8000).r(m_cart, FUNC(generic_slot_device::read_rom));
	map(0x0000, 0x07ff).mirror(0x1000).ram().share("nvram");
	map(0x8000, 0x800f).mirror(0x1ff0).m(m_via, FUNC(via6522_device::map));
}

void arb_state::v2_map(address_map &map)
{
	map(0x0000, 0x7fff).ram().share("nvram"); // BS62LV256
	map(0x8000, 0x800f).mirror(0x1ff0).m(m_via, FUNC(via6522_device::map));
	map(0xa000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( arb )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_7) PORT_NAME("Hint / Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_6) PORT_NAME("Variable / Clear / White / 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_5) PORT_NAME("Monitor / Take Back / King / 5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_4) PORT_NAME("Self Play / Verify / Queen / 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_3) PORT_NAME("Change Board / Rook / 3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_2) PORT_NAME("Change Color / Bishop / 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_1) PORT_NAME("Change Level / Knight / 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_0) PORT_NAME("New Game / Options / Pawn / 0")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_F1) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, arb_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_CODE(KEYCODE_F1) PORT_NAME("Halt") PORT_CHANGED_MEMBER(DEVICE_SELF, arb_state, halt_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void arb_state::v2(machine_config &config)
{
	// basic machine hardware
	W65C02S(config, m_maincpu, 16_MHz_XTAL); // W65C02S6TPG-14
	m_maincpu->set_addrmap(AS_PROGRAM, &arb_state::v2_map);

	W65C22S(config, m_via, 16_MHz_XTAL); // W65C22S6TPG-14
	m_via->writepa_handler().set(FUNC(arb_state::leds_w));
	m_via->writepb_handler().set(FUNC(arb_state::control_w));
	m_via->readpa_handler().set(FUNC(arb_state::input_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(FUNC(arb_state::init_board));
	m_board->set_spawnpoints(12+2); // +2 checkers pieces
	m_board->set_delay(attotime::from_msec(100));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9+1, 12);
	config.set_default_layout(layout_ave_arb);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void arb_state::arb(machine_config &config)
{
	v2(config);

	// basic machine hardware
	M6502(config.replace(), m_maincpu, 4_MHz_XTAL/2); // R6502P
	m_maincpu->set_addrmap(AS_PROGRAM, &arb_state::main_map);

	MOS6522(config.replace(), m_via, 4_MHz_XTAL/4); // R6522P
	m_via->writepa_handler().set(FUNC(arb_state::leds_w));
	m_via->writepb_handler().set(FUNC(arb_state::control_w));
	m_via->readpa_handler().set(FUNC(arb_state::input_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "arb");
	m_cart->set_device_load(FUNC(arb_state::cart_load));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("arb");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( arb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

ROM_START( arbv2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sargon_4.0", 0x0000, 0x10000, CRC(c519c9e8) SHA1(d7597d50c0f4f9aa6d990c8d3b485e39cb44ff06) ) // AT27C512R
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, arb,   0,      0,      arb,     arb,   arb_state, empty_init, "AVE Micro Systems", "Auto Response Board", MACHINE_SUPPORTS_SAVE )
SYST( 2012, arbv2, arb,    0,      v2,      arb,   arb_state, empty_init, "hack (Steve Braid)", "ARB V2 Sargon 4.0", MACHINE_SUPPORTS_SAVE )
