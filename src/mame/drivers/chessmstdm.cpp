// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
/******************************************************************************

Chess-Master Diamond (G-5004-500), by VEB Mikroelektronik "Karl Marx" Erfurt

The chess engine is a continuation of the older Chess-Master model. So it
plays quite weak when compared with other chess computers from 1987.

PM10 and PM11 modules were included (not separate purchases). No other modules
were released. The player is supposed to hotswap them with the [CH M] option.

Hardware notes:
- UA880 Z80 @ 4MHz
- 2*Z80 PIO
- 16KB ROM (2*U2364D), 3KB RAM (6*U224D)
- 4-digit 16seg display
- module slot for opening book/endgame
- chessboard with 64 hall sensors, 64+2 leds, beeper

TODO:
- hotswapping module doesn't work since MAME forces a hard reset
- the 555 only connects to BSTB pin, why is the data_b_write workaround needed?

******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/z80pio.h"
#include "machine/sensorboard.h"
#include "sound/beep.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "chessmstdm.lh"


namespace {

class chessmstdm_state : public driver_device
{
public:
	chessmstdm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio%u", 0),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_digit_pwm(*this, "digit_pwm"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(monitor_button);
	DECLARE_INPUT_CHANGED_MEMBER(view_button);

	void chessmstdm(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_digit_pwm;
	required_device<beep_device> m_beeper;
	required_ioport_array<2> m_inputs;
	output_finder<4> m_digits;

	void chessmstdm_mem(address_map &map);
	void chessmstdm_io(address_map &map);

	void reset_w(u8 data = 0);
	u8 reset_r();
	void digits_w(u8 data);
	void pio1_port_a_w(u8 data);
	void pio1_port_b_w(u8 data);
	u8 pio2_port_a_r();
	void pio2_port_b_w(u8 data);

	void update_leds();
	void update_digits();

	u16 m_matrix = 0;
	u8 m_led_data = 0;
	u8 m_direct_leds = 0;
	u8 m_digit_matrix = 0;
	int m_digit_dot = 0;
	u16 m_digit_data = 0;
};

void chessmstdm_state::machine_start()
{
	m_digits.resolve();

	// register for savestates
	save_item(NAME(m_matrix));
	save_item(NAME(m_direct_leds));
	save_item(NAME(m_led_data));
	save_item(NAME(m_digit_matrix));
	save_item(NAME(m_digit_dot));
	save_item(NAME(m_digit_data));
}

void chessmstdm_state::machine_reset()
{
	// reset digit shift registers
	m_digit_data = 0;
	update_digits();
}



/******************************************************************************
    I/O
******************************************************************************/

void chessmstdm_state::reset_w(u8 data)
{
	// accessing 9cxx asserts a reset
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	machine_reset();
}

u8 chessmstdm_state::reset_r()
{
	if (!machine().side_effects_disabled())
		reset_w();

	return 0xff;
}

void chessmstdm_state::update_digits()
{
	u16 digit_data = bitswap<16>(m_digit_data, 3,5,12,10,14,1,2,13,8,6,11,15,7,9,4,0);
	m_digit_pwm->matrix(m_digit_matrix, digit_data | (m_digit_dot << 16));
}

void chessmstdm_state::digits_w(u8 data)
{
	// d0-d3: shift digit segment data into 4015 shift registers
	m_digit_data = (m_digit_data << 4) | (data & 0x0f);

	// d4-d7: digit select
	m_digit_matrix = (data >> 4) & 0x0f;
	update_digits();
}

void chessmstdm_state::update_leds()
{
	m_led_pwm->matrix((m_matrix & 0xff) | (m_direct_leds << 8), m_led_data);
}

void chessmstdm_state::pio1_port_a_w(u8 data)
{
	// d0-d7: chessboard led data
	m_led_data = ~data;
	update_leds();
}

void chessmstdm_state::pio1_port_b_w(u8 data)
{
	// d2: input mux highest bit
	m_matrix = (m_matrix & 0xff) | ((data & 0x04) << 6);

	// d3: enable beeper
	m_beeper->set_state(BIT(data, 3));

	// d4: digits DP
	m_digit_dot = BIT(data, 4);
	update_digits();

	// d5: monitor led, d6: playmode led
	m_direct_leds = data >> 5 & 3;
	update_leds();

	// d4 and d7 also go to cartslot, but unused
}

