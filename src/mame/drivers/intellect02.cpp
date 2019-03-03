// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

Интеллект-02 (Intellect-02) driver

This is a Soviet electronic board game console, a dozen or so cartridge games were
made for it(can't say for certain how many released). PCB labels have prefix ДМП,
it's assumed to have been designed by НИИ БРЭА (SRI BREA). First shown in 1983,
produced during around 1985-1992.

hardware notes:
- КР580ВМ80А CPU (i8080A clone), speed unknown
- КР580ИК55 (i8255 clone)
- 1 KB RAM (8*КР565РУ2), cartridge port
- 4-digit 7seg panel, 2 leds, 16 buttons, game board above it

The chess/checkers board is detachable, with a board for Kalah underneath it.

The hardware is very similar to Fidelity Chess Challenger 3. Actually, one of the
first cartridges for this system, a Chess program, is a modified Chess Challenger 3 ROM.
The chess engine was kept identical. (note: the "lose" LED is used for "check")

Intellect-01 looks like it didn't get further than a prototype. It was a dedicated
chess computer, probably a clone of CC3.

keypad legend:

СБ - сброс (reset)
ВВ - ввод (input)
ВИ - выбор игры (game select)
СТ - стирание (erase)
ПП - просмотр позиции (view position)
УИ - уровень игры (game level)

TODO:
- identify ПП, УИ, 0, 9 buttons (only the chess program is dumped and it doesn't use them)
- verify cpu speed

******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "softlist.h"

// internal artwork
#include "intellect02.lh" // clickable


namespace {

class intel02_state : public driver_device
{
public:
	intel02_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255"),
		m_led_delay(*this, "led_delay_%u", 0),
		m_buttons(*this, "IN.%u", 0),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_digit(*this, "digit%u", 0U),
		m_led(*this, "led%u", 0U)
	{ }

	// machine drivers
	void intel02(machine_config &config);

	// assume that reset button is tied to RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	required_device_array<timer_device, 6> m_led_delay;
	required_ioport_array<2> m_buttons;
	required_device<dac_bit_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	output_finder<4> m_digit;
	output_finder<2> m_led;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cartridge);

	// display stuff
	void update_display();
	TIMER_DEVICE_CALLBACK_MEMBER(led_delay_off);

	u8 m_7seg_data;
	u8 m_led_select;
	u8 m_led_active;

	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(_7seg_w);
	DECLARE_WRITE8_MEMBER(control_w);
};

void intel02_state::machine_start()
{
	// resolve handlers
	m_led.resolve();
	m_digit.resolve();

	// zerofill
	m_7seg_data = 0;
	m_led_select = 0;
	m_led_active = 0;

	// register for savestates
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_active));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// cartridge

DEVICE_IMAGE_LOAD_MEMBER(intel02_state, cartridge)
{
	u32 size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}


// misc display handling

void intel02_state::update_display()
{
	// latch digits (low 4 bits of led select)
	for (int i = 0; i < 4; i++)
	{
		if (BIT(m_led_select, i))
			m_digit[i] = m_7seg_data;
		else if (!BIT(m_led_active, i))
			m_digit[i] = 0;
	}

	// led select d4: lose led, d5: win led
	m_led[0] = BIT(m_led_active, 4);
	m_led[1] = BIT(m_led_active, 5);
}

TIMER_DEVICE_CALLBACK_MEMBER(intel02_state::led_delay_off)
{
	u8 mask = 1 << param;
	m_led_active = (m_led_active & ~mask) | (m_led_select & mask);
	update_display();
}


// I8255 PPI

READ8_MEMBER(intel02_state::input_r)
{
	// d0-d3: buttons through a priority encoder
	// d4-d7: buttons (direct)
	return (count_leading_zeros(m_buttons[0]->read()) - 17) | (~m_buttons[1]->read() << 4 & 0xf0);
}

WRITE8_MEMBER(intel02_state::_7seg_w)
{
	// d0-d7: digit segment data
	m_7seg_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();
}

WRITE8_MEMBER(intel02_state::control_w)
{
	// d0-d5: select digit/leds
	for (int i = 0; i < 6; i++)
	{
		if (BIT(data, i))
			m_led_active |= 1 << i;

		// on falling edge, delay it going off to prevent flicker
		else if (BIT(m_led_select, i))
			m_led_delay[i]->adjust(attotime::from_msec(10), i);
	}

	m_led_select = data;
	update_display();

	// d6: unused?
	// it's a timer trigger on Chess Challenger 3

	// d7: speaker out
	m_dac->write(BIT(data, 7));
}



/******************************************************************************
    Address Maps
******************************************************************************/

void intel02_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xf000, 0xf3ff).ram();
}

void intel02_state::main_io(address_map &map)
{
	map.global_mask(0xff);
	map(0xf4, 0xf7).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( intel02 )
	PORT_START("IN.0")
	PORT_BIT(0x003f, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Input") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Game Select") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Erase") PORT_CODE(KEYCODE_DEL)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Reset") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, intel02_state, reset_button, nullptr)
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void intel02_state::intel02(machine_config &config)
{
	/* basic machine hardware */
	I8080A(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &intel02_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &intel02_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(intel02_state::input_r));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->out_pb_callback().set(FUNC(intel02_state::_7seg_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->out_pc_callback().set(FUNC(intel02_state::control_w));

	/* display hardware */
	for (int i = 0; i < 6; i++)
		TIMER(config, m_led_delay[i]).configure_generic(FUNC(intel02_state::led_delay_off));

	config.set_default_layout(layout_intellect02);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "intellect02", "bin");
	m_cart->set_device_load(device_image_load_delegate(&intel02_state::device_image_load_cartridge, this));
	m_cart->set_must_be_loaded(true);

	SOFTWARE_LIST(config, "cart_list").set_original("intellect02");
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( intel02 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00 )
	// nothing here, it's on a cartridge
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE  INPUT    STATE          INIT        COMPANY, FULLNAME, FLAGS
CONS( 1985, intel02, 0,      0, intel02, intel02, intel02_state, empty_init, "BREA Research Institute", "Intellect-02", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
