// license:BSD-3-Clause
// copyright-holders:hap
/******************************************************************************

AVR-Max Chess Computer, aka "SHAH"
In Germany it was named AVR-Max-Schachzwerg

The chess engine is H.G. Muller's micro-Max 4.8, ATMega88 port and prototype
hardware design by Andre Adrian. PCB finalization by Elektor, published in
Elektor magazine in 2009.

FN 1 = new game, FN 2 = set level, FN 3 = principle variation.
Moves are confirmed by pressing GO twice.

Hardware notes:
- PCB label 081101-1 (c)Elektor v1.4
- Atmel ATMega88P-20PU @ ~8MHz, 8KB internal ROM
- 4-digit 7seg display, button panel, no sound

TODO:
- AVR8 SLEEP opcode is not working, it's used for power-saving here and was
  harmless to hack out, but needs to be put back when it's emulated
- add Elektor CC2 version with custom LCD screen

******************************************************************************/

#include "emu.h"
#include "cpu/avr8/avr8.h"
#include "video/pwm.h"

// internal artwork
#include "avrmax.lh" // clickable


namespace {

class avrmax_state : public driver_device
{
public:
	avrmax_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void avrmax(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<avr8_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_inputs;

	// address maps
	void main_map(address_map &map);
	void data_map(address_map &map);

	// I/O handlers
	void update_display();
	void mux_w(u8 data);
	void led_w(u8 data);
	u8 input_r();

	u8 m_inp_mux = 0;
	u8 m_7seg_data = 0;
};

void avrmax_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
}



/******************************************************************************
    I/O
******************************************************************************/

void avrmax_state::update_display()
{
	m_display->matrix(m_inp_mux, m_7seg_data);
}

void avrmax_state::mux_w(u8 data)
{
	// PC0-PC3: input mux/digit select
	m_inp_mux = ~data & 0xf;
	update_display();
}

void avrmax_state::led_w(u8 data)
{
	// PD0-PD7: 7seg data
	m_7seg_data = ~data;
	update_display();
}

u8 avrmax_state::input_r()
{
	u8 data = 0;

	// PB0-PB2: multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data & 7;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void avrmax_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
}

void avrmax_state::data_map(address_map &map)
{
	map(0x0100, 0x04ff).ram();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( avrmax )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("FN") PORT_CODE(KEYCODE_N)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("GO") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void avrmax_state::avrmax(machine_config &config)
{
	/* basic machine hardware */
	ATMEGA88(config, m_maincpu, 8000000); // internal R/C clock
	m_maincpu->set_addrmap(AS_PROGRAM, &avrmax_state::main_map);
	m_maincpu->set_addrmap(AS_DATA, &avrmax_state::data_map);
	m_maincpu->set_eeprom_tag("eeprom");
	m_maincpu->gpio_in<AVR8_IO_PORTB>().set(FUNC(avrmax_state::input_r));
	m_maincpu->gpio_out<AVR8_IO_PORTC>().set(FUNC(avrmax_state::mux_w));
	m_maincpu->gpio_out<AVR8_IO_PORTD>().set(FUNC(avrmax_state::led_w));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0xff);
	config.set_default_layout(layout_avrmax);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( avrmax )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "elektor_081101-41.ic1", 0x0000, 0x2000, CRC(86d2a654) SHA1(3c235b8f6f735eaf408f54cbf44872166e7161d5) )

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x025c, 2, 0x00 )
	ROM_FILL( 0x0f8e, 2, 0x00 )
	ROM_FILL( 0x0fde, 2, 0x00 )
	ROM_FILL( 0x1020, 2, 0x00 )
	ROM_FILL( 0x1060, 2, 0x00 )
	ROM_FILL( 0x129a, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

ROM_START( avrmaxg )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "elektor_081101-41_d.ic1", 0x0000, 0x2000, CRC(18ec7a56) SHA1(a018421aa0ad8cce3d852f7519dec3691f3c55a0) )

	// HACK: changed SLEEP to NOP
	ROM_FILL( 0x025c, 2, 0x00 )
	ROM_FILL( 0x0f8e, 2, 0x00 )
	ROM_FILL( 0x0fde, 2, 0x00 )
	ROM_FILL( 0x1020, 2, 0x00 )
	ROM_FILL( 0x1060, 2, 0x00 )
	ROM_FILL( 0x129a, 2, 0x00 )

	ROM_REGION( 0x200, "eeprom", ROMREGION_ERASE00 )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT CMP MACHINE  INPUT   STATE         INIT        COMPANY, FULLNAME, FLAGS
CONS( 2009, avrmax,  0,      0, avrmax,  avrmax, avrmax_state, empty_init, "Elektor", "AVR-Max Chess Computer (English)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW )
CONS( 2009, avrmaxg, avrmax, 0, avrmax,  avrmax, avrmax_state, empty_init, "Elektor", "AVR-Max-Schachzwerg (German)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_NO_SOUND_HW ) // German 'text'
