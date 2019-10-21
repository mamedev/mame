// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

This appears to be a prototype or alternate version of SLC 1.
The ROM was first assumed to be VEB SC 1, but unfortunately it isn't.

TODO:
- merge with slc1.cpp? but hardware differs too much
- any way to access the "Lern" part?
- speaker, it's very noisy if hooked up as it is now
- LED(s)? they're not on digit d7
- 7seg sometimes flashes

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "slc1a.lh"


namespace {

class slc1_state : public driver_device
{
public:
	slc1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_dac(*this, "dac"),
		m_keypad(*this, "LINE%u", 1),
		m_digits(*this, "digit%u", 0U)
	{ }

	void slc1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_keypad;

	output_finder<4> m_digits;

	void main_io(address_map &map);
	void main_map(address_map &map);

	uint8_t m_matrix;

	DECLARE_WRITE8_MEMBER(matrix_w);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_READ8_MEMBER(pio_port_b_r);
};

void slc1_state::machine_start()
{
	m_digits.resolve();

	m_matrix = 0;
	save_item(NAME(m_matrix));
}

/***************************************************************************

    Display

***************************************************************************/

WRITE8_MEMBER(slc1_state::pio_port_a_w)
{
	// digit segment data
	uint8_t digit = bitswap<8>(data,3,4,6,0,1,2,7,5);

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

WRITE8_MEMBER(slc1_state::matrix_w)
{
	// d1: speaker out
	//m_dac->write(BIT(data, 1));

	// keypad/led mux
	m_matrix = data;
}

READ8_MEMBER(slc1_state::pio_port_b_r)
{
	uint8_t data = 0;

	// read keypad matrix
	for (int i = 0; i < 8; i++)
		if (BIT(m_matrix, i))
			data |= m_keypad[i]->read();

	return data;
}


void slc1_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x43ff).ram();
}

void slc1_state::main_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_pio, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xfc, 0xfc).w(FUNC(slc1_state::matrix_w));
}


/* Input ports */

static INPUT_PORTS_START( slc1 )
	PORT_START("LINE1")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4 T") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)

	PORT_START("LINE2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2 S") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6 K") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("LINE3")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3 L") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("LINE4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1 B") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5 D") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)

	PORT_START("LINE5")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_R)

	PORT_START("LINE6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A") PORT_CODE(KEYCODE_O)

	PORT_START("LINE7")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("LINE8")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("St") PORT_CODE(KEYCODE_S) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL)
INPUT_PORTS_END


/* Machine config */

void slc1_state::slc1(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 2500000); // U880 Z80 clone
	m_maincpu->set_addrmap(AS_PROGRAM, &slc1_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &slc1_state::main_io);

	/* video hardware */
	config.set_default_layout(layout_slc1a);

	/* devices */
	Z80PIO(config, m_pio, 2500000);
	m_pio->out_pa_callback().set(FUNC(slc1_state::pio_port_a_w));
	m_pio->in_pb_callback().set(FUNC(slc1_state::pio_port_b_r));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}


/* ROM definition */

ROM_START( slc1a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sc1.rom", 0x0000, 0x1000, CRC(26965b23) SHA1(01568911446eda9f05ec136df53da147b7c6f2bf))
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY, FULLNAME, FLAGS
COMP( 1989, slc1a, slc1,   0,      slc1,    slc1,  slc1_state, empty_init, "Dieter Scheuschner", "Schach- und Lerncomputer SLC 1 (prototype?)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
