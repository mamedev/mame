// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

Schachcomputer SC 2 driver

VEB Mikroelektronik's 2nd chess computer. The chess program is based on
Fidelity Chess Challenger 10(C?).

3 versions known: initial version, revision E, revision EP.

Schachcomputer SC 1 was canceled before wide release, it's assumed to
be on similar hardware(but PCB photos show 10 ROM chips instead of 9).

keypad legend:

R - Rückstellen (reset)
K - Programmstufen (level)
W - Figurenwahl (white/black)
P - Problemeingabe (problem mode)
T - Tonabschaltung (sound on/off)
L - Löschen (clear)
Q - Quittierung (enter)

Fidelity CC10 synonyms: RE, LV, RV, PB, ♪, CL, EN

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "sc2.lh"


namespace {

class sc2_state : public driver_device
{
public:
	sc2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pio(*this, "z80pio"),
		m_dac(*this, "dac"),
		m_keypad(*this, "LINE%u", 1),
		m_digits(*this, "digit%u", 0U),
		m_leds(*this, "led%u", 0U)
	{ }

	void sc2(machine_config &config);

	// Rückstellen is also tied to CPU RESET
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pio;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<4> m_keypad;

	output_finder<4> m_digits;
	output_finder<2> m_leds;

	void sc2_io(address_map &map);
	void sc2_mem(address_map &map);

	uint8_t m_kp_matrix;
	uint8_t m_led_selected;
	uint8_t m_digit_data;

	void sc2_update_display();
	DECLARE_READ8_MEMBER(pio_port_b_r);
	DECLARE_WRITE8_MEMBER(pio_port_a_w);
	DECLARE_WRITE8_MEMBER(pio_port_b_w);
	DECLARE_READ8_MEMBER(speaker_w);
	template<int State> DECLARE_READ8_MEMBER(speaker_w);
};

void sc2_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	m_kp_matrix = 0;
	m_led_selected = 0;
	m_digit_data = 0;

	save_item(NAME(m_kp_matrix));
	save_item(NAME(m_led_selected));
	save_item(NAME(m_digit_data));
}


template<int State>
READ8_MEMBER(sc2_state::speaker_w)
{
	if (!machine().side_effects_disabled())
		m_dac->write(State);

	return 0xff;
}

void sc2_state::sc2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom();
	map(0x1000, 0x13ff).ram();
	map(0x2000, 0x33ff).rom();
	map(0x3400, 0x3400).r(FUNC(sc2_state::speaker_w<1>));
	map(0x3c00, 0x3c00).r(FUNC(sc2_state::speaker_w<0>));
}

void sc2_state::sc2_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).mirror(0xfc).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}


void sc2_state::sc2_update_display()
{
	// latch display data
	for (int i = 0; i < 4; i++)
	{
		if (!BIT(m_led_selected, i))
		{
			m_digits[i] = m_digit_data & 0x7f;

			// schach/matt leds
			if (i < 2)
				m_leds[i] = BIT(m_digit_data, 7);
		}
	}
}

READ8_MEMBER(sc2_state::pio_port_b_r)
{
	uint8_t data = 0x0f;

	// read keypad matrix
	for (int i = 0; i < 4; i++)
		if (BIT(m_kp_matrix, i))
			data |= m_keypad[i]->read();

	return data;
}

WRITE8_MEMBER(sc2_state::pio_port_a_w)
{
	// digit segment data
	m_digit_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
}

WRITE8_MEMBER(sc2_state::pio_port_b_w)
{
	// d0-d3: keypad mux(active high), led mux(active low)
	if (data != 0xf1 && data != 0xf2 && data != 0xf4 && data != 0xf8)
	{
		m_led_selected = data;
		sc2_update_display();
	}
	else
		m_kp_matrix = data;
}


/* Input ports */

static INPUT_PORTS_START( sc2 )
	PORT_START("LINE1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("LINE2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)

	PORT_START("LINE3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("LINE4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, sc2_state, reset_button, nullptr)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P") PORT_CODE(KEYCODE_O)
INPUT_PORTS_END


/* Machine config */

