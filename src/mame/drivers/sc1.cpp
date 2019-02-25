// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

Schachcomputer SC 1 driver

VEB Mikroelektronik's 1st chess computer. It was canceled before wide release.

TODO:
- speaker, it's very noisy if hooked up as it is now, missing enable-bit?
  it still toggles matrix d1 if T(tone off) is pressed
- LEDs, they're not on digit d7
- 7seg sometimes flashes
- setting level doesn't work? game should boot with "PS 1"

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "sc1.lh"


namespace {

class sc1_state : public driver_device
{
public:
	sc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_dac(*this, "dac"),
		m_keypad(*this, "LINE%u", 1),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void sc1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_keypad;

	output_finder<4> m_digits;
	output_finder<2> m_leds;

	void sc1_io(address_map &map);
	void sc1_mem(address_map &map);

	uint8_t m_matrix;

	DECLARE_WRITE8_MEMBER(matrix_w);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_READ8_MEMBER(pio_port_b_r);
};

void sc1_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	m_matrix = 0;
	save_item(NAME(m_matrix));
}

/***************************************************************************

    Display

***************************************************************************/

WRITE8_MEMBER(sc1_state::pio_port_a_w)
{
	// digit segment data
	uint8_t digit = bitswap<8>(data,3,4,6,0,1,2,7,5) & 0x7f;

	if (m_matrix & 0x04)
		m_digits[3] = digit;
	if (m_matrix & 0x08)
		m_digits[2] = digit;
	if (m_matrix & 0x10)
		m_digits[1] = digit;
	if (m_matrix & 0x20)
		m_digits[0] = digit;
}


/***************************************************************************

    Keyboard

***************************************************************************/

WRITE8_MEMBER(sc1_state::matrix_w)
{
	// d1: speaker out
	//m_dac->write(BIT(data, 1));

	// keypad/led mux
	m_matrix = data;
}

READ8_MEMBER(sc1_state::pio_port_b_r)
{
	uint8_t data = 0;

	// read keypad matrix
	for (int i = 0; i < 8; i++)
		if (BIT(m_matrix, i))
			data |= m_keypad[i]->read();

	return data;
}


void sc1_state::sc1_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x43ff).ram();
}

void sc1_state::sc1_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xfc, 0xfc).w(FUNC(sc1_state::matrix_w));
}


/* Input ports */

static INPUT_PORTS_START( sc1 )
	PORT_START("LINE1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)

	PORT_START("LINE2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("LINE3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("LINE4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)

	PORT_START("LINE5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CE") PORT_CODE(KEYCODE_R)

	PORT_START("LINE6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("LINE7")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W/S") PORT_CODE(KEYCODE_W)

	PORT_START("LINE8")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END


/* Machine config */

void sc1_state::sc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 9.8304_MHz_XTAL/4); // U880 Z80 clone
	m_maincpu->set_addrmap(AS_PROGRAM, &sc1_state::sc1_mem);
	m_maincpu->set_addrmap(AS_IO, &sc1_state::sc1_io);

	/* video hardware */
	config.set_default_layout(layout_sc1);

	/* devices */
	Z80PIO(config, m_pio, 9.8304_MHz_XTAL/4);
	m_pio->out_pa_callback().set(FUNC(sc1_state::pio_port_a_w));
	m_pio->in_pb_callback().set(FUNC(sc1_state::pio_port_b_r));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}


/* ROM definition */

ROM_START( sc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY                       FULLNAME               FLAGS
COMP( 1981, sc1,  0,      0,      sc1,     sc1,   sc1_state, empty_init, "VEB Mikroelektronik Erfurt", "Schachcomputer SC 1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
