// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************
*
* ave_arb.cpp, subdriver of machine/chessbase.cpp

AVE Micro Systems ARB chess computer driver, in some regions redistributed
by Chafitz, and in Germany by Sandy Electronic.

*******************************************************************************

Auto Response Board (ARB) overview:
- R6502P CPU @ 2MHz(4MHz XTAL), R6522P VIA
- 2KB RAM(4*2114), cartridge port
- magnetic chessboard, 8*8+12 leds
- PCB label AV001C01 REV A

The electronic magnetic chessboard is the first of is kind. AVE later licensed
it to Fidelity (see fidel_elite.cpp).
ARB is a romless system, the program ROM is on a cartridge.

Known modules (*denotes not dumped yet):
- Sargon 2.5
- *Grand Master Series 3
- *Grand Master Series 3.5
- *Grand Master Series 4.0

Newer modules included button label stickers for OPTIONS, Verify, Take Back, Clear.

******************************************************************************/

#include "emu.h"
#include "includes/chessbase.h"

#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

// internal artwork
#include "ave_arb.lh" // clickable


namespace {

class arb_state : public chessbase_state
{
public:
	arb_state(const machine_config &mconfig, device_type type, const char *tag) :
		chessbase_state(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via(*this, "via"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot")
	{ }

	// halt button is tied to NMI, reset button to RESET(but only if halt button is held)
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, (newval && m_inp_matrix[9]->read() & 2) ? ASSERT_LINE : CLEAR_LINE); }
	DECLARE_INPUT_CHANGED_MEMBER(halt_button) { m_maincpu->set_input_line(M6502_NMI_LINE, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine drivers
	void arb(machine_config &config);

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<dac_bit_interface> m_dac;
	required_device<generic_slot_device> m_cart;

	// address maps
	void main_map(address_map &map);

	// cartridge
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);
	DECLARE_READ8_MEMBER(cartridge_r);
	u32 m_cart_mask;

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);
};


/******************************************************************************
    Devices, I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(arb_state, cartridge)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart_mask = ((1 << (31 - count_leading_zeros(size))) - 1) & 0x7fff;

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

READ8_MEMBER(arb_state::cartridge_r)
{
	// range is 0x4000-0x7fff + 0xc000-0xffff
	if (offset >= 0x8000)
		offset -= 0x4000;

	return m_cart->read_rom(offset & m_cart_mask);
}


// R6522 ports

void arb_state::prepare_display()
{
	// 12 led column data lines via 3 7475
	u16 mask = 0;
	mask |= (m_led_select & 1) ? 0xf00 : 0;
	mask |= (m_led_select & 2) ? 0x0ff : 0;

	m_led_data = (m_led_data & ~mask) | ((m_led_latch << 8 | m_led_latch) & mask);
	display_matrix(12, 9+1, m_led_data, m_inp_mux | 0x200);
}

WRITE8_MEMBER(arb_state::leds_w)
{
	// PA0-PA7: led latch input
	m_led_latch = ~data & 0xff;
	prepare_display();
}

WRITE8_MEMBER(arb_state::control_w)
{
	// PB0-PB3: 74145 A-D
	// 74145 0-8: input mux, led row select
	m_inp_mux = 1 << (data & 0xf) & 0x1ff;

	// PB4,PB5: led group select
	m_led_select = data >> 4 & 3;
	prepare_display();

	// PB7: speaker out
	m_dac->write(BIT(data, 7));
}

READ8_MEMBER(arb_state::input_r)
{
	// PA0-PA7: multiplexed inputs
	return ~read_inputs(9);
}



/******************************************************************************
    Address Maps
******************************************************************************/

void arb_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x3800).ram().share("nvram");
	map(0x4000, 0xffff).r(FUNC(arb_state::cartridge_r)); // gap in 0x8000-0xbfff
	map(0x8000, 0x800f).mirror(0x3ff0).rw(m_via, FUNC(via6522_device::read), FUNC(via6522_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( arb )
	PORT_INCLUDE( generic_cb_magnets )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_7) PORT_NAME("Hint / Black")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_CODE(KEYCODE_6) PORT_NAME("Variable / Clear / White / 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_CODE(KEYCODE_5) PORT_NAME("Monitor / Take Back / King / 5")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_4) PORT_NAME("Self Play / Verify / Queen / 4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_3) PORT_NAME("Change Board / Rook / 3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_2) PORT_NAME("Change Color / Bishop / 2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_1) PORT_NAME("Change Level / Knight / 1")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CODE(KEYCODE_0) PORT_NAME("New Game / Options / Pawn / 0")

	PORT_START("IN.9")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, arb_state, reset_button, nullptr)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("Halt") PORT_CHANGED_MEMBER(DEVICE_SELF, arb_state, halt_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void arb_state::arb(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 4_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &arb_state::main_map);

	VIA6522(config, m_via, 4_MHz_XTAL/4);
	m_via->writepa_handler().set(FUNC(arb_state::leds_w));
	m_via->writepb_handler().set(FUNC(arb_state::control_w));
	m_via->readpa_handler().set(FUNC(arb_state::input_r));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, "display_decay").configure_periodic(FUNC(arb_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_ave_arb);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "arb", "bin");
	m_cart->set_device_load(device_image_load_delegate(&arb_state::device_image_load_cartridge, this));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("arb");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( arb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME  PARENT CMP MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS */
CONS( 1980, arb,  0,      0, arb,     arb,   arb_state, empty_init, "AVE Micro Systems", "Auto Response Board", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