void sc2_state::sc2(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 9.8304_MHz_XTAL/4); // U880 Z80 clone
	m_maincpu->set_addrmap(AS_PROGRAM, &sc2_state::sc2_mem);
	m_maincpu->set_addrmap(AS_IO, &sc2_state::sc2_io);

	/* video hardware */
	config.set_default_layout(layout_sc2);

	/* devices */
	Z80PIO(config, m_pio, 9.8304_MHz_XTAL/4);
	m_pio->out_pa_callback().set(FUNC(sc2_state::pio_port_a_w));
	m_pio->in_pb_callback().set(FUNC(sc2_state::pio_port_b_r));
	m_pio->out_pb_callback().set(FUNC(sc2_state::pio_port_b_w));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}


/* ROM definition */

ROM_START( sc2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm008.bin", 0x0000, 0x0400, CRC(3023ea82) SHA1(07020d153d802c672c39e1af3c716dbe35e23f08) )
	ROM_LOAD( "bm009.bin", 0x0400, 0x0400, CRC(6a34814e) SHA1(e58ae6615297b028db135a48a8f9e186a4220f4f) )
	ROM_LOAD( "bm010.bin", 0x0800, 0x0400, CRC(deab0373) SHA1(81c9a7197eef8d9131e47ecd2ec35b943caee54e) )
	ROM_LOAD( "bm011.bin", 0x0c00, 0x0400, CRC(c8282339) SHA1(8d6b8861281e967a77609b6d77e80afd47d28ed2) )
	ROM_LOAD( "bm012.bin", 0x2000, 0x0400, CRC(2e6a4294) SHA1(7b9bd191c9ec73139a65c3a339ab88e1f3eb5ed2) )
	ROM_LOAD( "bm013.bin", 0x2400, 0x0400, CRC(3e02eb42) SHA1(2e4a9a8fd04c202c9518550d7e8cf9bfea394153) )
	ROM_LOAD( "bm014.bin", 0x2800, 0x0400, CRC(538d449e) SHA1(c4186995b69e97740e01eaff84a20d49d03d180f) )
	ROM_LOAD( "bm015.bin", 0x2c00, 0x0400, CRC(b4991dca) SHA1(6a6cdddf5c4afa24773acf693f58c34b99c8d328) )
	ROM_LOAD( "bm037.bin", 0x3000, 0x0400, CRC(2b67faf1) SHA1(5c65734acaeb766240dbd492a774c56fcfc382f7) )
ROM_END

ROM_START( sc2a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bm008.bin", 0x0000, 0x0400, CRC(3023ea82) SHA1(07020d153d802c672c39e1af3c716dbe35e23f08) )
	ROM_LOAD( "bm009.bin", 0x0400, 0x0400, CRC(6a34814e) SHA1(e58ae6615297b028db135a48a8f9e186a4220f4f) )
	ROM_LOAD( "bm010.bin", 0x0800, 0x0400, CRC(deab0373) SHA1(81c9a7197eef8d9131e47ecd2ec35b943caee54e) )
	ROM_LOAD( "bm011.bin", 0x0c00, 0x0400, CRC(c8282339) SHA1(8d6b8861281e967a77609b6d77e80afd47d28ed2) )
	ROM_LOAD( "bm012.bin", 0x2000, 0x0400, CRC(2e6a4294) SHA1(7b9bd191c9ec73139a65c3a339ab88e1f3eb5ed2) )
	ROM_LOAD( "bm013.bin", 0x2400, 0x0400, CRC(3e02eb42) SHA1(2e4a9a8fd04c202c9518550d7e8cf9bfea394153) )
	ROM_LOAD( "bm014.bin", 0x2800, 0x0400, CRC(538d449e) SHA1(c4186995b69e97740e01eaff84a20d49d03d180f) )
	ROM_LOAD( "bm015.bin", 0x2c00, 0x0400, CRC(b4991dca) SHA1(6a6cdddf5c4afa24773acf693f58c34b99c8d328) )
	ROM_LOAD( "bm016.bin", 0x3000, 0x0400, CRC(4fe0853a) SHA1(c2253e320778b0ea468fb54f26ae83d07f9700e6) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
COMP( 1981, sc2,  0,      0,      sc2,     sc2,   sc2_state, empty_init, "VEB Mikroelektronik Erfurt", "Schachcomputer SC 2 (rev. E)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
COMP( 1981, sc2a, sc2,    0,      sc2,     sc2,   sc2_state, empty_init, "VEB Mikroelektronik Erfurt", "Schachcomputer SC 2", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