u8 chessmstdm_state::pio2_port_a_r()
{
	u8 data = 0;

	// read chessboard sensors
	for (int i = 0; i < 8; i++)
		if (BIT(m_matrix, i))
			data |= m_board->read_file(i);

	// read other buttons
	if (m_matrix & 0x100)
		data |= m_inputs[0]->read();

	return ~data;
}

void chessmstdm_state::pio2_port_b_w(u8 data)
{
	// d0-d7: input mux/led select
	m_matrix = (data & 0xff) | (m_matrix & ~0xff);
	update_leds();
}



/******************************************************************************
    Address Maps
******************************************************************************/

void chessmstdm_state::chessmstdm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x8000, 0x8bff).ram();
	map(0x9c00, 0x9c00).mirror(0x0300).rw(FUNC(chessmstdm_state::reset_r), FUNC(chessmstdm_state::reset_w));
}

void chessmstdm_state::chessmstdm_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7f);
	//map(0x00, 0x03).mirror(0x70) read/write to both PIOs, but not used by software
	map(0x04, 0x07).mirror(0x70).rw(m_pio[0], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x08, 0x0b).mirror(0x70).rw(m_pio[1], FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x4c, 0x4c).mirror(0x33).w(FUNC(chessmstdm_state::digits_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

INPUT_CHANGED_MEMBER(chessmstdm_state::monitor_button)
{
	view_button(field, param, oldval, newval);

	// releasing monitor button clears reset
	if (!newval && oldval)
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(chessmstdm_state::view_button)
{
	// pressing both monitor+view buttons buttons causes a reset
	if ((m_inputs[1]->read() & 0x03) == 0x03)
		reset_w();
}

static INPUT_PORTS_START( chessmstdm )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Move Fore")               PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Move Back")               PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board")                   PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Match / Time")            PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Parameter / Information") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Selection / Dialogue")    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Function / Notation")     PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Enter")                   PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Monitor") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmstdm_state, monitor_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("View")    PORT_CODE(KEYCODE_F2) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmstdm_state, view_button, 0)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

static const z80_daisy_config chessmstdm_daisy_chain[] =
{
	{ "z80pio1" },
	{ nullptr }
};

void chessmstdm_state::chessmstdm(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &chessmstdm_state::chessmstdm_mem);
	m_maincpu->set_addrmap(AS_IO, &chessmstdm_state::chessmstdm_io);
	m_maincpu->set_daisy_config(chessmstdm_daisy_chain);

	auto &strobe(CLOCK(config, "strobe", 500)); // from 555 timer, 50% duty
	strobe.signal_handler().set(m_pio[1], FUNC(z80pio_device::strobe_b));
	strobe.signal_handler().append([this](int) { m_pio[1]->data_b_write(m_matrix); });

	Z80PIO(config, m_pio[0], 8_MHz_XTAL / 2);
	m_pio[0]->out_pa_callback().set(FUNC(chessmstdm_state::pio1_port_a_w));
	m_pio[0]->out_pb_callback().set(FUNC(chessmstdm_state::pio1_port_b_w));
	m_pio[0]->in_pb_callback().set_ioport("IN.1");

	Z80PIO(config, m_pio[1], 8_MHz_XTAL / 2);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[1]->in_pa_callback().set(FUNC(chessmstdm_state::pio2_port_a_r));
	m_pio[1]->out_pb_callback().set(FUNC(chessmstdm_state::pio2_port_b_w));

	SENSORBOARD(config, m_board);
	m_board->set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_digit_pwm).set_size(4, 17);
	m_digit_pwm->set_segmask(0xf, 0x1ffff);
	m_digit_pwm->output_digit().set([this](offs_t offset, u64 data) { m_digits[offset] = data; });

	PWM_DISPLAY(config, m_led_pwm).set_size(8+2, 8);
	config.set_default_layout(layout_chessmstdm);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	BEEP(config, m_beeper, 1000).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "chessmstdm_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("chessmstdm");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( chessmstdm )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("002.d224", 0x0000, 0x2000, CRC(bed56fef) SHA1(dad0f8ddbd9b10013a5bdcc09ee6db39cfb26b78) ) // U2364D45
	ROM_LOAD("201.d225", 0x2000, 0x2000, CRC(c9dc7f29) SHA1(a3e1b66d0e15ffe83a9165d15c4a83013852c2fe) ) // "
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT       CLASS             INIT        COMPANY                                     FULLNAME                FLAGS
CONS( 1987, chessmstdm, 0,      0,      chessmstdm, chessmstdm, chessmstdm_state, empty_init, "VEB Mikroelektronik \"Karl Marx\" Erfurt", "Chess-Master Diamond", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
